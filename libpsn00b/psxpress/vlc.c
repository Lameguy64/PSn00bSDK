/*
 * PSn00bSDK MDEC library (support code for the main VLC decompressor)
 * (C) 2022 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <psxpress.h>

/* Huffman code lookup table */

#define _val1(rl, dc)		(((rl) << 10) | ((uint16_t) (dc) & 0x3ff))
#define _val2(rl, dc, len)	(_val1(rl, dc) | (len << 16))

#define _pair(rl, dc)		_val1(rl, dc), _val1(rl, -(dc))
#define _pair2(rl, dc, len)	_val2(rl, dc, len), _val2(rl, -(dc), len)
#define _pair3(rl, dc, len) \
	_val2(rl, dc, len), _val2(rl, dc, len), \
	_val2(rl, -(dc), len), _val2(rl, -(dc), len)
#define _pair4(rl, dc, len) \
	_val2(rl, dc, len), _val2(rl, dc, len), _val2(rl, dc, len), _val2(rl, dc, len), \
	_val2(rl, dc, len), _val2(rl, dc, len), _val2(rl, dc, len), _val2(rl, dc, len), \
	_val2(rl, -(dc), len), _val2(rl, -(dc), len), _val2(rl, -(dc), len), _val2(rl, -(dc), len), \
	_val2(rl, -(dc), len), _val2(rl, -(dc), len), _val2(rl, -(dc), len), _val2(rl, -(dc), len)

// This table isn't compressed since it makes no sense to compress less than a
// kilobyte's worth of data.
static const DECDCTTAB _default_huffman_table = {
	.lut0 = {
		// 11 x
		_pair( 0,  1)
	},
	.lut2 = {
		// 01 0xx
		_pair2( 0,  2, 5), _pair2( 2,  1, 5),
		// 01 1x-
		_pair3( 1,  1, 4)
	},
	.lut3 = {
		// 001 00xxxx
		_pair2(13,  1, 9), _pair2( 0,  6, 9), _pair2(12,  1, 9), _pair2(11,  1, 9),
		_pair2( 3,  2, 9), _pair2( 1,  3, 9), _pair2( 0,  5, 9), _pair2(10,  1, 9),
		// 001 xxx---
		_pair4( 0,  3, 6), _pair4( 4,  1, 6), _pair4( 3,  1, 6)
	},
	.lut4 = {
		// 0001 xxx
		_pair( 7,  1), _pair( 6,  1), _pair( 1,  2), _pair( 5,  1)
	},
	.lut5 = {
		// 00001 xxx
		_pair( 2,  2), _pair( 9,  1), _pair( 0,  4), _pair( 8,  1)
	},
	.lut7 = {
		// 0000001 xxxx
		_pair(16,  1), _pair( 5,  2), _pair( 0,  7), _pair( 2,  3),
		_pair( 1,  4), _pair(15,  1), _pair(14,  1), _pair( 4,  2)
	},
	.lut8 = {
		// 00000001 xxxxx
		_pair( 0, 11), _pair( 8,  2), _pair( 4,  3), _pair( 0, 10),
		_pair( 2,  4), _pair( 7,  2), _pair(21,  1), _pair(20,  1),
		_pair( 0,  9), _pair(19,  1), _pair(18,  1), _pair( 1,  5),
		_pair( 3,  3), _pair( 0,  8), _pair( 6,  2), _pair(17,  1)
	},
	.lut9 = {
		// 000000001 xxxxx
		_pair(10,  2), _pair( 9,  2), _pair( 5,  3), _pair( 3,  4),
		_pair( 2,  5), _pair( 1,  7), _pair( 1,  6), _pair( 0, 15),
		_pair( 0, 14), _pair( 0, 13), _pair( 0, 12), _pair(26,  1),
		_pair(25,  1), _pair(24,  1), _pair(23,  1), _pair(22,  1)
	},
	.lut10 = {
		// 0000000001 xxxxx
		_pair( 0, 31), _pair( 0, 30), _pair( 0, 29), _pair( 0, 28),
		_pair( 0, 27), _pair( 0, 26), _pair( 0, 25), _pair( 0, 24),
		_pair( 0, 23), _pair( 0, 22), _pair( 0, 21), _pair( 0, 20),
		_pair( 0, 19), _pair( 0, 18), _pair( 0, 17), _pair( 0, 16)
	},
	.lut11 = {
		// 00000000001 xxxxx
		_pair( 0, 40), _pair( 0, 39), _pair( 0, 38), _pair( 0, 37),
		_pair( 0, 36), _pair( 0, 35), _pair( 0, 34), _pair( 0, 33),
		_pair( 0, 32), _pair( 1, 14), _pair( 1, 13), _pair( 1, 12),
		_pair( 1, 11), _pair( 1, 10), _pair( 1,  9), _pair( 1,  8)
	},
	.lut12 = {
		// 000000000001 xxxxx
		_pair( 1, 18), _pair( 1, 17), _pair( 1, 16), _pair( 1, 15),
		_pair( 6,  3), _pair(16,  2), _pair(15,  2), _pair(14,  2),
		_pair(13,  2), _pair(12,  2), _pair(11,  2), _pair(31,  1),
		_pair(30,  1), _pair(29,  1), _pair(28,  1), _pair(27,  1)
	}
};

/* Internal globals */

// Note that DecDCTvlc() and DecDCTvlc2() do *not* share the same variables.
static VLC_Context	_default_context;
static size_t		_max_buffer_size = 0;

const DECDCTTAB		*_vlc_huffman_table = &_default_huffman_table;

/* Stateful VLC decoder API (for Sony SDK compatibility) */

int DecDCTvlc(const uint32_t *bs, uint32_t *buf) {
	if (bs)
		return DecDCTvlcStart(&_default_context, buf, _max_buffer_size, bs);
	else
		return DecDCTvlcContinue(&_default_context, buf, _max_buffer_size);
}

size_t DecDCTvlcSize(size_t size) {
	size_t old_size  = _max_buffer_size;
	_max_buffer_size = size;

	return old_size;
}

/* Lookup table relocation API */

void DecDCTvlcCopyTable(DECDCTTAB *addr) {
	if (addr) {
		_vlc_huffman_table = addr;
		memcpy(addr, &_default_huffman_table, sizeof(DECDCTTAB));
	} else {
		_vlc_huffman_table = &_default_huffman_table;
	}
}
