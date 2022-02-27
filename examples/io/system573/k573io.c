/*
 * PSn00bSDK Konami System 573 example (I/O driver)
 * (C) 2022 spicyjpeg - MPL licensed
 *
 * Note that this is far from being a complete driver. It currently lacks:
 * - ATAPI driver
 * - Flash erasing/writing APIs
 * - JVS bus APIs
 * - Functions for accessing the digital I/O board's MP3 decoder
 */

#include <stdint.h>
#include <psxgpu.h>

#include "k573io.h"

K573_IOBoardType _board_type = IO_TYPE_ANALOG;

/* I/O board light control */

static void _k573_set_lights_analog(uint32_t lights) {
	uint32_t bits = 0xffffffff;

	bits ^= (lights & 0x01010101) << 7; // Lamp n*8+0 -> bit n*8+7
	bits ^= (lights & 0x02020202) << 5; // Lamp n*8+1 -> bit n*8+6
	bits ^= (lights & 0x04040404) >> 1; // Lamp n*8+2 -> bit n*8+1
	bits ^= (lights & 0x08080808) >> 3; // Lamp n*8+3 -> bit n*8+0
	bits ^= (lights & 0x10101010) << 1; // Lamp n*8+4 -> bit n*8+5
	bits ^= (lights & 0x20202020) >> 1; // Lamp n*8+5 -> bit n*8+4
	bits ^= (lights & 0x40404040) >> 3; // Lamp n*8+6 -> bit n*8+3
	bits ^= (lights & 0x80808080) >> 5; // Lamp n*8+7 -> bit n*8+2

	K573_IO_BOARD[ANALOG_IO_REG_LIGHTS0] = (bits)       & 0xff;
	K573_IO_BOARD[ANALOG_IO_REG_LIGHTS1] = (bits >>  8) & 0xff;
	K573_IO_BOARD[ANALOG_IO_REG_LIGHTS2] = (bits >> 16) & 0xff;
	K573_IO_BOARD[ANALOG_IO_REG_LIGHTS3] = (bits >> 24) & 0xff;
}

// This function controls light outputs on digital I/O boards (i.e. the ones
// that also include MP3 playback hardware, used by most Bemani games).
// TODO: test this on real hardware -- it might not work if lights are handled
// by the board's FPGA, which requires a binary blob...
static void _k573_set_lights_digital(uint32_t lights) {
	uint32_t bits = 0xffffffff;

	bits ^= (lights & 0x11111111);      // Lamp n*4+0 -> bit n*4+0
	bits ^= (lights & 0x22222222) << 1; // Lamp n*4+1 -> bit n*4+2
	bits ^= (lights & 0x44444444) << 1; // Lamp n*4+2 -> bit n*4+3
	bits ^= (lights & 0x88888888) >> 2; // Lamp n*4+3 -> bit n*4+1

	K573_IO_BOARD[DIGITAL_IO_REG_LIGHTS0] = ((bits)       & 0xf) << 12;
	K573_IO_BOARD[DIGITAL_IO_REG_LIGHTS1] = ((bits >>  4) & 0xf) << 12;
	K573_IO_BOARD[DIGITAL_IO_REG_LIGHTS2] = ((bits >>  8) & 0xf) << 12;
	K573_IO_BOARD[DIGITAL_IO_REG_LIGHTS3] = ((bits >> 12) & 0xf) << 12;
	K573_IO_BOARD[DIGITAL_IO_REG_LIGHTS4] = ((bits >> 16) & 0xf) << 12;
	K573_IO_BOARD[DIGITAL_IO_REG_LIGHTS5] = ((bits >> 20) & 0xf) << 12;
	//K573_IO_BOARD[DIGITAL_IO_REG_LIGHTS6] = ((bits >> 24) & 0xf) << 12;
	K573_IO_BOARD[DIGITAL_IO_REG_LIGHTS7] = ((bits >> 28) & 0xf) << 12;
}

static const void (*_k573_set_lights[])(uint32_t) = {
	&_k573_set_lights_analog,
	&_k573_set_lights_digital
};

/* Public API */

uint32_t K573_GetJAMMAInputs(void) {
	uint32_t inputs;

	inputs  =   K573_IO_CHIP[IO_REG_IN2];
	inputs |= ((K573_IO_CHIP[IO_REG_IN3_LOW]  >> 8) & 0x0f) << 16;
	inputs |= ((K573_IO_CHIP[IO_REG_IN3_HIGH] >> 8) & 0x0f) << 20;
	inputs |= ((K573_IO_CHIP[IO_REG_IN1_HIGH] >> 8) & 0x1f) << 24;
	inputs |=  (K573_IO_CHIP[IO_REG_IN1_LOW]        & 0x07) << 29;

	return inputs;
}

void K573_SetLights(uint32_t lights) {
	if (_board_type > IO_TYPE_DIGITAL)
		return;

	_k573_set_lights[_board_type](lights);
}

void K573_SetBoardType(K573_IOBoardType type) {
	_k573_set_lights[_board_type](0);
	_board_type = type;
}

/*void K573_DDRStageCommand(uint32_t value, uint32_t length) {
	uint32_t last_bit = 0;
	uint32_t mask     = 1;

	for (uint32_t i = 0; i < length; i++) {
		uint32_t bit = DDR_LIGHT_P1_MUX_DATA | DDR_LIGHT_P2_MUX_DATA;
		if (value & mask)
			bit = 0;

		K573_SetLights(last_bit | DDR_LIGHT_P1_MUX_CLK | DDR_LIGHT_P2_MUX_CLK);
		_k573_delay_hblanks(20);
		K573_SetLights(bit | DDR_LIGHT_P1_MUX_CLK | DDR_LIGHT_P2_MUX_CLK);
		_k573_delay_hblanks(20);
		K573_SetLights(bit);
		_k573_delay_hblanks(20);

		last_bit = bit;
		mask   <<= 1;
	}

	K573_SetLights(0);
}*/

//K573_DDRStageCommand(0x000c90, 13);
//K573_DDRStageCommand(0x000001, 22);

void K573_Init(void) {
	EXP1_ADDR = 0x1f000000;
	EXP1_CTRL = 0x24173f47; // 573 BIOS uses this value

	K573_RESET_WATCHDOG();
}
