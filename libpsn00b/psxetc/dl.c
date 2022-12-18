/*
 * PSn00bSDK dynamic linker
 * (C) 2021-2022 spicyjpeg - MPL licensed
 *
 * The bulk of this code is MIPS-specific but not PS1-specific, so the whole
 * dynamic linker could be ported to other MIPS platforms that do not have one
 * in their OS/SDK (e.g. N64, PIC32 microcontrollers). Note that, despite the
 * various ELF references, I did *not* implement a full ELF file parser: this
 * implementation just expects all library files to begin with several metadata
 * sections (.dynamic, .dynsym, .hash, .dynstr) laid out in a fixed order. Use
 * the dll.ld linker script to generate compatible libraries.
 *
 * Most of the stuff implemented here, despite looking like black magic, is
 * actually well-documented across several PDFs scattered around the internet:
 * - http://www.sco.com/developers/devspecs/gabi41.pdf
 * - http://math-atlas.sourceforge.net/devel/assembly/mipsabi32.pdf
 * - http://flint.cs.yale.edu/cs422/doc/ELF_Format.pdf
 *
 * TODO:
 * - Clean up duplicated code, especially hash-table-related loops
 * - Fix bugs and improve allocation checks
 * - Find a way to predict the smallest ELF hash table size for a certain set
 *   of entries
 */

#undef  SDK_LIBRARY_NAME
#define SDK_LIBRARY_NAME "psxetc/dl"

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <elf.h>
#include <dlfcn.h>
#include <string.h>
#include <psxapi.h>

/* Private types */

typedef struct {
	uint32_t hash;
	void     *ptr;
} MapEntry;

typedef struct {
	int nbucket, nchain, index;

	MapEntry *entries;
	uint32_t *bucket;
	uint32_t *chain;
} SymbolMap;

/* Internal globals */

static SymbolMap _symbol_map;

// Accessed by _dl_resolve_helper, stores the pointer to the current resolver
// function. Can be changed using DL_SetResolveCallback().
void *(*_dl_resolve_callback)(DLL *, const char *) = 0;

/* Private utilities */

void _dl_resolve_wrapper(void);

// Called by _dl_resolve_wrapper() (which is in turn called by GCC stubs) to
// resolve a function.
void *_dl_resolve_helper(DLL *dll, uint32_t index) {
	Elf32_Sym  *sym   = &(dll->symtab[index]);
	const char *_name = &(dll->strtab[sym->st_name]);
	void       *addr;

	if (_dl_resolve_callback)
		addr = _dl_resolve_callback(dll, _name);
	else
		addr = DL_GetMapSymbol(_name);

	if (!addr) {
		_sdk_log("FATAL! can't resolve %s, aborting\n", _name);
		abort();
	}

	// Patch the GOT entry to "cache" the resolved address.
	int _index = index - dll->first_got_symbol;
	dll->got[dll->got_local_count + _index] = (uint32_t) addr;

	return addr;
}

// Implementation of the weird obscure hashing function used in the ELF .hash
// section. Not sure how collision-prone this is.
// https://en.wikipedia.org/wiki/PJW_hash_function
static uint32_t _elf_hash(const char *str) {
	uint32_t value = 0;

	while (*str) {
		value <<= 4;
		value  += (uint32_t) *(str++);

		uint32_t nibble = value & 0xf0000000;
		if (nibble)
			value ^= nibble >> 24;

		value &= ~nibble;
	}

	return value;
}

/* Symbol map loading/introspection API */

int DL_InitSymbolMap(int num_entries) {
	if (_symbol_map.entries)
		DL_UnloadSymbolMap();

	// TODO: find a way to calculate the optimal number of hash table "buckets"
	// in order to minimize hash table size
	_symbol_map.nbucket = num_entries;
	_symbol_map.nchain  = num_entries;
	_symbol_map.index   = 0;
	_sdk_log("nbucket = %d, nchain = %d\n", _symbol_map.nbucket, num_entries);

	_symbol_map.entries = malloc(sizeof(MapEntry) * num_entries);
	_symbol_map.bucket  = malloc(sizeof(uint32_t) * num_entries);
	_symbol_map.chain   = malloc(sizeof(uint32_t) * num_entries);

	if (!_symbol_map.entries || !_symbol_map.bucket || !_symbol_map.chain) {
		_sdk_log("unable to allocate symbol map table\n");
		return -1;
	}

	memset(_symbol_map.bucket, 0xff, sizeof(uint32_t) * num_entries);
	memset(_symbol_map.chain,  0xff, sizeof(uint32_t) * num_entries);

	return 0;
}

