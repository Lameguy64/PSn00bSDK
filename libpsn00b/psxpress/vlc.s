# PSn00bSDK MDEC library (GTE-accelerated VLC decompressor)
# (C) 2022 spicyjpeg - MPL licensed
#
# Register map:
# - $a0 = ctx
# - $a1 = output
# - $a2 = max_size
# - $a3 = input
# - $t0 = window
# - $t1 = next_window
# - $t2 = remaining
# - $t3 = quant_scale
# - $t4 = is_v3
# - $t5 = bit_offset
# - $t6 = block_index
# - $t7 = coeff_index
# - $t8 = _vlc_huffman_table
# - $t9 = &ac_jump_area

.set noreorder

.set VLC_Context_input,			0
.set VLC_Context_window,		4
.set VLC_Context_next_window,	8
.set VLC_Context_remaining,		12
.set VLC_Context_quant_scale,	16
.set VLC_Context_is_v3,			18
.set VLC_Context_bit_offset,	19
.set VLC_Context_block_index,	20
.set VLC_Context_coeff_index,	21

.set DECDCTTAB_lut0,	0
.set DECDCTTAB_lut2,	4
.set DECDCTTAB_lut3,	36
.set DECDCTTAB_lut4,	292
.set DECDCTTAB_lut5,	308
.set DECDCTTAB_lut7,	324
.set DECDCTTAB_lut8,	356
.set DECDCTTAB_lut9,	420
.set DECDCTTAB_lut10,	484
.set DECDCTTAB_lut11,	548
.set DECDCTTAB_lut12,	612

.section .text.DecDCTvlcStart
.global DecDCTvlcStart
.type DecDCTvlcStart, @function
DecDCTvlcStart:
	# Create a new context on-the-fly without writing it to memory then jump
	# into DecDCTvlcContinue(), skipping context loading.
	lw    $t0, 8($a3) # window = (bs->data[0] << 16) | (bs->data[0] >> 16)
	nop
	srl   $v0, $t0, 16
	sll   $t0, 16

	lw    $t1, 12($a3) # next_window = (bs->data[1] << 16) | (bs->data[1] >> 16)
	or    $t0, $v0
	srl   $v0, $t1, 16
	sll   $t1, 16

	lhu   $t2, 0($a3) # remaining = bs->uncomp_length * 2
	or    $t1, $v0

	lhu   $t3, 4($a3) # quant_scale = (bs->quant_scale & 63) << 10
	sll   $t2, 1
	andi  $t3, 63

	lhu   $t4, 6($a3) # is_v3 = !(bs->version < 3)
	sll   $t3, 10
	sltiu $t4, $t4, 3
	xori  $t4, 1

	li    $t5, 32 # bit_offset = 32
	li    $t6, 5 # block_index = 5
	li    $t7, 0 # coeff_index = 0
	j     _vlc_skip_context_load
	addiu $a3, 16 # input = &(bs->data[2])

.section .text.DecDCTvlcContinue
.global DecDCTvlcContinue
.type DecDCTvlcContinue, @function
DecDCTvlcContinue:
	lw    $a3, VLC_Context_input($a0)
	lw    $t0, VLC_Context_window($a0)
	lw    $t1, VLC_Context_next_window($a0)
	lw    $t2, VLC_Context_remaining($a0)
	lhu   $t3, VLC_Context_quant_scale($a0)
	lb    $t4, VLC_Context_is_v3($a0)
	lb    $t5, VLC_Context_bit_offset($a0)
	lb    $t6, VLC_Context_block_index($a0)
	lb    $t7, VLC_Context_coeff_index($a0)

_vlc_skip_context_load:
	# Determine how many bytes to output. This whole block of code basically
	# does this:
	#   max_size   = min((max_size - 1) * 2, remaining)
	#   remaining -= max_size
	bgtz  $a2, .Lmax_size_valid # if (max_size <= 0) max_size = 0x7ffe0000
	addiu $a2, -1 # else max_size = (max_size - 1) * 2
	lui   $a2, 0x3fff
.Lmax_size_valid:
	sll   $a2, 1

	blt   $a2, $t2, .Lmax_size_ok # if (max_size > remaining) max_size = remaining
	lui   $v1, 0x3800
	move  $a2, $t2
