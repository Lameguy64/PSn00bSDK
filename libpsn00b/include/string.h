/*
 * PSn00bSDK standard library
 * (C) 2019-2023 PSXSDK authors, Lameguy64, spicyjpeg - MPL licensed
 */

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *memset(void *dest, int ch, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
void *memccpy(void *dest, const void *src, int ch, size_t count);
void *memmove(void *dest, const void *src, size_t count);
int memcmp(const void *lhs, const void *rhs, size_t count);
void *memchr(const void *ptr, int ch, size_t count);

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t count);
int strcmp(const char *lhs, const char *rhs);
int strncmp(const char *lhs, const char *rhs, size_t count);
char *strchr(const char *str, int ch);
char *strrchr(const char *str, int ch);
char *strpbrk(const char *str, const char *breakset);
char *strstr(const char *str, const char *substr);

size_t strlen(const char *str);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t count);
char *strdup(const char *str);
char *strndup(const char *str, size_t count);

char *strtok(char *str, const char *delim);

#ifdef __cplusplus
}
#endif
