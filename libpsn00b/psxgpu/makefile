# PSn00bSDK library makefile
# Part of the PSn00bSDK Project
# 2019 - 2021 Lameguy64 / Meido-Tek Productions

## Settings

PSN00BSDK_LIBS	?= ..

include ../../psn00bsdk-setup.mk

# Project target name
TARGET	= libpsxgpu.a

## Files

# Searches for C, C++ and S (assembler) files in local directory
CFILES	= $(notdir $(wildcard *.c))
CXXFILES= $(notdir $(wildcard *.cxx))
AFILES	= $(notdir $(wildcard *.s))

# Create names for object files
OFILES	= $(addprefix build/,$(CFILES:.c=.o)) \
	$(addprefix build/,$(CXXFILES:.cxx=.o)) \
	$(addprefix build/,$(AFILES:.s=.o))

# Project specific includes and libraries
# (use -I for include dirs, -L for library dirs, -l for static libraries)
INCLUDE	+=
LIBDIRS	+=
LIBS	+=

## Build rules

all: build/$(TARGET)

build/$(TARGET): $(OFILES)
	@mkdir -p $(dir $@)
	$(AR) crs $@ $(OFILES)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_LIB) $(INCLUDE) -c $< -o $@

build/%.o: %.cxx
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS_LIB) $(INCLUDE) -c $< -o $@
	
build/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS_LIB) $(INCLUDE) -c $< -o $@

install:
ifneq ($(PSN00BSDK_LIBS), "..")
	@mkdir -p $(PSN00BSDK_LIBS)
endif
	cp build/$(TARGET) $(PSN00BSDK_LIBS)/$(TARGET)
	
clean:
	rm -rf build
