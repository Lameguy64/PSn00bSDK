Limited C standard library implementation, part of PSn00bSDK
2019 Lameguy64 / Meido-Tek Productions

	Some components were inherited from PSXSDK. This library covers only the
most commonly used C functions, mainly most string and memory manipulation
functions. Improvements to this library such as adding more standard C
functions are welcome.

	This library also contains the start code written in assembler which
performs basic initialization such as clearing the bss section, setting the
correct gp register value and initializing the heap for malloc.

	The dynamic memory allocation functions featured in this library are of
an original implementation and do not use the BIOS memory allocation functions
as they are are reportedly prone to memory leakage and is even explained in
the official library documents. The implementation employed uses a simple
first-fit memory allocation logic.


Library developer(s)/contributor(s):

	Lameguy64


Library header(s):
	
	stdio.h
	stdlib.h
	string.h
	strings.h
	malloc.h
	

Todo list:
	  
	* Many of the string manipulation and memory fill functions in string.c
	  are yet to be replaced with more efficient assembly implementations.


Changelog:

	05-23-2019 by Lameguy64:

	* Made stack usage a lot less wastefull in _start entrypoint.
