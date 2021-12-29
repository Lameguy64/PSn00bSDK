# This script sets up all CPack-related variables and generates installation
# rules to bundle the GCC toolchain and CMake in packages. It is included by
# the main CMakeLists.txt script.

cmake_minimum_required(VERSION 3.20)

## Settings

# These can be set from the command line to completely disable bundling CMake
# and GCC in packages built by CPack. They have no effect on installing the SDK
# normally.
# TODO: BUNDLE_CMAKE should not be used, needs more testing
set(
	BUNDLE_TOOLCHAIN OFF
	CACHE BOOL       "Include the GCC toolchain in installer packages"
)
#set(
	#BUNDLE_CMAKE OFF
	#CACHE BOOL   "Include CMake in installer packages (Windows only)"
#)

## Bundled components

# "Install" the toolchain and CMake (by pulling files from their install
# locations). This is only useful when building installers, as CPack will pick
# up these installation rules and bundle the toolchain in the installers.
# NOTE: unfortunately there is no easy way to reuse the toolchain finding logic
# in sdk.cmake, so I had to copypaste it here.
if(BUNDLE_TOOLCHAIN)
	find_program(
		_gcc ${PSN00BSDK_TARGET}-gcc
		HINTS
			${PSN00BSDK_TC}/bin
			${PSN00BSDK_TC}/../bin
		PATHS
			"C:/Program Files/${PSN00BSDK_TARGET}/bin"
			"C:/Program Files (x86)/${PSN00BSDK_TARGET}/bin"
			"C:/${PSN00BSDK_TARGET}/bin"
			/opt/${PSN00BSDK_TARGET}/bin
			/usr/local/${PSN00BSDK_TARGET}/bin
			/usr/${PSN00BSDK_TARGET}/bin
		NO_CACHE REQUIRED
	)
	cmake_path(GET _gcc PARENT_PATH _bin)
	cmake_path(GET _bin PARENT_PATH _toolchain)

	# Check if the toolchain is actually part of an existing PSn00bSDK install.
	# If so, disable bundling as we can't determine which files are part of the
	# toolchain and which ones are part of the SDK.
	if(EXISTS ${_bin}/elf2x${CMAKE_EXECUTABLE_SUFFIX})
		message(FATAL_ERROR "${_toolchain} contains a full PSn00bSDK installation. To bundle the toolchain, set PSN00BSDK_TC to point to a directory containing only the toolchain files.")
	else()
		install(
			DIRECTORY   ${_toolchain}/
			DESTINATION .
			COMPONENT   toolchain
			USE_SOURCE_PERMISSIONS
			#EXCLUDE_FROM_ALL
		)
	endif()
endif()

if(BUNDLE_CMAKE)
	cmake_path(GET CMAKE_COMMAND PARENT_PATH _bin)
	cmake_path(GET _bin          PARENT_PATH _cmakedir)

	# Bundling CMake is only allowed on Windows, both because finding CMake
	# installation files is difficult on Linux and because it's better to
	# specify CMake as a DEB/RPM dependency anyway.
	if(NOT WIN32)
		message(FATAL_ERROR "Bundling CMake into installers is only supported (and should only be done) on Windows.")
	else()
		install(
			DIRECTORY   ${_cmakedir}/
			DESTINATION .
			COMPONENT   cmake
			USE_SOURCE_PERMISSIONS
			#EXCLUDE_FROM_ALL
		)
	endif()
endif()

## Variables common to all package types

if(NOT DEFINED CPACK_GENERATOR)
	if(WIN32)
		set(CPACK_GENERATOR ZIP NSIS)
	elseif(APPLE)
		# TODO: add a macOS installer and related options
		set(CPACK_GENERATOR ZIP)
	elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		set(CPACK_GENERATOR ZIP DEB RPM)
	else()
		set(CPACK_GENERATOR ZIP)
	endif()
endif()

