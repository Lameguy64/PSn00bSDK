# PSn00bSDK _start() trampoline and logging endpoint
# (C) 2022 spicyjpeg - MPL licensed
#
# This file provides a weak function that can be easily overridden to e.g. set
# $sp or perform additional initialization before the "real" _start() function
# (_start_inner()) is called. The _sdk_log_inner() function called by other
# libraries to log debug messages can also be overridden in a similar way.

.set noreorder

.section .text._start
.global _start
.type _start, @function
.weak _start
_start:
	la $gp, _gp
	j  _start_inner
	nop

.section .text._sdk_log_inner
.global _sdk_log_inner
.type _sdk_log_inner, @function
.weak _sdk_log_inner
_sdk_log_inner:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x3f
