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

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_C_STANDARD           11)
set(CMAKE_CXX_STANDARD         17)

## Dependencies

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/tinyxml2/tinyxml2.cpp)
	message(FATAL_ERROR "The tinyxml2 directory is empty. Run 'git submodule update --init --recursive' to populate it.")
endif()

# Build tinyxml2. I didn't bother with tinyxml2's actual CMake integration
# because it's far easier do do this. It is a single-file library after all.
add_library               (tinyxml2 STATIC tinyxml2/tinyxml2.cpp)
target_include_directories(tinyxml2 PUBLIC tinyxml2)

#add_subdirectory(tinyxml2)

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

add_executable(elf2x   util/elf2x.c)
add_executable(elf2cpe util/elf2cpe.c)
add_executable(smxlink smxlink/main.cpp smxlink/timreader.cpp)
add_executable(lzpack  lzpack/main.cpp lzpack/filelist.cpp)
target_link_libraries(smxlink tinyxml2)
target_link_libraries(lzpack  tinyxml2 lzp)

## Installation

# Install the executables and copy the Blender SMX export plugin to the data
# directory (for manual installation).
install(TARGETS elf2x elf2cpe smxlink lzpack)
install(
	DIRECTORY   plugin
	DESTINATION ${CMAKE_INSTALL_DATADIR}/psn00bsdk
)

include(InstallRequiredSystemLibraries)
