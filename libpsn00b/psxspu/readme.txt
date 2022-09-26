PSX SPU Library, part of PSn00bSDK
2019 Lameguy64 / Meido-Tek Productions

Licensed under Mozilla Public License

Open source implementation of the SPU library written entirely in C. Currently
only supports SPU initialization, reading/writing SPU RAM using DMA and basic
sample playback. Most of the official API is not going to be implemented as the
vast majority of it is just inefficient wrappers around accessing SPU registers
directly, which can be done already using the macros defined in hwregs_c.h.

Library developer(s):

	Lameguy64 (initial implementation in assembly)
	spicyjpeg

Library header(s):

	psxspu.h

Todo list:

	* SPU RAM allocation routines yet to be implemented (heap must only be
	  stored in main RAM and not SPU RAM like in the official SDK).

	* SPU reverb configuration functions yet to be implemented.
