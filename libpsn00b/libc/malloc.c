#include "malloc.h"

#if MALLOC_IMPL == MALLOC_IMPL_TLSF
#include "tlsf.h"

tlsf_t __tlsf_allocator = NULL;

void InitHeap(void* addr, size_t size) {
	if (__tlsf_allocator != NULL) {
		printf("[ERROR] Heap already initialised\n");
		abort();
		return;
	}
	__tlsf_allocator = tlsf_create_with_pool(addr, size);
	if (__tlsf_allocator == null) {
		printf("[ERROR] Unable to initialise allocator\n");
		return;
	}
}

void TrackHeapUsage(ptrdiff_t alloc_incr) {
}

void GetHeapUsage(HeapUsage* usage) {
}

void* malloc(size_t size) {
	return tlsf_malloc(__tlsf_allocator, size);
}

void* calloc(size_t num, size_t size) {
	return tlsf_malloc(__tlsf_allocator, num * size);
}

void* realloc(void* ptr, size_t size) {
	return tlsf_realloc(__tlsf_allocator, ptr, size);
}

void free(void* ptr) {
	tlsf_free(__tlsf_allocator, ptr);
}

#elif MALLOC_IMPL == MALLOC_IMPL_AFF
#include "aff.h"

void InitHeap(void* addr, size_t size) {
	affInitHeap(addr, size);
}

void TrackHeapUsage(ptrdiff_t alloc_incr) {
	affTrackHeapUsage(alloc_incr);
}

void GetHeapUsage(HeapUsage* usage) {
	affGetHeapUsage(usage);
}

void* malloc(size_t size) {
	return affMalloc(size);
}

void* calloc(size_t num, size_t size) {
	return affCalloc(num, size);
}

void* realloc(void* ptr, size_t size) {
	return affRealloc(ptr, size);
}

void free(void* ptr) {
	affFree(ptr);
}

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

#endif
