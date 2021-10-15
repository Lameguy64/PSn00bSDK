# PSn00bSDK main build script
# (C) 2021 spicyjpeg - MPL licensed

# NOTE: CMake doesn't support using multiple toolchains in a single project,
# so we can't use add_subdirectory() to build both the libraries and tools. A
# workaround is to use ExternalProject_Add() to launch multiple independent
# CMake instances, creating what's known as a "superbuild".

cmake_minimum_required(VERSION 3.21)
include(ExternalProject)

project(
	PSn00bSDK
	LANGUAGES    NONE
	# IMPORTANT TODO: set a version number
	VERSION      0.1.0
	DESCRIPTION  "Open source PlayStation 1 SDK"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

# Including this without initializing at least one language throws a warning and
# there's no way to mute it.
include(GNUInstallDirs)

# These are passed through to libpsn00b and the examples (they are defined in
# the toolchain file).
set(
	PSN00BSDK_TC $ENV{PSN00BSDK_TC}
	CACHE PATH   "Path to the GCC toolchain's installation directory"
)
set(
	PSN00BSDK_TARGET mipsel-unknown-elf
	CACHE STRING     "GCC toolchain target triplet"
)

set(
	SKIP_DOWNLOAD OFF
	CACHE BOOL    "Skip downloading and building tinyxml2 and mkpsxiso"
)
set(
	SKIP_EXAMPLES OFF
	CACHE BOOL    "Skip building SDK examples (not required for installation)"
)

# Forward some important variables to mkpsxiso and to the subprojects (they are
# not inherited automatically as they are not environment variables). This also
# sets all subprojects to "install" everything to a temporary directory in the
# build tree, so they don't actually get installed until "cmake --install" is
# invoked (ExternalProject_Add() runs the subprojects' install step at build
# time).
set(
	SUBPROJECT_ARGS
	-DPSN00BSDK_TC:PATH=${PSN00BSDK_TC}
	-DPSN00BSDK_TARGET:STRING=${PSN00BSDK_TARGET}
	-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}
	-DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_BINARY_DIR}/install_tree
	-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
)
set(
	EXAMPLES_ARGS
	-DPSN00BSDK_TC:PATH=${PSN00BSDK_TC}
	-DPSN00BSDK_TARGET:STRING=${PSN00BSDK_TARGET}
	-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${PROJECT_BINARY_DIR}/install_tree/${CMAKE_INSTALL_LIBDIR}/libpsn00b/cmake/sdk.cmake
	-DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_BINARY_DIR}/examples
	-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
)

## External dependencies

if(NOT SKIP_DOWNLOAD)
	list(APPEND SUBPROJECT_ARGS -Dtinyxml2_ROOT:PATH=${PROJECT_BINARY_DIR}/install_temp)

	ExternalProject_Add(
		tinyxml2
		GIT_REPOSITORY   "https://github.com/leethomason/tinyxml2"
		CMAKE_CACHE_ARGS
			-DCMAKE_INSTALL_PREFIX:PATH=${PROJECT_BINARY_DIR}/install_temp
			-DCMAKE_MSVC_RUNTIME_LIBRARY:STRING=MultiThreaded$<$<CONFIG:Debug>:Debug>
		INSTALL_DIR      install_temp
	)
	ExternalProject_Add(
		mkpsxiso
		# IMPORTANT TODO: migrate to Lameguy64/mkpsxiso once PR #18 is merged
		GIT_REPOSITORY   "https://github.com/Lameguy64/mkpsxiso"
		GIT_TAG          cmake
		#GIT_REPOSITORY   "https://github.com/Lameguy64/mkpsxiso"
		CMAKE_CACHE_ARGS ${SUBPROJECT_ARGS}
		INSTALL_DIR      install_tree
		DEPENDS          tinyxml2
	)
else()
	list(APPEND SUBPROJECT_ARGS -Dtinyxml2_ROOT:PATH=${tinyxml2_ROOT})

	# Create dummy targets so CMake doesn't throw missing dependency errors.
	add_library(tinyxml2 INTERFACE)
	add_library(mkpsxiso INTERFACE)
endif()

## Subprojects

