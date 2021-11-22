# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{PSN00BSDK_LIBS})
	set(CMAKE_TOOLCHAIN_FILE $ENV{PSN00BSDK_LIBS}/cmake/sdk.cmake)
endif()

project(
	gte
	LANGUAGES    C
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK GTE 3D cube example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(gte STATIC ${_sources})
#psn00bsdk_add_cd_image(gte_iso gte iso.xml DEPENDS gte)

install(FILES ${PROJECT_BINARY_DIR}/gte.exe TYPE BIN)
