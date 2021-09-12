#ifndef _QLP_H
#define _QLP_H

#define PACK_ERR_NONE			0
#define PACK_ERR_INVALID		-1
#define PACK_ERR_NOTFOUND		-2
#define PACK_ERR_INCOMPLETE		-3
#define PACK_ERR_READ_FAULT		-4

typedef struct {
	char			id[3];
	unsigned char	numfiles;
} QLP_HEAD;

typedef struct {
	char			name[16];
	unsigned int	size;
	unsigned int	offs;
} QLP_FILE;

int qlpFileCount(void* qlpfile);
QLP_FILE* qlpFileEntry(int index, void* qlpfile);
void* qlpFileAddr(int index, void* qlpfile);
int qlpFindFile(char* fileName, void* qlpfile);

#endif // _QLP_H