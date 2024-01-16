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
#include <stdio.h>
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
  printf("[Sbrk] literal shift %p, aligned shift %p\n", old_end + incr,
         new_end);

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
  printf("[FindFit] size: 0x%x\n", size);
  for (; prev; prev = prev->next) {
    if (prev->next) {
      uintptr_t next_bot = (uintptr_t)prev->next;
      printf("[FindFit] next_bot: %p\n", (void *)next_bot);
      next_bot -= (uintptr_t)prev->ptr + prev->size;
      printf("[FindFit] ptr: %p, size: 0x%x, offset: %p, next_bot: %p\n",
             prev->ptr, prev->size, prev->ptr + prev->size, (void *)next_bot);
      if (next_bot >= size) {
        printf("[FindFit] found %p\n", prev);
        return prev;
      }
    }
  }
  printf("[FindFit] Not found: %p\n", prev);
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
    printf("[Malloc] bottom heap shifted: %p < %p\n", _alloc_start + _size,
           _alloc_head);
    BlockHeader *new = (BlockHeader *)_alloc_start;

    void *ptr = (void *)&new[1];
    new->ptr = ptr;
    new->size = _size_nh;
    new->prev = 0;
    new->next = _alloc_head;
    printf("[Malloc] new->next: %p\n", new->next);

    _alloc_head->prev = new;
    _alloc_head = new;

    TrackHeapUsage(_size);
    return ptr;
  }

  // No luck at the beginning of the heap, let's walk the heap to find a fit.
  BlockHeader *prev = _find_fit(_alloc_head, _size);
  if (prev) {
    BlockHeader *new = (BlockHeader *)((uintptr_t)prev->ptr + prev->size);
    printf("[Malloc] found fit: %p\n", new);

    void *ptr = (void *)&new[1];
    new->ptr = ptr;
    new->size = _size_nh;
    new->prev = prev;
    new->next = prev->next;
    printf("[Malloc] fit, new->next: %p\n", new->next);

    (new->next)->prev = new;
    prev->next = new;
    printf("[Malloc] fit, prev->next: %p\n", prev->next);

    TrackHeapUsage(_size);
    return ptr;
  }

  // Time to extend the size of the heap.
  BlockHeader *new = (BlockHeader *)sbrk(_size);
  if (!new)
    return 0;
  printf("[Malloc] extended heap: %p\n", new);
  void *ptr = (void *)&new[1];
  new->ptr = ptr;
  new->size = _size_nh;
  new->prev = _alloc_tail;
  new->next = 0;

  _alloc_tail->next = new;
  printf("[Malloc] extend, _alloc_tail->next: %p\n", _alloc_tail->next);
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
    printf("[Realloc] new size shorter: 0x%x >= 0x%x\n", prev->size, _size_nh);
    TrackHeapUsage(_size_nh - prev->size);
    prev->size = _size_nh;

    // This is the last block, move the break back to accomodate shrinking
    if (!prev->next) {
      // We have overriden prev->size, need to calculate it from break
      void *new_break = sbrk((ptr - sbrk(0)) + _size_nh);
      printf("[Realloc] last block, shrink break: %p\n", new_break);
    }
    return ptr;
  }

  // New memory block larger; is it the last one?
  if (!prev->next) {
    printf("[Realloc] new block larger\n");
    void *new = sbrk(_size_nh - prev->size);
    if (!new)
      return 0;
    printf("[Realloc] new break: 0x%x => %p\n", _size_nh - prev->size, new);
    TrackHeapUsage(_size_nh - prev->size);
    prev->size = _size_nh;
    return ptr;
  }

  // Do we have free memory after it?
  if (((prev->next)->ptr - sizeof(BlockHeader) - ptr) >= _size_nh) {
    printf("[Realloc] free mem after: 0x%x >= 0x%x\n",
           (prev->next)->ptr - sizeof(BlockHeader) - ptr, _size_nh);
    TrackHeapUsage(_size_nh - prev->size);
    prev->size = _size_nh;
    return ptr;
  }

  // No luck.
  void *new = malloc(size);
  if (!new)
    return 0;
  printf("[Realloc] new malloc addr: %p\n", new);
  __builtin_memcpy(new, ptr, prev->size);
  free(ptr);
  return new;
}

__attribute__((weak)) void free(void *ptr) {
  if (!ptr || !_alloc_head)
    return;

  // First block; bumping head ahead.
  if (ptr == _alloc_head->ptr) {
    printf("[Free] first block, bump head forward\n");
    size_t size = _alloc_head->size + sizeof(BlockHeader);
    printf("[Free] size: 0x%x\n", size);
    _alloc_head = _alloc_head->next;
    printf("[Free] new head: %p\n", _alloc_head);
    if (_alloc_head) {
      _alloc_head->prev = 0;
      printf("[Free] New head exists, setting prev to null\n");
    } else {
      printf("[Free] No new head exists, nulling tail\n");
      _alloc_tail = 0;
      // sbrk(-size);
    }

    TrackHeapUsage(-size);
    return;
  }

  // Finding the proper block
  BlockHeader *cur = _alloc_head;
  printf("[Free] find block, base: %p\n", cur);
  for (cur = _alloc_head; ptr != cur->ptr; cur = cur->next) {
    if (!cur->next)
      return;
  }
  printf("[Free] found: %p\n", cur);

  size_t heap_change;
  if (cur->next) {
    // In the middle, just unlink it
    printf("[Free] has next, setting next->prev to cur->prev: %p\n", cur->prev);
    (cur->next)->prev = cur->prev;
    heap_change = -(cur->size) - sizeof(BlockHeader);
  } else {
    // At the end, shrink heap
    printf("[Free] at end of heap\n");
    void *top = sbrk(0);
    printf("[Free] heap top: %p\n", top);
    size_t size = (top - (cur->prev)->ptr) + (cur->prev)->size;
    printf("[Free] size: 0x%x\n", size);
    _alloc_tail = cur->prev;
    printf("[Free] new tail: %p\n", _alloc_tail);

    sbrk(-size);
    heap_change = -size;
  }
  printf("[Free] heap_change: 0x%x\n", heap_change);
  TrackHeapUsage(heap_change);
  (cur->prev)->next = cur->next;
  printf("[Free] cur->prev->next: %p\n", (cur->prev)->next);
  printf("[Free] setting prev->next to cur->next: %p\n", cur->next);
}
