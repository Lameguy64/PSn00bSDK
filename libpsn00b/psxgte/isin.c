/*
 * PSn00bSDK (incomplete) trigonometry library
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 *
 * Based on isin_S4 implementation from coranac:
 * https://www.coranac.com/2009/07/sines
 */

#define qN_l	10
#define qN_h	15
#define qA		12
#define B		19900
#define	C		3516

static inline int _isin(int qN, int x) {
	int c, x2, y;

	c  = x << (30 - qN);			// Semi-circle info into carry.
	x -= 1 << qN;					// sine -> cosine calc

	x <<= (31 - qN);				// Mask with PI
	x >>= (31 - qN);				// Note: SIGNED shift! (to qN)
	x  *= x;
	x >>= (2 * qN - 14);			// x=x^2 To Q14

	y = B - (x * C >> 14);			// B - x^2*C
	y = (1 << qA) - (x * y >> 16);	// A - x^2*(B-x^2*C)

	return (c >= 0) ? y : (-y);
}

int isin(int x) {
	return _isin(qN_l, x);
}

int icos(int x) {
	return _isin(qN_l, x + (1 << qN_l));
}

int hisin(int x) {
	return _isin(qN_h, x);
}

int hicos(int x) {
	return _isin(qN_h, x + (1 << qN_h));
}
