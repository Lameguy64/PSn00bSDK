# PSn00bSDK internal setup script for CMake
# (C) 2021-2022 spicyjpeg - MPL licensed

# This script is included automatically when using the toolchain file and
# defines helper functions.

cmake_minimum_required(VERSION 3.21)
include(GNUInstallDirs)

## CMake configuration

# Setting these variables and properties would technically be the toolchain
# script's responsibility, however they are overridden by project() so their
# setting is deferred to this script.
set(CMAKE_EXECUTABLE_SUFFIX     ".elf")
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
set(CMAKE_SHARED_MODULE_PREFIX  "")
set(CMAKE_SHARED_MODULE_SUFFIX  ".so")

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS ON)

## PSn00bSDK initialization

# Fetch SDK version information from build.json.
if(NOT DEFINED PSN00BSDK_VERSION)
	file(READ ${CMAKE_CURRENT_LIST_DIR}/../build.json _json)

	string(JSON PSN00BSDK_VERSION    GET ${_json} version)
	string(JSON PSN00BSDK_BUILD_DATE GET ${_json} build_date)
	string(JSON PSN00BSDK_GIT_TAG    GET ${_json} git_tag)
	string(JSON PSN00BSDK_GIT_COMMIT GET ${_json} git_commit)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/libpsn00b.cmake OPTIONAL)
if(TARGET psn00bsdk)
	link_libraries(psn00bsdk)
endif()
link_libraries(-lgcc)

# DON'T CHANGE THE ORDER or you'll break the libraries' internal dependencies.
set(
	PSN00BSDK_LIBRARIES
		psxgpu
		psxgte
		psxspu
		psxcd
		psxpress
		psxsio
		psxetc
		psxapi
		smd
		lzp
		c
)

## Settings (can be overridden by projects)

set(PSN00BSDK_EXECUTABLE_LINK_LIBRARIES     ${PSN00BSDK_LIBRARIES})
set(PSN00BSDK_SHARED_LIBRARY_LINK_LIBRARIES "")

set(PSN00BSDK_EXECUTABLE_SUFFIX     ".exe")
set(PSN00BSDK_SHARED_LIBRARY_SUFFIX ".dll")
set(PSN00BSDK_SYMBOL_MAP_SUFFIX     ".map")

define_property(
	TARGET PROPERTY PSN00BSDK_TARGET_TYPE
	BRIEF_DOCS      "Type of this target (EXECUTABLE_GPREL, EXECUTABLE_NOGPREL or SHARED_LIBRARY)"
	FULL_DOCS       "Type of this target (if executable or DLL) or of the executable/DLL this target is going to be linked to (if static library)"
)

## Include paths

set(PSN00BSDK_LDSCRIPTS ${CMAKE_CURRENT_LIST_DIR}/../ldscripts)
if(IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../include)
	set(PSN00BSDK_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/../include)
else()
	set(PSN00BSDK_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/../../../include/libpsn00b)
endif()

include_directories(AFTER ${PSN00BSDK_INCLUDE})

## Tool paths

