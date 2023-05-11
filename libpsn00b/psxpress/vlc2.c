/*
 * PSn00bSDK MDEC library (alternate VLC decompressor and support code)
 * (C) 2022 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <stddef.h>
#include <psxpress.h>

#define _min(x, y) (((x) < (y)) ? (x) : (y))

/* Huffman code lookup table */

#define TABLE_LENGTH 226

// This table is run-length compressed, with the number of repetitions of each
// value stored in the upper 11 bits which would be otherwise unused. It is
// decompressed at runtime by DecDCTvlcBuild().
static const uint32_t _compressed_table[TABLE_LENGTH] = {
	0x03e00000, 0x000d000b, 0x000d03f5, 0x000d2002, 0x000d23fe, 0x000d1003,
	0x000d13fd, 0x000d000a, 0x000d03f6, 0x000d0804, 0x000d0bfc, 0x000d1c02,
	0x000d1ffe, 0x000d5402, 0x000d57fe, 0x000d5001, 0x000d53ff, 0x000d0009,
	0x000d03f7, 0x000d4c01, 0x000d4fff, 0x000d4801, 0x000d4bff, 0x000d0405,
	0x000d07fb, 0x000d0c03, 0x000d0ffd, 0x000d0008, 0x000d03f8, 0x000d1802,
	0x000d1bfe, 0x000d4401, 0x000d47ff, 0x006b4001, 0x006b43ff, 0x006b1402,
	0x006b17fe, 0x006b0007, 0x006b03f9, 0x006b0803, 0x006b0bfd, 0x006b0404,
	0x006b07fc, 0x006b3c01, 0x006b3fff, 0x006b3801, 0x006b3bff, 0x006b1002,
	0x006b13fe, 0x0fe00000, 0x03e80802, 0x03e80bfe, 0x03e82401, 0x03e827ff,
	0x03e80004, 0x03e803fc, 0x03e82001, 0x03e823ff, 0x07e71c01, 0x07e71fff,
	0x07e71801, 0x07e71bff, 0x07e70402, 0x07e707fe, 0x07e71401, 0x07e717ff,
	0x01e93401, 0x01e937ff, 0x01e90006, 0x01e903fa, 0x01e93001, 0x01e933ff,
	0x01e92c01, 0x01e92fff, 0x01e90c02, 0x01e90ffe, 0x01e90403, 0x01e907fd,
	0x01e90005, 0x01e903fb, 0x01e92801, 0x01e92bff, 0x0fe60003, 0x0fe603fd,
	0x0fe61001, 0x0fe613ff, 0x0fe60c01, 0x0fe60fff, 0x1fe50002, 0x1fe503fe,
	0x1fe50801, 0x1fe50bff, 0x3fe40401, 0x3fe407ff, 0xffe2fe00, 0x7fe30001,
	0x7fe303ff, 0x03e00000, 0x00110412, 0x001107ee, 0x00110411, 0x001107ef,
	0x00110410, 0x001107f0, 0x0011040f, 0x001107f1, 0x00111803, 0x00111bfd,
	0x00114002, 0x001143fe, 0x00113c02, 0x00113ffe, 0x00113802, 0x00113bfe,
	0x00113402, 0x001137fe, 0x00113002, 0x001133fe, 0x00112c02, 0x00112ffe,
	0x00117c01, 0x00117fff, 0x00117801, 0x00117bff, 0x00117401, 0x001177ff,
	0x00117001, 0x001173ff, 0x00116c01, 0x00116fff, 0x00300028, 0x003003d8,
	0x00300027, 0x003003d9, 0x00300026, 0x003003da, 0x00300025, 0x003003db,
	0x00300024, 0x003003dc, 0x00300023, 0x003003dd, 0x00300022, 0x003003de,
	0x00300021, 0x003003df, 0x00300020, 0x003003e0, 0x0030040e, 0x003007f2,
	0x0030040d, 0x003007f3, 0x0030040c, 0x003007f4, 0x0030040b, 0x003007f5,
	0x0030040a, 0x003007f6, 0x00300409, 0x003007f7, 0x00300408, 0x003007f8,
	0x006f001f, 0x006f03e1, 0x006f001e, 0x006f03e2, 0x006f001d, 0x006f03e3,
	0x006f001c, 0x006f03e4, 0x006f001b, 0x006f03e5, 0x006f001a, 0x006f03e6,
	0x006f0019, 0x006f03e7, 0x006f0018, 0x006f03e8, 0x006f0017, 0x006f03e9,
	0x006f0016, 0x006f03ea, 0x006f0015, 0x006f03eb, 0x006f0014, 0x006f03ec,
	0x006f0013, 0x006f03ed, 0x006f0012, 0x006f03ee, 0x006f0011, 0x006f03ef,
	0x006f0010, 0x006f03f0, 0x00ee2802, 0x00ee2bfe, 0x00ee2402, 0x00ee27fe,
	0x00ee1403, 0x00ee17fd, 0x00ee0c04, 0x00ee0ffc, 0x00ee0805, 0x00ee0bfb,
	0x00ee0407, 0x00ee07f9, 0x00ee0406, 0x00ee07fa, 0x00ee000f, 0x00ee03f1,
	0x00ee000e, 0x00ee03f2, 0x00ee000d, 0x00ee03f3, 0x00ee000c, 0x00ee03f4,
	0x00ee6801, 0x00ee6bff, 0x00ee6401, 0x00ee67ff, 0x00ee6001, 0x00ee63ff,
	0x00ee5c01, 0x00ee5fff, 0x00ee5801, 0x00ee5bff
};

