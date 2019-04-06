#ifndef _SYS_FCNTL_H
#define _SYS_FCNTL_H

// File control mode flags for BIOS file functions
// (many weren't documented in nocash docs)
#define FREAD			0x1			// Read
#define FWRITE			0x2			// Write
#define FNBLOCK			0x4			// Non-blocking read access
#define FRLOCK			0x10		// Read lock
#define FWLOCK			0x20		// Write lock
#define FAPPEND			0x100		// Append
#define FCREATE			0x200		// Create if not exist
#define FTRUNC			0x400		// Truncate to zero length
#define FSCAN			0x2000		// Scanning type
#define FRCOM			0x2000		// Remote command entry
#define FNBUF			0x4000		// No ring buffer and terminal interrupt
#define FASYNC			0x8000		// Asynchronous I/O
#define FNBLOCKS(a)		(a<<16)		// Number of blocks? (from nocash docs)

#endif