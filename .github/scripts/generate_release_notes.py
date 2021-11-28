#!/usr/bin/env python3
# PSn00bSDK release notes generator
# (C) 2021 spicyjpeg - MPL licensed

import sys, re
from time     import gmtime, strptime
from argparse import ArgumentParser, FileType

## Helpers

VERSION_REGEX = re.compile(r"^(?:refs\/tags\/)?(?:v|ver|version|release)? *(.*)")

def parse_date(date):
	return strptime(date.strip(), "%Y-%m-%d")

def normalize_version(version):
	match = VERSION_REGEX.match(version.lower())
	if not match:
		raise ValueError(f"invalid version string: {version}")

	return match.group(1)

## Changelog parser

BLOCK_REGEX   = re.compile(r"^#{2,}[ \t]*([0-9]{4}-[0-9]{2}-[0-9]{2})(?:[:\- \t]+(.+?))?$", re.MULTILINE)
AUTHOR_REGEX  = re.compile(r"^([A-Za-z0-9_].*?)[ \t]*:.*?$", re.MULTILINE)
FIRST_VERSION = "initial"

def parse_authors(block):
	# [ _crap, author, body, author, body, ... ]
	items = AUTHOR_REGEX.split(block.strip())

	if items[0].strip():
		raise RuntimeError("a block has changes listed with no author specified")

	authors = {}
	for i in range(1, len(items), 2):
		name, body = items[i:i + 2]

		name = name.strip()
		body = body.strip()
		if name not in authors:
			authors[name] = ""

		authors[name] += body

	return authors

def parse_blocks(changelog):
	# [ _crap, date, version, body, date, version, body, ... ]
	items = BLOCK_REGEX.split(changelog.strip())

	#if items[0].strip():
		#raise RuntimeError("the changelog doesn't start with a valid block")

	# Iterate over all blocks from bottom to top (i.e. oldest first).
	last_version = FIRST_VERSION

	for i in range(len(items), 1, -3):
		date, version, body = items[i - 3:i]

		# If no version is present in the header, assume it's the same as the
		# previous block's version.
		if version:
			version      = normalize_version(version)
			last_version = version
		else:
			version = last_version

		yield parse_date(date), version, parse_authors(body)

## Release notes generation

VERSION_TEMPLATE = """New in version **{version}** (contributed by {authors}):

{changes}

"""
NOTES_TEMPLATE = """{notes}

-------------------------------------------------------
_These notes have been generated automatically._
_See the changelog or commit history for more details._
"""

NO_VERSIONS_TEMPLATE = "No information available about this release."
AUTHOR_LINK_TEMPLATE = "**{0}**"
#AUTHOR_LINK_TEMPLATE = "[{0}](https://github.com/{0})"

def generate_notes(versions):
	notes = ""

	for version, ( authors, changes ) in versions.items():
		_authors = list(set(authors))
		_authors.sort()
		_authors = map(AUTHOR_LINK_TEMPLATE.format, _authors)

		notes += VERSION_TEMPLATE.format(
			version = version,
			authors = ", ".join(_authors),
			changes = "\n\n".join(changes)
		)

	if not notes:
		notes = NO_VERSIONS_TEMPLATE

	return NOTES_TEMPLATE.format(notes = notes.strip())

## Main

def get_args():
	parser = ArgumentParser(
		description = "Generates and outputs release notes from a Markdown changelog file.",
		add_help    = False
	)

	tools_group = parser.add_argument_group("Tools")
	tools_group.add_argument(
		"-h", "--help",
		action = "help",
		help   = "Show this help message and exits"
	)

	files_group = parser.add_argument_group("Files")
	files_group.add_argument(
		"changelog",
		type    = FileType("rt"),
		help    = "Markdown changelog file to parse"
	)
	files_group.add_argument(
		"-o", "--output",
		type    = FileType("wt"),
		default = sys.stdout,
		help    = "Where to output release notes (stdout by default)",
		metavar = "file"
	)

	filter_group = parser.add_argument_group("Filters")
	filter_group.add_argument(
		"-v", "--version",
		action  = "append",
		type    = str,
		help    = "Ignore all changes not belonging to a version (can be specified multiple times)",
		metavar = "name"
	)
	filter_group.add_argument(
		"-f", "--from-date",
		type    = parse_date,
		default = parse_date("2000-01-01"),
		help    = "Ignore all changes before date",
		metavar = "yyyy-mm-dd"
	)
	filter_group.add_argument(
		"-t", "--to-date",
		type    = parse_date,
		default = gmtime(),
		help    = "Ignore all changes after date",
		metavar = "yyyy-mm-dd"
	)

	return parser.parse_args()

def main():
	args         = get_args()
	version_list = list(map(normalize_version, args.version or []))

	with args.changelog as _file:
		changelog = _file.read()

	# Iterate over all blocks in the changelog and sort them by version,
	# merging all changes and authors for each version.
	versions = {}

	for date, version, authors in parse_blocks(changelog):
		# Apply version and date filters.
		if version_list and (version not in version_list):
			continue
		if date < args.from_date or date > args.to_date:
			continue

		if version not in versions:
			versions[version] = [], []

		_authors, _changes = versions[version]
		_authors.extend(authors.keys())
		_changes.extend(authors.values())

	notes = generate_notes(versions)

	with args.output as _file:
		_file.write(notes)

if __name__ == "__main__":
	main()
