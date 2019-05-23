printf:
	addiu	$t2, $0, 0xA0
	jr		$t2
	addiu	$t1, $0, 0x3F

atoi:
	addiu	$t2, $0, 0xA0
	jr		$t2
	addiu	$t1, $0, 0x10
	
toupper:
	addiu	$t2, $0, 0xA0
	jr		$t2
	addiu	$t1, $0, 0x25
	
open:
	addiu   $t2, $0, 0xB0
	jr      $t2
	addiu   $t1, $0, 0x32

read:
	addiu   $t2, $0, 0xB0
	jr      $t2
	addiu   $t1, $0, 0x34

close:
	addiu   $t2, $0, 0xB0
	jr      $t2
	addiu   $t1, $0, 0x36

_96_init:
	addiu   $t2, $0, 0xA0
	jr      $t2
	addiu   $t1, $0, 0x71

LoadExe:
	addiu	$t2, $0, 0xA0
	jr		$t2
	addiu	$t1, $0, 0x42
	
DoExec:
	addiu	$t2, $0, 0xA0
	jr		$t2
	addiu	$t1, $0, 0x43
	
SetConf:
	addiu	$t2, $0, 0xA0
	jr		$t2
	addiu	$t1, $0, 0x9C
	
SetDefaultExitFromException:
	addiu	$t2, $0, 0xB0
	jr		$t2
	addiu	$t1, $0, 0x18
	
EnterCriticalSection:
	addiu	$a0, $0, 1
	syscall	0
	jr		$ra
	nop
	
ExitCriticalSection:
	addiu	$a0, $0, 2
	syscall	0
	jr		$ra
	nop
	