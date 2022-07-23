# The indev directory

This directory is intended to contain work-in-progress SDK components
(libraries, tools, etc) that are still in prototype stage. These files are
not to be compiled in a typical SDK installation but is added into the
main repo as the redundant SVN repo on the Lameguy64 website will soon be
retired. Contributed components that are in work-in-progress status may
also go into this directory.

## Lameguy's indev components

* libpad: The early beginnings of a pad/card library using routines that
  accesses the pad/card interfaces directly. Ideally PSn00bSDK's pad library
  should include the functionality of pad/tap/gun peripherals into one
  library as well as include functions for reading and writing memory card
  sectors. Routines and callback hooks for directly controlling the pad/card
  interface should also be provided to support homebrewn peripherals.

  The Github release of this work-in-progress component includes delay
  corrections for PAL consoles.

  **NOTE**: the `io/pads` example also shows how to poll controllers manually
  in a slightly different way (using a timer), and includes a reusable
  low-level pad driver.

Work-in-progress components such as psxcd, interlace-exp, xptest and partest
are not included, as the former was completed while the remaining latter are
merely scrap test programs.