# LibPSn00b Example Programs
# Part of the PSn00bSDk project
#
# TurboBoot Example by Lameguy64
#
# Note: This example is being obsoleted as GAS is not ideal for making
# ROM firmwares. Use ARMIPS instead, but it cannot build this example
# as it is not GAS syntax compatible.


# Uncomment either PAR or XPLORER depending on the cartridge
# you're going to use (makes disabling turbo boot via switch to work)

#.set PAR, 0
#.set XPLORER, 1


.set noreorder

.include "cop0.inc"				# Contains definitions for cop0 registers

.set SP_base,		0x801ffff0
.set BREAK_ADDR,	0xa0000040	# cop0 breakpoint vector address


.set RAM_buff,		2048
.set RAM_handle,	2052
.set RAM_tcb,		2056
.set RAM_evcb,		2060
.set RAM_stack,		2064
.set RAM_psexe,		2068


.set EXE_pc0,		0			# PS-EXE header offsets
.set EXE_gp0,		4
.set EXE_taddr,		8
.set EXE_tsize,		12
.set EXE_daddr,		16
.set EXE_dsize,		20
.set EXE_baddr,		24
.set EXE_bsize,		28
.set EXE_spaddr,	32
.set EXE_sp_size,	36
.set EXE_sp,		40
.set EXE_fp,		44
.set EXE_gp,		48
.set EXE_ret,		52
.set EXE_base,		56
.set EXE_datapos,	60


.section .text


# ROM header
#
# The Licensed by... strings are essential otherwise the BIOS will not
# execute the boot vectors. Always make sure the tty message fields (string
# after Licensed by) must be no more than 80 bytes long and must have a null
# terminating byte.
#
# Postboot vector isn't particularly practical as its only executed in between
# the PS boot logo and the point where game execution occurs.
#
header:
	# Postboot vector
	.word	0
	.ascii	"Licensed by Sony Computer Entertainment Inc."
	.ascii 	"Not officially licensed or endorsed by Sony Computer Entertainment Inc."
	
	.balign	0x80	# This keeps things in the header aligned
	
	# Preboot vector
	.word	preboot
	.ascii	"Licensed by Sony Computer Entertainment Inc."
	.ascii 	"Cart ROM example for PSn00bSDK https://github.com/lameguy64/psn00bsdk"
	
	.balign	0x80	# This keeps things in the header aligned
	
	
# Preboot vector
#
# All it does is it initializes a breakpoint vector at 0x40
# and sets a cop0 breakpoint at 0x80030000 to perform a midboot
# exploit as preboot doesn't have the kernel area initialized.
#
preboot:
	
	li		$v0, 1
	
.ifdef XPLORER
	lui		$a0, 0x1f06				# Read switch status for Xplorer
	lbu		$v0, 0($a0)
.endif

.ifdef PAR
	lui		$a0, 0x1f02				# Read switch status for PAR/GS devices
	lbu		$v0, 0x18($a0)
.endif

	nop
	andi	$v0, 0x1
	beqz	$v0, .no_rom			# If switch is off don't install hook
	nop								# and effectively disables the cartridge
	
	li		$v0, BREAK_ADDR			# Patch a jump at cop0 breakpoint vector
	
	li		$a0, 0x3c1a1f00			# lui $k0, $1f00
	sw		$a0, 0($v0)
	la		$a1, entry				# ori $k0, < address to code entrypoint >
	andi	$a1, 0xffff
	lui		$a0, 0x375a
	or		$a0, $a1
	sw		$a0, 4($v0)
	li		$a0, 0x03400008			# jr  $k0
	sw		$a0, 8($v0)
	sw		$0 , 12($v0)			# nop
	
	lui		$v0, 0xffff				# Set BPCM and BDAM masks
	ori		$v0, 0xffff
	mtc0	$v0, BDAM
	mtc0	$v0, BPCM
	
	
	li		$v0, 0x80030000			# Set break on PC and data-write address
	
	mtc0	$v0, BDA				# BPC break is for compatibility with no$psx
	mtc0	$v0, BPC				# as it does not emulate break on BDA
	
	lui		$v0, 0xeb80				# Enable break on data-write and and break
	mtc0	$v0, DCIC				# on PC to DCIC control register
	
.no_rom:

	jr		$ra						# Return to BIOS
	nop


