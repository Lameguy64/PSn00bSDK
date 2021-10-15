#!/bin/bash
# This script generates Windows bitmaps from the SVGs in this directory. The
# bitmaps are displayed by the NSIS installers generated using CPack. Inkscape
# and ImageMagick must be installed for this script to work.

INDEXED_SIZES=(16 24 32 48)
RGB_ONLY_SIZES=(64 96 128 256)
DITHER_OPTIONS="-dither riemersma"
OUTPUT_OPTIONS="+compress -strip"

svg_to_bitmap() {
	inkscape -z -e rgb.png $1

	# There seems to be no way to tell ImageMagick to generate an 8-bit indexed
	# BMP directly, so the image has to be converted to indexed PNG first. NSIS
	# seems to reject all BMP versions other than "bmp3".
	convert $DITHER_OPTIONS -colors 256 +remap rgb.png png8:i8.png
	convert i8.png $OUTPUT_OPTIONS bmp3:$2
	rm -f rgb.png i8.png
}

# https://stackoverflow.com/a/52636645
svg_to_icon() {
	for size in ${INDEXED_SIZES[@]} ${RGB_ONLY_SIZES[@]}; do
		inkscape -z -e $size.png -w $size -h $size $1
	done

	# Windows expects some of the smaller sizes to be available in 4- and 8-bit
	# indexed color formats, as well as RGB.
	for size in ${INDEXED_SIZES[@]}; do
		convert $DITHER_OPTIONS -colors 256 +remap $size.png png8:$size-i8.png
		convert $DITHER_OPTIONS -colors 16  +remap $size.png png8:$size-i4.png
	done

	icons="${INDEXED_SIZES[@]/%/.png} ${INDEXED_SIZES[@]/%/-i8.png} ${INDEXED_SIZES[@]/%/-i4.png} ${RGB_ONLY_SIZES[@]/%/.png}"
	convert $icons $OUTPUT_OPTIONS $2
	rm -f $icons
}

rm -f *.ico *.bmp

svg_to_icon   icon.svg        icon.ico
svg_to_icon   uninstall.svg   uninstall.ico
svg_to_bitmap nsis_banner.svg nsis_banner.bmp
svg_to_bitmap nsis_header.svg nsis_header.bmp
