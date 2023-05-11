/*
 * PSn00bSDK CD-ROM library
 * (C) 2020-2022 Lameguy64, spicyjpeg - MPL licensed
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

#pragma once

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
	CdlReset		= 0x1c,
	CdlGetQ			= 0x1d,
	CdlReadTOC		= 0x1e
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

typedef enum _CdlRegionCode {
	CdlRegionUnknown	= 0,
	CdlRegionSCEI		= 1,	// Japan
	CdlRegionSCEA		= 2,	// North America
	CdlRegionSCEE		= 3,	// Europe
	CdlRegionSCEW		= 4,	// Wordwide (Net Yaroze)
	CdlRegionDebug		= 5		// DebuggingStation or test console
} CdlRegionCode;

typedef enum _CdlIsoError {
	CdlIsoOkay		= 0,
	CdlIsoSeekError	= 1,
	CdlIsoReadError	= 2,
	CdlIsoInvalidFs	= 3,
	CdlIsoLidOpen	= 4
} CdlIsoError;

/**
 * @brief Translates a BCD value to decimal.
 *
 * @details Translates the specified value in BCD format (in 0-99 range) into
 * a decimal integer.
 */
#define btoi(b) (((b) / 16 * 10) + ((b) % 16))

/**
 * @brief Translates a decimal value to BCD.
 *
 * @details Translates a decimal integer in 0-99 range into a BCD format value.
 */
#define itob(i) (((i) / 10 * 16) | ((i) % 10))

/* Structure and type definitions */

/**
 * @brief CD-ROM MSF positional coordinates.
 *
 * @details This structure is used to specify CD-ROM coordinates in
 * minutes/seconds/frames format for some commands (see CdControl()). It can
 * be converted from/to a logical logical sector number using CdIntToPos() and
 * CdPosToInt() respectively.
 *
 * NOTE: the minute, second and sector fields are in BCD format. The track
 * field is only returned by CdGetToc() and otherwise ignored by all commands.
 *
 * @see CdIntToPos(), CdPosToInt(), CdControl()
 */
typedef struct _CdlLOC {
	uint8_t minute;	// Minutes (BCD)
	uint8_t second;	// Seconds (BCD)
	uint8_t sector;	// Sector or frame (BCD)
	uint8_t track;	// Track number
} CdlLOC;

/**
 * @brief CD-ROM volume mixing matrix.
 *
 * @details This structure is used to change the CD-ROM drive's volume levels
 * using CdMix(). Each field represents a volume level as a value in 0-255
 * range, with 128 being 100% and values above 128 distorting the output.
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
 * @brief File entry structure.
 *
 * @details This structure is used to store basic metadata of a file such as
 * its position and size. Currently, CdSearchFile() is the only function that
 * uses this structure.
 *
 * @see CdSearchFile()
 */
typedef struct _CdlFILE {
	CdlLOC	pos;		// CD-ROM position coordinates of file
	int		size;		// Size of file in bytes
	char	name[16];	// File name
} CdlFILE;

/**
 * @brief Current logical location information structure.
 *
 * @details This structure is returned by the CdlGetlocL command and contains
 * information about the last data sector read by the drive head, including its
 * location as well as mode and XA attributes if any.
 *
 * CdlGetlocL can only be issued while reading data sectors, as CD-DA data has
 * no headers. Use CdlGetlocP instead to obtain the current drive position when
 * playing CD-DA.
 *
 * @see CdControl()
 */
typedef struct _CdlLOCINFOL {
	uint8_t minute;			// Minutes (BCD)
	uint8_t second;			// Seconds (BCD)
	uint8_t sector;			// Sector or frame (BCD)
	uint8_t mode;			// Sector mode (usually 2)
	uint8_t file;			// XA file number (usually 0 or 1)
	uint8_t chan;			// XA channel number (0-31)
	uint8_t submode;		// XA submode
	uint8_t coding_info;	// XA coding information (ADPCM sectors only)
} CdlLOCINFOL;

/**
 * @brief Current physical location information structure.
 *
 * @details This structure is returned by the CdlGetlocP command and contains
 * information about the current location of the drive head relative to the
 * entire CD as well as to the beginning of the track being played or read.
 *
 * This information is obtained by reading subchannel Q, so CdlGetlocP works on
 * both data and CD-DA tracks (albeit with slightly lower precision on data
 * tracks).
 *
 * @see CdControl()
 */
