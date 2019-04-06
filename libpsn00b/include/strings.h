/* 
 * strings.h
 *
 * PSXSDK
 */

#ifndef _STRINGS_H
#define _STRINGS_H

#include <string.h>

#define bcopy(src,dst,len)				memmove(dst,src,len)
#define bzero(ptr, len)					memset(ptr, 0, len)
#define bcmp(b1,b2,len)					memcmp(b1,b2,len)
#define index(s, c)						strchr(s, c)
#define rindex(s, c)						strrchr(s, c)

#endif
