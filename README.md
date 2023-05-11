
# PSn00bSDK

PSn00bSDK is an open source homebrew software development kit for the original
Sony PlayStation, consisting of a C/C++ compiler toolchain and a set of
libraries that provide a layer of abstraction over the raw hardware in order to
make game and app development easier. A CMake-based build system, CD-ROM image
packing tool (`mkpsxiso`) and asset conversion utilities are also provided.

At the heart of PSn00bSDK is `libpsn00b`, a set of libraries that implements
most of the core functionality of the official Sony SDK (excluding higher-level
libraries) plus several new extensions to it. Most of the basic APIs commonly
used by homebrew apps and games built with the official SDK are available,
making PSn00bSDK a good starting point for those who have an existing codebase
but want to move away from Sony tools.

Currently supported features include:

* Full support for the GPU's functionality including all primitive types (lines,
  polygons, sprites) as well DMA transfers managed through a software-driven
  command queue that can optionally be extended with custom commands. Both NTSC
  and PAL video modes are fully supported.

* Extensive GTE support with rotate, translate, perspective correction and
  lighting calculation fully supported through C and/or assembly GTE macros
  paired with high speed matrix and vector helper functions. All calculations
  performed in fixed point integer math, not a single float used.

* BIOS-based interrupt dispatch system providing the ability to register custom
  callbacks for all IRQs and DMA channels while preserving compatibility with
  all functions provided by the BIOS.

* Basic support for controller input through the BIOS, with optional limited
  support for manual polling.

* Complete Serial I/O support with buffering and console driver to redirect BIOS
  standard input and output to the serial port. Hardware flow control supported.

* CD-ROM support featuring asynchronous reading, CD-DA and XA-ADPCM audio
  playback and a built-in ISO9660 file system parser with no file count limit.
  Additional support for multi-session discs and bypassing region checks on
  supported console models.

* Full MDEC support for hardware accelerated lossy image decompression and video
  playback.

* Preliminary limited support for Konami System 573 arcade hardware.

* Experimental support for dynamic linking at runtime, including function and
  variable introspection by loading a map file generated at build time.

Note that, while PSn00bSDK's API is to some extent compatible with the official
SDK's, the project is *not* meant to be a drop-in replacement for it, both
since reimplementing the entire SDK would be a major undertaking and because
many parts of it are inefficient, clunky and/or provide relatively little value.

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

## Tutorials and examples

The `examples` directory contains several example programs showcasing different
parts of the SDK (mostly graphics); the source code of `n00bdemo` can also be
found in the same directory. More example programs may be added in the future
and contributed example programs are welcome.

[Lameguy's PlayStation Programming Tutorial Series](http://lameguy64.net/tutorials/pstutorials)
was written with older versions of PSn00bSDK in mind and is outdated at this
point, but may still be useful as an introduction to the console's hardware and
the basics of the graphics and controller APIs.

## To-do List

* `libpsxgte`: Rewrite all assembly functions from scratch as parts of them have
  been lifted as-is from Sony libraries. **PSn00bSDK is currently** (and
  will probably always be) **in a legal gray area due to this.**

* `libpsxspu`: Plenty of work to be done. Some kind of MIDI sequencer (similar
  to the one present in the official SDK) should be added at some point, along
  with a proper API for audio streaming.

* `libpsxcd`: Implement a command queue mechanism for the CD-ROM.

* `libpsxpress`: Add an API for SPU-ADPCM audio encoding at runtime.

* `libc`: Improve the memory allocation framework with multiple allocators,
  replace the string functions with optimized implementations and maybe add
  helpers to manage swapping between main RAM and VRAM/SPU RAM.

* Add a full controller and memory card API that does not depend on the BIOS
  controller driver, and possibly a library for interfacing to IDE/ATAPI drives
  to make development for arcade systems easier.

## Credits

Main developers/authors:

* **Lameguy64** (John "Lameguy" Wilbert Villamor)
* **spicyjpeg**

Contributors:

* **Silent**, **G4Vi**, **Chromaryu**: `mkpsxiso` and `dumpsxiso` (maintained
  as a [separate repo](https://github.com/Lameguy64/mkpsxiso)).

Honorable mentions:

* **Soapy**: wrote the original version of the `inline_c.h` header containing
  GTE macros.
* **ijacquez**: helpful suggestions for getting C++ working.
* **Nicolas Noble**: author of the
  [pcsx-redux](https://github.com/grumpycoders/pcsx-redux) emulator, OpenBIOS
  and other projects which proved invaluable during development.

Helpful contributors can be found in the changelog.

References used:

* [Martin Korth's psx-spx document](http://problemkaputt.de/psx-spx.htm) and the
  [community-maintained version](https://psx-spx.consoledev.net).
* MIPS and System V ABI specs (for the dynamic linker).
* Tails92's PSXSDK project (during PSn00bSDK's infancy).

Additional references can be found in individual source files.
