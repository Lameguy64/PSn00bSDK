#
# LibPSn00b Example Programs
#
# Drawing Tile-maps with Assembler Routines
# 2022 Meido-Tek Productions / PSn00bSDK Project
#
# Example by John "Lameguy" Wilbert Villamor (Lameguy64)
#
# This file contains the assembler routine DrawTiles which can be called from
# a C or C++ compiled module. The routine makes use of constants and assembler
# macros written in the GNU GAS syntax.
#
# Assembler routines called from C-language modules can freely use registers
# $v0-$v1, $at, $a0-$a3, $t0-$t9 without preserving through stack. Registers
# $s0-$s9, $gp and $fp must be preserved through stack before returning and
# registers $k0-$k1 should not be used for obvious reasons (kernel registers).
# $sp and $ra is used as stack pointer and return address respectively.
# $0 or $zero is constantly zero.
#
# A C caller always passes arguments as 32-bit values on registers $a0 to $a3
# regardless of the data type specified in the function's C declaration.
# Additional arguments are stored in the stack 16 bytes relative to the stack
# pointer ($sp). To get around the stack being modified in different parts of
# a larger routine the stack pointer would be copied to the frame pointer ($fp)
# and the previous value of the frame pointer would be pushed into stack. This
# way additional arguments may be read anywhere starting from $fp+20 instead.
#
.set noreorder					# Disable GAS' annoying nop insertion

.equ db, 1						# Constants for "emulating" SNASM style structs
.equ dh, 2
.equ dw, 4

#
# TILEDEF struct
#
	rs=0						# rs is used to emulate SNASM style structs
.equ TILEDEF_uv			, rs	# Tile texture coordinate
	rs=rs+dh
.equ TILEDEF_clut		, rs	# Tile CLUT
	rs=rs+dh
.equ TILEDEF_pad		, rs	# Padding
	rs=rs+dh
.equ TILEDEF_tpage		, rs	# Tile tpage
	rs=rs+dh
.equ TILEDEF_len		, rs	# Entry length

#
# TILEINFO struct (to use as offsets)
#
	rs=0
.equ TILEINFO_window_x	, rs	# \
	rs=rs+dh					#  - Window coordinates
.equ TILEINFO_window_y	, rs	# /
	rs=rs+dh
.equ TILEINFO_window_w	, rs	# \
	rs=rs+dh					#  - Window size
.equ TILEINFO_window_h	, rs	# /
	rs=rs+dh
.equ TILEINFO_tiles		, rs	# Pointer to TILEDEF entries
	rs=rs+dw
.equ TILEINFO_mapdata	, rs	# Pointer to map data
	rs=rs+dw
.equ TILEINFO_map_w		, rs	# Map width in tile units
	rs=rs+dh
.equ TILEINFO_map_h		, rs	# Map height in tile units
	rs=rs+dh

#
# TILEPKT struct
#
	rs=0
.equ TILEPKT_tag		, rs	# Primitive tag
	rs=rs+dw
.equ TILEPKT_tpage		, rs	# tpage packet
	rs=rs+dw
.equ TILEPKT_rgbc		, rs	# Tile color
	rs=rs+dw
.equ TILEPKT_x			, rs	# Tile screen coordinates
	rs=rs+dh
.equ TILEPKT_y			, rs
	rs=rs+dh
.equ TILEPKT_uv			, rs	# Tile texture coordinates
	rs=rs+dh
.equ TILEPKT_clut		, rs	# Tile CLUT
	rs=rs+dh
.equ TILEPKT_len		, rs	# Packet length

# addprim Macro
#
# Registers a primitive to a ordering table entry.
#
# Arguments:
#	ot		- Register name of pointer to ordering table entry
#	pri		- Pointer to a primitive packet
#	len		- Size of packet in long words (specify as 0xnn00, ie. 0x2000)
#
# Destroys:
#	at, v0, v1
#
.macro addprim			ot,pri,len
	.set noat
	lw		$v0, 0(\ot)				# Get OT entry
	lui		$at, 0x00ff				# Mask out the packet length field
	or		$at, 0xffff
	and		$v0, $at
	lui		$v1, \len				# Merge packet length
	or		$v1, $v0
	sw		$v1, 0(\pri)			# Store updated OT entry to packet
	lw		$v0, 0(\ot)				# Get OT entry
	and		\pri, $at				# Mask out last 8-bits of packet address
	lui		$at, 0xff00				# Mask out OT entry's address
	and		$v0, $at
	or		$v0, \pri				# Merge packet address to OT entry
	sw		$v0, 0(\ot)				# Store updated OT entry
	.set at
.endm

#
# Start of text section
#
.section .text

