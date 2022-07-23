# PSn00bSDK syscall wrappers
# (C) 2022 spicyjpeg - MPL licensed

.set noreorder

.section .text.EnterCriticalSection
.global EnterCriticalSection
.type EnterCriticalSection, @function
EnterCriticalSection:
	li $a0, 0x01
	syscall 0

	jr $ra
	nop

.section .text.ExitCriticalSection
.global ExitCriticalSection
.type ExitCriticalSection, @function
ExitCriticalSection:
	li $a0, 0x02
	syscall 0

	jr $ra
	nop

.section .text.SwEnterCriticalSection
.global SwEnterCriticalSection
.type SwEnterCriticalSection, @function
SwEnterCriticalSection:
	mfc0 $a0, $12 # cop0r12 &= ~0x00000401
	li   $a1, -1026
	and  $a0, $a1
	mtc0 $a0, $12
	nop

	jr   $ra
	nop

.section .text.SwExitCriticalSection
.global SwExitCriticalSection
.type SwExitCriticalSection, @function
SwExitCriticalSection:
	mfc0 $a0, $12 # cop0r12 |= 0x00000401
	nop
	ori  $a0, 0x0401
	mtc0 $a0, $12
	nop

	jr   $ra
	nop
