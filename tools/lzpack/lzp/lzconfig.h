/*!	\file lzconfig.h
 *	\brief Library configuration header
 *	\details Define settings will only take effect when you recompile the library.
 */

#ifndef _LZP_CONFIG_H
#define _LZP_CONFIG_H


#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif


/* Set to TRUE to compile without data compression routines useful if you
 * plan to use this library on a program that does not require said routines
 * especially on a platform with limited memory (such as the PlayStation).
 *
 * This define will rule out lzCompress(), lzSetHashSizes() and
 * lzResetHashSizes() functions and their associated functions.
 */
#define LZP_NO_COMPRESS		FALSE


/* Set to TRUE to make default compression table sizes to maximum and works best
 * when compressing large amounts of data. LZP_USE_MALLOC must be set to TRUE to
 * prevent stack overflow errors.
 *
 * Do not enable this if you plan to compile for a platform with limited memory
 * otherwise, the library will consume all memory and crash the system.
 *
 * This define only affects lzCompress().
 */
#define LZP_MAX_COMPRESS	TRUE


/* Uncomment to make the library use malloc() instead of array initializers to
 * allocate hash tables. Enabling this is a must if you plan to use large hash
 * and window table sizes.
 */
#define LZP_USE_MALLOC		TRUE


/* Hash table sizes (in power-of-two multiple units)
 *
 * These define only affect lzCompress().
 */
#if LZP_MAX_COMPRESS == TRUE

// Minimal defaults
#define LZP_WINDOW_SIZE	17
#define LZP_HASH1_SIZE	8
#define LZP_HASH2_SIZE	10

#else

// Maximum defaults
#define LZP_WINDOW_SIZE	17
#define LZP_HASH1_SIZE	22
#define LZP_HASH2_SIZE	24

#endif


#endif // _LZP_CONFIG_H
