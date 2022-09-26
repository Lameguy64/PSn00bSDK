/*
 * PSn00bSDK standard library
 * (C) 2019-2022 PSXSDK authors, Lameguy64, spicyjpeg - MPL licensed
 */

#ifndef __STDLIB_H
#define __STDLIB_H

#include <stddef.h>

/* Definitions */

#define RAND_MAX 0x7fff

/* API */

#ifdef __cplusplus
extern "C" {
#endif

extern int			__argc;
extern const char	**__argv;

int rand(void);
void srand(unsigned long seed);

int abs(int j);
long labs(long i);
long long strtoll(const char *nptr, char **endptr, int base);
long strtol(const char *nptr, char **endptr, int base);
long double strtold(const char *nptr, char **endptr);

double strtod(const char *nptr, char **endptr);
float strtof(const char *nptr, char **endptr);

void _mem_init(size_t ram_size, size_t stack_max_size);
void InitHeap(void *addr, size_t size);
//int SetHeapSize(size_t size);
void *sbrk(ptrdiff_t incr);

void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
