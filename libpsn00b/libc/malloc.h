#pragma once

#ifndef _H_MALLOC_
#define _H_MALLOC_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "gnu_version.h"
#include "malloc_impl.h"

__attribute__((weak)) void InitHeap(void* addr, size_t size);
__attribute__((weak)) void TrackHeapUsage(ptrdiff_t alloc_incr);
__attribute__((weak)) void GetHeapUsage(HeapUsage* usage);

__attribute__((weak, hot)) void free(void* ptr);

__attribute__((weak, hot, malloc, alloc_size(1)
#ifdef gnu_version_10
, malloc(free, 1)
#endif
)) void* malloc(size_t size);

__attribute__((weak, hot, malloc, alloc_size(1, 2)
#ifdef gnu_version_10
, malloc(free, 1)
#endif
)) void* calloc(size_t num, size_t size);

__attribute__((weak, hot, alloc_size(2)
#ifdef gnu_version_10
, malloc(free, 2)
#endif
)) void* realloc(void* ptr, size_t size);

#endif // _H_MALLOC_
