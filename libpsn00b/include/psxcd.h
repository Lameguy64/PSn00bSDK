/*
 * PSn00bSDK CD-ROM drive library
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 */

/**
 * @file psxcd.h
 * @brief CD-ROM library header
 *
 * @details The PSn00bSDK CD-ROM library provides facilities for using the
 * CD-ROM hardware of the PS1. Unlike the CD-ROM library of the official SDK,
 * psxcd is immune to the 30 file and directory limit and is capable of parsing
 * directories containing as many files as the ISO9660 file system can support,
 * unless the records are too large to be loaded into memory. However, to
 * maintain compatibility with the PS1 BIOS, the root directory must not exceed
 * the 30 file limit and the entire disc should contain no more than 45
 * directories total.
 *
 * Whilst psxcd is not constrained by the 30 file per directory limit, it does
 * not support Joliet CD-ROM extensions to support long file names. However, a
 * library extension is considered for future development.
 */

#ifndef __PSXCD_H
#define __PSXCD_H

#include <stdint.h>

/* Enum definitions */

typedef enum _CdlCommand {
	CdlNop			= 0x01,
	CdlSetloc		= 0x02,
	CdlPlay			= 0x03,
	CdlForward		= 0x04,
	CdlBackward		= 0x05,
	CdlReadN		= 0x06,
	CdlStandby		= 0x07,
	CdlStop			= 0x08,
	CdlPause		= 0x09,
	CdlInit			= 0x0a,
	CdlMute			= 0x0b,
	CdlDemute		= 0x0c,
	CdlSetfilter	= 0x0d,
	CdlSetmode		= 0x0e,
	CdlGetparam		= 0x0f,
	CdlGetlocL		= 0x10,
	CdlGetlocP		= 0x11,
	CdlSetsession	= 0x12,
	CdlGetTN		= 0x13,
	CdlGetTD		= 0x14,
	CdlSeekL		= 0x15,
	CdlSeekP		= 0x16,
	CdlTest			= 0x19,
	CdlGetID		= 0x1a,
	CdlReadS		= 0x1b,
	CdlReset		= 0x1c
} CdlCommand;

typedef enum _CdlStatFlag {
	CdlStatError		= 1 << 0,
	CdlStatStandby		= 1 << 1,
	CdlStatSeekError	= 1 << 2,
	CdlStatIdError		= 1 << 3,
	CdlStatShellOpen	= 1 << 4,
	CdlStatRead			= 1 << 5,
	CdlStatSeek			= 1 << 6,
	CdlStatPlay			= 1 << 7
} CdlStatFlag;

typedef enum _CdlModeFlag {
	CdlModeDA		= 1 << 0,
	CdlModeAP		= 1 << 1,
	CdlModeRept		= 1 << 2,
	CdlModeSF		= 1 << 3,
	//CdlModeSize0	= 1 << 4,
	//CdlModeSize1	= 1 << 5,
	CdlModeIgnore	= 1 << 4,
	CdlModeSize		= 1 << 5,
	CdlModeRT		= 1 << 6,
	CdlModeSpeed	= 1 << 7
} CdlModeFlag;

typedef enum _CdlIntrResult {
	CdlNoIntr		= 0,
	CdlDataReady	= 1,
	CdlComplete		= 2,
	CdlAcknowledge	= 3,
	CdlDataEnd		= 4,
	CdlDiskError	= 5
} CdlIntrResult;

typedef enum _CdlIsoError {
	CdlIsoOkay		= 0,
	CdlIsoSeekError	= 1,
	CdlIsoReadError	= 2,
	CdlIsoInvalidFs	= 3,
	CdlIsoLidOpen	= 4
} CdlIsoError;

/**
 * @brief Translates a BCD format value to decimal
 *
 * @details Translates a specified value in BCD format (ie. 32/0x20 = 20) into
 * a decimal integer, as the CD-ROM controller returns integer values only in
 * BCD format.
 */
#define btoi(b) ((b)/16*10+(b)%16)

