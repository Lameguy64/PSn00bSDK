/*
 * PSn00bSDK buffered serial port driver
 * (C) 2022 spicyjpeg - MPL licensed
 *
 * TODO: add proper support for DTR/DSR flow control
 */

#include <stdint.h>
#include <assert.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxsio.h>
#include <hwregs_c.h>

#define BUFFER_LENGTH		128
#define SIO_SYNC_TIMEOUT	0x100000

/* Private types */

typedef struct {
	uint8_t data[BUFFER_LENGTH];
	uint8_t head, tail, length;
} RingBuffer;

/* Internal globals */

static SIO_FlowControl _flow_control;
static uint16_t _ctrl_reg_flag;

static int  (*_read_callback)(uint8_t) = (void *) 0;
static void (*_old_sio_handler)(void)  = (void *) 0;

static volatile RingBuffer _tx_buffer, _rx_buffer;

/* Private interrupt handler */

static void _sio_handler(void) {
	// Handle any incoming bytes.
	while (SIO_STAT(1) & SR_RXRDY) {
		uint8_t value  = SIO_DATA(1);

		// Skip storing this byte into the RX buffer if the callback returns a
		// non-zero value.
		if (_read_callback) {
			if (_read_callback(value))
				continue;
		}

		int length = _rx_buffer.length;

		if (length >= BUFFER_LENGTH) {
			//_sdk_log("RX overrun, dropping bytes\n");
			break;
		}

		int tail          = _rx_buffer.tail;
		_rx_buffer.tail   = (tail + 1) % BUFFER_LENGTH;
		_rx_buffer.length = length + 1;

		_rx_buffer.data[tail] = value;
	}

	// Send the next byte in the buffer if the TX unit is ready. Note that
	// checking for CTS is unnecessary as the serial port is already hardwired
	// to do so.
	if (SIO_STAT(1) & (SR_TXRDY | SR_TXU)) {
		int length = _tx_buffer.length;

		if (length) {
			int head          = _tx_buffer.head;
			_tx_buffer.head   = (head + 1) % BUFFER_LENGTH;
			_tx_buffer.length = length - 1;

			SIO_CTRL(1) |= CR_TXIEN;
			SIO_DATA(1)  = _tx_buffer.data[head];
		} else {
			SIO_CTRL(1) &= CR_TXIEN ^ 0xffff;
		}
	}

	// Acknowledge the IRQ and update flow control signals.
	if (_rx_buffer.length < BUFFER_LENGTH)
		SIO_CTRL(1) = CR_INTRST | (SIO_CTRL(1) | _ctrl_reg_flag);
	else
		SIO_CTRL(1) = CR_INTRST | (SIO_CTRL(1) & (_ctrl_reg_flag ^ 0xffff));
}

/* Serial port initialization API */

void SIO_Init(int baud, uint16_t mode) {
	int _exit        = EnterCriticalSection();
	_old_sio_handler = InterruptCallback(IRQ_SIO1, &_sio_handler);

	SIO_CTRL(1) = CR_ERRRST;
	SIO_MODE(1) = (mode & 0xfffc) | MR_BR_16;
	SIO_BAUD(1) = (uint16_t) ((int) 0x1fa400 / baud);
	SIO_CTRL(1) = CR_TXEN | CR_RXEN | CR_RXIEN;

	_tx_buffer.head   = 0;
	_tx_buffer.tail   = 0;
	_tx_buffer.length = 0;
	_rx_buffer.head   = 0;
	_rx_buffer.tail   = 0;
	_rx_buffer.length = 0;

	_flow_control  = SIO_FC_NONE;
	_ctrl_reg_flag = 0;

	if (_exit)
		ExitCriticalSection();
}

void SIO_Quit(void) {
	int _exit = EnterCriticalSection();

	InterruptCallback(IRQ_SIO1, _old_sio_handler);
	SIO_CTRL(1) = CR_ERRRST;

	if (_exit)
		ExitCriticalSection();
}

