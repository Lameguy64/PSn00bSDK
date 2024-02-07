#include "malloc.h"

#if MALLOC_IMPL == MALLOC_IMPL_TLSF
#include "tlsf.h"

#elif MALLOC_IMPL == MALLOC_IMPL_AFF
#include "aff.h"

#elif MALLOC_IMPL == MALLOC_IMPL_CUSTOM
#include <stdio.h>

void InitHeap(void* addr, size_t size) {
	printf("[ERROR] Unimplemented custom allocator handle: void InitHeap(void* addr, size_t size)\n");
	abort();
}

void TrackHeapUsage(ptrdiff_t alloc_incr) {
	printf("[ERROR] Unimplemented custom allocator handle: void TrackHeapUsage(ptrdiff_t alloc_incr)\n");
	abort();
}

void GetHeapUsage(HeapUsage* usage) {
	printf("[ERROR] Unimplemented custom allocator handle: void GetHeapUsage(HeapUsage* usage)\n");
	abort();
}

void* malloc(size_t size) {
	printf("[ERROR] Unimplemented custom allocator handle: void* malloc(size_t)\n");
	abort();
	return NULL;
}

void* calloc(size_t num, size_t size) {
	printf("[ERROR] Unimplemented custom allocator handle: void* calloc(size_t num, size_t size)\n");
	abort();
	return NULL;
}

void* realloc(void* ptr, size_t size) {
	printf("[ERROR] Unimplemented custom allocator handle: void* realloc(void* ptr, size_t size)\n");
	abort();
	return NULL;
}

void free(void* ptr) {
	printf("[ERROR] Unimplemented custom allocator handle: void free(void* ptr)\n");
	abort();
}

#else
#error Invalid MALLOC_IMPL defined, must be one of (0: custom, 1: AFF, 2: TLSF)
#endif
