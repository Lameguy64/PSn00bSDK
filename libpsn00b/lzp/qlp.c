#include <stddef.h>
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

int qlpFileCount(const QLP_HEAD* qlpfile) {

    if (strncmp(qlpfile->id, "QLP", 3) != 0)
		return(PACK_ERR_INVALID);

    return(qlpfile->numfiles);

}

const QLP_FILE* qlpFileEntry(int index, const QLP_HEAD* qlpfile) {

	if (strncmp(qlpfile->id, "QLP", 3) != 0)
		return(NULL);

	if (index > qlpfile->numfiles)
		return(NULL);

	return(&((QLP_FILE*)(((const char*)qlpfile)+sizeof(QLP_HEAD)))[index]);

}

const void* qlpFileAddr(int index, const QLP_HEAD* qlpfile) {

	return( ((const char*)qlpfile)+((QLP_FILE*)(((const char*)qlpfile)+sizeof(QLP_HEAD)))[index].offs );

}

int qlpFindFile(char* fileName, const QLP_HEAD* qlpfile) {

	int i;
	char nameBuff[2][16];

    strcpy(nameBuff[0], fileName);
    lcase(nameBuff[0]);

	for(i=0; i<(qlpfile->numfiles); i++) {

		strcpy(nameBuff[1], ((QLP_FILE*)(((const char*)qlpfile)+sizeof(QLP_HEAD)))[i].name);
        lcase(nameBuff[1]);

        if (strcmp(nameBuff[0], nameBuff[1]) == 0)
			return(i);

	}

	return(PACK_ERR_NOTFOUND);

}