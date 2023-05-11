/*
 * PSn00bSDK dynamic linker
 * (C) 2021-2022 spicyjpeg - MPL licensed
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <elf.h>

/* Macros */

/**
 * @brief Prepares for a DLL function call.
 *
 * @details Sets the $t9 register to the specified value (which should be a
 * pointer to a DLL function obtained using DL_GetDLLSymbol()). This must be
 * done prior to calling a DLL function from the main executable to ensure the
 * DLL can correctly invoke the symbol resolver if necessary.
 *
 * This macro is not required when calling a DLL function from another DLL, as
 * GCC will generate code to set $t9 appropriately.
 */
#define DL_PRE_CALL(func) \
	__asm__ volatile("move $t9, %0;" :: "r"(func) : "$t9")

/* Structure and enum definitions */

typedef enum _DL_ResolveMode {
	DL_LAZY				= 1, // Resolve functions when they are first called (default)
	DL_NOW				= 2, // Resolve all symbols immediately on load
	DL_FREE_ON_DESTROY	= 4  // Automatically free DLL buffer when closing DLL
} DL_ResolveMode;

// Members of this struct should not be accessed directly in most cases, but
// they are intentionally exposed for easier expandability.
typedef struct _DLL {
	void			*ptr, *malloc_ptr;
	size_t			size;
	const uint32_t	*hash;
	uint32_t		*got;
	Elf32_Sym		*symtab;
	const char		*strtab;
	uint16_t		symbol_count, first_got_symbol;
	uint16_t		got_local_count, got_extern_count;
} DLL;

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates an empty symbol map in memory.
 *
 * @details Initializes the internal symbol hash table to contain at most the
 * given number of symbols. Once this function is called, symbols can be
 * registered using DL_AddMapSymbol() and then looked up using
 * DL_GetMapSymbol(). The default DLL resolver will search the hash table for
 * external symbols required by DLLs.
 *
 * This function is normally not required when loading a map file through
 * DL_ParseSymbolMap(), but it can be used alongside DL_AddMapSymbol() to
 * implement a custom symbol map parser.
 *
 * @param num_entries
 * @return 0 or -1 in case of error
 *
 * @see DL_AddMapSymbol(), DL_GetMapSymbol()
 */
int DL_InitSymbolMap(int num_entries);

/**
 * @brief Destroys the currently loaded symbol map.
 *
 * @details Frees the internal hash table allocated by DL_InitSymbolMap() or
 * DL_ParseSymbolMap(), containing the currently loaded symbol map. Freeing the
 * table manually before loading a new symbol map is normally unnecessary as it
 * is done automatically, however this function can be useful to recover heap
 * space once the map is no longer needed.
 */
void DL_UnloadSymbolMap(void);

/**
 * @brief Adds a symbol to the currently loaded symbol map.
 *
 * @details Registers a new symbol (function or variable) with the given name
 * and address, and adds it to the internal hash table. The symbol can then be
 * looked up using DL_GetMapSymbol(). The default DLL resolver will search the
 * hash table for external symbols required by DLLs.
 *
 * This function shall only be called after DL_InitSymbolMap() or
 * DL_ParseSymbolMap() is called.
 *
 * @param name
 * @param ptr
 *
 * @see DL_GetMapSymbol()
 */
void DL_AddMapSymbol(const char *name, void *ptr);

/**
 * @brief Creates a symbol map in memory from a map file in text format.
 *
 * @details Initializes the internal symbol hash table, then parses entries
 * from the provided string buffer (which may or may not be null-terminated)
 * and adds each one to the table. The string buffer won't be further
 * referenced and can be safely deallocated after parsing. Returns the number 
 * of entries successfully parsed.
 *
 * The string buffer shall contain one or more lines, each of which must follow
 * this format:
 *
 *     <SYMBOL_NAME> <T|R|D|B> <HEX_ADDRESS> <HEX_SIZE> [...]
 *
 * The "nm" tool included in the GCC toolchain can be used to generate a map
 * file in the appropriate format after building the executable:
 *
 *     mipsel-none-elf-nm -f posix -l -n executable.elf >executable.map
 *
 * @param ptr
 * @param size
 * @return Number of entries parsed, -1 in case of failure
 *
 * @see DL_UnloadSymbolMap(), DL_GetMapSymbol()
 */
