#pragma once

#include "malloc_impl.h"

#if !defined(_H_ALLOCATED_BLOCK_FIRST_FIT_) && MALLOC_IMPL == MALLOC_IMPL_AFF
#define _H_ALLOCATED_BLOCK_FIRST_FIT_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

void affInitHeap(void* addr, size_t size);

void affTrackHeapUsage(ptrdiff_t alloc_incr);

void affGetHeapUsage(HeapUsage* usage);

void* affMalloc(size_t size);

void* affCalloc(size_t num, size_t size);

void* affRealloc(void* ptr, size_t size);

void affFree(void* ptr);

// ==== API ====

void InitHeap(void* addr, size_t size) {
	affInitHeap(addr, size);
	_sdk_log("Initialised AFF allocator\n");
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

#endif // _H_ALLOCATED_BLOCK_FIRST_FIT_
