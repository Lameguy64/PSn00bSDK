# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	system573
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK Konami System 573 example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c *.s)
psn00bsdk_add_executable(system573 GPREL ${_sources})
psn00bsdk_add_cd_image(system573_iso system573 iso.xml DEPENDS system573)

install(
	FILES
		${PROJECT_BINARY_DIR}/system573.bin
		${PROJECT_BINARY_DIR}/system573.cue
	TYPE BIN
)
