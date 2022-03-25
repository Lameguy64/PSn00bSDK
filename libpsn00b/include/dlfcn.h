/*
 * PSn00bSDK dynamic linker
 * (C) 2021-2022 spicyjpeg - MPL licensed
 */

#ifndef __DLFCN_H
#define __DLFCN_H 

#include <stdint.h>
#include <elf.h>

/* Helper macro for setting $t9 before calling a function */

#define DL_PRE_CALL(func) { \
	__asm__ volatile("move $t9, %0;" :: "r"(func) : "$t9"); \
}

/* Types */

#define RTLD_DEFAULT ((DLL *) 0)

typedef enum _DL_Error {
	RTLD_E_NONE			=  0, // No error
	RTLD_E_FILE_OPEN	=  1, // Unable to find or open file
	RTLD_E_FILE_ALLOC	=  2, // Unable to allocate buffer to load file into
	RTLD_E_FILE_READ	=  3, // Failed to read file
	RTLD_E_NO_MAP		=  4, // No symbol map has been loaded yet
	RTLD_E_MAP_ALLOC	=  5, // Unable to allocate symbol map structures
	RTLD_E_NO_SYMBOLS	=  6, // No symbols found in symbol map
	RTLD_E_DLL_NULL		=  7, // Unable to initialize DLL from null pointer
	RTLD_E_DLL_ALLOC	=  8, // Unable to allocate DLL metadata structures
	RTLD_E_DLL_FORMAT	=  9, // Unsupported DLL type or format
	RTLD_E_MAP_SYMBOL	= 10, // Symbol not found in symbol map
	RTLD_E_DLL_SYMBOL	= 11, // Symbol not found in DLL
	RTLD_E_HASH_LOOKUP	= 12  // Hash table lookup failed due to internal error
} DL_Error;

typedef enum _DL_ResolveMode {
	RTLD_LAZY				= 1, // Resolve functions when they are first called (default)
	RTLD_NOW				= 2, // Resolve all symbols immediately on load
	RTLD_FREE_ON_DESTROY	= 4  // Automatically free DLL buffer when closing DLL
} DL_ResolveMode;

// Members of this struct should not be accessed directly in most cases, but
// they are intentionally exposed for easier expandability.
typedef struct _DLL {
	void			*ptr;
	void			*malloc_ptr;
	size_t			size;
	const uint32_t	*hash;
	uint32_t		*got;
	Elf32_Sym		*symtab;
	const char		*strtab;
	uint16_t		symbol_count;
	uint16_t		got_length;
} DLL;

/* API */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reads the symbol table from the provided string buffer (which may or
 * may not be null-terminated), parses it and stores the parsed entries into a
 * private hash table; the buffer won't be further referenced and can be safely
 * deallocated after parsing. Returns the number of entries successfully parsed
 * or -1 if an error occurred.
 *
 * This function expects the string buffer to contain one more lines, each of
 * which must follow this format:
 *
 *   <SYMBOL_NAME> <T|R|D|B> <HEX_ADDRESS> <HEX_SIZE> [DEBUG_INFO...]
 *
 * The "nm" tool included in the GCC toolchain can be used to generate a map
 * file in the appropriate format after building the executable, by using this
 * command:
 *
 *   mipsel-none-elf-nm -f posix -l -n executable.elf >executable.map
 *
 * @param ptr
 * @param size
 * @return -1 or number of entries parsed
 */
int32_t DL_ParseSymbolMap(const char *ptr, size_t size);

/**
 * @brief File wrapper around DL_ParseSymbolMap(). Allocates a temporary buffer
 * then loads the specified map file into it (using BIOS APIs) and calls
 * DL_ParseSymbolMap() to parse it. The buffer is deallocated immediately after
 * parsing.
 *
 * @param filename Must always contain device name, e.g. "cdrom:MODULE.DLL;1"
 * @return -1 or number of entries parsed
 */
//int32_t DL_LoadSymbolMapFromFile(const char *filename);

/**
 * @brief Frees internal buffers containing the currently loaded symbol map.
 * This is automatically done before loading a new symbol map so there is no
 * need to call this function in most cases, however it can still be useful to
 * free up space on the heap once the symbol map is no longer needed.
 */
void DL_UnloadSymbolMap(void);

