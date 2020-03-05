# Adjustable common makefile values for PSn00bSDK example programs.
# You may need to modify these values to match with your toolchain setup.

# Toolchain prefix
PREFIX		= mipsel-unknown-elf-

# Include directories
INCLUDE	 	= -I../../libpsn00b/include

# Library directories, last entry must point toolchain libraries
LIBDIRS		= -L../../libpsn00b

ifndef GCC_VERSION

GCC_VERSION	= 8.2.0

endif

ifndef GCC_BASE

ifeq "$(OS)" "Windows_NT"	# For Windows

GCC_BASE	= /c/mipsel-unknown-elf

else						# For Linux/BSDs

GCC_BASE	= /usr/local/mipsel-unknown-elf

endif

CC			= $(GCC_BASE)/bin/$(PREFIX)gcc
CXX			= $(GCC_BASE)/bin/$(PREFIX)g++
AS			= $(GCC_BASE)/bin/$(PREFIX)as
LD			= $(GCC_BASE)/bin/$(PREFIX)ld

endif

LIBDIRS 	+= -L$(GCC_BASE)/lib/gcc/mipsel-unknown-elf/$(GCC_VERSION)
INCLUDE	 	+= -I$(GCC_BASE)/lib/gcc/mipsel-unknown-elf/$(GCC_VERSION)/include