set(CPACK_VERBATIM_VARIABLES        ON)
set(CPACK_ARCHIVE_THREADS           0)
set(CPACK_PACKAGE_DIRECTORY         ${PROJECT_BINARY_DIR}/packages)
set(CPACK_PACKAGE_NAME              PSn00bSDK)
set(CPACK_PACKAGE_VENDOR            Lameguy64)
set(CPACK_PACKAGE_CONTACT           Lameguy64)
set(CPACK_RESOURCE_FILE_README      ${PROJECT_SOURCE_DIR}/README.md)
set(CPACK_RESOURCE_FILE_LICENSE     ${PROJECT_SOURCE_DIR}/LICENSE.md)
set(CPACK_PACKAGE_ICON              ${CMAKE_CURRENT_LIST_DIR}/icon.ico)
set(CPACK_PACKAGE_DESCRIPTION_FILE  ${CMAKE_CURRENT_LIST_DIR}/description.txt)
set(CPACK_PRE_BUILD_SCRIPTS         ${CMAKE_CURRENT_LIST_DIR}/fakeroot_fix.cmake)
set(CPACK_PACKAGE_INSTALL_DIRECTORY PSn00bSDK)

## DEB/RPM variables

set(CPACK_DEBIAN_PACKAGE_DEPENDS    "libc6 (>= 2.28), cmake (>= 3.20)")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "ninja-build (>= 1.10)")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS   "git (>= 2.25)")
set(CPACK_DEBIAN_PACKAGE_SECTION    devel)
set(CPACK_RPM_PACKAGE_REQUIRES      "cmake >= 3.20")
set(CPACK_RPM_PACKAGE_SUGGESTS      "ninja-build >= 1.10, git >= 2.25")
#set(CPACK_RPM_PACKAGE_RELOCATABLE   ON)

## NSIS variables

set(CPACK_NSIS_MUI_ICON                        ${CMAKE_CURRENT_LIST_DIR}/icon.ico)
set(CPACK_NSIS_MUI_UNIICON                     ${CMAKE_CURRENT_LIST_DIR}/uninstall.ico)
set(CPACK_NSIS_MUI_HEADERIMAGE                 ${CMAKE_CURRENT_LIST_DIR}/nsis_header.bmp)
set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP    ${CMAKE_CURRENT_LIST_DIR}/nsis_banner.bmp)
set(CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP  ${CMAKE_CURRENT_LIST_DIR}/nsis_banner.bmp)
set(CPACK_NSIS_BRANDING_TEXT                   "PSn00bSDK - Meido-Tek Productions")
set(CPACK_NSIS_URL_INFO_ABOUT                  "${PROJECT_HOMEPAGE_URL}")
set(CPACK_NSIS_MODIFY_PATH                     ON)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(
	CPACK_NSIS_MENU_LINKS
	"${PROJECT_HOMEPAGE_URL}"                "About PSn00bSDK"
	"https://github.com/Lameguy64/PSn00bSDK" "GitHub repo"
	"Uninstall.exe"                          "Uninstall PSn00bSDK"
)

# Paths in CPACK_NSIS_* variables are not converted to native paths by CMake
# for some reason (and NSIS doesn't like paths with forward slashes), so we
# have to do it manually.
foreach(
	_var IN ITEMS
		CPACK_NSIS_MUI_ICON
		CPACK_NSIS_MUI_UNIICON
		CPACK_NSIS_MUI_HEADERIMAGE
		CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP
		CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP
)
	cmake_path(NATIVE_PATH ${_var} ${_var})
endforeach()

## Component setup

# This must be done after setting CPack-related variables.
include(CPack)

cpack_add_component(
	sdk
	DISPLAY_NAME "SDK libraries and tools"
	DESCRIPTION  "These files are always required and their installation cannot be skipped."
	REQUIRED
)
cpack_add_component(
	docs
	DISPLAY_NAME "SDK documentation"
	DESCRIPTION  "Select to install additional documentation files and a project template (recommended)."
)
cpack_add_component(
	examples
	DISPLAY_NAME "SDK examples"
	DESCRIPTION  "Select to copy the examples' source code to the documentation folder (recommended)."
)

if(BUNDLE_TOOLCHAIN)
	cpack_add_component(
		toolchain
		DISPLAY_NAME "GCC MIPS toolchain"
		DESCRIPTION  "Do not skip unless you already have a toolchain that targets ${PSN00BSDK_TARGET} installed."
	)
endif()
if(BUNDLE_CMAKE)
	cpack_add_component(
		cmake
		DISPLAY_NAME "CMake ${CMAKE_VERSION}"
		DESCRIPTION  "Select this if you do not have CMake installed already. Note that CMake will be installed in the same directory as PSn00bSDK."
		DISABLED
	)
endif()
