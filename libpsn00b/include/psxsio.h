#ifndef __PSXSIO_H
#define __PSXSIO_H

#define SR_TXRDY	0x1
#define SR_RXRDY	0x2
#define SR_TXU		0x4
#define SR_PERROR	0x8
#define SR_OE		0x10
#define SR_FE		0x20
#define SR_DSR		0x80
#define SR_CTS		0x100
#define SR_IRQ		0x200

#define SIO_TXRDY	0x1
#define SIO_RXRDY	0x2
#define SIO_TXU		0x4
#define SIO_PERROR	0x8
#define SIO_OE		0x10
#define SIO_FE		0x20
#define SIO_DSR		0x80
#define SIO_CTS		0x100
#define SIO_IRQ		0x200

#define MR_CHLEN_5	0x00
#define MR_CHLEN_6	0x04
#define MR_CHLEN_7	0x08
#define MR_CHLEN_8	0x0C
#define MR_PEN		0x10
#define MR_P_EVEN	0x30
#define MR_SB_01	0x40
#define MR_SB_10	0x80
#define MR_SB_11	0xc0

#define CR_TXEN		0x1
#define CR_DTR		0x2
#define CR_RXEN		0x4
#define CR_BRK		0x8
#define CR_INTRST	0x10
#define CR_RTS		0x20
#define CR_ERRRST	0x40
#define CR_BUFSZ_1	0x00
#define CR_BUFSZ_2	0x100
#define CR_BUFSZ_4	0x200
#define CR_BUFSZ_8	0x300
#define CR_TXIEN	0x400
#define CR_RXIEN	0x800
#define CR_DSRIEN	0x1000

#ifdef __cplusplus
extern "C" {
#endif

int _sio_control(int cmd, int arg, int param);
void AddSIO(int baud);
void DelSIO(void);

void *Sio1Callback(void (*func)(void));

// ORIGINAL
void WaitSIO(void);
int kbhit();

#ifdef __cplusplus
}
#endif

#endif
