
# Building the GCC toolchain

If you wish to build the toolchain yourself, beware that this process can get
pretty tedious if your machine is not fairly recent. Ensure you have at least a
quad-core processor and 4 GB of free space before continuing.

You'll need a Linux environment, even if you want to build a Windows toolchain
(as GCC is basically impossible to build under Windows but can be cross-compiled
via MinGW). Due to how the GCC build process works, you'll have to build a Linux
version of the toolchain first to be able to compile it for Windows. This
basically means you will have to build the whole toolchain twice if you want to
target Windows.

These instructions are for Debian/Ubuntu, however it should be relatively easy
to follow them if you are using another distro. If you do not have access to a
Linux system already, consider spinning up a VM (a headless Debian or Ubuntu
Server install is recommended) or using WSL, whose setup is out of the scope of
this guide.

## Choosing a GCC version

PSn00bSDK *should* work with any GCC version. In most cases you'll want to get
the latest stable release of GCC and binutils. If for some reason you are having
problems you may try building one of the following versions, which have been
tested extensively:

- ~~GCC 7.4.0 with binutils 2.31~~ (the linker fails to build PS1 DLLs)
- GCC **11.1.0** with binutils **2.36**
- GCC **11.2.0** with binutils **2.37**
- GCC **12.2.0** with binutils **2.40**

