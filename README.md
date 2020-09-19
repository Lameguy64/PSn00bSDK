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
As of September 19, 2020

* Extensive GPU support with polygon primitives, high-speed DMA VRAM
  transfers and DMA ordering table processing. All video modes for both NTSC
  and PAL standards also supported with fully adjustable display area and
  automatic video standard detection based on last GPU mode. No BIOS ROM
  string checks used.

* Extensive GTE support with rotate, translate, perspective correction and
  lighting through assembly macros (for both C and ASM) and high performance
  matrix and vector functions. All calculations performed in fixed point
  integer math.

* Stable interrupt service routine with easy to use callback system for
  simplified handling and hooking of hardware and DMA interrupts, no crude
  event handler hooks or kernel hacks used and should be compatible with
  HLE BIOS implementations. Should also play well with writing loader
  programs.

* Complete Serial I/O support with SIOCONS driver for tty console access
  through serial interface. Handshake and flow control also supported.

* BIOS controller functions for polling controller input work as intended
  thanks to proper interrupt handling. No crude manual polling of controllers
  in your main loop.

* Full CD-ROM support with data reading, CD audio and XA audio playback.
  Features built-in ISO9660 file system parser for locating files and
  supports directories containing more than 30 files (classic ISO9660 only,
  no Rock Ridge or Joliet extensions support). Also supports multi-session
  discs.
  
* Uses Sony SDK library syntax for familiarity to experienced programmers
  and to make porting existing homebrew projects to PSn00bSDK easier.

* Works on real hardware and most popular emulators.


## Obtaining PSn00bSDK

Because PSn00bSDK is updated semi-regularly due to this project being in
a work-in-progress state, it is better to obtain this SDK from source and
building it yourself in the long run. Prepared packages containing
precompiled libpsn00b libraries, the toolchain and additional utilities
not included in this repository for Windows users are planned, though some
arrangements would need to be made first. Perhaps once PSn00bSDK is
considered halfway production ready.

A precompiled copy of the GCC 7.4.0 toolchain for Windows is available
in the PSn00bSDK page of Lameguy64's website
( http://lameguy64.net/?page=psn00bsdk ). This should make building PSn00bSDK
under Windows a bit easier as building the toolchain is the hardest part
of building PSn00bSDK, as its more difficult to get it to compile correctly
under Windows than on Linux and BSDs.


## Building the SDK

You may set one of the following variables either with set/export or on the
make command line to specify various parameters in building PSn00bSDK and
projects made with it.

* ``GCC_VERSION`` specifies the GCC version number. This is only really
  required for building the libc library. If not defined, the default value
  of `7.4.0` is used.
* ``PSN00BSDK_TC`` specifies the base directory of the GCC toolchain to
  build PSn00bSDK with, otherwise the makefile assumes you have the
  toolchain binaries in one of your PATH directories.
* ``PSN00BSDK_LIBS`` specifies the target directory you wish to install
  the compiled libpsn00b libraries to. If not specified, compiled
  libraries are consolidated to the libpsn00b directory.


### Windows:
1. Download the following:
  * MSys2 (32-bit or 64-bit version whichever you prefer)
  * GCC 7.4.0 for mipsel-unknown-elf (download from Lameguy64's website at
    http://lameguy64.net?page=psn00bsdk )
2. Install MSys2, update packages (with pacman -Syu) then install the
   following packages:
  * git
  * make
  * mingw-w64-i686-gcc (32-bit) or mingw-w64-x86_64-gcc (64-bit)
    You will need to close and reopen MSys2 for the PATH environment to
    update for MinGW.
  * mingw-w64-i686-tinyxml2 (32-bit) or mingw-w64-x86_64-tinyxml2 (64-bit)
3. Extract GCC 7.4.0 for mipsel-unknown-elf to the root of your C drive.
4. Edit `mipsel-unknown-elf/mipsel-unknown-elf/lib/ldscripts/elf32elmip.x`
   and update the .text definitions as explained in toolchain.txt.
5. Add `export PATH=$PATH:/c/mipsel-unknown-elf/bin` to your `.bash_profile`
   file in MSys2. Test if mipsel-unknown-elf-gcc can be called from any
   directory in the terminal.
6. Clone from PSn00bSDK source with
   `git clone https://github.com/lameguy64/psn00bsdk`
   Clone it in the root of your C drive or in any location you choose.
7. Enter tools directory in PSn00bSDK and run `make` to build all tools.
   Then, run `make install` to consolidate all tools to a single bin
   directory. Add this directory to your PATH variable by adding
   `export=$PATH:<path to SDK>/tools/bin` in your .bash_profile file. You'll
   need to reload the MSys2 shell after making the changes.
8. Enter libpsn00b directory and run `make` to build all libpsn00b libraries.
6. Compile the example programs to test if the SDK is set up correctly.
   Update directory paths in `sdk-common.mk` when necessary.
   
If you prefer to do things in the Command Prompt, you can add the paths
c:\msys64\usr\bin, c:\msys64\mingw64\bin (mingw32 for 32-bit),
c:\mipsel-unknown-elf\bin and c:\psn00bsdk\tools\bin (paths may vary
depending on where you've installed/extracted them) to your system's
PATH environment variable. This way, you can invoke make and compile
programs with PSn00bSDK within the Command Prompt.
   

### Linux and Unix-likes:
1. Install gcc, make, texinfo, git and development packages of mpfr, mpc,
   gmp, isl and tinyxml2 libraries for your distro.
2. Build and install the GNU GCC toolchain targeting mipsel-unknown-elf
   (see toolchain.txt for details). Export a variable named `PSN00BSDK_TC`
   containing a path to the installed toolchain's base directory. Also
   export a `GCC_VERSION` variable if you're using a version of GCC other
   than 7.4.0.
3. Clone from PSn00bSDK source with
   `git clone https://github.com/lameguy64/psn00bsdk`
4. Enter tools directory and run `make`, then `make install` to consolidate
   the tools to a bin directory. Add this directory to your PATH variable and
   make sure `elf2x` and `lzpack` (required for n00bDEMO) is accessible from
   any directory.
5. Enter the libpsn00b directory and run `make`.
6. Compile the example programs to test if the SDK is set up correctly.


## Examples

There are a few examples and complete source code of n00bdemo included in
the examples directory. More example programs may be added in the future
and contributed example programs are welcome.


## To-do List

* psxgpu: VRAM move function, few more primitives and macros yet to be
  implemented.

* psxgte: Higher level GTE rotate translate and perspective functions,
  and many matrix transformation functions yet to be implemented.

* psxspu: Plenty of work to be done. Hardware timer driven sound/music
  system may need to be implemented (an equivalent to the Ss* series of
  functions in libspu basically).
  
* Support for MDEC.


## Usage terms

PSn00bSDK falls under the terms and conditions of the Mozilla Public
License. A quick summary of this license is that PSn00bSDK can be used
freely in both free and open source projects and commercial closed source
projects as projects using PSn00bSDK does not necessarily have to follow
the MPL as well.

If modifications to the SDK were made as part of the development of such
projects that enhance its functionality, such changes must be contributed
back in return.

Homebrew made with PSn00bSDK may not be released under 'annoyingmous'. Although
there's nothing that would enforce it, this term may as well be ignored despite
it annoying this SDK's author.


## Credits

Main developer:
* Lameguy64

Honorable mentions:
* ijacquez (for the helpful suggestions for getting C++ working)

Helpful contributors can be found in the changelog.

References used:
* nocash's PlayStation specs document (http://problemkaputt.de/psx-spx.htm)
* Tails92's PSXSDK project.
