/*!
*
*   \file    GnGeoXframeskip.h
*   \brief   Frame skipping routines header.
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

#ifndef _GNGEOX_FRAMECAP_H_
#define _GNGEOX_FRAMECAP_H_

#define TICKS_PER_SEC 1000000UL
#define MAX_FRAMESKIP 10

#ifdef _GNGEOX_FRAMESKIP_C_
static void neo_frame_rate_callback ( void );
#endif // _GNGEOX_FRAMESKIP_C_

void neo_frame_rate_display ( void );
void neo_frame_cap_init ( void );
void neo_frame_cap_start ( void );
void neo_frame_cap_stop ( void );
void neo_frame_cap_close ( void );

#endif // _GNGEOX_FRAMECAP_H_
