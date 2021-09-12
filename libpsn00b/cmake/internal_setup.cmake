# PSn00bSDK internal setup script for CMake
# (C) 2021 spicyjpeg - MPL licensed

# This script is included automatically when using the toolchain file and
# defines helper functions.

cmake_minimum_required(VERSION 3.21)
include(GNUInstallDirs)

## Settings (can be overridden by projects)

set(PSN00BSDK_EXECUTABLE_SUFFIX     ".exe")
set(PSN00BSDK_SHARED_LIBRARY_SUFFIX ".dll")
set(PSN00BSDK_SYMBOL_MAP_SUFFIX     ".map")

## SDK libraries

# DON'T CHANGE THE ORDER or you'll break the libraries' internal dependencies.
set(PSN00BSDK_LIBRARIES psxgpu psxgte psxspu psxcd psxsio psxetc psxapi lzp c)

include(${CMAKE_CURRENT_LIST_DIR}/libpsn00b.cmake OPTIONAL)
if(NOT TARGET psn00bsdk_common)
	include(${CMAKE_CURRENT_LIST_DIR}/virtual_targets.cmake)
endif()

# Use the toolchain path to find libgcc (used to build libpsn00b). Of course
# different installers, packages and distros have different opinions when it
# comes to deciding where to install toolchains, so we have to bruteforce
# multiple combinations of paths.
if(CMAKE_C_COMPILER_VERSION)
	string(REGEX MATCH "^([0-9]+)\." _dummy ${CMAKE_C_COMPILER_VERSION})

	find_library(
		PSN00BSDK_LIBGCC gcc
		HINTS
			${PSN00BSDK_TC}/lib/gcc-cross/${PSN00BSDK_TARGET}/${CMAKE_C_COMPILER_VERSION}
			${PSN00BSDK_TC}/lib/gcc-cross/${PSN00BSDK_TARGET}/${CMAKE_MATCH_1}
			${PSN00BSDK_TC}/lib/gcc/${PSN00BSDK_TARGET}/${CMAKE_C_COMPILER_VERSION}
			${PSN00BSDK_TC}/lib/gcc/${PSN00BSDK_TARGET}/${CMAKE_MATCH_1}
			${PSN00BSDK_TC}/../lib/gcc-cross/${PSN00BSDK_TARGET}/${CMAKE_C_COMPILER_VERSION}
			${PSN00BSDK_TC}/../lib/gcc-cross/${PSN00BSDK_TARGET}/${CMAKE_MATCH_1}
			${PSN00BSDK_TC}/../lib/gcc/${PSN00BSDK_TARGET}/${CMAKE_C_COMPILER_VERSION}
			${PSN00BSDK_TC}/../lib/gcc/${PSN00BSDK_TARGET}/${CMAKE_MATCH_1}
		NO_DEFAULT_PATH
		DOC "Path to libgcc (bundled with the GCC toolchain)"
	)
endif()

## Tools

