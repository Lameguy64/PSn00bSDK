#ifndef _LOOKAT_H
#define _LOOKAT_H

#include <sys/types.h>
#include <psxgte.h>
#include <psxgpu.h>

/* LookAt
 *
 * Generates a matrix that looks from 'eye' to 'at'.
 *
 * eye	- Position of viewpoint
 * at	- Position to 'look at' from viewpoint
 * up	- Vector that defines the 'up' direction
 * mtx	- Matrix output
 *
 */
void LookAt(VECTOR *eye, VECTOR *at, SVECTOR *up, MATRIX *mtx);

#endif // _LOOKAT_H