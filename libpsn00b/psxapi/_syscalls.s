# PSn00bSDK syscall wrappers
# (C) 2022-2023 spicyjpeg - MPL licensed

.set noreorder

## Interrupt enable/disable

.section .text.EnterCriticalSection, "ax", @progbits
.global EnterCriticalSection
.type EnterCriticalSection, @function

EnterCriticalSection:
	li  $a0, 0x01
	syscall 0

	jr  $ra
	nop

.section .text.ExitCriticalSection, "ax", @progbits
.global ExitCriticalSection
.type ExitCriticalSection, @function

ExitCriticalSection:
	li  $a0, 0x02
	syscall 0

	jr  $ra
	nop

.section .text.SwEnterCriticalSection, "ax", @progbits
.global SwEnterCriticalSection
.type SwEnterCriticalSection, @function

SwEnterCriticalSection:
	mfc0  $a0, $12 # cop0r12 &= ~0x401
	li    $a1, -1026
	and   $a1, $a0
	mtc0  $a1, $12
	andi  $a0, 0x0401 # return !((cop0r12_prev & 0x401) < 0x401)
	sltiu $v0, $a0, 0x0401

	jr    $ra
	xori  $v0, 1

.section .text.SwExitCriticalSection, "ax", @progbits
.global SwExitCriticalSection
.type SwExitCriticalSection, @function

SwExitCriticalSection:
	mfc0 $a0, $12 # cop0r12 |= 0x401
	nop
	ori  $a0, 0x0401
	mtc0 $a0, $12
	nop

	jr   $ra
	nop

## PCDRV (host file access) API

.section .text.PCinit, "ax", @progbits
.global PCinit
.type PCinit, @function

PCinit:
	break 0, 0x101 # () -> error

	jr    $ra
	nop

.section .text.PCcreat, "ax", @progbits
.global PCcreat
.type PCcreat, @function

PCcreat:
	li    $a2, 0
	move  $a1, $a0
	break 0, 0x102 # (path, path, 0) -> error, fd

	bgez  $v0, .Lcreate_ok # if (error < 0) fd = error
	nop
	move  $v1, $v0
.Lcreate_ok:
	jr    $ra # return fd
	move  $v0, $v1

.section .text.PCopen, "ax", @progbits
.global PCopen
.type PCopen, @function

PCopen:
	move  $a2, $a1
	move  $a1, $a0
	break 0, 0x103 # (path, path, mode) -> error, fd

	bgez  $v0, .Lopen_ok # if (error < 0) fd = error
	nop
	move  $v1, $v0
.Lopen_ok:
	jr    $ra # return fd
	move  $v0, $v1

.section .text.PCclose, "ax", @progbits
.global PCclose
.type PCclose, @function

PCclose:
	move  $a1, $a0
	break 0, 0x104 # (fd, fd) -> error

	jr    $ra
	nop

.section .text.PCread, "ax", @progbits
.global PCread
.type PCread, @function

PCread:
	move  $a3, $a1
	move  $a1, $a0
	break 0, 0x105 # (fd, fd, length, data) -> error, length

	bgez  $v0, .Lread_ok # if (error < 0) length = error
	nop
	move  $v1, $v0
.Lread_ok:
	jr    $ra # return length
	move  $v0, $v1

.section .text.PCwrite, "ax", @progbits
.global PCwrite
.type PCwrite, @function

PCwrite:
	move  $a3, $a1
	move  $a1, $a0
	break 0, 0x106 # (fd, fd, length, data) -> error, length

	bgez  $v0, .Lwrite_ok # if (error < 0) length = error
	nop
	move  $v1, $v0
.Lwrite_ok:
	jr    $ra # return length
	move  $v0, $v1

.section .text.PClseek, "ax", @progbits
.global PClseek
.type PClseek, @function

PClseek:
	move  $a3, $a2
	move  $a2, $a1
	move  $a1, $a0
	break 0, 0x107 # (fd, fd, offset, mode) -> error, offset

	bgez  $v0, .Lseek_ok # if (error < 0) offset = error
	nop
	move  $v1, $v0
.Lseek_ok:
	jr    $ra # return offset
	move  $v0, $v1
