# PSn00bSDK examples setup file
# Part of the PSn00bSDK Project
# 2019 - 2020 Lameguy64 / Meido-Tek Productions
#
# This is only for the PSn00bSDK example programs, not recommended
# for use with user projects

PREFIX		= mipsel-unknown-elf-

ifndef GCC_VERSION

GCC_VERSION	= 7.4.0

endif	# GCC_VERSION

# PSn00bSDK library/include path setup
ifndef PSN00BSDK_LIBS

# Default assumes libpsn00b is just in the parent dir of the examples dir

LIBDIRS		= -L../../../libpsn00b
INCLUDE	 	= -I../../../libpsn00b/include

else

LIBDIRS		= -L$(PSN00BSDK_LIBS)
INCLUDE		= -I$(PSN00BSDK_LIBS)/include

endif 		# PSN00BSDK_LIBS

# PSn00bSDK toolchain path setup
ifndef GCC_BASE

ifndef PSN00BSDK_TC

# Default assumes GCC toolchain is in root of C drive or /usr/local

ifeq "$(OS)" "Windows_NT"

GCC_BASE	= /c/mipsel-unknown-elf
GCC_BIN		=

else

GCC_BASE	= /usr/local/mipsel-unknown-elf
GCC_BIN		=

endif

else

GCC_BASE	= $(PSN00BSDK_TC)
GCC_BIN		= $(PSN00BSDK_TC)/bin/

endif		# PSN00BSDK_TC

endif		# GCC_BASE

CC			= $(GCC_BIN)$(PREFIX)gcc
CXX			= $(GCC_BIN)$(PREFIX)g++
AS			= $(GCC_BIN)$(PREFIX)as
AR			= $(GCC_BIN)$(PREFIX)ar
LD			= $(GCC_BIN)$(PREFIX)ld
RANLIB		= $(GCC_BIN)$(PREFIX)ranlib