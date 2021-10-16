
# Getting started with PSn00bSDK

## Building and installing

The instructions below are for Windows and Linux. Building on macOS hasn't been
tested but should work.

1. Set up a host compiler. Most Linux distros provide a `build-essential`,
   `base-devel` or similar all-in-one package. You'll also need to install the
   [Ninja](https://ninja-build.org) build engine (it's usually in a package
   called `ninja-build`). On Windows install [MSys2](https://www.msys2.org),
   then run the following command in the MSys2 shell to install MinGW and Ninja:

   ```bash
   pacman -Syu mingw-w64-x86_64-gcc mingw-w64-x86_64-ninja
   ```

   Add `C:\msys64\mingw64\bin` (replace `C:\msys64` if you installed MSys2 to a
   different location) to the `PATH` environment variable using System
   Properties.

2. Install Git and CMake. Note that MSys2 and some Linux distros ship relatively
   old versions (PSn00bSDK requires 3.21+), so grab the latest CMake release
   from [here](https://cmake.org) instead of through your package manager.

3. Build and install a GCC toolchain for `mipsel-unknown-elf`, as detailed in
   [TOOLCHAIN.md](TOOLCHAIN.md). On Windows, you may download a precompiled
   version from [Lameguy64's website](http://lameguy64.net?page=psn00bsdk) and
   extract it into one of the directories listed below instead.

4. If you chose a non-standard install location for the toolchain, add the `bin`
   subfolder (inside the top-level toolchain directory) to the `PATH`
   environment variable. This step is unnecessary if you installed/extracted the
   toolchain into any of these directories:

   - `C:\Program Files\mipsel-unknown-elf`
   - `C:\Program Files (x86)\mipsel-unknown-elf`
   - `C:\mipsel-unknown-elf`
   - `/usr/local/mipsel-unknown-elf`
   - `/usr/mipsel-unknown-elf`
   - `/opt/mipsel-unknown-elf`

5. Clone/download the PSn00bSDK repo and run the following commands from its
   directory:

   ```bash
   cmake -S . -B ./build -G Ninja
   cmake --build ./build
   ```

   If you want to install the SDK to a custom location rather than the default
   one (`C:\Program Files\PSn00bSDK` or `/usr/local` depending on your OS), add
   `--install-prefix <INSTALL_PATH>` to the first command. Remove `-G Ninja` to
   use `make` instead of Ninja (slower, not recommended).

   If you run into errors, try passing `-DSKIP_TINYXML2=ON` to the first command
   after installing `tinyxml2` manually. [See below](#advanced-build-options)
   for more details.

6. Install the SDK to the path you chose by running this command (add `sudo` if
   necessary):

   ```bash
   cmake --install ./build
   ```

   This will create and populate the following directories:

   - `<INSTALL_PATH>/bin`
   - `<INSTALL_PATH>/lib/libpsn00b`
   - `<INSTALL_PATH>/share/psn00bsdk`

7. Set the `PSN00BSDK_LIBS` environment variable to point to the `lib/libpsn00b`
   subfolder inside the install directory. You might also want to add the `bin`
   folder to `PATH` if it's not listed already.

Although not strictly required, you'll probably want to install a PS1 emulator
with debugging capabilities such as [no$psx](https://problemkaputt.de/psx.htm)
(Windows only), [DuckStation](https://github.com/stenzek/duckstation) or
[pcsx-redux](https://github.com/grumpycoders/pcsx-redux).
**Avoid ePSXe and anything based on MAME** as they are inaccurate.

## Advanced build options

### Skipping external dependency downloads

By default [mkpsxiso](https://github.com/Lameguy64/mkpsxiso) (required for
building CD images) and [tinyxml2](https://github.com/leethomason/tinyxml2)
(required to build mkpsxiso and other SDK tools) are automatically cloned from
their respective repos and built as part of the PSn00bSDK build process,
*even if they are already installed*.

If you wish to disable this behavior (e.g. because it leads to errors, or to
perform an offline build), invoke CMake with the `-DSKIP_MKPSXISO=ON` and/or
`-DSKIP_TINYXML2=ON` options when configuring the SDK. Note that you must have
`mkpsxiso` and/or `tinyxml2` already installed (either manually or via vcpkg or
your distro's package manager) to be able to skip them.

### Building installer packages

CPack can be used to build NSIS-based installers, DEB/RPM packages and zipped
releases that include built SDK libraries, headers as well as the GCC toolchain.
Distributing prebuilt releases is however discouraged since PSn00bSDK is still
far from being feature-complete.

1. Follow steps 1-4 above to set up the toolchain, then install
   [NSIS](https://nsis.sourceforge.io/Download) on Windows or `dpkg` and `rpm`
   on Linux.

2. Run the following commands from the PSn00bSDK directory:

   ```bash
   cmake -S . -B ./build -G Ninja -DBUNDLE_TOOLCHAIN=ON
   cmake --build ./build -t package
   ```

   All built packages will be copied to the `build/packages` folder.

   **NOTE**: do not use `-DSKIP_MKPSXISO=ON`, otherwise the mkpsxiso binary will
   not be included in the packages.

## Creating a project

1. Copy the contents of `INSTALL_PATH/share/psn00bsdk/template` (or the
   `template` folder within the repo) to your new project's root directory.

2. Configure and build the template by running:

   ```bash
   cmake -S . -B ./build -G Ninja
   cmake --build ./build
   ```

   If you did everything correctly there should be a `template.bin` CD image in
   the `build` folder. Test it in an emulator to ensure it works.

Note that, even though the template relies on the `PSN00BSDK_LIBS` environment
variable to locate the SDK by default, you can also specify the path directly
on the CMake command line by adding
`-DCMAKE_TOOLCHAIN_FILE=INSTALL_PATH/lib/libpsn00b/cmake/sdk.cmake` (replace
`INSTALL_PATH`) to the first command.

The toolchain script defines a few CMake macros to create PS1 executables, DLLs
and CD images. See the [reference](doc/cmake_reference.md) for details.

-----------------------------------------
_Last updated on 2021-10-18 by spicyjpeg_
