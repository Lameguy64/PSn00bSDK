# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	vagsample
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK SPU sound playback example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(vagsample GPREL ${_sources})
#psn00bsdk_add_cd_image(vagsample_iso vagsample iso.xml DEPENDS vagsample)

psn00bsdk_target_incbin(vagsample PRIVATE proyt proyt.vag)
psn00bsdk_target_incbin(vagsample PRIVATE tdfx  3dfx.vag)

install(FILES ${PROJECT_BINARY_DIR}/vagsample.exe TYPE BIN)
