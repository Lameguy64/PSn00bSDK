/*
 * stdlib.h
 *
 * Standard library functions
 *
 * Inherited from PSXSDK
 */

#ifndef _STDLIB_H
#define _STDLIB_H

#define RAND_MAX	0x7fff

/* Conversion functions (not yet implemented) */

/*
extern int atoi(char *s);
extern long atol(char *s);
extern char atob(char *s); // Is this right?
*/

// Random number functions (not yet implemented)

/*
int rand();
void srand(unsigned int seed);
*/

// Quick sort (not yet implemented)

//void qsort(void *base , int nel , int width , int (*cmp)(const void *,const void *));

// Memory allocation functions (not yet implemented, avoid using BIOS as they are reportedly buggy)

/*
#warning "malloc() family of functions NEEDS MORE TESTING"

void *malloc(int size);
void free(void *buf);
void *calloc(int number, int size);
void *realloc(void *buf , int n);
*/

int rand();
void srand(unsigned long seed);

int abs(int j);
long long strtoll(const char *nptr, char **endptr, int base);
long strtol(const char *nptr, char **endptr, int base);
long double strtold(const char *nptr, char **endptr);

// Note: these use floats internally!
double strtod(const char *nptr, char **endptr);
float strtof(const char *nptr, char **endptr);

#endif

