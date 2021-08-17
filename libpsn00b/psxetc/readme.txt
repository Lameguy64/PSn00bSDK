PSX Misc library, part of PSn00bSDK
2021 Lameguy64 / Meido-Tek Productions

Licensed under Mozilla Public License

Open source implementation of the ETC library. Currently provides the interrupt
and DMA callback dispatchers (used by other libraries) as well as the DL_* and
dl* functions for dynamic library loading (original, not present in the official
SDK but similar to the standard dlopen() API).

Library developer(s):

	Lameguy64
	spicyjpeg

Library header(s):

	psxetc.h
	dlfcn.h
	elf.h (used internally)
