/*
 * PSn00bSDK standard library
 * (C) 2019-2023 Lameguy64, spicyjpeg - MPL licensed
 */

#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* String I/O API (provided by BIOS) */

int printf(const char *fmt, ...);
char *gets(char *str);
void puts(const char *str);
int getchar(void);
void putchar(int ch);

/* String formatting API (built-in) */

int vsnprintf(char *string, unsigned int size, const char *fmt, va_list ap);
int vsprintf(char *string, const char *fmt, va_list ap);
int sprintf(char *string, const char *fmt, ...);
int snprintf(char *string, unsigned int size, const char *fmt, ...);

int vsscanf(const char *str, const char *format, va_list ap);
int sscanf(const char *str, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
