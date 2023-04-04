/*
 * PSn00bSDK serial port BIOS TTY driver
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 *
 * This driver is designed to be as simple and reliable as possible: as such it
 * only relies on the SIO_*() API for receiving and sends data synchronously.
 * This allows printf() to work without issues, albeit slowly, if called from a
 * critical section or even from an interrupt handler.
 */

#include <sys/ioctl.h>
#include <psxapi.h>
#include <psxsio.h>
#include <hwregs_c.h>

/* TTY device control block */

static int _sio_open(FCB *fcb, const char* file, int mode) {
	fcb->diskid = 1;
	return 0;
}

static int _sio_inout(FCB *fcb, int cmd) {
	char *ptr = (char*) fcb->trns_addr;

	switch (cmd) {
		case 1: // read
			for (int i = 0; i < fcb->trns_len; i++)
				*(ptr++) = (char) SIO_ReadByte();

			return fcb->trns_len;

		case 2: // write
			for (int i = 0; i < fcb->trns_len; i++) {
				while (!(SIO_STAT(1) & (SR_TXRDY | SR_TXU)))
					__asm__ volatile("");

				SIO_DATA(1) = *(ptr++);
			}

			return fcb->trns_len;

		default:
			return 0;
	}
}

static int _sio_close(FCB *fcb) {
	return 0;
}

static int _sio_ioctl(FCB *fcb, int cmd, int arg) {
	switch (cmd) {
		case FIOCSCAN:
			return SIO_ReadSync(1) ? 0 : -1;

		default:
			return -1;
	}
}

static int _sio_dummy(void) {
	return -1;
}

static DCB _sio_dcb = {
	.name			= "tty",
	.flags			= 3,
	.ssize			= 1,
	.desc			= "PSXSIO SERIAL CONSOLE",
	.f_init			= &_sio_dummy,
	.f_open			= &_sio_open,
	.f_inout		= &_sio_inout,
	.f_close		= &_sio_close,
	.f_ioctl		= &_sio_ioctl,
	.f_read			= &_sio_dummy,
	.f_write		= &_sio_dummy,
	.f_erase		= &_sio_dummy,
	.f_undelete		= &_sio_dummy,
	.f_firstfile	= &_sio_dummy,
	.f_nextfile		= &_sio_dummy,
	.f_format		= &_sio_dummy,
	.f_chdir		= &_sio_dummy,
	.f_rename		= &_sio_dummy,
	.f_remove		= &_sio_dummy,
	.f_testdevice	= &_sio_dummy
};

/* Public API */

void AddSIO(int baud) {
	SIO_Init(baud, MR_SB_01 | MR_CHLEN_8);

	close(0);
	close(1);
	DelDrv(_sio_dcb.name);
	AddDrv(&_sio_dcb);
	open(_sio_dcb.name, 2);
	open(_sio_dcb.name, 1);
}

void DelSIO(void) {
	SIO_Quit();

	DelDrv(_sio_dcb.name);
	add_nullcon_driver();
}
