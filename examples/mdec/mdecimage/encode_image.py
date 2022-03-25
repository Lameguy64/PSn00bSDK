#!/usr/bin/env python3
# Simple MDEC image encoder (requires PIL/Pillow and NumPy to be installed)
# (C) 2022 spicyjpeg - MPL licensed

import math
from warnings import warn
from argparse import ArgumentParser, FileType

import numpy
from PIL import Image

LUMA_SCALE   = 8
CHROMA_SCALE = 16

## Tables

ZIGZAG_TABLE = numpy.array((
	 0,  1,  5,  6, 14, 15, 27, 28,
	 2,  4,  7, 13, 16, 26, 29, 42,
	 3,  8, 12, 17, 25, 30, 41, 43,
	 9, 11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63
), numpy.uint8).argsort()

# The default luma and chroma quantization table is based on the MPEG-1
# quantization table, with the only difference being the first value (2 instead
# of 8).
QUANT_TABLE = numpy.array((
	 2, 16, 19, 22, 26, 27, 29, 34,
	16, 16, 22, 24, 27, 29, 34, 37,
	19, 22, 26, 27, 29, 34, 34, 38,
	22, 22, 26, 27, 29, 34, 37, 40,
	22, 26, 27, 29, 32, 35, 40, 48,
	26, 27, 29, 32, 35, 40, 48, 58,
	26, 27, 29, 34, 38, 46, 56, 69,
	27, 29, 35, 38, 46, 56, 69, 83
), numpy.uint8).reshape(( 8, 8 ))

S = [ math.cos((i or 4) / 16 * math.pi) / 2 for i in range(8) ]

DCT_MATRIX = numpy.array((
	 S[0],  S[0],  S[0],  S[0],  S[0],  S[0],  S[0],  S[0],
	 S[1],  S[3],  S[5],  S[7], -S[7], -S[5], -S[3], -S[1],
	 S[2],  S[6], -S[6], -S[2], -S[2], -S[6],  S[6],  S[2],
	 S[3], -S[7], -S[1], -S[5],  S[5],  S[1],  S[7], -S[3],
	 S[4], -S[4], -S[4],  S[4],  S[4], -S[4], -S[4],  S[4],
	 S[5], -S[1],  S[7],  S[3], -S[3], -S[7],  S[1], -S[5],
	 S[6], -S[2],  S[2], -S[6], -S[6],  S[2], -S[2],  S[6],
	 S[7], -S[5],  S[3], -S[1],  S[1], -S[3],  S[5], -S[7]
), numpy.float32).reshape(( 8, 8 ))

## Helpers

def to_int10(value):
	clamped = min(max(int(value), -0x200), 0x1ff)

	return clamped + (0 if clamped >= 0 else 0x400)

def rgb_to_ycbcr_planar(image):
	scaled  = image.astype(numpy.float32) / 255.0
	r, g, b = scaled.transpose(( 2, 0, 1 ))

	# https://en.wikipedia.org/wiki/YCbCr#ITU-R_BT.601_conversion
	y  =  16 + r *  65.481 + g * 128.553 + b *  24.966
	cb = 128 - r *  37.797 - g *  74.203 + b * 112.000
	cr = 128 + r * 112.000 - g *  93.786 - b *  18.214

	return y, cb, cr

## Block encoder

def encode_block(buffer, block, scale):
	# Perform discrete cosine transform on the block, divide the coefficients by
	# the quantization table and reorder them in zigzag order.
	_block = block.astype(numpy.float32) - 128.0
	coeffs = (DCT_MATRIX @ _block @ DCT_MATRIX.T) / QUANT_TABLE
	coeffs = coeffs.reshape(( 64, ))[ZIGZAG_TABLE]

	buffer[0] = (scale << 10) | to_int10(round(coeffs[0]))
	offset    = 1

	# Divide the AC coefficients by the given quantization scale and encode them
	# as run-length pairs by counting how many zeroes there are between each
	# non-zero value.
	ac_values  = coeffs[1:] * 8.0 / scale
	encoded    = []
	run_length = 0

	for ac in ac_values.round().astype(numpy.int32):
		if ac:
			buffer[offset] = (run_length << 10) | to_int10(ac)
			offset += 1

			run_length  = 0
		else:
			run_length += 1

	# Flush any remaining zeroes.
	if run_length:
		buffer[offset] = (run_length - 1) << 10
		offset += 1

	# Add 1 or 2 end-of-block codes depending on whether the number of 16-bit
	# values output so far is odd or even. Some emulators will break if blocks
	# are not 32-bit aligned.
	buffer[offset] = 0xfe00
	offset += 1
	if offset % 2:
		buffer[offset] = 0xfe00
		offset += 1

	return offset

