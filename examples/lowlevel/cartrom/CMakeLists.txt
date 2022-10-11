# PSn00bSDK example CMake script
# (C) 2021 spicyjpeg - MPL licensed

cmake_minimum_required(VERSION 3.21)

project(
	cartrom
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PSn00bSDK expansion port ROM example"
	HOMEPAGE_URL "http://lameguy64.net/?page=psn00bsdk"
)

file(GLOB _sources *.c *.s)

# This example only uses the toolchain (without the rest of the SDK), so the
# executable has to be created manually and converted into raw binary format
# (for testing on emulators or flashing to a cheat cartridge).
add_executable       (cartrom ${_sources})
target_link_options  (cartrom PRIVATE -T${PROJECT_SOURCE_DIR}/rom.ld)
set_target_properties(
	cartrom PROPERTIES
	PREFIX ""
	SUFFIX ".elf"
	PSN00BSDK_TARGET_TYPE EXECUTABLE_NOGPREL
)

target_include_directories(cartrom PRIVATE ${PROJECT_SOURCE_DIR})

add_custom_command(
	TARGET     cartrom POST_BUILD
	COMMAND    ${CMAKE_OBJCOPY} -O binary cartrom.elf cartrom.bin
	BYPRODUCTS cartrom.bin
)

install(FILES ${PROJECT_BINARY_DIR}/cartrom.bin TYPE BIN)
