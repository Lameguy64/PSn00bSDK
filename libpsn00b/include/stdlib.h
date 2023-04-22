/*
 * PSn00bSDK standard library
 * (C) 2019-2022 PSXSDK authors, Lameguy64, spicyjpeg - MPL licensed
 * (C) 2023 saxbophone (minor additions) - MPL licensed
 */

#ifndef __STDLIB_H
#define __STDLIB_H

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

typedef struct {
    int quot; // quotient result of integer division
    int rem;  // remainder
} div_t;

/* API */

#ifdef __cplusplus
extern "C" {
#endif

extern int			__argc;
extern const char	**__argv;

void abort(void);

int rand(void);
void srand(int seed);

int abs(int j);
long labs(long i);
div_t div(int x, int y);

long strtol(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);
float strtof(const char *nptr, char **endptr);
double strtod(const char *nptr, char **endptr);
long double strtold(const char *nptr, char **endptr);

void InitHeap(void *addr, size_t size);
void *sbrk(ptrdiff_t incr);

void TrackHeapUsage(ptrdiff_t alloc_incr);
void GetHeapUsage(HeapUsage *usage);

void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
