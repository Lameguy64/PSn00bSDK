# PSn00bSDK toolchain setup file for CMake
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

set(
	PSN00BSDK_TC $ENV{PSN00BSDK_TC}
	CACHE PATH   "Path to the GCC toolchain's installation directory"
)
set(
	PSN00BSDK_TARGET mipsel-unknown-elf
	CACHE STRING     "GCC toolchain target triplet"
)

## CMake configuration

set(CMAKE_SYSTEM_NAME      PlayStation)
set(CMAKE_SYSTEM_PROCESSOR mipsel)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Tell CMake not to run the linker when compiling test programs. This dodges
# missing C++ standard library errors.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

## Toolchain path setup

# Attempt to find GCC. PSN00BSDK_TC can be left unset if the toolchain can be
# found in the PATH environment variable.
find_program(
	_gcc ${PSN00BSDK_TARGET}-gcc
	HINTS
		${PSN00BSDK_TC}/bin
		${PSN00BSDK_TC}/../bin
		${CMAKE_CURRENT_LIST_DIR}/../../../${PSN00BSDK_TARGET}/bin
	PATHS
		"C:/Program Files/${PSN00BSDK_TARGET}/bin"
		"C:/${PSN00BSDK_TARGET}/bin"
		/usr/local/${PSN00BSDK_TARGET}/bin
		/usr/${PSN00BSDK_TARGET}/bin
	NO_CACHE REQUIRED
)
cmake_path(GET _gcc PARENT_PATH _bin)
cmake_path(GET _bin PARENT_PATH _toolchain)

# Overwrite the empty cache entry, so it won't have to be found again.
if(NOT IS_DIRECTORY PSN00BSDK_TC)
	set(
		PSN00BSDK_TC ${_toolchain}
		CACHE PATH   "Path to the GCC toolchain's installation directory"
		FORCE
	)
endif()

## Toolchain executables

set(_prefix ${_bin}/${PSN00BSDK_TARGET})

set(CMAKE_ASM_COMPILER ${_prefix}-gcc${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_C_COMPILER   ${_prefix}-gcc${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_CXX_COMPILER ${_prefix}-g++${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_AR           ${_prefix}-ar${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_LINKER       ${_prefix}-ld${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_RANLIB       ${_prefix}-ranlib${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_OBJCOPY      ${_prefix}-objcopy${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_SIZE         ${_prefix}-size${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_STRIP        ${_prefix}-strip${CMAKE_EXECUTABLE_SUFFIX})
set(TOOLCHAIN_NM       ${_prefix}-nm${CMAKE_EXECUTABLE_SUFFIX})

## SDK setup

# We can't set up the SDK here as the find_*() functions may fail if they are
# called before project(). We can however set a script to be executed right
# after project() is invoked.
set(CMAKE_PROJECT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/internal_setup.cmake)