/* Internal globals */

// Note that DecDCTvlc() and DecDCTvlc2() do *not* share the same variables.
static VLC_Context	_default_context;
static size_t		_max_buffer_size = 0;

const DECDCTTAB *_vlc_huffman_table2 = 0;

/* VLC decoder */

#define _get_bits_unsigned(length)	(((uint32_t) window) >> (32 - (length)))
#define _get_bits_signed(length)	(((int32_t)  window) >> (32 - (length)))
#define _advance_window(num) \
	window    <<= (num); \
	bit_offset -= (num);

int __attribute__((optimize(3))) DecDCTvlcContinue2(
	VLC_Context *ctx, uint32_t *buf, size_t max_size
) {
	const uint32_t	*input		= ctx->input;
	uint32_t		window		= ctx->window;
	uint32_t		next_window	= ctx->next_window;
	uint32_t		remaining	= ctx->remaining;
	int				is_v3		= ctx->is_v3;
	int				bit_offset	= ctx->bit_offset;
	int				block_index	= ctx->block_index;
	int				coeff_index	= ctx->coeff_index;
	uint16_t		quant_scale	= ctx->quant_scale;
	int16_t			last_y		= ctx->last_y;
	int16_t			last_cr		= ctx->last_cr;
	int16_t			last_cb		= ctx->last_cb;

	//if (!_vlc_huffman_table2)
		//return -1;
	if (!max_size)
		max_size = 0x7fffffff;

	// Write the length of the data that will be decoded to first 4 bytes of
	// the output buffer, which will be then parsed by DecDCTin().
	max_size   = _min((max_size - 1) * 2, remaining);
	remaining -= max_size;

	*buf = 0x38000000 | (max_size / 2);
	uint16_t *output = (uint16_t *) &buf[1];

	for (; max_size; max_size--) {
		uint32_t value;

		if (coeff_index) {
			// Parse the next AC coefficient. Most codes are decompressed via
			// the lookup table, however some need special handling.
			if ((window >> 30) == 0b10) {
				// Prefix 10 marks the end of a block.
				*output = 0xfe00;
				_advance_window(2);

				coeff_index = -1;
				block_index++;
				if (block_index > 5)
					block_index = 0;
			} else if ((window >> 26) == 0b000001) {
				// Prefix 000001 is an escape code followed by a full 16-bit
				// MDEC value.
				*output = (uint16_t) _get_bits_unsigned(22);
				_advance_window(22);
			} else if (window >> 24) {
				// The first lookup table is for codes that do not start with
				// 00000000.
				value = _vlc_huffman_table2->ac[_get_bits_unsigned(13)];
				_advance_window(value >> 16);
				*output = (uint16_t) value;
			} else {
				// If the code starts with 00000000, use the second lookup
				// table.
				value = _vlc_huffman_table2->ac00[_get_bits_unsigned(17)];
				_advance_window(value >> 16);
				*output = (uint16_t) value;
			}
		} else {
			// Parse the DC (first) coefficient for this block.
			if (is_v3) {
				// This implementation does not support version 3.
				return -1;
			} else {
				value = _get_bits_unsigned(10);
				if (value == 0x1ff)
					break;

				*output = value | quant_scale;
				_advance_window(10);
			}
		}

		output++;
		coeff_index++;

		// Update the bitstream window. For whatever reason Sony's DecDCTvlc()
		// implementation inefficiently reads the input stream 16 bits at a
		// time and processes each 16-bit word starting from the the MSB, so an
		// endianness conversion is necessary to preserve bit order when
		// reading 32 bits at a time. Also note that the PS1 CPU is not capable
		// of shifting by >=31 bits - it will shift by (N % 32) bits instead!
		if (bit_offset < 0) {
			window      = next_window << (-bit_offset);
			bit_offset += 32;
			next_window = (*input << 16) | (*input >> 16);
			input++;
		};
		window |= next_window >> bit_offset;
	}

	// Pad the buffer with end-of-block codes if necessary.
	for (; max_size; max_size--)
		*(output++) = 0xfe00;

	if (!remaining)
		return 0;

	ctx->input			= input;
	ctx->window			= window;
	ctx->next_window	= next_window;
	ctx->remaining		= remaining;
	ctx->bit_offset		= bit_offset;
	ctx->block_index	= block_index;
	ctx->coeff_index	= coeff_index;
	ctx->last_y			= last_y;
	ctx->last_cr		= last_cr;
	ctx->last_cb		= last_cb;
	return 1;
}

