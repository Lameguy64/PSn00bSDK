#include "tlsf.h"

tlsf_t tlsf;

__attribute__((weak)) void InitHeap(void *addr, size_t size) {
    tlsf = tlsf_create_with_pool(addr, size);
}

__attribute__((weak)) void TrackHeapUsage(ptrdiff_t alloc_incr) {

}

__attribute__((weak)) void GetHeapUsage(HeapUsage *usage) {

}

/* Memory allocator */

__attribute__((weak)) void *malloc(size_t size) {
    return tlsf_malloc(tlsf_t, size);
}

__attribute__((weak)) void *calloc(size_t num, size_t size) {
    return tlsf_malloc(tlsf, num * size);
}

__attribute__((weak)) void *realloc(void *ptr, size_t size) {
    return tlsf_realloc(tlsf, ptr, size);
}

__attribute__((weak)) void free(void *ptr) {
    tlsf_free(tlsf, ptr);
}