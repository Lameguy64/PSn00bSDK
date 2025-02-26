# PlayStation System Overview

**CPU:** Customized MIPS R3000A 32-bit RISC CPU (might be based on the
LSI L64360 embedded CPU)

-   Clocked at 33.8MHz.
-   32 32-bit general purpose registers (35 total if special hi, lo and
    pc registers are included, first register locked to zero).
-   4KB I-cache.
-   1KB D-cache repurposed as fast \'scratchpad\' memory.
-   Geometry Transformation Engine (GTE) for 3D vector calculations
    (accessed as cop2).
-   No hardware floating point unit.

**Graphics:** Custom GPU

-   Clocked at 33.8MHz (same as CPU, may need verification however).
-   Supports display resolutions of 256x240 to 640x480 for NTSC and
    256x256 to 640x512 for PAL (272 vertical lines is apparently
    possible for PAL as well).
-   Supports displaying in 16-bit or 24-bit color depths (GPU only draws
    primitives in 16-bit).
-   RGB555 pixel format (32K colors total) with mask bit for
    transparency/write protect masks.
-   1MB of VRAM (connected to GPU with 16-bit memory bus, not directly
    accessible by CPU).
-   2KB texture cache.
-   Flat and shaded lines and polylines of up to virtually unlimited
    points.
-   Fixed sized (1x1, 8x8, 16x16) and variable size flat and textured
    sprites.
-   Flat and textured 3-point and 4-point polygons with optional shading
    (quads are drawn as two triangles).
-   Supports 4-bit, 8-bit and 16-bit textures. 4-bit and 8-bit textures
    use color lookup tables or CLUTs as palettes.
-   4 semi-transparency blending modes.
-   24-bit color processing with final output converted to 16-bit with
    optional dithering (flawed on old 160pin GPUs resulting in banding
    on shaded textures).
-   GPU draws entirely in 2D resulting to affine distortion and various
    depth clipping issues.
-   VRAM addressed as a 1024x512 16bpp framebuffer divided into 32
    64x256 pixel pages (when texture mapping). Used for display/draw
    buffers, textures and CLUTs.

**Sound:** Custom SPU

-   24 voices with ADSR, pitch and modulation parameters.
-   Noise generator on any of the 24 voices.
-   512KB SPU RAM (not accessible by CPU).
-   44.1KHz mixing rate.
-   Only supports ADPCM samples with loop start and loop end flags.
-   Hardware reverb with programmable reverb registers.
-   CD-ROM digital audio input and external digital audio input via
    expansion port.

**CD-ROM:** 2x speed CD-ROM drive

-   Roughly 300KB/s maximum read speed.
-   Wobble groove based disc authentication scheme (easily defeated by
    modchips).
-   Supports CD-DA and CD-XA ADPCM audio playback.
-   Hardware ADPCM decoder for CD-XA ADPCM audio tracks.
-   CD-XA ADPCM sample rates of 18.9KHz or 37.8KHz in Mono or Stereo.
-   Up to 32 ADPCM streams per CD-XA track (one channel at a time in
    playback, channel switching possible during playback).
-   Supports simultaneous CD-XA playback and data streaming (commonly
    used in FMVs in conjunction with the MDEC).

**Memory:**

-   2MB DRAM (first 64KB of RAM used by system kernel).
-   1MB VRAM.
-   512KB SPU RAM.
-   512KB BIOS ROM.
-   Expansion ROM via expansion port up to 512KB supported (in default
    mapping).

**Storage:** Memory Cards

-   128KB total capacity.
-   1 block is 8KB each.

---

[Back to Index](index.md)