typedef struct _CdlLOCINFOP {
	uint8_t track;			// Track number (BCD)
	uint8_t index;			// Index number (BCD, usually 1)
	uint8_t track_minute;	// Minutes relative to beginning of track (BCD)
	uint8_t track_second;	// Seconds relative to beginning of track (BCD)
	uint8_t track_sector;	// Sector or frame relative to beginning of track (BCD)
	uint8_t minute;			// Minutes (BCD)
	uint8_t second;			// Seconds (BCD)
	uint8_t sector;			// Sector or frame (BCD)
} CdlLOCINFOP;

/**
 * @brief CD-ROM XA filter structure.
 *
 * @details This structure is used with the CdlSetfilter command to specify
 * sector filter parameters for XA-ADPCM audio playback.
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
	uint8_t		file;	// XA file number (usually 1)
	uint8_t		chan;	// XA channel number (0-31)
	uint16_t	pad;
} CdlFILTER;

/**
 * @brief CD-ROM directory query context handle.
 *
 * @details Used to store a directory context created by CdOpenDir(). An open
 * context can then be used with CdReadDir() and closed with CdCloseDir().
 *
 * @see CdOpenDir()
 */
typedef void *CdlDIR;

typedef void (*CdlCB)(CdlIntrResult, uint8_t *);

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the CD-ROM library.
 *
 * @details Initializes the CD-ROM subsystem which includes hooking the
 * required IRQ handler, sets up internal variables of the CD-ROM library and
 * attempts to initialize the CD-ROM controller. The mode parameter does
 * nothing but may be used in future updates of this library.
 *
 * This function shall only be called once after ResetGraph() or
 * ResetCallback(), and shall not be called on systems which are known not to
 * be equipped with a CD drive such as most PS1-based arcade boards.
 *
 * @return 1 if the drive is ready or 0 if an error occurred
 */
int CdInit(void);

/**
 * @brief Translates an LBA to MSF coordinates.
 *
 * @details Translates the provided logical sector number to CD-ROM positional
 * coordinates in minutes/seconds/frames format, which are stored into the
 * provided CdlLOC structure. The translation takes the lead-in offset into
 * account, so LBA 0 is correctly translated to 00:02:00 rather than 00:00:00.
 *
 * @param i Logical sector number minus the 150-sector lead-in
 * @param p Pointer to a CdlLOC structure
 * @return Pointer to the specified CdlLOC structure
 */
CdlLOC* CdIntToPos(int i, CdlLOC *p);

/**
 * @brief Translates MSF coordinates to an LBA.
 *
 * @details Translates the CD-ROM positional coordinates in
 * minutes/seconds/frames format from the provided CdlLOC structure to a
 * logical sector number. The translation takes the lead-in offset into account
 * so 00:02:00 is correctly translated to LBA 0 rather than 150.
 *
 * @param p Pointer to a CdlLOC structure
 * @return Logical sector number minus the 150-sector lead-in
 */
int CdPosToInt(const CdlLOC *p);

