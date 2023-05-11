# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	cdstream
	LANGUAGES    C
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK SPU CD audio streaming example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(cdstream GPREL ${_sources})
psn00bsdk_add_cd_image(
	cdstream_iso cdstream iso.xml
	DEPENDS cdstream system.cnf stream.vag
)

install(
	FILES
		${PROJECT_BINARY_DIR}/cdstream.bin
		${PROJECT_BINARY_DIR}/cdstream.cue
	TYPE BIN
)
