#include "malloc.h"

#if MALLOC_IMPL == MALLOC_IMPL_TLSF
#include "tlsf.h"

#elif MALLOC_IMPL == MALLOC_IMPL_AFF
#include "aff.h"

#elif MALLOC_IMPL == MALLOC_IMPL_CUSTOM
#include <assert.h>

__attribute__((weak))
void InitHeap(void* addr, size_t size) {
	_sdk_log("[ERROR] Unimplemented custom allocator handle: void InitHeap(void* addr, size_t size)\n");
	abort();
}

__attribute__((weak))
void TrackHeapUsage(ptrdiff_t alloc_incr) {
	_sdk_log("[ERROR] Unimplemented custom allocator handle: void TrackHeapUsage(ptrdiff_t alloc_incr)\n");
	abort();
}

__attribute__((weak))
void GetHeapUsage(HeapUsage* usage) {
	_sdk_log("[ERROR] Unimplemented custom allocator handle: void GetHeapUsage(HeapUsage* usage)\n");
	abort();
}

__attribute__((weak))
void* malloc(size_t size) {
	_sdk_log("[ERROR] Unimplemented custom allocator handle: void* malloc(size_t)\n");
	abort();
	return NULL;
}

__attribute__((weak))
void* calloc(size_t num, size_t size) {
	_sdk_log("[ERROR] Unimplemented custom allocator handle: void* calloc(size_t num, size_t size)\n");
	abort();
	return NULL;
}

__attribute__((weak))
void* realloc(void* ptr, size_t size) {
	_sdk_log("[ERROR] Unimplemented custom allocator handle: void* realloc(void* ptr, size_t size)\n");
	abort();
	return NULL;
}

__attribute__((weak))
void free(void* ptr) {
	_sdk_log("[ERROR] Unimplemented custom allocator handle: void free(void* ptr)\n");
	abort();
}

#else
#error Invalid MALLOC_IMPL defined, must be one of (0: custom, 1: AFF, 2: TLSF)
#endif
