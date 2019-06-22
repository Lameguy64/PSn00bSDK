/*
 * string.h
 *
 * Prototypes for string functions of the C library
 *
 * PSXSDK
 */

#ifndef _STRING_H
#define _STRING_H

#ifdef __cplusplus
extern "C" {
#endif

int strcmp(const char *dst , const char *src);
int strncmp(const char *dst , const char *src , int len);
char *strpbrk(const char *dst , const char *src);
char *strtok(char *s , char *set);
char *strstr(const char *big , const char *little);

//int strspn(char *s , char *set);
//int strcspn(char *s , char *set);
//int index(char *s , int c);
//int rindex(char *s , int c);

char *strcat(char *s , const char *append);
char *strncat(char *s , const char *append, int n);
char *strcpy(char *dst , const char *src);
char *strncpy(char *dst , const char *src , int n);
int strlen(const char *s);
//char *strchr(const char *s , int c);
//char *strrchr(const char *s , int c);

void *memmove(void *dst , const void *src , int n);
void *memchr(void *s , int c , int n);

// Functions converted to ASM

void *memcpy(void *dst , const void *src , int n);
void *memset(void *dst , char c , int n);
int memcmp(const void *b1 , const void *b2 , int n);

#ifdef __cplusplus
}
#endif

#endif

