# PSn00bSDK examples build script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	PSn00bSDK-examples
	LANGUAGES    NONE
	DESCRIPTION  "PSn00bSDK examples"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(
	GLOB_RECURSE _examples
	${PROJECT_SOURCE_DIR}/CMakeLists.txt
)

foreach(_script IN LISTS _examples)
	cmake_path(GET _script PARENT_PATH _dir)
	if(_dir STREQUAL PROJECT_SOURCE_DIR)
		continue()
	endif()

	add_subdirectory(${_dir})
endforeach()
