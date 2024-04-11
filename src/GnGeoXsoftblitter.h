/*!
*
*   \file    GnGeoXsoftblitter.h
*   \brief   Soft blitter routines header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    18/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_SOFTBLITTER_H_
#define _GNGEOX_SOFTBLITTER_H_

#define UPDATE_VISIBLE_AREA (visible_area.w>>0)

#ifdef _GNGEOX_SOFTBLITTER_C_
static void update_double ( void );
static void update_triple ( void );
#endif // _GNGEOX_SOFTBLITTER_C_

SDL_bool blitter_soft_init ( void ) __attribute__ ( ( warn_unused_result ) );
void blitter_soft_update ( void );
void blitter_soft_fullscreen ( void );
void blitter_soft_close ( void );

#endif // _GNGEOX_SOFTBLITTER_H_
