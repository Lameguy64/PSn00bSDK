#ifndef __PSXAPI__
#define __PSXAPI__

#define DescHW			0xf0000000
#define DescSW			0xf4000000

#define HwCARD			(DescHW|0x11)
#define HwCARD_1		(DescHW|0x12)
#define HwCARD_0		(DescHW|0x13)
#define SwCARD			(DescHW|0x02)

#define EvSpIOE			0x0004
#define EvSpERROR		0x8000
#define EvSpTIMOUT		0x0100
#define EvSpNEW			0x0200

#define EvMdINTR		0x1000
#define EvMdNOINTR		0x2000

// Root counter (timer) definitions
#define DescRC			0xf2000000

#define RCntCNT0		(DescRC|0x00)
#define RCntCNT1		(DescRC|0x01)
#define RCntCNT2		(DescRC|0x02)
#define RCntCNT3		(DescRC|0x03)

#define RCntMdINTR		0x1000		// Turns on IRQ
#define RCntMdNOINTR	0x2000		// Polling mode
#define RCntMdSC		0x0001		// IRQ when counter target
#define RCntMdSP		0x0000
#define RCntMdFR		0x0000
#define RCntMdGATE		0x0010

typedef struct {					// Device control block
	char	*name;
	int		flags;
	int		ssize;
	char	*desc;
	void	*f_init;
	void	*f_open;
	void	*f_inout;
	void	*f_close;
	void	*f_ioctl;
	void	*f_read;
	void	*f_write;
	void	*f_erase;
	void	*f_undelete;
	void	*f_firstfile;
	void	*f_nextfile;
	void	*f_format;
	void	*f_chdir;
	void	*f_rename;
	void	*f_remove;
	void	*f_testdevice;
} DCB;

typedef struct {			// File control block
	int				status;
	unsigned int	diskid;
	void			*trns_addr;
	unsigned int	trns_len;
	unsigned int	filepos;
	unsigned int	flags;
	unsigned int	lasterr;
	DCB				*dcb;
	unsigned int	filesize;
	unsigned int	lba;
	unsigned int	fcbnum;
} FCB;

struct DIRENTRY {			// Directory entry
	char			name[20];
	int				attr;
	int				size;
	struct DIRENTRY	*next;
	int				head;
	char			system[4];
};

struct EXEC {
	unsigned int pc0;
	unsigned int gp0;
	unsigned int t_addr;
	unsigned int t_size;
	unsigned int d_addr;
	unsigned int d_size;
	unsigned int b_addr;
	unsigned int b_size;
	unsigned int s_addr;
	unsigned int s_size;
	unsigned int sp,fp,rp,ret,base;
};

// Not recommended to use these functions to install IRQ handlers

typedef struct {
	unsigned int* next;
	unsigned int* func2;
	unsigned int* func1;
	unsigned int pad;
} INT_RP;

#ifdef __cplusplus
extern "C" {
#endif

void SysEnqIntRP(int pri, INT_RP *rp);
void SysDeqIntRP(int pri, INT_RP *rp);

// Event handler stuff

int OpenEvent(unsigned int cl, int spec, int mode, void (*func)());
int CloseEvent(int ev_desc);
int EnableEvent(int ev_desc);
int DisableEvent(int ev_desc);

// BIOS file functions

int open(const char *name, int mode);
int close(int fd);
int seek(int fd, unsigned int offset, int mode);
int read(int fd, char *buff, unsigned int len);
int write(int fd, const char *buff, unsigned int len);
int ioctl(int fd, int cmd, int arg);
struct DIRENTRY *firstfile(const char *wildcard, struct DIRENTRY *entry);
struct DIRENTRY *nextfile(struct DIRENTRY *entry);
int erase(const char *name);
int chdir(const char *path);

//#define delete( p )	erase( p )	// May conflict with delete operator in C++
#define cd( p )		chdir( p )		// For compatibility

// BIOS device functions

int AddDev(DCB *dcb);
int DelDev(const char *name);
void ListDev(void);
void AddDummyTty(void);

void EnterCriticalSection(void);
void ExitCriticalSection(void);

// BIOS CD functions
void _InitCd(void);
void _96_init(void);
void _96_remove(void);

// BIOS pad functions
void InitPAD(char *buff1, int len1, char *buff2, int len2);
void StartPAD(void);
void StopPAD(void);

// BIOS memory card functions
void InitCARD(int pad_enable);
void StartCARD(void);
void StopCARD(void);
void _bu_init(void);

int _card_load(int chan);
int _card_info(int chan);
int _card_status(int chan);
int _card_wait(int chan);
int _card_clear(int chan);
int _card_chan(void);
int _card_read(int chan, int sector, unsigned char *buf);
int _card_write(int chan, int sector, unsigned char *buf);
void _new_card(void);

// Timers
int SetRCnt(int spec, unsigned short target, int mode);
int GetRCnt(int spec);
int StartRCnt(int spec);
int StopRCnt(int spec);
int ResetRCnt(int spec);

// BIOS IRQ acknowledge control
void ChangeClearPAD(int mode);
void ChangeClearRCnt(int t, int m);

// Executable functions
int Exec(struct EXEC *exec, int argc, char **argv);

void _boot(void);

#ifdef __cplusplus
}
#endif

#endif