/**
 * @brief Issues a command to the CD-ROM controller.
 *
 * @details Sends a CD-ROM command specified by com to the CD-ROM controller,
 * waits for an acknowledge interrupt (very fast) then returns. It will also
 * issue parameters from param to the CD-ROM controller if the command accepts
 * parameters. Any response from the controller is stored into the provided
 * buffer asynchronously.
 *
 * Some commands (marked as blocking in the table below) will keep running in
 * the background after being acknowledged. Use CdSync() to wait for these
 * commands to finish, or CdSyncCallback() to register a callback to be
 * executed once the drive is idle.
 *
 * This function requires interrupts to be enabled and cannot be used in a
 * critical section or IRQ callback. Use CdControlF() in callbacks instead.
 *
 * The following commands are available:
 *
 * | Command       | Value | Parameter  | Blocking | Description                                                                                                       |
 * | :------------ | ----: | :--------- | :------- | :---------------------------------------------------------------------------------------------------------------- |
 * | CdlNop        |  0x01 |            | No       | Updates the current CD-ROM status and resets the CdlStatShellOpen flag, without doing anything else.              |
 * | CdlSetloc     |  0x02 | CdlLOC     | No       | Sets the seek target location, but does not seek. Actual seeking begins upon issuing a seek or read command.      |
 * | CdlPlay       |  0x03 | (uint8_t)  | No       | Begins CD-DA playback. Parameter specifies an optional track number to play (some emulators do not support it).   |
 * | CdlForward    |  0x04 |            | No       | Starts fast-forwarding (CD-DA only). Issue CdlPlay to stop fast forwarding.                                       |
 * | CdlBackward   |  0x05 |            | No       | Starts rewinding (CD-DA only). Issue CdlPlay to stop rewinding.                                                   |
 * | CdlReadN      |  0x06 | (CdlLOC)   | No       | Begins reading data sectors with automatic retry. Used in conjunction with CdReadyCallback().                     |
 * | CdlStandby    |  0x07 |            | Yes      | Starts the spindle motor if it was previously stopped.                                                            |
 * | CdlStop       |  0x08 |            | Yes      | Stops playback or data reading and shuts down the spindle motor.                                                  |
 * | CdlPause      |  0x09 |            | Yes      | Stops playback or data reading without stopping the spindle motor.                                                |
 * | CdlInit       |  0x0a |            | Yes      | Initializes the CD-ROM controller and aborts any ongoing command.                                                 |
 * | CdlMute       |  0x0b |            | No       | Mutes the drive's audio output (both CD-DA and XA).                                                               |
 * | CdlDemute     |  0x0c |            | No       | Unmutes the drive's audio output (both CD-DA and XA).                                                             |
 * | CdlSetfilter  |  0x0d | CdlFILTER  | No       | Configures the XA ADPCM sector filter.                                                                            |
 * | CdlSetmode    |  0x0e | uint8_t    | No       | Sets the CD-ROM mode.                                                                                             |
 * | CdlGetparam   |  0x0f |            | No       | Returns current CD-ROM mode and file/channel filter settings.                                                     |
 * | CdlGetlocL    |  0x10 |            | No       | Returns current logical CD position, mode and XA filter parameters.                                               |
 * | CdlGetlocP    |  0x11 |            | No       | Returns current physical CD position (using SubQ location data).                                                  |
 * | CdlSetsession |  0x12 | uint8_t    | Yes      | Attempts to seek to the specified session on a multi-session disc.                                                |
 * | CdlGetTN      |  0x13 |            | No       | Returns the number of tracks on the disc.                                                                         |
 * | CdlGetTD      |  0x14 | uint8_t    | No       | Returns the starting location of the specified track number.                                                      |
 * | CdlSeekL      |  0x15 | (CdlLOC)   | Yes      | Logical seek (using data sector headers) to target position, set by last CdlSetloc command.                       |
 * | CdlSeekP      |  0x16 | (CdlLOC)   | Yes      | Physical seek (using subchannel Q) to target position, set by last CdlSetloc command.                             |
 * | CdlTest       |  0x19 | (varies)   | Yes      | Executes a test subcommand (see nocash documentation). Shall be issued using CdCommand() rather than CdControl(). |
 * | CdlGetID      |  0x1a |            | Yes      | Identifies the disc type and returns its license string if any.                                                   |
 * | CdlReadS      |  0x1b | (CdlLOC)   | No       | Begins reading data sectors in real-time (without retry) mode. Intended for playing XA ADPCM or .STR files.       |
 * | CdlReset      |  0x1c |            | No       | Resets the CD-ROM controller (similar behavior to manually opening and closing the door).                         |
 * | CdlGetQ       |  0x1d | uint8_t[2] | Yes      | Reads up to 10 raw bytes of subchannel Q data directly from the table of contents.                                |
 * | CdlReadTOC    |  0x1e |            | Yes      | Forces reinitialization of the disc's table of contents.                                                          |
 *
 * Most commands return the current CD-ROM status as result (which is
 * automatically saved by the library and can be retrieved at any time using
 * CdStatus()). The following commands also return additional data:
 *
 * | Command     | Return values                           |
 * | :---------- | :-------------------------------------- |
 * | CdlGetparam | uint8_t status, mode, _pad, file, chan  |
 * | CdlGetlocL  | CdlLOCINFOL info                        |
 * | CdlGetlocP  | CdlLOCINFOP info                        |
 * | CdlGetTN    | uint8_t status, first_track, last_track |
 * | CdlGetTD    | uint8_t status, minutes, seconds        |
 *
 * NOTE: Values are in BCD format. For some commands (CdlReadN, CdlReadS,
 * CdlSeekL, CdlSeekP), if a CdlLOC parameter is passed, it will be sent to the
 * controller as a separate CdlSetloc command.
 *
 * @param cmd
 * @param param Pointer to command parameters
 * @param result Optional pointer to buffer to store result into
 * @return 1 if the command was issued successfully, 0 if a previously issued
 * command has not yet finished processing or -1 if a parameter is required but
 * was not specified
 *
 * @see CdSync(), CdControlF(), CdCommand()
 */
int CdControl(CdlCommand cmd, const void *param, uint8_t *result);

