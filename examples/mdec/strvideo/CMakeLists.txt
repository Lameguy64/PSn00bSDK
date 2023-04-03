# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	strvideo
	LANGUAGES    C
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK .STR video playback example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(strvideo GPREL ${_sources})
#psn00bsdk_add_cd_image(
	#strvideo_iso strvideo iso.xml
	#DEPENDS strvideo system.cnf video.str
#)

install(
	FILES
		#${PROJECT_BINARY_DIR}/strvideo.bin
		#${PROJECT_BINARY_DIR}/strvideo.cue
		${PROJECT_BINARY_DIR}/strvideo.exe
	TYPE BIN
)
