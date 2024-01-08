# PSn00bSDK MDEC library (GTE-accelerated VLC decompressor)
# (C) 2022-2023 spicyjpeg - MPL licensed
#
# TODO: reduce the size of the v3 DC coefficient decoder; currently the code is
# duplicated for each block type, but it can probably be shortened with no
# performance impact...

.include "gtereg.inc"

.set noreorder
.set noat

.set value,			$v0
.set length,		$v1
.set ctx,			$a0
.set output,		$a1
.set max_size,		$a2
.set input,			$a3
.set temp,			$t0
.set window,		$t1
.set next_window,	$t2
.set remaining,		$t3
.set is_v3,			$t4
.set bit_offset,	$t5
.set block_index,	$t6
.set coeff_index,	$t7
.set quant_scale,	$s0
.set last_y,		$s1
.set last_cr,		$s2
.set last_cb,		$s3
.set huffman_table,	$t8
.set ac_jump_area,	$t9

.set VLC_Context_input,			0x0
.set VLC_Context_window,		0x4
.set VLC_Context_next_window,	0x8
.set VLC_Context_remaining,		0xc
.set VLC_Context_is_v3,			0x10
.set VLC_Context_bit_offset,	0x11
.set VLC_Context_block_index,	0x12
.set VLC_Context_coeff_index,	0x13
.set VLC_Context_quant_scale,	0x14
.set VLC_Context_last_y,		0x16
.set VLC_Context_last_cr,		0x18
.set VLC_Context_last_cb,		0x1a

.set VLC_Table_ac0,		0x0
.set VLC_Table_ac2,		0x4
.set VLC_Table_ac3,		0x24
.set VLC_Table_ac4,		0x124
.set VLC_Table_ac5,		0x134
.set VLC_Table_ac7,		0x144
.set VLC_Table_ac8,		0x164
.set VLC_Table_ac9,		0x1a4
.set VLC_Table_ac10,	0x1e4
.set VLC_Table_ac11,	0x224
.set VLC_Table_ac12,	0x264
.set VLC_Table_dc,		0x2a4
.set VLC_Table_dc_len,	0x324

.section .text.DecDCTvlcStart, "ax", @progbits
.global DecDCTvlcStart
.type DecDCTvlcStart, @function

DecDCTvlcStart:
	addiu $sp, -16
	sw    $s0,  0($sp)
	sw    $s1,  4($sp)
	sw    $s2,  8($sp)
	sw    $s3, 12($sp)

	# Create a new context on-the-fly without writing it to memory then jump
	# into DecDCTvlcContinue(), skipping context loading.
	lw    window, 8(input) # window = (bs->data[0] << 16) | (bs->data[0] >> 16)
	li    last_y, 0
	srl   temp, window, 16
	sll   window, 16
	or    window, temp

	# next_window = (bs->data[1] << 16) | (bs->data[1] >> 16)
	lw    next_window, 12(input)
	li    last_cr, 0
	srl   temp, next_window, 16
	sll   next_window, 16
	or    next_window, temp

	lhu   remaining, 0(input) # remaining = bs->uncomp_length * 2
	li    last_cb, 0
	sll   remaining, 1

	lw    temp, 4(input) # quant_scale = (bs->quant_scale & 63) << 10
	li    bit_offset, 32
	andi  quant_scale, temp, 63
	sll   quant_scale, 10

	srl   temp, 16 # is_v3 = !(bs->version < 3)
	sltiu is_v3, temp, 3
	xori  is_v3, 1

	li    block_index, 5
	li    coeff_index, 0
	j     _vlc_skip_context_load
	addiu input, 16 # input = &(bs->data[2])

.section .text.DecDCTvlcContinue, "ax", @progbits
.global DecDCTvlcContinue
.type DecDCTvlcContinue, @function

DecDCTvlcContinue:
	addiu $sp, -16
	sw    $s0,  0($sp)
	sw    $s1,  4($sp)
	sw    $s2,  8($sp)
	sw    $s3, 12($sp)

	lw    input, VLC_Context_input(ctx)
	lw    window, VLC_Context_window(ctx)
	lw    next_window, VLC_Context_next_window(ctx)
	lw    remaining, VLC_Context_remaining(ctx)
	lb    is_v3, VLC_Context_is_v3(ctx)
	lb    bit_offset, VLC_Context_bit_offset(ctx)
	lb    block_index, VLC_Context_block_index(ctx)
	lb    coeff_index, VLC_Context_coeff_index(ctx)
	lhu   quant_scale, VLC_Context_quant_scale(ctx)
	lh    last_y, VLC_Context_last_y(ctx)
	lh    last_cr, VLC_Context_last_cr(ctx)
	lh    last_cb, VLC_Context_last_cb(ctx)

