.set noreorder

.global _mem_init
.type _mem_init, @function
_mem_init:

.section .text

_mem_init:
	la		$a0, __bss_start
	la		$a1, _end
.Lclear_bss:
	sb		$0 , 0($a0)
	blt		$a0, $a1, .Lclear_bss
	addiu	$a0, 1
	la		$a0, _end+4			# Initialize heap for malloc (does not use BIOS maalloc)
	li		$a1, 1572864		# Allocate 1.5MB at end of bss
	j		InitHeap
	nop
	