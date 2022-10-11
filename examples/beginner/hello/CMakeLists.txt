# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	hello
	LANGUAGES    C
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK hello world example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(hello GPREL ${_sources})
#psn00bsdk_add_cd_image(hello_iso hello iso.xml DEPENDS hello)

install(FILES ${PROJECT_BINARY_DIR}/hello.exe TYPE BIN)
