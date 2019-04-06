#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>

#ifndef NULL
#define NULL (void*)0
#endif

// BIOS seek modes
#ifndef SEEK_SET
#define SEEK_SET	0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR	1
#endif
#ifndef SEEK_END
#define SEEK_END	2		/* warning: reportedly buggy */
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// The following functions use the BIOS
extern void printf (const char *__format, ...);

extern int getc(int __fd);
extern int putc(int __char, int __fd);
extern void putchar(int __c);

// The following functions do not use the BIOS
int vsnprintf(char *string, unsigned int size, char *fmt, va_list ap);
int vsprintf(char *string, char *fmt, va_list ap);
int sprintf(char *string, char *fmt, ...);
int snprintf(char *string, unsigned int size, char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif	// _STDIO_H