void SIO_SetFlowControl(SIO_FlowControl mode) {
	FastEnterCriticalSection();

	switch (mode) {
		case SIO_FC_NONE:
			_flow_control  = SIO_FC_NONE;
			_ctrl_reg_flag = 0;

			SIO_CTRL(1) &= 0xffff ^ CR_DSRIEN;
			break;

		case SIO_FC_RTS_CTS:
			_flow_control  = SIO_FC_RTS_CTS;
			_ctrl_reg_flag = CR_RTS;

			SIO_CTRL(1) &= 0xffff ^ CR_DSRIEN;
			break;

		/*case SIO_FC_DTR_DSR:
			_flow_control  = SIO_FC_DTR_DSR;
			_ctrl_reg_flag = CR_DTR;

			SIO_CTRL(1) |= CR_DSRIEN;
			break;*/
	}

	FastExitCriticalSection();
}

/* Reading API */

int SIO_ReadByte(void) {
	/*for (int i = SIO_SYNC_TIMEOUT; i; i--) {
		if (_rx_buffer.length)
			return SIO_ReadByte2();
	}*/
	while (!_rx_buffer.length)
		__asm__ volatile("");

	return SIO_ReadByte2();
}

int SIO_ReadByte2(void) {
	if (!_rx_buffer.length)
		return -1;

	FastEnterCriticalSection();

	int head        = _rx_buffer.head;
	_rx_buffer.head = (head + 1) % BUFFER_LENGTH;
	_rx_buffer.length--;

	FastExitCriticalSection();
	return _rx_buffer.data[head];
}

int SIO_ReadSync(int mode) {
	if (mode)
		return _rx_buffer.length;

	/*for (int i = SIO_SYNC_TIMEOUT; i; i--) {
		if (_rx_buffer.length)
			return 0;
	}*/
	while (!_rx_buffer.length)
		__asm__ volatile("");

	return 0;
}

void *SIO_ReadCallback(int (*func)(uint8_t)) {
	FastEnterCriticalSection();

	void *old_callback  = _read_callback;
	_read_callback      = func;

	FastExitCriticalSection();
}

/* Writing API */

int SIO_WriteByte(uint8_t value) {
	for (int i = SIO_SYNC_TIMEOUT; i; i--) {
		if (_tx_buffer.length < BUFFER_LENGTH)
			return SIO_WriteByte2(value);
	}

	//_sdk_log("SIO_WriteByte() timeout\n");
	return -1;
}

int SIO_WriteByte2(uint8_t value) {
	// If the TX unit is currently busy, append the byte to the buffer instead
	// of sending it immediately. Note that interrupts must be disabled *prior*
	// to checking if TX is busy; disabling them afterwards would create a race
	// condition where the transfer could end while interrupts are being
	// disabled. Interrupts are disabled through the IRQ_MASK register rather
	// than via syscalls for performance reasons.
	FastEnterCriticalSection();

	if (SIO_STAT(1) & (SR_TXRDY | SR_TXU)) {
		SIO_DATA(1) = value;
		FastExitCriticalSection();
		return 0;
	}

	int length = _tx_buffer.length;

	if (length >= BUFFER_LENGTH) {
		FastExitCriticalSection();

		//_sdk_log("TX overrun, dropping bytes\n");
		return -1;
	}

	int tail          = _tx_buffer.tail;
	_tx_buffer.tail   = (tail + 1) % BUFFER_LENGTH;
	_tx_buffer.length = length + 1;

	_tx_buffer.data[tail] = value;
	SIO_CTRL(1) |= CR_TXIEN;

	FastExitCriticalSection();
	return length;
}

int SIO_WriteSync(int mode) {
	if (mode)
		return _tx_buffer.length;

	// Wait for the buffer to become empty.
	for (int i = SIO_SYNC_TIMEOUT; i; i--) {
		if (!_tx_buffer.length)
			break;
	}

	if (!_tx_buffer.length) {
		// Wait for the TX unit to finish sending the last byte.
		while (!(SIO_STAT(1) & (SR_TXRDY | SR_TXU)))
			__asm__ volatile("");
	} else {
		//_sdk_log("SIO_WriteSync() timeout\n");
	}

	return _tx_buffer.length;
}
