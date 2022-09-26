PSX GPU library, part of PSn00bSDK
2019 Lameguy64 / Meido-Tek Productions

Licensed under Mozilla Public License

Open source implementation of the GPU library written entirely in C. Supports
DMA transfers for drawing OTs (with an internal queue so DrawOTag() can be
called even when another OT is being drawn) and transferring image data to and
from VRAM. The syntax is intentionally made to closely resemble Sony's syntax
for familiarity and to make porting homebrew made using the official SDK to
PSn00bSDK a little easier.

Library developer(s):

	Lameguy64 (initial implementation in assembly, debug font API)
	spicyjpeg

Library header(s):

	psxgpu.h
