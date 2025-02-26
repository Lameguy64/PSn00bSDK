# Lameguy\'s PlayStation Programming Series

> [!CAUTION]
> Lameguy's PlayStation Programming Tutorial Series was written with
> older versions of PSn00bSDK in mind and is outdated at this point,
> but may still be useful as an introduction to the console's hardware
> and the basics of the graphics and controller APIs.

Welcome to Lameguy64\'s PlayStation Programming Series! This is a series
of tutorials I (Lameguy64) put together in my spare time that covers the
basics and eventually, more advanced topics of programming for the
original PlayStation.

Keep in mind that this tutorial series assumes you already have prior
experience in C programming as PlayStation specific programming topics
are only covered here. If you\'re completely new to programming in
general it is recommended to learn C first from a different tutorial
series.

## Tutorial Index

[System Overview](overview.md)

[Setting up an SDK (needs revising)](setup_sdk.md)

Chapter 1: Basic Homebrew Programming

- [1.1. Setting up Graphics and Hello World](chapter_1_1.md)
- [1.2. Drawing Graphics Primitives](chapter_1_2.md)
- [1.3. Textures, TPages and CLUTs](chapter_1_3.md)
- [1.4. Controllers](chapter_1_4.md)
- [1.5. Fixed Point Math](chapter_1_5.md)
- [1.6. Using the CD-ROM](chapter_1_6.md)

Chapter 2: Moderately Advanced Graphics Programming

- [2.1. The Principles of PlayStation 3D](chapter_2_1.md)
- 2.2. The Geometry Transformation Engine
- 2.3. Drawing a Spinning Cube with the GTE
- 2.4. The Rotation Matrix and Implementing a First-Person Camera

Chapter 3: Intro to Assembly Language Programming

- 3.1. Writing for the Processor Directly
- 3.2. Performing a Hello World in Assembly (GAS)
- 3.3. Argument Passing and Calling Conventions

Chapter 4: CD-ROM Basics (old)

- [4.1. About the CD-ROM](chapter_4_1.md)
- [4.2. Reading a file from CD-ROM](chapter_4_2.md)
- [4.3. CD-ROM Optimizations](chapter_4_3.md)

## About this tutorial series

I decided to work on this tutorial project to help make PlayStation
homebrew programming a little more accessible even though this is likely
not going to change much on the state of the PS1 homebrew scene and that
nobody has bothered to write good programming tutorials that I\'m aware
of that don\'t just use libgs right at the start, usually with very
little explanation to how the hardware works which in my opinion is not
the right way to teach how to program for the target platform.

The pages are written in plain HTML with minimal CSS styling for
compatibility with older browsers so you can conveniently view the pages
on your Windows 98 PC that has a DTL-H2000 or similar development
hardware equipped.

## Why is this a SVN repo and not a wiki or blog series?

Because it is easier for me to update, easier to run, accessible on very
old web browsers and I rather self-host my stuff whenever possible.
Wikis and things like wordpress can get bent.

## Changelog

**June 28, 2021:**

-   Added missing tutorial indexes in chapters 1.3, 1.4 and 1.5.
-   Forgot to reference Chapters 2.2 and 2.3 in the tutorial index,
    going to be replaced with a better, more consolidated chapter
    however to conclude Chapter 1 with.
-   Added better CD-ROM chapter to conclude Chapter 1 with.

**March 10, 2021:**

-   Improved controllers chapter.
-   Fixed incorrect cell count in framebuffer illustrations.
-   Added fixed point math chapter.

**December 1, 2020:**

-   Added Controllers chapter.
-   Made some corrections to Chapter 1.3.
-   Made some corrections in the System Overview page.

**November 12, 2020:**

-   Significantly revised Chapter 1.3, added new illustrations and
    corrected mistakes in example code.

**June 4, 2020:**

-   Started changelog (should have started sooner).
-   Rewrote Chapter 1.1, now includes hello world with some new
    graphics.

---

2020 - 2021 Lameguy64 / Meido-Tek Productions

---

[Back to Main Page](../../)

---
