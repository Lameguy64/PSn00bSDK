LibPSn00b, PSn00bSDK software libraries
2019 Lameguy64 / Meido-Tek Productions

Licensed under Mozilla Public License


LibPSn00b is a collection of libraries which make up the core backbone of
PSn00bSDK as it provides an essential software framework for developing
homebrew software for the PSX platform. Great majority of the libraries
are been written from scratch written in a mix of C and hand optimized
assembly language for best performance.

These libraries can only be compiled with a build of GCC that supports mipsel
as one of the supported targets, naturally as these libraries are intended
for the PSX hardware which uses a MIPS R3000 CPU supported by mipsel.
Compiler version shouldn't matter much and should build fine in 9.1.0,
though 7.4.0 is the most recommended version to use as it is the version
currently used for the majority of the development and testing of LibPSn00b.


Brief summary of libraries:

	libc	- Standard C library. Contains only a small subset of the full
			  standard C library such as string and memory manipulation
			  functions. Should include libgcc for special compiler specific
			  dependencies and prevents libc related linker hell.
			  
	psxgpu	- GPU library for video and graphics control. Also includes
			  interrupt handling for various library subsystems.
	
	psxgte	- GTE library for GTE accelerated vector transformations necessary
			  for high performance 3D graphics.
			  
	psxapi	- Provides access to various BIOS functions in the PSX BIOS.
			  
	psxetc	- Provides some miscellaneous features such as debug font.

	psxspu	- SPU library. Basic capabilities such as hardware init,
			  sample upload using DMA and playing samples are supported.
			  Lacks support for reverb and various sequenced music related
			  features.
			  
	psxcd	- CD-ROM support library. Provides greater CD-ROM functionality
			  than what the BIOS subsystem can provide.
			   
	Each library has its own readme file that contains a todo list, credits
	and some additional details of the library. Changelog of all libraries are
	compiled in the changelog.txt file.


Compiling:

	Compiling the LibPSn00b libraries requires GCC and binutils targeting
	mipsel-unknown-elf built with libgcc. The path to those binaries must be
	specified in your PATH environment variable and the binaries must be
	prefixed by the target architecture (ie. mipsel-unknown-elf-gcc).

	Simply run the Makefile in this directory with make (or gmake if you're
	working in a BSD environment). The Makefile should parse though each
	library directory and run the makefile in it.


Documentation:

	Documentation of all the libraries are found in libn00bref.odf and it
	features the same formatting as the official library documents. It is
	recommended to export it as a PDF document for easier viewing.


Contributing:

	Contributions are open for this project. But it is recommended to at
	least follow the following rules when contributing to prevent headaches.

	* If you add new functions to libpsn00b make sure you document them in
	  libn00bref.odf. Documentation using Doxygen is not recommended.
	  
	* Library functions can either be written in C or assembly language
	  whichever you prefer. Though it is not entirely mandatory but
	  recommended to write functions that directly interact with hardware
	  registers in assembly.
	  
	* Functions must work on both emulators and real hardware.
	
	* Don't forget to put your name in the readme file of the library you've
	  made a contribution on and details of what you've changed in the
	  changelog.txt file. Annoyingmous is not tolerated in this project.
	  
	* New functions that are not originally in the official SDK must be marked
	  as original code in both the library headers and the libn00bref document
	  to help differentiate from original functions and new additions.

	
LibPSn00b to-do list:

	Because the PSn00bSDK project still a work in progress, a number of
	essential libraries are considered but not yet developed. The following
	lists a number of essential libraries not yet developed.

	psxpad	 - Pad/tap/gun library for better controller support. May support
			   PS2 controllers natively. Should provide functions for directly
			   controlling the controller/memory card interfaces to cater to
			   development of custom peripherals.

	psxmcrd  - Better and faster memory card library that works alongside the
			   psxpad library.
			   
	psxpress - MDEC and data decompression library. May use DEFLATE for
			   compressing MDEC data instead of Huffman as used in the
			   official libraries, which may yield better compression and
			   higher quality FMVs.