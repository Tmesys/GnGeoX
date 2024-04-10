/*!
*
*   \file    GnGeoXscanline.h
*   \brief   Scanline image effect routines header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    11/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_SCANLINE_H_
#define _GNGEOX_SCANLINE_H_

extern Sint32 current_line;

SDL_bool effect_scanline_init ( void ) __attribute__ ( ( warn_unused_result ) );
void effect_scanline_update ( void );
void effect_scanline50_update ( void );
void effect_doublex_update ( void );
Sint32 update_scanline ( void )  __attribute__ ( ( warn_unused_result ) );;

#endif // _GNGEOX_SCANLINE_H_
