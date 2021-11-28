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
for familiarity reasons to experienced programmers, but also so that existing
sample code and tutorials would still apply to this SDK, as well as making
the process of porting over existing homebrew originally made with official
SDKs easier with minimal modification, provided it doesn't use libgs.

PSn00bSDK is currently a work in progress and cannot really be considered
production ready, but what is currently implemented should be enough to
produce some interesting homebrew with the SDK, especially with its extensive
support for the GPU and GTE hardware. There's no reason not to fully support
hardware features of a target platform when said hardware features have been
fully documented for years (nocash's PSX specs document in this case).

Most of libpsn00b is written mostly in MIPS assembly, moreso functions that
interface with the hardware. Many of the standard C functions are implemented
in custom MIPS assembly instead of equivalents found in the BIOS ROM, for both
stability (the BIOS libc implementation of the PlayStation is actually buggy)
and performance reasons.


## Notable features

As of August 16, 2021

* Extensive GPU support with polygon, line and sprite primitives, high-speed
  DMA transfers for VRAM data and ordering tables. All video modes for both
  NTSC and PAL standards also supported with fully adjustable display area
  and automatic video standard detection based on last GPU mode. No BIOS
  ROM checks used.

* Extensive GTE support with rotate, translate, perspective correction and
  lighting fully supported via C and assembly GTE macros paired with high
  speed matrix and vector functions. All calculations performed in fixed
  point integer math, not a single float defined.

* Flexible interrupt service routine with easy to use callback mechanism for
  simplified handling and hooking of hardware and DMA interrupts, no crude
  event handler hooks or kernel hacks providing great compatibility with
  HLE BIOS implementations. Should work without issue for loader/menu type
  programs as well.

* Complete Serial I/O support with SIOCONS driver for tty stdin/stdout
  console access. Hardware flow control also supported.

* BIOS controller functions for polling controller input work as intended
  thanks to proper handling of hardware interrupts. No crude manual polling
  of controllers in a main loop. BIOS memory card functions may also work,
  but not yet tested extensively.

* Full CD-ROM support via libpsxcd with data read, CD audio and XA audio
  playback support. Features built-in ISO9660 file system parser for locating
  and querying files and directories. Supports directories containing more
  than 30 files (classic ISO9660 only, no Rock Ridge or Joliet extensions)
  and also supports reading new sessions on a multi-session disc.

* Experimental support for compiling separate sections of an executable into
  shared library files (DLLs) and linking them dynamically at runtime, plus
  support for function and variable introspection by loading a map file
  generated at build time.

* Uses Sony SDK library syntax for familiarity to experienced programmers
  and to make porting existing homebrew projects to PSn00bSDK easier.

* Works on real hardware and most popular emulators.

* Fully expandable and customizable to your heart's content.


## Obtaining PSn00bSDK

PSn00bSDK has switched to a CMake-based build and installation system. See
[installation.md](doc/installation.md) for details.

Because PSn00bSDK is updated semi-regularly due to this project being in
a work-in-progress state, it is better to obtain this SDK from source and
building it yourself in the long run. Pre-compiled packages for Debian and
Msys2 are being planned however (it is already possible to build installers,
DEB and RPM packages through CPack so it's only a matter of time).

## Examples

There are a few examples and complete source code of n00bdemo included in
the examples directory. More example programs may be added in the future
and contributed example programs are welcome.

There's also Lameguy's PlayStation Programming Tutorial Series at
http://lameguy64.net/tutorials/pstutorials/ for learning how to program
for the PlayStation. The tutorials should still apply to PSn00bSDK.


## To-do List

* psxspu: Plenty of work to be done. Hardware timer driven sound/music
  system may need to be implemented (an equivalent to the Ss* series of
  functions in libspu basically). Need to figure out the correct frequency
  table for playing sounds in musical note notation. Functions that make use of
  the SPU RAM interrupt feature to play or capture streamed audio should also
  be added.

* psxcd: Implement a command queue mechanism for the CD-ROM?

* libc: Improve the memory allocation framework with multiple allocators, GC
  and maybe helpers to manage swapping between main RAM and VRAM/SPU RAM.

* Support for MDEC, and tooling to transcode videos to .STR files (either
  reimplementing the container and compression format used by the Sony SDK, or
  a custom format with better compression).

* Pad and memory card libraries that don't use the BIOS routines.

## Usage terms (or lack thereof)

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
* ijacquez - helpful suggestions for getting C++ working.
* NicolasNoble - his OpenBIOS project gave insight to how the BIOS works
  internally.

Helpful contributors can be found in the changelog.

References used:
* nocash's PlayStation specs document (http://problemkaputt.de/psx-spx.htm)
* Tails92's PSXSDK project (during PSn00bSDK's infancy).
