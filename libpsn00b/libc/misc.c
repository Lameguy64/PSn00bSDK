/*
 * PSn00bSDK standard library (misc. functions)
 * (C) 2022-2023 spicyjpeg - MPL licensed
 */

#undef SDK_LIBRARY_NAME

#include <assert.h>
#include <stdlib.h>

/* Abort functions */

void _assert_abort(const char *file, int line, const char *expr) {
	_sdk_log("%s:%d: assert(%s)\n", file, line, expr);

	for (;;)
		__asm__ volatile("");
}

void abort(void) {
	_sdk_log("abort()\n");

	for (;;)
		__asm__ volatile("");
}

void __cxa_pure_virtual(void) {
	_sdk_log("__cxa_pure_virtual()\n");

	for (;;)
		__asm__ volatile("");
}

/* abs() */

int abs(int value) {
	return (value < 0) ? (-value) : value;
}

/* Pseudorandom number generator */

static int _random_seed = 0;

int rand(void) {
	_random_seed *= 0x41c64e6d;
	_random_seed += 12345;

	return (_random_seed >> 16) & RAND_MAX;
}

void srand(int seed) {
	_random_seed = seed;
}
