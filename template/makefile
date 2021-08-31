# PSn00bSDK makefile template
# Part of the PSn00bSDK Project
# 2019 - 2021 Lameguy64 / Meido-Tek Productions

## Settings

# You can edit these here or pass them as environment variables.
#PREFIX			=
#GCC_VERSION	=
#PSN00BSDK_TC	=
#PSN00BSDK_LIBS	=

# Edit this to point to psn00bsdk-setup.mk, or copy that over to your project's
# root folder (it only depends on environment variables).
include ../psn00bsdk-setup.mk

# Project target name
TARGET	= template

## Files

# Searches for C, C++ and S (assembler) files in local directory
CFILES	= $(notdir $(wildcard *.c))
CPPFILES= $(notdir $(wildcard *.cpp))
AFILES	= $(notdir $(wildcard *.s))

# Create names for object files
OFILES	= $(addprefix build/,$(CFILES:.c=.o)) \
	$(addprefix build/,$(CPPFILES:.cpp=.o)) \
	$(addprefix build/,$(AFILES:.s=.o))

# Project specific includes and libraries
# (use -I for include dirs, -L for library dirs, -l for static libraries)
INCLUDE	+=
LIBDIRS	+=
LIBS	+=

## Build rules

all: iso
#all: build/$(TARGET)

iso: build/$(TARGET) resources
	$(MKPSXISO) -y -q iso.xml

resources:
	# Add commands to build/convert your assets here
	#$(LZPACK) data.xml

build/$(TARGET): $(OFILES)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS_EXE) $(LIBDIRS) $^ $(LIBS) -o $@
	$(NM) -f posix -l -n $@ >$@.map
	$(ELF2X) -q $@ $@.exe

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_EXE) $(INCLUDE) -c $< -o $@

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS_EXE) $(INCLUDE) -c $< -o $@

build/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS_EXE) $(INCLUDE) -c $< -o $@

clean:
	rm -rf build
