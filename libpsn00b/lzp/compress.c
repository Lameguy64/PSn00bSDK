// Based on ilia muraviev's CRUSH compressor program which falls under public domain

#include <string.h>
#if LZP_USE_MALLOC == TRUE
#include <stdlib.h>
#endif

#include "lzconfig.h"
#include "bit.h"
#include "lzp.h"


// Internal structure for hash table allocation sizes
#if LZP_NO_COMPRESS == FALSE

struct {
	short WindowSize;	// Window size (17 - 23)
	short Hash1Size;	// Hash 1 table size (10 - 21)
	short Hash2Size;	// Hash 2 table size (12 - 24)
} lzHashParam = {
	LZP_WINDOW_SIZE,
    LZP_HASH1_SIZE,
    LZP_HASH2_SIZE
};

#endif


// Defines and macros for lz77 compression/decompression (don't touch)
#define W_BITS		lzHashParam.WindowSize
#define HASH1_BITS	lzHashParam.Hash1Size
#define HASH2_BITS	lzHashParam.Hash2Size

#define W_SIZE		(1<<W_BITS)
#define W_MASK		(W_SIZE-1)
#define SLOT_BITS	4
#define NUM_SLOTS	(1<<SLOT_BITS)

#define A_BITS		2 // 1 xx
#define B_BITS		2 // 01 xx
#define C_BITS		2 // 001 xx
#define D_BITS		3 // 0001 xxx
#define E_BITS		5 // 00001 xxxxx
#define F_BITS		9 // 00000 xxxxxxxxx
#define A			(1<<A_BITS)
#define B			((1<<B_BITS)+A)
#define C			((1<<C_BITS)+B)
#define D			((1<<D_BITS)+C)
#define E			((1<<E_BITS)+D)
#define F			((1<<F_BITS)+E)
#define MIN_MATCH	3
#define MAX_MATCH	((F-1)+MIN_MATCH)

#define BUF_SIZE	(1<<26)
#define TOO_FAR		(1<<16)

#define HASH1_LEN	MIN_MATCH
#define HASH2_LEN	(MIN_MATCH+1)
#define HASH1_SIZE	(1<<HASH1_BITS)
#define HASH2_SIZE	(1<<HASH2_BITS)
#define HASH1_MASK	(HASH1_SIZE-1)
#define HASH2_MASK	(HASH2_SIZE-1)
#define HASH1_SHIFT	((HASH1_BITS+(HASH1_LEN-1))/HASH1_LEN)
#define HASH2_SHIFT	((HASH2_BITS+(HASH2_LEN-1))/HASH2_LEN)


// LZ77
//

#if LZP_NO_COMPRESS == FALSE

int update_hash1(int h, int c) {

	return(((h<<HASH1_SHIFT)+c)&HASH1_MASK);

}

int update_hash2(int h, int c) {

	return(((h<<HASH2_SHIFT)+c)&HASH2_MASK);

}

int get_min(int a, int b) {

	return(a<b?a:b);

}

int get_max(int a, int b) {

	return(a>b?a:b);

}

int get_penalty(int a, int b) {

	int p=0;

	while(a > b) {
		a >>= 3;
		++p;
	}

	return(p);

}

