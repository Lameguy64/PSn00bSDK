/*
 * PSn00bSDK MDEC library
 * (C) 2022 spicyjpeg - MPL licensed
 */

#ifndef __PSXPRESS_H
#define __PSXPRESS_H

#include <stdint.h>
#include <stddef.h>

/* Structure definitions */

typedef struct _DECDCTENV {
	uint8_t iq_y[64];	// Luma quantization table, stored in zigzag order
	uint8_t iq_c[64];	// Chroma quantization table, stored in zigzag order
	int16_t dct[64];	// Inverse DCT matrix (2.14 fixed-point)
} DECDCTENV;

typedef enum _DECDCTMODE {
	DECDCT_MODE_24BPP		= 1,
	DECDCT_MODE_16BPP		= 0,
	DECDCT_MODE_16BPP_BIT15	= 2,
	DECDCT_MODE_RAW			= -1
} DECDCTMODE;

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Resets the MDEC and aborts any MDEC DMA transfers. If mode = 0, the
 * default IDCT matrix and quantization tables are also loaded and the MDEC is
 * put into color output mode, discarding any custom environment previously set
 * with DecDCTPutEnv().
 *
 * DecDCTReset(0) must be called at least once prior to using the MDEC.
 *
 * @param mode
 */
void DecDCTReset(int32_t mode);

/**
 * @brief Uploads the specified decoding environment's quantization tables and
 * IDCT matrix to the MDEC, or restores the default tables if a null pointer is
 * passed. Calling this function is normally not required as DecDCTReset(0)
 * initializes the MDEC with the default tables, but it may be useful for e.g.
 * decoding JPEG or a format with custom quantization tables.
 *
 * The second argument, not present in the official SDK, specifies whether the
 * MDEC shall be put into color (0) or monochrome (1) output mode. In
 * monochrome mode each DCT block decoded from the input stream is transformed
 * into an 8x8x8bpp bitmap, while in color mode each group of 6 DCT blocks (Cr,
 * Cb, Y1-4) is used to form a 16x16 RGB bitmap.
 *
 * This function uses DecDCTinSync() to wait for the MDEC to become ready and
 * should not be called during decoding or after calling DecDCTin().
 *
 * @param env Pointer to DECDCTENV or 0 for default tables
 * @param mono 0 for color (normal), 1 for monochrome
 */
void DecDCTPutEnv(const DECDCTENV *env, int32_t mono);

/**
 * @brief Sets up the MDEC to start fetching and decoding a stream from the
 * given address in main RAM. The first 32-bit word is initially copied to the
 * MDEC0 register, then all subsequent data is read in 128-byte (32-word)
 * chunks. The length of the stream (in 32-bit units, minus the first word)
 * must be encoded in the lower 16 bits of the first word, as expected by the
 * MDEC.
 *
 * The mode argument optionally specifies the output color depth (0 for 16bpp,
 * 1 for 24bpp) if not already set in the first word. Passing -1 will result in
 * DecDCTin() copying the first word as-is to MDEC0 without manipulating any of
 * its bits.
 *
 * @param data
 * @param mode DECDCT_MODE_* or -1
 */
void DecDCTin(const uint32_t *data, int32_t mode);

/**
 * @brief Configures the MDEC to automatically fetch data (the input stream,
 * IDCT matrix or quantization tables) in 128-byte (32-word) chunks from the
 * specified address in main RAM. The transfer is stopped, and any callback
 * registered with DMACallback(0) is fired, once a certain number of 32-bit
 * words have been read; usually the length should match the number of input
 * words expected by the MDEC. If the MDEC expects more data its operation will
 * be paused and can be resumed by calling DecDCTinRaw() again.
 *
 * This is a low-level variant of DecDCTin() that only sets up the DMA transfer
 * and does not write anything to the MDEC0 register. The actual transfer won't
 * start until the MDEC is given a valid command.
 * 
 * @param data
 * @param length Number of 32-bit words to read (must be multiple of 32)
 */
void DecDCTinRaw(const uint32_t *data, size_t length);

/**
 * @brief Waits for the MDEC to finish decoding the input stream (if mode = 0)
 * or returns whether it is busy (if mode = 1). MDEC commands can be issued
 * only when the MDEC isn't busy.
 *
 * WARNING: DecDCTinSync(0) might time out and return -1 if the MDEC can't
 * output decoded data, e.g. if the length passed DecDCTout() was too small and
 * no callback is registered to set up further transfers. DecDCTinSync(0) shall
 * only be used alongside DMACallback(1) or if the entirety of the decoded
 * stream (usually a whole frame) is being written to main RAM.
 *
 * @param mode
 * @return 0 or -1 in case of a timeout (mode = 0) / MDEC busy flag (mode = 1)
 */
int32_t DecDCTinSync(int32_t mode);

/**
 * @brief Configures the MDEC to automatically transfer decoded image data in
 * 128-byte (32-word) chunks to the specified address in main RAM. MDEC
 * operation is paused once a certain number of 32-bit words have been output
 * and can be resumed by calling DecDCTout() again: the MDEC will continue
 * decoding the input stream from where it left off. Any callback registered
 * with DMACallback(1) is also fired whenever the transfer ends.
 *
 * This behavior allows the MDEC's output to be buffered into 16-pixel-wide
 * vertical strips in main RAM, which can then be uploaded to VRAM using
 * LoadImage().
 *
 * @param data
 * @param length Number of 32-bit words to output (must be multiple of 32)
 */
void DecDCTout(uint32_t *data, size_t length);

/**
 * @brief Waits until the transfer set up by DecDCTout() finishes (if mode = 0)
 * or returns whether it is still in progress (if mode = 1).
 *
 * WARNING: DecDCToutSync(0) might time out and return -1 if the MDEC is unable
 * to consume enough input data in order to produce the desired amount of data.
 * If the input stream isn't contiguous in memory, DMACallback(0) shall be used
 * to register a callback that calls DecDCTin() to feed the MDEC.
 *
 * @param mode
 * @return 0 or -1 in case of a timeout (mode = 0) / DMA busy flag (mode = 1)
 */
int32_t DecDCToutSync(int32_t mode);

#ifdef __cplusplus
}
#endif

#endif
