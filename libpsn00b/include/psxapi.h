/*
 * PSn00bSDK kernel API library
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 */

#ifndef __PSXAPI_H
#define __PSXAPI_H

#include <stdint.h>
#include <stddef.h>

/* Definitions */

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

/* Structure definitions */

typedef struct {	// Thread control block
	int					status;
	int					mode;
	union {
		uint32_t		reg[37];
		struct {
			uint32_t	zero, at;
			uint32_t	v0, v1;
			uint32_t	a0, a1, a2, a3;
			uint32_t	t0, t1, t2, t3, t4, t5, t6, t7;
			uint32_t	s0, s1, s2, s3, s4, s5, s6, s7;
			uint32_t	t8, t9;
			uint32_t	k0, k1;
			uint32_t	gp, sp, fp, ra;

			uint32_t	cop0r14;
			uint32_t	hi;
			uint32_t	lo;
			uint32_t	cop0r12;
			uint32_t	cop0r13;
		};
	};
	int					_reserved[9];
} TCB;

typedef struct {	// Process control block
	TCB *thread;
} PCB;

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
	int			status;
	uint32_t	diskid;
	void		*trns_addr;
	uint32_t	trns_len;
	uint32_t	filepos;
	uint32_t	flags;
	uint32_t	lasterr;
	DCB			*dcb;
	uint32_t	filesize;
	uint32_t	lba;
	uint32_t	fcbnum;
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
	uint32_t pc0, gp0;
	uint32_t t_addr, t_size;
	uint32_t d_addr, d_size;
	uint32_t b_addr, b_size;
	uint32_t s_addr, s_size;
	uint32_t sp, fp, rp, ret, base;
};

struct JMP_BUF {
	uint32_t ra, sp, fp;
	uint32_t s0, s1, s2, s3, s4, s5, s6, s7;
	uint32_t gp;
};

// Not recommended to use these functions to install IRQ handlers

typedef struct {
	uint32_t	*next;
	uint32_t	*func2;
	uint32_t	*func1;
	int			_reserved;
} INT_RP;

/* API */

#ifdef __cplusplus
extern "C" {
#endif

void SysEnqIntRP(int pri, INT_RP *rp);
void SysDeqIntRP(int pri, INT_RP *rp);

int OpenEvent(uint32_t cl, uint32_t spec, int mode, void (*func)());
int CloseEvent(int event);
int WaitEvent(int event);
int TestEvent(int event);
int EnableEvent(int event);
int DisableEvent(int event);
void DeliverEvent(uint32_t cl, uint32_t spec);
void UnDeliverEvent(uint32_t cl, uint32_t spec);

int open(const char *name, int mode);
int close(int fd);
int seek(int fd, uint32_t offset, int mode);
int read(int fd, uint8_t *buff, size_t len);
int write(int fd, const uint8_t *buff, size_t len);
int ioctl(int fd, int cmd, int arg);
struct DIRENTRY *firstfile(const char *wildcard, struct DIRENTRY *entry);
struct DIRENTRY *nextfile(struct DIRENTRY *entry);
int erase(const char *name);
int chdir(const char *path);

//#define cd(p) chdir(p)

int AddDev(DCB *dcb);
int DelDev(const char *name);
void ListDev(void);
void AddDummyTty(void);

void EnterCriticalSection(void);
void ExitCriticalSection(void);
void SwEnterCriticalSection(void);
void SwExitCriticalSection(void);

void _InitCd(void);
void _96_init(void);
void _96_remove(void);

void InitPAD(uint8_t *buff1, int len1, uint8_t *buff2, int len2);
void StartPAD(void);
void StopPAD(void);

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
int _card_read(int chan, int sector, uint8_t *buf);
int _card_write(int chan, int sector, uint8_t *buf);
void _new_card(void);

int SetRCnt(int spec, uint16_t target, int mode);
int GetRCnt(int spec);
int StartRCnt(int spec);
int StopRCnt(int spec);
int ResetRCnt(int spec);

void ChangeClearPAD(int mode);
void ChangeClearRCnt(int t, int m);

uint32_t OpenTh(uint32_t (*func)(), uint32_t sp, uint32_t gp);
int CloseTh(uint32_t thread);
int ChangeTh(uint32_t thread);

int Exec(struct EXEC *exec, int argc, char **argv);
void FlushCache(void);

void b_setjmp(struct JMP_BUF *buf);
void b_longjmp(struct JMP_BUF *buf, int param);
void SetDefaultExitFromException(void);
void SetCustomExitFromException(struct JMP_BUF *buf);

int GetSystemInfo(int index);
void *GetB0Table(void);
void *GetC0Table(void);

void *_kernel_malloc(int size);
void _kernel_free(void *ptr);

void _boot(void);

#ifdef __cplusplus
}
#endif

#endif
