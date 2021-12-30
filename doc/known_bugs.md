
# Known PSn00bSDK bugs

This is an incomplete list of things that are currently broken (or not behaving
as they should, or untested on real hardware) and haven't yet been fixed.

## Libraries

`psxspu`:

- Calls to `SpuSetTransferMode()` are ignored. SPU transfers are always
  performed using DMA, which imposes limitations such as the data length having
  to be a multiple of 64 bytes.

`psxetc`:

- `DL_LoadSymbolMapFromFile()`, `DL_LoadDLLFromFile()` and `dlopen()` have been
  disabled due to bugs in the BIOS file APIs. The dynamic linker can still be
  used by loading DLL binaries into RAM manually and calling `DL_CreateDLL()`
  on them (see the `system/dynlink` example).

## Tools

- The `mkpsxiso` submodule is temporarily set to point to a fork of `mkpsxiso`
  with bugfixed CMake scripts (the main repo is broken to the point it fails to
  build). There is [another fork](https://github.com/CookiePLMonster/mkpsxiso)
  which is currently work-in-progress and includes more fixes as well as a tool
  to dump existing CD images: PSn00bSDK will switch back to the main `mkpsxiso`
  repo once the changes get upstreamed.

## Examples

- `cdrom/cdxa` and `sound/spustream` demonstrate how to stream an audio file
  from CD-ROM. Such a file isn't provided however, as PSn00bSDK does not yet
  come with the tooling required for transcoding audio from a source file. In
  order to run these examples you'll have to provide your own audio files,
  convert them and build the CD image manually.

- `demos/n00bdemo` suffers from flickering on real hardware, especially when
  masking/stencil buffering is used.

- `graphics/render2tex` gets stuck after initialization on real hardware.

- `io/pads` seems to work on real hardware, but fails to automatically enable
  analog mode on DualShock controllers. This example needs more testing with
  official and unofficial controllers.

- `io/system573` hasn't been tested on a real Konami System 573. It runs on
  MAME, however MAME's System 573 emulation is *very* inaccurate.

-----------------------------------------
_Last updated on 2021-12-30 by spicyjpeg_
