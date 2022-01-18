#!/usr/bin/env python3
# Simple .VAG to STREAM.BIN interleaving tool
# (C) 2021 spicyjpeg - MPL licensed

import sys
from warnings  import warn
from struct    import Struct
from itertools import zip_longest
from argparse  import ArgumentParser, FileType

VAG_HEADER  = Struct("> 4s I 4x 2I 12x 16s")
VAG_MAGIC   = b"VAGp"
SAMPLE_RATE = 44100
BUFFER_SIZE = 26624 # (26624 / 16 * 28) / 44100 = 1.05 seconds
ALIGN_SIZE  = 2048

## Helpers

def align(data, size):
	chunks = (len(data) + size - 1) // size

	return data.ljust(chunks * size, b"\x00")

def set_loop_flag(data):
	last_block    = bytearray(data[-16:])
	last_block[1] = 0x03 # Jump to loop point + sustain

	return data[:-16] + last_block

## .VAG file reader

def read_vag(_file, chunk_size):
	with _file:
		header = _file.read(VAG_HEADER.size)
		(
			magic,
			version,
			size,
			sample_rate,
			name
		) = VAG_HEADER.unpack(header)

		#if magic != VAG_MAGIC:
			#raise RuntimeError(f"{_file.name} is not a valid .VAG file")
		if sample_rate != SAMPLE_RATE:
			warn(RuntimeWarning(f"{_file.name} sample rate is not {SAMPLE_RATE} Hz"))

		for i in range(0, size, chunk_size):
			chunk   = _file.read(chunk_size)

			if len(chunk) % 16:
				warn(RuntimeWarning(f"{_file.name} is not 16-byte aligned, trimming"))
				chunk = chunk[0:len(chunk) // 16 * 16]

			chunk = set_loop_flag(chunk)

			yield chunk.ljust(chunk_size, b"\x00")

## Main

def get_args():
	parser = ArgumentParser(
		description = "Generates interleaved stream data from one or more .VAG files."
	)
	parser.add_argument(
		"input_file",
		nargs = "+",
		type  = FileType("rb"),
		help  = f"mono input files for each channel (must be {SAMPLE_RATE} Hz .VAG)"
	)
	parser.add_argument(
		"-o", "--output",
		type    = FileType("wb"),
		default = "stream.bin",
		help    = "where to output converted stream data (stream.bin by default)",
		metavar = "file"
	)
	parser.add_argument(
		"-b", "--buffer-size",
		type    = int,
		default = BUFFER_SIZE,
		help    = f"size of each interleaved chunk (one per channel, default {BUFFER_SIZE})",
		metavar = "bytes"
	)
	parser.add_argument(
		"-a", "--align",
		type    = int,
		default = ALIGN_SIZE,
		help    = f"align each group of chunks to N bytes (default {ALIGN_SIZE})",
		metavar = "bytes"
	)

	return parser.parse_args()

def main():
	args = get_args()
	if args.buffer_size % 16:
		raise ValueError("buffer size must be a multiple of 16 bytes")

	interleave = zip_longest(
		*( read_vag(_file, args.buffer_size) for _file in args.input_file ),
		fillvalue = b"\x00" * args.buffer_size
	)

	with args.output as _file:
		for chunks in interleave:
			data = b"".join(chunks)

			_file.write(align(data, args.align))

if __name__ == "__main__":
	main()
