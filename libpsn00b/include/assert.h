/*
 * PSn00bSDK assert macro and internal logging
 * (C) 2022-2023 spicyjpeg - MPL licensed
 *
 * Note that the _sdk_log() macro is used internally by PSn00bSDK to output
 * debug messages and warnings.
 */

#ifndef __ASSERT_H
#define __ASSERT_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void _assert_abort(const char *file, int line, const char *expr);

#ifdef __cplusplus
}
#endif

#ifdef NDEBUG

#define assert(expr)
#define _sdk_log(fmt, ...)

#else

#define assert(expr) \
	((expr) ? ((void) 0) : _assert_abort(__FILE__, __LINE__, #expr))

#ifdef SDK_LIBRARY_NAME
#define _sdk_log(fmt, ...) \
	printf(SDK_LIBRARY_NAME ": " fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define _sdk_log(fmt, ...) \
	printf(fmt __VA_OPT__(,) __VA_ARGS__)
#endif

#endif

#endif