int DL_ParseSymbolMap(const char *ptr, size_t size);

/**
 * @brief Gets a pointer to a symbol in the currently loaded map by its name.
 *
 * @details Queries the currently loaded symbol map for the symbol with the
 * given name and returns a pointer to it, which can then be used to directly
 * access the symbol. If the symbol can't be found, a null pointer is returned.
 *
 * @param name
 * @return NULL or pointer to symbol
 */
void *DL_GetMapSymbol(const char *name);

/**
 * @brief Sets a custom handler for resolving symbols in DLLs.
 *
 * @details Sets a custom function to be called for resolving symbols in DLLs.
 * The function will be given a pointer to the current DLL and the unresolved
 * symbol's name, and should return the address of the symbol in the executable
 * (the dynamic linker will lock up if it returns null). Passing null instead
 * of a function resets the default behavior of calling DL_GetMapSymbol() to
 * find the symbol in the currently loaded symbol map.
 *
 * @param callback NULL or pointer to callback function
 * @return Previously set callback or NULL
 */
void *DL_SetResolveCallback(void *(*callback)(DLL *, const char *));

/**
 * @brief Initializes a DLL structure.
 *
 * @details Initializes a buffer holding the contents of a dynamically-loaded
 * library file (compiled with the dll.ld linker script and converted to a raw
 * binary) *in-place*. Metadata is written to the provided DLL struct but,
 * unlike DL_ParseSymbolMap(), the DLL's actual code, data and tables are
 * referenced directly from the provided buffer. The buffer must not be moved
 * or deallocated, at least not before calling DL_DestroyDLL() on the DLL
 * struct returned by this function.
 *
 * The third argument specifies when symbols in the DLL should be resolved.
 * Setting it to DL_LAZY defers resolution of undefined functions to when they
 * are first called, while DL_NOW forces all symbols to be resolved
 * immediately. Either mode can be OR'd with DL_FREE_ON_DESTROY to
 * automatically deallocate the provided buffer when DL_DestroyDLL() is called.
 *
 * If a custom resolver has been set via DL_SetResolveCallback(), it will be
 * called for each symbol to resolve.
 *
 * @param dll
 * @param ptr
 * @param size
 * @param mode DL_LAZY or DL_NOW, optionally with DL_FREE_ON_DESTROY
 * @return Pointer to DLL structure or NULL in case of failure
 *
 * @see DL_DestroyDLL(), DL_GetDLLSymbol()
 */
DLL *DL_CreateDLL(DLL *dll, void *ptr, size_t size, DL_ResolveMode mode);

/**
 * @brief Destroys a DLL structure.
 *
 * @details Destroys a loaded DLL by calling its global destructors. If the DLL
 * was initialized with the DL_FREE_ON_DESTROY flag, the buffer associated with
 * the DLL is also deallocated. Note that the DLL structure itself is *not*
 * deallocated.
 *
 * @param dll
 */
void DL_DestroyDLL(DLL *dll);

/**
 * @brief Gets a pointer to a symbol in a DLL by its name.
 *
 * @details Returns a pointer to the DLL symbol with the given name, or null if
 * it can't be found. If a null pointer is passed as first argument, the
 * executable itself is searched instead using the symbol map (behaving
 * identically to DL_GetMapSymbol()).
 *
 * @param dll Pointer to DLL structure or NULL
 * @param name
 * @return NULL or pointer to symbol (any type)
 */
void *DL_GetDLLSymbol(const DLL *dll, const char *name);

#ifdef __cplusplus
}
#endif