/**
 * @brief Translates a decimal value to BCD
 *
 * @details Translates a decimal integer into a BCD format value (ie.
 * 20 = 32/0x20), as the CD-ROM controller only accepts values in BCD format.
 */
#define itob(i) ((i)/10*16+(i)%10)

/* Structure and type definitions */

/**
 * @brief CD-ROM positional coordinates
 *
 * @details This structure is used to specify CD-ROM positional coordinates for
 * CdlSetloc, CdlReadN and CdlReadS CD-ROM commands. Use CdIntToPos() to set
 * parameters from a logical sector number.
 *
 * @see CdIntToPos(), CdControl()
 */
typedef struct _CdlLOC {
	uint8_t minute;	// Minutes (BCD)
	uint8_t second;	// Seconds (BCD)
	uint8_t sector;	// Sector or frame (BCD)
	uint8_t track;	// Track number (not used)
} CdlLOC;

/**
 * @brief CD-ROM attenuation parameters
 *
 * @details This structure specifies parameters for the CD-ROM attenuation.
 * Values must be of range 0 to 127.
 *
 * The CD-ROM attenuation can be used to set the CD-ROM audio output to mono
 * (0x40, 0x40, 0x40, 0x40) or reversed stereo (0x00, 0x80, 0x00, 0x80). It can
 * also be used to play one of two stereo channels to both speakers.
 *
 * The CD-ROM attenuation affects CD-DA and CD-XA audio.
 *
 * @see CdMix()
 */
typedef struct _CdlATV {
	uint8_t val0;	// CD to SPU L-to-L volume
	uint8_t val1;	// CD to SPU L-to-R volume
	uint8_t val2;	// CD to SPU R-to-R volume
	uint8_t val3;	// CD to SPU R-to-L volume
} CdlATV;

/**
 * @brief File entry structure
 *
 * @details Used to store basic information of a file such as logical block
 * location and size. Currently, CdSearchFile() is the only function that uses
 * this struct but it will be used in directory listing functions that may be
 * implemented in the future.
 *
 * @see CdSearchFile()
 */
typedef struct _CdlFILE {
	CdlLOC		pos;		// CD-ROM position coordinates of file
	uint32_t	size;		// Size of file in bytes
	char		name[16];	// File name
} CdlFILE;

/**
 * @brief Structure used to set CD-ROM XA filter
 *
 * @details This structure is used to specify stream filter parameters for
 * CD-ROM XA audio streaming using the CdlSetfilter command. This only affects
 * CD-ROM XA audio streaming.
 *
 * CD-ROM XA audio is normally comprised of up to 8 or more ADPCM compressed
 * audio streams interleaved into one continuous stream of data. The data
 * stream is normally read at 2x speed but only one of eight XA audio streams
 * can be played at a time. The XA stream to play is specified by the
 * CdlSetfilter command and this struct.
 *
 * The CD-ROM XA filter can be changed during CD-ROM XA audio playback with
 * zero audio interruption. This can be used to achieve dynamic music effects
 * by switching to alternate versions of a theme to fit specific scenes
 * seamlessly.
 *
 * @see CdControl()
 */
typedef struct _CdlFILTER {
	uint8_t		file;	// File number to fetch (usually 1)
	uint8_t		chan;	// Channel number (0 through 7)
	uint16_t	pad;	// Padding
} CdlFILTER;

/**
 * @brief CD-ROM directory query context handle
 *
 * @details Used to store a directory context created by CdOpenDir(). An open
 * context can then be used with CdReadDir() and closed with CdCloseDir().
 *
 * @see CdOpenDir()
 */
typedef void *CdlDIR;

typedef void (*CdlCB)(int, uint8_t *);

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the CD-ROM library
 *
 * @details Initializes the CD-ROM subsystem which includes hooking the
 * required IRQ handler, sets up internal variables of the CD-ROM library and
 * attempts to initialize the CD-ROM controller. The mode parameter does
 * nothing but may be used in future updates of this library.
 *
 * This function must be called after ResetGraph and before any other CD-ROM
 * library function that interfaces with the CD-ROM controller. This function
 * may not be called twice as it may cause instability or would just crash.
 *
 * @return Always 1. May change in the future.
 */
