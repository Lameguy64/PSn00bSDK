# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	dynlink
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK dynamic linker example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(dynlink_main NOGPREL ${_sources})
psn00bsdk_add_library   (dynlink_cube  SHARED library/cube.c)
psn00bsdk_add_library   (dynlink_balls SHARED library/balls.c)
psn00bsdk_add_cd_image(
	dynlink_iso dynlink iso.xml
	DEPENDS dynlink_main dynlink_cube dynlink_balls system.cnf
)

psn00bsdk_target_incbin(dynlink_balls PRIVATE ball16c library/ball16c.tim)

install(
	FILES
		${PROJECT_BINARY_DIR}/dynlink.bin
		${PROJECT_BINARY_DIR}/dynlink.cue
	TYPE BIN
)
