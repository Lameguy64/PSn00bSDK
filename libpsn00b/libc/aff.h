#pragma once

#include "malloc_impl.h"

#if !defined(_H_ALLOCATED_BLOCK_FIRST_FIT_) && MALLOC_IMPL == MALLOC_IMPL_AFF
#define _H_ALLOCATED_BLOCK_FIRST_FIT_

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

void affInitHeap(void* addr, size_t size);

void affTrackHeapUsage(ptrdiff_t alloc_incr);

void affGetHeapUsage(HeapUsage* usage);

void* affMalloc(size_t size);

void* affCalloc(size_t num, size_t size);

void* affRealloc(void* ptr, size_t size);

void affFree(void* ptr);

#endif // _H_ALLOCATED_BLOCK_FIRST_FIT_
