# Allocator Implementation

Supplying a custom allocator implementation can be done via the
`CUSTOM` value supplied to the `PSN00BSDK_SDK_ALLOCATOR` cmake
parameter.

## Functions

Using this variant requires the user to implement several calls
that are bound as weak functions (via `__attribute__((weak))`)
within the libc implementation. You are required to implement
the following to satisfy the `stdlib.h` header:

```c
void InitHeap(void* addr, size_t size);
void TrackHeapUsage(ptrdiff_t alloc_incr);
void GetHeapUsage(HeapUsage* usage);

void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t size);
void free(void* ptr);
```

For reference implementations, see the [aff.c](../libpsn00b/libc/aff.c) and [tlsf.c](../libpsn00b/libc/tlsf.c)
sources within the SDK.

## Placeholder Details

Internally, the default placeholder implementations that are bound when
you provide the `CUSTOM` variant via cmake all contain logging statements
that will cause the application to terminate immediately upon invocation
of any allocator function calls. For example:

```c
void* malloc(size_t size) {
	_sdk_log("[ERROR] Unimplemented custom allocator handle: void* malloc(size_t)\n");
	abort();
	return NULL;
}
```