_vlc_skip_context_load:
	# Determine how many bytes to output.
	#   if (max_size <= 0) max_size = 0x3fff0000
	#   max_size   = min((max_size - 1) * 2, remaining)
	#   remaining -= max_size
	bgtz  max_size, .Lmax_size_valid
	addiu max_size, -1
	lui   max_size, 0x3fff
.Lmax_size_valid:
	sll   max_size, 1

	subu  remaining, max_size
	bgez  remaining, .Lmax_size_ok
	lui   temp, 0x3800

	addu  max_size, remaining
	li    remaining, 0

.Lmax_size_ok:
	# Write the length of the data that will be decoded to first 4 bytes of the
	# output buffer, which will be then parsed by DecDCTin().
	srl   value, max_size, 1 # output[0] = 0x38000000 | (max_size / 2)
	or    value, temp
	sw    value, 0(output)

	# Obtain the addresses of the lookup table and jump area in advance so that
	# they don't have to be retrieved for each coefficient decoded.
	lw    huffman_table, _vlc_huffman_table
	la    ac_jump_area, .Lac_prefix_01 - 32

	beqz  max_size, .Lstop_processing
	addiu output, 4

.Lprocess_next_code_loop: # while (max_size)
	# This is the "hot" part of the decoder, executed for each code in the
	# bitstream. The first step is to determine if the next code is a DC or AC
	# coefficient; at the same time the GTE is given the task of counting the
	# number of leading zeroes/ones in the code (which takes 2 more cycles).
	mtc2  window, C2_LZCS

	bnez  coeff_index, .Lprocess_ac_coefficient
	addiu coeff_index, 1
	bnez  is_v3, .Lprocess_dc_v3_coefficient
	li    temp, 0x1ff

.Lprocess_dc_v2_coefficient: # if (!coeff_index && !is_v3)
	# The DC coefficient in version 2 frames is not compressed. Value 0x1ff is
	# used to signal the end of the bitstream.
	#   prefix = window >> (32 - 10)
	#   if (prefix == 0x1ff) break
	#   *output = prefix | quant_scale
	srl   value, window, 22
	beq   value, temp, .Lstop_processing
	or    value, quant_scale
	sll   window, 10
	addiu bit_offset, -10

	b     .Lfeed_bitstream
	sh    value, 0(output)

.Lprocess_dc_v3_coefficient: # if (!coeff_index && is_v3)
	# Version 3 DC coefficients are variable-length deltas, prefixed with a
	# Huffman code indicating their length. Since the prefix code is up to 7
	# bits long, it makes sense to decode it with a simple 128-byte lookup
	# table rather than using the GTE. The codes are different for luma and
	# chroma blocks, so each table entry contains the decoded length for both
	# block types (packed as two nibbles). Prefix 111111111 is used to signal
	# the end of the bitstream.
	#   prefix = window >> (32 - 9)
	#   if (prefix == 0x1ff) break
	#   lengths = huffman_table->dc[prefix >> 2]
	srl   length, window, 23
	beq   length, temp, .Lstop_processing
	srl   length, 2
	addu  length, huffman_table

	addiu $at, block_index, -4
	bltz  $at, .Ldc_block_y
	lbu   length, VLC_Table_dc(length)
	beqz  $at, .Ldc_block_cb
	andi  length, 15 # if (block_index >= Cb) dc_length = lengths & 15

.Ldc_block_cr: # if (block_index > Cb)
	# prefix_length = huffman_table->dc_len[dc_length] & 15
	addu  temp, length, huffman_table
	lbu   temp, VLC_Table_dc_len(temp)
	li    $at, 32
	andi  temp, 15

	sllv  window, window, temp
	beqz  length, .Ldc_cr_zero # if (dc_length)
	subu  bit_offset, temp

	subu  $at, length # value = window >> (32 - dc_length)
	srlv  value, window, $at

	# Decode the sign bit, then add the decoded delta to the current value.
	#   if (!(window >> 31)) value -= (1 << dc_length) - 1
	bltz  window, .Ldc_cr_positive
	li    temp, -1
	srlv  temp, temp, $at
	subu  value, temp
