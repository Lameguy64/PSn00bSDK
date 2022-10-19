# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	tty
	LANGUAGES    C
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK stdio terminal example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(tty GPREL ${_sources})
#psn00bsdk_add_cd_image(tty_iso tty iso.xml DEPENDS tty)

install(FILES ${PROJECT_BINARY_DIR}/tty.exe TYPE BIN)
