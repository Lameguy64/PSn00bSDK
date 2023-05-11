# PSn00bSDK BIOS API stubs
# (C) 2022 spicyjpeg - MPL licensed

# This file has been generated automatically. Each function is placed in its
# own section to allow the linker to strip unused functions.

.set noreorder

## B0 table functions (6)

.section .text.cd
.global cd
.type cd, @function
cd:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x40

.section .text.firstfile
.global firstfile
.type firstfile, @function
firstfile:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x42

.section .text.nextfile
.global nextfile
.type nextfile, @function
nextfile:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x43

.section .text.rename
.global rename
.type rename, @function
rename:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x44

.section .text.erase
.global erase
.type erase, @function
erase:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x45

.section .text.undelete
.global undelete
.type undelete, @function
undelete:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x46