void DL_UnloadSymbolMap(void) {
	if (!_symbol_map.entries)
		return;

	free(_symbol_map.entries);
	free(_symbol_map.bucket);
	free(_symbol_map.chain);

	_symbol_map.entries = 0;
	_symbol_map.bucket  = 0;
	_symbol_map.chain   = 0;
}

void DL_AddMapSymbol(const char *name, void *ptr) {
	uint32_t hash  = _elf_hash(name);
	int      index = _symbol_map.index;
	_symbol_map.index = index + 1;

	MapEntry *entry = &(_symbol_map.entries[index]);
	entry->hash = hash;
	entry->ptr  = ptr;

	// Append a reference to the entry to the hash table's chain.
	uint32_t *hash_entry = &(_symbol_map.bucket[hash % _symbol_map.nbucket]);
	while (*hash_entry != 0xffffffff)
		hash_entry = &(_symbol_map.chain[*hash_entry]);

	*hash_entry = index;
}

int DL_ParseSymbolMap(const char *ptr, size_t size) {
	int entries = 0;

	// Perform a quick scan over the entire map text and count the number of
	// newlines. This allows us to (over)estimate the number of entries and
	// allocate a sufficiently large hash table.
	for (int pos = 0; pos < size; pos++) {
		if (ptr[pos] == '\n')
			entries++;
	}

	int err = DL_InitSymbolMap(entries);
	if (err)
		return err;

	// Go again through the symbol map and fill in the hash table by calling
	// DL_AddMapSymbol() for each valid entry.
	entries = 0;

	for (int pos = 0; (pos < size) && ptr[pos]; pos++) {
		uint64_t full_addr;
		char     name[64], type_string[4];
		size_t   _size;

		// e.g. "main T ffffffff80000000 100 ...\n"
		int parsed = sscanf(
			&(ptr[pos]),
			"%63s %1s %Lx %x",
			name,
			type_string,
			&full_addr,
			&_size // Optional, unused (yet)
		);

		if (parsed >= 3) {
			// Drop the upper 32 bits of the address (for some reason MIPS nm
			// insists on printing 64-bit addresses... wtf) and check if the
			// entry is valid and non-null.
			void *addr = (void *) ((uint32_t) full_addr);
			char _type = toupper(type_string[0]);

			if (addr && (
				(_type == 'T') || // .text
				(_type == 'R') || // .rodata
				(_type == 'D') || // .data
				(_type == 'B')    // .bss
			)) {
				//_sdk_log("%08x, %08x [%c %s]\n", addr, _size, _type, name);

				DL_AddMapSymbol(name, addr);
				entries++;
			}
		}

		// Skip the line by advancing the pointer until a newline is found. The
		// for loop will then skip the newline itself.
		while ((pos < size) && (ptr[pos] != '\n'))
			pos++;
	}

	_sdk_log("parsed %d symbols\n", entries);
	return entries;
}

void *DL_GetMapSymbol(const char *name) {
	if (!_symbol_map.entries) {
		_sdk_log("DL_GetMapSymbol() with no map loaded\n");
		return 0;
	}

	// Go through the hash table's chain until the symbol hash matches the one
	// calculated.
	// https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-48031.html
	uint32_t hash = _elf_hash(name);

	for (int i = _symbol_map.bucket[hash % _symbol_map.nbucket]; i > 0;) {
		if (i >= _symbol_map.nchain) {
			_sdk_log(
				"DL_GetMapSymbol() index out of bounds (%d >= %d)\n",
				i, _symbol_map.nchain
			);
			return 0;
		}

		MapEntry *entry = &(_symbol_map.entries[i]);

		if (hash == entry->hash) {
			//_sdk_log("map lookup [%s = %08x]\n", name, entry->ptr);
			return entry->ptr;
		}

		i = _symbol_map.chain[i];
	}

	_sdk_log("map lookup [%s not found]\n", name);
	return 0;
}

