# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{PSN00BSDK_LIBS})
	set(CMAKE_TOOLCHAIN_FILE $ENV{PSN00BSDK_LIBS}/cmake/sdk.cmake)
endif()

project(
	cppdemo
	LANGUAGES    CXX
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK basic C++ example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.cpp)
psn00bsdk_add_executable(cppdemo STATIC ${_sources})
#psn00bsdk_add_cd_image(cppdemo_iso cppdemo iso.xml DEPENDS cppdemo)

install(FILES ${PROJECT_BINARY_DIR}/cppdemo.exe DESTINATION .)
