# PSn00bSDK BIOS API stubs
# (C) 2022 spicyjpeg - MPL licensed

# This file has been generated automatically. Each function is placed in its
# own section to allow the linker to strip unused functions.

.set noreorder

## A0 table functions (11)

.section .text.b_setjmp
.global b_setjmp
.type b_setjmp, @function
b_setjmp:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x13

.section .text.b_longjmp
.global b_longjmp
.type b_longjmp, @function
b_longjmp:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x14

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

.section .text.LoadExec
.global LoadExec
.type LoadExec, @function
LoadExec:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x51

.section .text.SetConf
.global SetConf
.type SetConf, @function
SetConf:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x9c

.section .text.GetConf
.global GetConf
.type GetConf, @function
GetConf:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x9d

.section .text.SetMem
.global SetMem
.type SetMem, @function
SetMem:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x9f

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

## B0 table functions (27)

.section .text.alloc_kernel_memory
.global alloc_kernel_memory
.type alloc_kernel_memory, @function
alloc_kernel_memory:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x00

.section .text.free_kernel_memory
.global free_kernel_memory
.type free_kernel_memory, @function
free_kernel_memory:
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

.section .text.DeliverEvent
.global DeliverEvent
.type DeliverEvent, @function
DeliverEvent:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x07

.section .text.OpenEvent
.global OpenEvent
.type OpenEvent, @function
OpenEvent:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x08

.section .text.CloseEvent
.global CloseEvent
.type CloseEvent, @function
CloseEvent:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x09

.section .text.WaitEvent
.global WaitEvent
.type WaitEvent, @function
WaitEvent:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x0a

.section .text.TestEvent
.global TestEvent
.type TestEvent, @function
TestEvent:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x0b

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

.section .text.OpenTh
.global OpenTh
.type OpenTh, @function
OpenTh:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x0e

.section .text.CloseTh
.global CloseTh
.type CloseTh, @function
CloseTh:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x0f

.section .text.ChangeTh
.global ChangeTh
.type ChangeTh, @function
ChangeTh:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x10

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

.section .text.ResetEntryInt
.global ResetEntryInt
.type ResetEntryInt, @function
ResetEntryInt:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x18

.section .text.HookEntryInt
.global HookEntryInt
.type HookEntryInt, @function
HookEntryInt:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x19

.section .text.UnDeliverEvent
.global UnDeliverEvent
.type UnDeliverEvent, @function
UnDeliverEvent:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x20

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

## C0 table functions (5)

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

.section .text.InstallExceptionHandlers
.global InstallExceptionHandlers
.type InstallExceptionHandlers, @function
InstallExceptionHandlers:
	li $t2, 0xc0
	jr $t2
	li $t1, 0x07

.section .text.SysInitMemory
.global SysInitMemory
.type SysInitMemory, @function
SysInitMemory:
	li $t2, 0xc0
	jr $t2
	li $t1, 0x08

.section .text.ChangeClearRCnt
.global ChangeClearRCnt
.type ChangeClearRCnt, @function
ChangeClearRCnt:
	li $t2, 0xc0
	jr $t2
	li $t1, 0x0a

