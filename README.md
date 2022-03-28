
# PSn00bSDK

PSn00bSDK is a 100% free and open source SDK project for the original Sony
PlayStation for developing homebrew applications and games for the console.
This SDK may be used for freeware, commercial, and open source homebrew
projects as far as what the SDK currently supports. Out of all the open
source PS1 SDK projects that have come and gone from active development
over the years, PSn00bSDK is arguably the most capable of them all.

Much of the SDK is merely just a set of libraries (`libpsn00b`) and some
utilities for converting executables and data files to formats more usable
on the target platform. The compiler used is just the standard GNU GCC
toolchain compiled to target mipsel and has to be acquired separately.
The library API was deliberately written to resemble the library API of the
official libraries as closely as possible not only for familiarity reasons
to experienced programmers but also so that existing sample code and tutorials
that have been written over the years would still apply to this SDK, as well
as making the process of porting over existing homebrew originally made with
official SDKs easier with minimal modificationn provided they do not depend
on libgs.

PSn00bSDK is currently a work in progress and cannot really be considered
production ready, but what is currently implemented should be enough to
produce some interesting homebrew with the SDK especially with its extensive
support for the GPU and GTE hardware. There's no reason not to fully support
hardware features of a target platform when said hardware features have been
fully documented for years (nocash's PSX specs document in this case).

Most of `libpsn00b` is written mostly in MIPS assembly more so functions that
interface with the hardware. Many of the standard C functions are implemented
in custom MIPS assembly instead of equivalents found in the BIOS ROM, for both
stability (the BIOS `libc` implementation of the PlayStation is actually buggy)
and performance reasons.


## Notable features

As of March 28, 2022

* Extensive GPU support with lines, flat shaded or textured polygon and sprite
  primitives, high-speed DMA for VRAM transfers and ordering tables. All video
  modes for both NTSC and PAL standards also supported with fully adjustable
  display area and automatic video standard detection based on last GPU mode.

* Extensive GTE support with rotate, translate, perspective correction and
  lighting calculation fully supported through C and/or assembly GTE macros
  paired with high speed matrix and vector helper functions. All calculations
  performed in fixed point integer math, not a single float used.

* Flexible interrupt service subsystem with easy to use callback mechanism for
  simplified handling and hooking of hardware and DMA interrupts. No crude
  event handler hooks or kernel hacks providing great compatibility with
  HLE BIOS implementations and loader/menu type homebrew programs.

* BIOS controller functions for polling controller input work as intended
  thanks to proper handling of hardware interrupts. No crude direct I/O polling
  of controllers in the main loop.
  
* Complete Serial I/O support with SIOCONS driver for tty stdin/stdout
  console access. Hardware flow control supported.

* Full CD-ROM support using libpsxcd featuring data read, CD audio and XA audio
  playback, built-in ISO9660 file system parser with no file count limit
  (classic ISO9660 only, no Rock Ridge or Joliet extensions) and multi-session.
  
* Preliminary MDEC support implemented with libpsxpress (no VLC decoding yet).
  Sample encoder included.

* Can target Konami System 573 arcade hardware with limited support
  (see examples/io/system573/main.c for details)

* Experimental support for compiling separate sections of an executable into
  shared library files (DLLs) and linking them dynamically at runtime, plus
  support for function and variable introspection by loading a map file
  generated at build time.

* Uses Sony SDK library syntax for familiarity to experienced programmers
  and makes porting existing homebrew projects to PSn00bSDK easier.

* Works on real hardware and most popular emulators.

* Fully expandable and customizable to your heart's content.


## Obtaining PSn00bSDK

Prebuilt PSn00bSDK packages for Windows and Linux are available through GitHub
Actions which includes the libraries, a copy of the GCC MIPS toolchain, command
line tools, examples and documentation. CMake is **not** included and must be
installed separately, either from [its website](https://cmake.org/download) or
via MSys2 or your distro's package manager.

See [installation.md](doc/installation.md) for a quick start guide and for
details on how to build the SDK yourself.


## Examples

There are a few examples and complete source code of `n00bdemo` included in the
`examples` directory. More example programs may be added in the future and
contributed example programs are welcome.

There's also [Lameguy's PlayStation Programming Tutorial Series](http://lameguy64.net/tutorials/pstutorials)
for learning how to program for the PlayStation. Much of the tutorials should
apply to PSn00bSDK.


## To-do List

* psxspu: Plenty of work to be done. Hardware timer driven sound/music
  system may need to be implemented (an equivalent to the Ss* series of
  functions in libspu basically). Functions that make use of the SPU RAM
  interrupt feature to play or capture streamed audio should also be added.

* psxcd: Implement a command queue mechanism for the CD-ROM?

* libc: Improve the memory allocation framework with multiple allocators, GC
  and maybe helpers to manage swapping between main RAM and VRAM/SPU RAM.

* Further support for MDEC, and tooling to transcode videos to .STR files
  (either reimplementing the container and compression format used by the Sony
  SDK, or a custom format with better compression).

* Pad and memory card libraries that don't use the BIOS routines.


## Credits

Main developer/author/whatever:

* **Lameguy64** (John "Lameguy" Wilbert Villamor)

Contributors:

* **spicyjpeg**: dynamic linker, CMake scripts, some docs and examples
  (`system/dynlink`, `sound/spustream`, `io/pads`, `io/system573`).
* **Silent**, **G4Vi**, **Chromaryu**: `mkpsxiso` and `dumpsxiso` (maintained
  as a [separate repo](https://github.com/Lameguy64/mkpsxiso)).

Honorable mentions:

* **ijacquez**: helpful suggestions for getting C++ working.
* **Nicolas Noble**: his OpenBIOS project gave insight to how the BIOS works
  internally.

Helpful contributors can be found in the changelog.

References used:

* [nocash's PlayStation specs document](http://problemkaputt.de/psx-spx.htm)
  and Nicolas Noble's [updated version](https://psx-spx.consoledev.net).
* MIPS and System V ABI specs (for the dynamic linker).
* Tails92's PSXSDK project (during PSn00bSDK's infancy).

Additional references can be found in individual source files.
