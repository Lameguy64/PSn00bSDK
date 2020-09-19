#ifndef _ISOFS_H
#define _ISOFS_H

#pragma pack(push, 1)

/// Structure of a double-endian unsigned short word
typedef struct ISO_USHORT_PAIR
{
	unsigned short	lsb;	/// LSB format 16-bit word
	unsigned short	msb;	/// MSB format 16-bit word
} ISO_USHORT_PAIR;

/// Structure of a double-endian unsigned int word
typedef struct ISO_UINT_PAIR
{
	unsigned int	lsb;		/// LSB format 32-bit word
	unsigned int	msb;		/// MSB format 32-bit word
} ISO_UINT_PAIR;

/// ISO descriptor header structure
typedef struct ISO_DESCRIPTOR_HEADER
{
	unsigned char	type;		/// Volume descriptor type (1 is descriptor, 255 is descriptor terminator)
	char			id[5];		/// Volume descriptor ID (always CD001)
	unsigned short	version;	/// Volume descriptor version (always 0x01)
} ISO_DESCRIPTOR_HEADER;

/// Structure of a date stamp for ISO_DIR_ENTRY structure
typedef struct ISO_DATESTAMP
{
	unsigned char	year;		/// number of years since 1900
	unsigned char	month;		/// month, where 1=January, 2=February, etc.
	unsigned char	day;		/// day of month, in the range from 1 to 31
	unsigned char	hour;		/// hour, in the range from 0 to 23
	unsigned char	minute;		/// minute, in the range from 0 to 59
	unsigned char	second;		/// Second, in the range from 0 to 59
	unsigned char	GMToffs;	/// Greenwich Mean Time offset
} ISO_DATESTAMP;

/// Structure of an ISO path table entry (specifically for the cd::IsoReader class)
typedef struct ISO_PATHTABLE_ENTRY
{
	unsigned char nameLength;	/// Name length (or 1 for the root directory)
	unsigned char extLength;	/// Number of sectors in extended attribute record
	unsigned int dirOffs;		/// Number of the first sector in the directory, as a double word
	unsigned short dirLevel;	/// Index of the directory record's parent directory
								/// If nameLength is odd numbered, a padding byte will be present after the identifier text.
} ISO_PATHTABLE_ENTRY;

typedef struct ISO_DIR_ENTRY
{
	unsigned char entryLength;			// Directory entry length (variable, use for parsing through entries)
	unsigned char extLength;			// Extended entry data length (always 0)
	ISO_UINT_PAIR entryOffs;			// Points to the LBA of the file/directory entry
	ISO_UINT_PAIR entrySize;			// Size of the file/directory entry
	ISO_DATESTAMP entryDate;			// Date & time stamp of entry
	unsigned char flags;				// File flags (0x02 for directories, 0x00 for files)
	unsigned char fileUnitSize;			// Unit size (usually 0 even with Form 2 files such as STR/XA)
	unsigned char interleaveGapSize;	// Interleave gap size (usually 0 even with Form 2 files such as STR/XA)
	ISO_USHORT_PAIR volSeqNum;			// Volume sequence number (always 1)
	unsigned char identifierLen;		// Identifier (file/directory name) length in bytes
} ISO_DIR_ENTRY;

typedef struct ISO_ROOTDIR_HEADER
{
	unsigned char entryLength;			// Always 34 bytes
	unsigned char extLength;			// Always 0
	ISO_UINT_PAIR entryOffs;			// Should point to LBA 22
	ISO_UINT_PAIR entrySize;			// Size of entry extent
	ISO_DATESTAMP entryDate;			// Record date and time
	unsigned char flags;				// File flags
	unsigned char fileUnitSize;
	unsigned char interleaveGapSize;
	ISO_USHORT_PAIR volSeqNum;
	unsigned char identifierLen;		// 0x01
	unsigned char identifier;			// 0x00
} ISO_ROOTDIR_HEADER;

// ISO descriptor structure
typedef struct ISO_DESCRIPTOR
{

	// ISO descriptor header
	ISO_DESCRIPTOR_HEADER header;
	// System ID (always PLAYSTATION)
	char systemID[32];
	// Volume ID (or label, can be blank or anything)
	char volumeID[32];
	// Unused null bytes
	unsigned char pad2[8];
	// Size of volume in sector units
	ISO_UINT_PAIR volumeSize;
	// Unused null bytes
	unsigned char pad3[32];
	// Number of discs in this volume set (always 1 for single volume)
	ISO_USHORT_PAIR volumeSetSize;
	// Number of this disc in volume set (always 1 for single volume)
	ISO_USHORT_PAIR volumeSeqNumber;
	// Size of sector in bytes (always 2048 bytes)
	ISO_USHORT_PAIR sectorSize;
	// Path table size in bytes (applies to all the path tables)
	ISO_UINT_PAIR pathTableSize;
	// LBA to Type-L path table
	unsigned int	pathTable1Offs;
	// LBA to optional Type-L path table (usually a copy of the primary path table)
	unsigned int	pathTable2Offs;
	// LBA to Type-L path table but with MSB format values
	unsigned int	pathTable1MSBoffs;
	// LBA to optional Type-L path table but with MSB format values (usually a copy of the main path table)
	unsigned int	pathTable2MSBoffs;
	// Directory entry for the root directory (similar to a directory entry)
	ISO_ROOTDIR_HEADER	rootDirRecord;
	// Volume set identifier (can be blank or anything)
	char	volumeSetIdentifier[128];
	// Publisher identifier (can be blank or anything)
	char	publisherIdentifier[128];
	// Data preparer identifier (can be blank or anything)
	char	dataPreparerIdentifier[128];
	// Application identifier (always PLAYSTATION)
	char	applicationIdentifier[128];
	// Copyright file in the file system identifier (can be blank or anything)
	char	copyrightFileIdentifier[37];
	// Abstract file in the file system identifier (can be blank or anything)
	char	abstractFileIdentifier[37];
	// Bibliographical file identifier in the file system (can be blank or anything)
	char	bibliographicFilelIdentifier[37];
	// Volume create date (in text format YYYYMMDDHHMMSSMMGG)
	char	volumeCreateDate[17];
	// Volume modify date (in text format YYYYMMDDHHMMSSMMGG)
	char	volumeModifyDate[17];
	// Volume expiry date (in text format YYYYMMDDHHMMSSMMGG)
	char	volumeExpiryDate[17];
	// Volume effective date (in text format YYYYMMDDHHMMSSMMGG)
	char	volumeEffeciveDate[17];
	// File structure version (always 1)
	unsigned char	fileStructVersion;
	// Padding
	unsigned char	dummy0;
	// Application specific data (says CD-XA001 at [141], the rest are null bytes)
	unsigned char	appData[512];
	// Padding
	unsigned char	pad4[653];

} ISO_DESCRIPTOR;

// Leave non-aligned structure packing
#pragma pack(pop)

#endif /* _ISOFS_H */