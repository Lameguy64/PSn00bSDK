# PSn00bSDK project setup file
# Part of the PSn00bSDK Project
# 2019 - 2021 Lameguy64 / Meido-Tek Productions
#
# This file does not depend on any other files (besides paths specified via
# environment variables) and may be copied for use with your projects. See the
# template directory for a makefile template.

#PREFIX	?= mipsel-none-elf
PREFIX	?= mipsel-unknown-elf

## Path setup

# PSn00bSDK library/include path setup
ifndef PSN00BSDK_LIBS
	# Default assumes PSn00bSDK is in the same parent dir as this project
	LIBDIRS	= -L../libpsn00b
	INCLUDE	= -I../libpsn00b/include -I../libpsn00b/lzp
	LDBASE	= ../libpsn00b/ldscripts
else
	LIBDIRS	= -L$(PSN00BSDK_LIBS)
	INCLUDE	= -I$(PSN00BSDK_LIBS)/include -I$(PSN00BSDK_LIBS)/lzp
	LDBASE	= ${PSN00BSDK_LIBS}/ldscripts
endif

# PSn00bSDK toolchain path setup
ifndef PSN00BSDK_TC
	# Default assumes GCC toolchain is in root of C drive or /usr/local
	ifeq "$(OS)" "Windows_NT"
		GCC_BASE	?= /c/$(PREFIX)
		GCC_BIN		?=
	else
		GCC_BASE	?= /usr/local/$(PREFIX)
		GCC_BIN		?=
	endif
else
	GCC_BASE	?= $(PSN00BSDK_TC)
	GCC_BIN		?= $(PSN00BSDK_TC)/bin/
endif

# Autodetect GCC version by folder name (ugly but it works, lol)
#GCC_VERSION	?= 7.4.0
GCC_VERSION	?= $(word 1, $(notdir $(wildcard $(GCC_BASE)/lib/gcc/$(PREFIX)/*)))

# PSn00bSDK tools path setup (TODO)
PSN00BSDK_BIN	?=

## Commands

# GCC toolchain
CC	= $(GCC_BIN)$(PREFIX)-gcc
CXX	= $(GCC_BIN)$(PREFIX)-g++
AS	= $(GCC_BIN)$(PREFIX)-as
AR	= $(GCC_BIN)$(PREFIX)-ar
LD	= $(GCC_BIN)$(PREFIX)-ld
RANLIB	= $(GCC_BIN)$(PREFIX)-ranlib
OBJCOPY	= $(GCC_BIN)$(PREFIX)-objcopy
NM	= $(GCC_BIN)$(PREFIX)-nm

# PSn00bSDK tools + mkpsxiso
ELF2X		= $(PSN00BSDK_BIN)elf2x
LZPACK		= $(PSN00BSDK_BIN)lzpack
SMXLINK		= $(PSN00BSDK_BIN)smxlink
MKPSXISO	= $(PSN00BSDK_BIN)mkpsxiso

## Flags

# SDK libraries (IMPORTANT: don't change the order)
LIBS	= -lpsxgpu -lpsxgte -lpsxspu -lpsxcd -lpsxsio -lpsxetc -lpsxapi -lc

# Common options:
# - Debugging symbols enabled
# - Wrap each symbol in a separate section
# - Optimize for R3000, no FPU, 32-bit ABI
# - Division by zero causes break opcodes to be executed
# - C standard library (including libgcc) disabled
# - C++ features that rely on runtime support disabled
AFLAGS	= -g -msoft-float -march=r3000 -mtune=r3000 -mabi=32
CFLAGS	= $(AFLAGS) -mdivide-breaks -O2 -ffreestanding -fno-builtin -nostdlib \
		-fdata-sections -ffunction-sections -fsigned-char -fno-strict-overflow
CPPFLAGS= $(CFLAGS) -fno-exceptions -fno-rtti -fno-unwind-tables \
		-fno-threadsafe-statics -fno-use-cxa-atexit
LDFLAGS	= -nostdlib

# Options for static libraries (and SDK libraries):
# - GP-relative addressing disabled
# - ABI-compatible calls disabled
# - Local stripping enabled
AFLAGS_LIB	= $(AFLAGS) -G0 -Wa,--strip-local-absolute
CFLAGS_LIB	= $(CFLAGS) -G0 -mno-abicalls -mno-gpopt
CPPFLAGS_LIB	= $(CPPFLAGS) -G0 -mno-abicalls -mno-gpopt

# Options for executables without support for dynamic linking:
# - Position-independent code disabled
# - GP-relative addressing enabled only for local symbols
# - ABI-compatible calls disabled (incompatible with GP-relative addressing)
# - Unused section stripping enabled
AFLAGS_EXE	= $(AFLAGS) -G8
CFLAGS_EXE	= $(CFLAGS) -G8 -mno-abicalls -mgpopt -mno-extern-sdata
CPPFLAGS_EXE	= $(CPPFLAGS) -G8 -mno-abicalls -mgpopt -mno-extern-sdata
LDFLAGS_EXE	= $(LDFLAGS) -G8 -static -T$(LDBASE)/exe.ld -gc-sections

# Options for executables with support for dynamic linking:
# - Position-independent code disabled
# - GP-relative addressing disabled
# - ABI-compatible calls disabled (must be done manually)
# - Unused section stripping enabled
AFLAGS_EXEDYN	= $(AFLAGS) -G0
CFLAGS_EXEDYN	= $(CFLAGS) -G0 -mno-abicalls -mno-gpopt
CPPFLAGS_EXEDYN	= $(CPPFLAGS) -G0 -mno-abicalls -mno-gpopt
LDFLAGS_EXEDYN	= $(LDFLAGS) -G0 -static -T$(LDBASE)/exe.ld -gc-sections

# Options for dynamically-loaded libraries:
# - Position-independent code enabled
# - GP-relative addressing disabled (incompatible with ABI calls)
# - ABI-compatible calls enabled
# - Unused section stripping not available
AFLAGS_DLL	= $(AFLAGS) -G0
CFLAGS_DLL	= $(CFLAGS) -G0 -mabicalls -mshared -mno-gpopt -fPIC
CPPFLAGS_DLL	= $(CPPFLAGS) -G0 -mabicalls -mshared -mno-gpopt -fPIC
LDFLAGS_DLL	= $(LDFLAGS) -G0 -shared -T$(LDBASE)/dll.ld
