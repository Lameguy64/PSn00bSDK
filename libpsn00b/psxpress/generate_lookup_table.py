#!/usr/bin/env python3
# Huffman lookup table generator script for psxpress
# (C) 2022 spicyjpeg - MPL licensed

import sys, json
from array     import array
from itertools import repeat
from argparse  import ArgumentParser, FileType

HUFFMAN_TREE = {
	"10":			0xfe00, # End of block
	"11":			(  0,  1 ),
	"01": {
		"1":		(  1,  1 ),
		"00":		(  0,  2 ),
		"01":		(  2,  1 )
	},
	"001": {
		"01":		(  0,  3 ),
		"10":		(  4,  1 ),
		"11":		(  3,  1 ),
		"00000":	( 13,  1 ),
		"00001":	(  0,  6 ),
		"00010":	( 12,  1 ),
		"00011":	( 11,  1 ),
		"00100":	(  3,  2 ),
		"00101":	(  1,  3 ),
		"00110":	(  0,  5 ),
		"00111":	( 10,  1 )
	},
	"0001": {
		"00":		(  7,  1 ),
		"01":		(  6,  1 ),
		"10":		(  1,  2 ),
		"11":		(  5,  1 )
	},
	"00001": {
		"00":		(  2,  2 ),
		"01":		(  9,  1 ),
		"10":		(  0,  4 ),
		"11":		(  8,  1 )
	},
	"0000001": {
		"000":		( 16,  1 ),
		"001":		(  5,  2 ),
		"010":		(  0,  7 ),
		"011":		(  2,  3 ),
		"100":		(  1,  4 ),
		"101":		( 15,  1 ),
		"110":		( 14,  1 ),
		"111":		(  4,  2 )
	},
	"00000001": {
		"0000":		(  0, 11 ),
		"0001":		(  8,  2 ),
		"0010":		(  4,  3 ),
		"0011":		(  0, 10 ),
		"0100":		(  2,  4 ),
		"0101":		(  7,  2 ),
		"0110":		( 21,  2 ),
		"0111":		( 20,  1 ),
		"1000":		(  0,  9 ),
		"1001":		( 19,  1 ),
		"1010":		( 18,  1 ),
		"1011":		(  1,  5 ),
		"1100":		(  3,  3 ),
		"1101":		(  0,  8 ),
		"1110":		(  6,  2 ),
		"1111":		( 17,  1 )
	},
	"000000001": {
		"0000":		( 10,  2 ),
		"0001":		(  9,  2 ),
		"0010":		(  5,  3 ),
		"0011":		(  3,  4 ),
		"0100":		(  2,  5 ),
		"0101":		(  1,  7 ),
		"0110":		(  1,  6 ),
		"0111":		(  0, 15 ),
		"1000":		(  0, 14 ),
		"1001":		(  0, 13 ),
		"1010":		(  0, 12 ),
		"1011":		( 26,  1 ),
		"1100":		( 25,  1 ),
		"1101":		( 24,  1 ),
		"1110":		( 23,  1 ),
		"1111":		( 22,  1 )
	},
	"0000000001": {
		"0000":		(  0, 31 ),
		"0001":		(  0, 30 ),
		"0010":		(  0, 29 ),
		"0011":		(  0, 28 ),
		"0100":		(  0, 27 ),
		"0101":		(  0, 26 ),
		"0110":		(  0, 25 ),
		"0111":		(  0, 24 ),
		"1000":		(  0, 23 ),
		"1001":		(  0, 22 ),
		"1010":		(  0, 21 ),
		"1011":		(  0, 20 ),
		"1100":		(  0, 19 ),
		"1101":		(  0, 18 ),
		"1110":		(  0, 17 ),
		"1111":		(  0, 16 )
	},
	"00000000001": {
		"0000":		(  0, 40 ),
		"0001":		(  0, 39 ),
		"0010":		(  0, 38 ),
		"0011":		(  0, 37 ),
		"0100":		(  0, 36 ),
		"0101":		(  0, 35 ),
		"0110":		(  0, 34 ),
		"0111":		(  0, 33 ),
		"1000":		(  0, 32 ),
		"1001":		(  1, 14 ),
		"1010":		(  1, 13 ),
		"1011":		(  1, 12 ),
		"1100":		(  1, 11 ),
		"1101":		(  1, 10 ),
		"1110":		(  1,  9 ),
		"1111":		(  1,  8 )
	},
	"000000000001": {
		"0000":		(  1, 18 ),
		"0001":		(  1, 17 ),
		"0010":		(  1, 16 ),
		"0011":		(  1, 15 ),
		"0100":		(  6,  3 ),
		"0101":		( 16,  2 ),
		"0110":		( 15,  2 ),
		"0111":		( 14,  2 ),
		"1000":		( 13,  2 ),
		"1001":		( 12,  2 ),
		"1010":		( 11,  2 ),
		"1011":		( 31,  1 ),
		"1100":		( 30,  1 ),
		"1101":		( 29,  1 ),
		"1110":		( 28,  1 ),
		"1111":		( 27,  1 )
	}
}

