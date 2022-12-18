#!/usr/bin/env python3
# Simple .VAG interleaving tool
# (C) 2021-2022 spicyjpeg - MPL licensed

import os, sys
from warnings  import warn
from struct    import Struct
from itertools import zip_longest
from argparse  import ArgumentParser, FileType

VAG_HEADER  = Struct("> 4s 4I 10x H 16s")
VAG_MAGIC   = b"VAGp"
VAGI_MAGIC  = b"VAGi"
VAG_VERSION = 0x20
BUFFER_SIZE = 0x1000
CHUNK_ALIGN = 0x800

## Helpers

def swap_endian(value, size):
	return int.from_bytes(value.to_bytes(size, "big"), "little")

def align(data, size):
	chunks = (len(data) + size - 1) // size

	return data.ljust(chunks * size, b"\x00")

def get_loop_offset(data):
	for index, flag in enumerate(data[1::16]):
		if flag & 0x01:
			return index * 16

	return len(data) - 16

## .VAG file reader

class VAGReader:
	def __init__(self, _file):
		self.file = _file
		header    = _file.read(VAG_HEADER.size)

		(
			magic, _, _,
			self.size,
			self.sample_rate,
			_, _
		) = VAG_HEADER.unpack(header)

		if magic == VAGI_MAGIC:
			raise RuntimeError(f"{_file.name} is an interleaved .VAG file (must be mono)")
		if magic != VAG_MAGIC:
			raise RuntimeError(f"{_file.name} is not a valid .VAG file")

	def read(self, chunk_size):
		for _ in range(0, self.size, chunk_size):
			chunk = self.file.read(chunk_size)

			if len(chunk) < 16:
				break
			if len(chunk) % 16:
				warn(RuntimeWarning(f"{self.file.name} is not 16-byte aligned, trimming"))
				chunk = chunk[0:(len(chunk) // 16) * 16]

			# If there already is an end flag in the chunk replace it with a
			# loop flag, otherwise add a new loop flag at the end.
			end   = get_loop_offset(chunk)
			chunk = bytearray(chunk)

			chunk[end + 1] = 0x03 # Jump to loop point + sustain
			yield chunk.ljust(chunk_size, b"\x00")

## Main

def get_args():
	parser = ArgumentParser(
		description = "Generates interleaved audio stream data from one or more .VAG files."
	)
	parser.add_argument(
		"input_file",
		nargs = "+",
		type  = FileType("rb"),
		help  = "mono input files for each channel in .VAG format"
	)
	parser.add_argument(
		"output_file",
		type = FileType("wb"),
		help = "where to output converted stream data"
	)
	parser.add_argument(
		"-b", "--buffer-size",
		type    = int,
		default = BUFFER_SIZE,
		help    = f"size of each channel buffer in each chunk (default {BUFFER_SIZE})",
		metavar = "bytes"
	)
	parser.add_argument(
		"-a", "--align",
		type    = int,
		default = CHUNK_ALIGN,
		help    = f"pad each chunk to a multiple of the given size (default {CHUNK_ALIGN})",
		metavar = "bytes"
	)
	parser.add_argument(
		"-r", "--raw",
		action = "store_true",
		help   = "do not add an interleaved .VAG header to the output file"
	)

	return parser.parse_args()

def main():
	args = get_args()
	if args.buffer_size % 16:
		raise ValueError("buffer size must be a multiple of 16 bytes")
	if args.buffer_size % args.align:
		warn(RuntimeWarning(f"buffer size should be a multiple of {args.align}"))

	input_files = tuple(map(VAGReader, args.input_file))
	size        = input_files[0].size
	sample_rate = input_files[0].sample_rate

	for vag in input_files[1:]:
		if vag.size != size:
			warn(RuntimeWarning(f"{vag.file.name} has a different file size"))
		if vag.sample_rate != sample_rate:
			warn(RuntimeWarning(f"{vag.file.name} has a different sample rate"))

	interleave = zip_longest(
		*( vag.read(args.buffer_size) for vag in input_files ),
		fillvalue = b"\x00" * args.buffer_size
	)

	with args.output_file as _file:
		if not args.raw:
			header = VAG_HEADER.pack(
				VAGI_MAGIC,
				VAG_VERSION,
				swap_endian(args.buffer_size, 4),
				size,
				sample_rate,
				swap_endian(len(input_files), 2),
				os.path.basename(_file.name).encode()[0:16]
			)

			_file.write(align(header, args.align))

		for chunks in interleave:
			data = b"".join(chunks)

			_file.write(align(data, args.align))

if __name__ == "__main__":
	main()
