# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	cdxa
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK CD-XA playback example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(cdxa GPREL ${_sources})
#psn00bsdk_add_cd_image(
	#cdxa_iso cdxa iso.xml
	#DEPENDS cdxa system.cnf xasample.xa
#)

psn00bsdk_target_incbin(cdxa PRIVATE ball16c ball16c.tim)

install(
	FILES
		#${PROJECT_BINARY_DIR}/cdxa.bin
		#${PROJECT_BINARY_DIR}/cdxa.cue
		${PROJECT_BINARY_DIR}/cdxa.exe
	TYPE BIN
)
