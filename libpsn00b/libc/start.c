/*
 * PSn00bSDK startup code
 * (C) 2021 Lameguy64, spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#define KERNEL_ARG_STRING	((const char *)   0x80000180)
#define KERNEL_RETURN_VALUE	((volatile int *) 0x8000dffc)

/* BIOS argv parser (unused, interferes with child executable argv passing) */

int			__argc;
const char	**__argv;

#if 0

#define ARGC_MAX 16

static const char	*_argv_buffer[ARGC_MAX];
static char			_arg_string_buffer[132];

static void _parse_kernel_args(void) {
	// Copy the argument string from kernel memory into a private buffer (which
	// won't be cleared or deallocated) and trim it at the first newline.
	memset(_arg_string_buffer, 0, 132);
	strncpy(_arg_string_buffer, KERNEL_ARG_STRING, 128);

	for (char *ptr = _arg_string_buffer; *ptr; ptr++) {
		if ((*ptr == '\r') || (*ptr == '\n')) {
			*ptr = 0;
			break;
		}
	}

	__argv = _argv_buffer;
	for (__argc = 0; __argc < ARGC_MAX; __argc++) {
		const char *ptr;
		if (!__argc)
			ptr = strtok(_arg_string_buffer, " ");
		else
			ptr = strtok(0, " ");

		_argv_buffer[__argc] = ptr;
		if (!ptr)
			break;
	}
}

#endif

/* Main */

// These are defined by the linker script. Note that these are *NOT* pointers,
// they are virtual symbols whose location matches their value. The simplest
// way to turn them into pointers is to declare them as arrays, so here we go.
extern uint8_t __text_start[];
extern uint8_t __bss_start[];
extern uint8_t _end[];
//extern uint8_t _gp[];

extern void (*__CTOR_LIST__[])(void);
extern void (*__DTOR_LIST__[])(void);

extern int main(int argc, const char **argv);

// Even though _start() usually takes no arguments, this implementation allows
// parent executables to pass args directly to child executables without having
// to overwrite the arg strings in kernel RAM.
int _start_inner(int argc, const char **argv) {
	__builtin_memset(__bss_start, 0, (void *) _end - (void *) __bss_start);

	// Initialize the heap and place it after the executable, assuming 2 MB of
	// RAM. Note that InitHeap() can be called again in main().
	InitHeap((void *) _end + 4, (void *) 0x801ffff8 - (void *) _end);

	//_parse_kernel_args();
	__argc = argc;
	__argv = argv;

	// Call the global constructors (if any) to initialize global objects
	// before calling main(). Constructors are put by the linker script in a
	// length-prefixed array in reverse order.
	for (int i = (int) __CTOR_LIST__[0]; i >= 1; i--)
		__CTOR_LIST__[i]();

	int value = main(argc, argv);

	// Call global destructors (in forward order).
	for (int i = 0; i < (int) __DTOR_LIST__[0]; i++)
		__DTOR_LIST__[i + 1]();

	//*KERNEL_RETURN_VALUE = value;
	return value;
}