/**
 * @brief Issues a command to the CD-ROM controller and waits for it to
 * complete.
 *
 * @details Equivalent to CdControl() followed by CdSync(). If a blocking
 * command is issued, this function blocks until said command has completed.
 *
 * This function requires interrupts to be enabled and cannot be used in a
 * critical section or IRQ callback. Use CdControlF() in callbacks instead.
 *
 * @param cmd
 * @param param Pointer to command parameters
 * @param result Optional pointer to buffer to store result into
 * @return 1 if the command was issued successfully, 0 if a previously issued
 * command has not yet finished processing or -1 if a parameter is required but
 * was not specified
 *
 * @see CdControl(), CdControlF()
 */
int CdControlB(CdlCommand cmd, const void *param, uint8_t *result);

/**
 * @brief Issues a command to the CD-ROM controller (asynchronous).
 *
 * @details This function works similarly to CdControl() but does not block for
 * the acknowledge interrupt from the CD-ROM controller, making it suitable for
 * usage in an IRQ handler or callback function. Use CdSync() (outside of a
 * callback) to wait for a command to finish, or CdSyncCallback() to register a
 * callback to be executed once the drive is idle.
 *
 * A maximum of two commands can be issued at once and only the first command
 * can have parameters, as the parameter buffer is not cleared until the last
 * command is acknowledged by the controller.
 *
 * NOTE: as with CdControl(), some commands (CdlReadN, CdlReadS, CdlSeekL,
 * CdlSeekP) are sent as two separate commands if a CdlLOC parameter is passed.
 * In some cases this may overflow the controller's two-command buffer.
 *
 * @param cmd
 * @param param Pointer to command parameters
 * @return -1 if a parameter is required but was not specified, otherwise 1
 * (even if sending the command failed)
 *
 * @see CdControl(), CdCommand()
 */
int CdControlF(CdlCommand cmd, const void *param);

/**
 * @brief Issues a custom packet to the CD-ROM controller.
 *
 * @details This is a more advanced variant of CdControl() that allows sending
 * commands with an arbitrary number of parameters, such as CdlTest commands,
 * and does not issue any additional CdlSetloc commands automatically. The
 * number of parameter bytes must be specified manually.
 *
 * As with CdControl(), this function waits for the drive to acknowledge the
 * command. Any response from the controller is stored into the provided buffer
 * asynchronously.
 *
 * This function requires interrupts to be enabled and cannot be used in a
 * critical section or IRQ callback. Use CdCommandF() in callbacks instead.
 *
 * @param cmd
 * @param param Pointer to command parameters if any
 * @param length Number of parameter bytes expected by the command
 * @param result Optional pointer to buffer to store result into
 * @return 1 if the command was issued successfully or 0 if a previously issued
 * command has not yet finished processing
 *
 * @see CdSync(), CdCommandF(), CdControl()
 */
int CdCommand(CdlCommand cmd, const void *param, int length, uint8_t *result);

/**
 * @brief Issues a custom packet to the CD-ROM controller (asynchronous).
 *
 * @details This function works similarly to CdCommand() but does not block for
 * the acknowledge interrupt from the CD-ROM controller, making it suitable for
 * usage in an IRQ handler or callback function. Use CdSync() (outside of a
 * callback) to wait for a command to finish, or CdSyncCallback() to register a
 * callback to be executed once the drive is idle.
 *
 * A maximum of two commands can be issued at once and only the first command
 * can have parameters, as the parameter buffer is not cleared until the last
 * command is acknowledged by the controller.
 *
 * @param cmd
 * @param param Pointer to command parameters
 * @param length Number of parameter bytes expected by the command
 * @return Always 1 (even if sending the command failed)
 *
 * @see CdCommand(), CdControlF()
 */
int CdCommandF(CdlCommand cmd, const void *param, int length);

/**
 * @brief Waits for a blocking command or returns the current status.
 *
 * @details Waits for the controller to finish processing any blocking command
 * (if mode = 0) or returns the current command status (if mode = 1). The
 * buffer specified by result will be populated with the most recent CD-ROM
 * status value from the last command issued.
 *
 * @param mode
 * @param result Optional pointer to buffer to store result into
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
CdlIntrResult CdSync(int mode, uint8_t *result);

/**
 * @brief Sets a callback for blocking commands.
 *
 * @details Registers a function to be called whenever a blocking command has
 * completed. The callback will get the interrupt status (CdlComplete or
 * CdlDiskError) and the pointer to any result buffer previously passed to
 * CdControl() as arguments.
 *
 * The callback will run in the exception handler's context, so it should be as
 * fast as possible and shall not call any function that relies on interrupts
 * being enabled.
 *
 * @param func
 * @return Previously set callback or NULL
 *
 * @see CdControl(), CdControlB(), CdSync()
 */
CdlCB CdSyncCallback(CdlCB func);

