/*
 * PSn00bSDK linker script for executables
 * (C) 2021 spicyjpeg - MPL licensed
 *
 * GP-relative addressing (i.e. placing small variables in a 64 KB block and
 * using $gp to reference them) is fully supported; the block is made up of
 * sections .sdata and .sbss. Note that GP-relative addressing is not
 * compatible with dynamic linking, as DLLs require GP to be unused.
 *
 * This linker script assumes main RAM is 8 MB to allow executables meant for
 * devkits or arcade systems to be built, however the executable conversion
 * tool (elf2x) will throw a warning if the size exceeds 2 MB.
 */

/*OUTPUT_FORMAT(elf32-littlemips)*/
ENTRY(_start)
/*STARTUP(start.o)*/

MEMORY {
	/* Mapped into KSEG0 */
	KERNEL_RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 0x010000
	APP_RAM    (rwx) : ORIGIN = 0x80010000, LENGTH = 0x7f0000
}

SECTIONS {
	/* Text section, i.e. code and constants */

	.text : {
		__text_start = .;

		*(.text .text.* .gnu.linkonce.t.*)
		*(.plt .MIPS.stubs)
	} > APP_RAM
	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	} > APP_RAM

	/* Global constructor and destructor arrays (length-prefixed) */

	/*
	 * TODO: replace this crap with .init_array and .fini_array, which are the
	 * "modern" way of doing it without reversed arrays and weird length
	 * prefixes. That would require even more patching though.
	 *
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
	} > APP_RAM
	.dtors : ALIGN(16) {
		__DTOR_LIST__ = .;

		LONG(((__DTOR_END__ - __DTOR_LIST__) / 4) - 2)
		KEEP(*(SORT(.dtors.*)))
		KEEP(*(.dtors))
		LONG(0)

		__DTOR_END__ = .;
	} > APP_RAM

	/* Data sections, i.e. variables with default values */

	.data : {
		*(.data .data.* .gnu.linkonce.d.*)
	} > APP_RAM

	/*
	 * Set _gp to point to the beginning of .sdata plus 0x7ff0, so anything
	 * within .sdata (and .sbss) can be accessed using the $gp register as
	 * base plus a signed 16-bit immediate. Note that $gp is set to _gp on
	 * boot by _start() in the SDK.
	 */
	HIDDEN(_gp = ALIGN(16) + 0x7ff0);

	.sdata : {
		*(.sdata .sdata.* .gnu.linkonce.s.*)
	} > APP_RAM

	/* BSS sections, i.e. uninitialized variables */

	/*
	 * Align all BSS sections to 4 bytes to ensure _start() doesn't perform
	 * unaligned memory accesses when clearing them.
	 */
	. = ALIGN((. != 0) ? 4 : 1);
	__bss_start = .;

	.sbss (NOLOAD) : {
		*(.sbss .sbss.* .gnu.linkonce.sb.*)
		*(.scommon)
	} > APP_RAM
	.bss (NOLOAD) : {
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
	} > APP_RAM

	. = ALIGN((. != 0) ? 4 : 1);
	_end = .;

	/* Dummy section */

	.dummy (NOLOAD) : {
		KEEP(*(.dummy))
	} > APP_RAM

	/* Remove anything flagged as link-time optimized */

	/DISCARD/ : {
		*(.gnu.lto_*)
	}
}
