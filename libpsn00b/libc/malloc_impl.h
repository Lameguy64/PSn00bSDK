#pragma once

#ifndef _H_MALLOC_IMPL_
#define _H_MALLOC_IMPL

// Custom allocator overriding default implementation
#define MALLOC_IMPL_CUSTOM 0
// Allocated block first-fit
#define MALLOC_IMPL_AFF 1
// Two-level segregated fit
#define MALLOC_IMPL_TLSF 2

#ifndef MALLOC_IMPL
#define MALLOC_IMPL MALLOC_IMPL_CUSTOM
#endif

#if MALLOC_IMPL == MALLOC_IMPL_CUSTOM
#warning Malloc implementation: CUSTOM
#elif MALLOC_IMPL == MALLOC_IMPL_AFF
#warning Malloc implementation: AFF
#elif MALLOC_IMPL == MALLOC_IMPL_TLSF
#warning Malloc implementation: TLSF
#endif

#endif // _H_MALLOC_IMPL
