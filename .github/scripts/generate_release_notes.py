#!/usr/bin/env python3
# PSn00bSDK release notes generator
# (C) 2021 spicyjpeg - MPL licensed

import sys, re
from time     import gmtime, strptime, struct_time
from argparse import ArgumentParser, FileType

## Helpers

VERSION_REGEX   = re.compile(r"^(?:refs\/tags\/)?(?:v|ver|version|release)? *(.*)")
TEXT_WRAP_REGEX = re.compile(r"(?<!\n)[ \t]*?\n[ \t]*(?!\n)", re.MULTILINE)

def parse_date(date):
	if isinstance(date, struct_time):
		return date

	return strptime(date.strip(), "%Y-%m-%d")

def normalize_version(version):
	return VERSION_REGEX.match(version.lower()).group(1)

def unwrap_text(text):
	return TEXT_WRAP_REGEX.sub(" ", text.strip())

def deduplicate_authors(authors):
	_authors = []
	folded   = []

	for name in authors:
		if (fold := name.lower()) in folded:
			continue

		_authors.append(name)
		folded.append(fold)

	_authors.sort()
	return _authors

## Changelog parser

BLOCK_REGEX  = re.compile(r"^#{2,}[ \t]*([0-9]{4}-[0-9]{2}-[0-9]{2})(?:[:\- \t]+(.*?))?$", re.MULTILINE)
AUTHOR_REGEX = re.compile(r"^([A-Za-z0-9_].*?)[ \t]*:.*?$", re.MULTILINE)

def parse_authors(block):
	# [ _crap, author, body, author, body, ... ]
	items = AUTHOR_REGEX.split(block.strip())

	if items[0].strip():
		raise RuntimeError("a block has changes listed with no author specified")

	authors = {}
	for i in range(1, len(items), 2):
		name, body = items[i:i + 2]
		if (name := name.strip()) not in authors:
			authors[name] = ""

		authors[name] += body.strip()

	return authors

def parse_blocks(changelog):
	# [ _crap, date, version, body, date, version, body, ... ]
	items = BLOCK_REGEX.split(changelog.strip())

	# Iterate over all blocks from bottom to top (i.e. oldest first) and group
	# them by the version number of the block they precede.
	blocks = []

	for i in range(len(items), 1, -3):
		date, version, body = items[i - 3:i]

		blocks.append(( parse_date(date), parse_authors(body) ))

		if version:
			yield normalize_version(version), tuple(blocks)
			blocks = []

## Release notes generation

VERSION_TEMPLATE = """New in version **{version}** (contributed by {authors}):

{changes}

"""
NOTES_TEMPLATE = """{notes}

------------------------------------------------------------------------------------------------------
_These notes have been generated automatically. See the changelog or commit history for more details._
"""

NO_VERSIONS_TEMPLATE = "No information available about this release."
AUTHOR_LINK_TEMPLATE = "**{0}**"
#AUTHOR_LINK_TEMPLATE = "[{0}](https://github.com/{0})"

def generate_notes(versions):
	notes = ""

	for version, ( authors, changes ) in versions.items():
		if not changes:
			continue

		_authors = deduplicate_authors(authors)
		_authors = map(AUTHOR_LINK_TEMPLATE.format, _authors)

		notes += VERSION_TEMPLATE.format(
			version = version,
			authors = ", ".join(_authors),
			changes = "\n\n".join(changes)
		)

	if not notes:
		notes = NO_VERSIONS_TEMPLATE

	return NOTES_TEMPLATE.format(notes = unwrap_text(notes))

## Main

def get_args():
	parser = ArgumentParser(
		description = "Generates and outputs release notes from a Markdown changelog file."
	)
	parser.add_argument(
		"changelog",
		type    = FileType("rt"),
		help    = "Markdown changelog file to parse"
	)
	parser.add_argument(
		"-o", "--output",
		type    = FileType("wt"),
		default = sys.stdout,
		help    = "where to output release notes (stdout by default)",
		metavar = "file"
	)
	parser.add_argument(
		"-v", "--version",
		action  = "append",
		type    = str,
		help    = "ignore all changes not belonging to a version (can be specified multiple times)",
		metavar = "name"
	)
	parser.add_argument(
		"-f", "--from-date",
		type    = parse_date,
		default = parse_date("2000-01-01"),
		help    = "ignore all changes before date",
		metavar = "yyyy-mm-dd"
	)
	parser.add_argument(
		"-t", "--to-date",
		type    = parse_date,
		default = gmtime(),
		help    = "ignore all changes after date",
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

	for version, blocks in parse_blocks(changelog):
		if version_list and (version not in version_list):
			continue

		if version not in versions:
			versions[version] = [], []

		_authors, _changes = versions[version]

		for date, authors in blocks:
			if date < args.from_date or date > args.to_date:
				continue

			_authors.extend(authors.keys())
			_changes.extend(authors.values())

	notes = generate_notes(versions)

	with args.output as _file:
		_file.write(notes)

if __name__ == "__main__":
	main()
