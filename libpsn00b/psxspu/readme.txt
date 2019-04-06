PSX SPU Library, part of PSn00bSDK
2019 Lameguy64 / Meido-Tek Productions

Licensed under Mozilla Public License

	Open source implementation of the SPU library written mostly in MIPS
assembly. Currently only supports SPU init, uploading sample data using DMA
transfer and basic sample playback but is currently lacking a bunch of
important functions.

	Very work in progress currently.
	

Library developer(s):

	Lameguy64
	
	
Library header(s):

	psxspu.h


Todo list:

	* SPU RAM allocation routines yet to be implemented (heap must only be
	  stored in main RAM and not SPU RAM like in the official SDK).

	* SpuKeyOn() is actually not part of the official library.

	* SPU reverb configuration functions yet to be implemented.


Changelog:

	None so far...