set(
	PSN00BSDK_TOOLS
	${CMAKE_CURRENT_LIST_DIR}/../../../${CMAKE_INSTALL_BINDIR}
	${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
)

find_program(ELF2X    elf2x    HINTS ${PSN00BSDK_TOOLS})
find_program(ELF2CPE  elf2cpe  HINTS ${PSN00BSDK_TOOLS})
find_program(SMXLINK  elf2x    HINTS ${PSN00BSDK_TOOLS})
find_program(LZPACK   lzpack   HINTS ${PSN00BSDK_TOOLS})
find_program(MKPSXISO mkpsxiso HINTS ${PSN00BSDK_TOOLS})

## Helper functions for executables

set(PSN00BSDK_INCLUDE   ${CMAKE_CURRENT_LIST_DIR}/../include)
set(PSN00BSDK_LDSCRIPTS ${CMAKE_CURRENT_LIST_DIR}/../ldscripts)

# psn00bsdk_add_executable(
#   <target name> <STATIC|DYNAMIC>
#   [EXCLUDE_FROM_ALL]
#   <sources> ...
# )
function(psn00bsdk_add_executable name type)
	string(TOLOWER ${type} _type)
	if(NOT ${_type} MATCHES "^(static|dynamic)$")
		message(FATAL_ERROR "Invalid executable type: ${type} (must be STATIC or DYNAMIC)")
	endif()

	add_executable       (${name} ${ARGN})
	target_link_libraries(${name} psn00bsdk_${_type}_exe)
	set_target_properties(${name} PROPERTIES PREFIX "" SUFFIX ".elf")
	target_link_options  (${name} PRIVATE -T${PSN00BSDK_LDSCRIPTS}/exe.ld)

	target_include_directories(${name} PRIVATE ${PSN00BSDK_INCLUDE})

	# Add post-build steps to generate the .exe and symbol map once the
	# executable is built. CMake 3.21 added support for target-dependent
	# generator expressions (catchy name lol) in add_custom_command(), so I'm
	# making heavy use of those here.
	add_custom_command(
		TARGET     ${name} POST_BUILD
		COMMAND    ${ELF2X} -q ${name}.elf ${name}${PSN00BSDK_EXECUTABLE_SUFFIX}
		COMMAND    ${TOOLCHAIN_NM} -f posix -l -n ${name}.elf $<ANGLE-R>${name}${PSN00BSDK_SYMBOL_MAP_SUFFIX}
		BYPRODUCTS ${name}${PSN00BSDK_EXECUTABLE_SUFFIX} ${name}${PSN00BSDK_SYMBOL_MAP_SUFFIX}
	)
endfunction()

# psn00bsdk_add_library(
#   <target name> <STATIC|SHARED|MODULE>
#   [EXCLUDE_FROM_ALL]
#   <sources> ...
# )
# Note that SHARED and MODULE have the same meaning (both will create a DLL).
# SDK libraries are NOT statically linked in by default; if you need to link
# something, use target_link_libraries() manually.
function(psn00bsdk_add_library name type)
	string(TOUPPER ${type} _type_upper)
	string(TOLOWER ${type} _type)
	if(NOT ${_type} MATCHES "^(static|object|shared|module)$")
		message(FATAL_ERROR "Invalid library type: ${type} (must be STATIC, OBJECT, SHARED or MODULE)")
	endif()

	add_library          (${name} ${_type_upper} ${ARGN})
	target_link_libraries(${name} psn00bsdk_${_type}_lib)

	target_include_directories(${name} PRIVATE ${PSN00BSDK_INCLUDE})

	if(${_type} MATCHES "^(shared|module)$")
		set_target_properties(${name} PROPERTIES PREFIX "" SUFFIX ".so")
		target_link_options  (${name} PRIVATE -T${PSN00BSDK_LDSCRIPTS}/dll.ld)

		# Add a post-build step to dump the DLL's raw contents into a new file
		# separate from the built ELF.
		add_custom_command(
			TARGET     ${name} POST_BUILD
			COMMAND    ${CMAKE_OBJCOPY} -O binary ${name}.so ${name}${PSN00BSDK_SHARED_LIBRARY_SUFFIX}
			BYPRODUCTS ${name}${PSN00BSDK_SHARED_LIBRARY_SUFFIX}
		)
	else()
		set_target_properties(${name} PROPERTIES PREFIX "lib" SUFFIX ".a")

		# Remove virtual target dependencies to make sure linking against the
		# library does not also propagate static library flags.
		set_target_properties(${name} PROPERTIES INTERFACE_LINK_LIBRARIES "")
	endif()
endfunction()

# psn00bsdk_add_cd_image(
#   <target name>
#   <image file name>
#   <mkpsxiso config template>
#   [DEPENDS <dependencies> ...]
#   [additional options passed to add_custom_target()]
# )
function(psn00bsdk_add_cd_image name image_name config_file)
	set(CD_IMAGE_NAME ${image_name})
	configure_file(${config_file} _gen_${config_file})

	add_custom_target(
		${name} ALL
		COMMAND    ${MKPSXISO} -y -q _gen_${config_file}
		BYPRODUCTS ${image_name}.bin ${image_name}.cue
		COMMENT    "Building CD image ${image_name}"
		${ARGN}
	)
endfunction()

## Helper functions for assets (TODO)
