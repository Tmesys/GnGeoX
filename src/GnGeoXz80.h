/*!
*
*   \file    GnGeoXz80.h
*   \brief   Interface to the mamez80 emulator header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    03/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/

#ifndef _GNGEOX_Z80_H_
#define _GNGEOX_Z80_H_

typedef enum
{
    Z80_BANK_0_OFFSET = 0xF000,
    Z80_BANK_1_OFFSET = 0xE000,
    Z80_BANK_2_OFFSET = 0xC000,
    Z80_BANK_3_OFFSET = 0x8000,
} enum_gngeoxz80_bankoffset;

typedef enum
{
    Z80_BANK_0_WINDOW_SIZE = 0x0800,
    Z80_BANK_1_WINDOW_SIZE = 0x1000,
    Z80_BANK_2_WINDOW_SIZE = 0x2000,
    Z80_BANK_3_WINDOW_SIZE = 0x4000,
} enum_gngeoxz80_bankwindowsize;

#ifdef _GNGEOX_Z80_C_
static void cpu_z80_switchbank ( Uint8, Uint16 );
static Sint32 neo_z80_irq_callback ( Sint32 );
#endif // _GNGEOX_Z80_C_

void neo_z80_init ( void );
void neo_z80_nmi ( void );
void neo_z80_irq ( Sint32 );

#endif // _GNGEOX_Z80_H_
