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
#include <string.h>
#include <stdlib.h>
#include <psxetc.h>
#include <psxapi.h>
#include <psxpad.h>

#include "spi.h"

/* Register definitions */

#define F_CPU 33868800UL

#define TIM_VALUE(N)	*((volatile uint32_t *) 0x1f801100 + 4 * (N))
#define TIM_CTRL(N)		*((volatile uint32_t *) 0x1f801104 + 4 * (N))
#define TIM_RELOAD(N)	*((volatile uint32_t *) 0x1f801108 + 4 * (N))

// IMPORTANT: even though JOY_TXRX is a 32-bit register, it should only be
// accessed as 8-bit. Reading it as 16 or 32-bit works fine on real hardware,
// but leads to problems in some emulators.
#define JOY_TXRX		*((volatile uint8_t *)  0x1f801040)
#define JOY_STAT		*((volatile uint16_t *) 0x1f801044)
#define JOY_MODE		*((volatile uint16_t *) 0x1f801048)
#define JOY_CTRL		*((volatile uint16_t *) 0x1f80104a)
#define JOY_BAUD		*((volatile uint16_t *) 0x1f80104e)

/* Internal structures and globals */

typedef struct _SPICONTEXT {
	uint8_t     tx_buff[SPI_BUFF_LEN];
	uint8_t     rx_buff[SPI_BUFF_LEN];
	uint32_t    tx_len, rx_len, port;
	SPICALLBACK callback;
} SPICONTEXT;

static volatile SPICONTEXT          ctx;
static volatile SPIREQUEST volatile *current_req;
static SPICALLBACK                  default_cb;

/* Request queue management */

static void prepare_poll_req(void) {
	PADREQUEST *req = (PADREQUEST *) ctx.tx_buff;

	req->addr     = 0x01;
	req->cmd      = PAD_CMD_READ;
	req->tap_mode = 0x00; // 0x01 to enable extended multitap response
	req->motor_l  = 0x00;
	req->motor_r  = 0x00;

	ctx.tx_len   = 4;
	ctx.rx_len   = 0;
	ctx.port    ^= 1;
	ctx.callback = default_cb;
}

static void prepare_next_req(void) {
	// Copy the contents of the first request in the queue into the TX buffer.
	memcpy((void *) ctx.tx_buff, (void *) current_req->data, current_req->len);

	ctx.tx_len   = current_req->len;
	ctx.rx_len   = 0;
	ctx.port     = current_req->port;
	ctx.callback = current_req->callback;

	// Pop the first request from the queue by deallocating it and adjusting
	// the pointer to the first queue item.
	SPIREQUEST *next = current_req->next;

	free((void *) current_req);
	current_req = next;
}

/* Interrupt handlers */

static void poll_timer_tick(void) {
	// Fetch the last response byte, which wasn't followed by a pulse on /ACK,
	// from the RX FIFO.
	if (JOY_STAT & 0x0002)
		ctx.rx_buff[ctx.rx_len - 1] = (uint8_t) JOY_TXRX;

	if (ctx.callback)
		ctx.callback(ctx.port, ctx.rx_buff, ctx.rx_len);

	// If the request queue is empty, create a pad polling request.
	if (current_req)
		prepare_next_req();
	else
		prepare_poll_req();

	// Prepare the SPI port by clearing any pending IRQ, pulling /CS high and
	// enabling the /ACK IRQ. In order to communicate with controllers, /CS has
	// to be driven low again for about 20 us before sending the first byte.
	JOY_CTRL = 0x0010;
	for (uint32_t i = 0; i < 50; i++)
		__asm__("nop");

	JOY_CTRL = 0x1003 | (ctx.port << 13);
	for (uint32_t i = 0; i < 500; i++)
		__asm__("nop");

	// Send the first byte indicating which device to address. If the matching
	// device is connected, it will reply by triggering the /ACK IRQ.
	JOY_TXRX = ctx.tx_buff[0];
}

static void spi_ack_handler(void) {
	// Wait until /ACK is pulled up by the controller before sending the next
	// byte. According to nocash docs, this has to be done before resetting the
	// IRQ.
	while (JOY_STAT & 0x0080)
		__asm__("nop");

	// Keep /CS pulled low and acknowledge the IRQ (bit 4) to ensure it can be
	// triggered again.
	JOY_CTRL = 0x1013 | (ctx.port << 13);

	if (!ctx.rx_len) {
		// We just sent the first address byte. Obviously the response we
		// received was read from an open bus, so the SPI port's internal FIFO
		// must be flushed (by performing dummy reads) to ensure we are only
		// going to read valid data from now on.
		JOY_TXRX;

	} else if (ctx.rx_len <= SPI_BUFF_LEN) {
		// If this is not the first byte, put it in the RX buffer.
		ctx.rx_buff[ctx.rx_len - 1] = (uint8_t) JOY_TXRX;
	}

	// Send the next byte, or a null byte if there is no more data to send and
	// we're just reading a response.
	ctx.rx_len++;
	if (ctx.rx_len < ctx.tx_len)
		JOY_TXRX = (uint32_t) ctx.tx_buff[ctx.rx_len];
	else
		JOY_TXRX = 0x00;
}

/* Public API */

SPIREQUEST *spi_new_request(void) {
	SPIREQUEST *req = malloc(sizeof(SPIREQUEST));

	req->len      = 0;
	req->port     = 0;
	req->callback = 0;
	req->next     = 0;

	// Find the last queued request by traversing the linked list and append a
	// pointer to the new request.
	if (!current_req) {
		current_req = req;
	} else {
		volatile SPIREQUEST *volatile last = current_req;
		while (last->next)
			last = last->next;

		last->next = req;
	}

	return req;
}

void spi_set_poll_rate(uint32_t value) {
	TIM_CTRL(2) = 0x0258; // CLK/8 input, IRQ on reload, disable one-shot IRQ

	if (value < 65)
		TIM_RELOAD(2) = 0xffff;
	else
		TIM_RELOAD(2) = (F_CPU / 8) / value;
}

void spi_init(SPICALLBACK callback) {
	// Disable the BIOS timer handler (which for some stupid reason is enabled
	// by default, even though it does nothing) and set up custom interrupt
	// handlers.
	EnterCriticalSection();
	ChangeClearRCnt(2, 0);
	InterruptCallback(6, &poll_timer_tick);
	InterruptCallback(7, &spi_ack_handler);
	ExitCriticalSection();

	JOY_CTRL = 0x0040; // Reset all registers
	JOY_MODE = 0x000d; // 1x multiplier, 8 data bits, no parity
	JOY_BAUD = 0x0088; // 250000 bps

	spi_set_poll_rate(250);
	current_req = 0;
	default_cb  = callback;
}
