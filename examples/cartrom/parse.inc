# Skips spaces and equal signs (used by CNF parser)
skipspace:
	# a0 - Input string
	# Return: v0 - Address to first non-space character.
	lbu		$v1, 0($a0)
	nop
	beq		$v1, ' ', .skip
	nop
	beq		$v1, '=', .skip
	nop
	jr		$ra
	move	$v0, $a0
.skip:
	b		skipspace
	addiu	$a0, 1
	

# Copies a string until a CR/LF or space is encountered
getline:
	# a0 - Output address
	# a1 - String to copy from
	lbu		$v0, 0($a1)
	addiu	$a1, 1
	beqz	$v0, .end
	nop
	beq		$v0, 0x0D, .end
	nop
	beq		$v0, 0x0A, .end
	nop
	beq		$v0, ' ', .end
	nop
	sb		$v0, 0($a0)
	b		getline
	addiu	$a0, 1
.end:
	jr		$ra
	sb		$0, 0($a0)

	
# strcasestr implementation
strcasestr:
	# a0 - String to search
	# a1 - String to find
	addiu	$sp, -24
	sw		$ra, 0($sp)
	sw		$s0, 4($sp)
	sw		$s1, 8($sp)
	sw		$a1, 16($sp)
	
.scan_start:

	sw		$a0, 12($sp)
	
.comp_loop:
	
	lbu		$s0, 0($a0)				# Load character from A and B
	lbu		$s1, 0($a1)
	
	beqz	$s0, .end_strcasestr
	nop
	beqz	$s1, .found
	nop
	
	sw		$a0, 20($sp)			# Save a0 parameter
	
	jal		toupper					# tolower character A
	move	$a0, $s0
	move	$s0, $v0
	
	jal		toupper					# tolower character B
	move	$a0, $s1
	move	$s1, $v0
	
	lw		$a0, 20($sp)			# Restore a0 parameter
	
	addiu	$a1, 1
	addiu	$a0, 1
	
	beq		$s0, $s1, .comp_loop	# If value matches continue compare
	nop

.end_strcasestr:

	lw		$a0, 12($sp)			# Rescan from next character of string A
	lw		$a1, 16($sp)
	addiu	$a0, 1
	
	lbu		$v0, 0($a0)
	nop
	beqz	$v0, .not_found			# If terminator is reached, string is not found
	nop
	
	b		.scan_start
	nop

.not_found:
	
	b		.quit
	move	$v0, $0
	
.found:

	lw		$v0, 12($sp)			# Return address of string match
	
.quit:

	lw		$ra, 0($sp)
	lw		$s0, 4($sp)
	lw		$s1, 8($sp)
	jr		$ra
	addiu	$sp, 24
	
	
