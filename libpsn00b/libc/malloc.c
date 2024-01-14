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

#define _align(x, n) (((x) + ((n)-1)) & ~((n)-1))

/* Private types */

typedef struct _BlockHeader {
  struct _BlockHeader *prev, *next;
  void *ptr;
  size_t size;
} BlockHeader;

/* Internal globals */

static void *_heap_start, *_heap_end, *_heap_limit;
static size_t _heap_alloc, _heap_alloc_max;

static void *_alloc_start;
static BlockHeader *_alloc_head, *_alloc_tail;

/* Heap management API */

__attribute__((weak)) void InitHeap(void *addr, size_t size) {
  _heap_start = addr;
  _heap_end = addr;
  _heap_limit = (void *)((uintptr_t)addr + size);

  _heap_alloc = 0;
  _heap_alloc_max = 0;

  _alloc_start = addr;
  _alloc_head = 0;
  _alloc_tail = 0;
}

__attribute__((weak)) void *sbrk(ptrdiff_t incr) {
  void *old_end = _heap_end;
  void *new_end = (void *)_align((uintptr_t)old_end + incr, 8);

  if (new_end > _heap_limit)
    return 0;

  _heap_end = new_end;
  return old_end;
}

__attribute__((weak)) void TrackHeapUsage(ptrdiff_t alloc_incr) {
  _heap_alloc += alloc_incr;

  if (_heap_alloc > _heap_alloc_max)
    _heap_alloc_max = _heap_alloc;
}

__attribute__((weak)) void GetHeapUsage(HeapUsage *usage) {
  usage->total = _heap_limit - _heap_start;
  usage->heap = _heap_end - _heap_start;
  usage->stack = _heap_limit - _heap_end;

  usage->alloc = _heap_alloc;
  usage->alloc_max = _heap_alloc_max;
}

/* Memory allocator */

static BlockHeader *_find_fit(BlockHeader *head, size_t size) {
  BlockHeader *prev = head;

  for (; prev; prev = prev->next) {
    if (prev->next) {
      uintptr_t next_bot = (uintptr_t)prev->next;
      next_bot -= (uintptr_t)prev->ptr + prev->size;

      if (next_bot >= size)
        return prev;
    }
  }

  return prev;
}

__attribute__((weak)) void *malloc(size_t size) {
  if (!size)
    return 0;

  size_t _size = _align(size + sizeof(BlockHeader), 8);
  size_t _size_nh = _size - sizeof(BlockHeader);

  // Nothing's initialized yet? Let's just initialize the bottom of our heap,
  // flag it as allocated.
  if (!_alloc_head) {
    // if (!_alloc_start)
    //_alloc_start = sbrk(0);

    BlockHeader *new = (BlockHeader *)sbrk(_size);
    if (!new)
      return 0;

    void *ptr = (void *)&new[1];
    new->ptr = ptr;
    new->size = _size_nh;
    new->prev = 0;
    new->next = 0;

    _alloc_head = new;
    _alloc_tail = new;

    TrackHeapUsage(_size);
    return ptr;
  }

  // We *may* have the bottom of our heap that has shifted, because of a free.
  // So let's check first if we have free space there, because I'm nervous
  // about having an incomplete data structure.
  if (((uintptr_t)_alloc_start + _size) < ((uintptr_t)_alloc_head)) {
    BlockHeader *new = (BlockHeader *)_alloc_start;

    void *ptr = (void *)&new[1];
    new->ptr = ptr;
    new->size = _size_nh;
    new->prev = 0;
    new->next = _alloc_head;

    _alloc_head->prev = new;
    _alloc_head = new;

    TrackHeapUsage(_size);
    return ptr;
  }

  // No luck at the beginning of the heap, let's walk the heap to find a fit.
  BlockHeader *prev = _find_fit(_alloc_head, _size);
  if (prev) {
    BlockHeader *new = (BlockHeader *)((uintptr_t)prev->ptr + prev->size);

    void *ptr = (void *)&new[1];
    new->ptr = ptr;
    new->size = _size_nh;
    new->prev = prev;
    new->next = prev->next;

    (new->next)->prev = new;
    prev->next = new;

    TrackHeapUsage(_size);
    return ptr;
  }

  // Time to extend the size of the heap.
  BlockHeader *new = (BlockHeader *)sbrk(_size);
  if (!new)
    return 0;

  void *ptr = (void *)&new[1];
  new->ptr = ptr;
  new->size = _size_nh;
  new->prev = _alloc_tail;
  new->next = 0;

  _alloc_tail->next = new;
  _alloc_tail = new;

  TrackHeapUsage(_size);
  return ptr;
}

__attribute__((weak)) void *calloc(size_t num, size_t size) {
  return malloc(num * size);
}

__attribute__((weak)) void *realloc(void *ptr, size_t size) {
  if (!size) {
    free(ptr);
    return 0;
  }
  if (!ptr)
    return malloc(size);

  size_t _size = _align(size + sizeof(BlockHeader), 8);
  size_t _size_nh = _size - sizeof(BlockHeader);
  BlockHeader *prev = (BlockHeader *)((uintptr_t)ptr - sizeof(BlockHeader));

  // New memory block shorter?
  if (prev->size >= _size_nh) {
    TrackHeapUsage(_size_nh - prev->size);
    prev->size = _size_nh;

    if (!prev->next)
      sbrk((ptr - sbrk(0)) + _size_nh);

    return ptr;
  }

  // New memory block larger; is it the last one?
  if (!prev->next) {
    void *new = sbrk(_size_nh - prev->size);
    if (!new)
      return 0;

    TrackHeapUsage(_size_nh - prev->size);
    prev->size = _size_nh;
    return ptr;
  }

  // Do we have free memory after it?
  if (((prev->next)->ptr - sizeof(BlockHeader) - ptr) > _size_nh) {
    TrackHeapUsage(_size_nh - prev->size);
    prev->size = _size_nh;
    return ptr;
  }

  // No luck.
  void *new = malloc(size);
  if (!new)
    return 0;

  __builtin_memcpy(new, ptr, prev->size);
  free(ptr);
  return new;
}

__attribute__((weak)) void free(void *ptr) {
  if (!ptr || !_alloc_head)
    return;

  // First block; bumping head ahead.
  if (ptr == _alloc_head->ptr) {
    size_t size = _alloc_head->size;
    size += (uintptr_t)_alloc_head->ptr - (uintptr_t)_alloc_head;
    _alloc_head = _alloc_head->next;

    if (_alloc_head) {
      _alloc_head->prev = 0;
    } else {
      _alloc_tail = 0;
      sbrk(-size - sizeof(BlockHeader));
    }

    TrackHeapUsage(-(_alloc_head->size) - sizeof(BlockHeader));
    return;
  }

  // Finding the proper block
  BlockHeader *cur = _alloc_head;

  for (cur = _alloc_head; ptr != cur->ptr; cur = cur->next) {
    if (!cur->next)
      return;
  }

  if (cur->next) {
    // In the middle, just unlink it
    (cur->next)->prev = cur->prev;
  } else {
    // At the end, shrink heap
    void *top = sbrk(0);
    size_t size = (top - (cur->prev)->ptr) + (cur->prev)->size;
    _alloc_tail = cur->prev;

    sbrk(-size);
  }

  TrackHeapUsage(-(cur->size) - sizeof(BlockHeader));
  (cur->prev)->next = cur->next;
}
