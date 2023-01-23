/*
 * PSn00bSDK MDEC library (support code for the main VLC decompressor)
 * (C) 2022-2023 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <psxpress.h>

/* Huffman code lookup table */

#define _DC(y, c)			(((y) << 4) | (c))
#define _AC(rl, dc)			(((rl) << 10) | ((uint16_t) (dc) & 0x3ff))
#define _ACL(rl, dc, len)	(_AC(rl, dc) | ((len) << 16))

#define _DC2(y, c)			_DC(y, c), _DC(y, c)
#define _DC3(y, c)			_DC(y, c), _DC(y, c), _DC(y, c), _DC(y, c)
#define _DC4(y, c) \
	_DC(y, c), _DC(y, c), _DC(y, c), _DC(y, c), \
	_DC(y, c), _DC(y, c), _DC(y, c), _DC(y, c)
#define _AC2(rl, dc)		_AC(rl, dc), _AC(rl, -(dc))
#define _ACL2(rl, dc, len)	_ACL(rl, dc, len), _ACL(rl, -(dc), len)
#define _ACL3(rl, dc, len) \
	_ACL(rl, dc, len), _ACL(rl, dc, len), \
	_ACL(rl, -(dc), len), _ACL(rl, -(dc), len)
#define _ACL4(rl, dc, len) \
	_ACL(rl, dc, len), _ACL(rl, dc, len), _ACL(rl, dc, len), _ACL(rl, dc, len), \
	_ACL(rl, dc, len), _ACL(rl, dc, len), _ACL(rl, dc, len), _ACL(rl, dc, len), \
	_ACL(rl, -(dc), len), _ACL(rl, -(dc), len), _ACL(rl, -(dc), len), _ACL(rl, -(dc), len), \
	_ACL(rl, -(dc), len), _ACL(rl, -(dc), len), _ACL(rl, -(dc), len), _ACL(rl, -(dc), len)