/**
 * @brief Sets a callback for incoming sector or report data.
 *
 * @details Registers a function to be called whenever a data sector or report
 * packet is available to be read from the drive, or in case of an error. The
 * callback will get the interrupt status (CdlDataReady or CdlDiskError) and
 * the pointer to any result buffer previously passed to CdControl() as
 * arguments.
 *
 * Note that the actual sector data is not retrieved automatically and must
 * instead be read into memory using CdGetSector() or CdGetSector2() before the
 * drive overwrites its internal sector buffer.
 *
 * The callback will run in the exception handler's context, so it should be as
 * fast as possible and shall not call any function that relies on interrupts
 * being enabled.
 *
 * NOTE: when using CdRead() or CdReadRetry(), any callback set using
 * CdReadyCallback() is temporarily disabled in order to let the library handle
 * read sectors internally.
 *
 * @param func
 * @return Previously set callback or NULL
 *
 * @see CdControl(), CdControlB(), CdGetSector()
 */
CdlCB CdReadyCallback(CdlCB func);

/**
 * @brief Sets a callback for the end of a track.
 *
 * @details Registers a function to be called whenever the drive encounters and
 * pauses at the end of a track (if the CdlModeAP bit is set in the CD-ROM
 * mode), or whether the end of the disc is reached during data reading or
 * audio playback. The callback will get the interrupt status (CdlDataEnd) and
 * the pointer to any result buffer previously passed to CdControl() as
 * arguments.
 *
 * Such a callback can be used along with the CdlModeAP flag to detect when
 * playback of a CD-DA track ends and automatically loop it or start playing
 * another track, with no manual polling required.
 *
 * @param func
 * @return Previously set callback or NULL
 *
 * @see CdControl()
 */
CdlCB CdAutoPauseCallback(CdlCB func);

/**
 * @brief Transfers data from the CD-ROM sector buffer.
 *
 * @details Reads sector data that is pending in the CD-ROM sector buffer and
 * stores it into the provided buffer (which must be 32-bit aligned). Blocks
 * until the transfer has finished.
 *
 * This function is intended to be called within a callback routine set using
 * CdReadyCallback() to fetch read data sectors from the CD-ROM sector buffer.
 *
 * @param madr
 * @param size Number of 32-bit words to read (usually 512 for a 2048-byte
 * sector)
 * @return Always 1
 *
 * @see CdReadyCallback()
 */
int CdGetSector(void *madr, int size);

/**
 * @brief Transfers data from the CD-ROM sector buffer (non-blocking).
 *
 * @details Reads sector data that is pending in the CD-ROM sector buffer and
 * stores it into the provided buffer (which must be 32-bit aligned). The
 * transfer runs in the background while keeping the CPU running; about one
 * word is transferred every 16 CPU cycles. Note this is slower than the
 * blocking transfer performed by CdGetSector().
 *
 * This function is intended to be called within a callback routine set using
 * CdReadyCallback() to fetch read data sectors from the CD-ROM sector buffer.
 * Since the transfer is asynchronous, CdDataSync() should be used to wait
 * until the whole sector has been read.
 *
 * @param madr
 * @param size Number of 32-bit words to read (usually 512 for a 2048-byte
 * sector)
 * @return Always 1
 *
 * @see CdReadyCallback(), CdDataSync()
 */
int CdGetSector2(void *madr, int size);

/**
 * @brief Waits for a sector buffer transfer to finish.
 *
 * @details Blocks until any sector DMA transfer initiated using CdGetSector2()
 * has finished (if mode = 0) or returns whether a transfer is currently in
 * progress (if mode = 1).
 *
 * @param mode
 * @return 0 if the transfer has finished, 1 if it is still in progress or -1
 * in case of a timeout
 *
 * @see CdGetSector2()
 */
int CdDataSync(int mode);

/**
 * @brief Reads one or more sectors into a buffer.
 *
 * @details Starts reading the specified number of sectors, starting from the
 * location set by the last CdlSetloc command issued, and stores them
 * contiguously into the provided buffer. The given mode is applied using a
 * CdlSetmode command prior to starting the read.
 *
 * Each sector read is 2340 bytes long if the CdlModeSize bit is set in the
 * mode, 2048 bytes long otherwise. Ideally, the CdlModeSpeed bit shall be set
 * to enable double speed mode.
 *
 * Reading is done asynchronously. Use CdReadSync() to block until all data has
 * been read, or CdReadCallback() to register a callback to be executed on
 * completion or error. In case of errors, reading is aborted immediately and
 * CdReadSync() will return -1.
 *
 * This function requires interrupts to be enabled and cannot be used in a
 * critical section or IRQ callback. Any callback set using CdReadyCallback()
 * is temporarily disabled and restored once the read operation completes.
 *
 * @param sectors
 * @param buf
 * @param mode CD-ROM mode to apply prior to reading using CdlSetmode
 * @return 1 if the read operation started successfully or 0 in case of errors
 *
 * @see CdReadRetry(), CdReadSync(), CdReadCallback()
 */
