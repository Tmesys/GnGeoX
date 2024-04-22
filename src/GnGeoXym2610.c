/*!
*
*   \file    GnGeoXym2610.c
*   \brief   Yamaha YM2610 sound chip emulation interface.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    01/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    The YM2610 emulator supports up to 2 chips, each chip has the
*            following connections :
*            - Status Read / Control Write A
*            - Port Read / Data Write A
*            - Control Write B
*            - Data Write B
*/
#ifndef _GNGEOX_YM2610_C_
#define _GNGEOX_YM2610_C_
#endif // _GNGEOX_YM2610_C_

#include <stdio.h>

#include <SDL2/SDL.h>
#include "zlog.h"
#include "qlibc.h"

#include "3rdParty/Z80/Z80.h"
#include "GnGeoXym2610.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXz80.h"
#include "GnGeoXym2610core.h"
#include "GnGeoXconfig.h"
#include "GnGeoXemu.h"

static qlist_t * ym2610_timers[2];
static double timer_count = 0;
static double timer_increment = 0;

/* ******************************************************************************************************************/
/*!
* \brief  Timers callback controlled by YM2610 emulation.
*
* \param  timer_id 0 for A or 1 for B.
* \param  count Number of step_time to target.
* \param  step_time Step time in ms.
*
* \note   The YM2610 provides 2 timers called A and B.
*         Used to time music playback by triggering Z80 interrupts.
*         The timer A is 10 bits wide, the timer B is only 8 bits wide.
*/
/* ******************************************************************************************************************/
static void neo_ym2610_callback ( Sint32 timer_id, Sint32 count, double step_time )
{
    /* Reset FM Timer */
    if ( count == 0 )
    {
        qlist_clear ( ym2610_timers[timer_id] );
    }
    /* Start new FM Timer */
    else
    {
        double duration = ( double ) count * step_time;
        struct_gngeoxtimer_timer timer;

        timer.target = ( timer_count + duration );

        if ( qlist_addfirst ( ym2610_timers[timer_id], &timer, sizeof ( timer ) ) == false )
        {
            zlog_error ( gngeox_config.loggingCat, "Can not add new timer" );
        }
        /* @note (Tmesys#1#13/04/2024): Just in case */
        if ( ym2610_timers[timer_id]->num > 1 )
        {
            zlog_warn ( gngeox_config.loggingCat, "Timer %d : count %d step %f", timer_id, count, step_time );
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Present count in ms used by YM2610 emulation..
*
* \return Present cound in ms unit.
*/
/* ******************************************************************************************************************/
double neo_ym2610_count ( void )
{
    return timer_count;
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes YM2610 and timers.
*
*/
/* ******************************************************************************************************************/
SDL_bool neo_ym2610_init ( void )
{
    /* initialize YM2610 */
    YM2610Init ( YM2610_CLOCK_FREQ_HZ,
                 gngeox_config.samplerate,
                 neogeo_memory.rom.rom_region[REGION_AUDIO_DATA_1].p,
                 neogeo_memory.rom.rom_region[REGION_AUDIO_DATA_1].size,
                 neogeo_memory.rom.rom_region[REGION_AUDIO_DATA_2].p,
                 neogeo_memory.rom.rom_region[REGION_AUDIO_DATA_2].size,
                 neo_ym2610_callback,
                 neo_z80_irq );

    for ( Sint32 loop = 0; loop < YM2610_MAX_TIMERS ; loop++ )
    {
        ym2610_timers[loop] = qlist ( QLIST_THREADSAFE );
        if ( ym2610_timers[loop] == NULL )
        {
            zlog_error ( gngeox_config.loggingCat, "Can not create timer list" );
            return ( SDL_FALSE );
        }
    }

    if ( gngeox_config.forcepal )
    {
        /* *(1<<TIMER_SH);*/
        timer_increment = ( ( double ) ( 0.02 ) / EMU_NB_INTERLACE );
    }
    else
    {
        /* *(1<<TIMER_SH);*/
        timer_increment = ( ( double ) ( 0.01666 ) / EMU_NB_INTERLACE );
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief Checks timers et updates them.
*
*/
/* ******************************************************************************************************************/
void neo_ym2610_update ( void )
{
    /* 16ms par frame */
    timer_count += timer_increment;

    for ( Uint32 loop1 = 0; loop1 < YM2610_MAX_TIMERS ; loop1++ )
    {
        for ( Uint32 loop2 = 0; loop2 < ym2610_timers[loop1]->num ; loop2++ )
        {
            struct_gngeoxtimer_timer * timer = qlist_getat ( ym2610_timers[loop1], loop2, NULL, true );

            if ( timer_count >= timer->target )
            {
                qlist_removeat ( ym2610_timers[loop1], loop2 );
                /* @note (Tmesys#1#13/04/2024): Triggers Z80 irq and reloads timers */
                YM2610TimerOver ( loop1 );
            }

            free ( timer );
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Executes timers.
*
*/
/* ******************************************************************************************************************/
void neo_ym2610_close ( void )
{
    for ( Sint32 loop = 0; loop < YM2610_MAX_TIMERS ; loop++ )
    {
        qlist_free ( ym2610_timers[loop] );
    }
}
#ifdef _GNGEOX_YM2610_C_
#undef _GNGEOX_YM2610_C_
#endif
