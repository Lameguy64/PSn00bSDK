# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{PSN00BSDK_LIBS})
	set(CMAKE_TOOLCHAIN_FILE $ENV{PSN00BSDK_LIBS}/cmake/sdk.cmake)
endif()

project(
	dynlink
	LANGUAGES    C
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK dynamic linker example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(dynlink_main DYNAMIC ${_sources})
psn00bsdk_add_library   (dynlink_cube  SHARED library/cube.c)
psn00bsdk_add_library   (dynlink_balls SHARED library/balls.c)
psn00bsdk_add_cd_image(
	dynlink_iso dynlink iso.xml
	DEPENDS dynlink_main dynlink_cube dynlink_balls
)

install(
	FILES
		${PROJECT_BINARY_DIR}/dynlink.bin
		${PROJECT_BINARY_DIR}/dynlink.cue
	DESTINATION .
)
