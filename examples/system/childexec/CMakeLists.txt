# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	childexec
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK child process example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
file(GLOB _child_sources child/*.c)
psn00bsdk_add_executable(childexec GPREL ${_sources})
psn00bsdk_add_executable(childexec_child GPREL ${_child_sources})
#psn00bsdk_add_cd_image(childexec_iso childexec iso.xml DEPENDS childexec)

psn00bsdk_target_incbin(childexec PRIVATE ball16c ball16c.tim)
psn00bsdk_target_incbin(
	childexec PRIVATE child_exe
	${PROJECT_BINARY_DIR}/childexec_child.exe
)

# Relocate the child executable to a non-default address to prevent it from
# overlapping with the main one at 0x80010000.
# NOTE: child executables are not position-independent and can't be relocated
# at runtime. If you need your code to be relocatable (e.g. to load it into a
# dynamically-allocated buffer), consider using a DLL instead.
target_link_options(childexec_child PRIVATE -Ttext=0x80030000)

# Make sure the child executable is built before the parent.
add_dependencies(childexec childexec_child)

install(FILES ${PROJECT_BINARY_DIR}/childexec.exe TYPE BIN)