int CdInit(void);

/**
 * @brief Translates a logical sector number to CD-ROM positional coordinates
 *
 * @details This function translates the logical sector number from i to CD-ROM
 * positional coordinates stored to a CdlLOC struct specified by p. The
 * translation takes the lead-in offset into account so the first logical
 * sector begins at 0 and the result will be offset by 150 sectors.
 *
 * @param i Logical sector number
 * @param p Pointer to a CdlLOC structure
 * @return Pointer to the specified CdlLOC struct plus 150 sectors.
 */
CdlLOC* CdIntToPos(int i, CdlLOC *p);

/**
 * @brief Translates CD-ROM positional coordinates to a logical sector number
 *
 * @details Translates the CD-ROM position parameters from a CdlLOC struct
 * specified by p to a logical sector number. The translation takes the lead-in
 * offset of 150 sectors into account so the logical sector number returned
 * would begin at zero.
 *
 * @param p Pointer to a CdlLOC struct
 * @return Logical sector number minus the 150 sector lead-in.
 */
int CdPosToInt(const CdlLOC *p);

/**
 * @brief Gets CD-ROM TOC information
 *
 * @details Retrieves the track entries from a CD's table of contents (TOC). The
 * function can return up to 99 track entries, which is the maximum number of
 * audio tracks the CD standard supports.
 *
 * This function only retrieve the minutes and seconds of an audio track's
 * position as the CD-ROM controller only returns the minutes and seconds of a
 * track, which may result in the end of the previous track being played
 * instead of the intended track to be played. This can be remedied by having a
 * 2 second pregap on each audio track on your disc.
 *
 * @param toc Pointer to an array of CdlLOC entries
 * @return Number of tracks on the disc, zero on error.
 *
 * @see CdControl()
 */
int CdGetToc(CdlLOC *toc);