# Actual ROM entrypoint
.global entry
entry:
	
	mtc0	$0 , DCIC				# Clear DCIC register
	
	la		$sp, SP_base			# Set stack base
	la		$gp, 0x8000c000			# Set GP address as RAM base addr in this case
	
	jal		SetDefaultExitFromException		# Set default exit handler just in case
	nop
	jal		ExitCriticalSection		# Exit out of critical section (brings back interrupts)
	nop
	
	# Beyond this point, the PS1 is in full control to the ROM
	
	la		$a0, m_banner			# Print out program banner
	jal		printf
	addiu	$sp, -4
	addiu	$sp, 4
	
	la		$a0, m_cdinit
	jal		printf
	addiu	$sp, -4
	addiu	$sp, 4
	
	jal		_96_init				# Initialize the CD
	nop
	
	la		$a0, m_ok				# Print OK message if init didn't crash
	jal		printf
	addiu	$sp, -4
	addiu	$sp, 4
	
	la		$a0, m_readfile
	la		$a1, s_systemcnf
	jal		printf
	addiu	$sp, -8
	addiu	$sp, 8
	
	la		$a0, s_systemcnf		# Attempt to open the SYSTEM.CNF file on CD
	li		$a1, 1
	jal		open
	addiu	$sp, -8
	addiu	$sp, 8
	
	bltz	$v0, .no_systemcnf		# Fallback to loading PSX.EXE if not found
	nop
	
	sw		$v0, RAM_handle($gp)	# Save file handle
	
	move	$a0, $v0				# Read file contents of SYSTEM.CNF
	move	$a1, $gp
	li		$a2, 0x0800
	jal		read
	addiu	$sp, -12
	addiu	$sp, 12
	
	lw		$a0, RAM_handle($gp)	# Close file
	jal		close
	addiu	$sp, -4
	addiu	$sp, 4
	
	la		$a0, m_ok				# Output ok message
	jal		printf
	addiu	$sp, -4
	addiu	$sp, 4
	
	# Parse CNF file
	
	la		$a0, m_parsecnf
	jal		printf
	nop
	
	la		$a1, s_tcb				# Get TCB number
	jal		strcasestr
	move	$a0, $gp
	jal		skipspace				# Skip spaces
	addiu	$a0, $v0, 3
	addiu	$a0, $v0, -2			# Step two charactters back and inject '0x'
	li		$v0, '0'
	sb		$v0, 0($a0)
	li		$v0, 'x'
	sb		$v0, 1($a0)
	jal		atoi
	addiu	$sp, -4
	addiu	$sp, 4
	move	$s1, $v0
	
	la		$a1, s_evcb				# Get EVCB number
	jal		strcasestr
	move	$a0, $gp
	jal		skipspace
	addiu	$a0, $v0, 5
	addiu	$a0, $v0, -2
	li		$v0, '0'
	sb		$v0, 0($a0)
	li		$v0, 'x'
	sb		$v0, 1($a0)
	jal		atoi
	addiu	$sp, -4
	addiu	$sp, 4
	move	$s0, $v0
	
	la		$a1, s_stack			# Get STACK address
	jal		strcasestr
	move	$a0, $gp
	jal		skipspace
	addiu	$a0, $v0, 5
	addiu	$a0, $v0, -2
	li		$v0, '0'
	sb		$v0, 0($a0)
	li		$v0, 'x'
	sb		$v0, 1($a0)
	jal		atoi
	addiu	$sp, -4
	addiu	$sp, 4
	move	$s2, $v0
	
	la		$a1, s_boot				# Get the PS-EXE file name
	jal		strcasestr
	move	$a0, $gp
	
	jal		skipspace				# Skip spaces
	addiu	$a0, $v0, 4
	
	addiu	$a0, $gp, RAM_psexe		# Extract the line
	jal		getline
	move	$a1, $v0
	
	la		$a0, m_ok				# Print successful parsing
	jal		printf
	addiu	$sp, -4
	addiu	$sp, 4
	
	la		$a0, m_readfile
	addiu	$a1, $gp, RAM_psexe
	jal		printf
	addiu	$sp, -8
	addiu	$sp, 8
	
	b		.do_load				# Proceed loading PS-EXE
	addiu	$a0, $gp, RAM_psexe
	
.no_systemcnf:						# Load fallback

	la		$a0, m_notfound
	jal		printf
	addiu	$sp, -4
	addiu	$sp, 4
	
	la		$a0, m_fallback
	jal		printf
	addiu	$sp, -4
	addiu	$sp, 4
	
	li		$s0, 0x10				# Default EvCBs and TCBs
	li		$s1, 0x04
	li		$s2, SP_base			# Default stack
	la		$a0, s_psxexe			# Attempt loading PSX.EXE
	
.do_load:

	jal		LoadExe					# Load PS-EXE
	move	$a1, $gp
	
	beqz	$v0, load_fail
	nop

	la		$a0, m_ok
	jal		printf
	addiu	$sp, -4
	addiu	$sp, 4
	
	sw		$s2, EXE_sp($gp)		# Patch the header
	sw		$s2, EXE_spaddr($gp)
	
	la		$a0, m_boot
	jal		printf
	addiu	$sp, -4
	addiu	$sp, 4
	
	jal		EnterCriticalSection	# Disable interrupt handling
	nop
	
	move	$a0, $s0				# Set configuration (EvCBs and TCBs)
	move	$a1, $s1
	move	$a2, $s2
	jal		SetConf
	addiu	$sp, -12
	addiu	$sp, 12
	
	move	$a0, $gp				# Transfer execution
	move	$a1, $0
	move	$a2, $0
	jal		DoExec
	addiu	$sp, -12
	addiu	$sp, 12
	

load_fail:							# Fail state
	la		$a0, m_loadfail
	jal		printf
	nop
.fail_loop:
	b		.fail_loop
	nop

	
.include "parse.inc"
.include "bios.inc"


# Strings

s_boot:
	.asciz "BOOT"
	.balign 4
s_tcb:
	.asciz "TCB"
	.balign 4
s_evcb:
	.asciz "EVENT"
	.balign 4
s_stack:
	.asciz "STACK"
	.balign 4
s_systemcnf:
	.asciz "cdrom:SYSTEM.CNF;1"
	.balign 4
s_psxexe:
	.asciz "cdrom:PSX.EXE;1"
	.balign 4
	

# Message strings

m_banner:
	.asciz "\nCARTROM Bootstrap Example by Lameguy64\nPart of the PSn00bSDK Project\n\n"
	.balign 4
m_cdinit:
	.asciz "Initializing CD-ROM (BIOS)... "
	.balign 4
m_readfile:
	.asciz "Attempting to read %s... "
	.balign 4
m_parsecnf:
	.asciz "Parsing CNF file... "
	.balign 4
m_fallback:
	.asciz "Falling back to loading PSX.EXE... "
	.balign 4
m_notfound:
	.asciiz "Not found.\n"
	.balign 4
m_ok:
	.asciz "Ok.\n"
	.balign 4
m_boot:
	.asciz "Boot!\n"
	.balign 4
m_loadfail:
	.asciz "Failed to load PS-EXE file.\n"
	.balign 4
