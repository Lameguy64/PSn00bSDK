#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <psxetc.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <inline_c.h>
#include <smd/smd.h>
#include <lzp/lzp.h>

#include "disp.h"
#include "data.h"

#define MAX_STARS	64


typedef struct {
	int x,y;
	int xvel,yvel;
	int scale;
	int rot,rotv;
} PARTICLE;

typedef struct {
	u_long	tag;
	u_long	tpage;
	u_long	mask1;
	u_char	r0,g0,b0,code;
	short	x0,y0;
	short	x1,y1;
	short	x2,y2;
	short	x3,y3;
	u_long	mask2;
} MASKP_F4;

typedef struct {
	u_long	tag;
	u_long	tpage;
	u_char	r0,g0,b0,code;
	short	x0,y0;
	short	w,h;
} FADERECT;


SMD *o_psn00b, *o_n00blogo;


typedef struct {
	u_long *prev;
	u_long *next;
	int size;
} NODE;

extern NODE _end[];

void DumpHeap() {

	NODE *n = _end;

	printf( "--\n" );

	while( 1 ) {

		printf( "B:%08p  P:%08p  N:%08p  SZ:%d  BS:%d\n",
			n, n->prev, n->next, n->size,
			((unsigned int)n->next - (unsigned int)n) );

		if ( !n->next )
			break;

		n = (NODE*)n->next;

	}

}


