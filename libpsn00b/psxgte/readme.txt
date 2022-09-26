PSX GTE library, part of PSn00bSDK
2019 Lameguy64 / Meido-Tek Productions

Licensed under Mozilla Public License

	Open source implementation of the GTE library written mostly in MIPS
assembly. It makes full use of the GTE in complex matrix multiplication
operations. The syntax is intentionally made to closely resemble Sony's syntax
for familiarity and to make porting homebrew made using the official SDK to
PSn00bSDK a little easier.

	Unlike the official GTE libraries using the inline GTE macro functions does
not require running your object file through some stupid tool such as DMPSX.
The GTE macros use the corresponding cop2 opcodes already.


Library developer(s):

	Lameguy64
	
	
Library header(s):

	gtereg.inc
	inline_c.h
	inline_s.inc
	psxgte.h
	
	
Todo list:

	* Alternate RotMatrix() functions with different rotation orders are yet to
	  be implemented.
	* Various high level RotTransPersp style functions not yet implemented.
	
