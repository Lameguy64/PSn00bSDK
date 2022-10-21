/*
 * PSn00bSDK assert macro and internal logging
 * (C) 2022 spicyjpeg - MPL licensed
 *
 * Note that the _sdk_log() macro is used internally by PSn00bSDK to output
 * debug messages and warnings.
 */

#ifndef __ASSERT_H
#define __ASSERT_H

void _assert_abort(const char *file, int line, const char *expr);
void _sdk_log_inner(const char *fmt, ...);

#ifdef NDEBUG

#define assert(expr)
#define _sdk_log(fmt, ...)

#else

#define assert(expr) { \
	if (!(expr)) _assert_abort(__FILE__, __LINE__, #expr); \
}

#ifdef SDK_LIBRARY_NAME
#define _sdk_log(fmt, ...) _sdk_log_inner(SDK_LIBRARY_NAME ": " fmt, ##__VA_ARGS__)
#else
#define _sdk_log(fmt, ...) _sdk_log_inner(fmt, ##__VA_ARGS__)
#endif

#endif

#endif
