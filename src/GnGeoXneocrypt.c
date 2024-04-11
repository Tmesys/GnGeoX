/*!
*
*   \file    GnGeoXneocrypt.c
*   \brief   Encrypting general routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.40 (final beta)
*   \date    17/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    Many of the NeoGeo bootlegs use their own form of encryption and
*            protection, presumably to make them harder for other bootleggers to
*            copy.  This encryption often involves non-trivial scrambling of the
*            program roms and the games are protected using an ALTERA chip which
*            provides some kind of rom overlay, patching parts of the code.
*            The graphics roms are usually scrambled in a different way to the
*            official SNK cartridges too.
*
*            1 - NeoGeo 'C' (Graphics) Rom encryption :
*
*            CMC42 and CMC50 protection chips. Also contains 'S' (Text Layer) data on these games
*            M1 (Z80) rom is also encrypted for CMC50.
*
*            The M1 roms contain some additional data at 0xf800.  Some of this
*            is said to be related to the C rom encryption.
*            If CMC50 is used, data located at 0xff00 - 0xffff is required for
*            m1 encryption checksum?.
*
*            Later games use additional basic scrambling on top of the standard
*            CMC scramble.
*
*             Starting with KOF99, all NeoGeo games have encrypted graphics. Additionally
*             to that, the data for the front text layer, which was previously stored in
*             a separate ROM, is stored at the end of the tile data.
*
*             The encryption is one of the nastiest implementation of a XOR scheme ever
*             seen, involving 9 seemingly uncorrelated 256-byte tables. All known games use
*             the same tables except KOF2000 and MS4 which use a different set.
*
*             The 32 data bits of every longword are decrypted in a single step (one byte at
*             a time), but the values to use for the xor are determined in a convoluted way.
*             It's actually so convoluted that it's too difficult to describe - please refer
*             to the source below.
*             Suffice to say that bytes are handled in couples (0&3 and 1&2), and the two xor
*             values are taken from three tables, the indexes inside the tables depending on
*             bits 0-7 and 8-15 of the address, in one case further xored through the table
*             used in step 5) below. Additionally, the bytes in a couple can be swapped,
*             depending either on bit 8 of the address, or on bit 16 xored with the table
*             used in step 4) below.
*
*             The 24 address bits are encrypted in five steps. Each step xors 8 bits with a
*             value taken from a different table; the index inside the table depends on 8
*             other bits.
*             0) xor bits  0-7  with a fixed value that changes from game to game
*             1) xor bits  8-15 depending on bits 16-23
*             2) xor bits  8-15 depending on bits  0-7
*             3) xor bits 16-23 depending on bits  0-7
*             4) xor bits 16-23 depending on bits  8-15
*             5) xor bits  0-7  depending on bits  8-15
*
*             Each step acts on the current value, so e.g. step 4) uses bits 8-15 as modified
*             by step 2).
*
*             [Note: the table used in step 1) is currently incomplete due to lack of data to
*             analyze]
*
*             There are two major weaknesses in this encryption algorithm, that exposed it to
*             a known plain text attack.
*
*             The first weakness is that the data xor depends on the address inside the
*             encrypted ROM instead that on the decrypted address; together with the high
*             concentration of 0x00 and 0xFF in the decrypted data (more than 60% of the
*             total), this exposed easily recognizable patterns in the encrypted data, which
*             could be exploited with some simple statistical checks. The deviousness of the
*             xor scheme was the major difficulty.
*
*             The second weakness is that the address scrambling works on 32-bit words. Since
*             there are a large number of 32-bit values that appear only once in the whole
*             encrypted ROM space, this means that once the xor layer was broken, a large
*             table of encrypted-decrypted address correspondences could be built and
*             analyzed, quickly leading to the algorithm.
*
*            2 - NeoGeo 'P' Rom Encryption :
*
*            Used on various games
*
*            kof98 :
*            - unique early encryption
*            kof99, garou, garouo, mslug3, kof2000 :
*            - complex SMA chip which appears to contain part of the game rom
*              internally and decrypts the 68k code on the board. Also has a
*              random number generator and  custom bankswitching (see machine/neoprot.c)
*            kof2002, matrim, samsho5, samsh5p
*            - some basic block / bank swapping
*            svc, kof2003, mslug5 :
*            - different scrambling with additional xor
*
*            3 - NeoGeo 'V' Rom encryption :
*
*            NEO-PCM2 chip used on various games :
*            type1 used on pnyaa, rotd, mslug4
*            type2 used on kof2002, matrim, mslug5, svc, samsho5, samsh5s, kof2003
*
*/
#ifndef _GNGEOX_NEOCRYPT_C_
#define _GNGEOX_NEOCRYPT_C_
#endif // _GNGEOX_NEOCRYPT_C_

#include <SDL2/SDL.h>
#include "qlibc.h"

#include "GnGeoXroms.h"
#include "GnGeoXmamelayer.h"
#include "GnGeoXneocrypt.h"
#include "GnGeoXneocryptdata.h"