/**
 * @brief Issues a control command to the CD-ROM controller
 *
 * @details Sends a CD-ROM command specified by com to the CD-ROM controller,
 * waits for an acknowledge interrupt (very fast) then returns. It will also
 * issue parameters from param to the CD-ROM controller if the command accepts
 * parameters. Response data from the CD-ROM controller is stored to result on
 * commands that produce response data.
 *
 * Because this function waits for an acknowledge interrupt from the CD-ROM
 * controller, this function should not be used in a callback. Instead, use
 * CdControlF().
 *
 * Commands that are blocking require the use of CdSync() to wait for the
 * command to fully complete.
 *
 * CD-ROM control commands:
 *
 * | Command       | Value | Parameter | Blocking | Description                                                                                                        |
 * | :------------ | ----: | :-------- | :------- | :----------------------------------------------------------------------------------------------------------------- |
 * | CdlNop        |  0x01 |           | No       | Also known as Getstat. Normally used to query the CD-ROM status, which is retrieved using CdStatus().              |
 * | CdlSetloc     |  0x02 | CdlLOC    | No       | Sets the seek target location, but does not seek. Actual seeking begins upon issuing a seek or read command.       |
 * | CdlPlay       |  0x03 | uint8_t   | No       | Begins CD Audio playback. Parameter specifies an optional track number to play (some emulators do not support it). |
 * | CdlForward    |  0x04 |           | No       | Fast forward (CD Audio only), issue CdlPlay to stop fast forward.                                                  |
 * | CdlBackward   |  0x05 |           | No       | Rewind (CD Audio only), issue CdlPlay to stop rewind.                                                              |
 * | CdlReadN      |  0x06 | CdlLOC    | No       | Begin reading data sectors. Used in conjunction with CdReadCallback().                                             |
 * | CdlStandby    |  0x07 |           | Yes      | Also known as MotorOn, starts CD motor and remains idle.                                                           |
 * | CdlStop       |  0x08 |           | Yes      | Stops playback and the disc itself.                                                                                |
 * | CdlPause      |  0x09 |           | Yes      | Stops playback or data reading, but leaves the disc on standby.                                                    |
 * | CdlInit       |  0x0A |           | Yes      | Initialize the CD-ROM controller.                                                                                  |
 * | CdlMute       |  0x0B |           | No       | Mutes CD audio (both DA and XA).                                                                                   |
 * | CdlDemute     |  0x0C |           | No       | Unmutes CD audio (both DA and XA).                                                                                 |
 * | CdlSetfilter  |  0x0D | CdlFILTER | No       | Set XA audio filter.                                                                                               |
 * | CdlSetmode    |  0x0E | uint8_t   | No       | Set CD-ROM mode.                                                                                                   |
 * | CdlGetparam   |  0x0F |           | No       | Returns current CD-ROM mode and file/channel filter settings.                                                      |
 * | CdlGetlocL    |  0x10 |           | No       | Returns current logical CD position, mode and XA filter parameters.                                                |
 * | CdlGetlocP    |  0x11 |           | No       | Returns current physical CD position (using SubQ location data).                                                   |
 * | CdlSetsession |  0x12 | uint8_t   | Yes      | Seek to specified session on a multi-session disc.                                                                 |
 * | CdlGetTN      |  0x13 |           | No       | Get CD-ROM track count.                                                                                            |
 * | CdlGetTD      |  0x14 | uint8_t   | No       | Get specified track position.                                                                                      |
 * | CdlSeekL      |  0x15 |           | Yes      | Logical seek to target position, set by last CdlSetloc command.                                                    |
 * | CdlSeekP      |  0x16 |           | Yes      | Physical seek to target position, set by last CdlSetloc command.                                                   |
 * | CdlTest       |  0x19 | (varies)  | Yes      | Special test command not disclosed to official developers (see nocash documents for more info).                    |
 * | CdlReadS      |  0x1B | CdlLOC    | No       | Begin reading data sectors without pausing for error correction.                                                   |
 *
 * CD-ROM return values:
 *
 * | Command     | 0     | 1     | 2      | 3    | 4       | 5       | 6    | 7      |
 * | :---------- | :---- | :---- | :----- | :--- | :------ | :------ | :--- | :----- |
 * | CdlGetparam | stat  | mode  | 0      | file | channel |         |      |        |
 * | CdlGetlocL  | amin  | asec  | aframe | mode | file    | channel | sm   | ci     |
 * | CdlGetlocP  | track | index | min    | sec  | frame   | amin    | asec | aframe |
 * | CdlGetTN    | stat  | first | last   |      |         |         |      |        |
 * | CdlGetTD    | stat  | min   | sec    |      |         |         |      |        |
 *
 * NOTE: Values are in BCD format.
 *
 * @param com Command value
 * @param param Command parameters
 * @param result Pointer of buffer to store result
 * @return 1 if the command was issued successfully. Otherwise 0 if a
 * previously issued command has not yet finished processing.
 *
 * @see CdSync(), CdControlF()
 */
int CdControl(uint8_t com, const void *param, uint8_t *result);

/**
 * @brief Issues a CD-ROM command to the CD-ROM controller (blocking)
 *
 * @details This function works just like CdControl(), but blocks on blocking
 * commands until said blocking command has completed.
 *
 * Because this function waits for an acknowledge interrupt from the CD-ROM
 * controller, this function should not be used in a callback. Use CdControlF()
 * instead.
 *
 * @param com Command value
 * @param param Command parameters
 * @param result Pointer of buffer to store result
 * @return 1 if the command was issued successfully. Otherwise 0 if a
 * previously issued command has not yet finished processing.
 *
 * @see CdControl(), CdControlF()
 */
int CdControlB(uint8_t com, const void *param, uint8_t *result);

