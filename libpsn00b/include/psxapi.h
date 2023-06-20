/*
 * PSn00bSDK kernel API library
 * (C) 2019-2023 Lameguy64, spicyjpeg - MPL licensed
 */

/**
 * @file psxapi.h
 * @brief Kernel API library header
 *
 * @details This header provides access to most of the APIs made available by
 * the system's BIOS, including basic file I/O, TTY output, controller and
 * memory card drivers, threads, events as well as kernel memory allocation.
 *
 * For more information and up-to-date documentation on kernel APIs, see:
 * https://psx-spx.consoledev.net/kernelbios/
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
#include <hwregs_c.h>

/* Definitions */

typedef enum _SeekMode {
	SEEK_SET = 0,
	SEEK_CUR = 1,
	SEEK_END = 2
} SeekMode;

typedef enum _EventDescriptor {
	DescMask = 0xff000000, // Event descriptor mask
	DescTH   = 0xff000000,
	DescHW   = 0xf0000000, // Hardware event (IRQ)
	DescEV   = 0xf1000000, // Event event
	DescRC   = 0xf2000000, // Root counter event
	DescUEV  = 0xf3000000, // User event
	DescSW   = 0xf4000000  // BIOS event
} EventDescriptor;

typedef enum _EventType {
	HwVBLANK = DescHW | 0x01, // VBlank
	HwGPU    = DescHW | 0x02, // GPU
	HwCdRom  = DescHW | 0x03, // CDROM
	HwDMAC   = DescHW | 0x04, // DMA
	HwRTC0   = DescHW | 0x05, // Timer 0
	HwRTC1   = DescHW | 0x06, // Timer 1
	HwRTC2   = DescHW | 0x07, // Timer 2
	HwCNTL   = DescHW | 0x08, // Controller
	HwSPU    = DescHW | 0x09, // SPU
	HwPIO    = DescHW | 0x0a, // PIO & lightgun
	HwSIO    = DescHW | 0x0b, // Serial
	HwCPU    = DescHW | 0x10, // Processor exception
	HwCARD   = DescHW | 0x11, // Memory card (lower level BIOS functions)
	HwCARD_0 = DescHW | 0x12,
	HwCARD_1 = DescHW | 0x13,

	RCntCNT0 = DescRC | 0x00,
	RCntCNT1 = DescRC | 0x01,
	RCntCNT2 = DescRC | 0x02,
	RCntCNT3 = DescRC | 0x03,

	SwCARD   = DescSW | 0x01, // Memory card (higher level BIOS functions)
	SwMATH   = DescSW | 0x02
} EventType;

typedef enum _EventFlag {
	EvSpIOE    = 1 <<  2,
	EvSpTIMOUT = 1 <<  8,
	EvSpNEW    = 1 <<  13,
	EvSpERROR  = 1 << 15,

	EvMdINTR   = 1 << 12,
	EvMdNOINTR = 1 << 13,

	RCntMdSP     = 0 <<  0,
	RCntMdFR     = 0 <<  0,
	RCntMdSC     = 1 <<  0, // IRQ when counter target
	RCntMdGATE   = 1 <<  4,
	RCntMdINTR   = 1 << 12, // Turns on IRQ
	RCntMdNOINTR = 1 << 13  // Polling mode
} EventFlag;

/* Structure definitions */

typedef struct _TCB { // Thread control block
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

typedef struct _PCB { // Process control block
	TCB *thread;
} PCB;

typedef struct _DCB { // Device control block
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

typedef struct _FCB { // File control block
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

struct DIRENTRY { // Directory entry
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

typedef struct _INT_RP {
	uint32_t	*next;
	uint32_t	*func2;
	uint32_t	*func1;
	int			_reserved;
} INT_RP;

/* Fast interrupt disabling macros */

// Clearing the IRQ_MASK register is faster than manipulating cop0r12, even
// though it requires declaring a "hidden" local variable to save its state to;
// it's also resilient to race conditions as there's no read-modify-write
// operation during which an interrupt can occur. Note that interrupt flags in
// the IRQ_STAT register will get set even if the respective enable bits in
// IRQ_MASK are cleared, so doing this will properly defer IRQs rather than
// dropping them.
#define FastEnterCriticalSection() \
	uint16_t __saved_irq_mask = IRQ_MASK; (IRQ_MASK = 0)
#define FastExitCriticalSection() \
	(IRQ_MASK = __saved_irq_mask)

#if 0
#define FastEnterCriticalSection() { \
	uint32_t r0, r1; \
	__asm__ volatile( \
		"mfc0 %0, $12;" \
		"li   %1, -1026;" \
		"and  %1, %0;" \
		"mtc0 %1, $12;" \
		"nop;" \
		: "=r"(r0), "=r"(r1) :: \
	); \
}
#define FastExitCriticalSection() { \
	uint32_t r0; \
	__asm__ volatile( \
		"mfc0 %0, $12;" \
		"nop;" \
		"ori  %0, 0x0401;" \
		"mtc0 %0, $12;" \
		"nop;" \
		: "=r"(r0) :: \
	); \
}
#endif

/* BIOS API */

#ifdef __cplusplus
extern "C" {
#endif

#if 0
static inline void _InitCd(void) {
	uint32_t old_dpcr = DMA_DPCR & 0xffff0fff;

	_96_init();
	DMA_DPCR |= old_dpcr;
}
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

int open(const char *path, int mode);
int close(int fd);
int lseek(int fd, uint32_t offset, int mode);
int read(int fd, void *buff, size_t len);
int write(int fd, const void *buff, size_t len);
int getc(int fd);
int putc(int ch, int fd);
int ioctl(int fd, int cmd, int arg);
int isatty(int fd);
struct DIRENTRY *firstfile(const char *wildcard, struct DIRENTRY *entry);
struct DIRENTRY *nextfile(struct DIRENTRY *entry);
int erase(const char *path);
int undelete(const char *path);
int cd(const char *path);

int _get_errno(void);
int _get_error(int fd);

int AddDrv(DCB *dcb);
int DelDrv(const char *name);
void ListDrv(void);
void add_nullcon_driver(void);

int EnterCriticalSection(void);
void ExitCriticalSection(void);
int SwEnterCriticalSection(void);
void SwExitCriticalSection(void);

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

int OpenTh(uint32_t (*func)(), uint32_t sp, uint32_t gp);
int CloseTh(int thread);
int ChangeTh(int thread);

int Exec(struct EXEC *exec, int argc, const char **argv);
int LoadExec(const char *path, int argc, const char **argv);
void FlushCache(void);

void ResetEntryInt(void);
void HookEntryInt(const JumpBuffer *buf);
void ReturnFromException(void);

int SetConf(int evcb, int tcb, uint32_t sp);
void GetConf(int *evcb, int *tcb, uint32_t *sp);
void SetMem(int size);

int GetSystemInfo(int index);
void *GetB0Table(void);
void *GetC0Table(void);

void *alloc_kernel_memory(int size);
void free_kernel_memory(void *ptr);

void _boot(void);

#ifdef __cplusplus
}
#endif
