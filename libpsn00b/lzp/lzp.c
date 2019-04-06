#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lzp.h"


static char* lcase(char* text) {

    int i;

    for(i=0; text[i]!=0x00; i++)
        text[i] = tolower(text[i]);

	return(text);

}


int lzpSearchFile(const char* fileName, void* lzpack) {

	int			i;
    char		searchName[16];
	char		compareName[16];
	LZP_FILE*	fileEntry;

	strcpy(searchName, fileName);
	lcase(searchName);

	fileEntry = (LZP_FILE*)(lzpack+4);
	for(i=0; i<((LZP_HEAD*)lzpack)->numFiles; i++) {

        strcpy(compareName, fileEntry[i].fileName);
        lcase(compareName);

        if (strcmp(searchName, compareName) == 0)
			return(i);

	}

    return(LZP_ERR_NOTFOUND);

}

LZP_FILE* lzpFileEntry(void* lzpack, int fileNum) {

	if (strncmp("LZP", ((LZP_HEAD*)lzpack)->id, 3) != 0)
        return(NULL);

	if ((fileNum < 0) || (fileNum > (((LZP_HEAD*)lzpack)->numFiles-1)))
		return(NULL);

    return(&((LZP_FILE*)(lzpack+4))[fileNum]);

}

int lzpFileSize(void* lzpack, int fileNum) {
	
	if (strncmp("LZP", ((LZP_HEAD*)lzpack)->id, 3) != 0)
        return 0;
	
	if ((fileNum < 0) || (fileNum > (((LZP_HEAD*)lzpack)->numFiles-1)))
		return 0;
	
	return ((LZP_FILE*)(lzpack+4))[fileNum].fileSize;
}

int lzpUnpackFile(void* buff, void* lzpack, int fileNum) {

	LZP_FILE*	fileEntry = &((LZP_FILE*)(lzpack+4))[fileNum];
	int			unpackedSize;

	// Check ID header
    if (strncmp("LZP", ((LZP_HEAD*)lzpack)->id, 3) != 0)
        return(LZP_ERR_INVALID_PACK);

	// Do a CRC16 check of the compressed data's integrity
	if (lzCRC32(lzpack+fileEntry->offset, fileEntry->packedSize, LZP_CRC32_REMAINDER) != fileEntry->crc)
		return(LZP_ERR_CRC_MISMATCH);

	// Decompress data to the specified address
	unpackedSize = lzDecompress(buff, lzpack+fileEntry->offset, fileEntry->packedSize);
	if (unpackedSize < 0)
		return(unpackedSize);

	return(unpackedSize);

}
