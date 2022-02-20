#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

extern "C" void __cxa_pure_virtual(void) {
	printf("__cxa_pure_virtual()\n");

	for (;;)
		__asm__ volatile("");
}

void* operator new(size_t size) {
	return malloc(size);
}

void* operator new[](size_t size) {
	return malloc(size);
}

void operator delete(void* ptr) {
	free(ptr);
}

void operator delete[](void* ptr) {
	free(ptr);
}

/*-
 * <https://en.cppreference.com/w/cpp/memory/new/operator_delete>
 *
 * Called if a user-defined replacement is provided, except that it's
 * unspecified whether other overloads or this overload is called when deleting
 * objects of incomplete type and arrays of non-class and trivially-destructible
 * class types.
 *
 * A memory allocator can use the given size to be more efficient */
void operator delete(void* ptr, unsigned int) {
	free(ptr);
}
