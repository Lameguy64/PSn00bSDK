/*
 * PSn00bSDK dynamic linker
 * (C) 2021 spicyjpeg - MPL licensed
 *
 * This file is used internally by the dynamic linker to parse the "header"
 * (.dynamic and .dynsym sections) of dynamically-linked libraries built with
 * the provided linker script. Most of it is copied from the standard Unix
 * elf.h header, with the only changes being removed typedefs and #defines
 * converted to enums.
 */

#pragma once

#include <stdint.h>

typedef enum {
	DT_NULL		= 0,		/* Marks end of dynamic section */
	DT_NEEDED	= 1,		/* Name of needed library */
	DT_PLTRELSZ	= 2,		/* Size in bytes of PLT relocs */
	DT_PLTGOT	= 3,		/* Processor defined value */
	DT_HASH		= 4,		/* Address of symbol hash table */
	DT_STRTAB	= 5,		/* Address of string table */
	DT_SYMTAB	= 6,		/* Address of symbol table */
	DT_RELA		= 7,		/* Address of Rela relocs */
	DT_RELASZ	= 8,		/* Total size of Rela relocs */
	DT_RELAENT	= 9,		/* Size of one Rela reloc */
	DT_STRSZ	= 10,		/* Size of string table */
	DT_SYMENT	= 11,		/* Size of one symbol table entry */
	DT_INIT		= 12,		/* Address of init function */
	DT_FINI		= 13,		/* Address of termination function */
	DT_SONAME	= 14,		/* Name of shared object */
	DT_RPATH	= 15,		/* Library search path (deprecated) */
	DT_SYMBOLIC	= 16,		/* Start symbol search here */
	DT_REL		= 17,		/* Address of Rel relocs */
	DT_RELSZ	= 18,		/* Total size of Rel relocs */
	DT_RELENT	= 19,		/* Size of one Rel reloc */
	DT_PLTREL	= 20,		/* Type of reloc in PLT */
	DT_DEBUG	= 21,		/* For debugging; unspecified */
	DT_TEXTREL	= 22,		/* Reloc might modify .text */
	DT_JMPREL	= 23,		/* Address of PLT relocs */
	DT_BIND_NOW	= 24,		/* Process relocations of object */
	DT_INIT_ARRAY	= 25,	/* Array with addresses of init fct */
	DT_FINI_ARRAY	= 26,	/* Array with addresses of fini fct */
	DT_INIT_ARRAYSZ	= 27,	/* Size in bytes of DT_INIT_ARRAY */
	DT_FINI_ARRAYSZ	= 28,	/* Size in bytes of DT_FINI_ARRAY */
	DT_RUNPATH	= 29,		/* Library search path */
	DT_FLAGS	= 30,		/* Flags for the object being loaded */
	DT_ENCODING	= 32,		/* Start of encoded range */
	DT_PREINIT_ARRAY	= 32,	/* Array with addresses of preinit fct*/
	DT_PREINIT_ARRAYSZ	= 33,	/* size in bytes of DT_PREINIT_ARRAY */
	DT_SYMTAB_SHNDX		= 34,	/* Address of SYMTAB_SHNDX section */
	DT_NUM		= 35,			/* Number used */
	DT_LOOS		= 0x6000000d,	/* Start of OS-specific */
	DT_HIOS		= 0x6ffff000,	/* End of OS-specific */
	DT_LOPROC	= 0x70000000,	/* Start of processor-specific */
	DT_HIPROC	= 0x7fffffff,	/* End of processor-specific */

	DT_MIPS_RLD_VERSION		= 0x70000001,
	DT_MIPS_FLAGS			= 0x70000005,
	DT_MIPS_BASE_ADDRESS	= 0x70000006,
	DT_MIPS_LOCAL_GOTNO		= 0x7000000a,
	DT_MIPS_SYMTABNO		= 0x70000011,
	DT_MIPS_UNREFEXTNO		= 0x70000012,
	DT_MIPS_GOTSYM			= 0x70000013,
	DT_MIPS_HIPAGENO		= 0x70000014
} Elf32_d_tag;

typedef enum {
	RHF_NONE		= 0,	/* No flags */
	RHF_QUICKSTART	= 1,	/* Use quickstart */
	RHF_NOTPOT		= 2,	/* Hash size not power of 2 */
	RHF_NO_LIBRARY_REPLACEMENT	= 4 /* Ignore LD_LIBRARY_PATH */
} Elf32_d_MIPS_FLAGS;

typedef struct {
	Elf32_d_tag	d_tag;			/* Dynamic entry type */
	union {
		uint32_t	d_val;		/* Integer value */
		void		*d_ptr;		/* Address value */
	} d_un;
} Elf32_Dyn;

typedef struct {
	uint32_t	st_name;		/* Symbol name (string tbl index) */
	void		*st_value;		/* Symbol value */
	size_t		st_size;		/* Symbol size */
	uint8_t		st_info;		/* Symbol type and binding */
	uint8_t		st_other;		/* Symbol visibility */
	uint16_t	st_shndx;		/* Section index */
} Elf32_Sym;

#define ELF32_ST_BIND(val)			((Elf32_st_bind) (((uint8_t) (val)) >> 4))
#define ELF32_ST_TYPE(val)			((Elf32_st_type) ((val) & 0xf))
#define ELF32_ST_INFO(bind, type)	((((uint8_t) (bind)) << 4) + (((uint8_t) (type)) & 0xf))

typedef enum {
	STB_LOCAL	= 0,		/* Local symbol */
	STB_GLOBAL	= 1,		/* Global symbol */
	STB_WEAK	= 2,		/* Weak symbol */
	STB_NUM		= 3,		/* Number of defined types.  */
	STB_LOOS	= 10,		/* Start of OS-specific */
	STB_GNU_UNIQUE	= 10,	/* Unique symbol.  */
	STB_HIOS	= 12,		/* End of OS-specific */
	STB_LOPROC	= 13,		/* Start of processor-specific */
	STB_HIPROC	= 15		/* End of processor-specific */
} Elf32_st_bind;

typedef enum {
	STT_NOTYPE	= 0,		/* Symbol type is unspecified */
	STT_OBJECT	= 1,		/* Symbol is a data object */
	STT_FUNC	= 2,		/* Symbol is a code object */
	STT_SECTION	= 3,		/* Symbol associated with a section */
	STT_FILE	= 4,		/* Symbol's name is file name */
	STT_COMMON	= 5,		/* Symbol is a common data object */
	STT_TLS		= 6,		/* Symbol is thread-local data object*/
	STT_NUM		= 7,		/* Number of defined types.  */
	STT_LOOS	= 10,		/* Start of OS-specific */
	STT_GNU_IFUNC	= 10,	/* Symbol is indirect code object */
	STT_HIOS	= 12,		/* End of OS-specific */
	STT_LOPROC	= 13,		/* Start of processor-specific */
	STT_HIPROC	= 15		/* End of processor-specific */
} Elf32_st_type;