# DrawTiles Function
#
# Renders a tilemap by generating TILEPKT primitives (combined SPRT_16 and
# DR_TPAGE primitives) and registering it to the specified ordering table.
# The drawing region, tile definitions and the tilemap are specified through
# a TILEINFO struct.
#
# C Declaration:
#   extern u_char *DrawTiles(int scroll_x, int scroll_y,
#		TILEINFO *info, long *ot, u_char *pri); 
#
# Arguments:
#	scroll_x - X scrolling offset of tile-map
#	scroll_y - Y scrolling offset of tile-map
#	info	 - Pointer to a TILEINFO struct
#	ot		 - Pointer to a ordering table entry
#	pri		 - Pointer to next primitive (placed in stack)
#
# Returns:
#	New next primitive pointer value.
#
.global DrawTiles							# Declare symbol as global
.type DrawTiles, @function					# Declare it as a function
DrawTiles:									# Symbol label of function

	addiu	$sp, -4							# Push frame pointer (fp) to stack
	sw		$fp, 0($sp)
	move	$fp, $sp						# Copy stack pointer (sp) to fp
	
	# Register reference:
	#
	# t0 - Packet address
	# t1 - Map data address
	# t2 - Tile X offset
	# t3 - Tile Y offset
	# t4 - Tile X coordinate backup
	# t5 - Tile X loop counter
	# t6 - Number of tiles to sort per row
	# t7 - Number of tile rows to sort
	
	lhu		$t6, TILEINFO_window_w($a2)		# Calculate size of window in tiles
	lhu		$t7, TILEINFO_window_h($a2)
	addi	$t6, 15							# So the result will be rounded-up
	addi	$t7, 15
	srl		$t6, 4							# Effectively divide by 16
	srl		$t7, 4
	
	lw		$t0, 20($sp)					# Obtain next primitive pointer
	
	srl		$t2, $a0, 4						# Compute map offset in tile units
	srl		$t3, $a1, 4
	
	bgez	$a0, .Lno_neg_clip_X			# Negative X clip test
	sub		$v0, $0 , $a0
	move	$t2, $0							# Force tile offset to zero
	add		$v0, 15							# Reduce number of tile columns
	srl		$v0, 4
	sub		$t6, $v0
.Lno_neg_clip_X:
	lhu		$v1, TILEINFO_map_w($a2)		# Positive X clip test
	add		$v0, $t2, $t6
	addi	$v0, 1
	blt		$v0, $v1, .Lno_pos_clip_X
	nop
	sub		$v0, $v1						# Compute how many tiles to clip
	sub		$t6, $v0						# Reduce number of tile columns
.Lno_pos_clip_X:
	bgez	$a1, .Lno_neg_clip_Y			# Negative Y clip test
	sub		$v0, $0 , $a1
	move	$t3, $0
	add		$v0, 15
	srl		$v0, 4
	sub		$t7, $v0
.Lno_neg_clip_Y:
	lhu		$v1, TILEINFO_map_h($a2)		# Positive Y clip test
	add		$v0, $t3, $t7
	addi	$v0, 1
	blt		$v0, $v1, .Lno_pos_clip_Y
	nop
	sub		$v0, $v1
	sub		$t7, $v0
.Lno_pos_clip_Y:
	bltz	$t6, .Lno_draw					# Exit when no tiles to draw
	nop
	bltz	$t7, .Lno_draw
	nop
	
	lh		$v0, TILEINFO_window_x($a2)		# Compute pixel coordinates for
	sub		$a0, $v0, $a0					# tiles based on the scroll offset
	sll		$v0, $t2, 4
	add		$a0, $v0
	lh		$v0, TILEINFO_window_y($a2)
	sub		$a1, $v0, $a1
	sll		$v0, $t3, 4
	add		$a1, $v0
	move	$t4, $a0
	sll		$t2, 1

.Lloop_y:									# Begin of tile row loop
	lhu		$v0, TILEINFO_map_w($a2)		# Get width of tilemap
	move	$t5, $t6						# n tiles to draw row
	mult	$t3, $v0						# Multiply Y offset by map width
	lw		$t1, TILEINFO_mapdata($a2)		# Get tilemap address
	nop
	addu	$t1, $t2						# Add X offset to address
	mflo	$v0								# Get Y offset result
	sll		$v0, 1							# Multiply by two
	addu	$t1, $v0						# Add to tile address

.Lloop_x:									# Begin of tile column loop
	lhu		$v1, 0($t1)						# Load tile index
	addiu	$t1, 2							# Advance to next tile
	beq		$v1, 0xFFFF, .Lskip_tile		# Skip tile if index is 0xFFFF
	nop
	lw		$v0, TILEINFO_tiles($a2)		# Get pointer to TILEDEF entries
	sll		$v1, 3							# Multiply by 8 (size of TILEDEFs)
	addu	$v1, $v0						# Adjust to tiledefs pointer
	lw		$v0, TILEDEF_uv($v1)			# Obtain UV+CLUT
	lhu		$v1, TILEDEF_tpage($v1)			# Obtain tpage
	sw		$v0, TILEPKT_uv($t0)			# Start constructing packet
	lui		$v0, 0xE100						# tpage packet code
	or		$v0, $v1						# Merge tpage bits
	sw		$v0, TILEPKT_tpage($t0)
	sh		$a0, TILEPKT_x($t0)				# Set tile screen coords
	sh		$a1, TILEPKT_y($t0)
	li		$v0, 0x7C7F7F7F					# Packet code and color
	sw		$v0, TILEPKT_rgbc($t0)
	addprim	$a3, $t0, 0x0400				# Register to OT
	addiu	$t0, TILEPKT_len				# Advance packet pointer
.Lskip_tile:
	addi	$t5, -1							# Decrement and continue iterating
	bgez	$t5, .Lloop_x					# if non-zero
	addiu	$a0, 16							# Advance X tile coordinate
	
	move	$a0, $t4						# Restore tile X coordinate
	addi	$t3, 1							# Increment Y offset
	addi	$t7, -1							# Decrement and continue iterating
	bgez	$t7, .Lloop_y					# if non-zero
	addiu	$a1, 16							# Advance Y tile coordinate
	
.Lno_draw:

	move	$v0, $t0						# Set packet pointer as return value
	
	lw		$fp, 0($sp)						# Restore frame pointer and return
	jr		$ra
	addiu	$sp, 4
	# DrawTiles
	