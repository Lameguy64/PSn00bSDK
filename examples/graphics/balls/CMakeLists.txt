# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	balls
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK sprites example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(balls GPREL ${_sources})
#psn00bsdk_add_cd_image(balls_iso balls iso.xml DEPENDS balls)

psn00bsdk_target_incbin(balls PRIVATE ball16c ball16c.tim)

install(FILES ${PROJECT_BINARY_DIR}/balls.exe TYPE BIN)
