# PSn00bSDK _start() trampoline
# (C) 2022 spicyjpeg - MPL licensed
#
# This file provides a weak function that can be easily overridden to e.g. set
# $sp or perform additional initialization before the "real" _start()
# (_start_inner()) is called.

.set noreorder
.section .text

.global _start
.type _start, @function
.weak _start
_start:
	la $gp, _gp

	j  _start_inner
	nop
