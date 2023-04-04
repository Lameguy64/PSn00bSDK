# PSn00bSDK BIOS API stubs
# (C) 2022 spicyjpeg - MPL licensed

# This file has been generated automatically. Each function is placed in its
# own section to allow the linker to strip unused functions.

.set noreorder

## A0 table functions (7)

.section .text._bu_init
.global _bu_init
.type _bu_init, @function
_bu_init:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x55

.section .text._96_init
.global _96_init
.type _96_init, @function
_96_init:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x71

.section .text._96_remove
.global _96_remove
.type _96_remove, @function
_96_remove:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x72

.section .text.add_nullcon_driver
.global add_nullcon_driver
.type add_nullcon_driver, @function
add_nullcon_driver:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x99

.section .text._card_info
.global _card_info
.type _card_info, @function
_card_info:
	li $t2, 0xa0
	jr $t2
	li $t1, 0xab

.section .text._card_load
.global _card_load
.type _card_load, @function
_card_load:
	li $t2, 0xa0
	jr $t2
	li $t1, 0xac

.section .text._card_clear
.global _card_clear
.type _card_clear, @function
_card_clear:
	li $t2, 0xa0
	jr $t2
	li $t1, 0xaf

## B0 table functions (12)

.section .text.AddDrv
.global AddDrv
.type AddDrv, @function
AddDrv:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x47

.section .text.DelDrv
.global DelDrv
.type DelDrv, @function
DelDrv:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x48

.section .text.ListDrv
.global ListDrv
.type ListDrv, @function
ListDrv:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x49

.section .text.InitCARD
.global InitCARD
.type InitCARD, @function
InitCARD:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x4a

.section .text.StartCARD
.global StartCARD
.type StartCARD, @function
StartCARD:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x4b

.section .text.StopCARD
.global StopCARD
.type StopCARD, @function
StopCARD:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x4c

.section .text._card_write
.global _card_write
.type _card_write, @function
_card_write:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x4e

.section .text._card_read
.global _card_read
.type _card_read, @function
_card_read:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x4f

.section .text._new_card
.global _new_card
.type _new_card, @function
_new_card:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x50

.section .text._card_chan
.global _card_chan
.type _card_chan, @function
_card_chan:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x58

.section .text._card_status
.global _card_status
.type _card_status, @function
_card_status:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x5c

.section .text._card_wait
.global _card_wait
.type _card_wait, @function
_card_wait:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x5d

