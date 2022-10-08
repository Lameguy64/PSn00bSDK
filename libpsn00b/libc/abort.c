
#include <stdio.h>

#ifdef DEBUG
#define _LOG(...) printf(__VA_ARGS__)
#else
#define _LOG(...)
#endif

/* Standard abort */

void abort() {
	_LOG("abort()\n");

	for (;;)
		__asm__ volatile("");
}

/* Internal function used by assert() macro */

void _assert_abort(const char *file, int line, const char *expr) {
	_LOG("%s:%d: assert(%s)\n", file, line, expr);

	for (;;)
		__asm__ volatile("");
}

/* Pure virtual function call (C++) */

void __cxa_pure_virtual(void) {
	_LOG("__cxa_pure_virtual()\n");

	for (;;)
		__asm__ volatile("");
}
