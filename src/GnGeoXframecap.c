/*!
*
*   \file    GnGeoXframeskip.c
*   \brief   Frame skipping routines.
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
#ifndef _GNGEOX_FRAMECAP_C_
#define _GNGEOX_FRAMECAP_C_
#endif // _GNGEOX_FRAMECAP_C_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "bstrlib.h"
#include "zlog.h"

#include "GnGeoXframecap.h"
#include "GnGeoXvideo.h"
#include "GnGeoXscreen.h"
#include "GnGeoXconfig.h"

static bstring frame_rate_string = NULL;
static double frame_rate_real = 0;
static double frame_rate_cap = 0;

static Uint64 ticks_start = 0;
static Uint64 ticks_stop_real = 0;
static Uint64 ticks_stop_cap = 0;
static Uint8 ticks_max = 0;
/* ******************************************************************************************************************/
/*!
* \brief  .
*
*/
/* ******************************************************************************************************************/
void neo_frame_cap_init ( void )
{
    if ( gngeox_config.forcepal )
    {
        ticks_max = 20;
    }
    else
    {
        ticks_max = 17;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
*/
/* ******************************************************************************************************************/
void neo_frame_cap_start ( void )
{
    ticks_start = SDL_GetTicks64();
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
*/
/* ******************************************************************************************************************/
void neo_frame_cap_stop ( void )
{
    Sint32 elapsed = 0;

    ticks_stop_real = SDL_GetTicks64();

    elapsed = ( ticks_stop_real - ticks_start );
    frame_rate_real = ( elapsed > 0 ) ? 1000.0f / elapsed : 0.0f;

    if ( elapsed < ticks_max )
    {
        SDL_Delay ( ticks_max - elapsed ) ;
    }

    ticks_stop_cap = SDL_GetTicks64();
    elapsed = ( ticks_stop_cap - ticks_start );
    frame_rate_cap = ( elapsed > 0 ) ? 1000.0f / elapsed : 0.0f;
}
/* ******************************************************************************************************************/
/*!
* \brief  Display frame rate.
*
*/
/* ******************************************************************************************************************/
void neo_frame_rate_display ( void )
{
    frame_rate_string = bformat ( "FPSR : %.2f FPSC : %.2f", frame_rate_real, frame_rate_cap );
    if ( gngeox_config.showfps == SDL_TRUE && frame_rate_string != NULL && frame_rate_string->mlen > 0 )
    {
        SDL_textout ( visible_area.x + 8, visible_area.y, ( const char* ) frame_rate_string->data );
    }
    bdestroy ( frame_rate_string );
}
/* ******************************************************************************************************************/
/*!
* \brief  .
*
*/
/* ******************************************************************************************************************/
void neo_frame_cap_close ( void )
{
}

#ifdef _GNGEOX_FRAMECAP_C_
#undef _GNGEOX_FRAMECAP_C_
#endif // _GNGEOX_FRAMECAP_C_