set(
	PSN00BSDK_TOOLS
	${CMAKE_CURRENT_LIST_DIR}/../../../${CMAKE_INSTALL_BINDIR}
	${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
)

find_program(ELF2X    elf2x    HINTS ${PSN00BSDK_TOOLS})
find_program(ELF2CPE  elf2cpe  HINTS ${PSN00BSDK_TOOLS})
find_program(SMXLINK  smxlink  HINTS ${PSN00BSDK_TOOLS})
find_program(LZPACK   lzpack   HINTS ${PSN00BSDK_TOOLS})
find_program(MKPSXISO mkpsxiso HINTS ${PSN00BSDK_TOOLS})
#find_program(PSXAVENC psxavenc HINTS ${PSN00BSDK_TOOLS})

## Target helpers

function(psn00bsdk_add_executable name type)
	string(TOUPPER ${type} _type)

	if(_type MATCHES "^(STATIC|GPREL)$")
		set(_type EXECUTABLE_GPREL)
	elseif(_type MATCHES "^(DYNAMIC|NOGPREL)$")
		set(_type EXECUTABLE_NOGPREL)
	else()
		message(FATAL_ERROR "Invalid executable type: ${type} (must be STATIC, GPREL, DYNAMIC or NOGPREL)")
	endif()

	# Throw an error if elf2x was not found (which should never happen if the
	# SDK is installed properly).
	if(ELF2X STREQUAL "ELF2X-NOTFOUND")
		message(FATAL_ERROR "Failed to locate elf2x. Check your PATH environment variable.")
	endif()

	add_executable       (${name} ${ARGN})
	set_target_properties(${name} PROPERTIES PSN00BSDK_TARGET_TYPE ${_type})
	target_link_libraries(${name} PRIVATE ${PSN00BSDK_EXECUTABLE_LINK_LIBRARIES})
	target_link_options  (${name} PRIVATE -T$<SHELL_PATH:${PSN00BSDK_LDSCRIPTS}/exe.ld>)

	# Add post-build steps to generate the .exe and symbol map once the
	# executable is built.
	# FIXME: CMake does not (yet) allow target-dependent generator expressions
	# to specify the byproducts, so we have to make sure the generated files
	# have no prefix/suffix and are in the current build directory.
	#set(_repl PATH:REPLACE_EXTENSION,LAST_ONLY,$<TARGET_FILE:${name}>)
	add_custom_command(
		TARGET ${name} POST_BUILD
		COMMAND
			${ELF2X} -q
			$<SHELL_PATH:$<TARGET_FILE:${name}>>
			#$<SHELL_PATH:$<${_repl},${PSN00BSDK_EXECUTABLE_SUFFIX}>>
			$<SHELL_PATH:${CMAKE_CURRENT_BINARY_DIR}/${name}${PSN00BSDK_EXECUTABLE_SUFFIX}>
		COMMAND
			${TOOLCHAIN_NM} -f posix -l -n
			$<SHELL_PATH:$<TARGET_FILE:${name}>>
			#$<ANGLE-R>$<SHELL_PATH:$<${_repl},${PSN00BSDK_SYMBOL_MAP_SUFFIX}>>
			$<ANGLE-R>$<SHELL_PATH:${CMAKE_CURRENT_BINARY_DIR}/${name}${PSN00BSDK_SYMBOL_MAP_SUFFIX}>
		BYPRODUCTS
			#$<${_repl},${PSN00BSDK_EXECUTABLE_SUFFIX}>
			#$<${_repl},${PSN00BSDK_SYMBOL_MAP_SUFFIX}>
			${CMAKE_CURRENT_BINARY_DIR}/${name}${PSN00BSDK_EXECUTABLE_SUFFIX}
			${CMAKE_CURRENT_BINARY_DIR}/${name}${PSN00BSDK_SYMBOL_MAP_SUFFIX}
		#VERBATIM
	)
endfunction()

function(psn00bsdk_add_library name type)
	string(TOUPPER ${type} _type)

	if(_type MATCHES "^(STATIC|OBJECT)$")
		add_library          (${name} ${_type} ${ARGN})
		#target_link_libraries(${name} PRIVATE psn00bsdk)
	elseif(_type MATCHES "^(SHARED|MODULE)$")
		add_library          (${name} ${_type} ${ARGN})
		set_target_properties(${name} PROPERTIES PSN00BSDK_TARGET_TYPE SHARED_LIBRARY)
		target_link_libraries(${name} PRIVATE ${PSN00BSDK_SHARED_LIBRARY_LINK_LIBRARIES})
		target_link_options  (${name} PRIVATE -T$<SHELL_PATH:${PSN00BSDK_LDSCRIPTS}/dll.ld>)

		# Add a post-build step to dump the DLL's raw contents into a new file
		# separate from the built ELF.
		#set(_repl PATH:REPLACE_EXTENSION,LAST_ONLY,$<TARGET_FILE:${name}>)
		add_custom_command(
			TARGET ${name} POST_BUILD
			COMMAND
				${CMAKE_OBJCOPY} -O binary
				$<SHELL_PATH:$<TARGET_FILE:${name}>>
				#$<SHELL_PATH:$<${_repl},${PSN00BSDK_SHARED_LIBRARY_SUFFIX}>>
				$<SHELL_PATH:${CMAKE_CURRENT_BINARY_DIR}/${name}${PSN00BSDK_SHARED_LIBRARY_SUFFIX}>
			#BYPRODUCTS $<${_repl},${PSN00BSDK_SHARED_LIBRARY_SUFFIX}>
			BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${name}${PSN00BSDK_SHARED_LIBRARY_SUFFIX}
			VERBATIM
		)
	else()
		message(FATAL_ERROR "Invalid library type: ${type} (must be STATIC, OBJECT, SHARED or MODULE)")
	endif()
endfunction()

## Linking helpers

function(psn00bsdk_target_incbin_a name type symbol_name size_name path section align)
	string(MAKE_C_IDENTIFIER "${symbol_name}" _id)
	string(MAKE_C_IDENTIFIER "${size_name}"   _size)
	cmake_path(ABSOLUTE_PATH path OUTPUT_VARIABLE _path)

	string(SHA1 _hash "${name} ${_id}")
	set(_asm_file ${CMAKE_CURRENT_BINARY_DIR}/incbin_${_hash}.s)

	# Generate an assembly source file that includes the binary file and add it
	# to the target's sources. The file is also added as a depedency to ensure
	# CMake builds it before the target (if it's not a static file).
	file(
		CONFIGURE
		OUTPUT  ${_asm_file}
		CONTENT [[
.section ${section}.${_id}, "aw"
.balign ${align}

.global ${_id}
.type ${_id}, @object
.size ${_id}, (${_id}_end - ${_id})
${_id}:
	.incbin "${_path}"

.local ${_id}_end
${_id}_end:

.balign ${align}

.section ${section}.${_size}, "aw"
.balign 4

.global ${_size}
.type ${_size}, @object
.size ${_size}, 4
${_size}:
	.int (${_id}_end - ${_id})
]]
		ESCAPE_QUOTES
		NEWLINE_STYLE LF
	)

	target_sources(${name} ${type} ${_asm_file})
	set_source_files_properties(${_asm_file} PROPERTIES OBJECT_DEPENDS "${_path}")
endfunction()

function(psn00bsdk_target_incbin name type symbol_name path)
	psn00bsdk_target_incbin_a(
		${name}
		${type}
		"${symbol_name}"
		"${symbol_name}_size"
		"${path}"
		.data
		4
	)
endfunction()

## CD image and asset helpers

function(psn00bsdk_add_cd_image name image_name config_file)
	# Throw an error if mkpsxiso was not found. Performing this check manually
	# (instead of just marking mkpsxiso as required) allows simple projects to
	# be built even if mkpsxiso is not installed.
	if(MKPSXISO STREQUAL "MKPSXISO-NOTFOUND")
		message(FATAL_ERROR "Failed to locate mkpsxiso. If mkpsxiso wasn't installed alongside the SDK, check your PATH environment variable.")
	endif()

	cmake_path(HASH config_file _hash)

	set(_xml_file ${CMAKE_CURRENT_BINARY_DIR}/cd_image_${_hash}.xml)
	configure_file("${config_file}" ${_xml_file})

	add_custom_command(
		OUTPUT ${image_name}.bin ${image_name}.cue
		COMMAND
			${MKPSXISO} -y
			-o ${image_name}.bin -c ${image_name}.cue ${_xml_file}
		COMMENT "Building CD image ${image_name}"
		VERBATIM
		${ARGN}
	)
	add_custom_target(
		${name} ALL
		DEPENDS
			${CMAKE_CURRENT_BINARY_DIR}/${image_name}.bin
			${CMAKE_CURRENT_BINARY_DIR}/${image_name}.cue
	)
endfunction()
