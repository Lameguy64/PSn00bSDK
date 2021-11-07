/*
 * PSn00bSDK startup code
 * (C) 2021 Lameguy64, spicyjpeg - MPL licensed
 */

#include <sys/types.h>
#include <string.h>
#include <malloc.h>

#define KERNEL_ARG_STRING   ((const char *)   0x80000180)
#define KERNEL_RETURN_VALUE ((volatile int *) 0x8000dffc)

/* Argument parsing */

int32_t    __argc;
const char **__argv;

#define ARGC_MAX 16

static const char *_argv_buffer[ARGC_MAX];
static char       _arg_string_buffer[132];

static void _parse_kernel_args() {
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

// How much space at the end of RAM to leave for the stack (instead of using it
// as heap). By default 128 KB are reserved for the stack, but this constant
// can be overridden in main.c (or anywhere else) simply by redeclaring it
// without the weak attribute.
const int32_t __attribute__((weak)) STACK_MAX_SIZE = 0x20000;

// These are defined by the linker script. Note that these are *NOT* pointers,
// they are virtual symbols whose location matches their value. The simplest
// way to turn them into pointers is to declare them as arrays, so here we go.
extern uint8_t __text_start[];
extern uint8_t __bss_start[];
extern uint8_t _end[];
//extern uint8_t _gp[];

extern void (*__CTOR_LIST__[])(void);
extern void (*__DTOR_LIST__[])(void);

extern int32_t main(int32_t argc, const char* argv[]);

// Even though _start() usually takes no arguments, this implementation allows
// parent executables to pass args directly to child executables without having
// to overwrite the arg strings in kernel RAM.
void _start(int32_t override_argc, const char **override_argv) {
	__asm__ volatile("la $gp, _gp;");
	
	// Mem init assembly function (clears BSS and InitHeap to _end which is
	// not possible to do purely in C because the linker complains about
	// relocation truncated to fit: R_MIPS_GPREL16 against `_end'
	// Workaround is to do it in assembly because la pseudo-op doesn't use
	// stupid gp relative addressing
	//_mem_init();

	// Clear BSS 4 bytes at a time. BSS is always aligned to 4 bytes by the
	// linker script.
	for (uint32_t *i = (uint32_t *) __bss_start; i < (uint32_t *) _end; i++)
		*i = 0;

	// Calculate how much RAM is available after the loaded executable and
	// initialize heap accordingly.
	void   *exe_end = _end + 4;
	size_t exe_size = (size_t) exe_end - (size_t) __text_start;
	InitHeap(exe_end, 0x1f0000 - (exe_size + STACK_MAX_SIZE) & 0xfffffffc);

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