.Ldc_cr_positive:
	sll   value, 2 # last_cr = (last_cr + (value << 2)) & 0x3ff
	addu  last_cr, value
	andi  last_cr, 0x3ff

.Ldc_cr_zero:
	or    temp, last_cr, quant_scale # *output = last_cr | quant_scale
	b     .Lupdate_window_dc # update_window(dc_length)
	sh    temp, 0(output)

.Ldc_block_cb: # if (block_index == Cb)
	# prefix_length = huffman_table->dc_len[dc_length] & 15
	addu  temp, length, huffman_table
	lbu   temp, VLC_Table_dc_len(temp)
	li    $at, 32
	andi  temp, 15

	sllv  window, window, temp
	beqz  length, .Ldc_cb_zero # if (dc_length)
	subu  bit_offset, temp

	subu  $at, length # value = window >> (32 - dc_length)
	srlv  value, window, $at

	# Decode the sign bit, then add the decoded delta to the current value.
	#   if (!(window >> 31)) value -= (1 << dc_length) - 1
	bltz  window, .Ldc_cb_positive
	li    temp, -1
	srlv  temp, temp, $at
	subu  value, temp
.Ldc_cb_positive:
	sll   value, 2 # last_cb = (last_cb + (value << 2)) & 0x3ff
	addu  last_cb, value
	andi  last_cb, 0x3ff

.Ldc_cb_zero:
	or    temp, last_cb, quant_scale # *output = last_cb | quant_scale
	b     .Lupdate_window_dc # update_window(dc_length)
	sh    temp, 0(output)

.Ldc_block_y: # if (block_index < Cb)
	nop
	srl   length, 4 # dc_length = lengths >> 4

	# prefix_length = huffman_table->dc_len[dc_length] >> 4
	addu  temp, length, huffman_table
	lbu   temp, VLC_Table_dc_len(temp)
	li    $at, 32
	srl   temp, 4

	sllv  window, window, temp
	beqz  length, .Ldc_y_zero # if (dc_length)
	subu  bit_offset, temp

	sll   temp, last_y, 2
	subu  $at, length # value = window >> (32 - dc_length)
	srlv  value, window, $at

	# Decode the sign bit, then add the decoded delta to the current value.
	#   if (!(window >> 31)) value -= (1 << dc_length) - 1
	bltz  window, .Ldc_y_positive
	li    temp, -1
	srlv  temp, temp, $at
	subu  value, temp
.Ldc_y_positive:
	sll   value, 2 # last_y = (last_y + (value << 2)) & 0x3ff
	addu  last_y, value
	andi  last_y, 0x3ff

.Ldc_y_zero:
	or    temp, last_y, quant_scale # *output = last_y | quant_scale
	b     .Lupdate_window_dc # update_window(dc_length)
	sh    temp, 0(output)

.Lprocess_ac_coefficient: # if (coeff_index)
	# Check whether the prefix code is 10 or 11 (i.e. if it starts with 1). If
	# not, retrieve the number of leading zeroes from the GTE and use it as an
	# index into the jump area. Each block in the area is 8 instructions long
	# and handles decoding a specific prefix.
	mfc2  temp, C2_LZCR

	bltz  window, .Lac_prefix_1 # if (!(window >> 31))
	addiu $at, temp, -11 # if (prefix > 11) return -1
	bgtz  $at, .Lreturn_error
	sll   temp, 5 # jump_addr = &ac_jump_area[prefix * 8 * sizeof(uint32_t)]
	addu  temp, ac_jump_area
	jr    temp
	nop

.Lreturn_error:
	b     .Lreturn
	li    $v0, -1

.Lac_prefix_1: # if (window >> 31)
	sll   window, 1
	bltz  window, .Lac_prefix_11
	li    temp, 0xfe00

.Lac_prefix_10:
	# Prefix 10 marks the end of a block.
	#   *output = 0xfe00
	#   coeff_index = 0
	#   if (--block_index < Y3) block_index = Cr
	sll   window, 1
	addiu bit_offset, -2
	sh    temp, 0(output)

	addiu block_index, -1
	bgez  block_index, .Lfeed_bitstream
	li    coeff_index, 0
	b     .Lfeed_bitstream
	li    block_index, 5

