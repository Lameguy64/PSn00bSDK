/*
 * PSn00bSDK internal debug logger
 * (C) 2022 spicyjpeg - MPL licensed
 *
 * This file provides the (admittedly minimal) logging system used by all
 * PSn00bSDK libraries. Log messages and warnings are issued using the
 * _sdk_log() macro and collected into a buffer, whose contents can be flushed
 * by calling _sdk_dump_log() (by default this is done by VSync()). Logging is
 * only enabled in debug builds of libpsn00b.
 */

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <psxapi.h>
#include <psxetc.h>

#define LOG_BUFFER_SIZE		256

#ifndef NDEBUG

/* Internal globals */

static char   _log_buffer[LOG_BUFFER_SIZE];
static size_t _log_buffer_length = 0;

/* Internal logging API */

void _sdk_log_inner(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	_log_buffer_length += vsnprintf(
		&_log_buffer[_log_buffer_length],
		LOG_BUFFER_SIZE - _log_buffer_length,
		fmt,
		ap
	);
	va_end(ap);
}

void _sdk_dump_log_inner(void) {
	if (!_log_buffer_length)
		return;

	write(1, _log_buffer, _log_buffer_length);
	_log_buffer_length = 0;
}

#endif
