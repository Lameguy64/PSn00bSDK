#include <stdio.h>
#include "psxcd.h"

extern volatile char _cd_ack_wait;
extern volatile unsigned char _cd_last_int;
extern volatile unsigned char _cd_last_mode;
extern volatile unsigned char _cd_status;
extern volatile CdlCB _cd_callback_int1_data;
volatile unsigned char *_cd_result_ptr;

extern volatile char _cd_media_changed;

void _cd_init(void);
void _cd_control(unsigned char com, unsigned char *param, int plen);
void _cd_wait_ack(void);
void _cd_wait(void);

int CdInit(int mode)
{
	// Sets up CD-ROM hardware and low-level subsystem
	_cd_init();
	
	// So CD ISO file system component will update the ISO descriptor
	_cd_media_changed = 1;
	
	// Issue commands to initialize the CD-ROM hardware
	CdControl(CdlNop, 0, 0);
	CdControl(CdlInit, 0, 0);
	
	if( CdSync(0, 0) != CdlDiskError )
	{
		CdControl(CdlDemute, 0, 0);
		printf("psxcd: Init Ok!\n");
	}
	else
	{
		printf("psxcd: Error initializing. Bad disc/drive or no disc inserted.\n");
	}
	
	return 1;
}

int CdControl(unsigned char com, unsigned char *param, unsigned char *result)
{	
	// Don't issue command if ack is not received yet
	if( _cd_ack_wait )
	{
		return 0;
	}
	
	_cd_result_ptr = result;
	
	CdControlF(com, param);
	_cd_wait_ack();
	
	return 1;
}

int CdControlB(unsigned char com, unsigned char *param, unsigned char *result)
{
	if( !CdControl(com, param, result) )
	{
		return 0;
	}
	
	CdSync(0, 0);
	return 1;
}

int CdControlF(unsigned char com, unsigned char *param)
{
	int param_len=0;
	
	// Command specific parameters
	switch(com)
	{
		case CdlSetloc:
			param_len = 3;
			break;
		case CdlPlay:
			if( param )
			{
				param_len = 1;
			}
			break;
		case CdlSetfilter:
			param_len = 2;
			break;
		case CdlSetmode:
			param_len = 1;
			break;
		case CdlSetsession:
			param_len = 1;
			break;
		case CdlTest:
			param_len = 1;
			break;
		case CdlGetTD:
			param_len = 1;
	}
	
	// Issue Setloc if parameters are specified on CdlReadN and CdlReadS
	if( ( com == CdlReadN ) || ( com == CdlReadS ) )
	{
		if( param )
		{
			_cd_control(CdlSetloc, param, 3);
		}
	}
	
	// Issue CD command
	_cd_control(com, param, param_len);
	
	return 1;
}

int CdSync(int mode, unsigned char *result)
{
	int cdirq;
	
	if( mode )
	{
		if( result )
		{
			*result = _cd_status;
		}
		
		cdirq = _cd_last_int;
		if( cdirq == CdlAcknowledge )
		{
			cdirq = CdlComplete;
		}
		return cdirq;
	}
	
	_cd_wait();
	
	if( result )
	{
		*result = _cd_status;
	}
	
	cdirq = _cd_last_int;
	if( cdirq == CdlAcknowledge )
	{
		cdirq = CdlComplete;
	}
	
	return cdirq;
}

int CdGetToc(CdlLOC *toc)
{
	u_char track_info[8];
	int i,tracks;
	
	// Get number of tracks
	if( !CdControl(CdlGetTN, 0, track_info) )
	{
		return 0;
	}
	
	if( CdSync(1, 0) != CdlComplete )
	{
		return 0;
	}
	
	tracks = 1+(btoi(track_info[2])-btoi(track_info[1]));
	
	// Get track positions
	for(i=0; i<tracks; i++)
	{
		int t = itob(1+i);
		struct { u_char status, toc_min, toc_sec; } res;
		if( !CdControl(CdlGetTD, (u_char*)&t, (u_char*)&res) )
		{
			return 0;
		}
		if( CdSync(1, 0) != CdlComplete )
		{
			return 0;
		}
		toc[i].minute = res.toc_min;
		toc[i].second = res.toc_sec;
		toc[i].sector = 0;
		toc[i].track = 1+i;
	}
	
	return tracks;
}

CdlLOC *CdIntToPos(int i, CdlLOC *p) {

	i += 150;
	
	p->minute = itob((i/75)/60);
	p->second = itob((i/75)%60);
	p->sector = itob(i%75);

	return p;
	
}

int CdPosToInt(CdlLOC *p)
{	
	return ((75*(btoi(p->minute)*60))+(75*btoi(p->second))+btoi(p->sector))-150;
}

int CdStatus(void)
{	
	return _cd_status;	
}

int CdMode(void)
{
	return _cd_last_mode;
}


// CD data read routines
volatile int _cd_sector_count = 0;
volatile unsigned int *_cd_read_addr;
volatile unsigned char _cd_read_result[8];
volatile unsigned int _cd_read_oldcb;
volatile unsigned int _cd_read_sector_sz;
volatile CdlCB _cd_read_cb;

// Sector callback
static void _CdReadReadyCallback(int status, unsigned char *result)
{
	if( status == CdlDataReady )
	{
		// Fetch sector from CD controller
		CdGetSector((void*)_cd_read_addr, _cd_read_sector_sz);
		
		// Increment destination address
		_cd_read_addr += _cd_read_sector_sz>>2;
	
		// Subtract sector count
		_cd_sector_count--;
	}
	
	// End reading with pause command when sector count reaches zero
	// or when an error occurs
	if( ( _cd_sector_count <= 0 ) || ( status == CdlDiskError ) )
	{
		// Stop reading
		_cd_control(CdlPause, 0, 0);
		
		// Revert previous ready callback
		_cd_callback_int1_data = (CdlCB)_cd_read_oldcb;
		
		// Execute read completion callback
		if( _cd_read_cb )
		{
			_cd_read_cb(status, result);
		}
	}
}

int CdRead(int sectors, unsigned int *buf, int mode)
{
	// Set sectors to read count
	_cd_sector_count = sectors;
	_cd_read_addr = buf;
	
	// Determine sector based on mode flags
	if( mode & CdlModeSize0 )
	{
		_cd_read_sector_sz = 2328;
	}
	else if( mode & CdlModeSize1 )
	{
		_cd_read_sector_sz = 2340;
	}
	else
	{
		_cd_read_sector_sz = 2048;
	}
	
	// Set readt callback
	_cd_read_oldcb = CdReadyCallback(_CdReadReadyCallback);
	
	// Set specified mode
	CdControl(CdlSetmode, (unsigned char*)&mode, 0);
	
	// Begin reading sectors
	CdControl(CdlReadN, 0, (unsigned char*)_cd_read_result);
	
	return 0;
}

int CdReadSync(int mode, unsigned char *result)
{
	if( mode )
	{
		if( CdSync(1, 0) == CdlDiskError )
		{
			return -1;
		}
		return _cd_sector_count;
	}
	
	while(_cd_sector_count > 0);
	if( CdSync(0, result) != CdlComplete )
	{
		return -1;
	}
	
	return 0;
}

unsigned int CdReadCallback(CdlCB func)
{
	unsigned int old_func;
	
	old_func = (unsigned int)_cd_read_cb;
	
	_cd_read_cb = func;
	
	return old_func;
}
