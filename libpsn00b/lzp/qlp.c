#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lzqlp.h"

static char* lcase(char* str) {

    while(*str != 0x00) {
		*str = tolower(*str);
		str++;
    }

    return(str);

}

int qlpFileCount(void* qlpfile) {

    if (strncmp(((QLP_HEAD*)qlpfile)->id, "QLP", 3) != 0)
		return(PACK_ERR_INVALID);

    return(((QLP_HEAD*)qlpfile)->numfiles);

}

QLP_FILE* qlpFileEntry(int index, void* qlpfile) {

	if (strncmp(((QLP_HEAD*)qlpfile)->id, "QLP", 3) != 0)
		return(NULL);

	if (index > ((QLP_HEAD*)qlpfile)->numfiles)
		return(NULL);

	return(&((QLP_FILE*)(qlpfile+4))[index]);

}

void* qlpFileAddr(int index, void* qlpfile) {

	return( qlpfile+((QLP_FILE*)(qlpfile+4))[index].offs );

}

int qlpFindFile(char* fileName, void* qlpfile) {

	int i;
	char nameBuff[2][16];

    strcpy(nameBuff[0], fileName);
    lcase(nameBuff[0]);

	for(i=0; i<((QLP_HEAD*)qlpfile)->numfiles; i++) {

		strcpy(nameBuff[1], ((QLP_FILE*)(qlpfile+4))[i].name);
        lcase(nameBuff[1]);

        if (strcmp(nameBuff[0], nameBuff[1]) == 0)
			return(i);

	}

	return(PACK_ERR_NOTFOUND);

}