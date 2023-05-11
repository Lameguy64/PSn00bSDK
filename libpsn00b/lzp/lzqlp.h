/*
 * liblzp data compression library
 * (C) 2019 Lameguy64 - MPL licensed
 */

/**
 * @file lzqlp.h
 * @brief Utility library for file bundling
 *
 * @details This library implements a simple in-memory archive format which
 * can be used to package and compress assets for faster loading, as well as a
 * generic LZ77 compressor and matching decompressor. Two archive formats are
 * supported, one uncompressed (.QLP) and one with individually compressed
 * entries (.LZP).
 *
 * This header provides functions to parse .QLP archives and retrieve pointers
 * to their contents after they have been loaded into memory.
 */

#pragma once

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


// Function prototypes
#ifdef __cplusplus
extern "C" {
#endif

int qlpFileCount(const QLP_HEAD* qlpfile);
const QLP_FILE* qlpFileEntry(int index, const QLP_HEAD* qlpfile);
const void* qlpFileAddr(int index, const QLP_HEAD* qlpfile);
int qlpFindFile(char* fileName, const QLP_HEAD* qlpfile);

#ifdef __cplusplus
}
#endif
