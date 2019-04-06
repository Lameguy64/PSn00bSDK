# Start function!
# This is essentially the entrypoint of the PS-EXE

.set noreorder

.section .text

.global _start
.type _start, @function
_start:

	addiu	$sp, -32
	sw		$ra, 28($sp)
	
	la		$gp, _gp		# Very important to set!
	
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
	nop
	
	lw		$ra, 28($sp)
	addiu	$sp, 32
	jr		$ra
	nop
	