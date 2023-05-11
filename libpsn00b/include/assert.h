/*
 * PSn00bSDK assert macro and internal logging
 * (C) 2022-2023 spicyjpeg - MPL licensed
 *
 * The _sdk_*() macros are used internally by PSn00bSDK to output messages when
 * building in debug mode.
 */

#pragma once

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
#define _sdk_assert(expr, fmt, ...)
#define _sdk_validate_args_void(expr)
#define _sdk_validate_args(expr, ret)

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

#define _sdk_assert(expr, ret, fmt, ...) \
	if (!(expr)) { \
		_sdk_log(fmt, __VA_ARGS__); \
		return ret; \
	}
#define _sdk_validate_args_void(expr) \
	if (!(expr)) { \
		_sdk_log("invalid args to %s() (%s)\n", __func__, #expr); \
		return; \
	}
#define _sdk_validate_args(expr, ret) \
	if (!(expr)) { \
		_sdk_log("invalid args to %s() (%s)\n", __func__, #expr); \
		return ret; \
	}

#endif
