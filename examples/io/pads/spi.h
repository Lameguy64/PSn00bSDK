/*
 * PSn00bSDK controller polling example (SPI driver)
 * (C) 2021 spicyjpeg - MPL licensed
 */

#ifndef __SPI_H
#define __SPI_H

#include <stdint.h>
#include <psxpad.h>

//#define SPI_BUFF_LEN 34
#define SPI_BUFF_LEN 140

/* Request structures */

typedef void (*SPICALLBACK)(uint32_t port, const volatile uint8_t *buff, size_t rx_len);

typedef struct _SPIREQUEST {
	union {
		uint8_t        data[SPI_BUFF_LEN];
		PADREQUEST     pad_req;
		MCDREQUEST     mcd_req;
	};
	uint32_t           len, port;
	SPICALLBACK        callback;
	struct _SPIREQUEST *next;
} SPIREQUEST;

/* Public API */

/**
 * @brief Allocates a new request object and adds it to the request queue. The
 * object must be populated afterwards by setting the length, callback and
 * filling in the TX data buffer.
 */
SPIREQUEST *spi_new_request(void);

/**
 * @brief Changes the controller polling rate. The lowest supported rate is 65
 * Hz (requests sent every 1/65th of a second, each port polled at 32.5 Hz when
 * no request is pending).
 *
 * @param value
 */
void spi_set_poll_rate(uint32_t value);

/**
 * @brief Installs the SPI and timer 2 interrupt handlers and starts the poll
 * timer. By default the polling rate is set to 250 Hz (125 Hz per port),
 * however it can be changed at any time by calling spi_set_poll_rate().
 *
 * The provided callback (if any) is called to report the result of poll
 * requests, which are issued automatically when no other request is in the
 * queue. Passing NULL as callback does not disable auto-polling.
 *
 * @param callback
 */
void spi_init(SPICALLBACK callback);

#endif
