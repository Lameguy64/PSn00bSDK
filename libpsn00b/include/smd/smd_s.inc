# PSn00bSDK .SMD model parser library
# (C) 2019-2023 Lameguy64, spicyjpeg - MPL licensed

.set OT_ADDR,			0
.set OT_LEN,			4
.set OT_ZDIV,			8
.set OT_ZOFF,			10

.set SMD_HEAD_ID,		0
.set SMD_HEAD_FLAG,		4
.set SMD_HEAD_NVERTS,	6
.set SMD_HEAD_NNORMS,	8
.set SMD_HEAD_NPRIMS,	10
.set SMD_HEAD_PVERTS,	12
.set SMD_HEAD_PNORMS,	16
.set SMD_HEAD_PPRIMS,	20
.set SMD_HEAD_SIZE,		24

.set POLYF3_tag,		0
.set POLYF3_tpage,		4
.set POLYF3_rgbc,		8
.set POLYF3_xy0,		12
.set POLYF3_xy1,		16
.set POLYF3_xy2,		20
.set POLYF3_len,		24

.set POLYFT3_tag,		0
.set POLYFT3_rgbc,		4
.set POLYFT3_xy0,		8
.set POLYFT3_uv0,		12
.set POLYFT3_clut,		14
.set POLYFT3_xy1,		16
.set POLYFT3_uv1,		20
.set POLYFT3_tpage,		22
.set POLYFT3_xy2,		24
.set POLYFT3_uv2,		28
.set POLYFT3_pad,		30
.set POLYFT3_len,		32

.set POLYG3_tag,		0
.set POLYG3_tpage,		4
.set POLYG3_rgbc0,		8
.set POLYG3_xy0,		12
.set POLYG3_rgbc1,		16
.set POLYG3_xy1,		20
.set POLYG3_rgbc2,		24
.set POLYG3_xy2,		28
.set POLYG3_len,		32

.set POLYGT3_tag,		0
.set POLYGT3_rgbc0,		4
.set POLYGT3_xy0,		8
.set POLYGT3_uv0,		12
.set POLYGT3_clut,		14
.set POLYGT3_rgbc1,		16
.set POLYGT3_xy1,		20
.set POLYGT3_uv1,		24
.set POLYGT3_tpage,		26
.set POLYGT3_rgbc2,		28
.set POLYGT3_xy2,		32
.set POLYGT3_uv2,		36
.set POLYGT3_pad,		38
.set POLYGT3_len,		40

.set POLYF4_tag,		0
.set POLYF4_tpage,		4
.set POLYF4_rgbc,		8
.set POLYF4_xy0,		12
.set POLYF4_xy1,		16
.set POLYF4_xy2,		20
.set POLYF4_xy3,		24
.set POLYF4_len,		28

.set POLYFT4_tag,		0
.set POLYFT4_rgbc,		4
.set POLYFT4_xy0,		8
.set POLYFT4_uv0,		12
.set POLYFT4_clut,		14
.set POLYFT4_xy1,		16
.set POLYFT4_uv1,		20
.set POLYFT4_tpage,		22
.set POLYFT4_xy2,		24
.set POLYFT4_uv2,		28
.set POLYFT4_pad0,		30
.set POLYFT4_xy3,		32
.set POLYFT4_uv3,		36
.set POLYFT4_pad1,		38
.set POLYFT4_len,		40

.set POLYG4_tag,		0
.set POLYG4_tpage,		4
.set POLYG4_rgbc0,		8
.set POLYG4_xy0,		12
.set POLYG4_rgbc1,		16
.set POLYG4_xy1,		20
.set POLYG4_rgbc2,		24
.set POLYG4_xy2,		28
.set POLYG4_rgbc3,		32
.set POLYG4_xy3,		36
.set POLYG4_len,		40

