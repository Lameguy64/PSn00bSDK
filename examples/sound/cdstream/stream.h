/*
 * PSn00bSDK SPU CD-ROM streaming example (streaming API)
 * (C) 2022-2023 spicyjpeg - MPL licensed
 */

/**
 * @file stream.h
 * @brief Helper library for SPU audio streaming
 *
 * @details This is a minimal driver for SPU ADPCM streaming, with support for
 * concurrent streams (although only one can be played at a time due to SPU
 * limitations), an arbitrary number of channels for each stream and optional
 * refill and underrun callbacks. Feel free to copy and modify it.
 *
 * The driver manages a FIFO (ring buffer) in main RAM. New audio data can be
 * pushed into the FIFO at any time. When the stream is active, the SPU IRQ
 * handler will periodically pull a chunk (i.e. an array of fixed-size slices of
 * audio data, one for each channel) from the FIFO and move it to SPU RAM for
 * playback.
 *
 * As the main buffer is in main RAM, SPU RAM usage is minimal: only a double
 * buffer holding two chunks is allocated in SPU RAM. The size of each chunk
 * depends on the specified interleave and number of channels, but not on the
 * buffer size.
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Type definitions */

typedef uint32_t    Stream_Time;
typedef void        (*Stream_Callback)(void);
typedef Stream_Time (*Stream_TimerFunction)(void);

/**
 * @brief Stream initialization settings structure.
 *
 * @details This structure is used to pass settings to Stream_Init() when
 * initializing a new stream.
 *
 * The SPU RAM address is in bytes and must be aligned to 16 bytes. The
 * 0x0000-0x100f region is reserved for capture buffers and the dummy block and
 * cannot be used. The channel mask is a bitfield whose bits represent which SPU
 * channels the stream is going to use: for instance, a value of 0b1101 will
 * assign the first channel of the stream to SPU channel 0, the second channel
 * to SPU channel 2 and the third channel to SPU channel 3. The sample rate and
 * optional timer rate are in Hertz, while the interleave and buffer size are in
 * bytes.
 *
 * The refill threshold, refill callback, underrun callback and timer function
 * are optional. If provided, the callbacks will be invoked by the SPU IRQ
 * handler once the FIFO's length goes below the specified threshold and once it
 * reaches zero, respectively. The timer function will be used to improve the
 * accuracy of Stream_GetSamplesPlayed(); if not provided, VSync(-1) will be
 * used by default.
 */
typedef struct {
	uint32_t spu_address, channel_mask;
	size_t   interleave, buffer_size, refill_threshold;
	int      sample_rate, timer_rate;

	Stream_Callback      refill_callback, underrun_callback;
	Stream_TimerFunction timer_function;
} Stream_Config;

typedef struct {
	uint8_t *data;
	size_t  head, tail, length;
} Stream_Buffer;

/**
 * @brief Stream instance object.
 *
 * @details This structure represents a single audio stream. An arbitrary number
 * of streams may be created concurrently, however only one can be active at a
 * time (as the SPU only provides a single interrupt). All fields are only used
 * internally and shall not be accessed directly.
 */
typedef struct {
	Stream_Config          config;
	volatile Stream_Buffer buffer;

	void    *old_irq_handler, *old_dma_handler;
	size_t  chunk_size, samples_per_chunk;
	uint8_t num_channels;

	volatile uint8_t     db_active, buffering, callback_issued;
	volatile Stream_Time last_updated, last_stopped, play_time;
	volatile int         new_sample_rate;
} Stream_Context;

/* Public API */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes a stream context and allocates its buffers.
 *
 * @details Sets up the provided stream context with the given configuration and
 * allocates the respective FIFO in the heap. The configuration object may be
 * safely modified or discarded once the stream has been initialized.
 *
 * @param ctx
 * @param config
 *
 * @see Stream_Config, Stream_Destroy()
 */
void Stream_Init(Stream_Context *ctx, const Stream_Config *config);

/**
 * @brief Deallocates a stream context's buffers.
 *
 * @details Frees the FIFO associated with the given stream context, which must
 * have been initialized beforehand by calling Stream_Init(). Note that this
 * function does *not* deallocate the context object itself.
 *
 * @param ctx
 */
