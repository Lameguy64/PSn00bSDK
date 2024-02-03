/*
 * PSn00bSDK default memory allocator
 * (C) 2022 Nicolas Noble, spicyjpeg
 *
 * This code is based on psyqo's malloc implementation, available here:
 * https://github.com/grumpycoders/pcsx-redux/blob/main/src/mips/psyqo/src/alloc.c
 *
 * Heap management and memory allocation are completely separate, with the
 * latter being built on top of the former. This makes it possible to override
 * only InitHeap() and sbrk() while still using the default allocator, or
 * override malloc()/realloc()/free() while using the default heap manager.
 * Custom allocators should call TrackHeapUsage() to let the heap manager know
 * how much memory is allocated at a given time.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "tlsf.h"

#define ALIGN_SIZE 8
#define _align(x, n) (((x) + ((n) - 1)) & ~((n) - 1))

/* Private types */

typedef struct __attribute__((aligned(ALIGN_SIZE))) _BlockHeader {
	struct _BlockHeader	*prev, *next;
	void				*ptr;
	size_t				size;
} BlockHeader;

/* Internal globals */

static void			*_heap_start, *_heap_end, *_heap_limit;
static size_t		_heap_alloc, _heap_alloc_max;

static void			*_alloc_start;
static BlockHeader	*_alloc_head, *_alloc_tail;

static tlsf_t tlsf;

/* Heap management API */

__attribute__((weak)) void InitHeap(void *addr, size_t size) {
	tlsf = tlsf_create_with_pool(addr, size);
	if (!tlsf) {
		printf("Unable to initialise heap\n");
		abort();
	}
	_heap_start = addr;
	_heap_end   = addr;
	_heap_limit = (void *) ((uintptr_t) addr + size);

	_heap_alloc     = 0;
	_heap_alloc_max = 0;

	_alloc_start = addr;
	_alloc_head  = 0;
	_alloc_tail  = 0;
}

__attribute__((weak)) void TrackHeapUsage(ptrdiff_t alloc_incr) {
	_heap_alloc += alloc_incr;

	if (_heap_alloc > _heap_alloc_max)
		_heap_alloc_max = _heap_alloc;
}

__attribute__((weak)) void GetHeapUsage(HeapUsage *usage) {
	usage->total = _heap_limit - _heap_start;
	usage->heap  = _heap_end   - _heap_start;
	usage->stack = _heap_limit - _heap_end;

	usage->alloc     = _heap_alloc;
	usage->alloc_max = _heap_alloc_max;
}

/* Memory allocator */

__attribute__((weak)) void *malloc(size_t size) {
	return tlsf_malloc(tlsf, size);
}

__attribute__((weak)) void *calloc(size_t num, size_t size) {
	return malloc(num * size);
}

__attribute__((weak)) void *realloc(void *ptr, size_t size) {
	return tlsf_realloc(tlsf, ptr, size);
}

__attribute__((weak)) void free(void *ptr) {
	return tlsf_free(tlsf, ptr);
}
