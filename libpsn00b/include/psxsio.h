/*
 * PSn00bSDK serial port library
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 */

#ifndef __PSXSIO_H
#define __PSXSIO_H

#include <stdint.h>

/* Register definitions (used internally) */

typedef enum _SIO_StatusRegFlag {
	SR_TXRDY	= 1 << 0,
	SR_RXRDY	= 1 << 1,
	SR_TXU		= 1 << 2,
	SR_PERROR	= 1 << 3,
	SR_OE		= 1 << 4,
	SR_FE		= 1 << 5,
	SR_DSR		= 1 << 7,
	SR_CTS		= 1 << 8,
	SR_IRQ		= 1 << 9
} SIO_StatusRegFlag;

typedef enum _SIO_ModeRegFlag {
	MR_BR_1		= 1 << 0,
	MR_BR_16	= 2 << 0,
	MR_BR_64	= 3 << 0,
	MR_CHLEN_5	= 0 << 2,
	MR_CHLEN_6	= 1 << 2,
	MR_CHLEN_7	= 2 << 2,
	MR_CHLEN_8	= 3 << 2,
	MR_PEN		= 1 << 4,
	MR_P_EVEN	= 1 << 5,
	MR_SB_01	= 1 << 6,
	MR_SB_10	= 2 << 6,
	MR_SB_11	= 3 << 6
} SIO_ModeRegFlag;

typedef enum _SIO_ControlRegFlag {
	CR_TXEN		= 1 <<  0,
	CR_DTR		= 1 <<  1,
	CR_RXEN		= 1 <<  2,
	CR_BRK		= 1 <<  3,
	CR_INTRST	= 1 <<  4,
	CR_RTS		= 1 <<  5,
	CR_ERRRST	= 1 <<  6,
	CR_BUFSZ_1	= 0 <<  8,
	CR_BUFSZ_2	= 1 <<  8,
	CR_BUFSZ_4	= 2 <<  8,
	CR_BUFSZ_8	= 3 <<  8,
	CR_TXIEN	= 1 << 10,
	CR_RXIEN	= 1 << 11,
	CR_DSRIEN	= 1 << 12
} SIO_ControlRegFlag;

typedef enum _SIO_FlowControl {
	SIO_FC_NONE		= 0,
	SIO_FC_RTS_CTS	= 1
	//SIO_FC_DTR_DSR	= 2
} SIO_FlowControl;

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Resets the serial port, initializes the library's internal ring
 * buffers and installs a serial IRQ handler. The given mode value (normally
 * MR_CHLEN_8|MR_SB_01 for 8 data bits, 1 stop bit and no parity) is copied to
 * the SIO_MODE register. Flow control is disabled by default (see
 * SIO_SetFlowControl() for more details).
 *
 * This function must be called prior to using SIO_ReadByte(), SIO_ReadSync(),
 * SIO_WriteByte(), SIO_WriteSync() or SIO_SetFlowControl(), and must not be
 * called from an IRQ callback.
 *
 * @param baud Baud rate in bits per second
 * @param mode Binary OR of SIO_ModeRegFlag enum members
 */
void SIO_Init(int baud, uint16_t mode);

/**
 * @brief Resets the serial port and removes the IRQ callback added by
 * SIO_Init(), restoring any previously installed handler.
 */
void SIO_Quit(void);

/**
 * @brief Changes the serial port's flow control mode. The following modes are
 * available:
 *
 * - SIO_FC_NONE (default): do not assert RTS or DTR automatically and ignore
 *   DSR. Note that the hardware will still wait for CTS to be asserted before
 *   sending any data; there is no way to disable this behavior.
 * - SIO_FC_RTS_CTS: assert RTS when the RX buffer is full and wait for CTS to
 *   be asserted before sending any data.
 *
 * The flow control mode shall only be changed while the TX and RX buffers are
 * empty.
 *
 * @param mode
 */
void SIO_SetFlowControl(SIO_FlowControl mode);

/**
 * @brief Reads a byte from the RX buffer. If the buffer is empty, blocks
 * indefinitely until a byte is received.
 *
 * WARNING: this function shall not be used in a critical section or IRQ
 * callback as no data is sent or received while interrupts are disabled. It
 * also lacks a timeout, so consider polling for new data using SIO_ReadByte2()
 * or SIO_ReadSync(1) and implementing a timeout instead.
 *
 * @return Received byte
 */
