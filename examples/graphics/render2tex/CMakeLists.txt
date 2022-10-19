# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	render2tex
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK render-to-texture example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(render2tex GPREL ${_sources})
#psn00bsdk_add_cd_image(render2tex_iso render2tex iso.xml DEPENDS render2tex)

psn00bsdk_target_incbin(render2tex PRIVATE tim_blendpattern blendpattern-16c.tim)

install(FILES ${PROJECT_BINARY_DIR}/render2tex.exe TYPE BIN)
