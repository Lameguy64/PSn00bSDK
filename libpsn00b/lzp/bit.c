#include "bit.h"

// Bit I/O
//

unsigned char* inPtr = 0;
int inBytes = 0;
unsigned char* outPtr = 0;
int outBytes = 0;

int bit_buf;
int bit_count;

void init_bits() {

	bit_count = bit_buf=0;

}

void put_bits(int n, int x) {

	bit_buf |= x<<bit_count;
	bit_count += n;

	while(bit_count >= 8) {

		*outPtr = bit_buf;
		outPtr++;
        outBytes++;

		bit_buf >>= 8;
		bit_count -= 8;

	}

}

void flush_bits() {

	put_bits(7, 0);
	bit_count = bit_buf = 0;

}

int get_bits(int n) {

	int x;

	while(bit_count < n) {

		bit_buf |= *inPtr<<bit_count;
		inPtr++;
		inBytes++;

		bit_count += 8;

	}

	x = bit_buf&((1<<n)-1);
	bit_buf >>= n;
	bit_count -= n;

	return(x);

}