.Lac_prefix_11:
	# Prefix 11 is followed by a single bit. Note that the 10/11 prefix check
	# already shifts the window by one bit (without updating the bit offset).
	#   index   = ((window >> (32 - 1 - 1)) & 1) * sizeof(uint16_t)
	#   *output = huffman_table->ac0[index]
	srl   value, window, 29
	andi  value, 2
	addu  value, huffman_table
	lhu   value, VLC_Table_ac0(value)
	sll   window, 2
	addiu bit_offset, -3

	b     .Lfeed_bitstream
	sh    value, 0(output)

.Lac_prefix_01:
	# Prefix 01 can be followed by a 2-bit lookup index starting with 1, or a
	# 3-bit lookup index starting with 0. A 32-bit lookup table is used,
	# containing both MDEC codes and lengths.
	#   index   = ((window >> (32 - 2 - 3)) & 7) * sizeof(uint32_t)
	#   *output = huffman_table->ac2[index] & 0xffff
	#   length  = huffman_table->ac2[index] >> 16
	srl   value, window, 25
	andi  value, 28
	addu  value, huffman_table
	lw    value, VLC_Table_ac2(value)

	b     .Lupdate_window_ac # update_window(value >> 16)
	sh    value, 0(output)
	.word 0, 0

.Lac_prefix_001:
	# Prefix 001 can be followed by a 6-bit lookup index starting with 00, or a
	# 3-bit lookup index starting with 01/10/11.
	#   index   = ((window >> (32 - 3 - 6)) & 63) * sizeof(uint32_t)
	#   *output = huffman_table->ac3[index] & 0xffff
	#   length  = huffman_table->ac3[index] >> 16
	srl   value, window, 21
	andi  value, 252
	addu  value, huffman_table
	lw    value, VLC_Table_ac3(value)

	b     .Lupdate_window_ac # update_window(value >> 16)
	sh    value, 0(output)
	.word 0, 0

.Lac_prefix_0001:
	# Prefix 0001 is followed by a 3-bit lookup index.
	#   index   = ((window >> (32 - 4 - 3)) & 7) * sizeof(uint16_t)
	#   *output = huffman_table->ac4[index]
	srl   value, window, 24
	andi  value, 14
	addu  value, huffman_table
	lhu   value, VLC_Table_ac4(value)
	sll   window, 7
	addiu bit_offset, -7

	b     .Lfeed_bitstream
	sh    value, 0(output)

.Lac_prefix_00001:
	# Prefix 00001 is followed by a 3-bit lookup index.
	#   index   = ((window >> (32 - 5 - 3)) & 7) * sizeof(uint16_t)
	#   *output = huffman_table->ac5[index]
	srl   value, window, 23
	andi  value, 14
	addu  value, huffman_table
	lhu   value, VLC_Table_ac5(value)
	sll   window, 8
	addiu bit_offset, -8

	b     .Lfeed_bitstream
	sh    value, 0(output)

.Lac_prefix_000001:
	# Prefix 000001 is an escape code followed by a full 16-bit MDEC value.
	#   *output = window >> (32 - 6 - 16)
	srl   value, window, 10
	sll   window, 22
	addiu bit_offset, -22

	b     .Lfeed_bitstream
	sh    value, 0(output)
	.word 0, 0, 0

.Lac_prefix_0000001:
	# Prefix 0000001 is followed by a 4-bit lookup index.
	#   index   = ((window >> (32 - 7 - 4)) & 15) * sizeof(uint16_t)
	#   *output = huffman_table->ac7[index]
	srl   value, window, 20
	andi  value, 30
	addu  value, huffman_table
	lhu   value, VLC_Table_ac7(value)
	sll   window, 11
	addiu bit_offset, -11

	b     .Lfeed_bitstream
	sh    value, 0(output)

.Lac_prefix_00000001:
	# Prefix 00000001 is followed by a 5-bit lookup index.
	#   index   = ((window >> (32 - 8 - 5)) & 31) * sizeof(uint16_t)
	#   *output = huffman_table->ac8[index]
	srl   value, window, 18
	andi  value, 62
	addu  value, huffman_table
	lhu   value, VLC_Table_ac8(value)
	sll   window, 13
	addiu bit_offset, -13

	b     .Lfeed_bitstream
	sh    value, 0(output)

