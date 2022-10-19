# libpsn00b build script
# (C) 2021-2022 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

# ${PROJECT_SOURCE_DIR} is not available until project() is called.
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/sdk.cmake)

project(
	libpsn00b
	LANGUAGES    C CXX ASM
	DESCRIPTION  "PSn00bSDK libraries"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

include(${PROJECT_SOURCE_DIR}/cmake/flags.cmake)

## Libraries

set(_types    EXECUTABLE_GPREL EXECUTABLE_NOGPREL SHARED_LIBRARY)
set(_suffixes _exe_gprel _exe_nogprel _dll)

foreach(_library IN LISTS PSN00BSDK_LIBRARIES)
	# libc needs special handling due to the different directory name.
	if(${_library} STREQUAL "c")
		set(_path ${PROJECT_SOURCE_DIR}/libc)
	else()
		set(_path ${PROJECT_SOURCE_DIR}/${_library})
	endif()

	file(
		GLOB_RECURSE _sources
		${_path}/*.s ${_path}/*.c ${_path}/*.cpp
	)

	# Build a separate version of the library for each supported target type
	# and create a virtual target that links the appropriate version of the
	# library.
	add_library(${_library} INTERFACE)

	foreach(_type _suffix IN ZIP_LISTS _types _suffixes)
		set(_name ${_library}${_suffix})
		list(APPEND _libraries ${_name})

		psn00bsdk_add_library(${_name} STATIC ${_sources})
		set_target_properties(${_name} PROPERTIES PSN00BSDK_TARGET_TYPE ${_type})
		target_link_libraries(
			${_library} INTERFACE
			$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSN00BSDK_TARGET_TYPE>>,${_type}>:${_name}>
		)
	endforeach()
endforeach()

# Add binary assets to each version of the libraries.
foreach(_suffix IN LISTS _suffixes)
	psn00bsdk_target_incbin(
		psxgpu${_suffix} PRIVATE _gpu_debug_font
		psxgpu/dbugfont.tim
	)
endforeach()

## Installation

install(
	TARGETS     psn00bsdk ${PSN00BSDK_LIBRARIES} ${_libraries}
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/libpsn00b/$<LOWER_CASE:$<CONFIG>>
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
# libraries. Note that CMake actually generates two separate import scripts
# (one setting configuration-specific options), so this won't create conflicts
# once the debug and release builds are merged into the same installation tree.
install(
	EXPORT      libpsn00b
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/libpsn00b
	#EXPORT_LINK_INTERFACE_LIBRARIES
)
