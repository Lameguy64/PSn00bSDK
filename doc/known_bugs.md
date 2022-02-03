
# Known PSn00bSDK bugs

This is an incomplete list of things that are currently broken (or not behaving
as they should, or untested on real hardware) and haven't yet been fixed.

## Toolchain

- It is currently not possible to link static libraries (including the SDK
  libraries themselves) with DLLs, since the build scripts currently assume that
  static library object files are always going to be linked into executables.
  This can be worked around by linking all static libraries as part of the main
  executable rather than the DLLs: the dynamic linker will automatically search
  the executable for undefined symbols used by a DLL and patch the code to use
  them. It might be necessary to list such symbols in a dummy array to prevent
  the compiler from stripping them away from the executable.

## Libraries

`psxgpu`:

- In some *very rare* cases, `VSync()` seems to crash the system by performing
  unaligned accesses for unknown reasons.

`psxspu`:

- Calls to `SpuSetTransferMode()` are ignored. SPU transfers are always
  performed using DMA, which imposes limitations such as the data length having
  to be a multiple of 64 bytes.

`psxetc`:

- `DL_LoadSymbolMapFromFile()`, `DL_LoadDLLFromFile()` and `dlopen()` have been
  disabled due to bugs in the BIOS file APIs. The dynamic linker can still be
  used by loading DLL binaries into RAM manually and calling `DL_CreateDLL()`
  on them (see the `system/dynlink` example).

## Examples

See [README.md in the examples directory](../examples/README.md#examples-summary).

-----------------------------------------
_Last updated on 2022-02-03 by spicyjpeg_
