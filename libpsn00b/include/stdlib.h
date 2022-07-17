/*
 * stdlib.h
 *
 * Standard library functions
 *
 * Inherited from PSXSDK
 */

#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

#define RAND_MAX	0x7fff

/* Conversion functions (not yet implemented) */

/*
extern int atoi(char *s);
extern long atol(char *s);
extern char atob(char *s); // Is this right?
*/

// Quick sort (not yet implemented)

//void qsort(void *base , int nel , int width , int (*cmp)(const void *,const void *));

#ifdef __cplusplus
extern "C" {
#endif

extern int __argc;
extern const char **__argv;

int rand(void);
void srand(unsigned long seed);

int abs(int j);
long labs(long i);
long long strtoll(const char *nptr, char **endptr, int base);
long strtol(const char *nptr, char **endptr, int base);
long double strtold(const char *nptr, char **endptr);

// Note: these use floats internally!
double strtod(const char *nptr, char **endptr);
float strtof(const char *nptr, char **endptr);

// Memory allocation functions
void _mem_init(size_t ram_size, size_t stack_max_size);
void InitHeap(void *addr, size_t size);
int SetHeapSize(size_t size);
void *malloc(size_t size);
void *calloc(size_t number, size_t size);
void free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif

