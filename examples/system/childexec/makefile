# PSn00bSDK makefile template
# Part of the PSn00bSDK Project
# 2019 - 2021 Lameguy64 / Meido-Tek Productions

## Settings

PSN00BSDK_LIBS	?= ../../../libpsn00b

# Edit this to point to psn00bsdk-setup.mk, or copy that over to your project's
# root folder (it only depends on environment variables).
include ../../../psn00bsdk-setup.mk

# Project target name
#TARGET	= childexec

## Files

# Searches for C, C++ and S (assembler) files in local directory
CFILES_PARENT	= $(notdir $(wildcard *.c))
CPPFILES_PARENT	= $(notdir $(wildcard *.cpp))
AFILES_PARENT	= $(notdir $(wildcard *.s))

CFILES_CHILD	= $(notdir $(wildcard child/*.c))
CPPFILES_CHILD	= $(notdir $(wildcard child/*.cpp))
AFILES_CHILD	= $(notdir $(wildcard child/*.s))

# Create names for object files
OFILES_PARENT	= $(addprefix build/,$(CFILES_PARENT:.c=.o)) \
	$(addprefix build/,$(CPPFILES_PARENT:.cpp=.o)) \
	$(addprefix build/,$(AFILES_PARENT:.s=.o))
OFILES_CHILD	= $(addprefix build/child/,$(CFILES_CHILD:.c=.o)) \
	$(addprefix build/child/,$(CPPFILES_CHILD:.cpp=.o)) \
	$(addprefix build/child/,$(AFILES_CHILD:.s=.o))

# Project specific includes and libraries
# (use -I for include dirs, -L for library dirs, -l for static libraries)
INCLUDE	+=
LIBDIRS	+=
LIBS	+=

# Relocate the child executable to 0x30000
LDFLAGS_CHILD	= -Ttext=0x80030000

## Build rules

#all: iso
all: build/child/child build/parent

iso: build/child/child build/parent resources
	$(MKPSXISO) -y -q iso.xml

resources:
	# Add commands to build/convert your assets here
	#$(LZPACK) data.xml

build/child/child: $(OFILES_CHILD)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS_EXE) $(LDFLAGS_CHILD) $(LIBDIRS) $^ $(LIBS) -o $@
	$(NM) -f posix -l -n $@ >$@.map
	$(ELF2X) -q $@ $@.exe

build/parent: $(OFILES_PARENT)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS_EXE) $(LIBDIRS) $^ $(LIBS) -o $@
	$(NM) -f posix -l -n $@ >$@.map
	$(ELF2X) -q $@ $@.exe

build/child/%.o: child/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_EXE) $(INCLUDE) -c $< -o $@

build/child/%.o: child/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS_EXE) $(INCLUDE) -c $< -o $@

build/child/%.o: child/%.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS_EXE) $(INCLUDE) -c $< -o $@

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