int CdRead(int sectors, uint32_t *buf, int mode);

/**
 * @brief Reads one or more sectors into a buffer (performs multiple attempts).
 *
 * @details This function works similarly to CdRead(), but retries reading in
 * case of errors. If reading fails, up to the specified number of attempts
 * will be done before an error is returned by CdReadSync().
 *
 * This function requires interrupts to be enabled and cannot be used in a
 * critical section or IRQ callback. Any callback set using CdReadyCallback()
 * is temporarily disabled and restored once the read operation completes.
 *
 * IMPORTANT: in order for retries to be correctly processed, CdReadSync(0)
 * (blocking) shall be called immediately after starting the read, or
 * CdReadSync(1) (non-blocking) shall be called frequently (e.g. once per
 * frame) until reading has finished.
 *
 * @param sectors
 * @param buf
 * @param mode CD-ROM mode to apply prior to reading using CdlSetmode
 * @param attempts Maximum number of attempts (>= 1)
 * @return 1 if the read operation started successfully or 0 in case of errors
 *
 * @see CdRead(), CdReadSync(), CdReadCallback()
 */
int CdReadRetry(int sectors, uint32_t *buf, int mode, int attempts);

/**
 * @brief Cancels reading initiated by CdRead().
 *
 * @details Aborts any ongoing read operation that was previously started by
 * calling CdRead() or CdReadRetry(). After aborting, CdReadSync() will return
 * -2 and any callback registered using CdReadCallback() *not* be called.
 *
 * NOTE: the CD-ROM controller may take several hundred milliseconds to
 * actually stop reading. CdReadSync() should be used to make sure the drive is
 * idle before sending a command or starting another read.
 *
 * @see CdReadSync()
 */
void CdReadBreak(void);

/**
 * @brief Waits for reading initiated by CdRead() to finish or returns status.
 *
 * @details Waits for any ongoing read operation previously started using
 * CdRead() or CdReadRetry() to finish (if mode = 0) or returns the number of
 * sectors pending (if mode = 1). The buffer specified by result will be
 * populated with the most recent CD-ROM status value from the last command
 * issued.
 *
 * When using CdReadRetry(), this function also handles starting a new read
 * attempt if the previous one failed. As such, it needs to be called once (if
 * mode = 0) or periodically (if mode = 1) in order to poll the drive for
 * failures, even if the return value is ignored.
 *
 * @param mode
 * @param result Buffer to store most recent CD-ROM status into
 * @return Number of sectors remaining, -1 in case of errors or -2 if the read
 * was aborted
 *
 * @see CdRead(), CdReadCallback()
 */
int CdReadSync(int mode, uint8_t *result);

/**
 * @brief Sets a callback for read operations started by CdRead().
 *
 * @details Registers a function to be called whenever a read started using
 * CdRead() or CdReadRetry() has completed. The callback will get the interrupt
 * status (CdlComplete or CdlDiskError) and the pointer to a result buffer
 * internal to the library.
 *
 * The callback will run in the exception handler's context, so it should be as
 * fast as possible and shall not call any function that relies on interrupts
 * being enabled.
 *
 * @param func 
 * @return Previously set callback or NULL
 *
 * @see CdRead(), CdReadSync()
 */
CdlCB CdReadCallback(CdlCB func);

/**
 * @brief Returns the last command issued.
 *
 * @details Returns the index of the last command sent to the CD-ROM controller
 * using CdCommand(), CdControl() or any of its variants. The value is stored
 * in an internal variable, so this function returns instantly and can be used
 * in an IRQ callback.
 *
 * @return Last command issued
 *
 * @see CdControl()
 */
CdlCommand CdLastCom(void);

/**
 * @brief Returns the last CD-ROM position set.
 *
 * @details Returns the last seek location set using the CdlSetloc command (or
 * any of the commands that are processed by sending CdlSetloc internally, such
 * as CdlReadN, CdlReadS, CdlSeekL or CdlSeekP). The value is stored in an
 * internal variable, so this function returns instantly and can be used in an
 * IRQ callback.
 *
 * Note that if CdlReadN, CdlReadS or CdlPlay were issued, this will be the
 * location the read was started from and *not* the current position of the
 * drive head, which can instead be retrieved using CdlGetlocL or CdlGetlocP.
 *
 * @return Pointer to an internal CdlLOC structure containing the last position
 *
 * @see CdControl()
 */