.Lmax_size_ok:
	subu  $t2, $a2 # remaining -= max_size

	# Write the length of the data that will be decoded to first 4 bytes of the
	# output buffer, which will be then parsed by DecDCTin().
	srl   $v0, $a2, 1 # output[0] = 0x38000000 | (max_size / 2)
	or    $v0, $v1
	sw    $v0, 0($a1)

	# Obtain the addresses of the lookup table and jump area in advance so that
	# they don't have to be retrieved for each coefficient decoded.
	lw    $t8, _vlc_huffman_table
	la    $t9, .Lac_prefix_10

	beqz  $a2, .Lstop_processing
	addiu $a1, 4 # output = (uint16_t *) &output[1]

.Lprocess_next_code_loop: # while (max_size)
	# This is the "hot" part of the decoder, executed for each code in the
	# bitstream. The first step is to determine if the next code is a DC or AC
	# coefficient.
	bnez  $t7, .Lprocess_ac_coefficient
	addiu $t7, 1 # coeff_index++
	bnez  $t4, .Lprocess_dc_v3_coefficient
	li    $v1, 0x01ff

.Lprocess_dc_v2_coefficient: # if (!coeff_index && !is_v3)
	# The DC coefficient in version 2 frames is not compressed. Value 0x1ff is
	# used to signal the end of the bitstream.
	srl   $v0, $t0, 22 # prefix = (window >> (32 - 10))
	beq   $v0, $v1, .Lstop_processing # if (prefix == 0x1ff) break
	or    $v0, $t3 # *output = prefix | quant_scale
	sll   $t0, 10 # window <<= 10
	b     .Lwrite_value
	addiu $t5, -10 # bit_offset -= 10

.Lprocess_dc_v3_coefficient: # if (!coeff_index && is_v3)
	# TODO: version 3 is currently not supported.
	jr    $ra
	li    $v0, -1

.Lprocess_ac_coefficient: # if (coeff_index)
	# Start counting the number of leading zeroes/ones using the GTE. This
	# takes 2 more cycles.
	mtc2  $t0, $30

	# Check whether the prefix code is one of the shorter, more common ones.
	srl   $v0, $t0, 30
	li    $v1, 3
	beq   $v0, $v1, .Lac_prefix_11
	li    $v1, 2
	beq   $v0, $v1, .Lac_prefix_10
	li    $v1, 1
	beq   $v0, $v1, .Lac_prefix_01
	nop

	# If the code is longer, retrieve the number of leading zeroes from the GTE
	# and use it as an index into the jump area. Each block in the area is 8
	# instructions long and handles decoding a specific prefix.
	mfc2  $v0, $31
	li    $v1, 11
	bgt   $v0, $v1, .Lreturn_error # if (prefix > 11) return -1
	sll   $v0, 5 # jump_addr = &ac_jump_area[prefix * 8 * sizeof(u32)]
	addu  $v0, $t9
	jr    $v0
	nop

.Lreturn_error:
	jr    $ra
	li    $v0, -1

.Lac_prefix_11:
	# Prefix 11 is followed by a single bit.
	srl   $v0, $t0, 28 # index = ((window >> (32 - 2 - 1)) & 1) * sizeof(u16)
	andi  $v0, 2
	addu  $v0, $t8 # value = table->lut0[index]
	lhu   $v0, DECDCTTAB_lut0($v0)
	sll   $t0, 3 # window <<= 3
	b     .Lwrite_value
	addiu $t5, -3 # bit_offset -= 3
	#.word 0

.Lac_prefix_10:
	# Prefix 10 marks the end of a block.
	li    $v0, 0xfe00 # value = 0xfe00
	sll   $t0, 2 # window <<= 2
	addiu $t5, -2 # bit_offset -= 2
	addiu $t6, -1 # block_index--
	bgez  $t6, .Lwrite_value
	li    $t7, 0 # coeff_index = 0
	b     .Lwrite_value
	li    $t6, 5 # if (block_index < 0) block_index = 5

