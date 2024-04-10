/*!
*
*   \file    GnGeoXsound.h
*   \brief   Sound routines header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    18/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_SOUND_H_
#define _GNGEOX_SOUND_H_

#include <SDL2/SDL.h>

#define NB_SEGMENT 20
#define MIXER_MAX_CHANNELS 16
#define BUFFER_LEN 16384
/* better resolution */
#define NB_SAMPLES 512

SDL_bool init_sound ( void );
SDL_bool init_sdl_audio ( void ) __attribute__ ( ( warn_unused_result ) );
void close_sdl_audio ( void );
void pause_audio ( Sint32 );

#endif
