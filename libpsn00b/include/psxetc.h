#ifndef _PSXETC_H
#define _PSXETC_H

#ifdef __cplusplus
extern "C" {
#endif

void FntLoad(int x, int y);
char *FntSort(unsigned int *ot, char *pri, int x, int y, const char *text);

#ifdef __cplusplus
}
#endif

#endif
