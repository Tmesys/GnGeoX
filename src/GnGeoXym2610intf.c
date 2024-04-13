/*!
*
*   \file    GnGeoXym2610intf.c
*   \brief   Yamaha YM2610 sound chip emulation interface.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
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
#ifndef _GNGEOX_YM2610_INTERF_C_
#define _GNGEOX_YM2610_INTERF_C_
#endif

#include <stdio.h>

#include <SDL2/SDL.h>
#include "zlog.h"
#include "qlibc.h"

#include "3rdParty/MameZ80/Z80.h"
#include "GnGeoXym2610intf.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXz80.h"
#include "GnGeoXym2610.h"
#include "GnGeoXconfig.h"
#include "GnGeoXemu.h"

static qlist_t * ym2610_timers = NULL;
static double timer_count = 0;
static double timer_increment = 0;

/* ******************************************************************************************************************/
/*!
* \brief  Timers callback.
*
* \param  timer_id 0 or 1
* \param  count Todo
* \param  step_time Todo
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
        for ( Sint32 loop = 0; loop < ym2610_timers->num ; loop++ )
        {
            struct_gngeoxtimer_timer * timer = qlist_getat ( ym2610_timers, loop, NULL, true );

            if ( timer->timer_id == timer_id )
            {
                if ( qlist_removeat ( ym2610_timers, loop ) == false )
                {
                    zlog_error ( gngeox_config.loggingCat, "Can not remove timer at %d", loop );
                }
            }

            free ( timer );
        }
    }
    /* Start new FM Timer */
    else
    {
        double duration = ( double ) count * step_time;
        struct_gngeoxtimer_timer timer;

        timer.target = ( timer_count + duration );
        timer.timer_id = timer_id;

        if ( qlist_addfirst ( ym2610_timers, &timer, sizeof ( timer ) ) == false )
        {
            zlog_error ( gngeox_config.loggingCat, "Can not add new timer" );
        }

        zlog_warn ( gngeox_config.loggingCat, "Timer %d : count %d step %f", timer_id, count, step_time );

        /* @todo (Tmesys#1#13/04/2024): Clean This as it is for test purposes as this new implementation is brand new. */
        if ( ym2610_timers->num > 2 )
        {
            zlog_warn ( gngeox_config.loggingCat, "more than 2 timers" );
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Present time in seconds unit for busy flag emulation.
*
* \return Present time in seconds unit.
*/
/* ******************************************************************************************************************/
double timer_get_time ( void )
{
    return timer_count;
}
/* ******************************************************************************************************************/
/*!
* \brief  Timer overflow callback from "timer.c".
*
*/
/* ******************************************************************************************************************/
void neo_ym2610_init ( void )
{
    /* initialize YM2610 */
    YM2610Init ( 8000000,
                 gngeox_config.samplerate,
                 neogeo_memory.rom.rom_region[REGION_AUDIO_DATA_1].p,
                 neogeo_memory.rom.rom_region[REGION_AUDIO_DATA_1].size,
                 neogeo_memory.rom.rom_region[REGION_AUDIO_DATA_2].p,
                 neogeo_memory.rom.rom_region[REGION_AUDIO_DATA_2].size,
                 neo_ym2610_callback,
                 neo_z80_irq );

    ym2610_timers = qlist ( QLIST_THREADSAFE );
    if ( ym2610_timers == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Can not create timer list" );
    }

    if ( gngeox_config.forcepal )
    {
        /* *(1<<TIMER_SH);*/
        timer_increment = ( ( double ) ( 0.02 ) / NB_INTERLACE );
    }
    else
    {
        /* *(1<<TIMER_SH);*/
        timer_increment = ( ( double ) ( 0.01666 ) / NB_INTERLACE );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Executes timers.
*
*/
/* ******************************************************************************************************************/
void neo_ym2610_update ( void )
{
    /* 16ms par frame */
    timer_count += timer_increment;

    for ( Sint32 loop = 0; loop < ym2610_timers->num ; loop++ )
    {
        struct_gngeoxtimer_timer * timer = qlist_getat ( ym2610_timers, loop, NULL, true );

        if ( timer_count >= timer->target )
        {
            qlist_popat ( ym2610_timers, loop, NULL );
            YM2610TimerOver ( timer->timer_id );
        }

        free ( timer );
    }
}

#ifdef _GNGEOX_YM2610_INTERF_C_
#undef _GNGEOX_YM2610_INTERF_C_
#endif