/**
 * @brief Issues a CD-ROM command to the CD-ROM controller (does not block)
 *
 * @details This function works more or less the same as CdControl() but it
 * does not block even for the acknowledge interrupt from the CD-ROM
 * controller. Since this function is non-blocking it can be used in a callback
 * function.
 *
 * When using this function in a callback, a maximum of two commands can be
 * issued at once and only the first command can have parameters. This is
 * because the CD-ROM controller can only queue up to two commands and the
 * parameter FIFO is not cleared until the last command is acknowledged. But
 * waiting for acknowledgment in a callback is not possible.
 *
 * @param com Command value
 * @param param Command parameters
 * @return 1 if the command was issued successfully. Otherwise 0 if a
 * previously issued command has not yet finished processing.
 *
 * @see CdControl()
 */
int CdControlF(uint8_t com, const void *param);

/**
 * @brief Waits for blocking command or blocking status
 *
 * @details If mode is zero the function blocks if a blocking command was
 * issued earlier until the command has finished. If mode is non-zero the
 * function returns a command status value.
 *
 * A buffer specified by result will be set with the most recent CD-ROM status
 * value from the last command issued.
 *
 * @param mode Mode
 * @param result Pointer to store most recent CD-ROM status
 * @return Command status is returned as one of the following definitions:
 *
 * | Definition   | Description                 |
 * | :----------- | :-------------------------- |
 * | CdlComplete  | Command completed.          |
 * | CdlNoIntr    | No interrupt, command busy. |
 * | CdlDiskError | CD-ROM error occurred.      |
 *
 * @see CdControl()
 */
int CdSync(int mode, uint8_t *result);

/**
 * @brief Sets a callback function
 *
 * @details Sets a callback with the specified function func. The callback is
 * executed whenever a blocking command has completed.
 *
 * status is the CD-ROM status from the command that has completed processing.
 * *result corresponds to the *result parameter on CdControl()/CdControlB() and
 * returns the pointer to the buffer last set with that function.
 *
 * @param func Callback function
 * @return Pointer to last callback function set, or NULL if none was set.
 *
 * @see CdControl, CdControlB, CdSync
 */
uint32_t CdSyncCallback(CdlCB func);

/**
 * @brief Sets a callback function
 *
 * @details Sets a callback with the specified function func. The callback is
 * executed whenever there's an incoming data sector from the CD-ROM controller
 * during CdlReadN or CdlReadS. The pending sector data can be retrieved using
 * CdGetSector().
 *
 * status is the CD-ROM status code from the last CD command that has finished
 * processing. *result corresponds to the result pointer that was passed by the
 * last CdControl()/CdControlB() call.
 *
 * This callback cannot be used in conjunction with CdRead() because it also
 * uses this callback hook for its own internal use. The previously set
 * callback is restored after read completion however.
 *
 * @param func Callback function
 * @return Pointer to last callback function set, or NULL if none was set.
 *
 * @see CdControl(), CdControlB(), CdGetSector()
 */
int CdReadyCallback(CdlCB func);

/**
 * @brief Gets data from the CD-ROM sector buffered
 *
 * @details Reads sector data that is pending in the CD-ROM sector buffer and
 * stores it to *madr. Uses DMA to transfer the sector data and blocks very
 * briefly until said transfer completes.
 *
 * This function is intended to be called within a callback routine set using
 * CdReadyCallback() to fetch read data sectors from the CD-ROM sector buffer.
 * 
 * @param madr Pointer to memory buffer to store sector data
 * @param size Number of 32-bit words to retrieve
 * @return Always 1.
 *
 * @see CdReadyCallback()
 */
int CdGetSector(void *madr, int size);

/**
 * @brief Gets data from the CD-ROM sector buffered (non-blocking)
 *
 * @details Reads sector data that is pending in the CD-ROM sector buffer and
 * stores it to *madr. Uses DMA to transfer the sector data in the background
 * while keeping the CPU running (one word is transferred every 16 CPU cycles).
 * Note this is much slower than the blocking transfer performed by
 * CdGetSector().
 *
 * This function is intended to be called within a callback routine set using
 * CdReadyCallback() to fetch read data sectors from the CD-ROM sector buffer.
 * Since the transfer is asynchronous, CdDataSync() should be used to wait
 * until the whole sector has been read.
 * 
 * @param madr Pointer to memory buffer to store sector data
 * @param size Number of 32-bit words to retrieve
 * @return Always 1.
 *
 * @see CdReadyCallback(), CdDataSync()
 */
