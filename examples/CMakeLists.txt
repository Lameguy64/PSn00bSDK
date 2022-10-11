# PSn00bSDK examples build script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	PSn00bSDK-examples
	LANGUAGES    NONE
	DESCRIPTION  "PSn00bSDK examples"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

include(GNUInstallDirs)

# Find all subdirectories that contain a CMake build script. This includes the
# top-level examples directory and this file as well, however we're going to
# skip it.
file(
	GLOB_RECURSE _examples
	RELATIVE     ${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/CMakeLists.txt
)

foreach(_script IN LISTS _examples)
	if(_script STREQUAL "CMakeLists.txt")
		continue()
	endif()

	# CMake provides no way to override the install prefix of a subdirectory,
	# as it "imports" its targets into the main project rather than treating it
	# as a separate project. However, as the example subdirectories use
	# install(... TYPE BIN) to install executables, it is possible to override
	# CMAKE_INSTALL_BINDIR temporarily to place them in any directory within
	# the install prefix. This is a hack, but it allows us to preserve the
	# examples' folder hierarchy in the installation directory (it is already
	# preserved by CMake in the build tree!) with minimal effort.
	cmake_path(GET _script PARENT_PATH _dir)
	cmake_path(GET _dir PARENT_PATH CMAKE_INSTALL_BINDIR)

	add_subdirectory(${PROJECT_SOURCE_DIR}/${_dir})
endforeach()