void *DL_SetResolveCallback(void *(*callback)(DLL *, const char *)) {
	void *old_callback   = _dl_resolve_callback;
	_dl_resolve_callback = callback;

	return old_callback;
}

/* Library loading and linking API */

DLL *DL_CreateDLL(DLL *dll, void *ptr, size_t size, DL_ResolveMode mode) {
	if (!dll || !ptr)
		return 0;

	dll->ptr        = ptr;
	dll->malloc_ptr = (mode & DL_FREE_ON_DESTROY) ? ptr : 0;
	dll->size       = size;
	_sdk_log("initializing DLL at %08x\n", ptr);

	// Interpret the key-value pairs in the .dynamic section to obtain info
	// about all the other sections. The pairs are null-terminated.
	for (Elf32_Dyn *dyn = (Elf32_Dyn *) ptr; dyn->d_tag; dyn++) {
		//_sdk_log("tag %08x = %08x\n", dyn->d_tag, dyn->d_un.d_val);

		switch (dyn->d_tag) {
			// Offset of .got section
			case DT_PLTGOT:
				dll->got = (uint32_t *)
					&((uint8_t *) ptr)[dyn->d_un.d_val];
				break;

			// Offset of .hash section
			case DT_HASH:
				dll->hash = (const uint32_t *)
					&((uint8_t *) ptr)[dyn->d_un.d_val];
				break;

			// Offset of .dynstr (NOT .strtab) section
			case DT_STRTAB:
				dll->strtab = (const char *)
					&((uint8_t *) ptr)[dyn->d_un.d_val];
				break;

			// Offset of .dynsym (NOT .symtab) section
			case DT_SYMTAB:
				dll->symtab = (Elf32_Sym *)
					&((uint8_t *) ptr)[dyn->d_un.d_val];
				break;

			// Length of each .dynsym entry
			case DT_SYMENT:
				// Only 16-byte symbol table entries are supported.
				if (dyn->d_un.d_val != sizeof(Elf32_Sym)) {
					_sdk_log("invalid DLL symtab entry size %d\n", dyn->d_un.d_val);
					return 0;
				}
				break;

			// MIPS ABI (?) version
			case DT_MIPS_RLD_VERSION:
				// Versions other than 1 are unsupported.
				if (dyn->d_un.d_val != 1) {
					_sdk_log("invalid DLL version %d\n", dyn->d_un.d_val);
					return 0;
				}
				break;

			// DLL/ABI flags
			case DT_MIPS_FLAGS:
				// Shortcut pointers are not supported.
				if (dyn->d_un.d_val & RHF_QUICKSTART) {
					_sdk_log("invalid DLL flags\n");
					return 0;
				}
				break;

			// Number of local (not to resolve) GOT entries
			case DT_MIPS_LOCAL_GOTNO:
				dll->got_local_count = dyn->d_un.d_val;
				break;

			// Base address DLL was compiled for
			case DT_MIPS_BASE_ADDRESS:
				// Base addresses other than zero are not supported. It would
				// be easy enough to support them, but why?
				if (dyn->d_un.d_val) {
					_sdk_log("invalid DLL base address %08x\n", dyn->d_un.d_val);
					return 0;
				}
				break;

			// Number of symbol table entries
			case DT_MIPS_SYMTABNO:
				dll->symbol_count = dyn->d_un.d_val;
				break;

			// Index of first symbol table entry which has a matching GOT entry
			case DT_MIPS_GOTSYM:
				dll->first_got_symbol = dyn->d_un.d_val;
				break;
		}
	}

	dll->got_extern_count = dll->symbol_count - dll->first_got_symbol;
	_sdk_log(
		"%d symbols, %d GOT locals, %d GOT externs\n",
		dll->symbol_count, dll->got_local_count - 2, dll->got_extern_count
	);

	// Relocate the DLL by adding its base address to all pointers in the local
	// section of the GOT except the first two, which are reserved. The first
	// entry in particular is a pointer to the lazy resolver, called by
	// auto-generated stubs when a function is first used. got[1] is normally
	// unused, but here we'll set it to the DLL's metadata struct so we can
	// look that up when resolving functions (see _dl_resolve_wrapper()).
	dll->got[0] = (uint32_t) &_dl_resolve_wrapper;
	dll->got[1] = (uint32_t) dll;

	for (int i = 2; i < dll->got_local_count; i++)
		dll->got[i] += (uint32_t) ptr;

	// Relocate all pointers in the symbol table and populate the global
	// section of the GOT.
	uint32_t *_got = &(dll->got[dll->got_local_count - dll->first_got_symbol]);

	for (int i = 0; i < dll->symbol_count; i++) {
		Elf32_Sym  *sym   = &(dll->symtab[i]);
		const char *_name = &(dll->strtab[sym->st_name]);

		sym->st_value += (uint32_t) ptr;
		//_sdk_log("%08x, %08x [%s]\n", sym->st_value, sym->st_size, _name);

		if (i < dll->first_got_symbol)
			continue;

		// Resolve the GOT entry if the symbol is an imported variable (or a
		// function in non-lazy mode), otherwise relocate the GOT pointer.
		if (
			!(sym->st_shndx) &&
			((ELF32_ST_TYPE(sym->st_info) != STT_FUNC) || (mode & DL_NOW))
		) {
			void *sym_ptr = _dl_resolve_callback(dll, _name);
			if (!sym_ptr)
				return 0;

			_got[i] = (uint32_t) sym_ptr;
		} else {
			_got[i] += (uint32_t) ptr;
		}
	}

	EnterCriticalSection();
	FlushCache();
	ExitCriticalSection();

	// Call the DLL's global constructors. This is the same thing we'd do in
	// _start() for regular executables, but we have to do it outside of the
	// DLL as there's no _start() or even a defined entry point within the
	// DLL itself.
	const uint32_t *ctor_list = DL_GetDLLSymbol(dll, "__CTOR_LIST__");
	if (ctor_list) {
		for (int i = ((int) ctor_list[0]); i >= 1; i--) {
			void (*ctor)(void) = (void (*)(void)) ctor_list[i];
			DL_PRE_CALL(ctor);
			ctor();
		}
	}

	return dll;
}