int CdGetSector2(void *madr, int size);

/**
 * @brief Waits for sector transfer to finish
 *
 * @details If mode is zero the function blocks until any sector DMA transfer
 * initiated by calling CdGetSector2() has finished. If mode is non-zero the
 * function returns a boolean value representing whether a transfer is
 * currently in progress.
 *
 * @param mode Mode
 * @return 0 if the transfer has finished, 1 if it is still in progress or -1
 * in case of a timeout.
 *
 * @see CdGetSector2()
 */
int CdDataSync(int mode);

/**
 * @brief Locates a file in the CD-ROM file system
 *
 * @details Searches a file specified by filename by path and name in the
 * CD-ROM file system and returns information of the file if found. The file
 * information acquired will be stored to loc.
 *
 * Directories can be separated with slashes (/) or backslashes (\), a leading
 * slash or backslash is optional but paths must be absolute. File version
 * identifier (;1) at the end of the file name is also optional. File and
 * directory names are case insensitive.
 *
 * The ISO9660 file system routines of libpsxcd do not support long file names
 * currently. Only MS-DOS style 8.3 file names are supported; extensions such
 * as Joliet and Rock Ridge are ignored.
 *
 * Upon calling this function for the first time, the ISO descriptor of the
 * disc is read and the whole path table is cached into memory. Next the
 * directory descriptor of the particular directory specified is loaded and
 * cached to locate the file specified. The directory descriptor is kept in
 * memory as long as the consecutive files to be searched are stored in the
 * same directory until a file in another directory is to be searched. On which
 * the directory descriptor is unloaded and a new directory descriptor is read
 * from the disc and cached. Therefore, locating files in the same directory is
 * faster as the relevant directory descriptor is already in memory and no disc
 * reads are issued.
 *
 * As of Revision 66 of PSn00bSDK, media change is detected by checking the
 * CD-ROM lid open status bit and attempting to acknowledge it with a CdlNop
 * command, to discriminate the status from an open lid or changed disc.
 *
 * @param loc Pointer to a CdlFILE struct to store file information
 * @param filename Path and name of file to locate
 * @return Pointer to the specified CdlFILE struct. Otherwise NULL is returned
 * when the file is not found.
 */
CdlFILE* CdSearchFile(CdlFILE *loc, const char *filename);

/**
 * @brief Reads sectors from the CD-ROM
 *
 * @details Reads a number sectors specified by sectors from the location set
 * by the last CdlSetloc command, the read sectors are then stored to a buffer
 * specified by buf. mode specifies the CD-ROM mode to use for the read
 * operation.
 *
 * The size of the sector varies depending on the sector read mode specified by
 * mode. For standard data sectors it is multiples of 2048 bytes. If
 * CdlModeSize0 is specified the sector size is 2328 bytes which includes the
 * whole sector minus sync, adress, mode and sub header bytes. CdlModeSize1
 * makes the sector size 2340 which is the entire sector minus sync bytes.
 * Ideally, CdlModeSpeed must be specified to read data sectors at double
 * CD-ROM speed.
 *
 * This function blocks very briefly to issue the necessary commands to start
 * CD-ROM reading. To determine if reading has completed use CdReadSync or
 * CdReadCallback.
 *
 * @param sectors Number of sectors to read
 * @param buf Pointer to buffer to store sectors read
 * @param mode CD-ROM mode for reading
 * @return Always returns 0 even on errors. This may change in future versions.
 *
 * @see CdReadSync(), CdReadCallback()
 */
int CdRead(int sectors, uint32_t *buf, int mode);

