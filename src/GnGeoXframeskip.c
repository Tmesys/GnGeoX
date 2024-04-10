/*!
*
*   \file    GnGeoXframeskip.c
*   \brief   Frame skipping routines.
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
#ifndef _GNGEOX_FRAMESKIP_C_
#define _GNGEOX_FRAMESKIP_C_
#endif // _GNGEOX_FRAMESKIP_C_

#include <unistd.h>
#include <sys/time.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "zlog.h"

#include "GnGeoXframeskip.h"
#include "GnGeoXvideo.h"
#include "GnGeoXscreen.h"
#include "GnGeoXconfig.h"

SDL_bool skip_this_frame = SDL_FALSE;

static char fps_str[32];
static SDL_bool skip_next_frame = SDL_FALSE;
static Uint32 frame;
static SDL_bool init_frame_skip = SDL_TRUE;
/* Looks like SDL_GetTicks is not as precise... */
static Uint32 init_tv = 0;

/* ******************************************************************************************************************/
/*!
* \brief  Computes tick.
*
* \return Current tick.
*/
/* ******************************************************************************************************************/
static Uint32 get_ticks ( void )
{
    if ( init_tv == 0 )
    {
        init_tv = SDL_GetTicks();
    }

    return ( SDL_GetTicks() - init_tv ) * 1000;
}
/* ******************************************************************************************************************/
/*!
* \brief  Resets frame skipping.
*
*/
/* ******************************************************************************************************************/
void neo_frame_skip_reset ( void )
{
    init_tv = 0;

    skip_next_frame = 0;
    init_frame_skip = SDL_TRUE;

    if ( gngeox_config.forcepal == SDL_TRUE )
    {
        frame = ( Uint32 ) ( ( double ) TICKS_PER_SEC / 50 );
    }
    else
    {
        frame = ( Uint32 ) ( ( double ) TICKS_PER_SEC / 61 );
    }

}
/* ******************************************************************************************************************/
/*!
* \brief  Frame skip computations.
*
*/
/* ******************************************************************************************************************/
void neo_frame_skip ( void )
{
    static Sint32 f2skip = 0;
    static Uint32 sec = 0;
    static Uint32 rfd = 0;
    static Uint32 target = 0;
    static Sint32 nb_frame = 0;

    skip_this_frame = skip_next_frame;

    if ( init_frame_skip == SDL_TRUE )
    {
        init_frame_skip = SDL_FALSE;
        target = get_ticks();

        nb_frame = 0;
        sec = 0;

        skip_next_frame = SDL_FALSE;
    }

    target += frame;

    if ( f2skip > 0 )
    {
        f2skip--;

        skip_next_frame = SDL_TRUE;
    }

    rfd = get_ticks();

    if ( gngeox_config.autoframeskip )
    {
        if ( rfd < target && f2skip == 0 )
        {
            while ( get_ticks() < target );
        }
        else
        {
            f2skip = ( rfd - target ) / ( double ) frame;

            if ( f2skip > MAX_FRAMESKIP )
            {
                f2skip = MAX_FRAMESKIP;
                neo_frame_skip_reset();
            }
        }
    }

    nb_frame++;

    if ( gngeox_config.showfps == SDL_TRUE )
    {
        if ( get_ticks() - sec >= TICKS_PER_SEC )
        {
            sprintf ( fps_str, "%2d", nb_frame - 1 );

            nb_frame = 0;
            sec = get_ticks();
        }
    }

    skip_next_frame = SDL_FALSE;
}
/* ******************************************************************************************************************/
/*!
* \brief  Frame skip computations.
*
*/
/* ******************************************************************************************************************/
void neo_frame_skip_display ( void )
{
    if ( gngeox_config.showfps == SDL_TRUE )
    {
        SDL_textout ( sdl_surface_buffer, visible_area.x + 8, visible_area.y, fps_str );
    }
}

#ifdef _GNGEOX_FRAMESKIP_C_
#undef _GNGEOX_FRAMESKIP_C_
#endif // _GNGEOX_FRAMESKIP_C_
