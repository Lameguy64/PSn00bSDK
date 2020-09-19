#ifndef _PSXGTE_H
#define _PSXGTE_H


#define ONE		4096


// For compatibility with official library syntax
#define csin(a) isin(a)
#define ccos(a) icos(a)
#define rsin(a) isin(a)
#define rcos(a) icos(a)


typedef struct MATRIX {
	short	m[3][3];
	int		t[3];
} MATRIX;

typedef struct VECTOR {
	int		vx, vy, vz;
} VECTOR;

typedef struct SVECTOR {
	short	vx, vy, vz, pad;
} SVECTOR;

typedef struct CVECTOR {
	unsigned char r, g, b, cd;
} CVECTOR;

typedef struct DVECTOR {
	short vx, vy;
} DVECTOR;


#ifdef __cplusplus
extern "C" {
#endif

void InitGeom();

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

#ifdef __cplusplus
}
#endif

#endif // _PSXGTE_H