const CdlLOC *CdLastPos(void);

/**
 * @brief Returns the last CD-ROM mode set.
 *
 * @details Returns the CD-ROM mode last set using the CdlSetmode command.
 * The value is stored in an internal variable, so this function returns
 * instantly and can be used in an IRQ callback.
 *
 * WARNING: upon receiving some commands, such as CdlInit, the CD-ROM drive may
 * change its mode automatically. The value returned by CdMode() will not
 * reflect those changes; use CdlGetparam instead.
 *
 * @return Last CD-ROM mode value set
 *
 * @see CdControl()
 */
int CdMode(void);

/**
 * @brief Returns the most recent CD-ROM status.
 *
 * @details Returns the CD-ROM status since the last command issued. The value
 * is stored in an internal variable, so this function returns instantly and
 * can be used in an IRQ callback.
 *
 * The status value is updated by most CD-ROM commands, with the exception of
 * CdlTest, CdlGetlocL and CdlGetlocP; it can also be updated manually by
 * issuing the CdlNop command.
 *
 * @return CD-ROM status returned by last comand issued
 *
 * @see CdControl()
 */
int CdStatus(void);

/**
 * @brief Returns the CD-ROM controller's region code.
 *
 * @details Reads region information from the drive using a CdlTest command.
 * This can be used to reliably determine the system's region without having to
 * resort to workarounds like probing the BIOS ROM.
 *
 * This function may return incorrect results and trigger error callbacks on
 * emulators or consoles equipped with CD-ROM drive emulation devices such as
 * the PSIO. It is not affected by modchips.
 *
 * @return Region code or 0 if the region cannot be determined
 */
CdlRegionCode CdGetRegion(void);

/**
 * @brief Attempts to disable the CD-ROM controller's region check.
 *
 * @details Sends undocumented commands to the drive in an attempt to disable
 * the region string check, in order to allow reading data from non-PS1 discs
 * as well as CD-Rs without needing a modchip. As unlocking commands are region
 * specific, the drive's region must be obtained beforehand using CdGetRegion()
 * and passed to this function. The unlock persists even if the lid is opened,
 * but not if a CdlReset command is issued.
 *
 * Unlocking is only supported on US, European and Net Yaroze consoles (not on
 * Japanese models, devkits and most emulators). This function will return 1
 * without doing anything if CdlRegionDebug is passed as region, as debug
 * consoles can already read unlicensed discs.
 *
 * NOTE: if any callbacks were set using CdReadyCallback() or CdSyncCallback()
 * prior to calling CdUnlock(), they will be called with an error code as part
 * of the unlocking sequence, even if the unlock was successful. It is thus
 * recommended to call this function before setting any callbacks.
 *
 * @param region
 * @return 1 if the drive was successfully unlocked, 0 otherwise
 *
 * @see CdGetRegion()
 */
int CdUnlock(CdlRegionCode region);

/**
 * @brief Retrieves the disc's table of contents.
 *
 * @details Retrieves the track entries from a CD's table of contents (TOC). The
 * function can return up to 99 track entries, which is the maximum number of
 * tracks on a CD. This function shall only be called while the drive is idle
 * and cannot be used in an IRQ callback or critical section.
 *
 * NOTE: the CD-ROM controller only returns the minutes and seconds of each
 * track's location. This makes it impossible to start playing a track from its
 * *exact* beginning, and may result in the end of the previous track being
 * played if there are no silent gaps between tracks. The CD specification
 * recommends adding a 2-second pregap to each track for this reason.
 *
 * @param toc Pointer to an array of CdlLOC entries
 * @return Number of tracks on the disc, or 0 in case of error
 *
 * @see CdControl()
 */
int CdGetToc(CdlLOC *toc);

/**
 * @brief Sets the CD-ROM volume mixing matrix.
 *
 * @details Sets the volume levels of the CD-ROM drive's audio output (used for
 * both CD-DA and XA playback) to match the values in the provided CdlATV
 * structure.
 *
 * The default setting is { 128, 0, 128, 0 }, i.e. send the left channel to the
 * left output only and the right channel to the right output only. The output
 * can be downmixed to mono by setting the values to { 64, 64, 64, 64 }, or the
 * left and right channels can be swapped using { 0, 128, 0, 128 }.
 *
 * NOTE: the SPU has an additional volume control for CD audio. Both must be
 * set in order for audio output to work.
 *
 * @param vol CD-ROM attenuation parameters
 * @return Always 1
 */
int CdMix(const CdlATV *vol);

