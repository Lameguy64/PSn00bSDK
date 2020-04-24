.set noreorder

.set POLYG4_tag,	0
.set POLYG4_rgb0,	4
.set POLYG4_xy0,	8
.set POLYG4_rgb1,	12
.set POLYG4_xy1,	16
.set POLYG4_rgb2,	20
.set POLYG4_xy2,	24
.set POLYG4_rgb3,	28
.set POLYG4_xy3,	32
.set POLYG4_len,	36

# a0 - Plasma output
# a1 - Counter
.global genPlasma
.type genPlasma, @function
genPlasma:
				move	$t1, $0	
				
.gn_y_loop:		move	$t0, $0
				
.gn_x_loop:		la		$v0, plasma_sin1
				sll		$v1, $t0, 1
				addu	$v1, $v0
				lh		$v0, 0($v1)
				
				la		$a2, plasma_sin2
				sll		$v1, $t1, 1
				addu	$v1, $a2
				lh		$v1, 0($v1)
				nop
				add		$v0, $v1
				
				add		$v1, $t0, $t1
				add		$v1, $a1
				divu	$v1, 90
				la		$a2, plasma_sin3
				mfhi	$v1
				
				sll		$v1, 1
				addu	$v1, $a2
				lh		$v1, 0($v1)
				nop
				add		$v0, $v1
				
				andi	$v0, 0xff
				
				sb		$v0, 0($a0)
				addu	$a0, 1
				
				addiu	$t0, 1
				blt		$t0, 41, .gn_x_loop
				nop
				
				addiu	$t1, 1
				blt		$t1, 31, .gn_y_loop
				nop
				
				jr		$ra
				nop


# a0 - OT entry
# a1 - Primitive address
# a2 - Plasma map source
.global sortPlasma
.type sortPlasma, @function
sortPlasma:		
				move	$t1, $0
				
.y_loop:		
				move	$t0, $0
				
.x_loop:		
				lbu		$v0, 0($a2)
				addiu	$a2, 1
				la		$a3, plasma_colors
				sll		$v0, 2
				addu	$v0, $a3
				lw		$v0, 0($v0)
				
				lui		$v1, 0x3800
				or		$v0, $v1
				sw		$v0, POLYG4_rgb0($a1)
				
				
				lbu		$v0, 0($a2)
				nop
				sll		$v0, 2
				addu	$v0, $a3
				lw		$v0, 0($v0)
				nop
				sw		$v0, POLYG4_rgb1($a1)
				
				
				lbu		$v0, 40($a2)
				nop
				sll		$v0, 2
				addu	$v0, $a3
				lw		$v0, 0($v0)
				nop
				sw		$v0, POLYG4_rgb2($a1)
				
				
				lbu		$v0, 41($a2)
				nop
				sll		$v0, 2
				addu	$v0, $a3
				lw		$v0, 0($v0)
				nop
				sw		$v0, POLYG4_rgb3($a1)
				
				
				sll		$v0, $t0, 4
				andi	$v0, 0xffff
				sll		$v1, $t1, 20
				or		$v0, $v1
				sw		$v0, POLYG4_xy0($a1)
				
				sll		$v0, $t0, 4
				andi	$v0, 0xffff
				addi	$v0, 16
				sll		$v1, $t1, 20
				or		$v0, $v1
				sw		$v0, POLYG4_xy1($a1)
				
				sll		$v0, $t0, 4
				andi	$v0, 0xffff
				sll		$v1, $t1, 4
				addi	$v1, 16
				sll		$v1, 16
				or		$v0, $v1
				sw		$v0, POLYG4_xy2($a1)
				
				sll		$v0, $t0, 4
				andi	$v0, 0xffff
				addi	$v0, 16
				sll		$v1, $t1, 4
				addi	$v1, 16
				sll		$v1, 16
				or		$v0, $v1
				sw		$v0, POLYG4_xy3($a1)
				
				.set noat

				lui		$v1, 0x0800
				lw		$v0, 0($a0)
				lui		$at, 0xff00
				and		$v1, $at
				lui		$at, 0x00ff
				or		$at, 0xffff
				and		$v0, $at
				or		$v1, $v0
				sw		$v1, 0($a1)
				lw		$v0, 0($a0)
				and		$a1, $at
				lui		$at, 0xff00
				and		$v0, $at
				or		$v0, $a1
				sw		$v0, 0($a0)
				
				.set at
		
				lui		$v0, 0x8000
				or		$a1, $v0
				addiu	$a1, POLYG4_len
				
				
				addiu	$t0, 1
				blt		$t0, 40, .x_loop
				nop
				
				addiu	$a2, 1
				addiu	$t1, 1
				blt		$t1, 30, .y_loop
				nop
				
				
				jr		$ra
				move	$v0, $a1