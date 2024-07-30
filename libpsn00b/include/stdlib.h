/*
 * PSn00bSDK standard library
 * (C) 2019-2023 PSXSDK authors, Lameguy64, spicyjpeg - MPL licensed
 */

#pragma once

#include <stddef.h>

/* Definitions */

#define RAND_MAX 0x7fff

/* Structure definitions */

typedef struct _HeapUsage {
	size_t total;		// Total size of heap + stack
	size_t heap;		// Amount of memory currently reserved for heap
	size_t stack;		// Amount of memory currently reserved for stack
	size_t alloc;		// Amount of memory currently allocated
	size_t alloc_max;	// Maximum amount of memory ever allocated
} HeapUsage;

/* API */

#ifdef __cplusplus
extern "C" {
#endif

extern int __argc;
extern const char **__argv;

__attribute__((noreturn)) void abort(void);

int abs(int value);
int rand(void);
void srand(int seed);

long strtol(const char *str, char **str_end, int base);
long long strtoll(const char *str, char **str_end, int base);
//float strtof(const char *str, char **str_end);
//double strtod(const char *str, char **str_end);
//long double strtold(const char *str, char **str_end);

void *sbrk(ptrdiff_t incr);

void InitHeap(void *addr, size_t size);
void TrackHeapUsage(ptrdiff_t alloc_incr);
void GetHeapUsage(HeapUsage *usage);

void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#ifdef __cplusplus
}
#endif