def encode_macroblock(buffer, block, y_scale, c_scale):
	#y, cb, cr = rgb_to_ycbcr_planar(block)
	y, cb, cr = block.transpose(( 2, 0, 1 ))
	offset    = 0

	# Split the macroblock into 6 monochrome 8x8 blocks (Cr, Cb at half
	# resolution + Y1-4). The MDEC uses 4:2:0 chroma subsampling.
	# TODO: use bilinear sampling instead of nearest-neighbor for chroma
	offset += encode_block(buffer[offset:], cr[0:16:2, 0:16:2], c_scale)
	offset += encode_block(buffer[offset:], cb[0:16:2, 0:16:2], c_scale)
	offset += encode_block(buffer[offset:], y[0: 8, 0: 8], y_scale)
	offset += encode_block(buffer[offset:], y[0: 8, 8:16], y_scale)
	offset += encode_block(buffer[offset:], y[8:16, 0: 8], y_scale)
	offset += encode_block(buffer[offset:], y[8:16, 8:16], y_scale)

	return offset

## Main

def get_args():
	parser = ArgumentParser(
		description = "Generates uncompressed MDEC bitstream data from an image."
	)
	parser.add_argument(
		"input_file",
		type = FileType("rb"),
		help = "input image file"
	)
	parser.add_argument(
		"-o", "--output",
		type    = FileType("wb"),
		default = "image.bin",
		help    = "where to output converted image data (image.bin by default)",
		metavar = "file"
	)
	parser.add_argument(
		"-m", "--monochrome",
		action = "store_true",
		help   = "encode image as monochrome (8x8 blocks) instead of color (16x16 macroblocks)"
	)
	parser.add_argument(
		"-y", "--luma",
		type    = int,
		default = LUMA_SCALE,
		help    = f"quantization scale for luma/monochrome blocks (0-63, default {LUMA_SCALE})",
		metavar = "scale"
	)
	parser.add_argument(
		"-c", "--chroma",
		type    = int,
		default = CHROMA_SCALE,
		help    = f"quantization scale for chroma blocks (0-63, default {CHROMA_SCALE})",
		metavar = "scale"
	)

	return parser.parse_args()

def main():
	args = get_args()
	if args.luma < 0 or args.luma > 63:
		raise ValueError("luma quantization scale must be in 0-63 range")
	if args.chroma < 0 or args.chroma > 63:
		raise ValueError("chroma quantization scale must be in 0-63 range")

	image = Image.open(args.input_file, "r")
	data  = numpy.array(image.convert("YCbCr"), numpy.uint8)
	size  = 8 if args.monochrome else 16

	if image.width % size:
		warn(RuntimeWarning(f"image width is not a multiple of {size}, trimming"))
	if image.height % size:
		warn(RuntimeWarning(f"image height is not a multiple of {size}, trimming"))

	# Preallocate 1 MB for the converted image data (faster than expanding an
	# array dynamically -- this script is too slow already).
	buffer = numpy.empty(0x80000, numpy.uint16)
	offset = 0

	# Split the image into 8x8 or 16x16 blocks and encode them in column-major
	# order.
	for x in range(0, image.width, size):
		for y in range(0, image.height, size):
			block = data[y:(y + size), x:(x + size)]

			if args.monochrome:
				offset += encode_block(buffer[offset:], block[:, :, 0], args.luma)
			else:
				offset += encode_macroblock(buffer[offset:], block, args.luma, args.chroma)

	# Pad the generated data to the size of a DMA chunk (32x 32-bit words or
	# 128 bytes).
	length = (offset + 63) & 0xffffffc0
	buffer[offset:length] = 0xfe00

	if length > (0xffff * 2):
		warn(RuntimeWarning("image is too large to be decoded with a single DecDCTin() call"))

	with args.output as _file:
		buffer[0:length].tofile(_file)

if __name__ == "__main__":
	main()
