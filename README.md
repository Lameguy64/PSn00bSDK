
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

As of October 11, 2022:

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
  thanks to proper handling of hardware interrupts. Optional limited support
  for manual polling.

* Complete Serial I/O support and console driver to redirect standard input and
  output to the serial port. Hardware flow control supported.

* Full CD-ROM support using `libpsxcd` featuring data reading, CD-DA and XA
  audio playback, a built-in ISO9660 file system parser with no file count
  limit and support for multi-session discs.

* MDEC support, lossy image decompression and video playback using
  `libpsxpress` (currently only bitstream versions 1 and 2 are supported).

* Preliminary limited support for Konami System 573 arcade hardware.

* Experimental support for dynamic linking at runtime, with support for
  function and variable introspection by loading a map file generated at build
  time.

* Uses Sony SDK library syntax for familiarity to experienced programmers
  and makes porting existing homebrew projects to PSn00bSDK easier.

* Works on real hardware and most popular emulators.

* Fully expandable and customizable to your heart's content.


## Obtaining PSn00bSDK

Prebuilt PSn00bSDK packages for Windows and Linux are available on the releases
page and include the libraries, a copy of the GCC MIPS toolchain, command-line
tools, examples and documentation. CMake is **not** included and must be
installed separately, either from [its website](https://cmake.org/download) or
via MSys2 or your distro's package manager.

The releases can be installed by simply extracting the archives into any
directory and adding the `bin` subfolder to the `PATH` environment variable.
`share/psn00bsdk/template` contains a barebones example project that can be
used as a starting point.

For more information on how to get started, or if you wish to build the SDK
yourself from source instead, refer to [installation.md](doc/installation.md).


## Examples

There are a few examples and complete source code of `n00bdemo` included in the
`examples` directory. More example programs may be added in the future and
contributed example programs are welcome.

There's also [Lameguy's PlayStation Programming Tutorial Series](http://lameguy64.net/tutorials/pstutorials)
for learning how to program for the PlayStation. Much of the tutorials should
apply to PSn00bSDK.


## To-do List

* `libpsxspu`: Plenty of work to be done. Some kind of MIDI sequencer (similar
  to the one present in the official SDK) should be added at some point, along
  with a proper API for audio streaming.

* `libpsxcd`: Implement a command queue mechanism for the CD-ROM.

* `libpsxpress`: Add support for version 3 and IKI frame bitstreams.

* `libc`: Improve the memory allocation framework with multiple allocators,
  replace the string functions with optimized implementations and maybe add
  helpers to manage swapping between main RAM and VRAM/SPU RAM.

* Add a full controller and memory card API that does not depend on the BIOS
  controller driver, and possibly a library for interfacing to IDE/ATAPI drives
  to make development for arcade systems easier.


## Credits

Main developer/author/whatever:

* **Lameguy64** (John "Lameguy" Wilbert Villamor)

Contributors:

* **spicyjpeg**: dynamic linker, `libpsxpress`, CMake scripts, some docs and
  examples.
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
