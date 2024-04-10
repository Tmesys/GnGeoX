/*!
*
*   \file    GnGeoXsound.c
*   \brief   Sound routines.
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
#ifndef _GNGEOX_SOUND_C_
#define _GNGEOX_SOUND_C_
#endif // _GNGEOX_SOUND_C_

#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include "zlog.h"

#include "GnGeoXsound.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXprofiler.h"
#include "GnGeoXym2610.h"
#include "GnGeoXconfig.h"
#include "GnGeoXz80interf.h"
#include "GnGeoXym2610intf.h"

static Uint16 play_buffer[BUFFER_LEN];

/* ******************************************************************************************************************/
/*!
* \brief  Prints effects list.
*
* \param  userdata Todo.
* \param  stream Todo.
* \param  len Todo.
*/
/* ******************************************************************************************************************/
static void sound_feed_callback ( void* userdata, Uint8* stream, Sint32 len )
{
    profiler_start ( PROF_SOUND );

    YM2610Update_stream ( len / 4, play_buffer );
    memcpy ( stream, ( Uint8* ) play_buffer, len );

    profiler_stop ( PROF_SOUND );
}
/* ******************************************************************************************************************/
/*!
* \brief Initializes NeoGeo sound system.
*
*/
/* ******************************************************************************************************************/
SDL_bool init_sound ( void )
{
    if ( init_sdl_audio() == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }

    neo_z80_init();

    YM2610_sh_start();

    pause_audio ( 0 );

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes audio.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
SDL_bool init_sdl_audio ( void )
{
    SDL_AudioSpec desired, obtain;

    desired.freq = gngeox_config.samplerate;
    desired.samples = NB_SAMPLES;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    desired.format = AUDIO_S16MSB;
#else
    desired.format = AUDIO_S16;
#endif
    desired.channels = 2;
    desired.callback = sound_feed_callback;
    desired.userdata = NULL;

    if ( SDL_OpenAudio ( &desired, &obtain ) < 0 )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return ( SDL_FALSE );
    }

    zlog_info ( gngeox_config.loggingCat, "Obtained sample rate : %d", obtain.freq );
    gngeox_config.samplerate = obtain.freq;

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Closes audio.
*
*/
/* ******************************************************************************************************************/
void close_sdl_audio ( void )
{
    SDL_PauseAudio ( 1 );
    SDL_CloseAudio();
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes audio.
*
* \param  on Todo.
*/
/* ******************************************************************************************************************/
void pause_audio ( Sint32 on )
{
    SDL_PauseAudio ( on );
}

#ifdef _GNGEOX_SOUND_C_
#undef _GNGEOX_SOUND_C_
#endif // _GNGEOX_SOUND_C_
