PSX GPU library, part of PSn00bSDK
2019 Lameguy64 / Meido-Tek Productions

Licensed under Mozilla Public License

	Open source implementation of the GPU library written mostly in MIPS
assembly. Supports DMA transfers for ordering table draw and transferring
image data to VRAM. The syntax is intentionally made to closely resemble
Sony's syntax for familiarity and to make porting homebrew made using the
official SDK to PSn00bSDK a little easier.


Library developer(s):

	Lameguy64


Library header(s):

	hwregs_a.h (GNU assembler port defs)
	psxgpu.h


Todo list:

	* VSync() and DrawSync() functions lack alternate operating modes such as
	  getting number of vsyncs elapsed and waiting until a specified number of
	  vsyncs have passed.

	* (old) VSync interrupt handler should be hooked using BIOS function
	  SetCustomExitFromException() like the official GPU library instead of
	  hooking an event handler, but said hook never seems to work. Perhaps
	  something in the kernel area needs to be patched/set or some event/IRQ
	  handler needs to be removed as such handlers can skip the custom
	  exception exit entirely.
	  
	  It also appears that all interrupt handling in the official libraries
	  are done through the GPU library. This would also explain why the
	  official documentation tells you to always call ResetGraph() at the
	  very beginning of your programs.

	* ClearOTag() function (non reverse version of ClearOTagR()) yet to be
	  implemented.
	  
	* StoreImage() equivalent yet to be implemented.


Changelog:

	05-23-2019 by Lameguy64:
	
	* Got custom exit handler set using SetCustomExitFromException() (BIOS
	  function B(19h)) working. Currently used to acknowledge VSync IRQ but
	  actual VSync handling is still done with events and needs to be
	  transferred to the custom exit handler. At least it lets BIOS
	  controller functions to work now. See doc/dev notes.txt for details
	  on how this handler behaves.
	
	* Made stack usage a lot less wasteful in ResetGraph() (you only need
	  to allocate N words on stack based on N arguments of the function
	  being called.
