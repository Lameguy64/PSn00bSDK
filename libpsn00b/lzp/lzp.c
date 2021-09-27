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


int lzpSearchFile(const char* fileName, const LZP_HEAD* lzpack) {

	int			i;
    char		searchName[16];
	char		compareName[16];
	LZP_FILE*	fileEntry;

	strcpy(searchName, fileName);
	lcase(searchName);

	fileEntry = (LZP_FILE*)(((const char*)lzpack)+sizeof(LZP_HEAD));
	for(i=0; i<(lzpack->numFiles); i++) {

        strcpy(compareName, fileEntry[i].fileName);
        lcase(compareName);

        if (strcmp(searchName, compareName) == 0)
			return(i);

	}

    return(LZP_ERR_NOTFOUND);

}

const LZP_FILE* lzpFileEntry(const LZP_HEAD* lzpack, int fileNum) {

	if (strncmp("LZP", lzpack->id, 3) != 0)
        return(NULL);

	if ((fileNum < 0) || (fileNum > (lzpack->numFiles-1)))
		return(NULL);

    return &((LZP_FILE*)(((const char*)lzpack)+sizeof(LZP_HEAD)))[fileNum];

}

int lzpFileSize(const LZP_HEAD* lzpack, int fileNum) {
	
	if (strncmp("LZP", lzpack->id, 3) != 0)
        return 0;
	
	if ((fileNum < 0) || (fileNum > (lzpack->numFiles-1)))
		return 0;
	
	return ((LZP_FILE*)(((const char*)lzpack)+sizeof(LZP_HEAD)))[fileNum].fileSize;
}

int lzpUnpackFile(void* buff, const LZP_HEAD* lzpack, int fileNum) {

	LZP_FILE*	fileEntry = &((LZP_FILE*)(((const char*)lzpack)+sizeof(LZP_HEAD)))[fileNum];
	int			unpackedSize;

	// Check ID header
    if (strncmp("LZP", lzpack->id, 3) != 0)
        return(LZP_ERR_INVALID_PACK);

	// Do a CRC16 check of the compressed data's integrity
	if (lzCRC32(((const char*)lzpack)+fileEntry->offset, fileEntry->packedSize, LZP_CRC32_REMAINDER) != fileEntry->crc)
		return(LZP_ERR_CRC_MISMATCH);

	// Decompress data to the specified address
	unpackedSize = lzDecompress(buff, ((const char*)lzpack)+fileEntry->offset, fileEntry->packedSize);
	if (unpackedSize < 0)
		return(unpackedSize);

	return(unpackedSize);

}
