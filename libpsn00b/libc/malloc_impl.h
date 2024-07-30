#pragma once

#ifndef _H_SDK_ALLOC_IMPL_
#define _H_SDK_ALLOC_IMPL

// Custom allocator overriding default implementation
#define SDK_ALLOC_IMPL_CUSTOM 0
// Allocated block first-fit
#define SDK_ALLOC_IMPL_AFF 1
// Two-level segregated fit
#define SDK_ALLOC_IMPL_TLSF 2

#ifndef SDK_ALLOC_IMPL
#define SDK_ALLOC_IMPL SDK_ALLOC_IMPL_AFF
#endif

#endif // _H_SDK_ALLOC_IMPL
