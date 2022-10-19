/*
 * PSn00bSDK assert macro
 * (C) 2022 spicyjpeg - MPL licensed
 */

#ifndef __ASSERT_H
#define __ASSERT_H

void _assert_abort(const char *file, int line, const char *expr);

#ifdef NDEBUG
#define assert(x)
#else
#define assert(expr) { \
	if (!(expr)) \
		_assert_abort(__FILE__, __LINE__, #expr); \
}
#endif

#endif