int SIO_ReadByte(void);

/**
 * @brief Non-blocking variant of SIO_ReadByte(). Reads a byte from the RX
 * buffer or returns -1 if the buffer is empty. Unlike SIO_ReadByte() this
 * function is safe to use in a critical section (although no data will be
 * received while interrupts are disabled).
 *
 * @return Received byte, -1 if no data is available
 */
int SIO_ReadByte2(void);

/**
 * @brief Waits for at least one byte to be available in the RX buffer (if
 * mode = 0) or returns the length of the RX buffer (if mode = 1).
 *
 * WARNING: this function shall not be used in a critical section or IRQ
 * callback as no data is sent or received while interrupts are disabled. Using
 * SIO_ReadSync(0) is additionally discouraged as it lacks a timeout; consider
 * polling for new data using SIO_ReadByte2() or SIO_ReadSync(1) and
 * implementing a timeout instead.
 *
 * @param mode
 * @return Number of RX bytes in the buffer
 */
int SIO_ReadSync(int mode);

/**
 * @brief Registers a function to be called whenever a byte is received. The
 * received byte is passed as an argument to the callback, which shall then
 * return a zero value to also store the byte in the RX buffer or a non-zero
 * value to drop it. This can be used to e.g. filter or validate incoming data,
 * or to bypass the library's RX buffer for custom buffering purposes.
 *
 * The callback will run in the exception handler's context, so it should be as
 * fast as possible and shall not call any function that relies on interrupts
 * being enabled.
 *
 * @param func
 * @return Previously set callback or NULL
 */
void *SIO_ReadCallback(int (*func)(uint8_t));

/**
 * @brief Sends the given byte, or appends it to the TX buffer if the serial
 * port is busy. If the buffer is full, blocks until the byte can be stored in
 * the buffer (with a timeout).
 *
 * WARNING: this function shall not be used in a critical section or IRQ
 * callback as no data is sent or received while interrupts are disabled.
 *
 * @param value
 * @return Number of TX bytes previously pending, -1 in case of a timeout
 */
int SIO_WriteByte(uint8_t value);

/**
 * @brief Non-blocking variant of SIO_WriteByte(). Sends the given byte, or
 * appends it to the TX buffer if the serial port is busy. If the buffer is
 * full, returns -1 without actually sending the byte. Unlike SIO_WriteByte()
 * this function is safe to use in a critical section (although no data will be
 * sent while interrupts are disabled).
 *
 * @param value
 * @return Number of TX bytes previously pending, -1 in case of failure
 */
int SIO_WriteByte2(uint8_t value);

/**
 * @brief Waits for all bytes pending in the TX buffer to be sent (if mode = 0)
 * or returns the length of the TX buffer (if mode = 1).
 *
 * WARNING: this function shall not be used in a critical section or IRQ
 * callback as no data is sent or received while interrupts are disabled.
 *
 * @param mode
 * @return Number of TX bytes pending, -1 in case of a timeout (mode = 0)
 */
int SIO_WriteSync(int mode);

/**
 * @brief Installs a BIOS file driver to redirect TTY stdin/stdout (including
 * BIOS messages as well as PSn00bSDK's own debug logging) to the serial port.
 * Uses SIO_Init() internally. The port is configured for 8 data bits, 1 stop
 * bit and no parity.
 *
 * This function shall only be used for debugging purposes. Picking a high baud
 * rate is recommended as all TTY writes are blocking and bypass the TX buffer.
 *
 * NOTE: some executable loaders, such as Unirom and Caetla, already replace
 * the BIOS TTY driver with a custom one. Calling AddSIO() will break the
 * built-in TTY functionality of these loaders.
 *
 * @param baud Baud rate in bits per second
 */
void AddSIO(int baud);

/**
 * @brief Uninstalls the BIOS driver installed by AddSIO() and attempts to
 * restore a "dummy" TTY driver. Calling this function is not recommended as
 * any further TTY usage may crash the system.
 */
void DelSIO(void);

#ifdef __cplusplus
}
#endif

#endif
