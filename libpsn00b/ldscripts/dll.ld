/*
 * PSn00bSDK linker script for dynamically-loaded libraries
 * (C) 2021 spicyjpeg - MPL licensed
 *
 * This script is similar to the one for executables. The main differences are
 * the header at the beginning of the file, the .sdata/.sbss sections being
 * replaced by the global offset table (GOT) and .bss being merged with .data.
 */

/*OUTPUT_FORMAT(elf32-littlemips)
ENTRY(_start)
STARTUP(start.o)*/

MEMORY {
	/* Code is position-independent, so we just set zero as address */
	RELOC_RAM (rwx) : ORIGIN = 0x00000000, LENGTH = 1M
}

SECTIONS {
	/*
	 * DLL "header", containing the following sections:
	 *
	 * .dynamic: key-value pairs describing various section offsets, flags and
	 *           other stuff
	 * .dynsym:  dynamic symbol table, listing all functions to relocate as well
	 *           as exported symbols
	 * .hash:    pre-generated hash table to quickly look up symbol table
	 *           entries by their names
	 * .dynstr:  string blob referenced by symbol table entries
	 */

	.dynamic : { *(.dynamic) } > RELOC_RAM
	.dynsym  : { *(.dynsym) }  > RELOC_RAM
	.hash    : { *(.hash) }    > RELOC_RAM
	.dynstr  : { *(.dynstr) }  > RELOC_RAM

	/* Text section, i.e. code and constants */

	.text : ALIGN(16) {
		__text_start = .;

		*(.text .text.* .gnu.linkonce.t.*)
		*(.plt .MIPS.stubs)
	} > RELOC_RAM
	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	} > RELOC_RAM

	/* Global constructor and destructor arrays (length-prefixed) */

	/*
	 * https://sourceware.org/binutils/docs/ld/Output-Section-Keywords.html
	 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=46770
	 */
	.ctors : ALIGN(16) {
		__CTOR_LIST__ = .;

		LONG(((__CTOR_END__ - __CTOR_LIST__) / 4) - 2)
		KEEP(*(SORT(.ctors.*)))
		KEEP(*(.ctors))
		LONG(0)

		__CTOR_END__ = .;
	} > RELOC_RAM
	.dtors : ALIGN(16) {
		__DTOR_LIST__ = .;

		LONG(((__DTOR_END__ - __DTOR_LIST__) / 4) - 2)
		KEEP(*(SORT(.dtors.*)))
		KEEP(*(.dtors))
		LONG(0)

		__DTOR_END__ = .;
	} > RELOC_RAM

	/* Data and BSS sections, i.e. variables */

	.data : {
		*(.data .data.* .gnu.linkonce.d.*)

		/*
		 * Merge the .bss section into the .data section, so uninitialized
		 * variables are treated as if they were initialized and preallocated.
		 * This makes DLLs unnecessarily larger (BSS values shouldn't be stored
		 * as they are always zero) but greatly simplifies the dynamic linker,
		 * as we don't have to worry about managing .bss separately from the
		 * main DLL blob.
		 */
		. = ALIGN((. != 0) ? 4 : 1);
		__bss_start = .;

		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)

		. = ALIGN((. != 0) ? 4 : 1);
	} > RELOC_RAM

	/*
	 * Set _gp to point to the beginning of the GOT plus 0x7ff0, so anything
	 * within the GOT can be accessed using the $gp register as base plus a
	 * signed 16-bit immediate. Note that $gp is set to _gp in all exported
	 * functions (GCC adds the relevant code automatically).
	 */
	HIDDEN(_gp = ALIGN(16) + 0x7ff0);

	.got : {
		*(.got)
	} > RELOC_RAM

	_end = .;

	/* Dummy section */

	.dummy (NOLOAD) : {
		KEEP(*(.dummy))
	} > RELOC_RAM

	/* Remove anything flagged as link-time optimized */

	/DISCARD/ : {
		*(.gnu.lto_*)
	}
}
