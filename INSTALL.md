
# Getting started with PSn00bSDK

## Building and installing

The instructions below are for Windows and Linux. Building on macOS hasn't been
tested but should work.

1. Install a host C compiler toolchain if you don't already have one (this is
   required to build the tools). You can use MSVC or MinGW on Windows, or
   install the `build-essential` package provided by most Linux distros.

2. Install Git and CMake. Note that some Linux distros ship relatively old
   versions of CMake, so make sure you have at least CMake 3.21. You will also
   need [Ninja](https://ninja-build.org) (it is a single executable, you have to
   copy it to any directory listed in the `PATH` environment variable) on
   Windows as there is no preinstalled build system; on Linux you can use `make`
   instead, but Ninja is still recommended.

3. Build and install a GCC toolchain for `mipsel-unknown-elf`. As GCC is
   notoriously hard to compile under Windows, you may download a precompiled
   version from [Lameguy64's website](http://lameguy64.net?page=psn00bsdk) and
   and extract it into Program Files instead. See [toolchain.txt](toolchain.txt)
   for details on compiling GCC.

4. If you chose a non-standard install location for the toolchain, set the
   `PSN00BSDK_TC` environment variable to point to the toolchain's root
   directory. This step is unnecessary if you installed/extracted the toolchain
   into any of these directories:

   - `C:\Program Files\mipsel-unknown-elf`
   - `C:\Program Files (x86)\mipsel-unknown-elf`
   - `C:\mipsel-unknown-elf`
   - `/usr/local/mipsel-unknown-elf`
   - `/usr/mipsel-unknown-elf`
   - `/opt/mipsel-unknown-elf`

5. Clone/download the PSn00bSDK repo and run the following commands:

   ```bash
   cmake -S . -B ./build -G Ninja --install-prefix INSTALL_PATH
   cmake --build ./build
   cmake --install ./build
   ```

   Replace `INSTALL_PATH` with the directory you want PSn00bSDK to be installed
   to (default is `C:\Program Files\PSn00bSDK` or `/usr/local`), and remove
   `-G Ninja` if you want to use `make` instead (not recommended). The following
   subdirectories will be created:

   - `INSTALL_PATH/bin`
   - `INSTALL_PATH/lib/libpsn00b`
   - `INSTALL_PATH/share/psn00bsdk`

6. Set the `PSN00BSDK_LIBS` environment variable to
   `INSTALL_PATH/lib/libpsn00b` and add `INSTALL_PATH/bin` to `PATH`.

Although not strictly required, you'll probably want to install a PS1 emulator
with debugging capabilities such as [no$psx](https://problemkaputt.de/psx.htm)
(Windows only) or [pcsx-redux](https://github.com/grumpycoders/pcsx-redux).
**Avoid ePSXe and anything based on MAME** as they are inaccurate.

## Building installer packages

CPack can be used to build NSIS-based installers, DEB/RPM packages and zipped
releases. Note that currently none of the built packages include the toolchain,
thus their usefulness is limited. Distributing prebuilt releases is discouraged
anyway since PSn00bSDK is still far from being feature-complete.

1. Follow steps 1-4 above to set up the toolchain, then install NSIS on Windows
   or `dpkg` and `rpm` on Linux.

2. Run the following commands from the PSn00bSDK directory:

   ```bash
   cmake -S . -B ./build -G Ninja
   cmake --build ./build -t package
   ```

   All built packages will be copied to the `build/cpack` folder.

## Creating a project

1. Copy the contents of `INSTALL_PATH/share/psn00bsdk/template` (or the
   `template` folder within the repo) to your new project's root directory.

2. Configure and build the template by running:

   ```bash
   cmake -S . -B ./build -G Ninja
   cmake --build ./build
   cmake --install ./build
   ```

   If you did everything correctly there should be a `template.bin` CD image in
   the `build` folder. Test it in an emulator to ensure it works.

Note that, even though the template relies on the `PSN00BSDK_LIBS` environment
variable to locate the SDK by default, you can also specify the path directly
on the CMake command line by adding
`-DCMAKE_TOOLCHAIN_FILE=INSTALL_PATH/lib/libpsn00b/cmake/sdk.cmake` (replace
`INSTALL_PATH` as usual) to the first command.

The toolchain script defines a few CMake macros to create PS1 executables, DLLs
and CD images. See the [reference](doc/cmake_reference.md) for details.

-----------------------------------------
_Last updated on 2021-09-26 by spicyjpeg_