If you wish to pick an older GCC release but don't know which binutils version
it requires, see [here](https://wiki.osdev.org/Cross-Compiler_Successful_Builds)
for a compatibility table.

## Downloading GCC

1. Run the following commands to install a host toolchain and prerequisites
   (adapt them for non-Debian distros if necessary):

   ```bash
   sudo apt update
   sudo apt install -y build-essential make wget
   ```

2. Create an empty directory to store build artifacts in. You'll be able to
   delete it once the toolchain is installed.

3. Download the GCC and binutils source packages from
   [here](https://ftpmirror.gnu.org/gnu) and unzip them into the folder you
   created, or run the following commands to do the same (replace `<VERSION>`
   with the versions you chose):

   ```bash
   wget https://ftpmirror.gnu.org/gnu/binutils/binutils-<VERSION>.tar.xz
   wget https://ftpmirror.gnu.org/gnu/gcc/gcc-<VERSION>/gcc-<VERSION>.tar.xz
   tar xvf binutils-<VERSION>.tar.xz
   tar xvf gcc-<VERSION>.tar.xz
   rm -f *.tar.xz
   ```

4. From the extracted GCC directory run the `download_prerequisites` script to
   download additional dependencies:

   ```bash
   cd gcc-<VERSION>
   ./contrib/download_prerequisites
   ```

## Building binutils

1. Go back to the folder you made earlier and create a new subdirectory to build
   binutils in (don't create it inside the extracted binutils source directory).
   Call it `binutils-build` or whatever.

2. Run the binutils configuration script from that folder:

   ```bash
   ../binutils-<VERSION>/configure \
     --prefix=/usr/local/mipsel-none-elf --target=mipsel-none-elf \
     --disable-docs --disable-nls --disable-werror --with-float=soft
   ```

   Replace `<VERSION>` as usual. If you don't want to install the toolchain into
   `/usr/local/mipsel-none-elf` you can change the `--prefix` option.

3. Compile and install binutils (this will take a few minutes to finish):

   ```bash
   make -j 4
   sudo make install-strip
   ```

   Increase `-j 4` to speed up the build if your machine or VM has more than 4
   CPU cores.

   **NOTE**: if the build fails with a "`uint` undeclared" or similar error, try
   editing the source file that caused the error and pasting this line at the
   beginning:

   ```c
   typedef unsigned int uint;
   ```

   Rerun `make` to resume the build after saving the file.

## Building GCC

The process is mostly the same as binutils, just with different configuration
options.

1. Go back to the main directory and create an empty `gcc-build` (or whatever)
   subfolder.

2. Run the GCC configuration script from there:

   ```bash
   ../gcc-<VERSION>/configure \
     --prefix=/usr/local/mipsel-none-elf --target=mipsel-none-elf \
     --disable-docs --disable-nls --disable-werror --disable-libada \
     --disable-libssp --disable-libquadmath --disable-threads \
     --disable-libgomp --disable-libstdcxx-pch --disable-hosted-libstdcxx \
     --enable-languages=c,c++ --without-isl --without-headers \
     --with-float=soft --with-gnu-as --with-gnu-ld
   ```

   If you previously set a custom installation path, remember to set it here as
   well (it must be the same).

3. Compile and install GCC (will take a long time, usually around half an hour):

   ```bash
   make -j 4
   sudo make install-strip
   ```

   Increase `-j 4` to speed up the build if your machine or VM has more than 4
   threads.

4. Add the toolchain to the `PATH` environment variable. This is required to
   rebuild the toolchain for Windows (see below), but it will also allow
   PSn00bSDK to find the toolchain if you installed it in a custom location.

   Edit the `.bashrc` or `.bash_profile` file in your home directory and add
   this line at the end (replace the path with the install location you chose
   earlier, but keep the `/bin` at the end):

   ```bash
   export PATH=$PATH:/usr/local/mipsel-none-elf/bin
   ```

   Restart the shell by closing and reopening the terminal window or SSH
   connection afterwards.

## Rebuilding for Windows

At this point you should be able to build and install PSn00bSDK on your Linux
system. The instructions below are for building a second copy of the toolchain
that runs on Windows.

1. Install the MinGW host toolchain:

   ```bash
   sudo apt install -y g++-mingw-w64-x86-64
   ```

2. Create two new `binutils-win` and `gcc-win` folders in the directory you
   extracted/built everything in.

3. From the `binutils-win` directory, rerun the binutils configuration script
   with the following options (do not change the installation path):

   ```bash
   ../binutils-<VERSION>/configure \
     --build=x86_64-linux-gnu --host=x86_64-w64-mingw32 \
     --prefix=/tmp/mipsel-none-elf --target=mipsel-none-elf \
     --disable-docs --disable-nls --disable-werror --with-float=soft
   ```

   Then build binutils again:

   ```bash
   make -j 4
   make install-strip
   ```

4. Do the same for GCC from the `gcc-win` directory:

   ```bash
   ../gcc-<VERSION>/configure \
     --build=x86_64-linux-gnu --host=x86_64-w64-mingw32 \
     --prefix=/usr/local/mipsel-none-elf --target=mipsel-none-elf \
     --disable-docs --disable-nls --disable-werror --disable-libada \
     --disable-libssp --disable-libquadmath --disable-threads \
     --disable-libgomp --disable-libstdcxx-pch --disable-hosted-libstdcxx \
     --enable-languages=c,c++ --without-isl --without-headers \
     --with-float=soft --with-gnu-as --with-gnu-ld
   ```

   And build it as usual:

   ```bash
   make -j 4
   make install-strip
   ```

5. Copy the entire `/tmp/mipsel-none-elf` directory over to your Windows
   machine using VM shared folders, a network share, `scp` or whichever method
   you prefer. It's recommended to put the toolchain in
   `C:\Program Files\mipsel-none-elf` or `C:\mipsel-none-elf`.

6. If you want to keep the toolchain in another location and/or use it from the
   command line, add the `bin` subdirectory inside `mipsel-none-elf` to the
   `PATH` environment variable (as you did on Linux) using System Properties.

## Note regarding C++ support

C++ support in PSn00bSDK is limited to the freestanding subset of the standard
library provided by GCC, which includes most metaprogramming and compile-time
utilities but not higher level functionality that requires runtime support such
as exceptions, containers or streams. Basic C++ features that only depend on the
compiler (classes, templates, `constexpr` and so on) are fully supported.

Implementing a full STL, while technically possible, is currently out of the
scope of PSn00bSDK. There are other PS1 SDKs that provide an STL, such as
[psyqo](https://github.com/grumpycoders/pcsx-redux/tree/main/src/mips/psyqo),
and they might be a better fit for your project if you plan to make heavy use of
C++ features.

-----------------------------------------
_Last updated on 2023-04-05 by spicyjpeg_
