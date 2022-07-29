/*
 * PSn00bSDK GTE library
 * (C) 2019-2022 Lameguy64 - MPL licensed
 */

#ifndef __PSXGTE_H
#define __PSXGTE_H

#include <stdint.h>

#define ONE (1 << 12)

/* Structure definitions */

typedef struct _MATRIX {
	int16_t m[3][3];
	int32_t t[3];
} MATRIX;

typedef struct _VECTOR {
	int32_t vx, vy, vz;
} VECTOR;

typedef struct _SVECTOR {
	int16_t vx, vy, vz, pad;
} SVECTOR;

typedef struct _CVECTOR {
	uint8_t r, g, b, cd;
} CVECTOR;

typedef struct _DVECTOR {
	int16_t vx, vy;
} DVECTOR;

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

void InitGeom(void);

// Integer SIN/COS functions (4096 = 360 degrees)
// Does not use tables!
int isin(int a);
int icos(int a);

// Higher precision integer sin/cos functions (131072 = 360 degrees)
// Does not use tables!
int hisin(int a);
int hicos(int a);

void PushMatrix(void);
void PopMatrix(void);

MATRIX *RotMatrix(SVECTOR *r, MATRIX *m);
MATRIX *HiRotMatrix(VECTOR *r, MATRIX *m);

MATRIX *TransMatrix(MATRIX *m, VECTOR *r);
MATRIX *ScaleMatrix(MATRIX *m, VECTOR *s);
MATRIX *ScaleMatrixL(MATRIX *m, VECTOR *s);

MATRIX *MulMatrix(MATRIX *m0, MATRIX *m1);
MATRIX *MulMatrix0(MATRIX *m0, MATRIX *m1, MATRIX *m2);

MATRIX *CompMatrixLV(MATRIX *v0, MATRIX *v1, MATRIX *v2);
VECTOR *ApplyMatrixLV(MATRIX *m, VECTOR *v0, VECTOR *v1);

void VectorNormalS(VECTOR *v0, SVECTOR *v1);

void Square0(VECTOR *v0, VECTOR *v1);

int SquareRoot12(int v);
int SquareRoot0(int v);

#define csin(a) isin(a)
#define ccos(a) icos(a)
#define rsin(a) isin(a)
#define rcos(a) icos(a)

#ifdef __cplusplus
}
#endif

#endif
