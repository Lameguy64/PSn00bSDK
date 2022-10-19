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

/* Argument parsing */

int			__argc;
const char	**__argv;

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

extern int main(int argc, const char* argv[]);

// Even though _start() usually takes no arguments, this implementation allows
// parent executables to pass args directly to child executables without having
// to overwrite the arg strings in kernel RAM.
void _start_inner(int32_t override_argc, const char **override_argv) {
	//__asm__ volatile("la $gp, _gp;");

	// Clear BSS 4 bytes at a time. BSS is always aligned to 4 bytes by the
	// linker script.
	for (uint32_t *i = (uint32_t *) __bss_start; i < (uint32_t *) _end; i++)
		*i = 0;

	// Initialize the heap and place it after the executable, assuming 2 MB of
	// RAM. Note that InitHeap() can be called again in main().
	InitHeap((void *) _end + 4, (void *) 0x801ffff8 - (void *) _end);

	if (override_argv) {
		__argc = override_argc;
		__argv = override_argv;
	} else {
		_parse_kernel_args();
	}

	// Call the global constructors (if any) to initialize global objects
	// before calling main(). Constructors are put by the linker script in a
	// length-prefixed array in reverse order.
	for (uint32_t i = (uint32_t) __CTOR_LIST__[0]; i >= 1; i--)
		__CTOR_LIST__[i]();

	// Store main()'s return value into the kernel return value area (for child
	// executables).
	*KERNEL_RETURN_VALUE = main(__argc, __argv);

	// Call global destructors (in forward order).
	for (uint32_t i = 0; i < (uint32_t) __DTOR_LIST__[0]; i++)
		__DTOR_LIST__[i + 1]();
}
