
# PSn00bSDK changelog

**NOTE**: this file is parsed by a script to generate release notes. When
contributing to PSn00bSDK, add a new block at the top following this template:

```
## <year>-<month>-<day>: [optional new version]

<contributor>:

- ...

- ...
```

You may run `.github/scripts/generate_release_notes.py CHANGELOG.md` afterwards
to ensure the changelog can be parsed correctly.

-------------------------------------------------------------------------------

## 2022-12-18: 0.22

spicyjpeg:

- libc: Fixed `assert()` macro and removed redundant type definitions. Added
  GTE-accelerated leading zero count intrinsics (`__builtin_clz()`).

- psxcd: Rewritten some parts of the library in pure C. Added `CdCommand()` and
  `CdCommandF()` for advanced usage of the CD-ROM, as well as `CdReadRetry()`,
  `CdReadBreak()` and `CdGetRegion()`.

- psxgpu: `PutDrawEnv()` and `DrawOTagEnv()` now properly apply the texture
  page and window attributes in the `DRAWENV` structure. RECTs passed to
  `LoadImage()` and `StoreImage()` are now copied into a private buffer. Added
  `setColor*()` macros.

- psxspu: Fixed setting of bus wait states when using `SpuRead()`.

- psxetc: Fixed several critical bugs in the dynamic linker. GOT relocation and
  linking of variables and objects are now handled properly.

- psxapi: Added `FastEnterCriticalSection()` and `FastExitCriticalSection()`
  inline macros for quick enabling and disabling of interrupts.

- Renamed some registers in `hwregs_c.h`, `hwregs_a.inc` and updated examples
  accordingly.

- All projects are now compiled with `-Og` in debug mode and `-O2` in release
  mode by default. CD images created using `psn00bsdk_add_cd_image()` are now
  only rebuilt if necessary (i.e. if any of their dependencies change).

## 2022-10-27

spicyjpeg:

- psxspu: Fixed bugs in `SpuInit()` and in `SpuWrite()` when using manual
  transfer mode (`SPU_TRANSFER_BY_IO`). Added `SpuWritePartly()`.

- psxetc: Added `IRQ_Channel` and `DMA_Channel` enums.

- examples: Refactored and bugfixed all sound examples. Renamed the old
  `spustream` example to `cdstream` and added a new `spustream` example
  demonstrating SPU audio streaming from main RAM. Both streaming examples now
  make use of the interleaved .VAG file format.

## 2022-10-21: 0.21

spicyjpeg:

- libpsn00b: Cleaned up the internal logging system.

- psxgpu: Added `MoveImage()` as well as `MoveImage2()`, `LoadImage2()` and
  `StoreImage2()`. `LoadImage()` and `StoreImage()` now make use of the
  library's internal drawing queue instead of blocking. Added `EnqueueDrawOp()`
  for more advanced control of the drawing queue. The `getTPage()` macro now
  supports extended Y coordinates (512-1023) on systems with 2 MB of VRAM.

- psxsio: Removed `_sio_control()` and replaced it with a completely new
  asynchronous buffered serial port driver. Rewritten the serial TTY driver to
  make use of the new API.

## 2022-10-16

spicyjpeg:

- psxetc: Fixed (another) critical bug in the IRQ callback dispatcher. This
  also fixed some examples that were broken ever since the library was
  rewritten in C. Made the dynamic linker less verbose, improving DLL loading
  speed in debug mode.

- psxcd: Added `CdDataSync()`. Renamed `CdlModeSize0` and `CdlModeSize1` to
  `CdlModeIgnore` and `CdlModeSize` respectively.

- psxpress: Fixed a bug in the Huffman decompression API that would make it
  crash if the bitstream header didn't contain a valid decompressed length,
  even if the bitstream was properly encoded. This fix makes the API fully
  usable for video playback (as demonstrated by the new example).

- examples: Added `mdec/strvideo` FMV playback example. Fixed
  `system/childexec` not properly uninstalling BIOS callbacks before launching
  the child executable. Added pause/resume code to `sound/spustream`.

## 2022-10-11

spicyjpeg:

- libpsn00b: Completely rewritten CMake scripts. Six copies of each library are
  now built and installed, one for each combination of configuration (debug and
  release) and target type (executable with/without $gp-relative addressing and
  DLL). Library debug logging is now completely disabled when a project is
  built in release mode (using `-DCMAKE_BUILD_TYPE=Release`).