/**
 * @brief Waits for CD-ROM read completion or returns read status
 *
 * @details This function works more or less like CdSync() but for CdRead(). If
 * mode is zero the function blocks if CdRead() was issued earlier until
 * reading has completed. If mode is non-zero the function completes
 * immediately and returns number of sectors remaining.
 *
 * A buffer specified by result will be set with the most recent CD-ROM status
 * value from the last read issued.
 *
 * @param mode Mode
 * @param result Pointer to store most recent CD-ROM status
 * @return Number of sectors remaining. If reading is completed, 0 is returned.
 * On error, -1 is returned.
 *
 * @see CdRead()
 */
int CdReadSync(int mode, uint8_t *result);

/**
 * @brief Sets a callback function for read completion
 *
 * @details Works much the same as CdSyncCallback() but for CdRead(). Sets a
 * callback with the specified function func. The callback is executed whenever
 * a read operation initiated by CdRead() has completed.
 *
 * status is the CD-ROM status from the command that has completed processing.
 * *result points to a read result buffer.
 *
 * @param func Callback function
 * @return Pointer to last callback function set, or NULL if none was set.
 *
 * @see CdRead()
 */
uint32_t CdReadCallback(CdlCB func);

/**
 * @brief Gets the most recent CD-ROM status
 *
 * @details Returns the CD-ROM status since the last command issued. The status
 * value is updated by most CD-ROM commands.
 *
 * To get the current CD-ROM status you can issue CdlNop commands at regular
 * intervals to update the CD-ROM status this function returns.
 *
 * @return CD-ROM status from last comand issued.
 *
 * @see CdControl()
 */
int CdStatus(void);

/**
 * @brief Gets the last CD-ROM mode
 *
 * @details Returns the CD-ROM mode last set when issuing a CdlSetmode command.
 * The function returns instantly as it merely returns a value stored in an
 * internal variable.
 *
 * Since the value is simply a copy of what was specified from the last
 * CdlSetmode command, the mode value may become inaccurate if CdlInit or other
 * commands that affect the CD-ROM mode have been issued previously.
 *
 * @return Last CD-ROM mode value.
 */
int CdMode(void);

/**
 * @brief Sets CD-ROM mixer or attenuation
 *
 * @details Sets the CD-ROM attenuation parameters from a CdlATV struct
 * specified by vol. The CD-ROM attenuation settings are different from the SPU
 * CD-ROM volume.
 *
 * Normally used to configure CD and XA audio playback for mono or reverse
 * stereo output, though this was rarely used in practice.
 *
 * @param vol CD-ROM attenuation parameters
 * @return Always 1.
 */
int CdMix(const CdlATV *vol);

/**
 * @brief Opens a directory on the CD-ROM file system
 *
 * @details Opens a directory on the CD-ROM file system to read the contents of
 * a directory.
 *
 * A path name can use a slash (/) or backslash character (\) as the directory
 * name separator. The path must be absolute and should begin with a slash or
 * backslash. It should also not be prefixed with a device name (ie.
 * \MYDIR1\MYDIR2 will work but not cdrom:\MYDIR1\MYDIR2). The file system
 * routines in libpsxcd can query directory paths of up to 128 characters.
 *
 * The ISO9660 file system routines of libpsxcd do not support long file names
 * currently. Only MS-DOS style 8.3 file names are supported; extensions such
 * as Joliet and Rock Ridge are ignored.
 *
 * @param path Directory path to open
 * @return Pointer of a CdlDIR context, NULL if an error occurred.
 *
 * @see CdReadDir(), CdCloseDir()
 */
CdlDIR* CdOpenDir(const char* path);

/**
 * @brief Reads a directory entry from an open directory context
 *
 * @details Retrieves a file entry from an open directory context and stores it
 * to a CdlFILE struct specified by file. Repeated calls of this function
 * retrieves the next directory entry available until there are no more
 * directory entries that follow.
 *
 * @param dir Open directory context (from CdOpenDir())
 * @param file Pointer to a CdlFILE struct
 * @return 1 if there are proceeding directory entries that follow, otherwise 0.
 *
 * @see CdOpenDir()
 */
int CdReadDir(CdlDIR* dir, CdlFILE* file);

