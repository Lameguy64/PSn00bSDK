# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.20)

project(
	PSn00bSDK-template
	LANGUAGES    C CXX ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK template"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

psn00bsdk_add_executable(template STATIC main.c)

psn00bsdk_add_cd_image(
	iso      # Target name
	template # Output file name (= template.bin + template.cue)
	iso.xml  # Path to config file
	DEPENDS template
)