void Stream_Destroy(Stream_Context *ctx);

/**
 * @brief Starts playback of a stream.
 *
 * @details Activates the given stream context and starts or resumes playing
 * audio from its FIFO. This function must be called while no other stream is
 * active and after the stream's FIFO has been filled up. In order to prevent
 * skipping, the resume argument shall be set to true if the stream was
 * previously stopped and its buffer in SPU RAM was not overwritten by another
 * stream or sample data.
 *
 * @param ctx
 * @param resume Should be true if resuming a previously stopped stream
 * @return True if the stream was started, false if another stream is active
 *
 * @see Stream_Stop()
 */
bool Stream_Start(Stream_Context *ctx, bool resume);

/**
 * @brief Stops playback of any currently active stream.
 *
 * @return True if a stream was active and has been stopped, false otherwise
 */
bool Stream_Stop(void);

/**
 * @brief Changes the sampling rate of a stream.
 *
 * @details If the stream is currently active the change will apply on the next
 * FIFO data pull, otherwise the new sampling rate will be applied once the
 * stream is started or resumed.
 *
 * @param ctx
 * @param value
 */
void Stream_SetSampleRate(Stream_Context *ctx, int value);

/**
 * @brief Returns whether or not the given stream is currently active.
 *
 * @param ctx
 * @return True if the stream is active, false otherwise
 */
bool Stream_IsActive(const Stream_Context *ctx);

/**
 * @brief Returns an estimate of how many audio samples have been played so far.
 *
 * @details The returned value can be divided by the stream's sampling rate to
 * obtain the total playback time in seconds. The value is interpolated over
 * time using VSync(-1) as a time reference by default. If greater accuracy is
 * required, a custom timer function can be provided instead by setting the
 * appropriate fields in the configuration object before calling Stream_Init().
 *
 * @param ctx
 * @return Number of audio samples
 *
 * @see Stream_ResetSamplesPlayed()
 */
uint32_t Stream_GetSamplesPlayed(const Stream_Context *ctx);

/**
 * @brief Resets the audio sample counter returned by Stream_GetSamplesPlayed().
 *
 * @param ctx
 */
void Stream_ResetSamplesPlayed(Stream_Context *ctx);

/**
 * @brief Returns how many bytes in a stream's FIFO are currently empty and can
 * be filled.
 *
 * @param ctx
 * @return Maximum number of bytes that can currently be pushed into the FIFO
 *
 * @see Stream_GetFeedPtr().
 */
size_t Stream_GetRefillLength(const Stream_Context *ctx);

/**
 * @brief Returns a pointer to a region of a stream's FIFO to be filled in.
 *
 * @details If the given stream's FIFO is partially or completely empty, returns
 * the number of bytes that can be filled and writes a pointer to the region to
 * fill in to the provided location. Stream_Feed() must be called after any new
 * data is written in order to ensure it gets acknowledged.
 *
 * NOTE: if the empty area in the FIFO wraps around its boundary and is not
 * contiguous, only the pointer to and length of the first contiguous region
 * will be returned, with the other region being returned after the first one is
 * filled completely. The length returned by this function may thus be lower
 * than the value returned by Stream_GetRefillLength().
 *
 * @param ctx
 * @param ptr Pointer to pointer variable to be set
 * @return Maximum number of bytes that can be written to the FIFO
 *
 * @see Stream_Feed()
 */
size_t Stream_GetFeedPtr(const Stream_Context *ctx, uint8_t **ptr);

/**
 * @brief Marks data that has been written into the stream's FIFO as available.
 *
 * @details Signals that the specified number of bytes, starting from the
 * pointer returned by Stream_GetFeedPtr(), are valid and updates the FIFO's
 * length accordingly. This function does not alter the FIFO's contents, so the
 * actual data must be written before calling Stream_Feed().
 *
 * @param ctx
 * @param length Number of bytes that have been written
 *
 * @see Stream_GetFeedPtr()
 */
void Stream_Feed(Stream_Context *ctx, size_t length);

#ifdef __cplusplus
}
#endif
