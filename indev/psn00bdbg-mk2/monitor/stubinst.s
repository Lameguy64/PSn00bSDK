#
# C-callable and linkable version of 'patchinst' portion of monitor.asm
#
# Assemble with:
#	mipsel-none-elf-gcc -march=r3000 -c stubinst.s -o stubinst.o
#
# Then call the function early in your program:
#
#	void mk2_InstallMonitor(void);
#
#	...
#
#	int main(int argc, const char *argv[])
#	{
#		ResetGraph(0);
#
#		EnterCriticalSection();
#		mk2_InstallMonitor();
#		ExitCriticalSection();
#		...
#
.set noreorder

#
# These constants must reflect those in monitor.asm
#
.set MONADDR,		0xC000
.set MAX_BREAK,		32

.set SAVE_mode,		0x30
.set SAVE_tmode,	0x31
.set SAVE_k0,		0x34
.set SAVE_k1,		0x38
.set SAVE_dcic,		0x3C

.set DB_BRK_FLAG,	0
.set DB_BRK_ADDR,	4
.set DB_BRK_INST,	8
.set DB_BRK_LCNT,	12
.set DB_BRK_HCNT,	14
.set DB_BRK_LEN,	16

.set DCIC,			$7

.section .text

.global _mk2_InstallMonitor
.type _mk2_InstallMonitor, @function
_mk2_InstallMonitor:

		addiu	$sp, -4
		sw		$ra, 0($sp)
		li		$a0, .Lbreakhook			# Install breakpoint vector hook
		li		$a1, (.Lbreakhook_end-.Lbreakhook)+4
		li		$a2, 0xA0000040
		jal		.Lcopymem
		nop
		li		$v0, 0x200					# Hook monitor entrypoint to
		li		$v1, 0x40					# SysErrUnresolvedException() slot
		sll		$v1, 2
		addu	$v0, $v1
		la		$v1, .Lpayload				# Get entrypoint address
		lw		$v1, 0($v1)
		nop
		sw		$v1, 0($v0)
		addiu	$t2, $0, 0xB0				# GetB0Table()
		jalr	$t2
		addiu	$t1, $0, 0x57
		li		$a0, 0x17					# Get pointer of ReturnFromException()
		sll		$a0, 2
		addu	$a0, $v0
		lw		$v0, 0($a0)					# Save original address for later
		la		$v1, .Lpayload
		lw		$v1, 0($v1)
		nop
		beq		$v0, $v1, .Lnoinstall		# Don't install if hooked already
		nop
		addiu	$sp, -4
		sw		$a0, 0($sp)
		li		$a0, .Lpayload				# Load monitor code into target
		li		$a1, (.Lpayload_end-.Lpayload)+4	# address
		lui		$a2, 0xA000					# write via uncached segment
		jal		.Lcopymem
		ori		$a2, MONADDR
		lw		$a0, 0($sp)
		addiu	$sp, 4
		lw		$v0, 0($a0)
		la		$v1, .Lpayload
		lw		$v1, 4($v1)
		nop
		sw		$v0, 0($v1)
		la		$v0, .Lpayload				# Set new address to table
		lw		$v0, 0($v0)
		nop
		sw		$v0, 0($a0)
		li		$a0, 0x80					# Move existing exception vector
		li		$a2, 0x90					# jump to prepend patch
		jal		.Lcopymem
		li		$a1, 16
		la		$a0, .Lexceptpatch			# Patch the exception vector for
		li		$a2, 0x80					# trace to work properly
		jal		.Lcopymem
		li		$a1, 16
		addiu	$t2, $0, 0xA0				# FlushCache() just to make sure
		jalr	$t2
		addiu	$t1, $0, 0x44
		jal		.Linit_breakpoints
		nop
	.Lnoinstall:
		lw		$ra, 0($sp)					# Return to caller, debugger
		addiu	$sp, 4                      # already installed
		jr		$ra
		nop
		
	# patchinst

#
# = installer's copy routine
#
.Lcopymem:
		addiu	$a1, -4
		lw		$v0, 0($a0)
		addiu	$a0, 4
		sw		$v0, 0($a2)
		bgtz	$a1, .Lcopymem
		addiu	$a2, 4
		jr		$ra
		nop
		
		# copymem

#
# = Initializes breakpoint list
#
.Linit_breakpoints:
		la		$a2, .Lpayload				# Get address of breakpoint table
		lw		$a2, 8($a2)
		li		$a3, MAX_BREAK
		addiu	$v0, $0, -1
	.Lclear_loop:
		sw		$v0, DB_BRK_INST($a2)
		sw		$v0, DB_BRK_ADDR($a2)
		sw		$0 , DB_BRK_FLAG($a2)
		addiu	$a3, -1
		bnez	$a3, .Lclear_loop
		addiu	$a2, DB_BRK_LEN
		jr		$ra
		nop
		# init_breakpoints
		
#
# = Break vector hook code
#
# This is copied to the breakpoint exception vector at address 40h
#
.Lbreakhook:
		sw		$k0, SAVE_k0($0)			# Save K0 and K1 registers
		mfc0	$k0, DCIC					# Save DCIC
		sw		$k1, SAVE_k1($0)
		mtc0	$0, DCIC					# Clear DCIC in case trace is still
		sw		$k0, SAVE_dcic($0)			# effective
		la		$k0, breakhandler			# Jump to break vector handler
		jr		$k0
		nop
.Lbreakhook_end:

#
# = Exception vector patch
#
# This is prepended to the exception vector jump at address 80h
#
.Lexceptpatch:
		mfc0	$k0, DCIC					# backup DCIC value
		nop
		mtc0	$0 , DCIC					# clear DCIC
		sw		$k0, SAVE_dcic($0)

#
# = Payload data
#
.section .data

.Lpayload:
	.incbin		"patchcode.bin"	
.Lpayload_end:
