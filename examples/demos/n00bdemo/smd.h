#ifndef _SMD_H
#define _SMD_H

typedef struct {
	u_long			*ot;
	short			otlen;
	unsigned char	zdiv,zoff;
} SC_OT;

typedef struct {
	char			id[3];
	unsigned char	version;
	unsigned short	flags;
	unsigned short	n_verts;
	unsigned short	n_norms;
	unsigned short	n_prims;
	SVECTOR			*p_verts;
	SVECTOR			*p_norms;
	void			*p_prims;
} SMD;

typedef struct {
	unsigned char type:2;
	unsigned char l_type:2;
	unsigned char c_type:1;
    unsigned char texture:1;
	unsigned char blend:2;
	unsigned char zoff:4;
	unsigned char nocull:1;
	unsigned char mask:1;
	unsigned char texwin:2;
	unsigned char texoff:2;
	unsigned char reserved:6;
	unsigned char len;
} SMD_PRI_TYPE;

typedef struct {
	SMD_PRI_TYPE	prim_id;
	unsigned short	v0,v1,v2,v3;		// Vertex indices
	unsigned short	n0,n1,n2,n3;		// Normal indices
	unsigned char	r0,g0,b0,code;		// RGB0
	unsigned char	r1,g1,b1,p0;		// RGB1
	unsigned char	r2,g2,b2,p1;		// RGB2
	unsigned char	r3,g3,b3,p2;		// RGB3
	unsigned char	tu0,tv0;
	unsigned char	tu1,tv1;
	unsigned char	tu2,tv2;
	unsigned char	tu3,tv3;
	unsigned short	tpage,clut;
} SMD_PRIM;


int OpenSMD(void *smd);
SMD_PRIM *ReadSMD(SMD_PRIM *pri);

void scSetClipRect(int x0, int y0, int x1, int y1);

SMD *smdInitData(void *data);
void smdSetBaseTPage(unsigned short tpage);

char *smdSortModel(SC_OT *ot, char* pribuff, SMD *smd);
char *smdSortModelFlat(u_long *ot, char* pribuff, SMD *smd);

void smdSetCelTex(unsigned short tpage, unsigned short clut);
void smdSetCelParam(int udiv, int vdiv, unsigned int col);
char *smdSortModelCel(SC_OT *ot, char* pribuff, SMD *smd);

#endif