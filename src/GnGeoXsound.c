/*!
*
*   \file    GnGeoXsound.c
*   \brief   Sound routines.
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
#include "GnGeoXym2610core.h"
#include "GnGeoXconfig.h"
#include "GnGeoXz80.h"
#include "GnGeoXym2610.h"

static Uint16 sound_buffer[BUFFER_LEN];

/* ******************************************************************************************************************/
/*!
* \brief  Prints effects list.
*
* \param userdata An application-specific parameter saved in
*                  the SDL_AudioSpec structure
* \param stream A pointer to the audio data buffer.
* \param len    The length of that buffer in bytes.
*/
/* ******************************************************************************************************************/
static void neo_sound_feed_callback ( void* userdata, Uint8* stream, Sint32 len )
{
#ifdef ENABLE_PROFILER
    profiler_start ( PROF_SOUND );
#endif // ENABLE_PROFILER

    /* @note (Tmesys#1#13/04/2024): Well, YM2610 expects length in 32 bits words, 16 bits for each channel.  */
    YM2610Update_stream ( len / 4, sound_buffer );
    memcpy ( stream, ( Uint8* ) sound_buffer, len );

#ifdef ENABLE_PROFILER
    profiler_stop ( PROF_SOUND );
#endif // ENABLE_PROFILER
}
/* ******************************************************************************************************************/
/*!
* \brief Initializes NeoGeo sound system.
*
*/
/* ******************************************************************************************************************/
SDL_bool neo_sound_init ( void )
{
    SDL_AudioSpec desired, obtain;

    SDL_zero ( desired );
    SDL_zero ( obtain );

    desired.freq = gngeox_config.samplerate;
    desired.samples = NB_SAMPLES;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    desired.format = AUDIO_S16MSB;
#else
    desired.format = AUDIO_S16;
#endif
    desired.channels = NB_CHANNELS;
    desired.callback = neo_sound_feed_callback;
    desired.userdata = NULL;

    if ( SDL_OpenAudio ( &desired, &obtain ) < 0 )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return ( SDL_FALSE );
    }

    if ( obtain.freq != gngeox_config.samplerate )
    {
        zlog_warn ( gngeox_config.loggingCat, "Forcing sample rate to obtained value : %d", obtain.freq );
        gngeox_config.samplerate = obtain.freq;
    }

    neo_z80_init();

    if ( neo_ym2610_init() == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }

    SDL_PauseAudio ( 0 );

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Closes audio.
*
*/
/* ******************************************************************************************************************/
void neo_sound_close ( void )
{
    SDL_PauseAudio ( 1 );
    SDL_CloseAudio();
    neo_ym2610_close ();
}

#ifdef _GNGEOX_SOUND_C_
#undef _GNGEOX_SOUND_C_
#endif // _GNGEOX_SOUND_C_
