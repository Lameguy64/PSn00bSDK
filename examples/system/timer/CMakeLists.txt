# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	timer
	LANGUAGES    C
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK hardware timer example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(timer GPREL ${_sources})
#psn00bsdk_add_cd_image(timer_iso timer iso.xml DEPENDS timer)

install(FILES ${PROJECT_BINARY_DIR}/timer.exe TYPE BIN)
