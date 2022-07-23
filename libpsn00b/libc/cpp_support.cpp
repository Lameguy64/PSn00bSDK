/*
 * PSn00bSDK C++ support library
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* GCC builtins */

extern "C" void *__builtin_new(size_t size) {
	return malloc(size);
}

extern "C" void __builtin_delete(void *ptr) {
	free(ptr);
}

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

// https://en.cppreference.com/w/cpp/memory/new/operator_delete
void operator delete(void *ptr, size_t size) noexcept {
	free(ptr);
}

void operator delete[](void *ptr, size_t size) noexcept {
	free(ptr);
}

/* Placement new operators */

void *operator new(size_t size, void *ptr) noexcept {
	return ptr;
}

void *operator new[](size_t size, void *ptr) noexcept {
	return ptr;
}