ExternalProject_Add(
	libpsn00b
	SOURCE_DIR       ${PROJECT_SOURCE_DIR}/libpsn00b
	CMAKE_CACHE_ARGS ${SUBPROJECT_ARGS}
	INSTALL_DIR      install_tree
)
ExternalProject_Add(
	tools
	SOURCE_DIR       ${PROJECT_SOURCE_DIR}/tools
	CMAKE_CACHE_ARGS ${SUBPROJECT_ARGS}
	INSTALL_DIR      install_tree
	DEPENDS          tinyxml2
)
ExternalProject_Add(
	examples
	SOURCE_DIR       ${PROJECT_SOURCE_DIR}/examples
	CMAKE_CACHE_ARGS ${EXAMPLES_ARGS}
	INSTALL_DIR      install_tree
	DEPENDS          libpsn00b tools mkpsxiso
	EXCLUDE_FROM_ALL ${SKIP_EXAMPLES}
)

# Install all files in the temporary installation tree, as well as static files
# from the source tree, when "cmake --install" is invoked.
install(
	DIRECTORY   ${PROJECT_BINARY_DIR}/install_tree/ # THE TRAILING SLASH IS IMPORTANT
	DESTINATION .
	USE_SOURCE_PERMISSIONS
)
install(
	DIRECTORY   doc template
	DESTINATION ${CMAKE_INSTALL_DATADIR}/psn00bsdk
)

## CPack configuration

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

set(CPACK_PACKAGE_DIRECTORY        ${PROJECT_BINARY_DIR}/cpack)
set(CPACK_PACKAGE_NAME             PSn00bSDK)
set(CPACK_PACKAGE_VENDOR           Lameguy64)
set(CPACK_PACKAGE_CONTACT          Lameguy64)
set(CPACK_PACKAGE_ICON             ${PROJECT_SOURCE_DIR}/cpack/icon.ico)
set(CPACK_PACKAGE_DESCRIPTION_FILE ${PROJECT_SOURCE_DIR}/cpack/description.txt)
set(CPACK_RESOURCE_FILE_WELCOME    ${PROJECT_SOURCE_DIR}/cpack/welcome.txt)
set(CPACK_RESOURCE_FILE_README     ${PROJECT_SOURCE_DIR}/README.md)
set(CPACK_RESOURCE_FILE_LICENSE    ${PROJECT_SOURCE_DIR}/LICENSE.md)
set(CPACK_PRE_BUILD_SCRIPTS        ${PROJECT_SOURCE_DIR}/cpack/fakeroot_fix.cmake)

set(CPACK_DEBIAN_PACKAGE_DEPENDS  "libc6 (>= 2.28), cmake (>= 3.21), gcc-mipsel-unknown-elf")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "git")
set(CPACK_DEBIAN_PACKAGE_SECTION  devel)
set(CPACK_RPM_PACKAGE_REQUIRES    "cmake >= 3.21, gcc-mipsel-unknown-elf")
set(CPACK_RPM_PACKAGE_SUGGESTS    "git")
#set(CPACK_RPM_PACKAGE_RELOCATABLE ON)

set(CPACK_NSIS_MUI_ICON                       ${PROJECT_SOURCE_DIR}/cpack/icon.ico)
set(CPACK_NSIS_MUI_UNIICON                    ${PROJECT_SOURCE_DIR}/cpack/uninstall.ico)
set(CPACK_NSIS_MUI_HEADERIMAGE                ${PROJECT_SOURCE_DIR}/cpack/nsis_header.bmp)
set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP   ${PROJECT_SOURCE_DIR}/cpack/nsis_banner.bmp)
set(CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP ${PROJECT_SOURCE_DIR}/cpack/nsis_banner.bmp)
set(CPACK_NSIS_BRANDING_TEXT                  "PSn00bSDK - Meido-Tek Productions")
set(CPACK_NSIS_URL_INFO_ABOUT                 "${PROJECT_HOMEPAGE_URL}")
set(CPACK_NSIS_MODIFY_PATH                    ON)
set(
	CPACK_NSIS_MENU_LINKS
	"${PROJECT_HOMEPAGE_URL}"                "About PSn00bSDK"
	"https://github.com/Lameguy64/PSn00bSDK" "GitHub repo"
)

# This will generate a CPack configuration file and add a "package" target to
# launch CPack.
include(CPack)
