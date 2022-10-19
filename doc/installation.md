
# Getting started with PSn00bSDK

## Installing a prebuilt release (recommended)

1. Install prerequisites. Currently CMake is the only external dependency; you
   can install it from [here](https://cmake.org/download) or using MSys2 or
   your distro's package manager. Make sure you have at least CMake 3.21.

2. Head over to the releases page, download the latest release's ZIP for your
   operating system and extract its contents to a directory of your choice,
   preferably one whose absolute path is short (such as `C:\PSn00bSDK` or
   `/opt/psn00bsdk`). Proceed to add the `bin` subdirectory you extracted (e.g.
   `C:\PSn00bSDK\bin` or `/opt/psn00bsdk/bin`) to your `PATH` environment
   variable, through System Properties on Windows or by modifying your profile
   script on Linux.

3. You may optionally set the `PSN00BSDK_LIBS` environment variable to point to
   the `lib/libpsn00b` subdirectory (again, the full path would be something
   like `C:\PSn00bSDK\lib\libpsn00b` or `/opt/psn00bsdk/lib/libpsn00b`). Doing
   so is highly recommended as it will save you from having to hardcode a path
   to the SDK in your projects later on.

4. You should now be able to invoke the compiler (`mipsel-none-elf-gcc`) as
   well as PSn00bSDK commands such as `elf2x` and `mkpsxiso`. If you get
   "command not found" errors try rebooting, otherwise you can skip to
   [Creating a project](#creating-a-project).

## Building from source

The instructions below are for Windows and Linux. Building on macOS hasn't been
tested extensively yet, however it should work once the GCC toolchain is built
and installed properly.

**IMPORTANT**: due to a bug in `libflac` (used by `mkpsxiso`), building using
MinGW on Windows currently requires `-DMKPSXISO_NO_LIBFLAC=ON` to be passed to
CMake when configuring PSn00bSDK. This will result in the `dumpsxiso` utility
being built without support for ripping CD audio tracks to FLAC, however the
`mkpsxiso` command will still retain FLAC support.

1. Install prerequisites and a host compiler toolchain. On Linux (most distros)
   install the following packages from your distro's package manager:

   - `git`
   - `build-essential`, `base-devel` or similar
   - `make` or `ninja-build`
   - `cmake` (3.21 or later is required, download it from
     [here](https://cmake.org/download) if your package manager only provides
     older versions)

   On Windows you can obtain these dependencies by installing
   [MSys2](https://www.msys2.org), opening the "MSys2 MSYS" shell and running:

   ```bash
   pacman -Syu git mingw-w64-x86_64-make mingw-w64-x86_64-ninja mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc
   ```

   If you are prompted to close the shell, you may have to reopen it afterwards
   and rerun the command to finish installation.
   **Do not use the MSys2 shell for the next steps**, use a regular command
   prompt or PowerShell instead.

   Add these directories (replace `C:\msys64` if you installed MSys2 to a
   different location) to the `PATH` environment variable using System
   Properties:

   - `C:\msys64\mingw64\bin`
   - `C:\msys64\usr\bin`

2. Download a precompiled copy of the GCC toolchain for `mipsel-none-elf` from
   the releases page and extract it into one of the directories listed in
   step 3. If you want to build the toolchain yourself, see
   [toolchain.md](toolchain.md).

   **NOTE**: PSn00bSDK is also compatible with toolchains that target
   `mipsel-unknown-elf`. If you already have such a toolchain, you can use it
   by passing `-DPSN00BSDK_TARGET=mipsel-unknown-elf` to CMake when configuring
   the SDK (see step 5).

3. If you chose a non-standard install location for the toolchain, add the
   `bin` subfolder (inside the top-level toolchain directory) to the `PATH`
   environment variable. This step is unnecessary if you installed/extracted
   the toolchain into any of these directories:

   - `C:\Program Files\mipsel-none-elf`
   - `C:\Program Files (x86)\mipsel-none-elf`
   - `C:\mipsel-none-elf`
   - `/usr/local/mipsel-none-elf`
   - `/usr/mipsel-none-elf`
   - `/opt/mipsel-none-elf`

4. Clone the PSn00bSDK repo, then run the following command from the PSn00bSDK
   repository to download additional dependencies:

   ```bash
   git submodule update --init --recursive
   ```

5. Compile the libraries, tools and examples using CMake:

   ```bash
   cmake --preset default .
   cmake --build ./build
   ```

   If you want to install the SDK to a custom location rather than the default
   one (`C:\Program Files\PSn00bSDK` or `/usr/local` depending on your OS), add
   `--install-prefix <location>` to the first command. Remember to add
   `-DPSN00BSDK_TARGET=mipsel-unknown-elf` if necessary.

   **NOTE**: Ninja is used by default to build the SDK. If you can't get it to
   work or don't have it installed, pass `-G "Unix Makefiles"` (or
   `-G "MinGW Makefiles"` on Windows) to the first command to build using
   `make` instead.

6. Install the SDK to the path you chose (add `sudo` or run it from a command
   prompt with admin privileges if necessary):

   ```bash
   cmake --install ./build
   ```

   This will create and populate the following subfolders in the installation
   directory:

   - `bin`
   - `lib/libpsn00b`
   - `share/psn00bsdk`

7. You may optionally set the `PSN00BSDK_LIBS` environment variable to point to
   the `lib/libpsn00b` subfolder inside the install directory. Doing so is
   highly recommended as it will save you from having to hardcode a path to the
   SDK in your projects later on. You might also want to add the `bin` folder
   to `PATH` if it's not listed already.

Although not strictly required, you'll probably want to install a PS1 emulator
with debugging capabilities such as [no$psx](https://problemkaputt.de/psx.htm)
(Windows only), [DuckStation](https://github.com/stenzek/duckstation) or
[pcsx-redux](https://github.com/grumpycoders/pcsx-redux).
**Avoid ePSXe and anything based on MAME** as they are inaccurate.

## Building installer packages

CPack can be used to build NSIS-based installers, DEB/RPM packages and zipped
releases that include built SDK libraries, headers as well as the GCC toolchain.
Distributing prebuilt releases is however discouraged since PSn00bSDK is still
far from being feature-complete.

1. Follow steps 1-4 above to set up the toolchain, then install
   [NSIS](https://nsis.sourceforge.io/Download) on Windows or `dpkg` and `rpm`
   on Linux.

2. Run the following commands from the PSn00bSDK directory (pass the
   appropriate options to the first command if necessary):

   ```bash
   cmake --preset package .
   cmake --build ./build -t package
   ```

   All built packages will be copied to the `build/packages` folder.

## Creating a project

1. Copy the contents of `share/psn00bsdk/template` within the PSn00bSDK
   installation directory (or the `template` folder within the repo) to a new
   empty folder, which will become your project's root directory.

2. If you haven't set the `PSN00BSDK_LIBS` environment variable previously or
   if you want to use a different PSn00bSDK installation for the project, edit
   `CMakePresets.json` to set the path you installed the SDK to. See the
   [setup guide](cmake_reference.md#setup) for details.

3. Configure and build the template by running:

   ```bash
   cmake --preset default .
   cmake --build ./build
   ```

   If you did everything correctly there should be a `template.bin` CD image in
   the `build` folder. Test it in an emulator to ensure it works.

The toolchain script defines a few CMake macros to create PS1 executables, DLLs
and CD images. See the [reference](cmake_reference.md) for details.

-----------------------------------------
_Last updated on 2022-10-11 by spicyjpeg_
