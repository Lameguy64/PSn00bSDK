# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{PSN00BSDK_LIBS})
	set(CMAKE_TOOLCHAIN_FILE $ENV{PSN00BSDK_LIBS}/cmake/sdk.cmake)
endif()

project(
	cdxa
	LANGUAGES    C
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK CD-XA playback example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

# TODO: add rules to actually generate a valid .XA file
file(GLOB _sources *.c)
psn00bsdk_add_executable(cdxa STATIC ${_sources})
#psn00bsdk_add_cd_image(cdxa_iso cdxa iso.xml DEPENDS cdxa)

install(
	FILES
		#${PROJECT_BINARY_DIR}/cdxa.bin
		#${PROJECT_BINARY_DIR}/cdxa.cue
		${PROJECT_BINARY_DIR}/cdxa.exe
	DESTINATION .
)
