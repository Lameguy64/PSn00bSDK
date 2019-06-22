#include <stdio.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <psxsio.h>

static void _sio_init() {
}

static int _sio_open(FCB *fcb, const char* file, int mode) {
	
	fcb->diskid = 1;
	
	return 0;
	
}

static int _sio_inout(FCB *fcb, int cmd) {
	
	int i;
	
	if(cmd == 2) {	// Write
		
		for(i=0; i<fcb->trns_len; i++) {
			while(!(_sio_control(0, 0, 0) & SR_TXU));
			_sio_control(1, 4, ((char*)fcb->trns_addr)[i]);
		}
		
		return fcb->trns_len;
		
	} else if (cmd == 1) { // Read
	
		for(i=0; i<fcb->trns_len; i++) {
			while(!(_sio_control(0, 0, 0) & SR_RXRDY));
			((char*)fcb->trns_addr)[i] = _sio_control(0, 4, 0);
		}
		
		return fcb->trns_len;
		
	}
	
	return 0;
	
}

static int _sio_close(int h) {
	
	return h;
	
}

static DCB _sio_dcb = {
	"tty",
	0x3,
	0x1,
	0x0,
	(void*)_sio_init,	// init
	(void*)_sio_open,	// open
	(void*)_sio_inout,	// inout
	_sio_close,			// close
	NULL,				// ioctl
	NULL,				// read
	NULL,				// write
	NULL,				// erase
	NULL,				// undelete
	NULL,				// firstfile
	NULL,				// nextfile
	NULL,				// format
	NULL,				// chdir
	NULL,				// rename
	NULL,				// remove
	NULL				// testdevice
};


volatile void (*_sio_callback)(void) = NULL;

extern void _sio_irq_handler(void);


void AddSIO(int baud) {
	
	_sio_control(2, 0, 0);
	_sio_control(1, 2, MR_SB_01|MR_CHLEN_8|0x02);
	_sio_control(1, 3, baud);
	_sio_control(1, 1, CR_RXEN|CR_TXEN);
	
	close(0);
	close(1);
	
	DelDev(_sio_dcb.name);
	AddDev(&_sio_dcb);
	
	open(_sio_dcb.name, 2);
	open(_sio_dcb.name, 1);
	
}

void DelSIO(void) {
	
	// Reset serial interface
	_sio_control(2, 0, 0);
	
	// Remove TTY device
	DelDev(_sio_dcb.name);
	
}

void WaitSIO(void) {
	
	while((_sio_control(0, 0, 0)&(SR_RXRDY)) != (SR_RXRDY));
	_sio_control(0, 4, NULL);
	
}

void *Sio1Callback(void (*func)()) {
	
	void *old_isr; //= *((int*)&_sio_callback);
	
	EnterCriticalSection();
	
	if( func ) { 
		
		old_isr = InterruptCallback(8, func);
		//_sio_callback = func;
		
	} else {
		
		old_isr = InterruptCallback(8, NULL);
		//_sio_callback = NULL;
		
	}
	
	ExitCriticalSection();
	
	return old_isr;
	
}