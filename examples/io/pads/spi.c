/*
 * PSn00bSDK controller polling example (SPI driver)
 * (C) 2021 spicyjpeg - MPL licensed
 *
 * This is a fairly complete timer driven, asynchronous high-speed SPI driver,
 * with support for sending custom commands (including memory card access), in
 * about 200 lines of code. Feel free to copypaste and adapt it.
 *
 * The way this works is by maintaining a queue of requests to send, each with
 * its own payload and callback. Timer 2 is configured to trigger an IRQ at
 * regular intervals. On each tick, the next request in the queue (or a poll
 * command if no request is pending) is prepared and the first byte is
 * sent; if the controller asks for more data by pulling /ACK low, the next
 * byte is sent and the received byte is placed into a buffer. This goes on
 * until the last byte is exchanged and the controller stops asserting /ACK.
 *
 * On the next tick, the response buffer is passed to the request's callback
 * and reset, and the next request in the queue is sent. This blindly assumes
 * it only takes one tick for a request/response to be sent, which is the case
 * for controllers' very small packets but not for memory cards. It is
 * advisable to call spi_set_poll_rate() to temporarily reduce poll rate while
 * accessing memory cards.
 *
 * Note that this driver completely takes over the SPI bus, so you won't be
 * able to use any BIOS functions that rely on SPI access (i.e. pad and memory
 * card APIs) alongside it.
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxpad.h>
#include <hwregs_c.h>

#include "spi.h"

/* Internal structures and globals */

typedef struct _SPI_Context {
	uint8_t			tx_buff[SPI_BUFF_LEN];
	uint8_t			rx_buff[SPI_BUFF_LEN];
	uint32_t		tx_len, rx_len, port;
	SPI_Callback	callback;
} SPI_Context;

static volatile SPI_Context				_context;
static volatile SPI_Request volatile	*_current_req;
static volatile SPI_Callback			_default_cb;

/* Request queue management */

static void _spi_create_poll_req(void) {
	PadRequest *req = (PadRequest *) _context.tx_buff;

	req->addr     = 0x01;
	req->cmd      = PAD_CMD_READ;
	req->tap_mode = 0x00; // 0x01 to enable extended multitap response
	req->motor_l  = 0x00;
	req->motor_r  = 0x00;

	_context.tx_len   = 4;
	_context.rx_len   = 0;
	_context.port    ^= 1;
	_context.callback = _default_cb;
}

static void _spi_next_req(void) {
	// Copy the contents of the first request in the queue into the TX buffer.
	memcpy(
		(void *) _context.tx_buff,
		(void *) _current_req->data,
		_current_req->len
	);

	_context.tx_len   = _current_req->len;
	_context.rx_len   = 0;
	_context.port     = _current_req->port;
	_context.callback = _current_req->callback;

	// Pop the first request from the queue by deallocating it and adjusting
	// the pointer to the first queue item.
	SPI_Request *next = _current_req->next;

	free((void *) _current_req);
	_current_req = next;
}

/* Interrupt handlers */

static void _spi_poll_irq_handler(void) {
	// Fetch the last response byte, which wasn't followed by a pulse on /ACK,
	// from the RX FIFO.
	if (JOY_STAT & 0x0002)
		_context.rx_buff[_context.rx_len - 1] = (uint8_t) JOY_TXRX;

	if (_context.callback)
		_context.callback(_context.port, _context.rx_buff, _context.rx_len);

	// If the request queue is empty, create a pad polling request.
	if (_current_req)
		_spi_next_req();
	else
		_spi_create_poll_req();

	// Prepare the SPI port by clearing any pending IRQ, pulling /CS high and
	// enabling the /ACK IRQ. In order to communicate with controllers, /CS has
	// to be driven low again for about 20 us before sending the first byte.
	// TODO: these delays can be probably tweaked for better performance
	JOY_CTRL = 0x0010;
	for (uint32_t i = 0; i < 1000; i++)
		__asm__ volatile("");

	JOY_CTRL = 0x1003 | (_context.port << 13);
	for (uint32_t i = 0; i < 2000; i++)
		__asm__ volatile("");

	// Send the first byte indicating which device to address. If the matching
	// device is connected, it will reply by triggering the /ACK IRQ.
	JOY_TXRX = _context.tx_buff[0];
}

static void _spi_ack_irq_handler(void) {
	// Wait until /ACK is pulled up by the controller before sending the next
	// byte. According to nocash docs, this has to be done before resetting the
	// IRQ.
	while (JOY_STAT & 0x0080)
		__asm__ volatile("");

	// Keep /CS pulled low and acknowledge the IRQ (bit 4) to ensure it can be
	// triggered again.
	JOY_CTRL = 0x1013 | (_context.port << 13);

	if (!_context.rx_len) {
		// We just sent the first address byte. Obviously the response we
		// received was read from an open bus, so the SPI port's internal FIFO
		// must be flushed (by performing dummy reads) to ensure we are only
		// going to read valid data from now on.
		JOY_TXRX;

	} else if (_context.rx_len <= SPI_BUFF_LEN) {
		// If this is not the first byte, put it in the RX buffer.
		_context.rx_buff[_context.rx_len - 1] = (uint8_t) JOY_TXRX;
	}

	// Send the next byte, or a null byte if there is no more data to send and
	// we're just reading a response.
	_context.rx_len++;
	if (_context.rx_len < _context.tx_len)
		JOY_TXRX = (uint32_t) _context.tx_buff[_context.rx_len];
	else
		JOY_TXRX = 0x00;
}

/* Public API */

SPI_Request *SPI_CreateRequest(void) {
	SPI_Request *req = malloc(sizeof(SPI_Request));

	req->len      = 0;
	req->port     = 0;
	req->callback = 0;
	req->next     = 0;

	// Find the last queued request by traversing the linked list and append a
	// pointer to the new request.
	if (!_current_req) {
		_current_req = req;
	} else {
		volatile SPI_Request *volatile last = _current_req;
		while (last->next)
			last = last->next;

		last->next = req;
	}

	return req;
}

void SPI_SetPollRate(uint32_t value) {
	TIMER_CTRL(2) = 0x0258; // CLK/8 input, IRQ on reload, disable one-shot IRQ

	if (value < 65)
		TIMER_RELOAD(2) = 0xffff;
	else
		TIMER_RELOAD(2) = (F_CPU / 8) / value;
}

void SPI_Init(SPI_Callback callback) {
	// Disable the BIOS timer handler (which for some stupid reason is enabled
	// by default, even though it does nothing) and set up custom interrupt
	// handlers.
	EnterCriticalSection();
	ChangeClearRCnt(2, 0);
	InterruptCallback(6, &_spi_poll_irq_handler);
	InterruptCallback(7, &_spi_ack_irq_handler);
	ExitCriticalSection();

	JOY_CTRL = 0x0040; // Reset all registers
	JOY_MODE = 0x000d; // 1x multiplier, 8 data bits, no parity
	JOY_BAUD = 0x0088; // 250000 bps

	SPI_SetPollRate(250);
	_current_req = 0;
	_default_cb  = callback;
}