int lzCompress(void* outBuff, void* inBuff, int inSize, int level) {

	#if LZP_USE_MALLOC == FALSE
	int head[HASH1_SIZE+HASH2_SIZE];
	int prev[W_SIZE];
	#else
	int* head = malloc(4*(HASH1_SIZE+HASH2_SIZE));
	int* prev = malloc(4*W_SIZE);
	#endif


	int max_chain[] = {4, 256, 1<<12};

	int i,s;
	int h1=0;
	int h2=0;
	int p=0;

	int len;
	int offset;

	int max_match;
	int limit;

	int chain_len;
	int next_p;
	int max_lazy;
	int log;


	inPtr = (unsigned char*)inBuff;
	outPtr = (unsigned char*)outBuff;
	outBytes = 0;


	for (i=0; i<HASH1_SIZE+HASH2_SIZE; ++i)
		head[i] = -1;

	for (i=0; i<HASH1_LEN; ++i)
		h1=update_hash1(h1, inPtr[i]);

	for (i=0; i<HASH2_LEN; ++i)
		h2=update_hash2(h2, inPtr[i]);

	init_bits();

	// Put window size value so that the compressed data will be independent of the compression settings
	put_bits(5, lzHashParam.WindowSize);

	while(p < inSize) {

		len = MIN_MATCH-1;
		offset = W_SIZE;

		max_match = get_min(MAX_MATCH, inSize-p);
		limit = get_max(p-W_SIZE, 0);

		if (head[h1] >= limit) {

			s = head[h1];

			if (inPtr[s] == inPtr[p]) {

				i = 0;

				while(++i < max_match) {
					if (inPtr[s+i] != inPtr[p+i])
						break;
				}

				if (i > len) {
					len = i;
					offset = p-s;
				}

			}

		}

		if (len < MAX_MATCH) {

			chain_len = max_chain[level];
			s = head[h2+HASH1_SIZE];

			while((chain_len-- != 0) && (s >= limit)) {

				if ((inPtr[s+len] == inPtr[p+len]) && (inPtr[s] == inPtr[p])) {

					i = 0;

					while(++i < max_match) {
						if (inPtr[s+i] != inPtr[p+i])
							break;
					}

					if (i > len+get_penalty((p-s)>>4, offset)) {
						len = i;
						offset = p-s;
					}

					if (i == max_match)
						break;

				}

				s=prev[s&W_MASK];

			}

		}

		if ((len == MIN_MATCH) && (offset > TOO_FAR))
			len=0;

		if ((level >= 2) && (len >= MIN_MATCH) && (len < max_match)) {

			next_p = p+1;
			max_lazy = get_min(len+4, max_match);

			chain_len = max_chain[level];
			s = head[update_hash2(h2, inPtr[next_p+(HASH2_LEN-1)])+HASH1_SIZE];

			while((chain_len-- != 0) && (s >= limit)) {

				if ((inPtr[s+len] == inPtr[next_p+len]) && (inPtr[s] == inPtr[next_p])) {

					i = 0;

					while(++i < max_lazy) {
						if (inPtr[s+i] != inPtr[next_p+i])
							break;
					}

					if (i > len+get_penalty(next_p-s, offset)) {
						len = 0;
						break;
					}

					if (i == max_lazy)
						break;

				}

				s = prev[s&W_MASK];

			}

		}


		if (len >= MIN_MATCH) { // Match

			put_bits(1, 1);

			i = len-MIN_MATCH;

			if (i < A) {
				put_bits(1, 1); // 1
				put_bits(A_BITS, i);
			} else if (i < B) {
				put_bits(2, 1<<1); // 01
				put_bits(B_BITS, i-A);
			} else if (i < C) {
				put_bits(3, 1<<2); // 001
				put_bits(C_BITS, i-B);
			} else if (i < D) {
				put_bits(4, 1<<3); // 0001
				put_bits(D_BITS, i-C);
			} else if (i < E) {
				put_bits(5, 1<<4); // 00001
				put_bits(E_BITS, i-D);
			} else {
				put_bits(5, 0); // 00000
				put_bits(F_BITS, i-E);
			}

			--offset;
			log = W_BITS-NUM_SLOTS;

			while(offset >= (2<<log))
				++log;

			put_bits(SLOT_BITS, log-(W_BITS-NUM_SLOTS));

			if (log>(W_BITS-NUM_SLOTS))
				put_bits(log, offset-(1<<log));
			else
				put_bits(W_BITS-(NUM_SLOTS-1), offset);

		} else { // Literal

			len = 1;
			put_bits(9, inPtr[p]<<1); // 0 xxxxxxxx

		}

		while(len-- != 0) { // Insert new strings

			head[h1] = p;
			prev[p&W_MASK] = head[h2+HASH1_SIZE];
			head[h2+HASH1_SIZE] = p;

			++p;

			h1 = update_hash1(h1, inPtr[p+(HASH1_LEN-1)]);
			h2 = update_hash2(h2, inPtr[p+(HASH2_LEN-1)]);

		}

	}

	flush_bits();

	#if LZP_USE_MALLOC == TRUE
	free(head);
	free(prev);
	#endif

	return(outBytes);

}

