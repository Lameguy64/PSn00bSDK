#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>

#ifndef NULL
#define NULL 0
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

#define fputc(__char, __fd) 	putc(__char, __fd)
#define fgetc(__char, __fd) 	getc(__char, __fd)

// Console TTY
extern void gets(char *__s);
extern void puts(const char *__s);
extern int getchar(void);
extern void putchar(int __c);

// The following functions do not use the BIOS
int vsnprintf(char *string, unsigned int size, const char *fmt, va_list ap);
int vsprintf(char *string, const char *fmt, va_list ap);
int sprintf(char *string, const char *fmt, ...);
int snprintf(char *string, unsigned int size, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif	// _STDIO_H