/*!
*
*   \file    GnGeoXprofiler.c
*   \brief   Profiler routines.
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
#ifndef _GNGEOX_PROFILER_C_
#define _GNGEOX_PROFILER_C_
#endif // _GNGEOX_PROFILER_C_

#include <SDL2/SDL.h>
#include "zlog.h"

#include "GnGeoXprofiler.h"
#include "GnGeoXconfig.h"

static Uint32 counter[MAX_BLOCK];

/* ******************************************************************************************************************/
/*!
* \brief  Starts profiler.
*
*/
/* ******************************************************************************************************************/
void profiler_start ( enum_gngeoxroms_profiler_type type )
{
    counter[type] = SDL_GetTicks();
}
/* ******************************************************************************************************************/
/*!
* \brief  Stops profiler.
*
*/
/* ******************************************************************************************************************/
void profiler_stop ( enum_gngeoxroms_profiler_type type )
{
    counter[type] = SDL_GetTicks() - counter[type];
}
/* ******************************************************************************************************************/
/*!
* \brief  Shows profiler stats.
*
*/
/* ******************************************************************************************************************/
void profiler_show_stat ( void )
{
    char buffer[256];

    Uint32 all = SDL_GetTicks() - counter[PROF_ALL];

    static Uint32 video = 0;

    if ( counter[PROF_VIDEO] > video )
    {
        video = counter[PROF_VIDEO];
    }

    sprintf ( buffer, "Video:%d (%d) Sound:%d 68K:%d Z80:%d ALL:%d",
              counter[PROF_VIDEO], video, counter[PROF_SOUND],
              counter[PROF_68K], counter[PROF_Z80], all );

    zlog_info ( gngeox_config.loggingCat, "%s", buffer );
}

#ifdef _GNGEOX_PROFILER_C_
#undef _GNGEOX_PROFILER_C_
#endif // _GNGEOX_PROFILER_C_
