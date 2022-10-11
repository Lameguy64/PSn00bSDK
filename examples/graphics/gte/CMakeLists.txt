# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	gte
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK GTE 3D cube example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(gte GPREL ${_sources})
#psn00bsdk_add_cd_image(gte_iso gte iso.xml DEPENDS gte)

psn00bsdk_target_incbin(gte PRIVATE tim_texture texture.tim)

install(FILES ${PROJECT_BINARY_DIR}/gte.exe TYPE BIN)
