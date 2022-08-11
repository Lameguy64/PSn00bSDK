# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.20)

project(
	spustream
	LANGUAGES    C
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK SPU custom streaming example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

# TODO: add rules to actually generate a valid STREAM.BIN file
file(GLOB _sources *.c)
psn00bsdk_add_executable(spustream STATIC ${_sources})
psn00bsdk_add_cd_image(spustream_iso spustream iso.xml DEPENDS spustream)

install(
	FILES
		${PROJECT_BINARY_DIR}/spustream.bin
		${PROJECT_BINARY_DIR}/spustream.cue
	TYPE BIN
)
