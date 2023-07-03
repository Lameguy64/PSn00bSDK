/*
 * PSn00bSDK standard library
 * (C) 2023 spicyjpeg - MPL licensed
 *
 * This setjmp() implementation is compatible with the one in the BIOS, making
 * it possible to pass a jmp_buf structure as-is to BIOS functions such as
 * HookEntryInt().
 */

#pragma once

#include <stdint.h>

typedef struct {
	uint32_t ra, sp, fp;
	uint32_t s0, s1, s2, s3, s4, s5, s6, s7;
	uint32_t gp;
} JumpBuffer;

typedef JumpBuffer jmp_buf[1];

#ifdef __cplusplus
extern "C" {
#endif

int setjmp(jmp_buf buf);
void longjmp(jmp_buf buf, int value);

#ifdef __cplusplus
}
#endif
