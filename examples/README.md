
# PSn00bSDK Example Programs

## Examples summary

The following list is a brief summary of all the example programs included.
Additional information may be found in the source code of each example.

| Path                                           | Description                                           | Type | Notes |
| :--------------------------------------------- | :---------------------------------------------------- | :--: | :---: |
| [`beginner/hello`](./beginner/hello)           | A simple "hello world" example/project template       | EXE  |       |
| [`beginner/hellocpp`](./beginner/hellocpp)     | C++ version of the example above                      | EXE  |       |
| [`cdrom/cdbrowse`](./cdrom/cdbrowse)           | File browser using libpsxcd's directory functions     | CD   |       |
| [`cdrom/cdxa`](./cdrom/cdxa)                   | CD-XA ADPCM audio player                              | CD   |   1   |
| [`demos/n00bdemo`](./demos/n00bdemo)           | The premiere demonstration program of PSn00bSDK       | EXE  |   2   |
| [`graphics/balls`](./graphics/balls)           | Draws colored balls bouncing around the screen        | EXE  |       |
| [`graphics/billboard`](./graphics/billboard)   | Demonstrates how to draw 2D sprites in a 3D space     | EXE  |       |
| [`graphics/fpscam`](./graphics/fpscam)         | First-person perspective camera with look-at          | EXE  |       |
| [`graphics/gte`](./graphics/gte)               | Displays a rotating cube using GTE macros             | EXE  |       |
| [`graphics/hdtv`](./graphics/hdtv)             | Demonstrates anamorphic widescreen at 704x480         | EXE  |       |
| [`graphics/render2tex`](./graphics/render2tex) | Procedural texture effects using off-screen drawing   | EXE  |       |
| [`graphics/rgb24`](./graphics/rgb24)           | Displays an uncompressed 640x480 24-bit RGB image     | EXE  |       |
| [`graphics/tilesasm`](./graphics/tilesasm)     | Drawing a tile-map with assembly language             | EXE  |       |
| [`io/pads`](./io/pads)                         | Demonstrates reading controllers via low-level access | EXE  |   3   |
| [`io/system573`](./io/system573)               | Konami System 573 (PS1-based arcade board) example    | CD   |       |
| [`lowlevel/cartrom`](./lowlevel/cartrom)       | ROM firmware for cheat devices written using GNU GAS  | ROM  |   4   |
| [`mdec/mdecimage`](./mdec/mdecimage)           | Displays a (raw) MDEC format image                    | EXE  |       |
| [`mdec/strvideo`](./mdec/strvideo)             | Plays a .STR video file using the MDEC                | CD   |   1   |
| [`sound/cdstream`](./sound/cdstream)           | Streams an interleaved .VAG file from the CD-ROM      | CD   |       |
| [`sound/spustream`](./sound/spustream)         | Streams an interleaved .VAG file from main RAM        | EXE  |       |
| [`sound/vagsample`](./sound/vagsample)         | Loads and plays .VAG sound files using the SPU        | EXE  |       |
| [`system/childexec`](./system/childexec)       | Loading a child program and returning to parent       | EXE  |       |
| [`system/console`](./system/console)           | TTY based text console that interrupts gameplay       | EXE  |       |
| [`system/dynlink`](./system/dynlink)           | Demonstrates dynamically linked libraries             | CD   |       |
| [`system/timer`](./system/timer)               | Demonstrates using hardware timers with interrupts    | EXE  |       |
| [`system/tty`](./system/tty)                   | Using TTY as a remote text console interface          | EXE  |       |

Notes:

1. `cdrom/cdxa` and  `mdec/strvideo` do not come with example files. In order
   to run these examples you'll have to provide your own files and build the CD
   image manually.
2. `demos/n00bdemo` suffers from flickering on real hardware, especially when
   masking/stencil buffering is used.
3. `io/pads` seems to work on real hardware, but fails to automatically enable
   analog mode on DualShock controllers. This example needs more testing with
   official and unofficial controllers.
4. The `lowlevel/cartrom` example is outdated and does not use SDK libraries.
   It is kept for reference purposes only.

## Building the examples

The instructions below assume that PSn00bSDK, CMake 3.21+ and a GCC toolchain
are already installed. Refer to the [installation guide](../doc/installation.md)
for details.

**NOTE**: all examples are compiled by default when building the PSn00bSDK
libraries and tools (check the `build/examples` directory). These instructions
are for rebuilding the examples *after* the SDK has been installed.

1. Copy the contents of this directory (`share/psn00bsdk/examples` within the
   PSn00bSDK installation directory) to your home directory or to another
   folder you have write access to.

2. Configure and build the examples by running:

   ```bash
   cmake -S . -B ./build -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=<INSTALL_PATH>/lib/libpsn00b/cmake/sdk.cmake
   cmake --build ./build
   ```

   Replace `<INSTALL_PATH>` with the installation prefix you chose when
   installing the SDK (usually `C:\Program Files\PSn00bSDK` or `/usr/local`,
   so the full path to `sdk.cmake` would be
   `C:\Program Files\PSn00bSDK\lib\libpsn00b\cmake\sdk.cmake` or
   `/usr/local/lib/libpsn00b/cmake/sdk.cmake` respectively).

   Add `-DPSN00BSDK_TARGET=mipsel-unknown-elf` to the first command if your
   toolchain targets `mipsel-unknown-elf` rather than `mipsel-none-elf`. If you
   can't get Ninja to work or don't have it installed, you can also replace
   `-G "Ninja"` with `-G "Unix Makefiles"` (`-G "MinGW Makefiles"` on Windows)
   to build using `make` instead.

   This should create a `build` directory whose structure mirrors the one of
   the parent directory, with each subfolder containing built executables and
   CD images for each example.

-----------------------------------------
_Last updated on 2023-05-21 by spicyjpeg_
