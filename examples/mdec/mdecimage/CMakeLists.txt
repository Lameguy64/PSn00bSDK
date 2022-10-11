# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	mdecimage
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK MDEC static image example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(mdecimage GPREL ${_sources})
#psn00bsdk_add_cd_image(mdecimage_iso mdecimage iso.xml DEPENDS mdecimage)

psn00bsdk_target_incbin(mdecimage PRIVATE mdec_image image.bin)

install(FILES ${PROJECT_BINARY_DIR}/mdecimage.exe TYPE BIN)
