# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{PSN00BSDK_LIBS})
	set(CMAKE_TOOLCHAIN_FILE $ENV{PSN00BSDK_LIBS}/cmake/sdk.cmake)
endif()

project(
	n00bdemo
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "n00bdemo (PSn00bSDK demo)"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

set(DATA_DIR ${PROJECT_SOURCE_DIR}/data)

configure_file(data.s.template data.s)
configure_file(data.xml.template data.xml)

# Add a build step to pack assets into a single .LZP file. This archive is then
# embedded into the binary through data.s, which is in turn generated from
# data.s.template. Note that, since we specify dependencies, CMake can detect
# when source assets are edited and rebuild the archive automatically.
file(GLOB _assets ${DATA_DIR}/*.tim ${DATA_DIR}/*.smd)
add_custom_command(
	COMMAND    ${LZPACK} -y data.xml
	OUTPUT     data.lzp
	BYPRODUCTS textures.qlp
	DEPENDS    ${_assets}
	COMMENT    "Building LZP archive"
)

file(GLOB _sources *.s *.c)
psn00bsdk_add_executable(
	n00bdemo STATIC
	${_sources}
	${PROJECT_BINARY_DIR}/data.s
)
target_include_directories(n00bdemo PRIVATE ${PROJECT_SOURCE_DIR})
#psn00bsdk_add_cd_image(n00bdemo_iso n00bdemo iso.xml DEPENDS n00bdemo)

# Ensure data.lzp is built before the executable. Due to CMake's limitations we
# actually have to wrap it in a dummy target.
add_custom_target(n00bdemo_data DEPENDS data.lzp)
add_dependencies(n00bdemo n00bdemo_data)

install(FILES ${PROJECT_BINARY_DIR}/n00bdemo.exe DESTINATION .)
