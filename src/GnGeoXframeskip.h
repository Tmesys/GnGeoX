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

#ifndef _GNGEOX_FRAMESKIP_H_
#define _GNGEOX_FRAMESKIP_H_

#define TICKS_PER_SEC 1000000UL
#define MAX_FRAMESKIP 10

extern SDL_bool skip_this_frame;

#ifdef _GNGEOX_FRAMESKIP_C_
static Uint32 get_ticks ( void ) __attribute__ ( ( warn_unused_result ) );
#endif // _GNGEOX_FRAMESKIP_C_

void neo_frame_skip_reset ( void );
void neo_frame_skip ( void );
void neo_frame_skip_display ( void );

#endif // _GNGEOX_FRAMESKIP_H_
