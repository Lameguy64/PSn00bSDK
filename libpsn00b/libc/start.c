#include <stdio.h>
#include <malloc.h>

#define load_gp() __asm__ volatile ( \
	"la $gp, _gp;" )

extern int _end;
extern int main(int argc, const char* argv[]);

void _mem_init(void);


static void _call_global_ctors(void)
{
	extern void (*__CTOR_LIST__[])(void);

	// Constructors are called in reverse order of the list
	int i;
	for (i = (int)__CTOR_LIST__[0]; i >= 1; i--) {
		// Each function handles one or more destructor (within
		// file scope)
		__CTOR_LIST__[i]();
	}
}

static void _call_global_dtors(void)
{
	extern void (*__DTOR_LIST__[])(void);

	/* Destructors in forward order */
	int i;
	for (i = 0; i < (int)__DTOR_LIST__[0]; i++) {
			/* Each function handles one or more destructor (within
			 * file scope) */
			__DTOR_LIST__[i + 1]();
	}
}

void _start(void) {
	
	// Load GP address
	load_gp();
	
	// Mem init assembly function (clears BSS and InitHeap to _end which is
	// not possible to do purely in C because the linker complains about
	// relocation truncated to fit: R_MIPS_GPREL16 against `_end'
	// Workaround is to do it in assembly because la pseudo-op doesn't use
	// stupid gp relative addressing
	_mem_init();
	
	_call_global_ctors();
	
	main(0, NULL);
	
	_call_global_dtors();
	
}