.Lac_prefix_000000001:
	# Prefix 000000001 is followed by a 5-bit lookup index.
	#   index   = ((window >> (32 - 9 - 5)) & 31) * sizeof(uint16_t)
	#   *output = huffman_table->ac9[index]
	srl   value, window, 17
	andi  value, 62
	addu  value, huffman_table
	lhu   value, VLC_Table_ac9(value)
	sll   window, 14
	addiu bit_offset, -14

	b     .Lfeed_bitstream
	sh    value, 0(output)

.Lac_prefix_0000000001:
	# Prefix 0000000001 is followed by a 5-bit lookup index.
	#   index   = ((window >> (32 - 10 - 5)) & 31) * sizeof(uint16_t)
	#   *output = huffman_table->ac10[index]
	srl   value, window, 16
	andi  value, 62
	addu  value, huffman_table
	lhu   value, VLC_Table_ac10(value)
	sll   window, 15
	addiu bit_offset, -15

	b     .Lfeed_bitstream
	sh    value, 0(output)

.Lac_prefix_00000000001:
	# Prefix 00000000001 is followed by a 5-bit lookup index.
	#   index   = ((window >> (32 - 11 - 5)) & 31) * sizeof(uint16_t)
	#   *output = huffman_table->ac11[index]
	srl   value, window, 15
	andi  value, 62
	addu  value, huffman_table
	lhu   value, VLC_Table_ac11(value)
	sll   window, 16
	addiu bit_offset, -16

	b     .Lfeed_bitstream
	sh    value, 0(output)

.Lac_prefix_000000000001:
	# Prefix 000000000001 is followed by a 5-bit lookup index.
	#   index   = ((window >> (32 - 12 - 5)) & 31) * sizeof(uint16_t)
	#   *output = huffman_table->ac12[index]
	srl   value, window, 14
	andi  value, 62
	addu  value, huffman_table
	lhu   value, VLC_Table_ac12(value)
	sll   window, 17
	addiu bit_offset, -17

	b     .Lfeed_bitstream
	sh    value, 0(output)

.Lupdate_window_ac:
	srl   length, value, 16
.Lupdate_window_dc:
	sllv  window, window, length
	subu  bit_offset, length

.Lfeed_bitstream:
	# Update the window. This makes sure the next iteration of the loop will be
	# able to read up to 32 bits from the bitstream.
	bgez  bit_offset, .Lskip_feeding # if (bit_offset < 0)
	addiu max_size, -1

	subu  temp, $0, bit_offset # window = next_window << (-bit_offset)
	sllv  window, next_window, temp
	lw    next_window, 0(input) # next_window = (*input << 16) | (*input >> 16)
	addiu bit_offset, 32
	srl   temp, next_window, 16
	sll   next_window, 16
	or    next_window, temp
	addiu input, 4

.Lskip_feeding:
	srlv  temp, next_window, bit_offset # window |= next_window >> bit_offset
	or    window, temp

	bnez  max_size, .Lprocess_next_code_loop
	addiu output, 2

.Lstop_processing:
	# If remaining = 0, skip flushing the context, pad the output buffer with
	# end-of-block codes if necessary and return 0. Otherwise flush the context
	# and return 1.
	beqz  remaining, .Lpad_output_buffer
	li    temp, 0xfe00

	sw    input, VLC_Context_input(ctx)
	sw    window, VLC_Context_window(ctx)
	sw    next_window, VLC_Context_next_window(ctx)
	sw    remaining, VLC_Context_remaining(ctx)
	sb    bit_offset, VLC_Context_bit_offset(ctx)
	sb    block_index, VLC_Context_block_index(ctx)
	sb    coeff_index, VLC_Context_coeff_index(ctx)
	sh    last_y, VLC_Context_last_y(ctx)
	sh    last_cr, VLC_Context_last_cr(ctx)
	sh    last_cb, VLC_Context_last_cb(ctx)

	b     .Lreturn
	li    $v0, 1

.Lpad_output_buffer:
	beqz  max_size, .Lreturn
	li    $v0, 0

.Lpad_output_buffer_loop: # while (max_size)
	sh    temp, 0(output)
	addiu max_size, -1
	bnez  max_size, .Lpad_output_buffer_loop
	addiu output, 2

.Lreturn:
	lw    $s0,  0($sp)
	lw    $s1,  4($sp)
	lw    $s2,  8($sp)
	lw    $s3, 12($sp)
	jr    $ra
	addiu $sp, 16
