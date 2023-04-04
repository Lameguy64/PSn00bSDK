/*
 * PSn00bSDK standard library
 * (C) 2019-2022 PSXSDK authors, Lameguy64, spicyjpeg - MPL licensed
 */

#pragma once

#include <string.h>

/* Compatibility macros (this header is useless) */

#define bcopy(src, dst, len)	memmove(dst, src, len)
#define bzero(ptr, len)			memset(ptr, 0, len)
#define bcmp(b1, b2, len)		memcmp(b1, b2, len)
#define index(s, c)				strchr(s, c)
#define rindex(s, c)			strrchr(s, c)
