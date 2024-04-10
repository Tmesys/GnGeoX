/*!
*
*   \file    GnGeoXmamelayer.h
*   \brief   MAME compatibility layer header ?
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    03/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
/* Here come as much as I can to preserve mame source code */
#ifndef _GNGEOX_MAME_LAYER_H_
#define _GNGEOX_MAME_LAYER_H_

#include <stdlib.h>
#include <string.h>

/* macros for accessing bytes and words within larger chunks */
#ifndef BIGENDIAN

#define BYTE_XOR_BE(a)                  ((a) ^ 1)       /* read/write a byte to a 16-bit space */
#define BYTE_XOR_LE(a)                  (a)
#define BYTE4_XOR_BE(a)                 ((a) ^ 3)       /* read/write a byte to a 32-bit space */
#define BYTE4_XOR_LE(a)                 (a)
#define WORD_XOR_BE(a)                  ((a) ^ 2)       /* read/write a word to a 32-bit space */
#define WORD_XOR_LE(a)                  (a)
#define BYTE8_XOR_BE(a)                 ((a) ^ 7)       /* read/write a byte to a 64-bit space */
#define BYTE8_XOR_LE(a)                 (a)
#define WORD2_XOR_BE(a)                 ((a) ^ 6)       /* read/write a word to a 64-bit space */
#define WORD2_XOR_LE(a)                 (a)
#define DWORD_XOR_BE(a)                 ((a) ^ 4)       /* read/write a dword to a 64-bit space */
#define DWORD_XOR_LE(a)                 (a)

#else

#define BYTE_XOR_BE(a)                  (a)
#define BYTE_XOR_LE(a)                  ((a) ^ 1)       /* read/write a byte to a 16-bit space */
#define BYTE4_XOR_BE(a)                 (a)
#define BYTE4_XOR_LE(a)                 ((a) ^ 3)       /* read/write a byte to a 32-bit space */
#define WORD_XOR_BE(a)                  (a)
#define WORD_XOR_LE(a)                  ((a) ^ 2)       /* read/write a word to a 32-bit space */
#define BYTE8_XOR_BE(a)                 (a)
#define BYTE8_XOR_LE(a)                 ((a) ^ 7)       /* read/write a byte to a 64-bit space */
#define WORD2_XOR_BE(a)                 (a)
#define WORD2_XOR_LE(a)                 ((a) ^ 6)       /* read/write a word to a 64-bit space */
#define DWORD_XOR_BE(a)                 (a)
#define DWORD_XOR_LE(a)                 ((a) ^ 4)       /* read/write a dword to a 64-bit space */

#endif

/* Useful macros to deal with bit shuffling encryptions */
#define BIT(x,n) (((x)>>(n))&1)

#define BITSWAP8(val,B7,B6,B5,B4,B3,B2,B1,B0) \
        ((BIT(val,B7) << 7) | \
         (BIT(val,B6) << 6) | \
         (BIT(val,B5) << 5) | \
         (BIT(val,B4) << 4) | \
         (BIT(val,B3) << 3) | \
         (BIT(val,B2) << 2) | \
         (BIT(val,B1) << 1) | \
         (BIT(val,B0) << 0))

#define BITSWAP15(val,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
        ((BIT(val,B14) << 14) | \
         (BIT(val,B13) << 13) | \
         (BIT(val,B12) << 12) | \
         (BIT(val,B11) << 11) | \
         (BIT(val,B10) << 10) | \
         (BIT(val, B9) <<  9) | \
         (BIT(val, B8) <<  8) | \
         (BIT(val, B7) <<  7) | \
         (BIT(val, B6) <<  6) | \
         (BIT(val, B5) <<  5) | \
         (BIT(val, B4) <<  4) | \
         (BIT(val, B3) <<  3) | \
         (BIT(val, B2) <<  2) | \
         (BIT(val, B1) <<  1) | \
         (BIT(val, B0) <<  0))

#define BITSWAP16(val,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
        ((BIT(val,B15) << 15) | \
         (BIT(val,B14) << 14) | \
         (BIT(val,B13) << 13) | \
         (BIT(val,B12) << 12) | \
         (BIT(val,B11) << 11) | \
         (BIT(val,B10) << 10) | \
         (BIT(val, B9) <<  9) | \
         (BIT(val, B8) <<  8) | \
         (BIT(val, B7) <<  7) | \
         (BIT(val, B6) <<  6) | \
         (BIT(val, B5) <<  5) | \
         (BIT(val, B4) <<  4) | \
         (BIT(val, B3) <<  3) | \
         (BIT(val, B2) <<  2) | \
         (BIT(val, B1) <<  1) | \
         (BIT(val, B0) <<  0))

