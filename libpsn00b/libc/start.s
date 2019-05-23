# Start function!
# This is essentially the entry point of the PS-EXE

.set noreorder

.section .text

.global _start
.type _start, @function
_start:
	addiu	$sp, -4
	sw		$ra, 0($sp)
	
	la		$gp, _gp		# Very important!
	
	la		$a0, .bss		# What are the CORRECT symbols for BSS start and end?
	la		$a1, _end
.clear_bss:
	sb		$0 , 0($a0)
	blt		$a0, $a1, .clear_bss
	addiu	$a0, 1
	
	la		$a0, _end+4		# Initialize heap for malloc (does not use BIOS maalloc)
	li		$a1, 1572864
	jal		InitHeap
	nop
	
	move	$a0, $0			# No support for arguments for now
	move	$a1, $0
	
	jal		main
	addiu	$sp, -8
	addiu	$sp, 8
	
	lw		$ra, 0($sp)		# Return
	addiu	$sp, 4
	jr		$ra
	nop
	