- libc: Replaced `memset()` with a much faster optimized implementation that
  makes use of Duff's device. Added `GetHeapUsage()` and `TrackHeapUsage()`.
  Removed `_mem_init()`.

- psxetc: Fixed a critical bug in `DMACallback()` that would lead to the DMA
  interrupt handler being disabled entirely in some edge cases.

- psxgpu: Replaced the debug font with an improved one. No changes have been
  made to the API.

- psxcd: Added `CdGetSector2()` (asynchronous variant of `CdGetSector()`).
  `CdControl()` can now take a `CdlLOC` as second argument when issuing a seek
  command (previously the argument was ignored if the command was not `ReadN`
  or `ReadS`).

- Replaced the `DEBUG` macro with the standard C `NDEBUG` macro, which is only
  defined if the project is being built in release mode.

- Added `PSN00BSDK_*_LINK_LIBRARIES` CMake variables to control which SDK
  libraries are linked to newly created executables and DLLs. Updated
  `cmake_reference.md` to reflect the changes.

## 2022-09-22

spicyjpeg:

- libpsn00b: Changed the extension of `gtereg.h`, `hwregs_a.h` and `inline_s.h`
  to `.inc`. Cleaned up old leftover PSXSDK headers.

- Updated documentation and added detailed instructions on how to install
  prebuilt releases.

## 2022-08-21: 0.20

spicyjpeg:

- psxgpu: Added `VSyncHaltFunction()`.

- psxetc: Rewritten the library in C, making `RestartCallback()` behave more
  like its official SDK counterpart. Added `StopCallback()` and
  `ResetCallback()`.

- psxspu: `SpuInit()` now properly resets the starting address of all channels,
  preventing them from accidentally triggering the SPU IRQ.

- examples: Added an example audio file to `sound/spustream` and a texture to
  `graphics/gte`. Replaced `ball16c.h` in all examples that included it with a
  binary .TIM file embedded through CMake.

## 2022-07-31

spicyjpeg:

- libc: Replaced default memory allocator with Nicolas Noble's PSYQo
  implementation. Added `sbrk()` and `realloc()`.

- psxspu: Rewritten the library in C, removing a few redundant non-standard
  functions and improving SPU DMA reliability. Renamed `SpuWait()` to
  `SpuIsTransferCompleted()` (and tweaked its prototype) for compatibility with
  the official SDK. `SpuInit()` now loads a dummy block at the beginning of SPU
  RAM by default.

- psxmdec: Added experimental Huffman decoding API. Two implementations with
  different performance and memory usage tradeoffs are available.

- psxapi: Added `SwEnterCriticalSection()`, `SwExitCriticalSection()`,
  `SetConf()` and various thread- and event-related BIOS API wrappers that were
  previously missing.

- Deprecated `u_short`, `u_int` and `u_long` types and replaced them with
  standard library fixed-size types (`uint16_t` and `uint32_t` respectively) in
  all library code.

- Ninja is now bundled with binary releases of the SDK. A `BUNDLE_NINJA` option
  was added to the main CMake script to toggle bundling when building packages.

## 2022-06-26

spicyjpeg:

- psxgpu: Rewritten the entire library in C, fixing a few bugs (including one
  that made 384x240 and 384x256 modes unusable) and adding a drawing queue in
  the process. `DrawOTag()` can now be called while another OT is being drawn
  to enqueue up to 8 other OTs.

- psxgpu: Added missing `setXYWH()` macro and implemented `ClearOTag()`,
  `DrawOTag2()`, `DrawOTagEnv()` and `GsGetTimInfo()`.

- libpsn00b: Renamed `_start()` to `_start_inner()` and added a `_start()` stub
  that can be overridden to insert custom startup code. Modified the linker
  scripts to assume RAM is 8 MB by default, to make it easier to target arcade
  or development hardware.

- examples: Fixed some functions in `io/system573`.

- tools: `elf2x` now warns if the converted executable crosses the 2 MB RAM
  boundary.

- Debugging symbols in ELF files are now generated by default for all target
  types. This does not affect the size of built executables or DLLs, as those
  are always stripped.

## 2022-03-30

lameguy64:

