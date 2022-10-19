/*
 * PSn00bSDK interrupt management library
 * (C) 2019-2022 Lameguy64, spicyjpeg - MPL licensed
 */

#ifndef __PSXETC_H
#define __PSXETC_H

/* Macros */

// This macro is used internally by PSn00bSDK to log debug messages to a buffer
// which is then printed to stdout when calling VSync().
#ifdef NDEBUG
#define _sdk_log(...)
#define _sdk_dump_log()
#else
#define _sdk_log(...)	_sdk_log_inner(__VA_ARGS__)
#define _sdk_dump_log()	_sdk_dump_log_inner()
#endif

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

void _sdk_log_inner(const char *fmt, ...);
void _sdk_dump_log_inner(void);

void *InterruptCallback(int irq, void (*func)(void));
void *GetInterruptCallback(int irq);
void *DMACallback(int dma, void (*func)(void));
void *GetDMACallback(int dma);

int ResetCallback(void);
void RestartCallback(void);
void StopCallback(void);

#ifdef __cplusplus
}
#endif

#endif
