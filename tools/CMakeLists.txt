# PSn00bSDK tools build script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	PSn00bSDK-tools
	LANGUAGES    C CXX
	DESCRIPTION  "PSn00bSDK command-line tools"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

include(GNUInstallDirs)

## External dependencies

# Let CMake attempt to find tinyxml2 on its own first. This avoids invoking
# pkg-config where it might not be installed, and allows usage of package
# managers like vcpkg. The path to tinyxml2 can also be specified manually by
# passing -Dtinyxml2_ROOT.
find_package(tinyxml2 CONFIG)

if(NOT tinyxml2_FOUND)
	find_package(PkgConfig REQUIRED)
	pkg_search_module(_tinyxml2 REQUIRED IMPORTED_TARGET tinyxml2)

	add_library(tinyxml2::tinyxml2 ALIAS PkgConfig::_tinyxml2)
endif()

## Internal dependencies

# Build liblzp using sources from the libpsn00b tree. Hacky but it works.
set(LIBPSN00B_PATH ${PROJECT_SOURCE_DIR}/../libpsn00b)
file(
	GLOB _sources
	${LIBPSN00B_PATH}/lzp/*.c
	#${LIBPSN00B_PATH}/lzp/*.cpp
)

add_library(lzp STATIC ${_sources})
target_include_directories(
	lzp PUBLIC
	lzpack/lzp
	${LIBPSN00B_PATH}/include/lzp
)

## Executables

# This is required in order to properly link against tinyxml2 under MSVC.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_C_STANDARD           11)
set(CMAKE_CXX_STANDARD         11)

add_executable(elf2x   util/elf2x.c)
add_executable(elf2cpe util/elf2cpe.c)
add_executable(smxlink smxlink/main.cpp smxlink/timreader.cpp)
add_executable(lzpack  lzpack/main.cpp lzpack/filelist.cpp)
target_link_libraries(smxlink tinyxml2::tinyxml2)
target_link_libraries(lzpack  tinyxml2::tinyxml2 lzp)

## Installation

# Install the executables alongside the tinyxml2 DLL (if any) and copy the
# Blender SMX export plugin to the data directory (for manual installation).
install(
	TARGETS elf2x elf2cpe smxlink lzpack
	RUNTIME_DEPENDENCIES
	PRE_EXCLUDE_REGEXES ".*"
	PRE_INCLUDE_REGEXES "tinyxml2"
)
install(
	DIRECTORY   plugin
	DESTINATION ${CMAKE_INSTALL_DATADIR}/psn00bsdk
)

include(InstallRequiredSystemLibraries)
