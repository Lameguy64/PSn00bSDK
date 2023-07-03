
# PSn00bSDK CMake reference

- [Setup](#setup)
- [Targets](#targets)
- [Commands](#commands)
- [Target properties](#target-properties)
- [Preprocessor definitions](#preprocessor-definitions)
- [Cached settings](#cached-settings)
- [Internal settings](#internal-settings)
- [Read-only variables](#read-only-variables)

## Setup

The only requirement to use the SDK in CMake is to set the
`CMAKE_TOOLCHAIN_FILE` variable to the absolute path to
`lib/libpsn00b/cmake/sdk.cmake` within the PSn00bSDK installation directory.
This can be done on the command line (`-DCMAKE_TOOLCHAIN_FILE=...`), in
`CMakeLists.txt` (`set(CMAKE_TOOLCHAIN_FILE ...)` before `project()`) or using
[presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html).

It's suggested to have a default preset that sets the toolchain file to
`$env{PSN00BSDK_LIBS}/cmake/sdk.cmake`, taking advantage of the
`PSN00BSDK_LIBS` environment variable (used by former PSn00bSDK versions) to
automatically find the SDK if set. Such a preset can be created by placing a
`CMakePresets.json` file in the project's root with the following contents:

```json
{
  "version": 3,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 21,
    "patch": 0
  },
  "configurePresets": [
    {
      "name":          "default",
      "displayName":   "Default configuration",
      "description":   "Use this preset to build the project using PSn00bSDK.",
      "generator":     "Ninja",
      "toolchainFile": "$env{PSN00BSDK_LIBS}/cmake/sdk.cmake",
      "binaryDir":     "${sourceDir}/build",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "PSN00BSDK_TC":     "",
        "PSN00BSDK_TARGET": "mipsel-none-elf"
      },
      "warnings": {
        "dev": false
      }
    }
  ]
}
```

To avoid having to pass variables to CMake each time the project is built, a
second file named `CMakeUserPresets.json` can be created and populated with
hardcoded values in the `cacheVariables` section. This file can be kept private
(e.g. by adding it to `.gitignore`), and CMake will automatically load presets
from it instead of `CMakePresets.json` if it exists.

See the [template](../template/CMakeLists.txt) for an example CMake script
showing how to build a simple project.

## Targets

The toolchain script creates a target for each PSn00bSDK library. Currently
the following targets are defined:

- `psxgpu`
- `psxgte`
- `psxspu`
- `psxcd`
- `psxpress`
- `psxsio`
- `psxetc`
- `psxapi`
- `smd`
- `lzp`
- `c`

Note that these are not actual libraries but virtual targets that link to the
appropriate version of the respective library, depending on the value of the
`PSN00BSDK_TARGET_TYPE` property; refer to the target properties section for
more information. Linking manually using the `target_link_libraries()` command
is usually not necessary for executables as they are linked to all libraries
by default (see `PSN00BSDK_EXECUTABLE_LINK_LIBRARIES`).

Additionally, two "hidden" libraries named `gcc` and `psn00bsdk` are linked by
default to all targets. The former is the GCC toolchain's `libgcc`, while the
latter is a virtual target used to set compiler flags and paths.

## Commands

### `psn00bsdk_add_executable`

```cmake
psn00bsdk_add_executable(
  <target name> <GPREL|STATIC|NOGPREL|DYNAMIC>
  [EXCLUDE_FROM_ALL]
  [sources...]
)
```

A wrapper around `add_executable()` to create PS1 executables. Three files will
be generated for each call to this function:

- `<target name>.elf` (regular ELF executable)
- `<target name>.exe` (executable converted to the format expected by the PS1)
- `<target name>.map` (symbol map file for dynamic linking/introspection)

The `.exe` and `.map` extensions can be customized by overriding
`PSN00BSDK_EXECUTABLE_SUFFIX` and `PSN00BSDK_SYMBOL_MAP_SUFFIX` prior to
creating the executable.

The second argument (mandatory) specifies whether the executable is going to
load DLLs at runtime. If set to `GPREL` or `STATIC`, $gp-relative addressing
(i.e. reusing the $gp register normally used for DLL addressing to reference
global variables) will be enabled, slightly reducing executable size and RAM
usage but breaking compatibility with the dynamic linker.

All executables are automatically linked to the libraries listed in
`PSN00BSDK_EXECUTABLE_LINK_LIBRARIES` (all SDK libraries by default). This
variable can be modified prior to creating the executable to select which
libraries to link.

### `psn00bsdk_add_library`

```cmake
psn00bsdk_add_library(
  <target name> <STATIC|OBJECT|SHARED|MODULE>
  [EXCLUDE_FROM_ALL]
  [sources...]
)
```

Wraps `add_library()` to create static libraries or dynamically-linked
libraries (DLLs).

The second argument (mandatory, unlike CMake's regular `add_library()`)
specifies the type of library to create. `STATIC` will create a static library
named `lib<target name>.a`. `SHARED` and `MODULE` will compile a DLL, producing
the following files (there is no `lib` prefix for DLLs):

- `<target name>.so` (regular ELF shared library)
- `<target name>.dll` (raw binary with some ELF headers prepended)

The `.dll` extension can be customized by setting
`PSN00BSDK_SHARED_LIBRARY_SUFFIX` prior to creating the DLL.

All DLLs are automatically linked to the libraries listed in
`PSN00BSDK_SHARED_LIBRARY_LINK_LIBRARIES` (none by default). This variable can
be modified prior to creating the DLL to select which libraries to link.

**IMPORTANT**: when adding a static library using this command (or CMake's
`add_library()`), the `PSN00BSDK_TARGET_TYPE` property **must** be set on it
afterwards in order to let CMake know whether the static library is going to be
linked to an executable or a DLL. See `PSN00BSDK_TARGET_TYPE` for more
information.

### `psn00bsdk_add_cd_image`

```cmake
psn00bsdk_add_cd_image(
  <target name>
  <image name>
  <path to XML config file>
  [DEPENDS <targets|files...>]
  [other options...]
)
```

Creates a new virtual target that will build a CD image using `mkpsxiso`. The
CD image will be added to the top-level target and rebuilt automatically if any
of its dependencies have been modified since the last build.

The first argument is the name of the target to create and associate with the
CD image. This target does not actually build the image but depends on it,
ensuring CMake will build it if necessary. Note that the target, along with any
other targets depending on it, are always considered out-of-date by CMake
*even if the CD image is actually up-to-date*.

The second argument specifies the name of the generated image files
(`<image name>.bin` + `<image name>.cue`) and is followed to the path to the
XML file (relative to the source directory) to be passed to `mkpsxiso`. Note
that the `image_name` and `cue_sheet` fields specified in the `<iso_project>`
root tag (see `mkpsxiso` documentation) are ignored and overridden by the image
name provided.

The XML file is "configured" by CMake, i.e. any `${var}` or `@var@` expressions
are replaced with the values of the respective CMake variables. Paths to source
files are interpreted as relative to the build directory. In order to include a
file from the source directory, `${PROJECT_SOURCE_DIR}` shall be prepended to
the path specified in the XML file (e.g. `${PROJECT_SOURCE_DIR}/system.cnf`).

Any additional argument is passed through to the underlying call to
`add_custom_command()`, so most of the options supported by
`add_custom_command()` (including `DEPENDS`) are also supported here.

### `psn00bsdk_target_incbin`

```cmake
psn00bsdk_target_incbin(
  <target name> <PRIVATE|PUBLIC|INTERFACE>
  <data symbol name>
  <path to binary file>
)
```

Embeds the contents of a binary file into an executable or a library.

A new symbol/object will be created with the given name, escaped by replacing
non-alphanumeric characters with underscores. The contents of the file will be
aligned to 4 bytes and placed in the `.data` section. An unsigned 32-bit
integer named `<symbol name>_size` will also be defined and set to the length
of the file in bytes (without taking alignment/padding into account).

Once added the file and its size can be accessed by C/C++ code by declaring the
respective symbols as an extern array and as an integer, like this:

```c
extern const uint8_t my_file[];
extern const size_t  my_file_size;
```

The fourth argument specifies the path to the binary file relative to the
source directory. This path can be prepended with `${PROJECT_BINARY_DIR}/` to
reference a file generated by the build script (such as an LZP archive): in
that case a file-level dependency will also be created, ensuring CMake does not
attempt to compile the executable or library before the file is built.

**IMPORTANT**: in order for this command to work, assembly language support
must be enabled by specifying `LANGUAGES C ASM` (or `LANGUAGES C CXX ASM` to
enable C++ support as well) when invoking `project()`.

### `psn00bsdk_target_incbin_a`

```cmake
psn00bsdk_target_incbin_a(
  <target name> <PRIVATE|PUBLIC|INTERFACE>
  <data symbol name>
  <size symbol name>
  <path to binary file>
  <section name>
  <alignment>
)
```

Advanced variant of `psn00bsdk_target_incbin()` that allows specifying a custom
name for the size symbol and changing the default alignment setting. The value
of the size integer is always rounded up to a multiple of 4 bytes.

See `psn00bsdk_target_incbin()` above for more details.

## Target properties

Each of the following properties can be set individually for each executable or
library using CMake's `set_property()` and `set_target_properties()` commands.

### `PSN00BSDK_TARGET_TYPE`

Determines which SDK libraries are linked to and which compiler flags are added
to the target. Must be set to `EXECUTABLE_GPREL`, `EXECUTABLE_NOGPREL` or
`SHARED_LIBRARY`.

This property is initialized automatically on executables and DLLs created via
`psn00bsdk_add_executable()` or `psn00bsdk_add_library()`, but *not* on static
libraries as CMake has no way to know about their intended usage (i.e. whether
they are going to be linked to an executable with or without $gp-relative
addressing, or to a DLL). Thus, `PSN00BSDK_TARGET_TYPE` must be set manually on
all static libraries and must match the value set on any executable or DLL the
static library is going to be linked to.

There is no way to build a "hybrid" static library that can be linked to
multiple target types, short of building multiple copies of it. A workaround
(used internally by PSn00bSDK) is to create a virtual target and use CMake
generator expressions to link to one of the copies depending on the value of
`PSN00BSDK_TARGET_TYPE`.

## Preprocessor definitions

When compiling executables and libraries using the commands listed above the
following C/C++ preprocessor macros are automatically `#define`d:

### `PSN00BSDK`

Always set to 1. Can be used to implement different options or code paths for
projects that target both PSn00bSDK and other platforms.

### `NDEBUG`

Defined and set to 1 in a release configuration, i.e. when `CMAKE_BUILD_TYPE`
is set to `Release` or when a multi-configuration generator is building the
project in release mode; not defined if the project is being built in debug
mode. This value is used by the PSn00bSDK libraries, and should be used in
projects, to enable assertions and additional debug logging (the `assert()`
macro already resolves to a no-op in release mode).

Note that the default CMake configuration is usually debug. It is recommended
to build a project in release mode whenever appropriate (by specifying
`-DCMAKE_BUILD_TYPE=Release` or using the Ninja multi-configuration generator)
to get rid of logging overhead.

## Cached settings

These variables are stored in CMake's cache and are meant to be set by the end
user when building the project (rather than by the project itself). They can be
modified by the build script after invoking `project()`, from the CMake command
line when configuring (`-Dname=value`) or using an IDE or other editor such as
the CMake GUI.

### `PSN00BSDK_TARGET` (`STRING`)

The GCC toolchain's target triplet. PSn00bSDK assumes the toolchain targets
`mipsel-none-elf` by default, however this can be changed to e.g. use a MIPS
toolchain that was compiled for `mipsel-unknown-elf` (as used by previous
versions of PSn00bSDK).

Toolchains that target `mipsel-linux-gnu` are not supported by PSn00bSDK.

### `PSN00BSDK_TC` (`PATH`)

Path to the GCC toolchain's installation prefix/directory. If not set, CMake
will attempt to find the toolchain in the `PATH` environment variable and store
its path in the project's variable cache (so the search does not have to be
repeated). It is recommended to add the toolchain's `bin` subfolder to `PATH`
rather than setting this variable.

**IMPORTANT**: if the toolchain's target triplet is not `mipsel-none-elf`,
`PSN00BSDK_TARGET` must be set regardless of whether or not `PSN00BSDK_TC` is
also set.

## Internal settings

These settings are not stored in CMake's cache and can only be set from within
the build script after invoking `project()`.

### `PSN00BSDK_EXECUTABLE_LINK_LIBRARIES`, `PSN00BSDK_SHARED_LIBRARY_LINK_LIBRARIES` (list of `STRING`)

Lists of SDK libraries to be linked automatically to all new executables and
DLLs, respectively. By default `PSN00BSDK_EXECUTABLE_LINK_LIBRARIES` includes
all libraries that ship with PSn00bSDK while
`PSN00BSDK_SHARED_LIBRARY_LINK_LIBRARIES` is empty.

These variables can be modified before invoking `psn00bsdk_add_executable()` or
`psn00bsdk_add_library()` to only link a subset of the SDK. Static libraries
are *not* automatically linked to any SDK libraries.

### `PSN00BSDK_EXECUTABLE_SUFFIX`, `PSN00BSDK_SHARED_LIBRARY_SUFFIX`, `PSN00BSDK_SYMBOL_MAP_SUFFIX` (`STRING`)

File extensions to use for generated PS1 files. The default values are `.exe`,
`.dll` and `.map` respectively. These extensions do not have to match the ones
used in the CD image (if any).

## Read-only variables

### `PSN00BSDK_VERSION`, `PSN00BSDK_BUILD_DATE`, `PSN00BSDK_GIT_TAG`, `PSN00BSDK_GIT_COMMIT` (`STRING`)

These variables are loaded from `lib/libpsn00b/build.json` and contain
information about the SDK's version. `PSN00BSDK_GIT_TAG` and
`PSN00BSDK_GIT_COMMIT` might be empty strings as they are only populated in CI
builds of PSn00bSDK.

### `PSN00BSDK_LIBRARIES` (list of `STRING`)

List of all libraries that ship with PSn00bSDK, excluding `libgcc`. Each
library in this list is also defined as a target. See the targets section for
more details.

### `PSN00BSDK_TOOLS`, `PSN00BSDK_INCLUDE`, `PSN00BSDK_LDSCRIPTS` (list of `PATH`)

Lists of paths used internally. Should not be set, manipulated or overridden by
scripts.

### `TOOLCHAIN_NM` (`FILEPATH`)

Path to the `nm` executable used to generate symbol maps. Although not used
internally by CMake, this program is part of the GCC toolchain.

### `ELF2X`, `ELF2CPE`, `MKPSXISO`, `LZPACK`, `SMXLINK` (`FILEPATH`)

Paths to the PSn00bSDK tools' executables. As no functions are currently
provided for building assets, `LZPACK` and `SMXLINK` can be used manually with
CMake's `add_custom_command()` and `add_custom_target()` to convert models and
generate LZP archives as part of the build pipeline.

-----------------------------------------
_Last updated on 2023-07-03 by spicyjpeg_
