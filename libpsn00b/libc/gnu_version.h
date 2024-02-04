#pragma once

#ifndef _H_GNU_VERSION_
#define _H_GNU_VERSION_

#if defined(__GNUC__) \
    && __GNUC__ >= 10    \
    && (__GNUC__ > 10 || (__GNUC__ >= 0 && __GNUC_MINOR__ >= 0)) \
    && defined(__GNUC_PATCHLEVEL__)
#define gnu_version_10
#endif

#endif // _H_GNU_VERSION_
