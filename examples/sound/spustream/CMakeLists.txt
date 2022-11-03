# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	spustream
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK SPU audio streaming example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(spustream GPREL ${_sources})
#psn00bsdk_add_cd_image(spustream_iso spustream iso.xml DEPENDS spustream)

psn00bsdk_target_incbin(spustream PRIVATE stream_data stream.vag)

install(FILES ${PROJECT_BINARY_DIR}/spustream.exe TYPE BIN)
