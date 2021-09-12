/*!	\file lzp.h
 *	\brief Main library header
 */

/*!	\mainpage
 *	\version	0.20b
 *	\author		John Wilbert 'Lameguy64' Villamor
 *
 * 	\section	creditsSection Credits
 * 	- LZ77 data compression/decompression routines based from Ilya Muravyov's
 *	  crush.cpp released under public domain. Refined and ported to C by Lameguy64.
 *	- CRC calculation routines based from Lammert Bies' lib_crc routines.
 *
 */

#ifndef _LZPACK_H
#define _LZPACK_H

#include <sys/types.h>
#ifdef _WIN32
#include <windows.h>
#endif

/*!	\addtogroup crcBaseRemainders CRC Base Remainder Values
 *	@{
 */
//! Initial remainder value for lzCRC16()
#define LZP_CRC16_REMAINDER		0x0000
//! Initial remainder value for lzCRC32()
#define LZP_CRC32_REMAINDER		0xFFFFFFFF
/*! @} */


/*!	\addtogroup compLevels Compression Levels
 *	\brief Compression levels for the lzCompress() function.
 *	@{
 */
//! Minimal (but fast) compression
#define LZP_COMPRESS_FAST	0
//! Normal compression level
#define LZP_COMPRESS_NORMAL	1
//! Maximum compression level
#define LZP_COMPRESS_MAX	2
/*!	@}	*/


/*!	\addtogroup libraryErrorCodes Library Error Codes
 *	@{
 */
//! No error
#define LZP_ERR_NONE			0
//! Decompression error
#define LZP_ERR_DECOMPRESS		-1
//! Not a valid LZP/QLP/PCK archive
#define LZP_ERR_INVALID_PACK	-2
//! File not found
#define LZP_ERR_NOTFOUND		-3
//! CRC check mismatch (data corruption)
#define LZP_ERR_CRC_MISMATCH	-4
/*! @} */


//! Header structure of an LZP format archive file
typedef struct {

	//! File ID (must always be 'LZP')
	char	id[3];
	//! File count
	u_char	numFiles;

} LZP_HEAD;

//! File entry structure for an LZP format archive file
typedef struct {

	//! File name
    char	fileName[16];
    //! CRC32 checksum of file
    u_int	crc;
    //! Original size of file in bytes
    u_int	fileSize;
    //! Compressed size of file
    u_int	packedSize;
    //! File data offset
    u_int	offset;

} LZP_FILE;


// Function prototypes
#ifdef __cplusplus
extern "C" {
#endif


/*!	\addtogroup compressFuncs Data Compression and Decompression Functions
 *	\brief Functions to compress and decompress data.
 *	@{
 */

/*! Compress a block of data.
 *
 *	\details This function compresses a specified block of data in LZ77 encoding.
 *	Depending on the size of the input data and speed of the computer, compression
 *	may take a while to complete.
 *
 *	\param[out]	*outBuff	Pointer to buffer to store compressed data.
 *	\param[in]	*inBuff		Pointer to data to compress.
 *	\param[in]	inSize		Size of data to compress in bytes.
 *	\param[in]	level		Compression level (see \ref compLevels).
 *
 *	\returns The size of the compressed data in bytes.
 */
int lzCompress(void* outBuff, void* inBuff, int inSize, int level);

/*! Decompress a compressed block of data.
 *
 *  \details Decompressed a compressed block of data produced by lzCompress(). It cannot
 *	return the decompressed size of the data ahead of time so you must preserve the decompressed
 *	size of the data yourself.
 *
 *	\note The decompression algorithm used in this function is completely independent
 *	of the compression settings set by lzSetHashSizes() before compressing the data with
 *	lzCompress().
 *
 *	\param[out]	*outBuff	Pointer to buffer to store decompressed data.
 *	\param[in]	*inBuff		Pointer to compressed data to decompress.
 *	\param[in]	inSize		Compressed data size in bytes.
 *
 *	\returns Size of decompressed data in bytes or LZP_ERR_DECOMPRESS if a
 *	decompression error occurred.
 */
int lzDecompress(void* outBuff, void* inBuff, int inSize);

int lzDecompressLen(void* outBuff, int outSize, void* inBuff, int inSize);

/*! Sets the sizes of hash tables for data compression.
 *
 *	\param[in]	window      Sliding window size.
 *	\param[in]	hash1		Hash table 1 size.
 *	\param[in]	hash2		Hash table 2 size.
 */
void lzSetHashSizes(int window, int hash1, int hash2);

/*! Reset the sizes of hash tables to their defaults.
 */
void lzResetHashSizes();

/*!	@}	*/


/*!	\addtogroup crcFuncs CRC Hashing Functions
 *	\brief Functions to calculate CRC hashes of data.
 *	@{
 */

/*! Calculates a CRC16 hash of the specified buffer.
 *
 *	\param[in]	*buff	Pointer to buffer to calculate a hash of.
 *	\param[in]	bytes	Size of buffer in bytes.
 *	\param[in]	crc		CRC remainder (use LZP_CRC16_REMAINDER).
 *
 *	\returns CRC16 hash of specified buffer.
 */
unsigned short lzCRC16(void* buff, int bytes, unsigned short crc);

/*!	Calculates a CRC32 hash of the specified buffer.
 *
 *	\param[in]	*buff	Pointer to buffer to calculate a hash of.
 *	\param[in]	bytes	Size of buffer in bytes.
 *	\param[in]	crc		CRC remainder (use LZP_CRC16_REMAINDER).
 *
 *	\returns CRC32 hash of specified buffer.
 */
unsigned int lzCRC32(void* buff, int bytes, unsigned int crc);

/*!	@}	*/


/*!	\addtogroup lzpFunctions	LZP Archive Handling Routines
 *	\brief Functions to index and unpack files from LZP archives.
 *	@{
 */

/*! Searches for a file by name in an LZP archive and returns a file entry number.
 *
 *	\param[in]	*fileName	String of file to search (must be less than 13 characters).
 *	\param[in]	*lzpack		Pointer to LZP archive file.
 *
 *	\returns File index of found file or one of \ref libraryErrorCodes if an error occurred.
 */
int lzpSearchFile(const char* fileName, void* lzpack);

int lzpFileSize(void* lzpack, int fileNum);

/*! Get a pointer to a file entry inside of an LZP archive.
 *
 *  \param[in]	*lzpack		Pointer to LZP archive file.
 *	\param[in]	fileNum		File number to get an entry of (you may use lzpSearchFile()).
 *
 *	\returns A pointer to an LZP_FILE struct or NULL if an error occurred.
 */
LZP_FILE* lzpFileEntry(void* lzpack, int fileNum);

/*! Unpacks a file from an LZP archive to the specified memory buffer.
 *
 *	\param[in]	*buff	Pointer to buffer to store unpacked file.
 *	\param[in]	*lzpack	Pointer to LZP archive file.
 *	\param[in]	fileNum	File entry number of file to extract (you may use lzpSearchFile()).
 *
 *	\returns Size of decompressed file in bytes or one of \ref libraryErrorCodes if an error occurred.
 */
int lzpUnpackFile(void* buff, void* lzpack, int fileNum);

/*!	@}	*/


#ifdef __cplusplus
}
#endif


#endif // _LZPACK_H
