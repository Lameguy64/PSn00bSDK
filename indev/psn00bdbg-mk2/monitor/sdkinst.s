#
# To use this installer properly, the program text must be compiled to
# load at a higher address such as 0x80012000.
#
# Then call the function early in your program:
#
#	void install_monitor(void);
#
#	...
#
#	int main(int argc, const char *argv[])
#	{
#		ResetGraph(0);
#
#		EnterCriticalSection();
#		install_monitor();
#		ExitCriticalSection();
#		...
#
.set noreorder

.section .text

.global _install_monitor
.type _install_monitor, @function
_install_monitor:							# = Patch installer routine

		addiu	$sp, -4
		sw		$ra, 0($sp)
		
		la		$a0, debug_payload
		la		$a1, debug_payload_end
		la		$a2, debug_payload
		subu	$a1, $a2
		jal		.Lcopymem
		lui		$a2, 0xA001
		
		lui		$a0, 0x8001
		jalr	$a0
		nop
		
		lw		$ra, 0($sp)
		addiu	$sp, 4
		jr		$ra
		nop
		
	.Lcopymem:								# installer's copy routine

		addiu	$a1, -4
		lw		$v0, 0($a0)
		addiu	$a0, 4
		sw		$v0, 0($a2)
		bgtz	$a1, .Lcopymem
		addiu	$a2, 4
		jr		$ra
		nop
		

.section .data

debug_payload:
	
		.incbin "patch.bin"

debug_payload_end:
