/*!
*
*   \file    GnGeoXhq2x.h
*   \brief   HQ2X image effect routines header https://en.wikipedia.org//wiki/Hqx.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    03/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    This effect is a rewritten implementation of the hq2x effect made by Maxim Stepin.
*/

#ifndef _GNGEOX_HQ2X_H_
#define _GNGEOX_HQ2X_H_

#ifdef _GNGEOX_HQ2X_C_
static void hq2x_16_def ( Uint16*, Uint16*, Uint16*, Uint16*, Uint16*, Uint32 );
#endif // _GNGEOX_HQ2X_C_

SDL_bool effect_hq2x_init ( void ) __attribute__ ( ( warn_unused_result ) );
void effect_hq2x_update ( void );

#endif // _GNGEOX_HQ2X_H_
