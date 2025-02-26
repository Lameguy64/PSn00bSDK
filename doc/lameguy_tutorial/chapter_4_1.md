# Chapter 4.1: About the CD-ROM

This chapter describes the CD-ROM subsystem of the PlayStation hardware
such as specifications, capabilities and limitations of the hardware.
This chapter also teaches the basics of using the CD-ROM subsystem in
your PlayStation projects, common optimization methods and more.

## Tutorial Index

- [The CD-ROM](#the-cd-rom)
- [Mod-chip Requirement](#mod-chip-requirement)
- [Methods of accessing the CD-ROM](#methods-of-accessing-the-cd-rom)

## The CD-ROM

Before delving into how to use the CD-ROM subsystem in your projects, it
is best to first understand the hardware first. You should know by now
that the CD-ROM subsystem uses CD\'s and that it can read either CD-ROMs
and CD-Rs. But getting the latter to read reliably requires a bit of
tweaking of the hardware to make such discs readable, though a mod-chip
is mandatory for being able to do anything useful with the CD-ROM
subsystem as far as homebrew is concerned which will be described in
greater detail later.

You should also know by now that the total capacity of a CD-ROM is about
700MB. Whilst pretty much any modern CD-ROM and CD-R disc is 700MB, the
original \'standard\' capacity of CDs is actually 650MB. Reason for
bringing this up is because the PS1 hardware could supposedly only
access up to the first 650MB of the disc and trying to access anything
past it will result in problems. This is something that may be worth
considering when packing a lot of content on a disc.

The CD-ROM hardware is capable of 2x read speed with transfer rates of
up to around 300KB/s. The hardware also supports ADPCM compressed \'XA\'
audio with interleaving capability of up to eight 37.8KHz Stereo audio
streams simultaneously, but only one of eight audio streams can be
\'listened\' from at a time. More channels is possible when using mono
and lower sample rate XA audio which may be covered in future chapters.
XA sectors can also be interleaved with data sectors and is usually used
for FMV movies. Obviously, the hardware can also play plain CD audio
tracks.

PS1 game discs are typically formatted in mixed Mode 2/Form 1 containing
a standard ISO9660 version 1 file system, so opening a PS1 disc in any
computer wouldn\'t look much different to a typical PC CD-ROM, except
perhaps files containing a data+XA interleave would be unreadable. XA
audio data is stored on the disc as Mode 2/Form 2 in the same track
containing Mode 2/Form 1 sectors.

## Mod-chip Requirement

An inherent problem in regards to producing homebrew on the PS1 is the
requirement of a mod-chip. The reason for this is to get around the
console\'s copy protection mechanism within the HC05 microcontroller
that drives the CD-ROM subsystem as a way to prevent unauthorized copies
from working on the console.

The copy protection mechanism works by storing a simple four character
license string inside the so-called wobble groove situated at the
innermost part of the disc. Normally, this special groove would contain
ATIP information used to identify CD-R medium with the wobble pattern
used for speed calibration prior to burning. CD-ROMs normally don\'t
contain this wobble groove but Sony repurposed it as an effective copy
protection mechanism, as it is impossible to replicate the wobble
pattern with any CD-R burner without special disc pressing equipment.
Producing special CD-Rs containing the license data in the wobble groove
is also not possible as replacing the ATIP information with the license
string would render the disc impossible to burn.

Perhaps as an extra level of security in the HC05 microcontroller, the
CD-ROM subsystem will not read data sectors properly if the disc is not
identified as a licensed PS1 disc. So even if you got your homebrew
running on the console through other means, such as a cheat cartridge,
you won\'t be able to do much with the CD-ROM subsystem as you won\'t be
able to read or stream data from the disc properly. However, a simple
swap trick with a licensed disc can be used to get around this, but it
has it\'s limitations. Namely, the table of contents read from the last
disc will persist, preventing the use of CD audio tracks and updating
the TOC is impossible wihtout clearing the licensed disc status in the
HC05. It is also not very convenient when testing homebrew as you\'ll
have to swap discs around in every turnaround.

The easiest and simplest solution to this problem is by installing a
mod-chip in the console, either a 12C508P microcontroller programmed
with MM3 or a compatible Arduino programmed with PSnee. The function of
a mod-chip is to basically fool the HC05 microcontroller into thinking
that the disc inserted is a licensed disc, by feeding the appropriate
license string into it at the right time. If you live in an area where
bootleg PS1 discs were prominent during the relevancy of the console
(often Asian territories), there\'s a good chance your console already
has a chip. Do not remove the chip even for \'legality\' reasons if you
intend to develop homebrew, unless you want to be counter intuitive to
yourself.

There is one more alternative to getting CD-Rs to read properly on the
PS1 that doesn\'t involve the use of a mod-chip or swap method and that
is by a backdoor in the HC05 firmware through the use of special [unlock
commands](http://problemkaputt.de/psx-spx.htm#cdromsecretunlockcommands).
Once these commands are issued the HC05 would behave like as if a
licensed disc is inserted and data sectors will be read properly.
However, this only works in US and EU region consoles and the only
bootloader that supports this feature currently is Sicklebrick\'s
UniROM.

## Methods of accessing the CD-ROM

There are two ways of accessing the CD-ROM in the software side. One is
through the BIOS CD-ROM subsystem and the other is through a CD-ROM
library (libcd for PsyQ/Programmers\' Tool or psxcd for PSn00bSDK). The
CD-ROM library is most recommended as it gives full control over the
CD-ROM hardware which the BIOS subsystem does not offer, but using the
BIOS yields a much smaller executable size and permits listing of
directory contents useful for utility style homebrew. PSn00bSDK however
provides directory querying facilities not available in
PsyQ/Programmers\' Tool SDK in it\'s CD-ROM library.

This chapter will only cover using the CD-ROM subsystem through the
libraries, as it offers greater functionality and performancec over the
BIOS subsystem.

---

  [Back to Index](index.md)  |  [Next](chapter_4_2.md)

---