/**
 * @brief Queries the currently loaded symbol map for the symbol with the given
 * name and returns a pointer to it, which can then be used to directly access
 * the symbol. If the symbol can't be found, null is returned instead.
 *
 * @param name
 * @return NULL or pointer to symbol (any type)
 */
void *DL_GetSymbolByName(const char *name);

/**
 * @brief Sets a custom function to be called for resolving symbols in DLLs.
 * The function will be given a pointer to the current DLL and the unresolved
 * symbol's name, and should return the address of the symbol in the executable
 * (the dynamic linker will lock up if it returns null). Passing null instead
 * of a function resets the default behavior of calling DL_GetSymbolByName() to
 * find the symbol in the currently loaded symbol map.
 * 
 * @param callback NULL or pointer to callback function
 */
void DL_SetResolveCallback(void *(*callback)(DLL *, const char *));

/**
 * @brief Initializes a buffer holding the contents of a dynamically-loaded
 * library file (compiled with the dll.ld linker script and converted to a raw
 * binary) *in-place*. A new DLL struct is allocated to store metadata but,
 * unlike DL_ParseSymbolMap(), the DLL's actual code, data and tables are
 * referenced directly from the provided buffer. The buffer must not be moved
 * or deallocated, at least not before calling DL_DestroyDLL() on the DLL
 * struct returned by this function.
 *
 * The third argument specifies when symbols in the DLL should be resolved.
 * Setting it to RTLD_LAZY defers resolution of undefined functions to when
 * they are first called, while RTLD_NOW forces all symbols to be resolved
 * immediately. If a custom resolver has been set via DL_SetResolveCallback(),
 * it will be called for each symbol to resolve.
 *
 * @param ptr
 * @param size
 * @param mode RTLD_LAZY or RTLD_NOW
 * @return NULL or pointer to a new DLL struct
 */
DLL *DL_CreateDLL(void *ptr, size_t size, DL_ResolveMode mode);

/**
 * @brief File wrapper around dlinit(). Allocates a new buffer, loads the
 * specified file into it (using BIOS APIs) and calls dlinit() on that. When
 * calling dlclose() on a DLL loaded from a file, the file buffer is
 * automatically destroyed.
 *
 * @param filename Must always contain device name, e.g. "cdrom:MODULE.DLL;1"
 * @param mode RTLD_LAZY or RTLD_NOW + optionally RTLD_FREE_ON_DESTROY
 * @return NULL or pointer to a new DLL struct
 */
//DLL *DL_LoadDLLFromFile(const char *filename, DL_ResolveMode mode);

/**
 * @brief Destroys a loaded DLL by calling its global destructors and freeing
 * the buffer it's loaded in. Any pointer passed to DL_DestroyDLL() should no
 * longer be used after the call. If the DLL was initialized in-place using
 * DL_CreateDLL(), DL_DestroyDLL() will only free the buffer initially passed
 * to DL_CreateDLL() if RTLD_FREE_ON_DESTROY was used.
 *
 * @param dll
 */
void DL_DestroyDLL(DLL *dll);

/**
 * @brief Returns a pointer to the DLL symbol with the given name, or null if
 * it can't be found. If null or RTLD_DEFAULT is passed as first argument, the
 * executable itself is searched instead using the symbol map (behaving the
 * same as DL_GetSymbolByName()).
 *
 * @param dll DLL struct or RTLD_DEFAULT
 * @param name
 * @return NULL or pointer to symbol (any type)
 */
void *DL_GetDLLSymbol(const DLL *dll, const char *name);

/**
 * @brief Returns a code describing the last error that occurred, or DL_E_NONE
 * if no error has occurred since the last call to dlerror() (i.e. calling this
 * also resets the internal error flags).
 *
 * @return NULL or member of DL_Error enum
 */
DL_Error DL_GetLastError(void);

/* POSIX "compatibility" macros */

#define dlinit(ptr, size, mode)	DL_CreateDLL(ptr, size, mode)
//#define dlopen(filename, mode)	DL_LoadDLLFromFile(filename, mode)
#define dlsym(dll, name)		DL_GetDLLSymbol(dll, name)
#define dlclose(dll)			DL_DestroyDLL(dll)
#define dlerror()				DL_GetLastError()

#ifdef __cplusplus
}
#endif

#endif