#define BITSWAP19(val,B18,B17,B16,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
        ((BIT(val,B18) << 18) | \
         (BIT(val,B17) << 17) | \
         (BIT(val,B16) << 16) | \
         (BIT(val,B15) << 15) | \
         (BIT(val,B14) << 14) | \
         (BIT(val,B13) << 13) | \
         (BIT(val,B12) << 12) | \
         (BIT(val,B11) << 11) | \
         (BIT(val,B10) << 10) | \
         (BIT(val, B9) <<  9) | \
         (BIT(val, B8) <<  8) | \
         (BIT(val, B7) <<  7) | \
         (BIT(val, B6) <<  6) | \
         (BIT(val, B5) <<  5) | \
         (BIT(val, B4) <<  4) | \
         (BIT(val, B3) <<  3) | \
         (BIT(val, B2) <<  2) | \
         (BIT(val, B1) <<  1) | \
         (BIT(val, B0) <<  0))

#define BITSWAP24(val,B23,B22,B21,B20,B19,B18,B17,B16,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
        ((BIT(val,B23) << 23) | \
         (BIT(val,B22) << 22) | \
         (BIT(val,B21) << 21) | \
         (BIT(val,B20) << 20) | \
         (BIT(val,B19) << 19) | \
         (BIT(val,B18) << 18) | \
         (BIT(val,B17) << 17) | \
         (BIT(val,B16) << 16) | \
         (BIT(val,B15) << 15) | \
         (BIT(val,B14) << 14) | \
         (BIT(val,B13) << 13) | \
         (BIT(val,B12) << 12) | \
         (BIT(val,B11) << 11) | \
         (BIT(val,B10) << 10) | \
         (BIT(val, B9) <<  9) | \
         (BIT(val, B8) <<  8) | \
         (BIT(val, B7) <<  7) | \
         (BIT(val, B6) <<  6) | \
         (BIT(val, B5) <<  5) | \
         (BIT(val, B4) <<  4) | \
         (BIT(val, B3) <<  3) | \
         (BIT(val, B2) <<  2) | \
         (BIT(val, B1) <<  1) | \
         (BIT(val, B0) <<  0))

#define BITSWAP32(val,B31,B30,B29,B28,B27,B26,B25,B24,B23,B22,B21,B20,B19,B18,B17,B16,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
        ((BIT(val,B31) << 31) | \
         (BIT(val,B30) << 30) | \
         (BIT(val,B29) << 29) | \
         (BIT(val,B28) << 28) | \
         (BIT(val,B27) << 27) | \
         (BIT(val,B26) << 26) | \
         (BIT(val,B25) << 25) | \
         (BIT(val,B24) << 24) | \
         (BIT(val,B23) << 23) | \
         (BIT(val,B22) << 22) | \
         (BIT(val,B21) << 21) | \
         (BIT(val,B20) << 20) | \
         (BIT(val,B19) << 19) | \
         (BIT(val,B18) << 18) | \
         (BIT(val,B17) << 17) | \
         (BIT(val,B16) << 16) | \
         (BIT(val,B15) << 15) | \
         (BIT(val,B14) << 14) | \
         (BIT(val,B13) << 13) | \
         (BIT(val,B12) << 12) | \
         (BIT(val,B11) << 11) | \
         (BIT(val,B10) << 10) | \
         (BIT(val, B9) <<  9) | \
         (BIT(val, B8) <<  8) | \
         (BIT(val, B7) <<  7) | \
         (BIT(val, B6) <<  6) | \
         (BIT(val, B5) <<  5) | \
         (BIT(val, B4) <<  4) | \
         (BIT(val, B3) <<  3) | \
         (BIT(val, B2) <<  2) | \
         (BIT(val, B1) <<  1) | \
         (BIT(val, B0) <<  0))

#define alloc_array_or_die(type,size) ((type*)malloc_or_die(sizeof(type)*size))

typedef enum
{
    GNGEO_MEMORYREGION_AUDIOCPU = 0,
    GNGEO_MEMORYREGION_AUDIOCRYPT = 1,
    GNGEO_MEMORYREGION_FIXED = 2,
    GNGEO_MEMORYREGION_MAINCPU = 3,
    GNGEO_MEMORYREGION_MAINBIOS = 4,
    GNGEO_MEMORYREGION_SPRITES = 5,
    GNGEO_MEMORYREGION_YM = 6,
} enum_gngeoxmamelayer_memoryregion;

Uint8* memory_region ( struct_gngeoxroms_game_roms*, enum_gngeoxmamelayer_memoryregion ) __attribute__ ( ( warn_unused_result ) );
Uint32 memory_region_length ( struct_gngeoxroms_game_roms*, enum_gngeoxmamelayer_memoryregion ) __attribute__ ( ( warn_unused_result ) );
void* malloc_or_die ( Uint32 ) __attribute__ ( ( warn_unused_result ) );

#endif
