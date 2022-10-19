# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	n00bdemo
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "n00bdemo (PSn00bSDK demo)"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

set(DATA_DIR ${PROJECT_SOURCE_DIR}/data)

configure_file(data.xml _data.xml)

# Add a build step to pack assets into a single .LZP file. Note that, since we
# are specifying dependencies, CMake can detect when source assets are edited
# and rebuild the archive automatically.
file(GLOB _assets ${DATA_DIR}/*.tim ${DATA_DIR}/*.smd)
add_custom_command(
	COMMAND    ${LZPACK} -y _data.xml
	OUTPUT     data.lzp
	BYPRODUCTS textures.qlp
	DEPENDS    ${_assets}
	COMMENT    "Building LZP archive"
)

file(GLOB _sources *.s *.c)
psn00bsdk_add_executable(n00bdemo GPREL ${_sources})
#psn00bsdk_add_cd_image(n00bdemo_iso n00bdemo iso.xml DEPENDS n00bdemo)

target_include_directories(n00bdemo PRIVATE ${PROJECT_SOURCE_DIR})
psn00bsdk_target_incbin(
	n00bdemo PRIVATE _lz_resources
	${PROJECT_BINARY_DIR}/data.lzp
)

install(FILES ${PROJECT_BINARY_DIR}/n00bdemo.exe TYPE BIN)
