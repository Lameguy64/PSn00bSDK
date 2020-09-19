LibPSn00b, PSn00bSDK software libraries
2019 - 2020 Lameguy64 / Meido-Tek Productions

Licensed under Mozilla Public License


LibPSn00b is a collection of libraries that make up the core backbone of
PSn00bSDK, as it provides a higher level software interface for developing
software for the PS1 platform. The majority of the libraries are been written
from scratch in a mix of C and hand optimized assembly language for optimal
software performance.

These libraries can only be compiled using a build of GCC that targets
mipsel-unknown-elf. Compiler version shouldn't matter much and it should work
fine with GCC 9.1.0, though 7.4.0 is the recommended version as that is what
LibPSn00b is most tested most on.

	
Brief summary of libraries:

	libc	- Standard C library. Covers only a small subset of the full
			  standard C library such as basic string and memory manipulation
			  functions. Should include libgcc to avoid libc/libgcc linker
			  hell (endless cross referencing).
			  
	psxgpu	- GPU library for video, graphics control, and interrupt service
			  subsystem that other libraries that uses interrupts depend on.
	
	psxgte	- GTE library for hardware accelerated vector transformations
			  that are integral for high performance 3D graphics on the PS1
			  (it is a Geometry Transformation Engine, NOT Transfer Engine).
			  
	psxapi	- Provides function calls for using functions provided by the PS1
			  BIOS.
			  
	psxetc	- Provides some miscellaneous features such as debug font.

	psxspu	- SPU library (work in progress). Currently supports hardware
			  init, sample data upload via DMA and playing sound samples.
			  Lacks support for reverb and a sequenced music subsystem.
			  
	psxcd	- CD-ROM library for loading files, parsing directories
			  (PSn00bSDK addition), CD Audio/XA playback with provisions for
			  data streaming. Also supports multi-session discs (must be
			  selected manually).
			   
	Each library has its own readme file that contains a todo list, credits
	and some additional details of the library. Changes of all the libraries
	must be covered in the changelog.txt file.


Compiling:

	To compile the LibPSn00b libraries, you will first need a working GCC
	toolchain which you can either build yourself as described in the
	toolchain.txt file (preferred on Linux/BSDs) or download a precompiled
	toolchain from the PSn00bSDK page of Lameguy64's website:
	http://lameguy64.net/?page=psn00bsdk
	
	Once the toolchain is set up, you may want to set the following
	environment variables if they're required for your particular setup:
	
	* PSN00BSDK_TC specifies the base directory of the GCC toolchain to
	compile the libraries with. If GCC_BASE is defined in the environment it
	will use the path of that variable instead. If neither variable is
	defined the makefiles will assume the toolchain binaries are accessible
	from your PATH variable and the GCC toolchain directory is at
	C:\mipsel-unknown-elf in Windows or /usr/local/mipsel-unknown-elf
	in Linux/BSD.
	
	* GCC_VERSION specifies the version of the toolchain. If not specified
	the makefiles will default to 7.4.0. This is only used for building the
	libc library which needs to inherit the libgcc library from the
	toolchain directories to avoid the aforementioned linker hell, and the
	path to that library file includes a directory named after the version
	number.
	
	* PSN00BSDK_LIBS specifies the absolute directory path you wish the
	compiled libraries will be copied to. If not defined the compiled
	libraries will simply be copied to the parent directory of the libraries
	(libpsn00b).
	
	Once the environment variables are set, building the libraries should just
	be a matter of simply running 'make all install' within the LibPSn00b
	directory and it should run through all the libraries. You may specify a
	-j parameter to run the jobs in parallel on a mult-processor/core/thread
	system.


Documentation:

	LibPSn00b documentation is all covered in the LibPSn00b Reference.odf
	document  complete with the same document formatting as found in official
	library documents. It may be wise to export the document as a PDF
	document for easier viewing.


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
	  changelog.txt file. Updates by 'annoyingmous' are not tolerated in this
	  project.
	  
	* New functions that are not originally in the official SDK must be marked
	  as original code in both the library headers and the libn00bref document
	  to help differentiate from original functions and new additions.

	
LibPSn00b Library to-do list:

	Because the PSn00bSDK project still a work in progress, a number of
	libraries have been considered but not yet developed. The following
	lists a number of libraries that are not yet in development.

	psxpad	 - Pad/tap/gun library for better controller support. May support
			   PS2 controllers natively. Should provide functions for directly
			   controlling the controller/memory card interfaces to cater to
			   the development of custom peripherals.

	psxmcrd  - Better and faster memory card library with provisions for
			   low-level card access. The psxpad library would provide
			   communication routines for the card library.
			   
	psxpress - MDEC and data decompression library. May use DEFLATE or LZ77
			   for compressing MDEC data instead of Huffman as used in the
			   official libraries. It may yield better compression which may
			   potentially result in higher quality FMVs.