
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Default new/delete operators */

void *operator new(size_t size) noexcept {
	return malloc(size);
}

void *operator new[](size_t size) noexcept {
	return malloc(size);
}

void operator delete(void *ptr) noexcept {
	free(ptr);
}

void operator delete[](void *ptr) noexcept {
	free(ptr);
}

/*
 * https://en.cppreference.com/w/cpp/memory/new/operator_delete
 *
 * Called if a user-defined replacement is provided, except that it's
 * unspecified whether other overloads or this overload is called when deleting
 * objects of incomplete type and arrays of non-class and trivially
 * destructible class types.
 *
 * A memory allocator can use the given size to be more efficient.
 */
void operator delete(void *ptr, size_t size) noexcept {
	free(ptr);
}

/* Placement new operators */

void *operator new(size_t size, void *ptr) noexcept {
	return ptr;
}

void *operator new[](size_t size, void *ptr) noexcept {
	return ptr;
}