/**
 * @brief Closes a directory context created by CdOpenDir()
 *
 * @details Closes a directory query context created by CdOpenDir(). Behavior
 * is undefined when closing a previously closed directory context.
 *
 * @param dir Directory context
 *
 * @see CdOpenDir()
 */
void CdCloseDir(CdlDIR* dir);

int CdGetVolumeLabel(char* label);

/**
 * @brief Sets a callback function for auto pause
 *
 * @details The callback function specified in *func is executed when an auto
 * pause interrupt occurs when the current CD-ROM mode is set with CdlModeAP.
 * Auto pause interrupt occurs when CD Audio playback reaches the end of the
 * audio track. Specifying 0 disables the callback.
 *
 * This can be used to easily loop CD audio automatically without requiring any
 * intervention in your software loop.
 *
 * @param func Callback function
 * @return Pointer to the last callback function set. Zero if no callback was
 * set previously.
 *
 * @see CdControl()
 */
int* CdAutoPauseCallback(void(*func)());

/**
 * @brief Retrieves CD-ROM ISO9660 parser status
 *
 * @details Returns the status of the file system parser from the last call of
 * a file system related function, such as CdSearchFile(), CdGetVolumeLabel()
 * and CdOpenDir(). Use this function to retrieve the exact error occurred when
 * either of those functions fail.
 *
 * @return CD-ROM ISO9660 parser error code, as listed below:
 *
 * | Value           | Description                                                                                         |
 * | :-------------- | :-------------------------------------------------------------------------------------------------- |
 * | CdlIsoOkay      | File system parser okay.                                                                            |
 * | CdlIsoSeekError | Logical seek error occurred. May occur when attempting to query the filesystem on an audio-only CD. |
 * | CdlIsoReadError | Read error occurred while reading the CD-ROM file system descriptor.                                |
 * | CdlIsoInvalidFs | Disc does not contain a standard ISO9660 file system.                                               |
 * | CdlIsoLidOpen   | Lid is open when attempting to parse the CD-ROM file system.                                        |
 */
int CdIsoError(void);

/**
 * @brief Locates and parses the specified disc session
 *
 * @details Loads a session specified by session on a multi-session disc. Uses
 * CdlSetsession to seek to the specified disc session, then scans the
 * following 512 sectors for an ISO volume descriptor. If a volume descriptor
 * is found the file system of that session is parsed and files inside the new
 * session can be accessed using regular CD-ROM file and directory querying
 * functions (CdSearchFile(), CdOpenDir(), CdReadDir(), CdCloseDir()). No
 * special consideration is required when reading files from a new session.
 *
 * Loading a session takes 5-10 seconds to complete depending on the distance
 * between the beginning of the disc and the start of the specified session. If
 * the session specified does not exist, the disc will stop and would take
 * 15-20 seconds to restart. The function does not support loading the most
 * recent session of a disc automatically due to limitations of the CD-ROM
 * hardware, so the user must be prompted to specify which session to load and
 * to keep a record of the number of sessions that have been written to the
 * disc.
 *
 * This function can also be used to update the Table of Contents (TOC) and
 * reparse the file system regardless of the media change status by simply
 * loading the first session. This is most useful for accessing files or audio
 * tracks on a disc that was inserted using the swap trick method (it is
 * recommended to stop the disc using CdlStop then restart it with CdlStandby
 * after a button prompt for convenience, if you wish to implement this
 * capability). Seeking to sessions other than the first session does not work
 * with the swap trick however, so a chipped or unlockable console is desired
 * for reading multi-session discs.
 *
 * NOTE: When the lid has been opened, the current CD-ROM session is reset to
 * the first session on the disc. The console may produce an audible click
 * sound when executing this function. This is normal, and the click sound is
 * no different to the click heard on disc spin-up in older models of the
 * console.
 *
 * @param session Session number (1 = first session)
 * @return 0 on success. On failure due to open lid, bad session number or no
 * volume descriptor found in specified session, returns -1 and return value of
 * CdIsoError() is updated.
 */
int CdLoadSession(int session);

#ifdef __cplusplus
}
#endif

#endif
