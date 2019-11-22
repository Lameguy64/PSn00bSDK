PSX GPU library, part of PSn00bSDK
2019 Lameguy64 / Meido-Tek Productions

Licensed under Mozilla Public License

	Open source implementation of the GPU library written mostly in MIPS
assembly. Supports DMA transfers for ordering table draw and transferring
image data to and from VRAM. The syntax is intentionally made to closely
resemble Sony's syntax for familiarity and to make porting homebrew made
using the official SDK to PSn00bSDK a little easier.


Library developer(s):

	Lameguy64


Library header(s):

	hwregs_a.h (GNU assembler port defs)
	psxgpu.h


Todo list:

	* ClearOTag() function (non reverse version of ClearOTagR()) yet to be
	  implemented (but should be trivial).
