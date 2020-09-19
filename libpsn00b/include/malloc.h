#ifndef _MALLOC_H
#define _MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned int *GetBSSend();
void InitHeap(unsigned int *addr, int size);
int SetHeapSize(int size);
void *malloc(int size);
void free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif // _MALLOC_H