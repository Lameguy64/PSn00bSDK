/*
 * PSn00bSDK Konami System 573 example (I/O driver)
 * (C) 2022 spicyjpeg - MPL licensed
 */

#include <stdint.h>
#include <psxgpu.h>
#include <hwregs_c.h>

#include "k573io.h"

uint32_t K573_GetJAMMAInputs(void) {
	uint32_t inputs;

	inputs  =   K573_JAMMA_IN;
	inputs |= ((K573_EXT_IN_P1 >> 8) & 0x0f) << 16;
	inputs |= ((K573_EXT_IN_P2 >> 8) & 0x0f) << 20;
	inputs |= ((K573_MISC_IN   >> 8) & 0x1f) << 24;
	inputs |=  (K573_DIP_CART_IN     & 0x07) << 29;

	return ~inputs;
}

void K573_SetAnalogLights(uint32_t lights) {
	uint32_t bits = 0xffffffff;

	bits ^= (lights & 0x00010101) <<  7; // Bit 0 -> bit 7
	bits ^= (lights & 0x00020202) >>  1; // Bit 1 -> bit 0
	bits ^= (lights & 0x00040404) <<  4; // Bit 2 -> bit 6
	bits ^= (lights & 0x00080808) >>  2; // Bit 3 -> bit 1
	bits ^= (lights & 0x00101010) <<  1; // Bit 4 -> bit 5
	bits ^= (lights & 0x00202020) >>  3; // Bit 5 -> bit 2
	bits ^= (lights & 0x00404040) >>  2; // Bit 6 -> bit 4
	bits ^= (lights & 0x00808080) >>  4; // Bit 7 -> bit 3
	bits ^= (lights & 0x0f000000) >> 24;

	K573_IO_BOARD[ANALOG_IO_REG_LIGHTS_A] = (bits)       & 0xff;
	K573_IO_BOARD[ANALOG_IO_REG_LIGHTS_B] = (bits >>  8) & 0xff;
	K573_IO_BOARD[ANALOG_IO_REG_LIGHTS_C] = (bits >> 16) & 0xff;
	K573_IO_BOARD[ANALOG_IO_REG_LIGHTS_D] = (bits >> 24) & 0xff;
}

void K573_Init(void) {
	BUS_EXP1_ADDR = 0x1f000000;
	BUS_EXP1_CFG  = 0x24173f47; // 573 BIOS uses this value

	// Bit 6 of this register controls the audio DAC and must be set, otherwise
	// no sound will be output. Most of the other bits are data clocks/strobes
	// and should be pulled high when not in use.
	K573_MISC_OUT =
		MISC_OUT_ADC_MOSI | MISC_OUT_ADC_CS | MISC_OUT_ADC_SCK |
		MISC_OUT_CDDA_ENABLE | MISC_OUT_SPU_ENABLE | MISC_OUT_MCU_CLOCK;

	K573_RESET_WATCHDOG();
}
