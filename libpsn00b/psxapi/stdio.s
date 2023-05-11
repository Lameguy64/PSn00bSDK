# PSn00bSDK BIOS API stubs
# (C) 2022 spicyjpeg - MPL licensed

# This file has been generated automatically. Each function is placed in its
# own section to allow the linker to strip unused functions.

.set noreorder

## A0 table functions (14)

.section .text.open
.global open
.type open, @function
open:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x00

.section .text.lseek
.global lseek
.type lseek, @function
lseek:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x01

.section .text.read
.global read
.type read, @function
read:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x02

.section .text.write
.global write
.type write, @function
write:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x03

.section .text.close
.global close
.type close, @function
close:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x04

.section .text.ioctl
.global ioctl
.type ioctl, @function
ioctl:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x05

.section .text.isatty
.global isatty
.type isatty, @function
isatty:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x07

.section .text.getc
.global getc
.type getc, @function
getc:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x08

.section .text.putc
.global putc
.type putc, @function
putc:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x09

.section .text.getchar
.global getchar
.type getchar, @function
getchar:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x3b

.section .text.putchar
.global putchar
.type putchar, @function
putchar:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x3c

.section .text.gets
.global gets
.type gets, @function
gets:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x3d

.section .text.puts
.global puts
.type puts, @function
puts:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x3e

.section .text.printf
.global printf
.type printf, @function
printf:
	li $t2, 0xa0
	jr $t2
	li $t1, 0x3f

## B0 table functions (2)

.section .text._get_errno
.global _get_errno
.type _get_errno, @function
_get_errno:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x54

.section .text._get_error
.global _get_error
.type _get_error, @function
_get_error:
	li $t2, 0xb0
	jr $t2
	li $t1, 0x55