- indev: psxmdec prototype is now functional through `psxpress`.

## 2022-03-25: 0.19

lameguy64:

- examples: Replaced sample image of `mdec/mdecimage` with different artwork.
  Original image [here](http://lameguy64.net/?page=drawings&post_id=9).

- examples: Updated readme file in examples directory to reference the new
  `mdec/mdecimage` example program.

- psxsio: Added dummy hooks to unsupported device functions for tty device.

- docs: Minor corrections on `dev_notes.md`.

- docs: Updated documentation for `CdGetSector()` and `CdReadCallback()`
  functions in `libn00bref.odt`.

## 2022-02-27

spicyjpeg:

- libpsn00b: Added `hwregs_c.h` header and renamed some registers in
  `hwregs_a.h`. Added `assert()` as a proper macro.

- libpsn00b: Added new MDEC library; `psxpress`.

- psxspu: Fixed critical bug in `SpuSetReverb()`.

- psxetc: Fixed minor typos and bugs in `dlfcn.h`. Replaced `DL_CALL()` macro
  with `DL_PRE_CALL()`.

- psxapi: Rewritten all BIOS API stubs using a Python script and a JSON list,
  both of which are included in the library's source directory.

- examples: Updated `io/system573` with bugfixed I/O board code and separated
  the APIs into a separate source file. Fixed timing bugs in `io/pads`.

- mkpsxiso: Switched back to the master repository. The `dumpsxiso` command is
  now included in PSn00bSDK installations.

- Added `psn00bsdk_target_incbin()` and `psn00bsdk_target_incbin_a()` CMake
  functions for quickly embedding binary files into executables and libraries.
  The examples have been modified to use this function where applicable.

- Added `LIBPSN00B_GENERATOR` CMake option to allow the host-side tools to be
  compiled with MSVC or Xcode while still using Ninja or `make` to build the
  libraries and examples. This is required as neither MSBuild nor Xcode support
  custom toolchains.

- Updated `LICENSE.md` with additional license information as well as a full
  copy of the GPLv2 and GPLv3 (under which `mkpsxiso` and GCC are licensed
  respectively).

## 2022-01-17

Lameguy64:

- docs: Removed old and incomplete `libn00bref.odt` document (a precursor of
  the LibPSn00b Library Reference document).

- examples: Improved description of `hdtv` example. Examples directory is now
  copied into `share/psn00bsdk` directory for both installation and package
  building. Build instructions for examples also included.

- docs: Removed documentation for `SetDrawTPageVal()` as the function was
  removed ages ago. Added documentation to `DR_AREA`, `DR_TWIN` and `DR_OFFSET`
  primitives and their associated macros.

- examples: Added `tilesasm` example.

- Updated readme file.

## 2021-12-23

spicyjpeg:

- psxcd: `CdGetSector()` now expects the sector size to be in 32-bit word units
  (rather than bytes) for consistency with the official CD-ROM library. The
  library's ISO9660 parser and helper functions have been updated accordingly.
  **This is a breaking change**.

- examples: Added `spustream` audio streaming example.

## 2021-11-28: 0.18

spicyjpeg:

- libc: Removed `STACK_MAX_SIZE` and added `_mem_init()` back. RAM and stack
  size can now be set by calling `_mem_init()` manually before allocating any
  memory (however this seems to be currently broken).

- libc: `sprintf()` now supports fixed padding when using the `%@` (binary
  integer) format specifier.

- psxcd: File paths with forward slashes instead of backslashes, as well as
  paths containing both slash types, are now accepted.

- psxapi: Added wrapper around BIOS function `GetSystemInfo()`.

- examples: Added `pads` and `system573` examples. Also added CMake build
  script to `cartrom`, which is now built alongside all other examples.

- Header files are now installed to `include/libpsn00b` instead of
  `lib/libpsn00b/include`.

- Deprecated `malloc.h` and removed all references to it (`stdlib.h` should be
  used instead). Moved `int*_t` and `uint*_t` types to `stdint.h`.

- Fixed file permission errors when attempting to install the SDK on macOS.

- Cleaned up, updated and moved all documentation to the `doc` folder.
  Rewritten this changelog and added a script to generate release notes.

## 2021-10-31

spicyjpeg:

- Added `tinyxml2` and `mkpsxiso` as git submodules and removed the (admittedly
  short-lived) `SKIP_*` options completely. CMake's `ExternalProject` is no
  longer used to download dependencies at build time.

- Added CMake presets file with presets for building installer packages.

## 2021-10-25

Lameguy64:

- Made a very tiny change in the readme file.

- Included some of the indev directory from the SVN repository.

## 2021-10-17: 0.17

spicyjpeg:

- Added `SKIP_TINYXML2` and `SKIP_MKPSXISO` build options in place of the old
  `SKIP_DOWNLOAD` option, and a new `BUNDLE_TOOLCHAIN` option for building
  packages. Updated `INSTALL.md` accordingly and fixed git branch error when
  downloading mkpsxiso during build.

- Rewritten `toolchain.txt` (which is now `TOOLCHAIN.md`) and added proper
  Windows toolchain build instructions.

- Added safety checks in CMake scripts to ensure `elf2x`/`mkpsxiso` are
  installed before attempting to build an executable or CD image.

- examples: Added CMake build script to (and removed makefile from) `vagsample`.

## 2021-06-10

Lameguy64:

- psxspu: Fixed register typo in `SpyKeyOn` causing unpredictable instability.

- psxspu: Fixed bug where `SpuKeyOn` does not set the key-on bits for voices 16
  to 23.

- examples: Added `vagsample` SPU example.

## 2021-09-27

spicyjpeg:

- liblzp, tools: Fixed tools not building on MSVC and cleaned up LZP API
  declarations (replaced meaningless `void*` pointers with proper types).

- libpsn00b: Added missing `PSN00BSDK_VERSION` CMake variable.

- examples: Fixed childexec (`parent.exe`) example crashing due to the child
  executable not being relocated in the new build script. Patched n00bdemo to
  suppress `liblzp` pointer type warnings.

- Fixed another MSVC linker error when building `tinyxml2` automatically, as
  well as various toolchain/compiler test errors sometimes thrown by CMake.
  Updated `INSTALL.md` with more details.

## 2021-09-13

spicyjpeg:

- Migrated libpsn00b, tools, examples and template to CMake, added a top-level
  `CMakeLists.txt` and the `libpsn00b/cmake` directory for CMake setup scripts,
  got rid of the old makefiles and installation method.

- Updated docs to match the new build system, moved installation instructions
  to `INSTALL.md` and added `doc/cmake_reference.md` describing the SDK's CMake
  API and variables.

- mkpsxiso: Added rules to automatically download, build and install mkpsxiso
  (as well as its dependencies) as part of the SDK.

- cpack: Added initial CPack support and created the cpack directory containing
  branding assets for installers built through CPack. A shell script is
  provided to regenerate the assets from their source SVGs.

- liblzp: Copied library headers to `libpsn00b/include/lzp`.

- psxetc: Fixed a random typo in a javadoc comment (those should be eventually
  rewritten into proper documentation anyway).

- Added CMake, VS Code and DLL related entries to `.gitignore`.

## 2021-08-16: 0.16

spicyjpeg:

- psxetc: Added dynamic linking APIs (`dl*`, `DL_*`), implemented in `dl.c` and
  `_dl_resolve_wrapper.s`. The APIs can be accessed by including `dlfcn.h`,
  while another header (`elf.h`) provides structs and enums to help with manual
  parsing of library headers.

- libc: Removed `_mem_init.s`, improved heap initialization code. Added support
  for setting how much RAM to use for the stack (by defining `STACK_MAX_SIZE`)
  and for overriding/replacing the default memory allocator.

- psxapi: Added wrapper around BIOS function `FlushCache()`.

- psxspu: Fixed wrong bounds check in `SpuSetTransferStartAddr()`.

- examples: Added an example showing how to compile dynamically-loaded
  libraries (DLLs) and load them at runtime using the new dynamic linker.

- libpsn00b: Added `libpsn00b/ldscripts` directory and linker scripts for both
  executables and DLLs. Also fixed several broken header files (`psxgpu.h` not
  including `sys/types.h`, missing `scanf()` declarations, ...).

- Rewritten most makefiles (both for libraries and examples) and centralized
  all compiler flags into a single `psn00bsdk-setup.mk` file for consistency
  and easier editing. Added flags to build libpsn00b without gp-relative
  addressing for compatibility with the dynamic linker. Also added `nm`
  commands to generate symbol maps required by the linker.

## 2021-07-01

Lameguy64:

- Libpsn00b: Added `int8_t`, `int16_t`, `int32_t`, `int64_t`, `uint8_t`,
  `uint16_t`, `uint32_t` and `uint64_t` variable types in `sys/types.h`.
  
- psxgte: Replaced unsigned int variable types with `u_long` to further improve
  compatibility with code written for the official Sony SDK and to make my
  tutorial examples easier to compile on PSn00bSDK. Example programs have been
  updated to account for this change.
  
- psxcd: Changed type of 2nd argument of `CdRead()` from `u_int` to `u_long`,
  as well as changing the type of the size element in `CdlFILE` from `u_int` to
  `u_long`.
  
## 2021-02-17

Lameguy64:

- Improved build instructions in readme file and fixed some typos.

- Improved `toolchain.txt` instructions.

- tools: Added experimental `elf2cpe` converter for executing PSn00bSDK
  programs in official development units. No debug symbols however.

- Fixed prefixes to allow SDK libraries and examples to be built
  with `mipsel-none-elf`.
  
- examples: Fixed typo in `plasma_tbl.h` causing multiple definitions when
  compiling with newer versions of GCC in `n00bdemo`.

- examples: `cartrom` example now marked as obsoleted, but still kept for
  reference purposes.
  
- Includes alextrevisan's GTE macros in `inline_c.h`.

- Added makefile template.

## 2021-01-05

Lameguy64:

- psxgpu: Added struct names to many primitives.

- psxgte: Defined `SquareRoot0()` and `SquareRoot12()`.

- psxgte: Added `gte_lddp()` C macro.

- examples: Added C++ demo.

- Updated readme a bit.

## 2020-11-29

Lameguy64:

- libpsn00b: Defined MDEC hardware and related DMA registers in `hwregs_a.h`
  file.

- psxgte: Fixed entry typo in table of `squareroot.s` (pointed out by
  SoapyMan).

- libc: `memmove` updated to account for forward-looped memory move (by
  ckosmic).

- examples: Removed redundant toolchain executable definitions in the
  makefiles.
  
- examples: Included HDTV example for Github repo.

## 2020-09-19

Lameguy64:

- Revised makefiles for building the libraries and examples in a way to make
  building with different toolchain versions easier with the `PSN00BSDK_TC`
  environment variable. Library installation and linking is also made easier
  with the `PSN00BSDK_LIBS` environment variable. See readme in the `libpsn00b`
  directory for details.
  
- examples: Fixed libgcc not found error when compiling some of the examples.

- libc: Added `strtok()`.

- libc: Added support for command line arguments. Pass arguments via
  `SYSTEM.CNF` `BOOT=` string or a string array with `Exec()`. Arguments can
  be read via `argc`/`argv[]` in `main()` or `__argc`/`__argv` anywhere else.
  
- libc: Added `SetHeapSize()`.

- psxgpu: Moved ISR and callback subsystem to `psxetc`. You'll have to link
  psxetc after `psxgpu` and `psxapi` after `psxetc` in your library string.

- psxetc: Moved debug font functions (`FntInit()`, `FntOpen()`, etc) to `psxgpu`.

- psxetc: Fixed stack management in `RestartCallback()`.

- examples: Added argument passing in `childexec` example.

- psxcd: Fixed crashing on PSIO and possibly some emulators by implementing
  a response buffer read limiter.
  
- psxgpu: Interrupts are now disabled before setting up ISR and callbacks in
  `ResetGraph()`, as `LoadExec()` still has interrupts enabled when jumping to
  the loaded PS-EXE's entrypoint. Fixes programs made with PSn00bSDK crashing
  at `ResetGraph()` on PSIO when loading from the Menu System and possibly some
  emulators.

## 2020-07-25

Lameguy64:

- psxgte: Added `ScaleMatrixL()`.

- doc: Corrected documentation for `CdReadSync()` function.

- psxcd: Added new define: `CdlIsoLidOpen`.

- psxcd: Updated media change detection logic, media change is checked
  by lid open status bit in all CD-ROM file functions. `CdControl()` calls will
  also trigger the media change status on lid open.
  
- psxcd: Fixed bug in `CdGetVolumeLabel()` where it constantly reparses the
  file system regardless of media change status.
  
- examples: Updated `cdrom/cdbrowse` example slightly.

- psxcd: Added `CdLoadSession()`.

- psxcd: Fixed bug where `CdReadDir()` locks up in an infinite loop when it
  encounters a NULL directory record, and the parser has not yet exceeded the
  length of the directory record.
  
- doc: Replaced library version numbers with SVN revision numbers in the
  introduced fields.
  
- doc: Started work on CD-ROM library overview.

## 2020-04-24

Lameguy64:

- Refined toolchain instructions a bit.

- psxcd: Added automatic read retry for `CdRead()` operations, as long as
  `CdReadSync()` is called until read completion.

- psxapi: Added BIOS `atoi()` and `atol()` calls. Temporary, may be replaced
  with a faster implementation.
  
- psxsio: Added `ioctl()` support for `FIOCSCAN` to probe for pending input in
  serial tty driver.
  
- examples: Reorganized examples, added new `tty` and `console` examples.

## 2020-03-11

Lameguy64:

- psxcd: Fixed `CdInit()` syntax (there should be no arguments).

- psxcd: Fixed `CdlFILE`, `loc` variable renamed to `pos`.

- documentation: Improved and updated toolchain instructions a bit.

## 2020-02-28

Lameguy64:

- documentation: Added docs for `CdOpenDir()`, `CdReadDir()` and
  `CdCloseDir()`.

- psxgpu: Added `GetODE()`.

- psxspu: Fixed `SpuKeyOn()` bug where only 16 of the 24 channels can be
  activated, due to channel bits being written as a 16-bit word.

- psxcd: Added `CdOpenDir()`, `CdReadDir()` and `CdCloseDir()` for parsing
  directories.

- psxcd: Issuing `CdlNop` (`GetStat`) commands now triggers the media change
  flag internal to the libraries when CD lid is opened, so file system
  functions can update the cached ISO descriptor when the disc has been
  changed.
  
- psxcd: Made internal variables and functions for iso9660 parsing static.

## 2020-02-25

thp:

- libc: Added `abs()` and `labs()` functions.

## 2020-01-06

Lameguy64:

- libpsn00b: Renamed `libpsxcd` directory to `psxcd` to match with other
  libraries.

- examples: Added obligatory hello world example.

- documentation: Added CD-ROM library documentation.

## 2019-11-22: 0.15b

Lameguy64:

- psxapi: Added root counter or timer functions and related definitions.

- examples: Added timer example.

- psxgpu: Added `DR_AREA`, `DR_OFFSET` and `DR_TWIN` primitives and
  accompanying `setDrawArea()`, `setDrawOffset()` and `setTexWindow()` macros.

- psxgpu: Added parenthesis to argument value in `setlen()`, `setaddr()` and
  `setcode()` macros, preventing `addPrims()` from being used in a more
  sensible manner (ie. `addPrims(ot, sub_ot+3, sub_ot)`).
  
- examples: Added render2tex render to texture example.

- psxspu: Fixed typo in `spuinit.s` on section specifier specifying a data
  section instead of text section, resulting to jump to
  non-instruction-aligned linker errors.
  
- psxgpu: Increased ISR stack size to 2048 bytes.

- psxsio: Added `kbhit()` to poll keyboard input asynchronously and stdin
  is now buffered with an IRQ handler.

- psxapi: Added `AddDummyTty()` (for psxsio `DelSIO()` fix).

- psxsio: `DelSIO()` now calls `AddDummyTty()`.

- libc: Fixed bug in `strncpy()` not placing a NULL byte at end of string.

- libc: Fixed `strchr()` and `strrchr()` declarations commented out in
  `string.h`.

- libpsn00b: Added the long awaited libpsxcd library with cdxa example.
  Documentation will come soon but existing libcd docs should be good
  enough for awhile.

- psxgpu: Fixed non functioning GPU DMA wait in `DrawSync()`.

- LibPSn00b run-time library is now officially 0.15b.

## 2019-10-11

Lameguy64:

- psxetc: Added `FntOpen()`, `FntPrint()` and `FntFlush()` functions.

- psxgpu: Fixed typo in `termPrim()` macro (value too long).

- examples: Added billboarding sprites example.

- psxapi: Transferred `putchar()` BIOS function to libc.

- libpsn00b: Updated makefiles adding `-fdata-sections` and
  `-ffunction-sections` so unused functions will be stripped out, which yields
  smaller executables.

- libc: Fixed negative integers not displaying properly in
  `vsprintf()`/`vsnprintf()`.
  
- libc: Fixed zero padding not working in `vsprintf()`/`vsnprintf()`.

- fpscam: Added debug text using `FntOpen()`, `FntPrint()` and `FntFlush()`.
  Also added `_boot()` call for returning to CD based serial loaders.

## 2019-09-23

Lameguy64:

- libc: Added `strcat()` function.

- Included PSn00bSDK logo vector.

- psxgte: Added `gte_stsz()` macro.

- psxgpu: Fixed typos in `setUVWH()` macro.

- Added `_boot()` BIOS function (`A(A0h)` aka Warmboot, useful for CD based
  serial loaders).

## 2019-08-17: 0.13b

Lameguy64:

- LibPSn00b run-time library is officially version 0.13b.

- examples: updated balls and n00bdemo examples for the `setDrawTPage()`
  correction.

- psxgpu: Fixed error on `setDrawTPage()` syntax and removed
  `setDrawTPageVal()` macro (not needed).

- psxapi: Added event handler definitions related to memory cards.

- psxapi: Added BIOS memory card functions.

- examples: Added childexec example demonstrating a parent executable
  transferring execution to a child executable and back.

- elf2x: `s_addr` no longer set on PS-EXE header, console BIOS already sets it
  by STACK value in `SYSTEM.CNF` and allows child executables to return to
  parent if left zero.

- psxapi: Renamed `_InitPad()`, `_StartPad()` and `_StopPad()` to `InitPAD()`,
  `StartPAD()` and `StopPAD()` respectively. Fpscam example updated to new
  syntax.

## 2019-07-18

Lameguy64:

- Added `fpscam` example.

- Gave examples small descriptions.

- Updated readme a little.

## 2019-07-17: 0.12b

Lameguy64:

- LibPSn00b run-time library is officially version 0.12b.

- libc: Added basic C++ support (many thanks to ijacquez).

- libc: Updated start function that should make it possible for a child
  executable to return to a parent executable, return logic automatically
  calls `EnterCriticalSection()`.

- libc: Updated build method which takes `libgcc` from the compiler and adds
  its own object files into it, eliminating linker problems caused by having
  to order `libc` and `libgcc` libraries in a specific manner.
  
- psxgpu: Added `RestartCallback()`.

- psxgpu: Added `StoreImage()` function.

- psxgpu: Fixed bugged `setRECT()` macro.

- libc: Added `assert.h`.

- examples: Balls example now has 166% more balls.

- psxgpu: Increased ISR stack size to 1024 bytes so printf can be used in
  callbacks safely.

- libc: Removed `int64` (long long) printing in `vsprintf()` for better
  performance, as the R3000 does not support 64-bit arithmetic natively
  so its emulated like floats. `int64` still used for processing floats and
  doubles and old `vsprintf.c` file is still included for those who really
  want `int64` support for whatever reason.
  
- libc: Removed `stdarg.h` which is part of GCC and not license compatible
  with MPL. The toolchain compiled with libgcc provides `stdarg.h` and other
  standard headers.
  
- examples: Updated `sdk-common.mk` variable convention for better flexibility.

- libpsn00b: Added `common.mk` file containing global values for all libraries.

- Updated library and toolchain build instructions.

- psxgpu: Fixed bug in DMACallback where the internal DMA handler would fail
  to install due to `GetInterruptCallback()` retrieving the callback value
  immediately in the branch delay slot of a `jr` instruction, which resuls to
  an inconsistent return value. This also broke `DrawSyncCallback()`.
  
- psxsio: Done fixes on `_sio_control()` from the aformentioned issues with
  load instructions in delay slots.
  
- psxgte: Added `DVECTOR` struct.

- psxgpu: Added `setLineF2()`, `setLineG2()`, `setLineF3()` and `setLineG3()`
  primitive macros.
  
- Added more functions in documentation.
  
## 2019-07-01

williamblair:

- Fixed `FntLoad()` Y coordinate not working properly for debug font (due to
  Y coordinate not being specified for the `getTPage()` macro.

## 2019-06-23: 0.10b

Lameguy64:

- LibPSn00b Run-time Library is officially version 0.10b.

- Reworked readme file with improved build instructions.

- libpsn00b: Added missing C extern groups for C++ compatibility in
  `psxapi.h`, `stdlib.h` and `string.h`.

- libpsn00b: Removed changelogs in the readmes of each libpsn00b library
  as its much easier to keep track of changes in a single changelog than
  scattering them across multiple changelogs.

- psxapi: Added `Exec()` function and EXEC struct.

- psxgpu: Added `DrawPrim()` function (uses direct I/O).

- psxgpu: `VSync()` function completely overhauled. Logic is now based on
  Sony's code but more efficient and can return total number of vblanks
  elapsed, number of hblanks since last call and wait up to n vblanks
  specified by an appropriate argument value. It will also generate a timeout
  when vblank interrupt stops working and would attempt to restart it.

- psxapi: Added `gets()` and `getc()` BIOS functions.

- psxapi: Corrected a `putc()`/`putchar()` discrepancy. `putc()` puts a
  character to a file stream, `putchar()` puts a character to stdout.

- Completely revised library reference manual with better formatting and
  more functions documented.

- psxgpu: Added `DrawSyncCallback()`.

- psxgpu: Implemented DMA interrupt callback system with `DMACallback()`
  allowing to handle DMA interrupts very easily.

- libpsn00b: Implemented Serial I/O library (`psxsio`) with serial tty device
  (installed using `AddSIO`) and serial callback support. Serial library
  is also fully documented.

- psxgpu: Implemented IRQ callback system with `InterruptCallback()` allowing
  to set interrupt callbacks very easily.
  
- psxgpu: Implemented proper IRQ handler installed using HookEntryInt or
  `SetCustomExitFromException()` for handling VSync and other interrupts.
  `ChangeClearPAD(0)` must now be called after `_InitPad()` or vsync timeout
  will occur.

- libpsn00b: Started making local labels prefixed with `.L` instead of `.`
  making them not appear in symbol lists resulting in a cleaner symbol dump.
  Still not possible to do function-scope local labels like in ASMPSX because
  GAS syntax is ASS (or ASS GAS which is farts, GAS is farts).
  
- psxgpu: `DrawSync()` function now waits for DMA completion and GPU transfer
  ready instead of simply waiting for GPU transfer ready which is the likely
  cause of subtle GPU related timing issues, it also sets GPU DMA transfer
  mode to off afterwards. It can also read number of words remaining in DMA
  transfer if `a0` is non-zero but it likely only returns the correct value on
  VRAM transfers. Exact way how `DrawSync()` returns the count in the official
  SDK is currently unknown.

## 2019-06-07

qbradq:

- lzpack: Swapped a buffer length litteral with sizeof operator, fixing a stack
  overflow bug in some instances.

## 2019-05-23

Lameguy64:

- Added `dev notes.txt` file in docs that includes notes about the many quirks
  quirks about the console discovered during the development of this SDK
  project.

- Updated `libn00bref.odf` a little.

- Added `cartrom` example.

- Added `rgb24` example.

- Got custom exit handler set using `SetCustomExitFromException()` (BIOS
  function `B(19h)`) working. Currently used to acknowledge VSync IRQ but
  actual VSync handling is still done with events and needs to be
  transferred to the custom exit handler. At least it lets BIOS
  controller functions to work now. See `doc/dev notes.txt` for details
  on how this handler behaves.

- Made stack usage in `ResetGraph()` less wasteful. You only need to
  allocate N words on stack based on N arguments of the function
  being called.

- VSync IRQ handling now done using a custom exit from exception handler.
  Actual VSync handling is yet to be moved to the custom exit handler
  though.

- Made stack usage in `start.s` of libc a lot less wasteful.

- Implemented controller support via BIOS functions (use `_InitPad()`,
  `_StartPad()` and `_StopPad()`). BIOS memory card functions may also
  work as well but its not tested yet.

- Removed duplicate `initpad.s` and `initcard.s` functions in psxapi.

- Added `_InitCd()` function to psxapi which is a safer version of `_96_init()`
  as it preserves other DMA channel settings. Use BIOS file functions such
  as `open()`, `read()` and `close()` with path names starting with `cdrom:/`
  to access files from CD after calling `_InitCd()`.
