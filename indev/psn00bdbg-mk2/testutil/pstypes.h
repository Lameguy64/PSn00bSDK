#ifndef PSTYPES_H
#define PSTYPES_H

// CPU
#define PS_REG_at		0
#define PS_REG_v0		1
#define PS_REG_v1		2
#define PS_REG_a0		3
#define PS_REG_a1		4
#define PS_REG_a2		5
#define PS_REG_a3		6
#define PS_REG_t0		7
#define PS_REG_t1		8
#define PS_REG_t2		9
#define PS_REG_t3		10
#define PS_REG_t4		11
#define PS_REG_t5		12
#define PS_REG_t6		13
#define PS_REG_t7		14
#define PS_REG_s0		15
#define PS_REG_s1		16
#define PS_REG_s2		17
#define PS_REG_s3		18
#define PS_REG_s4		19
#define PS_REG_s5		20
#define PS_REG_s6		21
#define PS_REG_s7		22
#define PS_REG_t8		23
#define PS_REG_t9		24
#define PS_REG_k0		25
#define PS_REG_k1		26
#define PS_REG_gp		27
#define PS_REG_sp		28
#define PS_REG_fp		29
#define PS_REG_ra		30
#define PS_REG_lo		31
#define PS_REG_hi		32

// cop0
#define PS_REG_epc		33
#define PS_REG_cause	34
#define PS_REG_status	35
#define PS_REG_baddr	36
#define PS_REG_tar		37
#define PS_REG_dcic		38
#define PS_REG_opcode	39

typedef struct {
	unsigned int pc0;
	unsigned int gp0;
	unsigned int t_addr;
	unsigned int t_size;
	unsigned int d_addr;
	unsigned int d_size;
	unsigned int b_addr;
	unsigned int b_size;
	unsigned int sp_addr;
	unsigned int sp_size;
	unsigned int sp;
	unsigned int fp;
	unsigned int gp;
	unsigned int ret;
	unsigned int base;
} EXEC;

typedef struct {
	char header[8];
	char pad[8];
	EXEC params;
	char license[64];
	char pad2[1908];
} PSEXE;

typedef struct {
	EXEC params;
	unsigned int crc32;
	unsigned int flags;
} EXEPARAM;

#endif /* PSTYPES_H */