.Lac_prefix_01:
	# Prefix 01 can be followed by a 2-bit lookup index starting with 1, or a
	# 3-bit lookup index starting with 0. A 32-bit lookup table is used,
	# containing both MDEC codes and lengths.
	srl   $v0, $t0, 25 # index = ((window >> (32 - 2 - 3)) & 7) * sizeof(u32)
	andi  $v0, 28
	addu  $v0, $t8 # value = table->lut2[index]
	lw    $v0, DECDCTTAB_lut2($v0)
	b     .Lupdate_window_and_write
	srl   $v1, $v0, 16 # length = value >> 16
	.word 0, 0

.Lac_prefix_001:
	# Prefix 001 can be followed by a 6-bit lookup index starting with 00, or a
	# 3-bit lookup index starting with 01/10/11.
	srl   $v0, $t0, 21 # index = ((window >> (32 - 3 - 6)) & 63) * sizeof(u32)
	andi  $v0, 252
	addu  $v0, $t8 # value = table->lut3[index]
	lw    $v0, DECDCTTAB_lut3($v0)
	b     .Lupdate_window_and_write
	srl   $v1, $v0, 16 # length = value >> 16
	.word 0, 0

.Lac_prefix_0001:
	# Prefix 0001 is followed by a 3-bit lookup index.
	srl   $v0, $t0, 24 # index = ((window >> (32 - 4 - 3)) & 7) * sizeof(u16)
	andi  $v0, 14
	addu  $v0, $t8 # value = table->lut4[index]
	lhu   $v0, DECDCTTAB_lut4($v0)
	sll   $t0, 7 # window <<= 4 + 3
	b     .Lwrite_value
	addiu $t5, -7 # bit_offset -= 4 + 3
	.word 0

.Lac_prefix_00001:
	# Prefix 00001 is followed by a 3-bit lookup index.
	srl   $v0, $t0, 23 # index = ((window >> (32 - 5 - 3)) & 7) * sizeof(u16)
	andi  $v0, 14
	addu  $v0, $t8 # value = table->lut5[index]
	lhu   $v0, DECDCTTAB_lut5($v0)
	sll   $t0, 8 # window <<= 5 + 3
	b     .Lwrite_value
	addiu $t5, -8 # bit_offset -= 5 + 3
	.word 0

.Lac_prefix_000001:
	# Prefix 000001 is an escape code followed by a full 16-bit MDEC value.
	srl   $v0, $t0, 10 # value = window >> (32 - 6 - 16)
	sll   $t0, 22 # window <<= 6 + 16
	b     .Lwrite_value
	addiu $t5, -22 # bit_offset -= 6 + 16
	.word 0, 0, 0, 0

.Lac_prefix_0000001:
	# Prefix 0000001 is followed by a 4-bit lookup index.
	srl   $v0, $t0, 20 # index = ((window >> (32 - 7 - 4)) & 15) * sizeof(u16)
	andi  $v0, 30
	addu  $v0, $t8 # value = table->lut7[index]
	lhu   $v0, DECDCTTAB_lut7($v0)
	sll   $t0, 11 # window <<= 7 + 4
	b     .Lwrite_value
	addiu $t5, -11 # bit_offset -= 7 + 4
	.word 0

.Lac_prefix_00000001:
	# Prefix 00000001 is followed by a 5-bit lookup index.
	srl   $v0, $t0, 18 # index = ((window >> (32 - 8 - 5)) & 31) * sizeof(u16)
	andi  $v0, 62
	addu  $v0, $t8 # value = table->lut8[index]
	lhu   $v0, DECDCTTAB_lut8($v0)
	sll   $t0, 13 # window <<= 8 + 5
	b     .Lwrite_value
	addiu $t5, -13 # bit_offset -= 8 + 5
	.word 0

.Lac_prefix_000000001:
	# Prefix 000000001 is followed by a 5-bit lookup index.
	srl   $v0, $t0, 17 # index = ((window >> (32 - 9 - 5)) & 31) * sizeof(u16)
	andi  $v0, 62
	addu  $v0, $t8 # value = table->lut9[index]
	lhu   $v0, DECDCTTAB_lut9($v0)
	sll   $t0, 14 # window <<= 9 + 5
	b     .Lwrite_value
	addiu $t5, -14 # bit_offset -= 9 + 5
	.word 0

