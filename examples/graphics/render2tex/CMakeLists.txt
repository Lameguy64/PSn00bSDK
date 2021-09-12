# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{PSN00BSDK_LIBS})
	set(CMAKE_TOOLCHAIN_FILE $ENV{PSN00BSDK_LIBS}/cmake/sdk.cmake)
endif()

project(
	render2tex
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK render-to-texture example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

configure_file(texture.s.template texture.s)

file(GLOB _sources *.c)
psn00bsdk_add_executable(
	render2tex STATIC
	${_sources}
	${PROJECT_BINARY_DIR}/texture.s
)
#psn00bsdk_add_cd_image(render2tex_iso render2tex iso.xml DEPENDS render2tex)

install(FILES ${PROJECT_BINARY_DIR}/render2tex.exe DESTINATION .)
