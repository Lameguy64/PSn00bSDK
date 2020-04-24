/* Polygon clip detection code
 *
 * The polygon clipping logic is based on the Cohen-Sutherland algorithm, but
 * only the off-screen detection logic is used to determine which polygon edges
 * are off-screen.
 *
 * In tri_clip, the following edges are checked as follows:
 *
 *  |\
 *  |  \
 *  |    \
 *  |      \
 *  |--------
 *
 * In quad_clip, the following edges are checked as follows:
 *
 *  |---------|
 *  | \     / |
 *  |   \ /   |
 *  |   / \   |
 *  | /     \ |
 *  |---------|
 *
 * The inner portion of the quad is checked, otherwise the quad will be
 * culled out if the camera faces right into it, where all four edges
 * are off-screen at once.
 *
 */
 
#include "clip.h"

#define CLIP_LEFT	1
#define CLIP_RIGHT	2
#define CLIP_TOP	4
#define CLIP_BOTTOM	8

int test_clip(RECT *clip, short x, short y) {
	
	// Tests which corners of the screen a point lies outside of
	
	int result = 0;

	if ( x < clip->x ) {
		result |= CLIP_LEFT;
	}
	
	if ( x >= (clip->x+(clip->w-1)) ) {
		result |= CLIP_RIGHT;
	}
	
	if ( y < clip->y ) {
		result |= CLIP_TOP;
	}
	
	if ( y >= (clip->y+(clip->h-1)) ) {
		result |= CLIP_BOTTOM;
	}

	return result;
	
}

int tri_clip(RECT *clip, DVECTOR *v0, DVECTOR *v1, DVECTOR *v2) {
	
	// Returns non-zero if a triangle is outside the screen boundaries
	
	short c[3];

	c[0] = test_clip(clip, v0->vx, v0->vy);
	c[1] = test_clip(clip, v1->vx, v1->vy);
	c[2] = test_clip(clip, v2->vx, v2->vy);

	if ( ( c[0] & c[1] ) == 0 )
		return 0;
	if ( ( c[1] & c[2] ) == 0 )
		return 0;
	if ( ( c[2] & c[0] ) == 0 )
		return 0;

	return 1;
}

int quad_clip(RECT *clip, DVECTOR *v0, DVECTOR *v1, DVECTOR *v2, DVECTOR *v3) {
	
	// Returns non-zero if a quad is outside the screen boundaries
	
	short c[4];

	c[0] = test_clip(clip, v0->vx, v0->vy);
	c[1] = test_clip(clip, v1->vx, v1->vy);
	c[2] = test_clip(clip, v2->vx, v2->vy);
	c[3] = test_clip(clip, v3->vx, v3->vy);

	if ( ( c[0] & c[1] ) == 0 )
		return 0;
	if ( ( c[1] & c[2] ) == 0 )
		return 0;
	if ( ( c[2] & c[3] ) == 0 )
		return 0;
	if ( ( c[3] & c[0] ) == 0 )
		return 0;
	if ( ( c[0] & c[2] ) == 0 )
		return 0;
	if ( ( c[1] & c[3] ) == 0 )
		return 0;

	return 1;
}