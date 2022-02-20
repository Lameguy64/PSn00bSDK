#include <stdio.h>

void abort() {
	printf("abort()\n");

	for (;;)
		__asm__ volatile("");
}

void _assert_abort(const char *file, int line, const char *expr) {
	printf("%s:%d: assert(%s)\n", file, line, expr);

	for (;;)
		__asm__ volatile("");
}
