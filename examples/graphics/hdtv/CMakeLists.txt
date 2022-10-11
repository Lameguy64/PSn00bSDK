# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	hdtv
	LANGUAGES    C
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK HDTV widescreen example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(hdtv GPREL ${_sources})
#psn00bsdk_add_cd_image(hdtv_iso hdtv iso.xml DEPENDS hdtv)

install(FILES ${PROJECT_BINARY_DIR}/hdtv.exe TYPE BIN)
