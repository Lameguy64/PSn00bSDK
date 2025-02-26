# Setting up the SDK

Obviously, to make some PS1 homebrew you\'ll need an SDK to be able to
develop software for the platform in the first place. The SDKs that are
going to be covered in this tutorial series is the official Programmers
Tool SDK (often known as PsyQ) and PSn00bSDK.

There are other open source SDK projects but with varying degrees of
programming quality and hardware support and usually have drastically
different library APIs which are not compatible with this tutorial
series. If you much rather use an open source SDK I recommend using
PSn00bSDK as its the best one so far (not a shameless self plug, its
actually better than others) and follows more or less the same API
syntax as the official SDK making it very compatible with this tutorial
series.

## Choosing an SDK

### Programmers Tool/PsyQ SDK

This is the official SDK provided by Sony and SN Systems used by pretty
much all officially licensed developers back in the day, though there
were a number that used Codewarrior for PS1 instead. Its often called
PsyQ SDK but the correct name is Programmers Tool SDK as that is what it
was officially called in the distribution discs and manuals, and the
PsyQ name also exists in the PC compatible versions of the Ultra64 SDK.

Majority of the SDK such as tools and libraries was developed by Sony
themselves but the toolchain (SDevTC) was made by SN Systems and is
based off of the GCC toolchain. But the object and library file format
appear to be of a custom format and do not appear to be compatible with
the standard GCC file formats, the toolchain additionally comes with
custom linker and library tools.

The biggest advantage of using this SDK is that you have all console
functionality readily accessible out of the box, but the biggest
downside is that while Sony no longer seems to care about protecting the
official SDK it is still technically a legal gray area to use it for
homebrew. While people have distributed commercial homebrew made with
the official SDK and never seem to have gotten in trouble for it, I\'d
say you can still use this SDK for non commercial projects but that\'s
entirely up to you, not me.

A lot of people say this SDK only works in 32-bit versions of Windows
due to its age and that you have to use a VM running XP to be able to
use it on a modern PC running 64-bit Windows. While many of the command
line tools were only distributed in 16-bit DOS versions there are Win32
versions of the toolchain that would would work in 64-bit Windows so the
ability to compile programs would still be possible. The cpe2x tool can
be replaced with Orion\'s version that\'s built for Win32 and bmp2tim
can be replaced with the much more flexible img2tim tool. There are no
Win32 versions of the RSD/TMD tools for 3D models unfortunately.

### PSn00bSDK

This is an open source PS1 SDK project of my own creation and was used
to create a small demo called n00bdemo, which demonstrates the graphical
potential that PSn00bSDK is capable of wiht full GTE support. As far as
I\'m aware, this is the only open source PS1 SDK capable of pushing out
graphics as competently as the official SDK (especially the GTE
support).

PSn00bSDK\'s library API follows the same API of the official libraries
for familiarity with experienced developers and to make porting existing
homebrew originally made with the Programmers Tool/PsyQ SDK to PSn00bSDK
easier. The main goal of the PSn00bSDK project is to create a free and
open source PS1 SDK solution that has the potential of replacing the
official SDK with eventual support of all the hardware features of the
console.

The biggest advantage of PSn00bSDK is its 100% legal to use and is by
far the most powerful as far as open source PS1 SDKs go. The SDK project
is licensed under MPL (Mozilla Public License) which shouldn\'t be a
deal breaker to those looking into making commercial homebrew with
PSn00bSDK as the MPL license only applies to PSn00bSDK, not the project
using PSn00bSDK. You may want to look up the license yourself if you
want to know the exact details of how it works however.

One of the downsides of PSn00bSDK however is its a bit tricky to setup
compared to the Programmers Tool SDK especially to those working in
Windows. If you\'re new to programming for the PS1 I recommend sticking
with the official SDK for educational purposes unless you\'re a seasoned
Linux user who knows how to compile things.

## Setting up the Programmer\'s Tool/PsyQ SDK

### Choosing an SDK download

There are two Programmers Tool SDK archives floating around the
Internet. One is a very old dump that leaked many years ago and is the
version that is most commonly found. The other one and is one I
recommend using is a dump of the Programmers Tool CD 2.2 from 1998 and
is much cleaner than the old dump. Most importantly it comes with Win32
versions of the library/linker utilities among other things.

The old PsyQ dump however includes some tools not available in the
Programmers Tool CD such as newer versions of ccpsx, cc1psx and cc1plpsx
compilers and BUILDCD. Though I don\'t think BUILDCD is all that useful
anymore now that there\'s MKPSXISO that\'s newer, faster, easier to use
and works on modern operating systems (including Linux and BSDs).

### Setting up the SDK

Since the installer program in the Programmers Tool CD is a 16-bit
application, installation of the SDK had to be done manually.

-   Download the Programmers Tool CD from the
    [psxdev](http://www.psxdev.net/downloads.html)
    downloads page.
-   Extract the ISO image from the RAR archive.
-   Create a directory named PS at the root of your C drive (C:\\PS \<-
    recommended path).
-   Open the ISO image with WinRAR or 7zip. Extract the following
    files/directories to the PS directory:
    -   DOC
    -   GNU
    -   PSSN
    -   PSX
    -   PSXGRAPH
    -   PSXSOUND
-   Go to System Properties -\> Advanced Tab and click on Environment
    Variables.
-   Edit the PATH variable in the user variables list add the following
    paths (separated by semicolons):
    -   C:\\PS\\PSSN\\BIN
    -   C:\\PS\\PSX\\BIN
    -   C:\\PS\\PSXGRAPH\\BIN
-   Create an SN\_PATH environment variable and set it to
    C:\\PS\\PSSN\\BIN.
-   Open up the command prompt and enter **ccpsx**. You should get a
    list of options for ccpsx.

### Setting up additional tools

-   Download the win32 replacement of cpe2x from [Orion\'s website](http://onorisoft.free.fr/psx/cpe2x.zip).
-   Extract and replace the **cpe2x.exe** file in C:\\PS\\PSX\\BIN.
-   Download img2tim from the [img2tim Github repo](https://github.com/Lameguy64/img2tim).
-   Extract the img2tim files to C:\\PS\\PSXGRAPH\\BIN
-   Download mkpsxiso from the [mkpsxiso Github repo](https://github.com/Lameguy64/mkpsxiso).
-   Extract the mkpsxiso files to C:\\PS\\PSSN\\BIN

---

[Back to Index](index.md)
