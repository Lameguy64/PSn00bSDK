# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	billboard
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK billboard sprite example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(billboard GPREL ${_sources})
#psn00bsdk_add_cd_image(billboard_iso billboard iso.xml DEPENDS billboard)

psn00bsdk_target_incbin(billboard PRIVATE tim_image texture64.tim)

install(FILES ${PROJECT_BINARY_DIR}/billboard.exe TYPE BIN)
