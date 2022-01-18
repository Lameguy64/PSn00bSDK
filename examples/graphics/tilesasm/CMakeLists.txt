# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.20)

project(
	tilesasm
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK Tiles drawing with assembly example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

configure_file(data.s.template data.s)

file(GLOB _sources *.s *.c)
psn00bsdk_add_executable(
	tilesasm STATIC
	${_sources}
	${PROJECT_BINARY_DIR}/data.s
)
#psn00bsdk_add_cd_image(tilesasm_iso tilesasm iso.xml DEPENDS tilesasm)

install(FILES ${PROJECT_BINARY_DIR}/tilesasm.exe TYPE BIN)
