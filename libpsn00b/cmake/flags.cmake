# PSn00bSDK interface targets
# (C) 2021-2022 spicyjpeg - MPL licensed

# This script creates several "virtual" targets (psn00bsdk_*) that set include
# directories and compiler flags when a target is linked against them. It is
# only used when building libpsn00b, as CMake automatically saves these targets
# into the export script once libpsn00b is installed.

add_library   (psn00bsdk INTERFACE)
link_libraries(psn00bsdk)

target_compile_options(
	psn00bsdk INTERFACE
		# Options common to all target types
		-g
		-Wa,--strip-local-absolute
		-ffreestanding
		-fno-builtin
		-nostdlib
		-fdata-sections
		-ffunction-sections
		-fsigned-char
		-fno-strict-overflow
		-fdiagnostics-color=always
		-msoft-float
		-march=r3000
		-mtune=r3000
		-mabi=32
		-mno-mt
		-mno-llsc
	$<$<COMPILE_LANGUAGE:CXX>:
		# Options common to all target types (C++)
		-fno-exceptions
		-fno-rtti
		-fno-unwind-tables
		-fno-threadsafe-statics
		-fno-use-cxa-atexit
	>
	$<IF:$<CONFIG:Debug>,
		# Options for debug builds
		-Og
		-mdivide-breaks
	,
		# Options for release builds
		-O2
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSN00BSDK_TARGET_TYPE>>,EXECUTABLE_GPREL>:
		# Options for executables with $gp-relative addressing
		-G8
		-fno-pic
		-mno-abicalls
		-mgpopt
		-mno-extern-sdata
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSN00BSDK_TARGET_TYPE>>,EXECUTABLE_NOGPREL>:
		# Options for executables without $gp-relative addressing
		-G0
		-fno-pic
		-mno-abicalls
		-mno-gpopt
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSN00BSDK_TARGET_TYPE>>,SHARED_LIBRARY>:
		# Options for DLLs
		-G0
		-fPIC
		-mabicalls
		-mno-gpopt
		-mshared
	>
)
target_link_options(
	psn00bsdk INTERFACE
		# Options common to all target types
		-nostdlib
		-Wl,-gc-sections
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSN00BSDK_TARGET_TYPE>>,EXECUTABLE_GPREL>:
		# Options for executables with $gp-relative addressing
		-G8
		-static
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSN00BSDK_TARGET_TYPE>>,EXECUTABLE_NOGPREL>:
		# Options for executables without $gp-relative addressing
		-G0
		-static
	>
	$<$<STREQUAL:$<UPPER_CASE:$<TARGET_PROPERTY:PSN00BSDK_TARGET_TYPE>>,SHARED_LIBRARY>:
		# Options for DLLs
		-G0
		-shared
	>
)
target_compile_definitions(
	psn00bsdk INTERFACE
		PSN00BSDK=1
	$<IF:$<CONFIG:Debug>,
		#NDEBUG=0
	,
		NDEBUG=1
	>
)
