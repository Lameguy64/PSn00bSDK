#ifndef _QLP_H
#define _QLP_H

#include <stdint.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define PACK_ERR_NONE			0
#define PACK_ERR_INVALID		-1
#define PACK_ERR_NOTFOUND		-2
#define PACK_ERR_INCOMPLETE		-3
#define PACK_ERR_READ_FAULT		-4

typedef struct {
	char		id[3];
	uint8_t		numfiles;
} QLP_HEAD;

typedef struct {
	char		name[16];
	uint32_t	size;
	uint32_t	offs;
} QLP_FILE;

int qlpFileCount(const QLP_HEAD* qlpfile);
const QLP_FILE* qlpFileEntry(int index, const QLP_HEAD* qlpfile);
const void* qlpFileAddr(int index, const QLP_HEAD* qlpfile);
int qlpFindFile(char* fileName, const QLP_HEAD* qlpfile);

#endif // _QLP_H