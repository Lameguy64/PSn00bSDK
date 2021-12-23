# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.20)

project(
	console
	LANGUAGES    C
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK stdio console example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(console STATIC ${_sources})
#psn00bsdk_add_cd_image(console_iso console iso.xml DEPENDS console)

install(FILES ${PROJECT_BINARY_DIR}/console.exe TYPE BIN)
