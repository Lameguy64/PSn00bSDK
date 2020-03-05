# Adjustable common makefile values for PSn00bSDK example programs.
# You may need to modify these values to match with your toolchain setup.

# GCC version
ifndef GCC_VERSION

GCC_VERSION	= 8.2.0

endif

# GCC base paths
ifndef GCC_BASE

ifeq "$(OS)" "Windows_NT"	# For Windows

GCC_BASE	= /c/mipsel-unknown-elf

else						# For Linux/BSDs

GCC_BASE	= /usr/local/mipsel-unknown-elf

endif

endif


# Toolchain prefix
PREFIX		= mipsel-unknown-elf-

CC		= $(GCC_BASE)/bin/$(PREFIX)gcc
CXX		= $(GCC_BASE)/bin/$(PREFIX)g++
AS		= $(GCC_BASE)/bin/$(PREFIX)as
AR		= $(GCC_BASE)/bin/$(PREFIX)ar
RANLIB	= $(GCC_BASE)/bin/$(PREFIX)ranlib

# Include directories
INCLUDE	 	= -I../include

# Finish paths
LIBDIRS 	+= -L$(GCC_BASE)/lib/gcc/mipsel-unknown-elf/$(GCC_VERSION)
INCLUDE	 	+= -I$(GCC_BASE)/lib/gcc/mipsel-unknown-elf/$(GCC_VERSION)/include