/**
 * @brief Locates a file in the CD-ROM file system.
 *
 * @details Searches the CD-ROM's ISO9660 file system for the specified file
 * and populates the given CdlFILE structure with information about the file if
 * found. This function uses dynamic memory allocation.
 *
 * Directories can be separated with slashes (/) or backslashes (\), a leading
 * slash or backslash is optional but paths must be absolute. The device prefix
 * (cdrom:) must be omitted. A file version identifier (;1) at the end of the
 * name is also optional. File and directory names are case insensitive; long
 * names and ISO9660 extensions such as Joliet are currently not supported.
 *
 * This function is blocking and may take several seconds to load and
 * subsequently parse the path table and directory records if none of the file
 * system functions have yet been called.
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
 * Since file system access is slow, it is recommended to only use
 * CdSearchFile() sparingly to e.g. find the location of a custom archive file,
 * and then use the archive's internal table of contents to locate entries
 * within the archive.
 *
 * @param loc Pointer to a CdlFILE structure
 * @param filename
 * @return Pointer to the specified CdlFILE structure, or a null pointer if the
 * file cannot be found or another error occurred; the return value of
 * CdIsoError() is also updated
 *
 * @see CdOpenDir()
 */
CdlFILE* CdSearchFile(CdlFILE *loc, const char *filename);

/**
 * @brief Opens a directory on the CD-ROM file system.
 *
 * @details Opens a directory on the CD-ROM's ISO9660 file system and reads its
 * contents, returning a newly allocated CdlDIR structure. This function uses
 * dynamic memory allocation.
 *
 * Directories can be separated with slashes (/) or backslashes (\), a leading
 * slash or backslash is optional but paths must be absolute. The device prefix
 * (cdrom:) must be omitted. Directory names are case insensitive; long names
 * and ISO9660 extensions such as Joliet are currently not supported.
 *
 * This function is blocking and may take several seconds to load and
 * subsequently parse the path table and directory records if none of the file
 * system functions have yet been called.
 *
 * @param path
 * @return Pointer of a CdlDIR context or a null pointer if an error occurred;
 * the return value of CdIsoError() is also updated
 *
 * @see CdReadDir(), CdCloseDir()
 */
CdlDIR *CdOpenDir(const char *path);

/**
 * @brief Obtains information about the next file in the directory.
 *
 * @details Retrieves a file entry from an open directory context and stores
 * information about the file into the provided CdlFILE structure. This
 * function is meant to be called repeatedly until no more files are available
 * in the directory, in which case it returns 0.
 *
 * @param dir
 * @param file Pointer to a CdlFILE structure
 * @return 1 if there are proceeding directory entries that follow, 0 otherwise
 *
 * @see CdOpenDir()
 */
int CdReadDir(CdlDIR *dir, CdlFILE *file);

/**
 * @brief Closes a directory opened by CdOpenDir().
 *
 * @details Closes and deallocates directory query context returned by
 * CdOpenDir(). Behavior is undefined when closing a previously closed
 * directory context.
 *
 * @param dir
 *
 * @see CdOpenDir()
 */
void CdCloseDir(CdlDIR *dir);

/**
 * @brief Retrieves the volume label of the CD-ROM file system.
 *
 * @details Reads the volume identifier of the disc's ISO9660 file system and
 * stores it into the provided string buffer. The volume label can be up to 32
 * characters long.
 *
 * This function is blocking and may take several seconds to load and
 * subsequently parse the path table and directory records if none of the file
 * system functions have yet been called.
 *
 * @param label
 * @return Length of the volume label (excluding the null terminator) or -1 in
 * case of an error; the return value of CdIsoError() is also updated
 */
int CdGetVolumeLabel(char *label);

/**
 * @brief Retrieves CD-ROM ISO9660 parser status.
 *
 * @details Returns the status of the file system parser from the last call of
 * a file system related function, such as CdSearchFile(), CdGetVolumeLabel(),
 * CdOpenDir() and CdLoadSession(). Use this function to retrieve the exact
 * error occurred when any of those functions fail.
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
 *
 * @see CdSearchFile(), CdOpenDir()
 */
int CdIsoError(void);

/**
 * @brief Locates and loads the specified disc session.
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
 * NOTE: when the lid has been opened, the current CD-ROM session is reset to
 * the first session on the disc. The console may produce an audible click
 * sound when executing this function. This is normal, and the click sound is
 * no different to the click heard on disc spin-up in older models of the
 * console.
 *
 * @param session Session number (1 = first session)
 * @return 0 on success or -1 in case of errors; the return value of
 * CdIsoError() is also updated
 */
int CdLoadSession(int session);

#ifdef __cplusplus
}
#endif
