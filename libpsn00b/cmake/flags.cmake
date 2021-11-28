# libpsn00b interface targets
# (C) 2021 spicyjpeg - MPL licensed

# This script creates several "virtual" targets (psn00bsdk_*) that set include
# directories and compiler flags when a target is linked against them. The
# following targets are currently defined:
# - psn00bsdk_common
# - psn00bsdk_object_lib (same as psn00bsdk_common)
# - psn00bsdk_static_exe
# - psn00bsdk_dynamic_exe
# - psn00bsdk_static_lib
# - psn00bsdk_shared_lib
# - psn00bsdk_module_lib (same as psn00bsdk_shared_lib)
#
# NOTE: building a static library and linking it as part of a DLL is currently
# *not* supported.

if(NOT TARGET psn00bsdk_common) # Include guard

add_library(psn00bsdk_common INTERFACE)

foreach(
	_target IN ITEMS
		object_lib
		static_exe
		dynamic_exe
		static_lib
		shared_lib
		module_lib
)
	add_library          (psn00bsdk_${_target} INTERFACE)
	target_link_libraries(psn00bsdk_${_target} INTERFACE psn00bsdk_common)
endforeach()

# Options common to all target types:
# - Define PLAYSTATION=1
# - Optimize for MIPS R3000
# - Inject zero checks into division operations (will throw breaks)
# - All standard libraries (including libgcc) disabled
# - Put all symbols into separate sections when building
# - C++ features that require runtime support disabled
# - Unused section stripping enabled
target_compile_options(
	psn00bsdk_common INTERFACE
		# CPU options
		-msoft-float
		-march=r3000
		-mtune=r3000
		-mabi=32
		-mdivide-breaks
		-O2
		# Standard library options
		-ffreestanding
		-fno-builtin
		-nostdlib
		# Other options
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
		PLAYSTATION=1
		$<$<CONFIG:DEBUG>:DEBUG=1>
)

# Options for executables without support for dynamic linking:
# - Position-independent code disabled
# - GP-relative addressing enabled only for local symbols
# - ABI-compatible calls disabled (incompatible with GP-relative addr)
target_compile_options(
	psn00bsdk_static_exe INTERFACE
		-G8
		-fno-pic
		-mno-abicalls
		-mgpopt
		-mno-extern-sdata
)
target_link_options(
	psn00bsdk_static_exe INTERFACE
		-G8
		-static
)

# Options for executables with support for dynamic linking:
# - Position-independent code disabled
# - GP-relative addressing disabled
# - ABI-compatible calls disabled (must be performed manually)
target_compile_options(
	psn00bsdk_dynamic_exe INTERFACE
		-G0
		-fno-pic
		-mno-abicalls
		-mno-gpopt
)
target_link_options(
	psn00bsdk_dynamic_exe INTERFACE
		-G0
		-static
)

# Options for static libraries:
# - Position-independent code disabled
# - GP-relative addressing disabled
# - ABI-compatible calls disabled
# - Local stripping enabled
target_compile_options(
	psn00bsdk_static_lib INTERFACE
		-G0
		-fno-pic
		-mno-abicalls
		-mno-gpopt
		-Wa,--strip-local-absolute
)

# Options for dynamically-loaded libraries:
# - Position-independent code enabled
# - GP-relative addressing disabled (incompatible with ABI calls)
# - ABI-compatible calls enabled
target_compile_options(
	psn00bsdk_shared_lib INTERFACE
		-G0
		-fPIC
		-mabicalls
		-mno-gpopt
		-mshared
)
target_link_options(
	psn00bsdk_shared_lib INTERFACE
		-G0
		-shared
)

target_link_libraries(psn00bsdk_module_lib INTERFACE psn00bsdk_shared_lib)

endif()
