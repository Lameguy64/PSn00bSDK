# PSn00bSDK toolchain setup file for CMake
# (C) 2021-2022 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

set(
	PSN00BSDK_TC ""
	CACHE PATH   "Path to the GCC toolchain's installation directory (if not in PATH)"
)
set(
	PSN00BSDK_TARGET mipsel-none-elf
	CACHE STRING     "GCC toolchain target triplet"
)

## CMake configuration

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR mipsel)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Tell CMake not to run the linker when compiling test programs, and to pass
# toolchain settings to the generated test projects. This dodges missing C++
# standard library errors.
set(CMAKE_TRY_COMPILE_TARGET_TYPE        STATIC_LIBRARY)
set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES PSN00BSDK_TC PSN00BSDK_TARGET PSN00BSDK_VERSION)

# Always generate compile_commands.json alongside build scripts. This allows
# some IDEs and tools (such as clangd) to automatically configure include
# directories and other options.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## Toolchain path setup

# Attempt to find GCC using a list of common installation locations.
# PSN00BSDK_TC can be left unset if the toolchain can be found in any of these
# or in the PATH environment variable.
find_program(
	_gcc ${PSN00BSDK_TARGET}-gcc
	HINTS
		${PSN00BSDK_TC}/bin
		${PSN00BSDK_TC}/../bin
		# Same as ${CMAKE_INSTALL_PREFIX}/bin
		${CMAKE_CURRENT_LIST_DIR}/../../../bin
		# Same as ${CMAKE_INSTALL_PREFIX}/${PSN00BSDK_TARGET}/bin
		${CMAKE_CURRENT_LIST_DIR}/../../../${PSN00BSDK_TARGET}/bin
	PATHS
		"C:/Program Files/${PSN00BSDK_TARGET}/bin"
		"C:/Program Files (x86)/${PSN00BSDK_TARGET}/bin"
		"C:/${PSN00BSDK_TARGET}/bin"
		/opt/${PSN00BSDK_TARGET}/bin
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
		CACHE PATH   "Path to the GCC toolchain's installation directory (if not in PATH)"
		FORCE
	)
endif()

## Toolchain executables

# As we have overridden ${CMAKE_EXECUTABLE_SUFFIX} we can't rely on it to
# determine the host OS extension for executables. A workaround is to extract
# the extension from the path returned by find_program() using a regex.
set(_prefix ${_bin}/${PSN00BSDK_TARGET})
string(REGEX MATCH ".+-gcc(.*)$" _dummy ${_gcc})

set(CMAKE_ASM_COMPILER ${_prefix}-gcc${CMAKE_MATCH_1})
set(CMAKE_C_COMPILER   ${_prefix}-gcc${CMAKE_MATCH_1})
set(CMAKE_CXX_COMPILER ${_prefix}-g++${CMAKE_MATCH_1})
set(CMAKE_AR           ${_prefix}-ar${CMAKE_MATCH_1})
set(CMAKE_LINKER       ${_prefix}-ld${CMAKE_MATCH_1})
set(CMAKE_RANLIB       ${_prefix}-ranlib${CMAKE_MATCH_1})
set(CMAKE_OBJCOPY      ${_prefix}-objcopy${CMAKE_MATCH_1})
set(CMAKE_SIZE         ${_prefix}-size${CMAKE_MATCH_1})
set(CMAKE_STRIP        ${_prefix}-strip${CMAKE_MATCH_1})
set(TOOLCHAIN_NM       ${_prefix}-nm${CMAKE_MATCH_1})

## SDK setup

# Continue initialization by running internal_setup.cmake after project() is
# invoked.
set(CMAKE_PROJECT_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/internal_setup.cmake)
