# PSn00bSDK

PSn00bSDK is a 100% free and open source SDK project for the original Sony
PlayStation for developing homebrew applications and games for the console
100% freely. This SDK can be used for freeware, commercial, and open source
homebrew projects.

The SDK is composed mainly of libraries (libpsn00b) and some utilities that
provide a basic framework for developing software for the PlayStation
hardware, the compiler is separate (GCC) and should be acquired from GNU.
The library API is intentionally written to resemble the library API of the
official libraries as closely as possible. This design decision is not only
for familiarity reasons and so that existing sample code and tutorials would
still apply, but to also make porting existing homebrew originally made with
official SDKs to PSn00bSDK easier with few modifications.

PSn00bSDK is currently a work in progress and cannot really be considered
production ready, but what is currently implemented should be enough to
produce some interesting homebrew with the SDK, especially with extensive
support for the GPU and GTE hardware. There's no reason not to fully support
the hardware features of the target platform when they have been fully
documented for years (nocash's PSX specs document in this case).

Most of libpsn00b is written mostly in MIPS assembly, more so functions that
interface with hardware. Many of the standard C functions are implemented in
custom MIPS assembly instead of equivalents found in the BIOS ROM for
performance reasons.


## Notable features
As of libpsn00b run-time library v0.10

* Extensive GPU support with polygon primitives, high-speed DMA VRAM
  transfers and DMA ordering table processing. All video modes for both NTSC
  and PAL standards also supported with fully adjustable display area and
  automatic video standard detection based on last GPU mode, no BIOS ROM
  string checks used.

* Extensive GTE support with rotate, translate, perspective correction and
  lighting through assembly macros (for both C and ASM) and high performance
  matrix and vector functions, all calculations performed in fixed point
  integer math.

* Stable and easy to use interrupt service routine with callback system for
  simplified handling of hardware and DMA interrupts, no crude event handlers
  or kernel hacks used and should be compatible with HLE BIOS implementations
  and homebrew loaders and menus.

* Complete Serial I/O support with SIOCONS driver for tty console access
  through serial interface.

* BIOS controller functions for polling controller input work as intended
  thanks to proper interrupt handling, no crude manual polling of controllers
  in your main loop.

* BIOS CD-ROM support with a custom initialization function that doesn't
  clear other DMA channel settings (such as GPU and SPU DMA) for easier
  initialization.

* Uses Sony SDK library syntax for familiarity to experienced programmers
  and to make porting existing homebrew to PSn00bSDK easier.

* Works on real hardware and most popular emulators.


## Obtaining PSn00bSDK

Because PSn00bSDK is updated semi-regularly due to this project being in
a work-in-progress state, it is better to obtain this SDK from source by
building it yourself in the long run. Prepared packages containing
precompiled libpsn00b libraries, the toolchain and additional utilities
not included in this repository for Windows users are planned, though some
arrangements would need to be made first. Perhaps once PSn00bSDK is
considered halfway production ready.

A precompiled copy of the GCC 7.2.0 toolchain for Windows is available
in lameguy64's website (http://lameguy64.tk) in the PSn00bSDK page. This
should make building PSn00bSDK under Windows a bit easier as building the
toolchain is the hardest part of building PSn00bSDK as its more difficult
to get it to compile correctly under Windows than on Linux and BSDs.


## Building the SDK

### Windows (needs work/testing):
1. Download the following:
  * MinGW GCC (32-bit or 64-bit whichever you prefer)
  * MSys2 (32-bit or 64-bit whichever you prefer)
  * tinyxml2 (for lzpack and smxlink)
  * GCC 7.2.0 for mipsel-unknown-elf (download from Lameguy64's website)
2. Install MSys2 and MinGW GCC.
3. Extract GCC 7.2.0 for mipsel-unknown-elf to the root of your C drive.
4. Update your PATH environment variable to point to the bin directories of
   GCC 7.2.0 for mipsel-unknown-elf, MSys2 and MinGW GCC. Make sure you can
   access gcc, mipsel-unknown-elf-gcc and make from any directory in the
   command prompt.
5. Build tinyxml2 with MinGW GCC through MSys2's shell
   (./configure then make).
6. Clone/download PSn00bSDK source files.
7. Enter libpsn00b directory and run make.
8. Enter tools directory and run make, then 'make install' to consolidate the
   executables to a bin directory. Add this directory to your PATH
   environment variable and make sure elf2x is accessible from any directory.
9. Compile the example programs to test if the SDK is set up correctly.
   Update directory paths in sdk-common.mk when necessary.

### Linux and Unix-likes:
1. Build and install the GNU GCC toolchain configured for mipsel-unknown-elf
   (see toolchain.txt for details).
2. Update your PATH environment variable to point to the bin directory of the
   toolchain. Make sure the toolchain executables are accessible from any
   directory in the terminal.
2. Install the following (development) package:
  * tinyxml2
3. Clone/download PSn00bSDK source files.
4. Enter the libpsn00b directory and run make.
5. Enter the tools directory and run make, then 'make install' to consolidate
   the executables to a bin directory. Add this directory to your PATH
   variable and make sure elf2x is accessible from any directory.
6. Compile the example programs to test if the SDK is set up correctly.
   Update directory paths in sdk-common.mk when necessary.


## Examples

There are a few examples and complete source code of n00bdemo included in
the examples directory. More example programs may be added in the future
and contributed example programs are welcome.


## To-do List

* libc: C++ support (getting classes, new and delete working is enough)
  and better sprintf() (one from PSXSDK is slow due to unnecessary usage
  of int64 and somewhat a bit buggy) yet to be implemented. More standard
  C library stuff also yet to be implemented.

* psxgpu: VRAM download and VRAM move functions, some more primitives
  and macros yet to be implemented.

* psxgte: Higher level GTE rotate translate and perspective functions
  yet to be implemented.

* psxspu: Lots of work to do.

* psxapi: Plenty of BIOS function calls yet to be added.

* psxetc: Text stream stuff (FntOpen(), FntPrint(), FndFlush()) for
  debug purposes yet to be implemented.
  
* Libraries yet to be made: psxcd (for better CD-ROM support) and
  psxmdec (MDEC support).


## Usage terms

PSn00bSDK falls under the terms and conditions of the Mozilla Public
License. A quick summary of this license is that PSn00bSDK can be used
freely in both free and open source projects and commercial closed source
projects as projects using PSn00bSDK does not necessarily have to follow
the MPL as well.

If modifications to the SDK were made as part of the development of such
projects that enhance its functionality, such changes must be contributed
back in return.


## Credits

Main developer:
* Lameguy64

References used:
* nocash's PlayStation specs document (http://problemkaputt.de/psx-spx.htm)
* Tails92's PSXSDK project.