void DL_DestroyDLL(DLL *dll) {
	if (!dll)
		return;

	if (dll->ptr) {
		// Call the DLL's global destructors.
		const uint32_t *dtor_list = DL_GetDLLSymbol(dll, "__DTOR_LIST__");
		if (dtor_list) {
			for (int i = 0; i < ((int) dtor_list[0]); i++) {
				void (*dtor)(void) = (void (*)(void)) dtor_list[i + 1];
				DL_PRE_CALL(dtor);
				dtor();
			}
		}

		dll->ptr = 0;
	}

	// If the DLL is associated to a buffer, free that buffer.
	if (dll->malloc_ptr) {
		free(dll->malloc_ptr);
		dll->malloc_ptr = 0;
	}
}

void *DL_GetDLLSymbol(const DLL *dll, const char *name) {
	if (!dll)
		return DL_GetMapSymbol(name);
		//return _dl_resolve_callback(0, name);

	uint32_t       nbucket = dll->hash[0];
	uint32_t       nchain  = dll->hash[1];
	const uint32_t *bucket = &(dll->hash[2]);
	const uint32_t *chain  = &(dll->hash[2 + nbucket]);

	// Go through the hash table's chain until the symbol name matches the one
	// provided.
	for (int i = bucket[_elf_hash(name) % nbucket]; i > 0;) {
		if (i >= nchain) {
			_sdk_log("DL_GetDLLSymbol() index out of bounds (%d >= %d)\n", i, nchain);
			return 0;
		}

		Elf32_Sym  *sym   = &(dll->symtab[i]);
		const char *_name = &(dll->strtab[sym->st_name]);

		if (!strcmp(name, _name)) {
			//_sdk_log("DLL lookup [%s = %08x]\n", name, sym->st_value);
			return sym->st_value;
		}

		i = chain[i];
	}

	_sdk_log("DLL lookup [%s not found]\n", name);
	return 0;
}