int DecDCTvlcStart2(
	VLC_Context *ctx, uint32_t *buf, size_t max_size, const uint32_t *bs
) {
	const BS_Header *header = (const BS_Header *) bs;
	const uint32_t  *input  = (const uint32_t *) &header[1];

	if (!_vlc_huffman_table2)
		return -1;
	if (header->version > 3)
		return -1;

	ctx->input			= &input[2];
	ctx->window			= (input[0] << 16) | (input[0] >> 16);
	ctx->next_window	= (input[1] << 16) | (input[1] >> 16);
	ctx->remaining		= (header->mdec0_header & 0xffff) * 2;
	ctx->is_v3			= (header->version >= 3);
	ctx->bit_offset		= 32;
	ctx->block_index	= 0;
	ctx->coeff_index	= 0;
	ctx->quant_scale	= (header->quant_scale & 63) << 10;
	ctx->last_y			= 0;
	ctx->last_cr		= 0;
	ctx->last_cb		= 0;

	return DecDCTvlcContinue2(ctx, buf, max_size);
}

/* Stateful VLC decoder API (for Sony SDK compatibility) */

int DecDCTvlc2(const uint32_t *bs, uint32_t *buf, DECDCTTAB *table) {
	if (table)
		_vlc_huffman_table2 = table;

	if (bs)
		return DecDCTvlcStart2(&_default_context, buf, _max_buffer_size, bs);
	else
		return DecDCTvlcContinue2(&_default_context, buf, _max_buffer_size);
}

size_t DecDCTvlcSize2(size_t size) {
	size_t old_size  = _max_buffer_size;
	_max_buffer_size = size;

	return old_size;
}

/* Lookup table decompressor */

void DecDCTvlcBuild(DECDCTTAB *table) {
	uint32_t *output    = (uint32_t *) table;
	_vlc_huffman_table2 = table;

	for (int i = 0; i < TABLE_LENGTH; i++) {
		uint32_t value = _compressed_table[i] & 0x001fffff;

		for (int j = (_compressed_table[i] >> 21); j >= 0; j--)
			*(output++) = value;
	}
}
