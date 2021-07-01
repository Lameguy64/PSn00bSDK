#ifndef _DISP_H
#define _DISP_H

#include <sys/types.h>
#include <psxgte.h>

#define SCENE_TIME	60*15

#define CENTERX 320
#define CENTERY 240

#define OT_LEN	260

void initDisplay();
void display();

extern u_long ot[2][OT_LEN];
extern char *nextpri;
extern int db;

extern DISPENV disp;
extern DRAWENV draw;

extern MATRIX mtx;

#endif // _DISP_H