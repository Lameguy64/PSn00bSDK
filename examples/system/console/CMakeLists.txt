# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	console
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK stdio console example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(console GPREL ${_sources})
#psn00bsdk_add_cd_image(console_iso console iso.xml DEPENDS console)

psn00bsdk_target_incbin(console PRIVATE ball16c ball16c.tim)

install(FILES ${PROJECT_BINARY_DIR}/console.exe TYPE BIN)
