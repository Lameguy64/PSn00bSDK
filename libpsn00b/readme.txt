LibPSn00b, PSn00bSDK software libraries
2019 Lameguy64 / Meido-Tek Productions

Licensed under Mozilla Public License

LibPSn00b make up the majority of PSn00bSDK as it provides the functions
necessary for developing software for the PSX. Most libraries, mainly the
GPU and GTE libraries are written mostly in assembly language.


Brief library overview:

	libc	- Standard C library. Contains only a small subset of the full
			  standard C library, mostly string and memory manipulation
			  functions.
			  
	psxgpu	- GPU library for video and graphics control. Most of the
			  important functions are implemented.
	
	psxgte	- GTE library for GTE accelerated vector transformations. Most
			  important functions are implemented.
			  
	psxapi	- Provides access to most of the BIOS functions calls of the
			  PSX BIOS.
			  
	psxetc	- Provides some misc functions such as debug font.

	psxspu	- SPU library. Basic functions such as hardware init, uploading
			  data to SPU RAM via DMA transfer and playing sound playback
			  are fully working but is currently lacking a number of
			  important functions, especially reverb.
			   
	Each library has its own readme file that contains its changelog, todo
	list, credits and some additional details of the library.


Compiling:

	Compiling the LibPSn00b libraries requires GCC and binutils targetting
	mipsel-unknown-elf built with libgcc. The path to those binaries must be
	specified in your PATH environment variable and the binaries must be
	prefixed by the target architecture (ie. mipsel-unknown-elf-gcc).

	Simply run the Makefile in this directory using make (or gmake if you're
	in a BSD environment). The Makefile should parse though each library
	directory and run the makefile in it.


Documentation:

	Documentation of all the libraries are found in libn00bref.odf and it features
	the same formatting as the official library documents. The document can be
	exported in PDF format for easier viewing with a web browser or PDF viewer.


Contributing:

	Contributions are open for this project. Just obey the following rules
	when making a contribution:

	* If you add new functions to libpsn00b make sure you document them in
	  libn00bref.odf. Documentation using Doxygen is discouraged.
	* This project desires functions that interact with hardware registers to
	  be written in assembly language. If you decide to write your functions
	  that interact with hardware registers in C make sure you define your
	  register pointers as volatile and they must work properly when compiled
	  with -O2 optimization.
	* Functions must work flawlessly on both emulators and real hardware.
	* Don't forget to put your user name in the readme file of the library
	  you've made a contribution on, and details of what you've changed in the
	  changelog.
	* New functions that are not originally in the official SDK must be marked
	  as original code in both the library headers and the libn00bref document.

	
Library to-do list:

	Since the PSn00bSDK project still a work in progress, essential libraries
	for CD and controllers support are not yet created but are high priority.

	libc	 - Yet to include a complete C standard function
	
	psxcd	 - CD library. Not much progress as getting the CD controller to
			   cooperate has been one heck of a force to be reckoned with.
			   Gave up with my repeated attempts with no success. Absolutely
			   huge props to anyone who can figure out how to use the CD
			   controller properly! -Lameguy64

	psxpad	 - Pad/tap/gun library. No work has been done on it currently.
			   Supporting the Konami Justifier can be ignored as the way how
			   that lightgun works is pretty crude and likely going to be
			   awful to implement. Namco's Guncon is a lot simpler to
			   implement.

	psxpress - MDEC library. No work has been done on it currently. libpress
			   from the official libraries also contains functions for
			   encoding SPU compatible ADPCM data from raw PCM samples.

	Before implementing CD and controller support a better interrupt handling
	scheme must be implemented as it would save a lot of trouble. Details
	regarding this interrupt handler implementation can be found in psxgpu's
	readme document.