/*
 * PSn00bSDK CD-ROM library (high-level reading API)
 * (C) 2020-2022 Lameguy64, spicyjpeg - MPL licensed
 *
 * CdRead() and its related functions are separate from the "main" psxcd code
 * since handling retries is fairly complicated. In particular the drive
 * controller will not process any command properly for some time after a
 * CdlPause command, so an external timer (the vblank counter) and manual
 * polling are required to defer the next attempt.
 */

#include <stdint.h>
#include <assert.h>
#include <psxgpu.h>
#include <psxapi.h>
#include <psxcd.h>

#define CD_READ_TIMEOUT		180
#define CD_READ_COOLDOWN	60

/* Internal globals */

static CdlCB _read_callback = (CdlCB) 0;

static int     _total_sectors, _sector_size;
static uint8_t _read_result[4];

static volatile uint32_t *_read_addr;
static volatile int      _read_timeout, _pending_attempts, _pending_sectors;

extern CdlCB _cd_override_callback;

/* Private utilities and sector callback */

static void _sector_callback(CdlIntrResult irq, uint8_t *result) {
	if (irq == CdlDataReady) {
		CdGetSector((void *) _read_addr, _sector_size);
		_read_addr += _sector_size;

		if (--_pending_sectors > 0) {
			_read_timeout = VSync(-1) + CD_READ_TIMEOUT;
			return;
		}
	}

	// Stop reading if an error occurred or if no more sectors need to be read.
	CdCommandF(CdlPause, 0, 0);

	_cd_override_callback = (CdlCB) 0;
	if ((!_pending_sectors || !_pending_attempts) && _read_callback)
		_read_callback(irq, result);

	_read_timeout = VSync(-1) + CD_READ_COOLDOWN;
}

static int _poll_retry(void) {
	if (!_pending_attempts) {
		_sdk_log("CdRead() failed, too many attempts\n");

		_pending_sectors = 0;
		return -1;
	}

	//CdControlB(CdlPause, 0, 0);

	_sdk_log("CdRead() failed, retrying (%d sectors pending)\n", _pending_sectors);
	_pending_attempts--;

	// Restart from the first sector that returned an error.
	CdlLOC pos;
	CdIntToPos(
		CdPosToInt(CdLastPos()) + _total_sectors - _pending_sectors,
		&pos
	);

	_read_timeout  = VSync(-1) + CD_READ_TIMEOUT;
	_total_sectors = _pending_sectors;

	FastEnterCriticalSection();
	_cd_override_callback = &_sector_callback;
	FastExitCriticalSection();

	if (CdCommand(CdlSetloc, (uint8_t *) &pos, 3, _read_result))
		CdCommand(CdlReadN, 0, 0, _read_result);

	return _pending_sectors;
}

/* Public API */

int CdReadRetry(int sectors, uint32_t *buf, int mode, int attempts) {
	_sdk_validate_args((sectors > 0) && buf && (attempts > 0), -1);

	if (CdReadSync(1, 0) > 0) {
		_sdk_log("CdRead() failed, another read in progress (%d sectors pending)\n", _pending_sectors);
		return 0;
	}

	_read_addr        = buf;
	_read_timeout     = VSync(-1) + CD_READ_TIMEOUT;
	_pending_attempts = attempts - 1;
	_pending_sectors  = sectors;
	_total_sectors    = sectors;
	_sector_size      = (mode & CdlModeSize) ? 585 : 512;

	FastEnterCriticalSection();
	_cd_override_callback = &_sector_callback;
	FastExitCriticalSection();

	uint8_t _mode = mode;
	if (!CdCommand(CdlSetmode, &_mode, 1, 0))
		return 0;
	if (!CdCommand(CdlReadN, 0, 0, _read_result))
		return 0;

	return 1;
}

int CdRead(int sectors, uint32_t *buf, int mode) {
	return CdReadRetry(sectors, buf, mode, 1);
}

void CdReadBreak(void) {
	if (_pending_sectors > 0)
		_pending_sectors = -1;
}

int CdReadSync(int mode, uint8_t *result) {
	if (mode) {
		if (_pending_sectors < 0)
			return -2;
		if (!_pending_sectors)
			return 0;

		if (VSync(-1) > _read_timeout)
			return _poll_retry();
		if (CdSync(1, 0) == CdlDiskError)
			return -1;

		return _pending_sectors;
	}

	while (_pending_sectors > 0) {
		if (VSync(-1) > _read_timeout) {
			if (_poll_retry() < 0)
				return -1;
		}

		//if (CdSync(1, 0) == CdlDiskError)
			//return -1;
	}

	CdlIntrResult status = CdSync(0, result);
	if (_pending_sectors < 0)
		return -2;
	if (status != CdlComplete)
		return -1;

	return 0;
}

CdlCB CdReadCallback(CdlCB func) {
	FastEnterCriticalSection();

	CdlCB old_callback = _read_callback;
	_read_callback     = func;

	FastExitCriticalSection();
	return old_callback;
}
