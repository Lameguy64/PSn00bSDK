#ifndef _LZP_BIT_H
#define _LZP_BIT_H

extern unsigned char* inPtr;
extern int inBytes;
extern unsigned char* outPtr;
extern int outBytes;

extern int bit_buf;
extern int bit_count;

#ifdef __cplusplus
extern "C" {
#endif

void init_bits();
void put_bits(int n, int x);
void flush_bits();
int get_bits(int n);

#ifdef __cplusplus
}
#endif


#endif // _LZP_BIT_H
