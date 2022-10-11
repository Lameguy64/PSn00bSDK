# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	rgb24
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK 24-bit RGB display example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(rgb24 GPREL ${_sources})
#psn00bsdk_add_cd_image(rgb24_iso rgb24 iso.xml DEPENDS rgb24)

psn00bsdk_target_incbin(rgb24 PRIVATE tim_image bunpattern.tim)

install(FILES ${PROJECT_BINARY_DIR}/rgb24.exe TYPE BIN)