.Lac_prefix_0000000001:
	# Prefix 0000000001 is followed by a 5-bit lookup index.
	srl   $v0, $t0, 16 # index = ((window >> (32 - 10 - 5)) & 31) * sizeof(u16)
	andi  $v0, 62
	addu  $v0, $t8 # value = table->lut10[index]
	lhu   $v0, DECDCTTAB_lut10($v0)
	sll   $t0, 15 # window <<= 10 + 5
	b     .Lwrite_value
	addiu $t5, -15 # bit_offset -= 10 + 5
	.word 0

.Lac_prefix_00000000001:
	# Prefix 00000000001 is followed by a 5-bit lookup index.
	srl   $v0, $t0, 15 # index = ((window >> (32 - 11 - 5)) & 31) * sizeof(u16)
	andi  $v0, 62
	addu  $v0, $t8 # value = table->lut11[index]
	lhu   $v0, DECDCTTAB_lut11($v0)
	sll   $t0, 16 # window <<= 11 + 5
	b     .Lwrite_value
	addiu $t5, -16 # bit_offset -= 11 + 5
	.word 0

.Lac_prefix_000000000001:
	# Prefix 000000000001 is followed by a 5-bit lookup index.
	srl   $v0, $t0, 14 # index = ((window >> (32 - 12 - 5)) & 31) * sizeof(u16)
	andi  $v0, 62
	addu  $v0, $t8 # value = table->lut12[index]
	lhu   $v0, DECDCTTAB_lut12($v0)
	sll   $t0, 17 # window <<= 12 + 5
	b     .Lwrite_value
	addiu $t5, -17 # bit_offset -= 12 + 5
	.word 0

.Lupdate_window_and_write:
	sllv  $t0, $t0, $v1 # window <<= length
	subu  $t5, $v1 # bit_offset -= length
.Lwrite_value:
	sh    $v0, 0($a1)
.Lfeed_bitstream:
	# Update the window. This makes sure the next iteration of the loop will be
	# able to read up to 32 bits from the bitstream.
	bgez  $t5, .Lskip_feeding # if (bit_offset < 0)
	addiu $a2, -1 # max_size--

	subu  $v0, $0, $t5 # window = next_window << (-bit_offset)
	sllv  $t0, $t1, $v0
	lw    $t1, 0($a3) # next_window = (*input << 16) | (*input >> 16)
	addiu $t5, 32 # bit_offset += 32
	srl   $v0, $t1, 16
	sll   $t1, 16
	or    $t1, $v0
	addiu $a3, 4 # input++

.Lskip_feeding:
	srlv  $v0, $t1, $t5 # window |= next_window >> bit_offset
	or    $t0, $v0

	bnez  $a2, .Lprocess_next_code_loop
	addiu $a1, 2 # output++

.Lstop_processing:
	# If remaining = 0, skip flushing the context, pad the output buffer with
	# end-of-block codes if necessary and return 0. Otherwise flush the context
	# and return 1.
	beqz  $t2, .Lpad_output_buffer
	nop

	sw    $a3, VLC_Context_input($a0)
	sw    $t0, VLC_Context_window($a0)
	sw    $t1, VLC_Context_next_window($a0)
	sw    $t2, VLC_Context_remaining($a0)
	sh    $t3, VLC_Context_quant_scale($a0)
	sb    $t4, VLC_Context_is_v3($a0)
	sb    $t5, VLC_Context_bit_offset($a0)
	sb    $t6, VLC_Context_block_index($a0)
	sb    $t7, VLC_Context_coeff_index($a0)

	jr    $ra
	li    $v0, 1

.Lpad_output_buffer:
	beqz  $a2, .Lreturn_zero
	li    $v0, 0xfe00
.Lpad_output_buffer_loop: # while (max_size)
	sh    $v0, 0($a1) # *output = 0xfe00
	addiu $a2, -1 # max_size--
	bnez  $a2, .Lpad_output_buffer_loop
	addiu $a1, 2 # output++

.Lreturn_zero:
	jr    $ra
	li    $v0, 0
