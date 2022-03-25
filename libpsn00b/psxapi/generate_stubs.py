#!/usr/bin/env python3
# PSn00bSDK BIOS stub generator
# (C) 2022 spicyjpeg - MPL licensed

import os, sys, json
from argparse import ArgumentParser, FileType

## Listing section generation

SECTION_TEMPLATE = """## {title} ({count})

{body}"""
FUNCTION_TEMPLATE = """.section .text.{name}
.global {name}
.type {name}, @function
{name}:
	li $t2, 0x{address:02x}
	jr $t2
	li $t1, 0x{id:02x}

"""
SYSCALL_TEMPLATE = """.section .text.{name}
.global {name}
.type {name}, @function
{name}:
	li      $a0, 0x{id:02x}
	syscall 0
	jr      $ra
	nop

"""

STUB_TYPES = {
	"a":       ( 0xa0, FUNCTION_TEMPLATE, "A0 table functions" ),
	"b":       ( 0xb0, FUNCTION_TEMPLATE, "B0 table functions" ),
	"c":       ( 0xc0, FUNCTION_TEMPLATE, "C0 table functions" ),
	"syscall": ( 0x00, SYSCALL_TEMPLATE,  "Syscalls" )
}

def generate_section(_type, entries):
	address, template, title = STUB_TYPES[_type]
	body = ""

	for entry in entries:
		if entry.get("comment", None):
			body += f"# {entry['comment'].strip()}\n"

		body += template.format(
			name    = entry["name"].strip(),
			address = address,
			id      = entry["id"]
		)

	return SECTION_TEMPLATE.format(
		title = title,
		count = len(entries),
		body  = body
	)

## Main

HEADER_TEMPLATE = """# PSn00bSDK BIOS API stubs
# (C) 2022 spicyjpeg - MPL licensed

# This file has been generated automatically. Each function is placed in its
# own section to allow the linker to strip unused functions.

.set noreorder

"""

def get_args():
	parser = ArgumentParser(
		description = "Generates MIPS assembly listings of BIOS API stubs."
	)
	parser.add_argument(
		"json_list",
		type    = FileType("rt"),
		help    = "path to JSON list of stubs to generate"
	)
	parser.add_argument(
		"-o", "--output",
		type    = str,
		default = os.curdir,
		help    = "where to save generated files (current directory by default)",
		metavar = "directory"
	)

	return parser.parse_args()

def main():
	args = get_args()

	with args.json_list as _file:
		all_entries = json.load(_file)

	# { file: { type: [ entry, ... ], ... }, ... }
	files = {}

	# Sort all functions by the file they are going to be put in, and by the
	# type within the file.
	for entry in all_entries:
		_type = entry["type"].strip().lower()
		_file = entry["file"].strip()

		if type(entry["id"]) is str:
			entry["id"] = int(entry["id"], 0)

		if _file not in files:
			files[_file] = { _type: [] for _type in STUB_TYPES.keys() }

		files[_file][_type].append(entry)

	for path, types in files.items():
		full_path = os.path.normpath(os.path.join(args.output, path))
		listing   = HEADER_TEMPLATE

		for _type, entries in types.items():
			if not entries:
				continue

			entries.sort(key = lambda entry: entry["id"])
			listing += generate_section(_type, entries)

		with open(full_path, "wt", newline = "\n") as _file:
			_file.write(listing)

if __name__ == "__main__":
	main()
