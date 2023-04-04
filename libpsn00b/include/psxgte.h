/*
 * PSn00bSDK GTE library
 * (C) 2019-2022 Lameguy64 - MPL licensed
 */

/**
 * @file psxgte.h
 * @brief GTE library header
 *
 * @details The Geometry Transformation Engine, often referred to as the GTE,
 * is most responsible for providing 3D capabilities to the PS1. This is
 * effectively an all-integer math co-processor connected directly to the CPU,
 * as it is accessed using COP2 and related MIPS instructions to access
 * registers and issue commands to the GTE.
 */

#pragma once

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

#define csin(a) isin(a)
#define ccos(a) icos(a)
#define rsin(a) isin(a)
#define rcos(a) icos(a)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Gets sine of angle (fixed-point)
 *
 * @details Returns the sine of angle a.
 *
 * @param a Angle in fixed-point format (131072 = 360 degrees)
 * @return Sine value in 20.12 fixed-point format (4096 = 1.0).
 */
int isin(int a);

/**
 * @brief Gets cosine of angle (fixed-point)
 *
 * @details Returns the cosine of angle a.
 *
 * @param a Angle in fixed-point format (131072 = 360 degrees)
 * @return Cosine value in 20.12 fixed-point format (4096 = 1.0).
 */
int icos(int a);

/**
 * @brief Gets sine of angle (fixed-point, high precision version)
 *
 * @details Returns the sine of angle a.
 *
 * @param a Angle in fixed-point format (4194304 = 360 degrees)
 * @return Sine value in 20.12 fixed-point format (4096 = 1.0).
 */
int hisin(int a);

/**
 * @brief Gets cosine of angle (fixed-point, high precision version)
 *
 * @details Returns the cosine of angle a.
 *
 * @param a Angle in fixed-point format (4194304 = 360 degrees)
 * @return Cosine value in 20.12 fixed-point format (4096 = 1.0).
 */
int hicos(int a);

/**
 * @brief Initializes the GTE
 *
 * @details Resets, enables and initializes the GTE. Must be called prior to
 * using any GTE function or macro.
 */
void InitGeom(void);

/**
 * @brief Gets square root (fixed-point)
 *
 * @details Returns the square root of value v.
 *
 * @param v Value in 20.12 fixed-point format (4096 = 1.0)
 * @return Square root in 20.12 fixed-point format (4096 = 1.0).
 */
int SquareRoot12(int v);

/**
 * @brief Gets square root (integer)
 *
 * @details Returns the square root of value v.
 *
 * @param v Value in integer format
 * @return Square root in integer format.
 */
int SquareRoot0(int v);

/**
 * @brief Pushes the current GTE matrix to the matrix stack
 *
 * @details Pushes the current GTE rotation matrix and translation vector to
 * the internal matrix stack. Only one matrix stack level is currently
 * supported.
 */
void PushMatrix(void);

/**
 * @brief Pops the last matrix pushed into the matrix stack back to the GTE
 *
 * @details Pops the last inserted matrix in the internal matrix stack back to
 * the GTE. Only one matrix stack level is currently supported.
 */
void PopMatrix(void);

/**
 * @brief Defines the rotation matrix of a MATRIX
 *
 * @details Defines the rotation matrix of m from rotation coordinates of r.
 * The matrix is computed as follows:
 *
 *     [ 1   0   0 ]   [ cy  0   sy]   [ cz -sz  0 ]
 *     [ 0   cx -sx] * [ 0   1   0 ] * [ sz  cz  0 ]
 *     [ 0   sx  cx]   [-sy  0   cy]   [ 0   0   1 ]
 *
 * where:
 *
 *     sx = sin(r.x)   sy = sin(r.y)   sz = sin(r.z)
 *     cx = cos(r.x)   cy = cos(r.y)   cz = cos(r.z)
 *
 * @param r Rotation vector (input)
 * @param m Matrix (output)
 * @return Pointer to m.
 *
 * @see TransMatrix(), CompMatrixLV()
 */
MATRIX *RotMatrix(SVECTOR *r, MATRIX *m);

/**
 * @brief Defines the rotation matrix of a MATRIX (high precision version)
 *
 * @details Defines the rotation matrix of m from rotation coordinates of r.
 * This function is a variant of RotMatrix() that uses hisin()/hicos() instead
 * of isin()/icos().
 *
 * See RotMatrix() for more details.
 *
 * @param r Rotation vector (input)
 * @param m Matrix (output)
 * @return Pointer to m.
 *
 * @see RotMatrix()
 */
MATRIX *HiRotMatrix(VECTOR *r, MATRIX *m);

/**
 * @brief Defines the translation vector of a MATRIX
 *
 * @details Simply sets the translation vector of MATRIX m. To perform
 * accumulative translation operations, see CompMatrixLV().
 *
 * @param m Matrix (output)
 * @param r Translation vector (input)
 * @return Pointer to m.
 *
 * @see RotMatrix(), CompMatrixLV()
 */
MATRIX *TransMatrix(MATRIX *m, VECTOR *r);

MATRIX *ScaleMatrix(MATRIX *m, VECTOR *s);
MATRIX *ScaleMatrixL(MATRIX *m, VECTOR *s);

MATRIX *MulMatrix(MATRIX *m0, MATRIX *m1);
MATRIX *MulMatrix0(MATRIX *m0, MATRIX *m1, MATRIX *m2);

/**
 * @brief Composite coordinate matrix transform
 *
 * @details Performs vector multiply by matrix with vector addition from v0 to
 * the translation vector of v1. Then, multiples the rotation matrix of v0 by
 * the rotation matrix of v1. The result of both operations is then stored in
 * v2. Replaces the current GTE rotation matrix and translation vector with v0.
 *
 * Often used to adjust the matrix (includes rotation and translation) of an
 * object relative to a world matrix, so the object would render relative to
 * the world matrix.
 *
 * @param v0 Input matrix A
 * @param v1 Input matrix B
 * @param v2 Output matrix
 * @return Pointer to v2.
 */
MATRIX *CompMatrixLV(MATRIX *v0, MATRIX *v1, MATRIX *v2);

/**
 * @brief Multiplies a vector by a matrix
 *
 * @details Multiplies vector v0 with matrix m, result is stored to v1.
 * Replaces the current GTE rotation matrix and translation vector with m.
 *
 * Often used to calculate a translation vector in relation to the rotation
 * matrix for first person or vector camera perspectives.
 *
 * @param m Input matrix
 * @param v0 Input vector
 * @param v1 Output vector
 * @return Pointer to v1.
 */
VECTOR *ApplyMatrixLV(MATRIX *m, VECTOR *v0, VECTOR *v1);

/**
 * @brief Normalizes a VECTOR into SVECTOR format
 *
 * Normalizes a 32-bit vector into a 16-bit vector in 4.12 fixed-point format
 * (4096 = 1.0, 2048 = 0.5).
 *
 * @param v0 Input (raw) 32-bit vector
 * @param v1 Output (normalized) 16-bit vector
 */
void VectorNormalS(VECTOR *v0, SVECTOR *v1);

/**
 * @brief Calculates the square of a VECTOR
 *
 * @details Calculates the square of vector v0 and stores the result to v1.
 *
 * @param v0 Input vector
 * @param v1 Output vector
 */
void Square0(VECTOR *v0, VECTOR *v1);

#ifdef __cplusplus
}
#endif