static Uint8* type0_t03 = 0;
static Uint8* type0_t12 = 0;
static Uint8* type1_t03 = 0;
static Uint8* type1_t12 = 0;
static Uint8* address_8_15_xor1 = 0;
static Uint8* address_8_15_xor2 = 0;
static Uint8* address_16_23_xor1 = 0;
static Uint8* address_16_23_xor2 = 0;
static Uint8* address_0_7_xor = 0;
/*
static Uint8* m1_address_8_15_xor = 0;
static Uint8* m1_address_0_7_xor = 0;
*/
/* ******************************************************************************************************************/
/*!
* \brief  Decrypts.
*
* \param r0 Todo.
* \param r1 Todo.
* \param c0 Todo.
* \param c1 Todo.
* \param table0hi Todo.
* \param table0lo Todo.
* \param table1 Todo.
* \param base Todo.
* \param invert Todo.
*
*/
/* ******************************************************************************************************************/
static void decrypt ( Uint8* r0, Uint8* r1, Uint8 c0,  Uint8 c1, const Uint8* table0hi, const Uint8* table0lo,
                      const Uint8* table1, Sint32 base, Sint32 invert )
{
    Uint8 tmp = 0, xor0 = 0, xor1 = 0;

    tmp = table1[ ( base & 0xff ) ^ address_0_7_xor[ ( base >> 8 ) & 0xff]];
    xor0 = ( table0hi[ ( base >> 8 ) & 0xff] & 0xfe ) | ( tmp & 0x01 );
    xor1 = ( tmp & 0xfe ) | ( table0lo[ ( base >> 8 ) & 0xff] & 0x01 );

    if ( invert )
    {
        *r0 = c1 ^ xor0;
        *r1 = c0 ^ xor1;
    }
    else
    {
        *r0 = c0 ^ xor0;
        *r1 = c1 ^ xor1;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts game gfx.
*
* \param machine Todo.
* \param extra_xor Todo.
*
*/
/* ******************************************************************************************************************/
static void neogeo_gfx_decrypt ( struct_gngeoxroms_game_roms* machine, Sint32 extra_xor )
{
    Sint32 rom_size = 0;
    Uint8* buf = NULL;
    Uint8* rom = NULL;
    Sint32 cnt = 0;

    rom_size = memory_region_length ( machine, GNGEO_MEMORYREGION_SPRITES );
    buf = alloc_array_or_die ( Uint8, rom_size );
    rom = memory_region ( machine, GNGEO_MEMORYREGION_SPRITES );

    // Data xor
    cnt = 0;

    for ( Sint32 rpos = 0; rpos < ( rom_size / 4 ); rpos++ )
    {
        decrypt ( buf + 4 * rpos + 0, buf + 4 * rpos + 3, rom[4 * rpos + 0], rom[4 * rpos + 3], type0_t03, type0_t12, type1_t03, rpos, ( rpos >> 8 ) & 1 );
        decrypt ( buf + 4 * rpos + 1, buf + 4 * rpos + 2, rom[4 * rpos + 1], rom[4 * rpos + 2], type0_t12, type0_t03, type1_t12, rpos, ( ( rpos >> 16 ) ^ address_16_23_xor2[ ( rpos >> 8 ) & 0xff] ) & 1 );
        /*
                if ( cnt++ > 32768 )
                {
                    cnt = 0;
                }
        */
    }

    cnt = 0;

    // Address xor
    for ( Sint32 rpos = 0; rpos < rom_size / 4; rpos++ )
    {
        Sint32 baser;
        /*
                if ( cnt++ > 32768 )
                {
                    cnt++;
                }
        */
        baser = rpos;
        baser ^= extra_xor;
        baser ^= address_8_15_xor1[ ( baser >> 16 ) & 0xff] << 8;
        baser ^= address_8_15_xor2[baser & 0xff] << 8;
        baser ^= address_16_23_xor1[baser & 0xff] << 16;
        baser ^= address_16_23_xor2[ ( baser >> 8 ) & 0xff] << 16;
        baser ^= address_0_7_xor[ ( baser >> 8 ) & 0xff];

        if ( rom_size == 0x3000000 ) /* special handling for preisle2 */
        {
            if ( rpos < 0x2000000 / 4 )
            {
                baser &= ( 0x2000000 / 4 ) - 1;
            }
            else
            {
                baser = 0x2000000 / 4 + ( baser & ( ( 0x1000000 / 4 ) - 1 ) );
            }
        }
        else
        {
            if ( rom_size == 0x6000000 )    /* special handling for kf2k3pcb */
            {
                if ( rpos < 0x4000000 / 4 )
                {
                    baser &= ( 0x4000000 / 4 ) - 1;
                }
                else
                {
                    baser = 0x4000000 / 4 + ( baser & ( ( 0x1000000 / 4 ) - 1 ) );
                }
            }

            else /* Clamp to the real rom size */
            {
                baser &= ( rom_size / 4 ) - 1;
            }
        }

        rom[4 * rpos + 0] = buf[4 * baser + 0];
        rom[4 * rpos + 1] = buf[4 * baser + 1];
        rom[4 * rpos + 2] = buf[4 * baser + 2];
        rom[4 * rpos + 3] = buf[4 * baser + 3];
    }

    qalloc_delete ( buf );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts sfix.
*
* \param machine Todo.
*
*/
/* ******************************************************************************************************************/
/* @note (Tmesys#1#12/17/2022): The S data comes from the end of the C data. */
static void neogeo_sfix_decrypt ( struct_gngeoxroms_game_roms* machine )
{
    Sint32 rom_size = memory_region_length ( machine, GNGEO_MEMORYREGION_SPRITES );
    Sint32 tx_size = memory_region_length ( machine, GNGEO_MEMORYREGION_FIXED );
    Uint8* src = memory_region ( machine, GNGEO_MEMORYREGION_SPRITES ) + rom_size - tx_size;
    Uint8* dst = memory_region ( machine, GNGEO_MEMORYREGION_FIXED );

    for ( Sint32 i = 0; i < tx_size; i++ )
    {
        dst[i] = src[ ( i & ~0x1f ) + ( ( i & 7 ) << 2 ) + ( ( ~i & 8 ) >> 2 ) + ( ( i & 0x10 ) >> 4 )];
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Loads CMC42 table.
*
*/
/* ******************************************************************************************************************/
static void load_cmc42_table ( void )
{
    type0_t03 =          kof99_type0_t03;
    type0_t12 =          kof99_type0_t12;
    type1_t03 =          kof99_type1_t03;
    type1_t12 =          kof99_type1_t12;
    address_8_15_xor1 =  kof99_address_8_15_xor1;
    address_8_15_xor2 =  kof99_address_8_15_xor2;
    address_16_23_xor1 = kof99_address_16_23_xor1;
    address_16_23_xor2 = kof99_address_16_23_xor2;
    address_0_7_xor =    kof99_address_0_7_xor;
}
/* ******************************************************************************************************************/
/*!
* \brief Loads CMC50 table.
*
*/
/* ******************************************************************************************************************/
static void load_cmc50_table ( void )
{
    type0_t03 =          kof2000_type0_t03;
    type0_t12 =          kof2000_type0_t12;
    type1_t03 =          kof2000_type1_t03;
    type1_t12 =          kof2000_type1_t12;
    address_8_15_xor1 =  kof2000_address_8_15_xor1;
    address_8_15_xor2 =  kof2000_address_8_15_xor2;
    address_16_23_xor1 = kof2000_address_16_23_xor1;
    address_16_23_xor2 = kof2000_address_16_23_xor2;
    address_0_7_xor =    kof2000_address_0_7_xor;
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts kof99 gfx.
*
* \param machine Todo.
* \param extra_xor Todo.
*
* \note CMC42 protection chip.
*/
/* ******************************************************************************************************************/
void kof99_neogeo_gfx_decrypt ( struct_gngeoxroms_game_roms* machine, Sint32 extra_xor )
{
    load_cmc42_table();
    neogeo_gfx_decrypt ( machine, extra_xor );
    neogeo_sfix_decrypt ( machine );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts kof2000 gfx.
*
* \param machine Todo.
* \param extra_xor Todo.
*
* \note CMC50 protection chip.
*/
/* ******************************************************************************************************************/
void kof2000_neogeo_gfx_decrypt ( struct_gngeoxroms_game_roms* machine, Sint32 extra_xor )
{
    load_cmc50_table();
    neogeo_gfx_decrypt ( machine, extra_xor );
    neogeo_sfix_decrypt ( machine );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts cmc42 gfx.
*
* \param machine Todo.
* \param extra_xor Todo.
*
* \note CMC42 protection chip.
*/
/* ******************************************************************************************************************/
void cmc42_neogeo_gfx_decrypt ( struct_gngeoxroms_game_roms* machine, Sint32 extra_xor )
{
    load_cmc42_table();
    neogeo_gfx_decrypt ( machine, extra_xor );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts cmc50 gfx.
*
* \param machine Todo.
* \param extra_xor Todo.
*
* \note CMC50 protection chip.
*/
/* ******************************************************************************************************************/
void cmc50_neogeo_gfx_decrypt ( struct_gngeoxroms_game_roms* machine, Sint32 extra_xor )
{
    load_cmc50_table();
    neogeo_gfx_decrypt ( machine, extra_xor );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts svcpcb gfx.
*
* \param machine Todo.
*
* \note ms5pcb and svcpcb have an additional scramble on top of the standard CMC scrambling.
*/
/* ******************************************************************************************************************/
void svcpcb_gfx_decrypt ( struct_gngeoxroms_game_roms* machine )
{
    static const Uint8 xorval[4] = { 0x34, 0x21, 0xc4, 0xe9 };
    Sint32 ofst = 0;
    Sint32 rom_size = memory_region_length ( machine, GNGEO_MEMORYREGION_SPRITES );
    Uint8* rom = memory_region ( machine, GNGEO_MEMORYREGION_SPRITES );
    Uint8* buf = alloc_array_or_die ( Uint8,  rom_size );

    for ( Sint32 i = 0; i < rom_size; i++ )
    {
        rom[ i ] ^= xorval[ ( i % 4 ) ];
    }

    for ( Sint32 i = 0; i < rom_size; i += 4 )
    {
        Uint32 rom32 = rom[i] | rom[i + 1] << 8 | rom[i + 2] << 16 | rom[i + 3] << 24;
        rom32 = BITSWAP32 ( rom32, 0x09, 0x0d, 0x13, 0x00, 0x17, 0x0f, 0x03, 0x05, 0x04, 0x0c, 0x11, 0x1e, 0x12, 0x15, 0x0b, 0x06, 0x1b, 0x0a, 0x1a, 0x1c, 0x14, 0x02, 0x0e, 0x1d, 0x18, 0x08, 0x01, 0x10, 0x19, 0x1f, 0x07, 0x16 );
        rom[i] = rom32 & 0xff;
        rom[i + 1] = ( rom32 >> 8 ) & 0xff;
        rom[i + 2] = ( rom32 >> 16 ) & 0xff;
        rom[i + 3] = ( rom32 >> 24 ) & 0xff;
    }

    memcpy ( buf, rom, rom_size );

    for ( Sint32 i = 0; i < rom_size / 4; i++ )
    {
        ofst =  BITSWAP24 ( ( i & 0x1fffff ), 0x17, 0x16, 0x15, 0x04, 0x0b, 0x0e, 0x08, 0x0c, 0x10, 0x00, 0x0a, 0x13, 0x03, 0x06, 0x02, 0x07, 0x0d, 0x01, 0x11, 0x09, 0x14, 0x0f, 0x12, 0x05 );
        ofst ^= 0x0c8923;
        ofst += ( i & 0xffe00000 );
        memcpy ( &rom[ i * 4 ], &buf[ ofst * 4 ], 0x04 );
    }

    qalloc_delete ( buf );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts svcpcb s1data.
*
* \param machine Todo.
*
* \note and a further swap on the s1 data.
*/
/* ******************************************************************************************************************/
void svcpcb_s1data_decrypt ( struct_gngeoxroms_game_roms* machine )
{
    Uint8* s1 = memory_region ( machine, GNGEO_MEMORYREGION_FIXED );
    size_t s1_size = memory_region_length ( machine, GNGEO_MEMORYREGION_FIXED );

    for ( Uint32 i = 0; i < s1_size; i++ ) // Decrypt S
    {
        s1[ i ] = BITSWAP8 ( s1[ i ] ^ 0xd2, 4, 0, 7, 2, 5, 1, 6, 3 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts kf2k3pcb gfx.
*
* \param machine Todo.
*
* \note kf2k3pcb has an additional scramble on top of the standard CMC scrambling (Thanks to Razoola & Halrin
*       for the info).
*/
/* ******************************************************************************************************************/
/* @fixme (Tmesys#1#12/17/2022): Not used ? */
void kf2k3pcb_gfx_decrypt ( struct_gngeoxroms_game_roms* machine )
{
    static const Uint8 xorval[ 4 ] = { 0x34, 0x21, 0xc4, 0xe9 };
    Sint32 ofst = 0;
    Sint32 rom_size = memory_region_length ( machine, GNGEO_MEMORYREGION_SPRITES );
    Uint8* rom = memory_region ( machine, GNGEO_MEMORYREGION_SPRITES );
    Uint8* buf = alloc_array_or_die ( Uint8,  rom_size );

    for ( Sint32 i = 0; i < rom_size; i++ )
    {
        rom[ i ] ^= xorval[ ( i % 4 ) ];
    }

    for ( Sint32 i = 0; i < rom_size; i += 4 )
    {
        Uint32* rom32 = ( Uint32* ) &rom[ i ];
        *rom32 = BITSWAP32 ( *rom32, 0x09, 0x0d, 0x13, 0x00, 0x17, 0x0f, 0x03, 0x05, 0x04, 0x0c, 0x11, 0x1e, 0x12, 0x15, 0x0b, 0x06, 0x1b, 0x0a, 0x1a, 0x1c, 0x14, 0x02, 0x0e, 0x1d, 0x18, 0x08, 0x01, 0x10, 0x19, 0x1f, 0x07, 0x16 );
    }

    memcpy ( buf, rom, rom_size );

    for ( Sint32 i = 0; i < rom_size; i += 4 )
    {
        ofst = BITSWAP24 ( ( i & 0x7fffff ), 0x17, 0x15, 0x0a, 0x14, 0x13, 0x16, 0x12, 0x11, 0x10, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 );
        ofst ^= 0x000000;
        ofst += ( i & 0xff800000 );
        memcpy ( &rom[ ofst ], &buf[ i ], 0x04 );
    }

    qalloc_delete ( buf );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts kf2k3pcb gfx.
*
* \param machine Todo.
*
* \note and a further swap on the s1 data.
*/
/* ******************************************************************************************************************/
/* @fixme (Tmesys#1#12/17/2022): Not used ? */
void kf2k3pcb_decrypt_s1data ( struct_gngeoxroms_game_roms* machine )
{
    Uint8* src = NULL;
    Uint8* dst = NULL;
    Sint32 tx_size = memory_region_length ( machine, GNGEO_MEMORYREGION_FIXED );
    Sint32 srom_size = memory_region_length ( machine, GNGEO_MEMORYREGION_SPRITES );

    src = memory_region ( machine, GNGEO_MEMORYREGION_SPRITES ) + srom_size - 0x1000000 - 0x80000; // Decrypt S
    dst = memory_region ( machine, GNGEO_MEMORYREGION_FIXED );

    for ( Sint32 i = 0; i < tx_size / 2; i++ )
    {
        dst[ i ] = src[ ( i & ~0x1f ) + ( ( i & 7 ) << 2 ) + ( ( ~i & 8 ) >> 2 ) + ( ( i & 0x10 ) >> 4 ) ];
    }

    src = memory_region ( machine, GNGEO_MEMORYREGION_SPRITES ) + srom_size - 0x80000;
    dst = memory_region ( machine, GNGEO_MEMORYREGION_FIXED ) + 0x80000;

    for ( Sint32 i = 0; i < tx_size / 2; i++ )
    {
        dst[ i ] = src[ ( i & ~0x1f ) + ( ( i & 7 ) << 2 ) + ( ( ~i & 8 ) >> 2 ) + ( ( i & 0x10 ) >> 4 ) ];
    }

    dst = memory_region ( machine, GNGEO_MEMORYREGION_FIXED );

    for ( Sint32 i = 0; i < tx_size; i++ )
    {
        dst[ i ] = BITSWAP8 ( dst[ i ] ^ 0xd2, 4, 0, 7, 2, 5, 1, 6, 3 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Generates cs16.
*
* \param rom Todo.
* \param size Todo.
*
* \note The CMC50 hardware does a checksum of the first 64kb of the M1 rom, and uses this checksum as the basis of
*       the key with which to decrypt the rom.
*/
/* ******************************************************************************************************************/
static Uint16 generate_cs16 ( Uint8* rom, Sint32 size )
{
    Uint16 cs16 = 0;
    //cs16 = 0x0000;

    for ( Sint32 i = 0; i < size; i++ )
    {
        cs16 += rom[i];
    }

    return ( cs16 & 0xFFFF );
}
/* ******************************************************************************************************************/
/*!
* \brief Scrambles M1 address.
*
* \param address Todo.
* \param key Todo.
*
*/
/* ******************************************************************************************************************/
static Sint32 m1_address_scramble ( Sint32 address, Uint16 key )
{
    Sint32 block = 0;
    Sint32 aux = 0;

    const Sint32 p1[8][16] =
    {
        {15, 14, 10, 7, 1, 2, 3, 8, 0, 12, 11, 13, 6, 9, 5, 4},
        {7, 1, 8, 11, 15, 9, 2, 3, 5, 13, 4, 14, 10, 0, 6, 12},
        {8, 6, 14, 3, 10, 7, 15, 1, 4, 0, 2, 5, 13, 11, 12, 9},
        {2, 8, 15, 9, 3, 4, 11, 7, 13, 6, 0, 10, 1, 12, 14, 5},
        {1, 13, 6, 15, 14, 3, 8, 10, 9, 4, 7, 12, 5, 2, 0, 11},
        {11, 15, 3, 4, 7, 0, 9, 2, 6, 14, 12, 1, 8, 5, 10, 13},
        {10, 5, 13, 8, 6, 15, 1, 14, 11, 9, 3, 0, 12, 7, 4, 2},
        {9, 3, 7, 0, 2, 12, 4, 11, 14, 10, 5, 8, 15, 13, 1, 6},
    };

    block = ( address >> 16 ) & 7;
    aux = address & 0xffff;

    aux ^= BITSWAP16 ( key, 12, 0, 2, 4, 8, 15, 7, 13, 10, 1, 3, 6, 11, 9, 14, 5 );
    aux = BITSWAP16 ( aux,
                      p1[block][15], p1[block][14], p1[block][13], p1[block][12],
                      p1[block][11], p1[block][10], p1[block][9], p1[block][8],
                      p1[block][7], p1[block][6], p1[block][5], p1[block][4],
                      p1[block][3], p1[block][2], p1[block][1], p1[block][0] );
    aux ^= m1_address_0_7_xor[ ( aux >> 8 ) & 0xff];
    aux ^= m1_address_8_15_xor[aux & 0xff] << 8;
    aux = BITSWAP16 ( aux, 7, 15, 14, 6, 5, 13, 12, 4, 11, 3, 10, 2, 9, 1, 8, 0 );

    return ( ( block << 16 ) | aux );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts CMC50 M1.
*
* \param machine Todo.
*
*/
/* ******************************************************************************************************************/
void neogeo_cmc50_m1_decrypt ( struct_gngeoxroms_game_roms* machine )
{
    Uint8* rom = memory_region ( machine, GNGEO_MEMORYREGION_AUDIOCRYPT );
    size_t rom_size = 0x80000;
    //size_t rom_size = memory_region_length(machine, GNGEO_MEMORYREGION_AUDIOCRYPT);;
    Uint8* rom2 = memory_region ( machine, GNGEO_MEMORYREGION_AUDIOCPU );
    Uint8* buffer = alloc_array_or_die ( Uint8, rom_size );
    Uint16 key = generate_cs16 ( rom, 0x10000 );

    /* @todo (Tmesys#1#20/10/2023): don't open it 2 times */
    load_cmc50_table();

    for ( Uint32 i = 0; i < rom_size; i++ )
    {
        buffer[i] = rom[m1_address_scramble ( i, key )];
    }

    memcpy ( rom, buffer, rom_size );
    memcpy ( rom2, rom, 0x10000 );
    memcpy ( rom2 + 0x10000, rom, 0x80000 );

    qalloc_delete ( buffer );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts kof98 68k.
*
* \param machine Todo.
* \note Kof98 uses an early encryption, quite different from the others.
*
*/
/* ******************************************************************************************************************/
void kof98_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    Uint8* src = memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );
    Uint8* dst = alloc_array_or_die ( Uint8, 0x200000 );
    static const Uint32 sec[] = {0x000000, 0x100000, 0x000004, 0x100004, 0x10000a, 0x00000a, 0x10000e, 0x00000e};
    static const Uint32 pos[] = {0x000, 0x004, 0x00a, 0x00e};

    memcpy ( dst, src, 0x200000 );

    for ( Sint32 i = 0x800; i < 0x100000; i += 0x200 )
    {
        for ( Sint32 j = 0; j < 0x100; j += 0x10 )
        {
            for ( Sint32 k = 0; k < 16; k += 2 )
            {
                memcpy ( &src[i + j + k],       &dst[ i + j + sec[k / 2] + 0x100 ], 2 );
                memcpy ( &src[i + j + k + 0x100], &dst[ i + j + sec[k / 2] ],       2 );
            }

            if ( i >= 0x080000 && i < 0x0c0000 )
            {
                for ( Sint32 k = 0; k < 4; k++ )
                {
                    memcpy ( &src[i + j + pos[k]],       &dst[i + j + pos[k]],       2 );
                    memcpy ( &src[i + j + pos[k] + 0x100], &dst[i + j + pos[k] + 0x100], 2 );
                }
            }
            else
            {
                if ( i >= 0x0c0000 )
                {
                    for ( Sint32 k = 0; k < 4; k++ )
                    {
                        memcpy ( &src[i + j + pos[k]],       &dst[i + j + pos[k] + 0x100], 2 );
                        memcpy ( &src[i + j + pos[k] + 0x100], &dst[i + j + pos[k]],       2 );
                    }
                }
            }
        }

        memcpy ( &src[i + 0x000000], &dst[i + 0x000000], 2 );
        memcpy ( &src[i + 0x000002], &dst[i + 0x100000], 2 );
        memcpy ( &src[i + 0x000100], &dst[i + 0x000100], 2 );
        memcpy ( &src[i + 0x000102], &dst[i + 0x100100], 2 );
    }

    memmove ( &src[0x100000], &src[0x200000], 0x400000 );

    qalloc_delete ( dst );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts kof99 68k.
*
* \param machine Todo.
* \note kof99, garou, garouo, mslug3 and kof2000 have and SMA chip which contains program code and decrypts the 68k roms.
*
*/
/* ******************************************************************************************************************/
void kof99_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    Uint16* rom = NULL;

    rom = ( Uint16* ) ( memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU ) + 0x100000 );

    /* swap data lines on the whole ROMs */
    for ( Sint32 i = 0; i < 0x800000 / 2; i++ )
    {
        rom[i] = BITSWAP16 ( rom[i], 13, 7, 3, 0, 9, 4, 5, 6, 1, 12, 8, 14, 10, 11, 2, 15 );
    }

    /* swap address lines for the banked part */
    for ( Sint32 i = 0; i < 0x600000 / 2; i += 0x800 / 2 )
    {
        Uint16 buffer[0x800 / 2];
        memcpy ( buffer, &rom[i], 0x800 );

        for ( Sint32 j = 0; j < 0x800 / 2; j++ )
        {
            rom[i + j] = buffer[BITSWAP24 ( j, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 6, 2, 4, 9, 8, 3, 1, 7, 0, 5 )];
        }
    }

    /* swap address lines & relocate fixed part */
    rom = ( Uint16* ) memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );

    for ( Sint32 i = 0; i < 0x0c0000 / 2; i++ )
    {
        rom[i] = rom[0x700000 / 2 + BITSWAP24 ( i, 23, 22, 21, 20, 19, 18, 11, 6, 14, 17, 16, 5, 8, 10, 12, 0, 4, 3, 2, 7, 9, 15, 13, 1 )];
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts garou 68k.
*
* \param machine Todo.
*
*/
/* ******************************************************************************************************************/
void garou_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    Uint16* rom = NULL;

    /* thanks to Razoola and Mr K for the info */
    rom = ( Uint16* ) ( memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU ) + 0x100000 );

    /* swap data lines on the whole ROMs */
    for ( Sint32 i = 0; i < 0x800000 / 2; i++ )
    {
        rom[i] = BITSWAP16 ( rom[i], 13, 12, 14, 10, 8, 2, 3, 1, 5, 9, 11, 4, 15, 0, 6, 7 );
    }

    /* swap address lines & relocate fixed part */
    rom = ( Uint16* ) memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );

    for ( Sint32 i = 0; i < 0x0c0000 / 2; i++ )
    {
        rom[i] = rom[0x710000 / 2 + BITSWAP24 ( i, 23, 22, 21, 20, 19, 18, 4, 5, 16, 14, 7, 9, 6, 13, 17, 15, 3, 1, 2, 12, 11, 8, 10, 0 )];
    }

    /* swap address lines for the banked part */
    rom = ( Uint16* ) ( memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU ) + 0x100000 );

    for ( Sint32 i = 0; i < 0x800000 / 2; i += 0x8000 / 2 )
    {
        Uint16 buffer[0x8000 / 2];
        memcpy ( buffer, &rom[i], 0x8000 );

        for ( Sint32 j = 0; j < 0x8000 / 2; j++ )
        {
            rom[i + j] = buffer[BITSWAP24 ( j, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 9, 4, 8, 3, 13, 6, 2, 7, 0, 12, 1, 11, 10, 5 )];
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts garouo 68k.
*
* \param machine Todo.
*
*/
/* ******************************************************************************************************************/
void garouo_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    Uint16* rom = NULL;

    /* thanks to Razoola and Mr K for the info */
    rom = ( Uint16* ) ( memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU ) + 0x100000 );

    /* swap data lines on the whole ROMs */
    for ( Sint32 i = 0; i < 0x800000 / 2; i++ )
    {
        rom[i] = BITSWAP16 ( rom[i], 14, 5, 1, 11, 7, 4, 10, 15, 3, 12, 8, 13, 0, 2, 9, 6 );
    }

    /* swap address lines & relocate fixed part */
    rom = ( Uint16* ) memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );

    for ( Sint32 i = 0; i < 0x0c0000 / 2; i++ )
    {
        rom[i] = rom[0x7f8000 / 2 + BITSWAP24 ( i, 23, 22, 21, 20, 19, 18, 5, 16, 11, 2, 6, 7, 17, 3, 12, 8, 14, 4, 0, 9, 1, 10, 15, 13 )];
    }

    /* swap address lines for the banked part */
    rom = ( Uint16* ) ( memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU ) + 0x100000 );

    for ( Sint32 i = 0; i < 0x800000 / 2; i += 0x8000 / 2 )
    {
        Uint16 buffer[0x8000 / 2];
        memcpy ( buffer, &rom[i], 0x8000 );

        for ( Sint32 j = 0; j < 0x8000 / 2; j++ )
        {
            rom[i + j] = buffer[BITSWAP24 ( j, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 12, 8, 1, 7, 11, 3, 13, 10, 6, 9, 5, 4, 0, 2 )];
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts mslug3 68k.
*
* \param machine Todo.
*
*/
/* ******************************************************************************************************************/
void mslug3_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    Uint16* rom = NULL;

    /* thanks to Razoola and Mr K for the info */
    rom = ( Uint16* ) ( memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU ) + 0x100000 );

    /* swap data lines on the whole ROMs */
    for ( Sint32 i = 0; i < 0x800000 / 2; i++ )
    {
        rom[i] = BITSWAP16 ( rom[i], 4, 11, 14, 3, 1, 13, 0, 7, 2, 8, 12, 15, 10, 9, 5, 6 );
    }

    /* swap address lines & relocate fixed part */
    rom = ( Uint16* ) memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );

    for ( Sint32 i = 0; i < 0x0c0000 / 2; i++ )
    {
        //rom[i] = rom[ ( 0x5d0000 / 2 ) + BITSWAP24 ( i, 23, 22, 21, 20, 19, 18, 15, 2, 1, 13, 3, 0, 9, 6, 16, 4, 11, 5, 7, 12, 17, 14, 10, 8 )];
        /* @fixme (Tmesys#1#05/04/2024): From MAME */
        rom[i] = rom[ ( 0x5d0000 / 2 ) + BITSWAP19 ( i, 18, 15, 2, 1, 13, 3, 0, 9, 6, 16, 4, 11, 5, 7, 12, 17, 14, 10, 8 )];
    }

    /* swap address lines for the banked part */
    rom = ( Uint16* ) ( memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU ) + 0x100000 );

    for ( Sint32 i = 0; i < 0x800000 / 2; i += 0x10000 / 2 )
    {
        Uint16 buffer[0x10000 / 2];
        memcpy ( buffer, &rom[i], 0x10000 );

        for ( Sint32 j = 0; j < 0x10000 / 2; j++ )
        {
            //rom[i + j] = buffer[BITSWAP24 ( j, 23, 22, 21, 20, 19, 18, 17, 16, 15, 2, 11, 0, 14, 6, 4, 13, 8, 9, 3, 10, 7, 5, 12, 1 )];
            /* @fixme (Tmesys#1#05/04/2024): From MAME */
            rom[i + j] = buffer[BITSWAP15 ( j, 2, 11, 0, 14, 6, 4, 13, 8, 9, 3, 10, 7, 5, 12, 1 )];
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts kof2000 68k.
*
* \param machine Todo.
*
*/
/* ******************************************************************************************************************/
void kof2000_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    Uint16* rom = NULL;

    /* thanks to Razoola and Mr K for the info */
    rom = ( Uint16* ) ( memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU ) + 0x100000 );

    /* swap data lines on the whole ROMs */
    for ( Sint32 i = 0; i < 0x800000 / 2; i++ )
    {
        rom[i] = BITSWAP16 ( rom[i], 12, 8, 11, 3, 15, 14, 7, 0, 10, 13, 6, 5, 9, 2, 1, 4 );
    }

    /* swap address lines for the banked part */
    for ( Sint32 i = 0; i < 0x63a000 / 2; i += 0x800 / 2 )
    {
        Uint16 buffer[0x800 / 2];
        memcpy ( buffer, &rom[i], 0x800 );

        for ( Sint32 j = 0; j < 0x800 / 2; j++ )
        {
            rom[i + j] = buffer[BITSWAP24 ( j, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 4, 1, 3, 8, 6, 2, 7, 0, 9, 5 )];
        }
    }

    /* swap address lines & relocate fixed part */
    rom = ( Uint16* ) memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );

    for ( Sint32 i = 0; i < 0x0c0000 / 2; i++ )
    {
        rom[i] = rom[0x73a000 / 2 + BITSWAP24 ( i, 23, 22, 21, 20, 19, 18, 8, 4, 15, 13, 3, 14, 16, 2, 6, 17, 7, 12, 10, 0, 5, 11, 1, 9 )];
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts kof2000 68k.
*
* \param machine Todo.
* \note kof2002, matrim, samsho5, samsh5sp have some simple block swapping
*/
/* ******************************************************************************************************************/
void kof2002_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    static const Sint32 sec[] = {0x100000, 0x280000, 0x300000, 0x180000, 0x000000, 0x380000, 0x200000, 0x080000};
    Uint8* src = memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU ) + 0x100000;
    Uint8* dst = alloc_array_or_die ( Uint8, 0x400000 );
    memcpy ( dst, src, 0x400000 );

    for ( Sint32 i = 0; i < 8; ++i )
    {
        memcpy ( src + i * 0x80000, dst + sec[i], 0x80000 );
    }

    qalloc_delete ( dst );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts matrim 68k.
*
* \param machine Todo.
*/
/* ******************************************************************************************************************/
void matrim_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    static const Sint32 sec[] = {0x100000, 0x280000, 0x300000, 0x180000, 0x000000, 0x380000, 0x200000, 0x080000};
    Uint8* src = memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU ) + 0x100000;
    Uint8* dst = alloc_array_or_die ( Uint8, 0x400000 );
    memcpy ( dst, src, 0x400000 );

    for ( Sint32 i = 0; i < 8; ++i )
    {
        memcpy ( src + i * 0x80000, dst + sec[i], 0x80000 );
    }

    qalloc_delete ( dst );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts samsho5 68k.
*
* \param machine Todo.
*/
/* ******************************************************************************************************************/
void samsho5_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    static const Sint32 sec[] = {0x000000, 0x080000, 0x700000, 0x680000, 0x500000, 0x180000, 0x200000, 0x480000, 0x300000, 0x780000, 0x600000, 0x280000, 0x100000, 0x580000, 0x400000, 0x380000};
    Uint8* src = memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );
    Uint8* dst = alloc_array_or_die ( Uint8, 0x800000 );

    memcpy ( dst, src, 0x800000 );

    for ( Sint32 i = 0; i < 16; ++i )
    {
        memcpy ( src + i * 0x80000, dst + sec[i], 0x80000 );
    }

    qalloc_delete ( dst );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts samsho5sp 68k.
*
* \param machine Todo.
*/
/* ******************************************************************************************************************/
/* @fixme (Tmesys#1#12/17/2022): Not used. */
void samsh5sp_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    static const Sint32 sec[] = {0x000000, 0x080000, 0x500000, 0x480000, 0x600000, 0x580000, 0x700000, 0x280000, 0x100000, 0x680000, 0x400000, 0x780000, 0x200000, 0x380000, 0x300000, 0x180000};
    Uint8* src = memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );
    Uint8* dst = alloc_array_or_die ( Uint8, 0x800000 );

    memcpy ( dst, src, 0x800000 );

    for ( Sint32 i = 0; i < 16; ++i )
    {
        memcpy ( src + i * 0x80000, dst + sec[i], 0x80000 );
    }

    qalloc_delete ( dst );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts mslug5 68k.
*
* \param machine Todo.
* \note kf2k3pcb, kof2003, kof2003h, mslug5 and svc have updated P rom scramble.
*/
/* ******************************************************************************************************************/
void mslug5_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    static const Uint8 xor1[ 0x20 ] = { 0xc2, 0x4b, 0x74, 0xfd, 0x0b, 0x34, 0xeb, 0xd7, 0x10, 0x6d, 0xf9, 0xce, 0x5d, 0xd5, 0x61, 0x29, 0xf5, 0xbe, 0x0d, 0x82, 0x72, 0x45, 0x0f, 0x24, 0xb3, 0x34, 0x1b, 0x99, 0xea, 0x09, 0xf3, 0x03 };
    static const Uint8 xor2[ 0x20 ] = { 0x36, 0x09, 0xb0, 0x64, 0x95, 0x0f, 0x90, 0x42, 0x6e, 0x0f, 0x30, 0xf6, 0xe5, 0x08, 0x30, 0x64, 0x08, 0x04, 0x00, 0x2f, 0x72, 0x09, 0xa0, 0x13, 0xc9, 0x0b, 0xa0, 0x3e, 0xc2, 0x00, 0x40, 0x2b };
    Sint32 ofst = 0;
    Sint32 rom_size = 0x800000;
    Uint8* rom = memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );
    Uint8* buf = alloc_array_or_die ( Uint8,  rom_size );

    for ( Sint32 i = 0; i < 0x100000; i++ )
    {
        rom[ i ] ^= xor1[ ( BYTE_XOR_LE ( i ) % 0x20 ) ];
    }

    for ( Sint32 i = 0x100000; i < 0x800000; i++ )
    {
        rom[ i ] ^= xor2[ ( BYTE_XOR_LE ( i ) % 0x20 ) ];
    }

    for ( Sint32 i = 0x100000; i < 0x0800000; i += 4 )
    {
        Uint16 rom16 = 0;

        rom16 = rom[BYTE_XOR_LE ( i + 1 )] | rom[BYTE_XOR_LE ( i + 2 )] << 8;
        rom16 = BITSWAP16 ( rom16, 15, 14, 13, 12, 10, 11, 8, 9, 6, 7, 4, 5, 3, 2, 1, 0 );
        rom[BYTE_XOR_LE ( i + 1 )] = rom16 & 0xff;
        rom[BYTE_XOR_LE ( i + 2 )] = rom16 >> 8;
    }

    memcpy ( buf, rom, rom_size );

    for ( Sint32 i = 0; i < 0x0100000 / 0x10000; i++ )
    {
        ofst = ( i & 0xf0 ) + BITSWAP8 ( ( i & 0x0f ), 7, 6, 5, 4, 1, 0, 3, 2 );
        memcpy ( &rom[ i * 0x10000 ], &buf[ ofst * 0x10000 ], 0x10000 );
    }

    for ( Sint32 i = 0x100000; i < 0x800000; i += 0x100 )
    {
        ofst = ( i & 0xf000ff ) + ( ( i & 0x000f00 ) ^ 0x00700 ) + ( BITSWAP8 ( ( ( i & 0x0ff000 ) >> 12 ), 5, 4, 7, 6, 1, 0, 3, 2 ) << 12 );
        memcpy ( &rom[ i ], &buf[ ofst ], 0x100 );
    }

    memcpy ( buf, rom, rom_size );
    memcpy ( &rom[ 0x100000 ], &buf[ 0x700000 ], 0x100000 );
    memcpy ( &rom[ 0x200000 ], &buf[ 0x100000 ], 0x600000 );
    qalloc_delete ( buf );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts mslug5 68k.
*
* \param machine Todo.
* \note kf2k3pcb, kof2003, kof2003h, mslug5 and svc have updated P rom scramble.
*/
/* ******************************************************************************************************************/
/* @fixme (Tmesys#1#12/17/2022): Not used. */
void svc_px_decrypt ( struct_gngeoxroms_game_roms* machine )
{
    static const Uint8 xor1[ 0x20 ] = { 0x3b, 0x6a, 0xf7, 0xb7, 0xe8, 0xa9, 0x20, 0x99, 0x9f, 0x39, 0x34, 0x0c, 0xc3, 0x9a, 0xa5, 0xc8, 0xb8, 0x18, 0xce, 0x56, 0x94, 0x44, 0xe3, 0x7a, 0xf7, 0xdd, 0x42, 0xf0, 0x18, 0x60, 0x92, 0x9f };
    static const Uint8 xor2[ 0x20 ] = { 0x69, 0x0b, 0x60, 0xd6, 0x4f, 0x01, 0x40, 0x1a, 0x9f, 0x0b, 0xf0, 0x75, 0x58, 0x0e, 0x60, 0xb4, 0x14, 0x04, 0x20, 0xe4, 0xb9, 0x0d, 0x10, 0x89, 0xeb, 0x07, 0x30, 0x90, 0x50, 0x0e, 0x20, 0x26 };
    Sint32 ofst = 0;
    Sint32 rom_size = 0x800000;
    Uint8* rom = memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );
    Uint8* buf = alloc_array_or_die ( Uint8,  rom_size );

    for ( Sint32 i = 0; i < 0x100000; i++ )
    {
        rom[ i ] ^= xor1[ ( BYTE_XOR_LE ( i ) % 0x20 ) ];
    }

    for ( Sint32 i = 0x100000; i < 0x800000; i++ )
    {
        rom[ i ] ^= xor2[ ( BYTE_XOR_LE ( i ) % 0x20 ) ];
    }

    for ( Sint32 i = 0x100000; i < 0x0800000; i += 4 )
    {
        Uint16 rom16 = 0;

        rom16 = rom[BYTE_XOR_LE ( i + 1 )] | rom[BYTE_XOR_LE ( i + 2 )] << 8;
        rom16 = BITSWAP16 ( rom16, 15, 14, 13, 12, 10, 11, 8, 9, 6, 7, 4, 5, 3, 2, 1, 0 );
        rom[BYTE_XOR_LE ( i + 1 )] = rom16 & 0xff;
        rom[BYTE_XOR_LE ( i + 2 )] = rom16 >> 8;
    }

    memcpy ( buf, rom, rom_size );

    for ( Sint32 i = 0; i < 0x0100000 / 0x10000; i++ )
    {
        ofst = ( i & 0xf0 ) + BITSWAP8 ( ( i & 0x0f ), 7, 6, 5, 4, 2, 3, 0, 1 );
        memcpy ( &rom[ i * 0x10000 ], &buf[ ofst * 0x10000 ], 0x10000 );
    }

    for ( Sint32 i = 0x100000; i < 0x800000; i += 0x100 )
    {
        ofst = ( i & 0xf000ff ) + ( ( i & 0x000f00 ) ^ 0x00a00 ) + ( BITSWAP8 ( ( ( i & 0x0ff000 ) >> 12 ), 4, 5, 6, 7, 1, 0, 3, 2 ) << 12 );
        memcpy ( &rom[ i ], &buf[ ofst ], 0x100 );
    }

    memcpy ( buf, rom, rom_size );
    memcpy ( &rom[ 0x100000 ], &buf[ 0x700000 ], 0x100000 );
    memcpy ( &rom[ 0x200000 ], &buf[ 0x100000 ], 0x600000 );
    qalloc_delete ( buf );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts kf2k3pcb 68k.
*
* \param machine Todo.
*/
/* ******************************************************************************************************************/
void kf2k3pcb_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    static const Uint8 xor2[ 0x20 ] = { 0xb4, 0x0f, 0x40, 0x6c, 0x38, 0x07, 0xd0, 0x3f, 0x53, 0x08, 0x80, 0xaa, 0xbe, 0x07, 0xc0, 0xfa, 0xd0, 0x08, 0x10, 0xd2, 0xf1, 0x03, 0x70, 0x7e, 0x87, 0x0b, 0x40, 0xf6, 0x2a, 0x0a, 0xe0, 0xf9 };
    Sint32 ofst = 0;
    Sint32 rom_size = 0x900000;
    Uint8* rom = memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );
    Uint8* buf = alloc_array_or_die ( Uint8,  rom_size );

    for ( Sint32 i = 0; i < 0x100000; i++ )
    {
        rom[ 0x800000 + i ] ^= rom[ 0x100002 | BYTE_XOR_LE ( i ) ];
    }

    for ( Sint32 i = 0x100000; i < 0x800000; i++ )
    {
        rom[ i ] ^= xor2[ ( BYTE_XOR_LE ( i ) % 0x20 ) ];
    }

    for ( Sint32 i = 0x100000; i < 0x800000; i += 4 )
    {
        Uint16 rom16 = 0;

        rom16 = rom[BYTE_XOR_LE ( i + 1 )] | rom[BYTE_XOR_LE ( i + 2 )] << 8;
        rom16 = BITSWAP16 ( rom16, 15, 14, 13, 12, 4, 5, 6, 7, 8, 9, 10, 11, 3, 2, 1, 0 );
        rom[BYTE_XOR_LE ( i + 1 )] = rom16 & 0xff;
        rom[BYTE_XOR_LE ( i + 2 )] = rom16 >> 8;
    }

    for ( Sint32 i = 0; i < 0x0100000 / 0x10000; i++ )
    {
        ofst = ( i & 0xf0 ) + BITSWAP8 ( ( i & 0x0f ), 7, 6, 5, 4, 1, 0, 3, 2 );
        memcpy ( &buf[ i * 0x10000 ], &rom[ ofst * 0x10000 ], 0x10000 );
    }

    for ( Sint32 i = 0x100000; i < 0x900000; i += 0x100 )
    {
        ofst = ( i & 0xf000ff ) + ( ( i & 0x000f00 ) ^ 0x00300 ) + ( BITSWAP8 ( ( ( i & 0x0ff000 ) >> 12 ), 4, 5, 6, 7, 1, 0, 3, 2 ) << 12 );
        memcpy ( &buf[ i ], &rom[ ofst ], 0x100 );
    }

    memcpy ( &rom[0x000000], &buf[0x000000], 0x100000 );
    memcpy ( &rom[0x100000], &buf[0x800000], 0x100000 );
    memcpy ( &rom[0x200000], &buf[0x100000], 0x700000 );
    qalloc_delete ( buf );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts kof2003 68k.
*
* \param machine Todo.
*/
/* ******************************************************************************************************************/
void kof2003_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    static const Uint8 xor1[0x20] = { 0x3b, 0x6a, 0xf7, 0xb7, 0xe8, 0xa9, 0x20, 0x99, 0x9f, 0x39, 0x34, 0x0c, 0xc3, 0x9a, 0xa5, 0xc8, 0xb8, 0x18, 0xce, 0x56, 0x94, 0x44, 0xe3, 0x7a, 0xf7, 0xdd, 0x42, 0xf0, 0x18, 0x60, 0x92, 0x9f };
    static const Uint8 xor2[0x20] = { 0x2f, 0x02, 0x60, 0xbb, 0x77, 0x01, 0x30, 0x08, 0xd8, 0x01, 0xa0, 0xdf, 0x37, 0x0a, 0xf0, 0x65, 0x28, 0x03, 0xd0, 0x23, 0xd3, 0x03, 0x70, 0x42, 0xbb, 0x06, 0xf0, 0x28, 0xba, 0x0f, 0xf0, 0x7a };
    Sint32 ofst = 0;
    Sint32 rom_size = 0x900000;
    Uint8* rom = memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );
    Uint8* buf = alloc_array_or_die ( Uint8,  rom_size );

    for ( Sint32 i = 0; i < 0x100000; i++ )
    {
        rom[ 0x800000 + i ] ^= rom[ 0x100002 | BYTE_XOR_LE ( i ) ];
    }

    for ( Sint32 i = 0; i < 0x100000; i++ )
    {
        rom[ i ] ^= xor1[ ( BYTE_XOR_LE ( i ) % 0x20 ) ];
    }

    for ( Sint32 i = 0x100000; i < 0x800000; i++ )
    {
        rom[ i ] ^= xor2[ ( BYTE_XOR_LE ( i ) % 0x20 ) ];
    }

    for ( Sint32 i = 0x100000; i < 0x800000; i += 4 )
    {
        Uint16 rom16 = 0;

        rom16 = rom[BYTE_XOR_LE ( i + 1 )] | rom[BYTE_XOR_LE ( i + 2 )] << 8;
        rom16 = BITSWAP16 ( rom16, 15, 14, 13, 12, 5, 4, 7, 6, 9, 8, 11, 10, 3, 2, 1, 0 );
        rom[BYTE_XOR_LE ( i + 1 )] = rom16 & 0xff;
        rom[BYTE_XOR_LE ( i + 2 )] = rom16 >> 8;
    }

    for ( Sint32 i = 0; i < 0x0100000 / 0x10000; i++ )
    {
        ofst = ( i & 0xf0 ) + BITSWAP8 ( ( i & 0x0f ), 7, 6, 5, 4, 0, 1, 2, 3 );
        memcpy ( &buf[ i * 0x10000 ], &rom[ ofst * 0x10000 ], 0x10000 );
    }

    for ( Sint32 i = 0x100000; i < 0x900000; i += 0x100 )
    {
        ofst = ( i & 0xf000ff ) + ( ( i & 0x000f00 ) ^ 0x00800 ) + ( BITSWAP8 ( ( ( i & 0x0ff000 ) >> 12 ), 4, 5, 6, 7, 1, 0, 3, 2 ) << 12 );
        memcpy ( &buf[ i ], &rom[ ofst ], 0x100 );
    }

    memcpy ( &rom[0x000000], &buf[0x000000], 0x100000 );
    memcpy ( &rom[0x100000], &buf[0x800000], 0x100000 );
    memcpy ( &rom[0x200000], &buf[0x100000], 0x700000 );
    qalloc_delete ( buf );
}
/* ******************************************************************************************************************/
/*!
* \brief Decrypts kof2003h 68k.
*
* \param machine Todo.
*/
/* ******************************************************************************************************************/
/* @note (Tmesys#1#12/17/2022): Thanks to IQ_132 for the info. */
/* @fixme (Tmesys#1#12/17/2022): Not used. */
void kof2003h_decrypt_68k ( struct_gngeoxroms_game_roms* machine )
{
    static const Uint8 xor1[0x20] = { 0xc2, 0x4b, 0x74, 0xfd, 0x0b, 0x34, 0xeb, 0xd7, 0x10, 0x6d, 0xf9, 0xce, 0x5d, 0xd5, 0x61, 0x29, 0xf5, 0xbe, 0x0d, 0x82, 0x72, 0x45, 0x0f, 0x24, 0xb3, 0x34, 0x1b, 0x99, 0xea, 0x09, 0xf3, 0x03 };
    static const Uint8 xor2[0x20] = { 0x2b, 0x09, 0xd0, 0x7f, 0x51, 0x0b, 0x10, 0x4c, 0x5b, 0x07, 0x70, 0x9d, 0x3e, 0x0b, 0xb0, 0xb6, 0x54, 0x09, 0xe0, 0xcc, 0x3d, 0x0d, 0x80, 0x99, 0x87, 0x03, 0x90, 0x82, 0xfe, 0x04, 0x20, 0x18 };
    Sint32 ofst = 0;
    Sint32 rom_size = 0x900000;
    Uint8* rom = memory_region ( machine, GNGEO_MEMORYREGION_MAINCPU );
    Uint8* buf = alloc_array_or_die ( Uint8,  rom_size );

    for ( Sint32 i = 0; i < 0x100000; i++ )
    {
        rom[ 0x800000 + i ] ^= rom[ 0x100002 | BYTE_XOR_LE ( i ) ];
    }

    for ( Sint32 i = 0; i < 0x100000; i++ )
    {
        rom[ i ] ^= xor1[ ( BYTE_XOR_LE ( i ) % 0x20 ) ];
    }

    for ( Sint32 i = 0x100000; i < 0x800000; i++ )
    {
        rom[ i ] ^= xor2[ ( BYTE_XOR_LE ( i ) % 0x20 ) ];
    }

    for ( Sint32 i = 0x100000; i < 0x800000; i += 4 )
    {
        Uint16 rom16 = 0;

        rom16 = rom[BYTE_XOR_LE ( i + 1 )] | rom[BYTE_XOR_LE ( i + 2 )] << 8;
        rom16 = BITSWAP16 ( rom16, 15, 14, 13, 12, 10, 11, 8, 9, 6, 7, 4, 5, 3, 2, 1, 0 );
        rom[BYTE_XOR_LE ( i + 1 )] = rom16 & 0xff;
        rom[BYTE_XOR_LE ( i + 2 )] = rom16 >> 8;
    }

    for ( Sint32 i = 0; i < 0x0100000 / 0x10000; i++ )
    {
        ofst = ( i & 0xf0 ) + BITSWAP8 ( ( i & 0x0f ), 7, 6, 5, 4, 1, 0, 3, 2 );
        memcpy ( &buf[ i * 0x10000 ], &rom[ ofst * 0x10000 ], 0x10000 );
    }

    for ( Sint32 i = 0x100000; i < 0x900000; i += 0x100 )
    {
        ofst = ( i & 0xf000ff ) + ( ( i & 0x000f00 ) ^ 0x00400 ) + ( BITSWAP8 ( ( ( i & 0x0ff000 ) >> 12 ), 6, 7, 4, 5, 0, 1, 2, 3 ) << 12 );
        memcpy ( &buf[ i ], &rom[ ofst ], 0x100 );
    }

    memcpy ( &rom[0x000000], &buf[0x000000], 0x100000 );
    memcpy ( &rom[0x100000], &buf[0x800000], 0x100000 );
    memcpy ( &rom[0x200000], &buf[0x100000], 0x700000 );
    qalloc_delete ( buf );
}
/* ******************************************************************************************************************/
/*
NeoGeo 'V' (PCM) ROM encryption NEOPCM2 chip
*/
/* ******************************************************************************************************************/
/* ******************************************************************************************************************/
/*!
* \brief Neo-Pcm2 Drivers for Encrypted V Roms.
*
* \param machine Todo.
* \param value Todo.
*/
/* ******************************************************************************************************************/
/* @note (Tmesys#1#12/17/2022): thanks to Elsemi for the NEO-PCM2 info */
void neo_pcm2_snk_1999 ( struct_gngeoxroms_game_roms* machine, Sint32 value )
{
    Uint16* rom = ( Uint16* ) memory_region ( machine, GNGEO_MEMORYREGION_YM );
    Sint32 size = memory_region_length ( machine, GNGEO_MEMORYREGION_YM );

    if ( rom != NULL )
    {
        /* swap address lines on the whole ROMs */
        Uint16* buffer = alloc_array_or_die ( Uint16, value / 2 );

        for ( Sint32 i = 0; i < size / 2; i += ( value / 2 ) )
        {
            memcpy ( buffer, &rom[ i ], value );

            for ( Sint32 j = 0; j < ( value / 2 ); j++ )
            {
                rom[ i + j ] = buffer[ j ^ ( value / 4 ) ];
            }
        }

        qalloc_delete ( buffer );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Neo-Pcm2 Drivers for Encrypted V Roms.
*
* \param machine Todo.
* \param value Todo.
*
* \note the later PCM2 games have additional scrambling.
*/
/* ******************************************************************************************************************/
void neo_pcm2_swap ( struct_gngeoxroms_game_roms* machine, Sint32 value )
{
    static const Uint32 addrs[7][2] =
    {
        {0x000000, 0xa5000},
        {0xffce20, 0x01000},
        {0xfe2cf6, 0x4e001},
        {0xffac28, 0xc2000},
        {0xfeb2c0, 0x0a000},
        {0xff14ea, 0xa7001},
        {0xffb440, 0x02000}
    };
    static const Uint8 xordata[7][8] =
    {
        {0xf9, 0xe0, 0x5d, 0xf3, 0xea, 0x92, 0xbe, 0xef},
        {0xc4, 0x83, 0xa8, 0x5f, 0x21, 0x27, 0x64, 0xaf},
        {0xc3, 0xfd, 0x81, 0xac, 0x6d, 0xe7, 0xbf, 0x9e},
        {0xc3, 0xfd, 0x81, 0xac, 0x6d, 0xe7, 0xbf, 0x9e},
        {0xcb, 0x29, 0x7d, 0x43, 0xd2, 0x3a, 0xc2, 0xb4},
        {0x4b, 0xa4, 0x63, 0x46, 0xf0, 0x91, 0xea, 0x62},
        {0x4b, 0xa4, 0x63, 0x46, 0xf0, 0x91, 0xea, 0x62}
    };
    Uint8* src = memory_region ( machine, GNGEO_MEMORYREGION_YM );
    Uint8* buf = alloc_array_or_die ( Uint8, 0x1000000 );
    Sint32 j = 0, d = 0;

    memcpy ( buf, src, 0x1000000 );

    for ( Sint32 i = 0; i < 0x1000000; i++ )
    {
        j = BITSWAP24 ( i, 23, 22, 21, 20, 19, 18, 17, 0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 16 );
        j = j ^ addrs[value][1];
        d = ( ( i + addrs[value][0] ) & 0xffffff );
        src[j] = buf[d] ^ xordata[value][j & 0x7];
    }

    qalloc_delete ( buf );
}
/* ******************************************************************************************************************/
/*
NeoGeo 'SP1' (BIOS) ROM encryption
*/
/* ******************************************************************************************************************/
/* ******************************************************************************************************************/
/*!
* \brief kof2003 bios decode.
*
* \param machine Todo.
*
* \note only found on kf2k3pcb.
*/
/* ******************************************************************************************************************/
/* @note (Tmesys#1#12/17/2022): Not used. */
void kof2003biosdecode ( struct_gngeoxroms_game_roms* machine )
{
    static const Uint8 address[0x80] =
    {
        0xb9, 0xb8, 0x36, 0x37, 0x3d, 0x3c, 0xb2, 0xb3,
        0xb9, 0xb8, 0x36, 0x37, 0x3d, 0x3c, 0xb2, 0xb3,
        0x65, 0xea, 0x6f, 0xe0, 0xe1, 0x6e, 0xeb, 0x64,
        0x65, 0xea, 0x6f, 0xe0, 0xe1, 0x6e, 0xeb, 0x64,
        0x45, 0xca, 0x47, 0xc8, 0xc9, 0x46, 0xcb, 0x44,
        0x45, 0xca, 0x47, 0xc8, 0xc9, 0x46, 0xcb, 0x44,
        0x9a, 0x15, 0x98, 0x17, 0x1e, 0x91, 0x1c, 0x93,
        0x9a, 0x15, 0x98, 0x17, 0x1e, 0x91, 0x1c, 0x93,
        0x7e, 0xf1, 0x7c, 0xf3, 0xf0, 0x7f, 0xf2, 0x7d,
        0x7e, 0xf1, 0x7c, 0xf3, 0xf0, 0x7f, 0xf2, 0x7d,
        0x27, 0xa8, 0x25, 0xaa, 0xa3, 0x2c, 0xa1, 0x2e,
        0x27, 0xa8, 0x25, 0xaa, 0xa3, 0x2c, 0xa1, 0x2e,
        0x04, 0x8b, 0x06, 0x89, 0x80, 0x0f, 0x82, 0x0d,
        0x04, 0x8b, 0x06, 0x89, 0x80, 0x0f, 0x82, 0x0d,
        0xd3, 0xd2, 0x5c, 0x5d, 0x57, 0x56, 0xd8, 0xd9,
        0xd3, 0xd2, 0x5c, 0x5d, 0x57, 0x56, 0xd8, 0xd9,
    };
    Uint16* src = ( Uint16* ) memory_region ( machine, GNGEO_MEMORYREGION_MAINBIOS );
    Uint16* buf = alloc_array_or_die ( Uint16, 0x80000 / 2 );
    Sint32 addr = 0;

    for ( Sint32 a = 0; a < 0x80000 / 2; a++ )
    {
        if ( src[a] & ( 0x0004 << ( 8 * BYTE_XOR_LE ( 0 ) ) ) )
        {
            src[a] ^= 0x0001 << ( 8 * BYTE_XOR_LE ( 0 ) );
        }

        if ( src[a] & ( 0x0010 << ( 8 * BYTE_XOR_LE ( 0 ) ) ) )
        {
            src[a] ^= 0x0002 << ( 8 * BYTE_XOR_LE ( 0 ) );
        }

        if ( src[a] & ( 0x0020 << ( 8 * BYTE_XOR_LE ( 0 ) ) ) )
        {
            src[a] ^= 0x0008 << ( 8 * BYTE_XOR_LE ( 0 ) );
        }

        //address xor
        addr  = a & ~0xff;
        addr |= address[BYTE_XOR_LE ( a & 0x7f )];

        if ( a & 0x00008 )
        {
            addr ^= 0x0008;
        }

        if ( a & 0x00080 )
        {
            addr ^= 0x0080;
        }

        if ( a & 0x00200 )
        {
            addr ^= 0x0100;
        }

        if ( ~a & 0x02000 )
        {
            addr ^= 0x0400;
        }

        if ( ~a & 0x10000 )
        {
            addr ^= 0x1000;
        }

        if ( a & 0x02000 )
        {
            addr ^= 0x8000;
        }

        buf[addr] = src[a];
    }

    memcpy ( src, buf, 0x80000 );
    qalloc_delete ( buf );
}

#ifdef _GNGEOX_NEOCRYPT_C_
#undef _GNGEOX_NEOCRYPT_C_
#endif // _GNGEOX_NEOCRYPT_C_