void lzSetHashSizes(int window, int hash1, int hash2) {

	lzHashParam.WindowSize = window;
	lzHashParam.Hash1Size = hash1;
	lzHashParam.Hash2Size = hash2;

}

void lzResetHashSizes() {

	lzHashParam.WindowSize	= LZP_WINDOW_SIZE;
	lzHashParam.Hash1Size	= LZP_HASH1_SIZE;
	lzHashParam.Hash2Size	= LZP_HASH2_SIZE;

}

#endif // LZP_NO_COMPRESS

int lzDecompress(void* outBuff, void* inBuff, int inSize) {

	int p=0;
	int len;
	int log;
	int s;
	int windowSize;

    inPtr = (unsigned char*)inBuff;
    outPtr = (unsigned char*)outBuff;
    inBytes = 0;
    outBytes = 0;

	init_bits();

	// Get window size value
	windowSize = get_bits(5);

	while(inBytes < inSize) {

		if (get_bits(1)) {

			if (get_bits(1))
				len = get_bits(A_BITS);
			else if (get_bits(1))
				len = get_bits(B_BITS)+A;
			else if (get_bits(1))
				len = get_bits(C_BITS)+B;
			else if (get_bits(1))
				len = get_bits(D_BITS)+C;
			else if (get_bits(1))
				len = get_bits(E_BITS)+D;
			else
				len = get_bits(F_BITS)+E;

			log = get_bits(SLOT_BITS)+(windowSize-NUM_SLOTS);

			s =~ (log>(windowSize-NUM_SLOTS) ? get_bits(log)+(1<<log) : get_bits(windowSize-(NUM_SLOTS-1)))+p;

			if (s < 0)
				return(LZP_ERR_DECOMPRESS);

			outPtr[p++] = outPtr[s++];
			outPtr[p++] = outPtr[s++];
			outPtr[p++] = outPtr[s++];

			while(len-- != 0)
				outPtr[p++] = outPtr[s++];

		} else {

			outPtr[p++] = get_bits(8);

		}

	}

	return(p);

}

int lzDecompressLen(void* outBuff, int outSize, void* inBuff, int inSize) {

	int p=0;
	int len;
	int log;
	int s;
	int windowSize;

    inPtr = (unsigned char*)inBuff;
    outPtr = (unsigned char*)outBuff;
    inBytes = 0;
    outBytes = 0;

	init_bits();

	// Get window size value
	windowSize = get_bits(5);

	while(inBytes < inSize) {

		if (get_bits(1)) {

			if (get_bits(1))
				len = get_bits(A_BITS);
			else if (get_bits(1))
				len = get_bits(B_BITS)+A;
			else if (get_bits(1))
				len = get_bits(C_BITS)+B;
			else if (get_bits(1))
				len = get_bits(D_BITS)+C;
			else if (get_bits(1))
				len = get_bits(E_BITS)+D;
			else
				len = get_bits(F_BITS)+E;

			log = get_bits(SLOT_BITS)+(windowSize-NUM_SLOTS);

			s =~ (log>(windowSize-NUM_SLOTS) ? get_bits(log)+(1<<log) : get_bits(windowSize-(NUM_SLOTS-1)))+p;

			if (s < 0)
				return(LZP_ERR_DECOMPRESS);

			outPtr[p++] = outPtr[s++];
			if (p >= outSize)
				break;

			outPtr[p++] = outPtr[s++];
			if (p >= outSize)
				break;

			outPtr[p++] = outPtr[s++];
			if (p >= outSize)
				break;

			while(len-- != 0) {

				outPtr[p++] = outPtr[s++];
				if (p >= outSize)
					break;

			}

			if (p >= outSize)
				break;

		} else {

			outPtr[p++] = get_bits(8);

		}

		if (p >= outSize)
			break;

	}

	return(p);

}
