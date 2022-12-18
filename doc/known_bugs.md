
# Known PSn00bSDK bugs

This is an incomplete list of things that are known to be currently broken (or
not behaving as they should, or untested on real hardware) and haven't yet been
fixed.

## Toolchain

- ~~It is currently not possible to link static libraries (including the SDK~~
  ~~libraries themselves) with DLLs, since the build scripts currently assume~~
  ~~that static library object files are always going to be linked into~~
  ~~executables. This can be worked around by linking all static libraries as~~
  ~~part of the main executable rather than the DLLs: the dynamic linker will~~
  ~~automatically search the executable for undefined symbols used by a DLL~~
  ~~and patch the code to use them.~~ Static libraries are now fully supported,
  and SDK libraries can be linked to both executables and DLLs. See the CMake
  reference for more details.

- Link-time optimization is broken due to GCC not supporting it when linking
  weak functions written in assembly.

## Libraries

`psxgpu`:

- `LoadImage()` and `StoreImage()` use DMA to transfer data to/from the GPU.
  As the DMA channel is configured to transfer 8 words (32 bytes) at a time,
  the length of the data *must* be a multiple of 32 bytes. Attempting to
  transfer any data whose length isn't a multiple of 32 bytes will result in
  `DrawSync()` hanging and never returning, however a warning will be printed
  on the debug console if the executable is built in debug mode.

`psxspu`:

- `SpuInit()`, `SpuRead()` and `SpuWrite()` may take several seconds on MAME
  due to the SPU status register being emulated incorrectly. They work as
  expected on other emulators as well as on real hardware.

`psxcd`:

- The library seems to get into an unpredictable state when removing discs on
  pcsx-redux (and possibly real hardware?).

## Examples

See [README.md in the examples directory](../examples/README.md#examples-summary).

-----------------------------------------
_Last updated on 2022-12-18 by spicyjpeg_
