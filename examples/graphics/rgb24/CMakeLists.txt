# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{PSN00BSDK_LIBS})
	set(CMAKE_TOOLCHAIN_FILE $ENV{PSN00BSDK_LIBS}/cmake/sdk.cmake)
endif()

project(
	rgb24
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK 24-bit RGB display example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

configure_file(tim.s.template tim.s)

file(GLOB _sources *.c)
psn00bsdk_add_executable(
	rgb24 STATIC
	${_sources}
	${PROJECT_BINARY_DIR}/tim.s
)
#psn00bsdk_add_cd_image(rgb24_iso rgb24 iso.xml DEPENDS rgb24)

install(FILES ${PROJECT_BINARY_DIR}/rgb24.exe TYPE BIN)