void intro() {

	SVECTOR		quad_coords[] = {
		{ -100, -100, 0, 0 },
		{  100, -100, 0, 0 },
		{ -100,  100, 0, 0 },
		{  100,  100, 0, 0 }
	};

	PARTICLE	stars[MAX_STARS];

	VECTOR		mpos;
	SVECTOR		mrot,trot;
	SC_OT		s_ot;

	SMD			*o_disk, *o_star, *o_text, *o_psn00b, *o_n00blogo;

	int i,count = 0;

	int logo_scale;
	int logo_rot;

	int logo_yvel;
	int logo_ypos;
	int logo_step;
	int logo_spin;
	int logo_svel;
	int logo_count = 0;
	int logo_tscale;

	int logo_n00b_spin;
	int logo_fade = 0;

	struct {
		int step;
		int yvel;
		int ypos;
		int rot;
		int scale;
		int spin;
		int spinvel;
		int tscale;
		int trot;
	} mtek = {
		0,
		0,
		ONE*750,
		-ONE*512,
		ONE*7,
		0,
		0,
		3072,
		0
	};

	struct {
		VECTOR pos;
		int scale;
		int scarlet_spin;
	} psn00b = {
		{ 0, 0, 0 },
		ONE,
		1024
	};


	i = lzpSearchFile( "mtekdisk", lz_resources );
	o_disk = (SMD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( o_disk, lz_resources, i );
	smdInitData( o_disk );

	i = lzpSearchFile( "mtektext", lz_resources );
	o_text = (SMD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( o_text, lz_resources, i );
	smdInitData( o_text );

	i = lzpSearchFile( "starsprite", lz_resources );
	o_star = (SMD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( o_star, lz_resources, i );
	smdInitData( o_star );

	i = lzpSearchFile( "psn00blogo", lz_resources );
	o_psn00b = (SMD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( o_psn00b, lz_resources, i );
	smdInitData( o_psn00b );

	i = lzpSearchFile( "n00blogo", lz_resources );
	o_n00blogo = (SMD*)malloc( lzpFileSize( lz_resources, i ) );
	lzpUnpackFile( o_n00blogo, lz_resources, i );
	smdInitData( o_n00blogo );

	//DumpHeap();


	// Set some Scarlet global parameters
	smdSetBaseTPage( 0x200 );

	// Change clear color
	setRGB0( &draw, 255, 255, 255 );

	setVector( &trot, 0, 0, 0 );


	// Initialize stars
	for( i=0; i<MAX_STARS; i++ ) {

		stars[i].x = ONE*(-200+(rand()%400));
		stars[i].y = ONE*(80+(rand()%100));

		stars[i].yvel = -(rand()%32768);
		stars[i].xvel = -(ONE*(10-(rand()%20)));

		stars[i].scale = 2048+(rand()%2048);
		stars[i].rot = ONE*(rand()%4096);
		stars[i].rotv = ONE*(-40+(rand()%80));

	}

	logo_n00b_spin = ONE+1024;

	while( logo_count < 1320 ) {

		if( logo_count < 700 ) {

			if( logo_count > 360 ) {
				mtek.spin -= mtek.spinvel;
				mtek.spinvel += 1280;
			}

			setVector( &mpos, 0, -(mtek.ypos>>12)-20, 400 );
			setVector( &mrot, 0, mtek.spin>>12, mtek.rot>>12 );

			RotMatrix( &mrot, &mtx );
			TransMatrix( &mtx, &mpos );
			setVector( &mpos, mtek.scale, mtek.scale, 4096 );
			ScaleMatrix( &mtx, &mpos );

			gte_SetRotMatrix( &mtx );
			gte_SetTransMatrix( &mtx );

			nextpri = smdSortModelFlat( ot[db]+10, nextpri, o_disk );


			if( mtek.step > 0 ) {

				setVector( &mpos, 0, 180, 400 );
				TransMatrix( &mtx, &mpos );
				gte_SetTransMatrix( &mtx );

				nextpri = smdSortModelFlat( ot[db]+8, nextpri, o_text );

				if( logo_count < 480 ) {

					SVECTOR srot;

					for( i=0; i<MAX_STARS; i++ ) {

						setVector( &mpos, stars[i].x>>12, stars[i].y>>12, 320 );
						setVector( &srot, 0, 0, stars[i].rot>>12 );

						stars[i].x += stars[i].xvel;
						stars[i].y += stars[i].yvel;

						stars[i].yvel += 512;
						stars[i].rot += stars[i].rotv;

						RotMatrix( &srot, &mtx );
						TransMatrix( &mtx, &mpos );

						setVector( &mpos, stars[i].scale, stars[i].scale, 4096 );
						ScaleMatrix( &mtx, &mpos );

						gte_SetRotMatrix( &mtx );
						gte_SetTransMatrix( &mtx );

						nextpri = smdSortModelFlat( ot[db]+6, nextpri, o_star );

					}

				}

			}

			if( !mtek.step ) {
				mtek.rot -= ONE*20;
			} else if( mtek.step == 1 ) {
				mtek.rot += ( -mtek.rot )>>4;
			}


			if( mtek.step < 2 ) {
				mtek.scale	+= (ONE-mtek.scale )>>5;
				mtek.yvel += 512;
				mtek.ypos -= mtek.yvel;
			}


			if( ( mtek.ypos <= 0 ) && ( mtek.yvel > 0 ) ) {
				mtek.yvel = -ONE*6;
				mtek.step++;
			}

			// Does the transition effect
			if( logo_count > 480 ) {

				TILE *rect = (TILE*)nextpri;

				mtek.trot += 16;

				setTile( rect );
				setXY0( rect, 0, 0 );
				setWH( rect, 640, 511 );
				setRGB0( rect, 0, 0, 0 );
				addPrim( ot[db]+4, rect );
				nextpri += sizeof(TILE);

				setVector( &mrot, 0, 0, mtek.trot );
				setVector( &mpos, mtek.tscale, mtek.tscale, 4096 );

				RotMatrix( &mrot, &mtx );
				ScaleMatrix( &mtx, &mpos );

				gte_SetRotMatrix( &mtx );

				for( i=0; i<6; i++ ) {

					MASKP_F4 *pol4 = (MASKP_F4*)nextpri;

					setVector( &mpos, -100+(100*(i%3)), -60+(120*(i/3)), 160 );
					TransMatrix( &mtx, &mpos );
					gte_SetTransMatrix( &mtx );

					pol4->tag = 0x08000000;
					pol4->tpage = 0xe1000020;
					pol4->mask1 = 0xe6000001;
					pol4->mask2 = 0xe6000002;
					pol4->code = 0x2A;

					gte_ldv3( &quad_coords[0], &quad_coords[1], &quad_coords[2] );
					gte_rtpt();
					gte_stsxy0( &pol4->x0 );
					gte_ldv0( &quad_coords[3] );
					gte_rtps();
					gte_stsxy3( &pol4->x1, &pol4->x2, &pol4->x3 );
					setRGB0( pol4, 0, 0, 0 );

					addPrim( ot[db]+4, pol4 );
					nextpri += sizeof(MASKP_F4);

				}

				mtek.tscale += ( -mtek.tscale )>>6;

			}

		} else {

			setRGB0( &draw, 0, 0, 0 );

		}

		if( logo_count >= 480 ) {

			if( logo_count > 840 ) {

				psn00b.pos.vx += ((ONE*-450)-psn00b.pos.vx)>>4;
				psn00b.pos.vy += ((ONE*350)-psn00b.pos.vy)>>4;

				setVector( &mpos, 0, 0, 600 );
				mpos.vx = psn00b.pos.vx>>12;
				mpos.vy = psn00b.pos.vy>>12;

				psn00b.scale += ( 1536-psn00b.scale )>>4;

				setVector( &mrot, 0, 0, 0 );

				RotMatrix( &mrot, &mtx );
				TransMatrix( &mtx, &mpos );

				setVector( &mpos, psn00b.scale, psn00b.scale, ONE );

				ScaleMatrix( &mtx, &mpos );

			} else {

				setVector( &mpos, 0, 0, 600 );
				setVector( &mrot, 0, logo_n00b_spin, 0 );

				RotMatrix( &mrot, &mtx );
				TransMatrix( &mtx, &mpos );

				logo_n00b_spin += ( -logo_n00b_spin )>>6;

			}

			gte_SetRotMatrix( &mtx );
			gte_SetTransMatrix( &mtx );

			nextpri = smdSortModelFlat( ot[db]+3, nextpri, o_psn00b );

			if( logo_count > 900 ) {

				setVector( &mpos, 0, 0, 250 );
				setVector( &mrot, psn00b.scarlet_spin, 0, 0 );

				RotMatrix( &mrot, &mtx );
				TransMatrix( &mtx, &mpos );

				gte_SetRotMatrix( &mtx );
				gte_SetTransMatrix( &mtx );

				nextpri = smdSortModelFlat( ot[db]+3, nextpri, o_n00blogo );

				psn00b.scarlet_spin += ( -psn00b.scarlet_spin )>>6;

			}

		}

		if( logo_count > 1200 ) {

			FADERECT *fade = (FADERECT*)nextpri;

			fade->tag = 0x04000000;
			fade->tpage = 0xe1000040;
			fade->code = 0x62;

			setRGB0( fade, logo_fade, logo_fade, logo_fade );
			setXY0( fade, 0, 0 );
			setWH( fade, 640, 480 );
			addPrim( ot[db]+2, fade );

			if( logo_fade < 250 ) {
				logo_fade += 4;
			}

			nextpri += sizeof(FADERECT);

		}

		display();

		logo_count++;

	}

	free( o_disk );
	free( o_text );
	free( o_star );
	free( o_psn00b );
	free( o_n00blogo );

}
