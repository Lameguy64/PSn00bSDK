/*!	\file lzconfig.h
 *	\brief Library configuration header
 *	\details Define settings will only take effect when you recompile the library.
 */

#pragma once

/* Uncomment to make default compression table sizes to maximum and works best
 * when compressing large amounts of data. LZP_USE_MALLOC must be set to TRUE to
 * prevent stack overflow errors.
 *
 * Do not enable this if you plan to compile for a platform with limited memory
 * otherwise, the library will consume all memory and crash the system.
 *
 * This define only affects lzCompress().
 */
//#define LZP_MAX_COMPRESS

/* Uncomment to make the library use malloc() instead of array initializers to
 * allocate hash tables. Enabling this is a must if you plan to use large hash
 * and window table sizes.
 */
//#define LZP_USE_MALLOC


#if defined(PSN00BSDK) && !defined(LZP_MAX_COMPRESS)

// Minimal defaults
#define LZP_WINDOW_SIZE	17
#define LZP_HASH1_SIZE	8
#define LZP_HASH2_SIZE	10

#else

#define LZP_USE_MALLOC

// Maximum defaults
#define LZP_WINDOW_SIZE	17
#define LZP_HASH1_SIZE	22
#define LZP_HASH2_SIZE	24

#endif
