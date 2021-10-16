# libpsn00b build script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

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

# Copy libgcc.a and merge the PSn00bSDK libc into it after building.
add_custom_command(
	TARGET  c POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${PSN00BSDK_LIBGCC} $<TARGET_FILE:c>
	#COMMAND ${CMAKE_AR} rsuU $<TARGET_FILE:c> *.o
	COMMAND ${CMAKE_AR} rsuU $<TARGET_FILE:c> $<JOIN:"$<TARGET_OBJECTS:c>"," ">
	COMMENT "Merging SDK libc contents into libgcc"
)

## Installation

install(
	TARGETS     ${PSN00BSDK_LIBRARIES} ${PSN00BSDK_VIRTUAL_TARGETS}
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/libpsn00b
	EXPORT      libpsn00b
)
install(
	DIRECTORY   cmake include ldscripts
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/libpsn00b
)

# Generate an import script, which will be used by the setup script to find the
# libraries.
install(
	EXPORT      libpsn00b
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/libpsn00b/cmake
	#EXPORT_LINK_INTERFACE_LIBRARIES
)
