#include <sys/types.h>
#include <string.h>
#include <psxgte.h>
#include <psxgpu.h>
#include <inline_c.h>
#include <smd/smd.h>
#include "disp.h"

extern MATRIX lgt_colmtx;

extern SMD *o_world;
extern SMD *o_lightbulb;

void sort_overlay(int showlotl);

void lightdemo() {

	/*
		The point lighting demo is perhaps the most impressive part of
		n00bDEMO. A more streamlined version of this demo where you control
		various attributes of the light source such as position, intensity
		and color might be made as a dedicated example program in the future.

		The point lighting trick is actually not that too complicated. You
		basically calculate the distance and direction vector of two points
		which are the light source and the vertex of a polygon.

		Calculating the normal whose result can later be used to calculate
		the distance between two points is achieved with:

		vec_dir.vx = lgt_point.vx - pri_vert.vx;
		vec_dir.vy = lgt_point.vy - pri_vert.vy;
		vec_dir.vz = lgt_point.vz - pri_vert.vz;

		The intensity is calculated with (this might not be accurate but this
		is faster than applying a square root):

		i = 4096 - ( (
			(vec_dir.vx*vec_dir.vx) +
			(vec_dir.vy*vec_dir.vy) +
			(vec_dir.vz*vec_dir.vz) ) >> 7 );

		// Clip minimum intensity
		if( i < 0 )
			i = 0;

		This intensity value is then used to set the color of the light source
		through the light color matrix.

		col_mtx.m[0][0] = i;
		col_mtx.m[1][0] = i;
		col_mtx.m[2][0] = i;
		gte_SetColorMatrix( &col_mtx );

		The direction vector can then be used as the direction of the light
		source. It is recommended to normalize it first to prevent possible
		overflow related issues.

		VectorNormalS( &vec_dir, &vec_norm );

		lgt_mtx.m[0][0] = vec_norm.vx;
		lgt_mtx.m[0][1] = vec_norm.vy;
		lgt_mtx.m[0][2] = vec_norm.vz;

		gte_SetLightMatrix( &lgt_mtx );

		This operation is then performed for each point of a polygon to
		achieve a nice smooth shaded point lighting effect. The macros used
		are still the same as doing light source calculation with the GTE
		the normal way.

		3D geometry still requires normal data as with most lighting
		processing operations. 'Flat' normals (faces with a single normal
		vector) work best on flat surfaces while 'smooth' normals (faces with
		normals on each point) work best on round or curved surfaces.

	*/

	int			i,p_ang;

	SC_OT		s_ot;

	SVECTOR		rot;
	VECTOR		pos;
	SMD_PRIM	s_pri;

	VECTOR		l_point;
	SVECTOR		nrm;

	MATRIX		lmtx,llmtx,omtx;

	SVECTOR orot = { 0 };

	int timeout = SCENE_TIME;


	// Set clear color to black
	setRGB0( &draw, 0, 0, 0 );

	// Base values for the environment geometry
	setVector( &pos, 0, 0, 600 );
	setVector( &rot, 512, 0, 0 );

	// Set base tpage value for the SMD drawing routines
	smdSetBaseTPage( 0x200 );

	// Set back or ambient color to black for pure darkness
	gte_SetBackColor( 0, 0, 0 );

	memset( &llmtx, 0, sizeof(MATRIX) );


	// demo loop
	while( 1 ) {

		char buff[32];

		RotMatrix( &rot, &mtx );
		TransMatrix( &mtx, &pos );

		rot.vy += 4;

		gte_SetRotMatrix( &mtx );
		gte_SetTransMatrix( &mtx );


		setVector( &l_point, (icos( p_ang )>>2)>>2, -350+(icos( p_ang<<1 )>>4), (isin( p_ang )>>2)>>2 );
		p_ang += 16;


		// Begin parsing the SMD data of the environment
		OpenSMD( o_world );

		// Prototype point lighting renderer
		while( ReadSMD( &s_pri ) ) {

			VECTOR	v_dir;
			SVECTOR v_nrm;
			VECTOR	v_sqr;

			int		flg;

			if( s_pri.prim_id.texture ) {

				POLY_GT4	*pri;

				// Perform standard rotate, translate and perspective
				// transformation of the geometry
				pri = (POLY_GT4*)nextpri;

				gte_ldv3(
					&o_world->p_verts[s_pri.v0],
					&o_world->p_verts[s_pri.v1],
					&o_world->p_verts[s_pri.v2] );

				gte_rtpt();

				gte_nclip();			// Backface culling

				gte_stopz( &flg );

				if( flg < 0 )
					continue;

				gte_stsxy3( &pri->x0, &pri->x1, &pri->x2 );

				gte_ldv0( &o_world->p_verts[s_pri.v3] );
				gte_rtps();

				gte_avsz4();			// Depth sort
				gte_stotz( &flg );

				if( (flg>>2) >= OT_LEN )
					continue;

				gte_stsxy( &pri->x3 );


				// Load base color of polygon to GTE
				gte_ldrgb( &s_pri.r0 );

				// Load normal of polygon
				gte_ldv0( &o_world->p_norms[s_pri.n0] );

				// Calculate the direction between the vertex of the
				// polygon and the light source
				v_dir.vx = l_point.vx - o_world->p_verts[s_pri.v0].vx;
				v_dir.vy = l_point.vy - o_world->p_verts[s_pri.v0].vy;
				v_dir.vz = l_point.vz - o_world->p_verts[s_pri.v0].vz;

				// Calculate distance and light intensity using square
				i = 4096 - ( (
					(v_dir.vx*v_dir.vx) +
					(v_dir.vy*v_dir.vy) +
					(v_dir.vz*v_dir.vz) ) >> 7 );

				// Clip minimum intensity
				if( i < 0 )
					i = 0;

				// Set intensity to color matrix
				llmtx.m[0][0] = i;
				llmtx.m[1][0] = i;
				llmtx.m[2][0] = i;
				gte_SetColorMatrix( &llmtx );

				// Normalize light direction and set it to light matrix
				VectorNormalS( &v_dir, &v_nrm );
				lmtx.m[0][0] = v_nrm.vx;
				lmtx.m[0][1] = v_nrm.vy;
				lmtx.m[0][2] = v_nrm.vz;
				gte_SetLightMatrix( &lmtx );

				// Calculate (output is retrieved through gte_strgb)
				gte_nccs();


				// Repeat process for the next 3 vertices
				v_dir.vx = l_point.vx - o_world->p_verts[s_pri.v1].vx;
				v_dir.vy = l_point.vy - o_world->p_verts[s_pri.v1].vy;
				v_dir.vz = l_point.vz - o_world->p_verts[s_pri.v1].vz;

				i = 4096 - ( (
					(v_dir.vx*v_dir.vx) +
					(v_dir.vy*v_dir.vy) +
					(v_dir.vz*v_dir.vz) ) >> 7 );

				if( i < 0 )
					i = 0;


				llmtx.m[0][0] = i;
				llmtx.m[1][0] = i;
				llmtx.m[2][0] = i;

				gte_strgb( &pri->r0 );

				gte_SetColorMatrix( &llmtx );
				VectorNormalS( &v_dir, &v_nrm );
				lmtx.m[0][0] = v_nrm.vx;
				lmtx.m[0][1] = v_nrm.vy;
				lmtx.m[0][2] = v_nrm.vz;
				gte_SetLightMatrix( &lmtx );
				gte_nccs();

				v_dir.vx = l_point.vx - o_world->p_verts[s_pri.v2].vx;
				v_dir.vy = l_point.vy - o_world->p_verts[s_pri.v2].vy;
				v_dir.vz = l_point.vz - o_world->p_verts[s_pri.v2].vz;

				i = 4096 - ( (
					(v_dir.vx*v_dir.vx) +
					(v_dir.vy*v_dir.vy) +
					(v_dir.vz*v_dir.vz) ) >> 7 );

				if( i < 0 )
					i = 0;

				llmtx.m[0][0] = i;
				llmtx.m[1][0] = i;
				llmtx.m[2][0] = i;

				gte_strgb( &pri->r1 );

				gte_SetColorMatrix( &llmtx );
				VectorNormalS( &v_dir, &v_nrm );
				lmtx.m[0][0] = v_nrm.vx;
				lmtx.m[0][1] = v_nrm.vy;
				lmtx.m[0][2] = v_nrm.vz;
				gte_SetLightMatrix( &lmtx );
				gte_nccs();

				v_dir.vx = l_point.vx - o_world->p_verts[s_pri.v3].vx;
				v_dir.vy = l_point.vy - o_world->p_verts[s_pri.v3].vy;
				v_dir.vz = l_point.vz - o_world->p_verts[s_pri.v3].vz;

				i = 4096 - ( (
					(v_dir.vx*v_dir.vx) +
					(v_dir.vy*v_dir.vy) +
					(v_dir.vz*v_dir.vz) ) >> 7 );

				if( i < 0 )
					i = 0;

				llmtx.m[0][0] = i;
				llmtx.m[1][0] = i;
				llmtx.m[2][0] = i;

				gte_strgb( &pri->r2 );

				gte_SetColorMatrix( &llmtx );
				VectorNormalS( &v_dir, &v_nrm );
				lmtx.m[0][0] = v_nrm.vx;
				lmtx.m[0][1] = v_nrm.vy;
				lmtx.m[0][2] = v_nrm.vz;
				gte_SetLightMatrix( &lmtx );
				gte_nccs();

				setUV4( pri,
					s_pri.tu0, s_pri.tv0,
					s_pri.tu1, s_pri.tv1,
					s_pri.tu2, s_pri.tv2,
					s_pri.tu3, s_pri.tv3 );

				pri->tpage = s_pri.tpage;
				pri->clut = s_pri.clut;

				setPolyGT4( pri );
				addPrim( ot[db]+(flg>>2), pri );
				nextpri += sizeof(POLY_GT4);

				gte_strgb( &pri->r3 );

			} else {

				POLY_G4	*pri;

				pri = (POLY_G4*)nextpri;

				gte_ldv3(
					&o_world->p_verts[s_pri.v0],
					&o_world->p_verts[s_pri.v1],
					&o_world->p_verts[s_pri.v2] );

				gte_rtpt();

				gte_nclip();

				gte_stopz( &flg );

				if( flg < 0 )
					continue;

				gte_stsxy3( &pri->x0, &pri->x1, &pri->x2 );

				gte_ldv0( &o_world->p_verts[s_pri.v3] );
				gte_rtps();

				gte_avsz4();
				gte_stotz( &flg );

				if( (flg>>2) >= OT_LEN )
					continue;

				gte_stsxy( &pri->x3 );

				gte_ldrgb( &s_pri.r0 );
				gte_ldv0( &o_world->p_norms[s_pri.n0] );

				v_dir.vx = l_point.vx - o_world->p_verts[s_pri.v0].vx;
				v_dir.vy = l_point.vy - o_world->p_verts[s_pri.v0].vy;
				v_dir.vz = l_point.vz - o_world->p_verts[s_pri.v0].vz;

				i = 4096 - ( (
					(v_dir.vx*v_dir.vx) +
					(v_dir.vy*v_dir.vy) +
					(v_dir.vz*v_dir.vz) ) >> 7 );

				if( i < 0 )
					i = 0;

				llmtx.m[0][0] = i;
				llmtx.m[1][0] = i;
				llmtx.m[2][0] = i;
				gte_SetColorMatrix( &llmtx );
				VectorNormalS( &v_dir, &v_nrm );
				lmtx.m[0][0] = v_nrm.vx;
				lmtx.m[0][1] = v_nrm.vy;
				lmtx.m[0][2] = v_nrm.vz;
				gte_SetLightMatrix( &lmtx );
				gte_nccs();


				v_dir.vx = l_point.vx - o_world->p_verts[s_pri.v1].vx;
				v_dir.vy = l_point.vy - o_world->p_verts[s_pri.v1].vy;
				v_dir.vz = l_point.vz - o_world->p_verts[s_pri.v1].vz;

				i = 4096 - ( (
					(v_dir.vx*v_dir.vx) +
					(v_dir.vy*v_dir.vy) +
					(v_dir.vz*v_dir.vz) ) >> 7 );

				if( i < 0 )
					i = 0;

				llmtx.m[0][0] = i;
				llmtx.m[1][0] = i;
				llmtx.m[2][0] = i;

				gte_strgb( &pri->r0 );

				gte_SetColorMatrix( &llmtx );
				VectorNormalS( &v_dir, &v_nrm );
				lmtx.m[0][0] = v_nrm.vx;
				lmtx.m[0][1] = v_nrm.vy;
				lmtx.m[0][2] = v_nrm.vz;
				gte_SetLightMatrix( &lmtx );
				gte_nccs();

				v_dir.vx = l_point.vx - o_world->p_verts[s_pri.v2].vx;
				v_dir.vy = l_point.vy - o_world->p_verts[s_pri.v2].vy;
				v_dir.vz = l_point.vz - o_world->p_verts[s_pri.v2].vz;

				i = 4096 - ( (
					(v_dir.vx*v_dir.vx) +
					(v_dir.vy*v_dir.vy) +
					(v_dir.vz*v_dir.vz) ) >> 7 );

				if( i < 0 )
					i = 0;

				llmtx.m[0][0] = i;
				llmtx.m[1][0] = i;
				llmtx.m[2][0] = i;

				gte_strgb( &pri->r1 );

				gte_SetColorMatrix( &llmtx );
				VectorNormalS( &v_dir, &v_nrm );
				lmtx.m[0][0] = v_nrm.vx;
				lmtx.m[0][1] = v_nrm.vy;
				lmtx.m[0][2] = v_nrm.vz;
				gte_SetLightMatrix( &lmtx );
				gte_nccs();

				v_dir.vx = l_point.vx - o_world->p_verts[s_pri.v3].vx;
				v_dir.vy = l_point.vy - o_world->p_verts[s_pri.v3].vy;
				v_dir.vz = l_point.vz - o_world->p_verts[s_pri.v3].vz;

				i = 4096 - ( (
					(v_dir.vx*v_dir.vx) +
					(v_dir.vy*v_dir.vy) +
					(v_dir.vz*v_dir.vz) ) >> 7 );

				if( i < 0 )
					i = 0;

				llmtx.m[0][0] = i;
				llmtx.m[1][0] = i;
				llmtx.m[2][0] = i;

				gte_strgb( &pri->r2 );

				gte_SetColorMatrix( &llmtx );
				VectorNormalS( &v_dir, &v_nrm );
				lmtx.m[0][0] = v_nrm.vx;
				lmtx.m[0][1] = v_nrm.vy;
				lmtx.m[0][2] = v_nrm.vz;
				gte_SetLightMatrix( &lmtx );
				gte_nccs();

				setPolyG4( pri );
				addPrim( ot[db]+(flg>>2), pri );

				nextpri += sizeof(POLY_G4);

				gte_strgb( &pri->r3 );

			}

		}


		// Sort the light bulb to represent the position of the light source
		orot.vx += 32;
		orot.vy += 32;
		orot.vz += 32;

		RotMatrix( &orot, &omtx );
		TransMatrix( &omtx, &l_point );

		CompMatrixLV( &mtx, &omtx, &mtx );

		gte_SetRotMatrix( &mtx );
		gte_SetTransMatrix( &mtx );

		s_ot.ot = ot[db];
		s_ot.otlen = OT_LEN;
		s_ot.zdiv = 2;
		s_ot.zoff = 0;

		nextpri = smdSortModel( &s_ot, nextpri, o_lightbulb );


		// Sort overlay and display
		sort_overlay( 1 );

		display();

		timeout--;
		if( timeout < 0 )
			break;

	}

}