# Adjustable common makefile values for PSn00bSDK example programs.
# You may need to modify these values to correspond to your toolchain setup.

# Toolchain prefix. Can include an absolute path to the toolchain executables.
PREFIX		= mipsel-unknown-elf-

# Include directories.
INCLUDE	 	= -I../../libpsn00b/include

# Library directories. Last entry must point to a directory containing libgcc.
LIBDIRS		= -L../../libpsn00b
# Directory path for toolchain's libraries (you may need to change this)
ifeq "$(OS)" "Windows_NT"
# For Windows
LIBDIRS		+= -L/c/psn00bsdk/lib
else
# For Linux/BSDs
LIBDIRS 	+= -L/usr/local/mipsel-unknown-elf/lib/gcc/mipsel-unknown-elf/6.3.0
endif