.set POLYGT4_tag,		0
.set POLYGT4_rgbc0,		4
.set POLYGT4_xy0,		8
.set POLYGT4_uv0,		12
.set POLYGT4_clut,		14
.set POLYGT4_rgbc1,		16
.set POLYGT4_xy1,		20
.set POLYGT4_uv1,		24
.set POLYGT4_tpage,		26
.set POLYGT4_rgbc2,		28
.set POLYGT4_xy2,		32
.set POLYGT4_uv2,		36
.set POLYGT4_pad0,		38
.set POLYGT4_rgbc3,		40
.set POLYGT4_xy3,		44
.set POLYGT4_uv3,		48
.set POLYGT4_pad1,		50
.set POLYGT4_len,		52

.set CLIP_LEFT,		1
.set CLIP_RIGHT,	2
.set CLIP_TOP,		4
.set CLIP_BOTTOM,	8


# Clip routine macros, based on Cohen-Sutherland line clipping algorithm
# but only with the off-screen dectection logic extended for polygons

.macro ClipTestTri

	# X0 clip test
	mfc2	$t7, C2_SXY0
	sll		$v1, $t8, 16
	sra		$v1, 16
	sll		$v0, $t7, 16
	sra		$v0, 16
	bge		$v0, $v1, .no_clip_l_x0
	move	$s0, $0
	ori		$s0, CLIP_LEFT
.no_clip_l_x0:
	sll		$v1, $t9, 16
	sra		$v1, 16
	ble		$v0, $v1, .no_clip_r_x0
	nop
	ori		$s0, CLIP_RIGHT
.no_clip_r_x0:
	# Y0 clip test
	sra		$v0, $t7, 16
	sra		$v1, $t8, 16
	bge		$v0, $v1, .no_clip_t_y0
	nop
	ori		$s0, CLIP_TOP
.no_clip_t_y0:
	sra		$v1, $t9, 16
	ble		$v0, $v1, .no_clip_b_y0
	nop
	ori		$s0, CLIP_BOTTOM
.no_clip_b_y0:

	# X1 clip test
	mfc2	$t7, C2_SXY1
	sll		$v1, $t8, 16
	sra		$v1, 16
	sll		$v0, $t7, 16
	sra		$v0, 16
	bge		$v0, $v1, .no_clip_l_x1
	move	$s1, $0
	ori		$s1, CLIP_LEFT
.no_clip_l_x1:
	sll		$v1, $t9, 16
	sra		$v1, 16
	ble		$v0, $v1, .no_clip_r_x1
	nop
	ori		$s1, CLIP_RIGHT
.no_clip_r_x1:
	# Y1 clip test
	sra		$v0, $t7, 16
	sra		$v1, $t8, 16
	bge		$v0, $v1, .no_clip_t_y1
	nop
	ori		$s1, CLIP_TOP
.no_clip_t_y1:
	sra		$v1, $t9, 16
	ble		$v0, $v1, .no_clip_b_y1
	nop
	ori		$s1, CLIP_BOTTOM
.no_clip_b_y1:

	# X2 clip test
	mfc2	$t7, C2_SXY2
	sll		$v1, $t8, 16
	sra		$v1, 16
	sll		$v0, $t7, 16
	sra		$v0, 16
	bge		$v0, $v1, .no_clip_l_x2
	move	$s2, $0
	ori		$s2, CLIP_LEFT
.no_clip_l_x2:
	sll		$v1, $t9, 16
	sra		$v1, 16
	ble		$v0, $v1, .no_clip_r_x2
	nop
	ori		$s2, CLIP_RIGHT
.no_clip_r_x2:
	# Y2 clip test
	sra		$v0, $t7, 16
	sra		$v1, $t8, 16
	bge		$v0, $v1, .no_clip_t_y2
	nop
	ori		$s2, CLIP_TOP
.no_clip_t_y2:
	sra		$v1, $t9, 16
	ble		$v0, $v1, .no_clip_b_y2
	nop
	ori		$s2, CLIP_BOTTOM
.no_clip_b_y2:

.endm


