# PSn00bSDK makefile template
# Part of the PSn00bSDK Project
# 2019 - 2021 Lameguy64 / Meido-Tek Productions

## Settings

PSN00BSDK_LIBS	?= ../../../libpsn00b

# Edit this to point to psn00bsdk-setup.mk, or copy that over to your project's
# root folder (it only depends on environment variables).
include ../../../psn00bsdk-setup.mk

# Project target name
#TARGET	= dynlink

## Files

# Searches for C, C++ and S (assembler) files in local directory
CFILES_MAIN		= $(notdir $(wildcard *.c))
CPPFILES_MAIN	= $(notdir $(wildcard *.cpp))
AFILES_MAIN		= $(notdir $(wildcard *.s))

CFILES_DLL		= $(notdir $(wildcard library/*.c))
CPPFILES_DLL	= $(notdir $(wildcard library/*.cpp))
AFILES_DLL		= $(notdir $(wildcard library/*.s))

# Create names for object files
OFILES_MAIN		= $(addprefix build/,$(CFILES_MAIN:.c=.o)) \
		$(addprefix build/,$(CPPFILES_MAIN:.cpp=.o)) \
		$(addprefix build/,$(AFILES_MAIN:.s=.o))
OFILES_CUBE		= build/library/cube.o
OFILES_BALLS	= build/library/balls.o

# Project specific includes and libraries
# (use -I for include dirs, -L for library dirs, -l for static libraries)
INCLUDE	+=
LIBDIRS	+=
LIBS	+=

## Build rules

all: iso
#all: build/library/cube build/library/balls build/main

iso: build/library/cube build/library/balls build/main resources
	$(MKPSXISO) -y -q iso.xml

resources:
	# Add commands to build/convert your assets here
	#$(LZPACK) data.xml

build/library/cube: $(OFILES_CUBE)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS_DLL) $^ -o $@
	#$(NM) -f posix -l -n $@ >$@.map
	$(OBJCOPY) -O binary $@ $@.dll

build/library/balls: $(OFILES_BALLS)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS_DLL) $^ -o $@
	#$(NM) -f posix -l -n $@ >$@.map
	$(OBJCOPY) -O binary $@ $@.dll

build/main: $(OFILES_MAIN)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS_EXEDYN) $(LIBDIRS) $^ $(LIBS) -o $@
	$(NM) -f posix -l -n $@ >$@.map
	$(ELF2X) -q $@ $@.exe

build/library/%.o: library/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_DLL) $(INCLUDE) -c $< -o $@

build/library/%.o: library/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS_DLL) $(INCLUDE) -c $< -o $@

build/library/%.o: library/%.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS_DLL) $(INCLUDE) -c $< -o $@

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_EXEDYN) $(INCLUDE) -c $< -o $@

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS_EXEDYN) $(INCLUDE) -c $< -o $@

build/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS_EXEDYN) $(INCLUDE) -c $< -o $@

clean:
	rm -rf build
