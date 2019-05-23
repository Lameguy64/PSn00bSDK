# PSn00bSDK

PSn00bSDK is a 100% free and open source SDK for developing homebrew games
and applications for the original Sony PlayStation. The SDK consists mainly
of libraries and some tools for converting and building resources to be
used on the console.

While PSn00bSDK is currently a work in progress the project aims to develop
an SDK that is as close to the official Sony SDK as possiblein in terms of
supported hardware features which include GPU, GTE, SPU, CD, MDEC and
controller/memory card peripherals and a library API written to follow the
official SDK's API syntax. With extensive low-level technical documentation
of the PSX readily available (such as nocash's PSX specs) there should be no
excuse to not have full support of the aforementioned hardware features.

The PSn00bSDK libraries are written mostly in MIPS assembly language with
compiler generated code limited to small and moderately sized support
functions for best possible performance and to keep the runtime library
footprint as small as possible. Many of the library functions avoid using
BIOS calls such as C string and memory manipulation functions and use pure
assembly equivalents for improved performance for memory and string
manipulation operations.


## Building the SDK

For most users (particularly those who run Windows) it is recommended to
just download a release package containing the GCC toolchain and libraries
in binary form ready to be used.

If you wish to build the SDK yourself building PSn00bSDK requires a GNU
GCC toolchain targeting mipsel-unknown-elf. For instructions on how to
build the GCC toolchain please read toolchain.txt.

To build the PSn00bSDK libraries simply enter the libpsn00b directory and
run make. Make sure you have the path of the toolchain binaries in your PATH
environment variable. If things go accordingly it should run through all
library directories and produce library files.

To build the PSn00bSDK tools simply enter the tools directory and run make.
You'll need tinyxml2 to satisfy the lzpack and smxlink tools.

To build the PSn00bSDK examples which also tests if your SDK setup works
correctly simply enter the examples directory and run make. You may want
to modify the sdk-common.mk file first and make sure the library paths are
correct and that your PATH environment variable has the tools/bin directory
in it as elf2x and lzpack are required for the examples to build correctly.


## Examples

There are a few graphics examples and complete source code of n00bdemo
included in the examples directory. More example programs may be added in
future updates and contributions are welcome.


## To-do

* Support functions to get C++ classes working are yet to be implemented.
  glibc won't compile (or its not included with GCC sources) and likely
  depends on a Linux kernel that does not exist on the PS1. Newlib might
  be undesirable as it appears to be too bloated for PS1. Just getting
  classes to work would be enough.

* psxspu needs to be expanded upon. Currently lacks support for reverb
  and many voice controls.

* An 'IRQ handler for all' implementation through BIOS function
  SetCustomExitFromException() is yet to be implemented for better/more
  reliable interrupt handling. UPDATE: SetCustomExitFromException() hook
  is now working! See readme of psxgpu for details.

* CD-ROM library is yet to be made.

* Better (and faster) sprintf() function.


## Usage terms

PSn00bSDK falls under the terms and conditions of the Mozilla Public
License. A quick summary of this license is that PSn00bSDK can be used
freely in both free and open source projects and commercial closed source
projects. But if modifications to the SDK were made as part of the
development of such projects such changes must be contributed back in
return.


## Credits

Main developer:
* Lameguy64

Important references used:
* nocash's PlayStation specs document (http://problemkaputt.de/psx-spx.htm)
* Tails92's PSXSDK project (bits and pieces of it).