.macro ClipTestQuad

	# X0 clip test
	sll		$v0, $t6, 16
	sra		$v0, 16
	sll		$v1, $t8, 16
	sra		$v1, 16
	bge		$v0, $v1, .no_clip_l_x0_q
	move	$s0, $0
	ori		$s0, CLIP_LEFT
.no_clip_l_x0_q:
	sll		$v1, $t9, 16
	sra		$v1, 16
	ble		$v0, $v1, .no_clip_r_x0_q
	nop
	ori		$s0, CLIP_RIGHT
.no_clip_r_x0_q:
	# Y0 clip test
	sra		$v0, $t6, 16
	sra		$v1, $t8, 16
	bge		$v0, $v1, .no_clip_t_y0_q
	nop
	ori		$s0, CLIP_TOP
.no_clip_t_y0_q:
	sra		$v1, $t9, 16
	ble		$v0, $v1, .no_clip_b_y0_q
	nop
	ori		$s0, CLIP_BOTTOM
.no_clip_b_y0_q:

	# X1 clip test
	mfc2	$t7, C2_SXY0
	sll		$v1, $t8, 16
	sra		$v1, 16
	sll		$v0, $t7, 16
	sra		$v0, 16
	bge		$v0, $v1, .no_clip_l_x1_q
	move	$s1, $0
	ori		$s1, CLIP_LEFT
.no_clip_l_x1_q:
	sll		$v1, $t9, 16
	sra		$v1, 16
	ble		$v0, $v1, .no_clip_r_x1_q
	nop
	ori		$s1, CLIP_RIGHT
.no_clip_r_x1_q:
	# Y1 clip test
	sra		$v0, $t7, 16
	sra		$v1, $t8, 16
	bge		$v0, $v1, .no_clip_t_y1_q
	nop
	ori		$s1, CLIP_TOP
.no_clip_t_y1_q:
	sra		$v1, $t9, 16
	ble		$v0, $v1, .no_clip_b_y1_q
	nop
	ori		$s1, CLIP_BOTTOM
.no_clip_b_y1_q:

	# X2 clip test
	mfc2	$t7, C2_SXY1
	sll		$v1, $t8, 16
	sra		$v1, 16
	sll		$v0, $t7, 16
	sra		$v0, 16
	bge		$v0, $v1, .no_clip_l_x2_q
	move	$s2, $0
	ori		$s2, CLIP_LEFT
.no_clip_l_x2_q:
	sll		$v1, $t9, 16
	sra		$v1, 16
	ble		$v0, $v1, .no_clip_r_x2_q
	nop
	ori		$s2, CLIP_RIGHT
.no_clip_r_x2_q:
	# Y2 clip test
	sra		$v0, $t7, 16
	sra		$v1, $t8, 16
	bge		$v0, $v1, .no_clip_t_y2_q
	nop
	ori		$s2, CLIP_TOP
.no_clip_t_y2_q:
	sra		$v1, $t9, 16
	ble		$v0, $v1, .no_clip_b_y2_q
	nop
	ori		$s2, CLIP_BOTTOM
.no_clip_b_y2_q:

	# X3 clip test
	mfc2	$t7, C2_SXY2
	sll		$v1, $t8, 16
	sra		$v1, 16
	sll		$v0, $t7, 16
	sra		$v0, 16
	bge		$v0, $v1, .no_clip_l_x3_q
	move	$s3, $0
	ori		$s3, CLIP_LEFT
.no_clip_l_x3_q:
	sll		$v1, $t9, 16
	sra		$v1, 16
	ble		$v0, $v1, .no_clip_r_x3_q
	nop
	ori		$s3, CLIP_RIGHT
.no_clip_r_x3_q:
	# Y4 clip test
	sra		$v0, $t7, 16
	sra		$v1, $t8, 16
	bge		$v0, $v1, .no_clip_t_y3_q
	nop
	ori		$s3, CLIP_TOP
.no_clip_t_y3_q:
	sra		$v1, $t9, 16
	ble		$v0, $v1, .no_clip_b_y3_q
	nop
	ori		$s3, CLIP_BOTTOM
.no_clip_b_y3_q:

.endm
