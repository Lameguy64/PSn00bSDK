# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	tilesasm
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK Tiles drawing with assembly example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.s *.c)
psn00bsdk_add_executable(tilesasm GPREL ${_sources})
#psn00bsdk_add_cd_image(tilesasm_iso tilesasm iso.xml DEPENDS tilesasm)

psn00bsdk_target_incbin(tilesasm PRIVATE tim_tileset tiles_256.tim)

install(FILES ${PROJECT_BINARY_DIR}/tilesasm.exe TYPE BIN)
