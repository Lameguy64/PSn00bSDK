#ifndef _PSXKERNEL_H
#define _PSXKERNEL_H

// Event descriptors
#define DescMask	0xff000000		// Event descriptor mask
#define DescTH		DescMask
#define DescHW		0xf0000000		// Hardware event (IRQ)
#define DescEV		0xf1000000		// Event event
#define DescRC		0xf2000000		// Root counter event
#define DescUEV		0xf3000000		// User event
#define DescSW		0xf4000000		// BIOS event

// Hardware events
#define	HwVBLANK	(DescHW|0x01)	// VBlank
#define HwGPU		(DescHW|0x02)	// GPU
#define HwCdRom		(DescHW|0x03)	// CDROM
#define HwDMAC		(DescHW|0x04)	// DMA
#define HwRTC0		(DescHW|0x05)	// Timer 0
#define HwRTC1		(DescHW|0x06)	// Timer 1
#define HwRTC2		(DescHW|0x07)	// Timer 2
#define HwCNTL		(DescHW|0x08)	// Controller
#define HwSPU		(DescHW|0x09)	// SPU
#define HwPIO		(DescHW|0x0a)	// PIO & lightgun
#define HwSIO		(DescHW|0x0b)	// Serial

#define HwCPU		(DescHW|0x10)	// Processor exception
#define HwCARD		(DescHW|0x11)	// Memory card (lower level BIOS functions)
#define HwCard_0	(DescHW|0x12)	// Memory card (unused)
#define HwCard_1	(DescHW|0x13)	// Memory card (unused)
#define SwCARD		(DescSW|0x01)	// Memory card (higher level BIOS functions)
#define SwMATH		(DescSW|0x02)	// Libmath related apparently, unknown purpose

#define RCntCNT0	(DescRC|0x00)	// Root counter 0 (dot clock)
#define RCntCNT1	(DescRC|0x01)	// Horizontal sync
#define RCntCNT2	(DescRC|0x02)	// 1/8 of system clock
#define RCntCNT3	(DescRC|0x03)	// Vertical blank

#define RCntMdINTR		0x1000		// General interrupt
#define RCntMdNOINTR	0x2000		// New device
#define RCntMdSC		0x0001		// Counter becomes zero
#define RCntMdSP		0x0000		// Unknown purpose
#define RCntMdFR		0x0000
#define RCntMdGATE		0x0010		// Command acknowledged

#endif // _PSXKERNEL_H