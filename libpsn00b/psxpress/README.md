
# PSn00bSDK MDEC library

This is a fully original reimplementation of the official SDK's "data
compression" library. This library is made up of two parts, the MDEC API and
functions to decompress Huffman-encoded bitstreams (.BS files, or frames in
.STR files) into data to be fed to the MDEC. Two different implementations of
the latter are provided, one using the GTE and scratchpad region and an older
one using a large lookup table in main RAM.

FMV playback is not part of this library per se, but can implemented using the
APIs defined here alongside some code to stream data from the CD drive.

Currently bitstream versions 1, 2 and 3 are supported. Version 0 and .IKI
bitstreams are not supported, but no encoder is publicly available for those
anyway.

## MDEC API

The MDEC data input/output API is almost identical to the official one, with
only two minor differences:

- `DecDCTPutEnv()` takes a second argument specifying whether the MDEC shall be
  put into monochrome or color mode (the original API only supported color).
- A `DecDCTinRaw()` function was added for easier feeding of headerless data
  buffers to the MDEC. This function does not affect how `DecDCTin()` works.

## Decompression API

The following functions are currently provided:

- `DecDCTvlcStart()`, `DecDCTvlcContinue()`: a decompressor implementation that
  uses a small (<1 KB) lookup table and leverages the GTE, written in assembly.
  `DecDCTvlcCopyTableV2()` or `DecDCTvlcCopyTableV3()` may optionally be called
  to temporarily move the table to the scratchpad region in order to boost
  decompression speed.
- `DecDCTvlcStart2()`, `DecDCTvlcContinue2()`: an older implementation using
  a large (34 KB) lookup table in main RAM, written in C. The table must be
  decompressed ahead of time manually using `DecDCTvlcBuild()`, but can be
  deallocated when no longer needed. **This implementation does not support**
  **version 3 bitstreams**.
- `DecDCTvlc()`, `DecDCTvlc2()`: wrappers around the functions listed above,
  for compatibility with the Sony SDK.

## SPU ADPCM encoding API

The Sony library has functions that can be used to convert raw 16-bit PCM audio
data to SPU ADPCM. These are currently unimplemented due to their limited
usefulness, but might be added at some point.
