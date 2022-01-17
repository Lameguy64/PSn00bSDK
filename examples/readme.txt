PSn00bSDK Example Programs
2019 - 2022 Meido-Tek Productions / PSn00bSDK Project

## Building the examples ##

The instructions below assumes that a mipsel-unknown-elf or mipsel-none-elf GNU
toolchain is installed, CMake and Ninja or make is installed and the PSn00bSDK
libraries and tools are compiled and installed. CMake version must be at least
version 3.20 or newer.

1. If the examples are in /usr/local/share, copy the directory into your home
   directory.
   
2. Configure the examples with CMake by running:

   cmake -S . -B ./build -DCMAKE_TOOLCHAIN_FILE=<path to sdk.cmake>

   <path to sdk.cmake> must point to the sdk.cmake file provided by the SDK.
   Unless you've installed the SDK with a custom path, normally this file is
   located in /usr/local/lib/libpsn00b/cmake on *nix style systems or
   C:\Program Files\PSn00bSDK\lib\libpsn00b\cmake in Windows.
   
   If the mipsel toolchain has a different prefix (ie. mipsel-none-elf), specify
   -DPSN00BSDK_TARGET=<prefix> to override the default toolchain prefix.
   
   If Ninja does not work for you or don't have it installed, pass 
   -G "Unix Makefiles" (or -G "MSys Makefiles" on Windows) to build using make
   instead.
   
3. Build the example programs by running:

   cmake --build ./build
   
   This should create a build directory with a directory structure that mirrors
   the parent directory. The directories should contain compiled versions of the
   example programs as a PS-EXE or ISO file.
   

## Examples summary ##

The following list is a brief summary of all the example programs included.
Additional information may be found in the source code of each example.

  beginner/cppdemo		Simple demonstration of (dynamic) C++ classes
  beginner/hello		The obligatory "Hello World" example program
  cdrom/cdbrowse		File browser using libpsxcd's directory functions
  cdrom/cdxa			Plays CD-XA audio (XA audio not included)
  demos/n00bdemo		The premiere demonstration program of PSn00bSDK
  graphics/balls		Draws colored balls bouncing around the screen
  graphics/billboard	Demonstrates how to draw 2D sprites in a 3D space
  graphics/fpscam		First-person perspective camera with look-at
  graphics/gte			Displays a rotating cube using GTE macros
  graphics/hdtv			Demonstrates anamorphic widescreen at 704x480
  graphics/render2tex	Procedural texture effects using off-screen drawing
  graphics/rgb24		Displays a 640x480 24-bit RGB image
  graphics/tilesasm		Drawing a tile-map with assembly language
  io/pads				Demonstrates reading controllers via low-level access
  lowlevel/cartrom		ROM firmware for cheat devices written using GNU GAS
  sound/vagsample		Demonstrates playing VAG sound files with the SPU
  system/childexec		Loading a child program and returning to parent
  system/console		TTY based text console that interrupts gameplay
  system/dynlink		Demonstrates dynamically linked libraries
  system/timer			Demonstrates using hardware timers with interrupts
  system/tty			Using TTY as a remote text console interface