// This table isn't compressed since it makes no sense to compress less than a
// kilobyte's worth of data.
static const VLC_TableV3 _default_huffman_table = {
	.ac0 = {
		// 11 x
		_AC2( 0,  1)
	},
	.ac2 = {
		// 01 0xx
		_ACL2( 0, 2, 5), _ACL2( 2, 1, 5),
		// 01 1x-
		_ACL3( 1, 1, 4)
	},
	.ac3 = {
		// 001 00xxxx
		_ACL2(13, 1, 9), _ACL2( 0, 6, 9), _ACL2(12,  1, 9), _ACL2(11, 1, 9),
		_ACL2( 3, 2, 9), _ACL2( 1, 3, 9), _ACL2( 0,  5, 9), _ACL2(10, 1, 9),
		// 001 xxx---
		_ACL4( 0, 3, 6), _ACL4( 4, 1, 6), _ACL4( 3,  1, 6)
	},
	.ac4 = {
		// 0001 xxx
		_AC2( 7,  1), _AC2( 6,  1), _AC2( 1,  2), _AC2( 5,  1)
	},
	.ac5 = {
		// 00001 xxx
		_AC2( 2,  2), _AC2( 9,  1), _AC2( 0,  4), _AC2( 8,  1)
	},
	.ac7 = {
		// 0000001 xxxx
		_AC2(16,  1), _AC2( 5,  2), _AC2( 0,  7), _AC2( 2,  3),
		_AC2( 1,  4), _AC2(15,  1), _AC2(14,  1), _AC2( 4,  2)
	},
	.ac8 = {
		// 00000001 xxxxx
		_AC2( 0, 11), _AC2( 8,  2), _AC2( 4,  3), _AC2( 0, 10),
		_AC2( 2,  4), _AC2( 7,  2), _AC2(21,  1), _AC2(20,  1),
		_AC2( 0,  9), _AC2(19,  1), _AC2(18,  1), _AC2( 1,  5),
		_AC2( 3,  3), _AC2( 0,  8), _AC2( 6,  2), _AC2(17,  1)
	},
	.ac9 = {
		// 000000001 xxxxx
		_AC2(10,  2), _AC2( 9,  2), _AC2( 5,  3), _AC2( 3,  4),
		_AC2( 2,  5), _AC2( 1,  7), _AC2( 1,  6), _AC2( 0, 15),
		_AC2( 0, 14), _AC2( 0, 13), _AC2( 0, 12), _AC2(26,  1),
		_AC2(25,  1), _AC2(24,  1), _AC2(23,  1), _AC2(22,  1)
	},
	.ac10 = {
		// 0000000001 xxxxx
		_AC2( 0, 31), _AC2( 0, 30), _AC2( 0, 29), _AC2( 0, 28),
		_AC2( 0, 27), _AC2( 0, 26), _AC2( 0, 25), _AC2( 0, 24),
		_AC2( 0, 23), _AC2( 0, 22), _AC2( 0, 21), _AC2( 0, 20),
		_AC2( 0, 19), _AC2( 0, 18), _AC2( 0, 17), _AC2( 0, 16)
	},
	.ac11 = {
		// 00000000001 xxxxx
		_AC2( 0, 40), _AC2( 0, 39), _AC2( 0, 38), _AC2( 0, 37),
		_AC2( 0, 36), _AC2( 0, 35), _AC2( 0, 34), _AC2( 0, 33),
		_AC2( 0, 32), _AC2( 1, 14), _AC2( 1, 13), _AC2( 1, 12),
		_AC2( 1, 11), _AC2( 1, 10), _AC2( 1,  9), _AC2( 1,  8)
	},
	.ac12 = {
		// 000000000001 xxxxx
		_AC2( 1, 18), _AC2( 1, 17), _AC2( 1, 16), _AC2( 1, 15),
		_AC2( 6,  3), _AC2(16,  2), _AC2(15,  2), _AC2(14,  2),
		_AC2(13,  2), _AC2(12,  2), _AC2(11,  2), _AC2(31,  1),
		_AC2(30,  1), _AC2(29,  1), _AC2(28,  1), _AC2(27,  1)
	},
	.dc = {
		// 00-----
		_DC4(1, 0), _DC4(1, 0), _DC4(1, 0), _DC4(1, 0),
		// 01-----
		_DC4(2, 1), _DC4(2, 1), _DC4(2, 1), _DC4(2, 1),
		// 100----
		_DC4(0, 2), _DC4(0, 2),
		// 101----
		_DC4(3, 2), _DC4(3, 2),
		// 110----
		_DC4(4, 3), _DC4(4, 3),
		// 1110---
		_DC4(5, 4),
		// 11110--
		_DC3(6, 5),
		// 111110-
		_DC2(7, 6),
		// 1111110
		_DC(8, 7),
		// 1111111(0)
		_DC(0, 8)
	},
	.dc_len = {
		_DC(3, 2), _DC(2, 2), _DC(2, 2), _DC(3, 3),
		_DC(3, 4), _DC(4, 5), _DC(5, 6), _DC(6, 7),
		_DC(7, 8)
	}
};

/* Internal globals */

// Note that DecDCTvlc() and DecDCTvlc2() do *not* share the same variables.
static VLC_Context	_default_context;
static size_t		_max_buffer_size = 0;

const VLC_TableV3 *_vlc_huffman_table = &_default_huffman_table;

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

void DecDCTvlcCopyTableV2(VLC_TableV2 *addr) {
	if (addr) {
		_vlc_huffman_table = (const VLC_TableV3 *) addr;
		memcpy(addr, &_default_huffman_table, sizeof(VLC_TableV2));
	} else {
		_vlc_huffman_table = &_default_huffman_table;
	}
}

void DecDCTvlcCopyTableV3(VLC_TableV3 *addr) {
	if (addr) {
		_vlc_huffman_table = (const VLC_TableV3 *) addr;
		memcpy(addr, &_default_huffman_table, sizeof(VLC_TableV3));
	} else {
		_vlc_huffman_table = &_default_huffman_table;
	}
}
