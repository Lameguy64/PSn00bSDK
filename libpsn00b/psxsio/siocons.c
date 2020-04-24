#include <stdio.h>
#include <string.h>
#include <ioctl.h>
#include <psxapi.h>
#include <psxgpu.h>
#include <psxsio.h>

#define SIO_BUFF_LEN	32

static volatile int _sio_key_pending;

static volatile int _sio_buff_rpos;
static volatile int _sio_buff_wpos;
static volatile char _sio_buff[SIO_BUFF_LEN];

static void _sio_init() {
	
	_sio_key_pending = 0;
	
	memset((void*)_sio_buff, 0, SIO_BUFF_LEN);
	_sio_buff_rpos = 0;
	_sio_buff_wpos = 0;

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
	
		/*for(i=0; i<fcb->trns_len; i++) {
			while(!(_sio_control(0, 0, 0) & SR_RXRDY));
			((char*)fcb->trns_addr)[i] = _sio_control(0, 4, 0);
		}*/
		
		
		
		for(i=0; i<fcb->trns_len; i++) {
			
			while( _sio_key_pending == 0 );
			
			((char*)fcb->trns_addr)[i] = _sio_buff[_sio_buff_rpos];
			_sio_key_pending--;
			_sio_buff_rpos++;
			if( _sio_buff_rpos >= SIO_BUFF_LEN )
			{
				_sio_buff_rpos = 0;
			}
			
		}
		
		return fcb->trns_len;
		
	}
	
	return 0;
	
}

static int _sio_ioctl(FCB *fcb, int cmd, int arg)
{
	if( cmd == FIOCSCAN )
	{
		if( _sio_key_pending )
		{
			return 0;
		}
	}
	
	return -1;
}

static int _sio_close(int h) {
	
	return h;
	
}

static void _sio_tty_cb(void)
{
	_sio_key_pending++;
	
	// Get received byte
	if( _sio_key_pending < SIO_BUFF_LEN )
	{
		_sio_buff[_sio_buff_wpos] = _sio_control(0, 4, 0);
		_sio_buff_wpos++;
		if( _sio_buff_wpos >= SIO_BUFF_LEN )
		{
			_sio_buff_wpos = 0;
		}
	}
	else
	{
		_sio_control(0, 4, 0);
	}
	
	// Acknowledge SIO IRQ
	_sio_control(2, 1, 0);
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
	_sio_ioctl,			// ioctl
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

void AddSIO(int baud) {
	
	_sio_control(2, 0, 0);
	_sio_control(1, 2, MR_SB_01|MR_CHLEN_8|0x02);
	_sio_control(1, 3, baud);
	_sio_control(1, 1, CR_RXEN|CR_TXEN|CR_RXIEN);
	
	close(0);
	close(1);
	
	DelDev(_sio_dcb.name);
	AddDev(&_sio_dcb);
	
	Sio1Callback(_sio_tty_cb);
	
	open(_sio_dcb.name, 2);
	open(_sio_dcb.name, 1);
	
}

void DelSIO(void) {
	
	Sio1Callback(NULL);
	
	// Reset serial interface
	_sio_control(2, 0, 0);
	
	// Remove TTY device
	DelDev(_sio_dcb.name);
	
	// Add dummy TTY device
	AddDummyTty();
		
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

int kbhit()
{
	return(_sio_key_pending>0);
}
