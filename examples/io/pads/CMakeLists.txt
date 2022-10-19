# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	pads
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK controller polling example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c *.s)
psn00bsdk_add_executable(pads GPREL ${_sources})
#psn00bsdk_add_cd_image(pads_iso pads iso.xml DEPENDS pads)

install(FILES ${PROJECT_BINARY_DIR}/pads.exe TYPE BIN)
