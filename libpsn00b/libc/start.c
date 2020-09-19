#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define load_gp() __asm__ volatile ( \
	"la $gp, _gp;" )

extern int _end;
extern int main(int argc, const char* argv[]);

void _mem_init(void);

int __argc;
const char **__argv;

static const char *_arg_ptrs_int[8];
static char _arg_buff[132];

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

void _parse_args( int argc, const char *args[] )
{
	int i;
	char *c,*s;
	
	memset( _arg_buff, 0, 132 );
	
	if( !args )
	{
		// Use arguments from kernel if args is NULL
		strncpy( _arg_buff, (char*)0x180, 128 );
		
		// Clean-up args froom stray line-ends
		while( ( c = strrchr( _arg_buff, '\r' ) ) ||
			( c = strrchr( _arg_buff, '\n' ) ) )
			*c = 0;
	}
	else
	{
		__argc = argc;
		__argv = args;
		return;
	}
	
	__argc = 0;
	for( i=0; i<8; i++ )
		_arg_ptrs_int[i] = 0;
	
	s = _arg_buff;
	while( c = strtok( s, " " ) )
	{
		_arg_ptrs_int[__argc] = c;
		__argc++;
		s = NULL;
		if( __argc >= 8 )
			break;
	}
	
	__argv = _arg_ptrs_int;
	
} /* parse_args */

void _start( int argc, const char *args[] )
{
	// Load GP address
	load_gp();
	
	// Mem init assembly function (clears BSS and InitHeap to _end which is
	// not possible to do purely in C because the linker complains about
	// relocation truncated to fit: R_MIPS_GPREL16 against `_end'
	// Workaround is to do it in assembly because la pseudo-op doesn't use
	// stupid gp relative addressing
	_mem_init();
	
	// process command line arguments
	_parse_args( argc, args );
	
	_call_global_ctors();
	
	*((int*)0x8000DFFC) = main( __argc, __argv );
	
	_call_global_dtors();
	
	// Set return value to kernel return value area
	
} /* _start */