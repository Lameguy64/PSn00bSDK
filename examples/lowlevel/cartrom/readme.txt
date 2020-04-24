LibPSn00b Example Programs
Part of the PSn00bSDK Poject

TurboBoot Example by Lameguy64


Explanation:

This example demonstrates how to create a ROM program to be used on a cheat
cartridge such as an Action Replay or Xplorer using GNU assembler. The program
stored in ROM is executed pretty much instantly on power-on.

The PS1 BIOS can execute code from a ROM chip connected to the expansion port
(or Parallel I/O port) such as a cheat cartridge provided the valid headers
are present. However, the boot vectors are somewhat limited as it only
provides a preboot vector and a postboot vector.

The preboot vector is executed at the earliest point of the BIOS start-up
sequence which means the RAM is almost completely blank and you can't execute
PS-EXEs from this point whearas the postboot vector is only executed between
the PS logo and game execution. To get around this limitation, the preboot
vector is used to execute code that places a breakpoint to a desired point
in memory and returns execution to the BIOS. This will effectively intercept
execution once it reaches the breakpoint allowing to run code from ROM with
an initialized kernel allowing for bootstrapping PS-EXEs from ROM, CD or
from a comms interface such as serial.

The Xplorer and later Pro Action Replay cheat devices use this so called
'midboot' trick in their firmwares.

In this TurboBoot example, the ROM program attempts to boot a PS-EXE from
CD using BIOS CD-ROM and file functions. It will also parse through the
SYSTEM.CNF to retrieve the file name of executable as well as some boot
parameters such as stack addres and number of TCBs and EvCBs. The ROM
program can fallback to attempting to load a PS-EXE named PSX.EXE if the
SYSTEM.CNF file does not exist like in the official BIOS.

This example can also be used as a turbo boot utility as it boots straight
to the game, skipping the start-up animation altogether. It cannot be used
as a way to bypass the authentication check of an unmodified console however
(though it would help with bypassing the license data check on older Japanese
consoles) as the CD-ROM would only read data discs correctly IF the elusive
wobble groove containing the SCE string of the disc is present or when a
modchip is installed. It is possible to circumvent this by issuing special
'CD Unlock' commands (see nocash's psx-spx document) to the CD-ROM controller
which would effectively turn this into a chipless modchip solution that plugs
to the expansion port. However, the unlock commands only works on US and EU
consoles.

It is possible and pretty trivial to boot a PS-EXE straight from ROM by
simply copying the program text from ROM to its desired location in RAM and
transferring execution to it. Such an example may be created in the future.

This ROM program example had to be written entirely in assembly as C cannot
be used to write a ROM program even though it should be possible with some
assembly code to take care of the bootstrapping. GNU ld would complain about
'relocation truncation of R_MIPS_GPREL16' when you map program text to ROM
starting at 0x1f000000 (EXP ROM segment) and program data and bss sections
to RAM. Unknown how to get around this as all methods I've tried so far
either don't work or it just produces a massive binary file.


Building:

To build this example, simply run make and a file named cartrom.rom should
be created. Run the ROM in no$psx (make sure a ISO image has been opened
first) or burn it to an Xplorer/PAR EEPROM (I recommend modding a PAR with
a DIP32 socket if you wish to program the chip externally which in many
cases would be easier).


Changelog:

May 23, 2019 - Initial version.
