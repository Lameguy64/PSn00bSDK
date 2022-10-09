# PSn00bSDK interface targets
# (C) 2021-2022 spicyjpeg - MPL licensed

# This script creates several "virtual" targets (psn00bsdk_*) that set include
# directories and compiler flags when a target is linked against them. It is
# only used when building libpsn00b, as CMake automatically saves these targets
# into the export script once libpsn00b is installed.

## Options common to all target types

# - Define PSN00BSDK=1
# - Always generate debug symbols (stripped by elf2x)
# - Optimize for MIPS R3000
# - Inject zero checks into division operations (will throw breaks)
# - All standard libraries (including libgcc) disabled
# - Put all symbols into separate sections when building
# - C++ features that require runtime support disabled
# - Unused section stripping enabled

add_library(psn00bsdk_common INTERFACE)
target_compile_options(
	psn00bsdk_common INTERFACE
		# CPU options
		-msoft-float
		-march=r3000
		-mtune=r3000
		-mabi=32
		-mno-mt
		-mno-llsc
		-mdivide-breaks
		-O2
		# Standard library options
		-ffreestanding
		-fno-builtin
		-nostdlib
		# Other options
		-g
		-Wa,--strip-local-absolute
		-fdata-sections
		-ffunction-sections
		-fsigned-char
		-fno-strict-overflow
		-fdiagnostics-color=always
	$<$<COMPILE_LANGUAGE:CXX>:
		# C++ options
		-fno-exceptions
		-fno-rtti
		-fno-unwind-tables
		-fno-threadsafe-statics
		-fno-use-cxa-atexit
	>
)
target_link_options(
	psn00bsdk_common INTERFACE
		-nostdlib
		-Wl,-gc-sections
)
target_compile_definitions(
	psn00bsdk_common INTERFACE
		PSN00BSDK=1
		$<$<CONFIG:DEBUG>:DEBUG=1>
)

## Options for executables without support for dynamic linking

# - Position-independent code disabled
# - $gp-relative addressing enabled only for local symbols
# - ABI-compatible calls disabled (incompatible with $gp-relative addressing)

add_library(psn00bsdk_exe_gprel INTERFACE)
target_link_libraries(psn00bsdk_exe_gprel INTERFACE psn00bsdk_common)
target_compile_options(
	psn00bsdk_exe_gprel INTERFACE
		-G8
		-fno-pic
		-mno-abicalls
		-mgpopt
		-mno-extern-sdata
)
target_link_options(
	psn00bsdk_exe_gprel INTERFACE
		-G8
		-static
)

## Options for executables with support for dynamic linking

# - Position-independent code disabled
# - $gp-relative addressing disabled
# - ABI-compatible calls disabled (must be performed manually)

add_library(psn00bsdk_exe_nogprel INTERFACE)
target_link_libraries(psn00bsdk_exe_nogprel INTERFACE psn00bsdk_common)
target_compile_options(
	psn00bsdk_exe_nogprel INTERFACE
		-G0
		-fno-pic
		-mno-abicalls
		-mno-gpopt
)
target_link_options(
	psn00bsdk_exe_nogprel INTERFACE
		-G0
		-static
)

## Options for dynamically-linked libraries

# - Position-independent code enabled
# - $gp-relative addressing disabled (incompatible with ABI calls)
# - ABI-compatible calls enabled

add_library(psn00bsdk_dll INTERFACE)
target_link_libraries(psn00bsdk_dll INTERFACE psn00bsdk_common)
target_compile_options(
	psn00bsdk_dll INTERFACE
		-G0
		-fPIC
		-mabicalls
		-mno-gpopt
		-mshared
)
target_link_options(
	psn00bsdk_dll INTERFACE
		-G0
		-shared
)
