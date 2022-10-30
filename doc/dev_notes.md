
# Development notes

These are some development notes I've put together that would be of great aid
to those willing to contribute to the PSn00bSDK project. Many of these came
from my own experience dealing with the PS1 at low-level when I ran into some
unexplained quirks while some are from disassembly observations and
clarification of existing documents. More entries will be added when I run into
more previously undocumented quirks in the future. _- Lameguy64_

Porting PSn00bSDK to CMake also uncovered a lot of bugs and undocumented
behavior in CMake itself. I documented [below](#cmake) all issues I ran into.
_- spicyjpeg_

## MIPS ABI / compiler

- When calling C functions (including BIOS functions) from assembly code you'll
  need to allocate one 32-bit word on the stack for each argument the function
  takes (`addiu $sp, -(4*N)` where `N` = number of arguments of the function
  being called), *even if the function takes its arguments from registers*
  *`$a0`-`$a3` rather than from the stack*. When calling a variadic function
  (i.e. one with a variable number of arguments, such as `printf`) always
  allocate 16 bytes of stack.

- For some reason `mipsel-unknown-elf-nm` and `mipsel-none-elf-nm` (symbol map
  generators) insist on outputting 64-bit addresses (with the top 32 bits set,
  e.g. `FFFFFFFF80010000`) even when feeding them a regular 32-bit MIPS
  executable, while the standard (x86) `nm` tool that ships with most GCC
  packages prints the proper 32-bit address. Unclear whether this is a bug,
  intended behavior or the result of some ancient ELF ABI flag crap.
  `DL_ParseSymbolMap()` will ignore the top 32 bits, so this should only bother
  you if you're implementing your own symbol map parser.

- If you are overriding any of the memory allocation functions,
  **DO NOT ENABLE LINK-TIME OPTIMIZATION**. GCC has a long-standing bug with
  LTO and weak functions written in assembly, also LTO hasn't been tested at
  all yet.

## BIOS and interrupts

- Hooking a custom handler using BIOS function `HookEntryInt` (`B(19h)`, known
  as `SetCustomExitFromException` in nocash docs) is only triggered when
  there's an IRQ that is not yet acknowledged by previous IRQ handlers built
  into the kernel. This is also the best point to acknowledge any IRQs without
  breaking compatibility with built-in BIOS IRQ handlers and is what the
  official SDK uses to handle IRQs. To make sure this handler is triggered on
  every interrupt you must call `ChangeClearPad(0)` and `ChangeClearRCnt(3, 0)`
  (which are functions `B(5Bh)` and `C(0Ah)` respectively) otherwise the pad
  and root counter handlers in the kernel will acknowledge the interrupt before
  your handler, preventing you from handling them yourself.

- It is not advisable to handle interrupts using event handlers like in PSXSDK
  as it breaks BIOS features that depend on interrupts. Clearing the VBlank IRQ
  in a event handler for example prevents the BIOS controller functions from
  working as it depends on the VBlank IRQ to determine when to query
  controllers. Acknowledge interrupts using a custom handler set by BIOS
  function `HookEntryInt` (`B(19h)`, known as `SetCustomExitFromException` in
  nocash docs).

- In the official SDK, DMA IRQs appear to be enabled only when a callback
  function is set (ie. `DrawSyncCallback()` enables IRQ for DMA channel 2). DMA
  IRQs are only triggered on transfer completion.

- PSn00bSDK provides no support yet for replacing the BIOS exception handler
  with a custom one, however it can be done (if you are ok with losing all BIOS
  controller, memory card and file functionality). In order not to break
  anything your exception handler must do the following:

  - prevent GTE opcodes from being executed twice due to a hardware glitch (the
    nocash docs explain how to do this);
  - define `_irq_func_table[12]` as an *extern* array of function pointers,
    call the appropriate entry when an IRQ occurs and clear the respective flag
    in register `1F801070h`;
  - handle syscalls `01h`-`02h`, i.e. `EnterCriticalSection` and
    `ExitCriticalSection`, properly. You should also handle syscall `FF00h`
    (invalid API usage), as well as breaks `1800h` and `1C00h` (division errors
    injected by GCC), by locking up and maybe showing a BSOD or similar;
  - overwrite the default BIOS API vectors with a passthrough that checks no
    controller- or interrupt-related function is being called. This is necessary
    (although ugly) as `libpsn00b` often calls such functions internally.

## Hardware

- When running in high resolution mode you must additionally wait for bit 31 in
  `GPUSTAT` (`1F801814h`) to alternate on every frame (frame 0: wait until 0,
  frame 1: wait until 1, frame 2: wait until 0) before waiting for VSync
  otherwise the GPU will only draw the first field if you don't have drawing to
  displayed area enabled. Performing this wait operation in non-interlaced modes
  is harmless.

- There's a hardware bug in the GPU `FillVRAM` command `GP0(02h)` where if you
  set the height to 512 pixels the primitive is drawn with a height of 0 as
  the hardware does not appear to interpret the last bit of the height field.
  This is most apparent when applying a DRAWENV with the height of 512 pixels
  (for PAL standard for example) and isbg is set, hence this method also does
  not work in the official SDK either.

- The controller/memory card SPI interface is poorly implemented in most
  emulators, making custom controller polling code insanely hard to write and
  debug. The only emulator that comes close to real hardware is no$psx, which
  seems to correctly implement all features of the SPI port (even those not
  used by the BIOS pad driver, such as TX/RX interrupts). DuckStation only
  emulates the bare minimum required by the BIOS and Sony libraries, and
  pcsx-redux has major bugs that break most custom polling implementations.
  This pretty much means TX/RX IRQs and many flags in the `JOY_*` registers
  should **not** be used unless you are willing to break compatibility with
  emulators.

- As if communicating with controllers wasn't difficult enough already,
  DualShock pads also have a built-in watchdog timer that gets enabled when
  first putting them in configuration mode (and is **not** disabled after
  exiting config mode). If no polling commands are sent to the controller for
  about 1 second, vibration motors are switched off and analog mode is
  disabled; the same happens if the analog button is pressed while in analog
  mode. In order to always keep the pad in analog mode you must:

  1. Poll both controller ports at least once per frame by sending command
     `42h`. Polling at a higher rate might be desirable in some cases (such as
     rhythm games) to increase timing accuracy.
  2. If a digital pad response (type = 4) is received from a port that hasn't
     previously been flagged as digital-only, attempt to put the pad into
     config mode using command `43h` *twice* (as the proper response is
     delayed).
     - If the pad doesn't recognize the config command, flag it as digital-only
       and treat all further digital pad responses from it as valid.
     - If the pad recognizes the command, it will reply by identifying as an
       analog pad in config mode (type = 15). Send command `44h` immediately
       (without sending `42h` first, otherwise the pad will exit config mode)
       to enable analog mode and turn on the LED.
     - Pressing the analog button will result in the controller identifying as
       digital even though it is not flagged as such, thus re-triggering the
       configuration process and putting it back into analog mode.
  3. All analog pad responses (type = 7) can be treated as valid, as they will
     come from controllers that have already been configured.
  4. If no valid response is received, assume no controller is connected and
     reset the port's digital-only flag.

- The SPU *really* doesn't like 32-bit register writes. It is connected to the
  CPU through a 16-bit bus; 32-bit writes are automatically split into two
  transactions, however the SPU has a tendency to miss one of them (perhaps due
  to the bus controller issuing them too quickly). This might be why nocash
  docs claim that writing to the SPU is unstable when in actual fact 16-bit
  writes seem to be perfectly stable.

- This is the formula to calculate SPU pitch values for playing musical notes
  (`^` is the power operator, not xor):

  ```
  frequency = (ref / 32) * (2 ^ ((note - 9) / 12))
  spu_pitch = (frequency * 4096) / 44100

  ref  = frequency the sample should be played at to play a middle A (MIDI note 69)
  note = MIDI note number (usually 0-127, 60 is middle C)
  ```

## CMake

- Toolchain files are loaded "early" according to the CMake docs. What this
  means in practice is that a lot of commands, such as `find_*()`, won't work
  properly in a toolchain script as they rely on variables initialized by the
  `project()` command. The poorly documented solution to this is to move such
  commands to a separate file and set `CMAKE_PROJECT_INCLUDE` to point to it,
  so `project()` will execute it immediately after initialization.

- After executing the toolchain file, CMake generates and attempts to build
  several dummy projects to test the compiler. Each of these projects
  re-includes the toolchain script (which is why you'll see commands executed
  multiple times) and uses the same variable values as the main project,
  however CMake will *not* pass custom variables through by default. If your
  toolchain script has options that can be set via custom variables (like
  `PSN00BSDK_TC` and `PSN00BSDK_TARGET` in PSn00bSDK), you'll have to set
  `CMAKE_TRY_COMPILE_PLATFORM_VARIABLES` to a list of variable names to be
  exported to generated dummy projects. Additionally you may need to set
  `CMAKE_TRY_COMPILE_TARGET_TYPE` to `STATIC_LIBRARY` to prevent CMake from
  invoking the linker.

- When `project()` is called, CMake uses the value of `CMAKE_SYSTEM_NAME` to
  determine default values for several variables and properties. Most of these
  defaults are undocumented: e.g. setting the system name to `Generic` (as
  suggested by the docs for bare metal targets) will result in CMake assuming
  that the target platform does not support dynamic linking, so you'll have to
  turn it back on by setting the `TARGET_SUPPORTS_SHARED_LIBS` global property
  *after* invoking `project()`.

- It is not possible to use custom toolchains (through toolchain files or by
  setting `CMAKE_*_COMPILER` manually) with any of the Xcode or VS generators,
  as their respective build systems handle compiler selection internally rather
  than relying on variables passed by CMake. Ninja or `make` is thus always
  required to build PSn00bSDK and any projects made with it, even if the
  host-side tools are built using Xcode or MSVC.

- There is no way to use multiple toolchains (PS1 + host) in a single project,
  even if you use `add_subdirectory()` to execute multiple project files
  (which, confusingly, adds their targets to the parent project rather than
  treating them as separate projects). Thankfully though CMake provides support
  for automating the build process of independent CMake projects via the
  `ExternalProject` module. Which brings me to the next issue...

- If you run CPack on a "superbuild" project (i.e. a project that calls
  `ExternalProject_Add()` to configure, compile and install subprojects at
  build time), you'll likely run into a weird issue with CPack bundling folders
  from your build directory into DEB and RPM packages. This is caused by the
  DEB/RPM generators running `cmake --install` in a chroot/fakeroot to prepare
  the files to be packaged, which seems to interfere with absolute paths in the
  project cache or something like that (?). The only workaround I know of is to
  use `CPACK_PRE_BUILD_SCRIPTS` to trigger a custom script that deletes
  anything other than the actual files to be packaged (see
  `cpack/fakeroot_fix.cmake`).

- Project installation might fail on macOS (and possibly some Linux distros) if
  CMake attempts to set permissions on system directories such as `/usr/local`.
  This is usually caused by `install()` commands that copy files to the root
  installation directory rather than to a subfolder, like this:

  ```cmake
  install(
    DIRECTORY   install_tree/
    DESTINATION .
    USE_SOURCE_PERMISSIONS
  )
  ```

  If the `USE_SOURCE_PERMISSIONS` flag is specified CMake will attempt to set
  permissions on the `DESTINATION` folder, which in this case would be the root
  prefix (`/usr/local` by default on macOS), to match the source directory.
  This will however fail as macOS restricts top-level system directories from
  having their permissions changed. The simplest workaround is to avoid using
  `DESTINATION .` and install each subdirectory explicitly instead, like this:

  ```cmake
  foreach(
    _dir IN ITEMS
      ${CMAKE_INSTALL_BINDIR}
      ${CMAKE_INSTALL_LIBDIR}
      ${CMAKE_INSTALL_INCLUDEDIR}
      ${CMAKE_INSTALL_DATADIR}
  )
    install(
      DIRECTORY   install_tree/${_dir}/
      DESTINATION ${_dir}
      USE_SOURCE_PERMISSIONS
    )
  endforeach()
  ```

  CMake will scan the executable at install time and copy all the required DLLs
  that match the second regex. If no regex is specified CMake will also copy OS
  DLLs like `libc` or `msvcrt`, which usually isn't the desired behavior.

- Using interface targets to set include directories can be finicky. Not only
  do you have to use generator expressions to conditionally use different paths
  depending on whether the targets are installed, but CMake can get confused on
  which options to pass to the compiler. I spent hours trying to get CMake to
  use `-I` rather than `-isystem` for include directories (for some reason GCC
  would ignore `-isystem` completely). I eventually gave up and just set the
  include directories manually for each target, and for some reason CMake
  actually started passing `-I` instead of `-isystem` to GCC.

- When using CPack with NSIS, all `CPACK_NSIS_*` variables are passed to NSIS
  verbatim, i.e. without the usual slash-to-backslash path conversion that
  CMake does on Windows. Most Windows programs accept paths with slashes
  without issue, unfortunately the NSIS builder is not one of those. To add
  insult to injury, CMake doesn't even escape backslashes by default when
  quoting strings in the generated CPack config file! So you have to convert
  the paths manually *and* tell CMake to enable escaping by setting
  `CPACK_VERBATIM_VARIABLES`, like this:

  ```cmake
  set(CPACK_VERBATIM_VARIABLES ON)
  foreach(
    _var IN ITEMS
      CPACK_NSIS_MUI_ICON
      CPACK_NSIS_MUI_UNIICON
      CPACK_NSIS_MUI_HEADERIMAGE
      CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP
      CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP
  )
    cmake_path(NATIVE_PATH ${_var} ${_var})
  endforeach()
  ```

- Not a CMake/CPack bug per se, but NSIS is picky about the banner and header
  images shown in generated installers. They must be Windows BMP version 3
  files with no alpha channel, no compression and no metadata. They can either
  be 24-bit RGB or indexed, though it's common to use indexed colors to save
  space.

-----------------------------------------
_Last updated on 2022-10-30 by spicyjpeg_
