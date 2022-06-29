# libpsn00b build script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.20)

# ${PROJECT_SOURCE_DIR} is not available until project() is called.
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/sdk.cmake)

project(
	libpsn00b
	LANGUAGES    C CXX ASM
	DESCRIPTION  "PSn00bSDK libraries"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

if(NOT DEFINED PSN00BSDK_LIBGCC)
	message(FATAL_ERROR "Failed to obtain information about the GCC toolchain. Check your toolchain settings.")
elseif(PSN00BSDK_LIBGCC STREQUAL "PSN00BSDK_LIBGCC-NOTFOUND")
	message(FATAL_ERROR "Failed to find libgcc in the GCC toolchain's files. Check your toolchain settings, or set the path to libgcc using -DPSN00BSDK_LIBGCC.")
endif()

## Libraries

foreach(_library IN LISTS PSN00BSDK_LIBRARIES)
	# libc needs special handling due to the different directory name.
	if(${_library} STREQUAL "c")
		set(_path ${PROJECT_SOURCE_DIR}/libc)
	else()
		set(_path ${PROJECT_SOURCE_DIR}/${_library})
	endif()

	file(
		GLOB_RECURSE _sources
		${_path}/*.s
		${_path}/*.c
		${_path}/*.cpp
		${_path}/*.cxx
	)

	psn00bsdk_add_library(${_library} STATIC ${_sources})
endforeach()

psn00bsdk_target_incbin(psxgpu PRIVATE _gpu_debug_font psxgpu/dbugfont.tim)

# Extract libgcc's contents and merge them into libc after building.
# Unfortunately glob expressions won't work on Windows, so we have to manually
# enumerate the contents of libgcc and save the list to a temporary file (as it
# is too long to be passed directly on the command line). This is actually the
# most reliable way I found to do this (I tried $<TARGET_OBJECTS> to no avail).
add_custom_command(
	TARGET  c POST_BUILD
	COMMAND ${CMAKE_AR} t ${PSN00BSDK_LIBGCC} $<ANGLE-R>_libgcc.txt
	COMMAND ${CMAKE_AR} x ${PSN00BSDK_LIBGCC}
	COMMAND ${CMAKE_AR} q $<TARGET_FILE:c> \@_libgcc.txt
	COMMAND ${CMAKE_AR} s $<TARGET_FILE:c>
	#COMMAND ${CMAKE_AR} rsuU $<TARGET_FILE:c> *.o
	COMMENT "Merging libgcc contents into SDK libc"
)

## Installation

install(
	TARGETS     ${PSN00BSDK_LIBRARIES}
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/libpsn00b
	EXPORT      libpsn00b
)
install(
	DIRECTORY   cmake ldscripts
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/libpsn00b
)
install(
	DIRECTORY   include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libpsn00b
)

# Generate build.json. This file is used to determine the SDK version after
# installation and may contain additional metadata about the build.
file(
	CONFIGURE
	OUTPUT  build.json
	CONTENT [[{
	"version":       "${PSN00BSDK_VERSION}",
	"build_date":    "${PSN00BSDK_BUILD_DATE}",
	"git_tag":       "${PSN00BSDK_GIT_TAG}",
	"git_commit":    "${PSN00BSDK_GIT_COMMIT}",
	"cmake_version": "${CMAKE_VERSION}",
	"host_system":   "${CMAKE_HOST_SYSTEM_NAME}"
}]]
	ESCAPE_QUOTES
	NEWLINE_STYLE LF
)
install(
	FILES       ${PROJECT_BINARY_DIR}/build.json
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/libpsn00b
)

# Generate an import script, which will be used by the setup script to find the
# libraries.
install(
	EXPORT      libpsn00b
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/libpsn00b/cmake
	#EXPORT_LINK_INTERFACE_LIBRARIES
)
