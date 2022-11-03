/*
 * PSn00bSDK assert macro and internal logging
 * (C) 2022 spicyjpeg - MPL licensed
 *
 * Note that the _sdk_log() macro is used internally by PSn00bSDK to output
 * debug messages and warnings.
 */

#ifndef __ASSERT_H
#define __ASSERT_H

#include <stdio.h>

void _assert_abort(const char *file, int line, const char *expr);

#ifdef NDEBUG

#define assert(expr)
#define _sdk_log(fmt, ...)

#else

#define assert(expr) { \
	if (!(expr)) _assert_abort(__FILE__, __LINE__, #expr); \
}

#ifdef SDK_LIBRARY_NAME
#define _sdk_log(fmt, ...) printf(SDK_LIBRARY_NAME ": " fmt, ##__VA_ARGS__)
#else
#define _sdk_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

#endif

#endif
