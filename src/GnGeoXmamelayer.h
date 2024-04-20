/*!
*
*   \file    GnGeoXmamelayer.h
*   \brief   MAME compatibility layer header ?
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
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
#define RCHIFT_BIT(x,n) (((x)>>(n))&1)

#define BITSWAP8(val,B7,B6,B5,B4,B3,B2,B1,B0) \
        ((RCHIFT_BIT(val,B7) << 7) | \
         (RCHIFT_BIT(val,B6) << 6) | \
         (RCHIFT_BIT(val,B5) << 5) | \
         (RCHIFT_BIT(val,B4) << 4) | \
         (RCHIFT_BIT(val,B3) << 3) | \
         (RCHIFT_BIT(val,B2) << 2) | \
         (RCHIFT_BIT(val,B1) << 1) | \
         (RCHIFT_BIT(val,B0) << 0))

#define BITSWAP15(val,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
        ((RCHIFT_BIT(val,B14) << 14) | \
         (RCHIFT_BIT(val,B13) << 13) | \
         (RCHIFT_BIT(val,B12) << 12) | \
         (RCHIFT_BIT(val,B11) << 11) | \
         (RCHIFT_BIT(val,B10) << 10) | \
         (RCHIFT_BIT(val, B9) <<  9) | \
         (RCHIFT_BIT(val, B8) <<  8) | \
         (RCHIFT_BIT(val, B7) <<  7) | \
         (RCHIFT_BIT(val, B6) <<  6) | \
         (RCHIFT_BIT(val, B5) <<  5) | \
         (RCHIFT_BIT(val, B4) <<  4) | \
         (RCHIFT_BIT(val, B3) <<  3) | \
         (RCHIFT_BIT(val, B2) <<  2) | \
         (RCHIFT_BIT(val, B1) <<  1) | \
         (RCHIFT_BIT(val, B0) <<  0))

#define BITSWAP16(val,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
        ((RCHIFT_BIT(val,B15) << 15) | \
         (RCHIFT_BIT(val,B14) << 14) | \
         (RCHIFT_BIT(val,B13) << 13) | \
         (RCHIFT_BIT(val,B12) << 12) | \
         (RCHIFT_BIT(val,B11) << 11) | \
         (RCHIFT_BIT(val,B10) << 10) | \
         (RCHIFT_BIT(val, B9) <<  9) | \
         (RCHIFT_BIT(val, B8) <<  8) | \
         (RCHIFT_BIT(val, B7) <<  7) | \
         (RCHIFT_BIT(val, B6) <<  6) | \
         (RCHIFT_BIT(val, B5) <<  5) | \
         (RCHIFT_BIT(val, B4) <<  4) | \
         (RCHIFT_BIT(val, B3) <<  3) | \
         (RCHIFT_BIT(val, B2) <<  2) | \
         (RCHIFT_BIT(val, B1) <<  1) | \
         (RCHIFT_BIT(val, B0) <<  0))

#define BITSWAP19(val,B18,B17,B16,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
        ((RCHIFT_BIT(val,B18) << 18) | \
         (RCHIFT_BIT(val,B17) << 17) | \
         (RCHIFT_BIT(val,B16) << 16) | \
         (RCHIFT_BIT(val,B15) << 15) | \
         (RCHIFT_BIT(val,B14) << 14) | \
         (RCHIFT_BIT(val,B13) << 13) | \
         (RCHIFT_BIT(val,B12) << 12) | \
         (RCHIFT_BIT(val,B11) << 11) | \
         (RCHIFT_BIT(val,B10) << 10) | \
         (RCHIFT_BIT(val, B9) <<  9) | \
         (RCHIFT_BIT(val, B8) <<  8) | \
         (RCHIFT_BIT(val, B7) <<  7) | \
         (RCHIFT_BIT(val, B6) <<  6) | \
         (RCHIFT_BIT(val, B5) <<  5) | \
         (RCHIFT_BIT(val, B4) <<  4) | \
         (RCHIFT_BIT(val, B3) <<  3) | \
         (RCHIFT_BIT(val, B2) <<  2) | \
         (RCHIFT_BIT(val, B1) <<  1) | \
         (RCHIFT_BIT(val, B0) <<  0))

#define BITSWAP24(val,B23,B22,B21,B20,B19,B18,B17,B16,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
        ((RCHIFT_BIT(val,B23) << 23) | \
         (RCHIFT_BIT(val,B22) << 22) | \
         (RCHIFT_BIT(val,B21) << 21) | \
         (RCHIFT_BIT(val,B20) << 20) | \
         (RCHIFT_BIT(val,B19) << 19) | \
         (RCHIFT_BIT(val,B18) << 18) | \
         (RCHIFT_BIT(val,B17) << 17) | \
         (RCHIFT_BIT(val,B16) << 16) | \
         (RCHIFT_BIT(val,B15) << 15) | \
         (RCHIFT_BIT(val,B14) << 14) | \
         (RCHIFT_BIT(val,B13) << 13) | \
         (RCHIFT_BIT(val,B12) << 12) | \
         (RCHIFT_BIT(val,B11) << 11) | \
         (RCHIFT_BIT(val,B10) << 10) | \
         (RCHIFT_BIT(val, B9) <<  9) | \
         (RCHIFT_BIT(val, B8) <<  8) | \
         (RCHIFT_BIT(val, B7) <<  7) | \
         (RCHIFT_BIT(val, B6) <<  6) | \
         (RCHIFT_BIT(val, B5) <<  5) | \
         (RCHIFT_BIT(val, B4) <<  4) | \
         (RCHIFT_BIT(val, B3) <<  3) | \
         (RCHIFT_BIT(val, B2) <<  2) | \
         (RCHIFT_BIT(val, B1) <<  1) | \
         (RCHIFT_BIT(val, B0) <<  0))

#define BITSWAP32(val,B31,B30,B29,B28,B27,B26,B25,B24,B23,B22,B21,B20,B19,B18,B17,B16,B15,B14,B13,B12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
        ((RCHIFT_BIT(val,B31) << 31) | \
         (RCHIFT_BIT(val,B30) << 30) | \
         (RCHIFT_BIT(val,B29) << 29) | \
         (RCHIFT_BIT(val,B28) << 28) | \
         (RCHIFT_BIT(val,B27) << 27) | \
         (RCHIFT_BIT(val,B26) << 26) | \
         (RCHIFT_BIT(val,B25) << 25) | \
         (RCHIFT_BIT(val,B24) << 24) | \
         (RCHIFT_BIT(val,B23) << 23) | \
         (RCHIFT_BIT(val,B22) << 22) | \
         (RCHIFT_BIT(val,B21) << 21) | \
         (RCHIFT_BIT(val,B20) << 20) | \
         (RCHIFT_BIT(val,B19) << 19) | \
         (RCHIFT_BIT(val,B18) << 18) | \
         (RCHIFT_BIT(val,B17) << 17) | \
         (RCHIFT_BIT(val,B16) << 16) | \
         (RCHIFT_BIT(val,B15) << 15) | \
         (RCHIFT_BIT(val,B14) << 14) | \
         (RCHIFT_BIT(val,B13) << 13) | \
         (RCHIFT_BIT(val,B12) << 12) | \
         (RCHIFT_BIT(val,B11) << 11) | \
         (RCHIFT_BIT(val,B10) << 10) | \
         (RCHIFT_BIT(val, B9) <<  9) | \
         (RCHIFT_BIT(val, B8) <<  8) | \
         (RCHIFT_BIT(val, B7) <<  7) | \
         (RCHIFT_BIT(val, B6) <<  6) | \
         (RCHIFT_BIT(val, B5) <<  5) | \
         (RCHIFT_BIT(val, B4) <<  4) | \
         (RCHIFT_BIT(val, B3) <<  3) | \
         (RCHIFT_BIT(val, B2) <<  2) | \
         (RCHIFT_BIT(val, B1) <<  1) | \
         (RCHIFT_BIT(val, B0) <<  0))

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
