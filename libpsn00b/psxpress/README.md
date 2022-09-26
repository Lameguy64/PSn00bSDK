
# PSn00bSDK MDEC library

This is a fully open source reimplementation of the official SDK's "data
compression" library. This library is made up of two parts, the MDEC API and
functions to decompress Huffman-encoded bitstreams (.BS files, or frames in
.STR files) into data to be fed to the MDEC. FMV playback is not part of this
library (nor the official one) per se, but can implemented by using these APIs
alongside some code to stream data from the CD drive.

**Currently only version 1 and 2 bitstreams are supported**.

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
  `DecDCTvlcCopyTable()` can optionally be called to temporarily move the table
  to the scratchpad region to improve decompression speed.
- `DecDCTvlcStart2()`, `DecDCTvlcContinue2()`: a different implementation using
  a large (34 KB) lookup table in main RAM, written in C. The table must be
  decompressed ahead of time using `DecDCTvlcBuild()`, but can be deallocated
  when no longer needed.
- `DecDCTvlc()`, `DecDCTvlc2()`: wrappers around the functions listed above,
  for compatibility with the Sony SDK. Using them is not recommended.

## SPU ADPCM encoding API

The Sony library has functions that can be used to convert raw 16-bit PCM audio
data to SPU ADPCM. These are currently unimplemented due to their limited
usefulness, but might be added at some point.
