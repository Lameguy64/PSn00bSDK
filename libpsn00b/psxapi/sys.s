# PSn00bSDK BIOS API stubs
# (C) 2022 spicyjpeg - MPL licensed

# This file has been generated automatically. Each function is placed in its
# own section to allow the linker to strip unused functions.

.set noreorder

## A0 table functions (5)

.section .text.b_InitHeap
.global b_InitHeap
.type b_InitHeap, @function
b_InitHeap:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x39

.section .text.Exec
.global Exec
.type Exec, @function
Exec:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x43

.section .text.FlushCache
.global FlushCache
.type FlushCache, @function
FlushCache:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x44

.section .text._boot
.global _boot
.type _boot, @function
_boot:
	li $t2, 0xa0
	jr $t2
	li $t1, 0xa0

.section .text.GetSystemInfo
.global GetSystemInfo
.type GetSystemInfo, @function
GetSystemInfo:
	li $t2, 0xa0
	jr $t2
	li $t1, 0xb4

## B0 table functions (19)

.section .text._kernel_malloc
.global _kernel_malloc
.type _kernel_malloc, @function
_kernel_malloc:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x00

.section .text._kernel_free
.global _kernel_free
.type _kernel_free, @function
_kernel_free:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x01

.section .text.SetRCnt
.global SetRCnt
.type SetRCnt, @function
SetRCnt:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x02

.section .text.GetRCnt
.global GetRCnt
.type GetRCnt, @function
GetRCnt:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x03

.section .text.StartRCnt
.global StartRCnt
.type StartRCnt, @function
StartRCnt:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x04

.section .text.StopRCnt
.global StopRCnt
.type StopRCnt, @function
StopRCnt:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x05

.section .text.ResetRCnt
.global ResetRCnt
.type ResetRCnt, @function
ResetRCnt:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x06

.section .text.OpenEvent
.global OpenEvent
.type OpenEvent, @function
OpenEvent:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x08

.section .text.EnableEvent
.global EnableEvent
.type EnableEvent, @function
EnableEvent:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x0c

.section .text.DisableEvent
.global DisableEvent
.type DisableEvent, @function
DisableEvent:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x0d

.section .text.InitPAD
.global InitPAD
.type InitPAD, @function
InitPAD:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x12

.section .text.StartPAD
.global StartPAD
.type StartPAD, @function
StartPAD:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x13

.section .text.StopPAD
.global StopPAD
.type StopPAD, @function
StopPAD:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x14

.section .text.ReturnFromException
.global ReturnFromException
.type ReturnFromException, @function
ReturnFromException:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x17

.section .text.SetDefaultExitFromException
.global SetDefaultExitFromException
.type SetDefaultExitFromException, @function
SetDefaultExitFromException:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x18

.section .text.SetCustomExitFromException
.global SetCustomExitFromException
.type SetCustomExitFromException, @function
SetCustomExitFromException:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x19

.section .text.GetC0Table
.global GetC0Table
.type GetC0Table, @function
GetC0Table:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x56

.section .text.GetB0Table
.global GetB0Table
.type GetB0Table, @function
GetB0Table:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x57

.section .text.ChangeClearPAD
.global ChangeClearPAD
.type ChangeClearPAD, @function
ChangeClearPAD:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x5b

## C0 table functions (3)

.section .text.SysEnqIntRP
.global SysEnqIntRP
.type SysEnqIntRP, @function
SysEnqIntRP:
	li $t2, 0xc0
	jr $t2
	li $t1, 0x02

.section .text.SysDeqIntRP
.global SysDeqIntRP
.type SysDeqIntRP, @function
SysDeqIntRP:
	li $t2, 0xc0
	jr $t2
	li $t1, 0x03

.section .text.ChangeClearRCnt
.global ChangeClearRCnt
.type ChangeClearRCnt, @function
ChangeClearRCnt:
	li $t2, 0xc0
	jr $t2
	li $t1, 0x0a

## Syscalls (2)

.section .text.EnterCriticalSection
.global EnterCriticalSection
.type EnterCriticalSection, @function
EnterCriticalSection:
	li      $a0, 0x01
	syscall 0
	jr      $ra
	nop

.section .text.ExitCriticalSection
.global ExitCriticalSection
.type ExitCriticalSection, @function
ExitCriticalSection:
	li      $a0, 0x02
	syscall 0
	jr      $ra
	nop

