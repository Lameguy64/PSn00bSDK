# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	fpscam
	LANGUAGES    C
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK 3D camera controls example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(fpscam GPREL ${_sources})
#psn00bsdk_add_cd_image(fpscam_iso fpscam iso.xml DEPENDS fpscam)

install(FILES ${PROJECT_BINARY_DIR}/fpscam.exe TYPE BIN)