## Utilities

def to_int10(value):
	clamped = min(max(int(value), -0x200), 0x1ff)
	return clamped + (0 if clamped >= 0 else 0x400)

def uint32_to_lines(data, indent = "\t", columns = 6):
	for offset in range(0, len(data), columns):
		line = f"{indent}0x{data[offset]:08x}"

		for item in data[(offset + 1):(offset + columns)]:
			line += f", 0x{item:08x}"

		yield line

## Table generation

def iterate_tree(tree):
	for code, value in tree.items():
		if type(value) is dict:
			# Iterate through any subtree recursively.
			for suffix, _value in iterate_tree(value):
				yield f"{code}{suffix}", _value

		elif type(value) is tuple:
			run_length, ac = value
			yield f"{code}0", (run_length << 10) | to_int10(ac)
			yield f"{code}1", (run_length << 10) | to_int10(-ac)

		else:
			yield code, value

def generate_table(codes, table_bits, prefix_bits = 0):
	table = array("I", repeat(0, 2 ** table_bits))

	for code, value in codes:
		used_bits = len(code)
		free_bits = table_bits - (used_bits - prefix_bits)
		index     = int(code[prefix_bits:], 2) << free_bits

		# Fill out every entry in the table whose index starts with the same
		# string of bits.
		for combo in range(2 ** free_bits):
			table[index | combo] = (used_bits << 16) | value

	return table

def compress_table(table):
	values     = []
	last_value = table[0]
	run_length = 0

	for value in table[1:]:
		if value == last_value and run_length < 0x7ff:
			run_length += 1
			continue

		# The run length is stored in the top 11 bits of each value, which are
		# otherwise unused.
		values.append((run_length << 21) | last_value)
		last_value = value
		run_length = 0

	values.append((run_length << 21) | last_value)
	return array("I", values)

## Main

UNCOMPRESSED_TEMPLATE = """static const DECDCTTAB {name} = {{
	.lut = {{
{short}
	}},
	.lut00 = {{
{long}
	}}
}};
"""
COMPRESSED_TEMPLATE = """static const uint32_t {name}[{length}] = {{
{table}
}};
"""

def get_args():
	parser = ArgumentParser(
		description = "Generates a Huffman lookup table structure, to be used by DecDCTvlc2()."
	)
	parser.add_argument(
		"-c", "--compress",
		action = "store_true",
		help   = "generate run-length compressed data instead of a DECDCTTAB struct"
	)
	parser.add_argument(
		"-n", "--name",
		type    = str,
		default = "_default_huffman_table",
		help    = "set the symbol name in the generated C source",
		metavar = "file"
	)
	parser.add_argument(
		"-t", "--tree",
		type    = FileType("rt"),
		help    = "use a custom Huffman tree from the specified JSON file",
		metavar = "json_file"
	)
	parser.add_argument(
		"-o", "--output",
		type    = FileType("wt"),
		default = sys.stdout,
		help    = "where to output generated table (stdout by default)",
		metavar = "file"
	)

	return parser.parse_args()

def main():
	args = get_args()
	tree = json.load(args.tree) if args.tree else HUFFMAN_TREE

	short_codes, short_bits = [], 0
	long_codes, long_bits   = [], 0

	for pair in iterate_tree(tree):
		if (code := pair[0]).startswith("00000000"):
			long_codes.append(pair)
			long_bits = max(long_bits, len(code) - 8)
		else:
			short_codes.append(pair)
			short_bits = max(short_bits, len(code))

	short_table = generate_table(short_codes, short_bits, 0)
	long_table  = generate_table(long_codes,  long_bits,  8)

	if args.compress:
		short_table.extend(long_table)
		table = compress_table(short_table)

		source = COMPRESSED_TEMPLATE.format(
			name   = args.name,
			length = len(table),
			table  = ",\n".join(uint32_to_lines(table, "\t"))
		)
	else:
		source = UNCOMPRESSED_TEMPLATE.format(
			name  = args.name,
			short = ",\n".join(uint32_to_lines(short_table, "\t\t")),
			long  = ",\n".join(uint32_to_lines(long_table,  "\t\t"))
		)

	with args.output as _file:
		_file.write(source)

if __name__ == "__main__":
	main()
