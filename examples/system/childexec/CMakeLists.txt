# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{PSN00BSDK_LIBS})
	set(CMAKE_TOOLCHAIN_FILE $ENV{PSN00BSDK_LIBS}/cmake/sdk.cmake)
endif()

project(
	childexec
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK child process example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

configure_file(child_exe.s.template child_exe.s)

file(GLOB _sources *.c)
file(GLOB _child_sources child/*.c)
psn00bsdk_add_executable(
	parent STATIC
	${_sources}
	${PROJECT_BINARY_DIR}/child_exe.s
)
psn00bsdk_add_executable(child STATIC ${_child_sources})
#psn00bsdk_add_cd_image(childexec_iso childexec iso.xml DEPENDS parent)

# Make sure the child executable is built before the parent (so it can be
# embedded via child_exe.s).
add_dependencies(parent child)

install(FILES ${PROJECT_BINARY_DIR}/parent.exe DESTINATION .)
