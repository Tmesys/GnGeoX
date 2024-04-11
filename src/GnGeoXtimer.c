/*!
*
*   \file    GnGeoXtimer.c
*   \brief   .
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.40 (final beta)
*   \date    04/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_TIMER_C_
#define _GNGEOX_TIMER_C_
#endif // _GNGEOX_TIMER_C_

#include <stdlib.h>

#include <SDL2/SDL.h>
#include "zlog.h"

#include "GnGeoXemu.h"
#include "GnGeoXtimer.h"
#include "GnGeoXym2610.h"
#include "GnGeoXconfig.h"

static double timer_count = 0;

static struct_gngeoxtimer_timer timers[MAX_TIMER];

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
* \brief Present time in seconds unit for busy flag emulation.
*
* \param duration Timer duration in seconds unit.
* \param param Callback function parameters.
* \param func Callback function.
* \return Present time in seconds unit.
*/
/* ******************************************************************************************************************/
struct_gngeoxtimer_timer* insert_timer ( double duration, Sint32 param, void ( *func ) ( Sint32 ) )
{
    for ( Sint32 loop = 0; loop < MAX_TIMER; loop++ )
    {
        if ( timers[loop].del_it )
        {
            timers[loop].time = timer_count + duration;
            timers[loop].param = param;
            timers[loop].func = func;
            timers[loop].del_it = 0;

            return ( &timers[loop] );
        }
    }

    /* No timer free */
    zlog_error ( gngeox_config.loggingCat, "No timer available" );
    return ( NULL );
}
/* ******************************************************************************************************************/
/*!
* \brief Deletes all Timers.
*
*/
/* ******************************************************************************************************************/
void free_all_timer ( void )
{
    for ( Sint32 loop = 0; loop < MAX_TIMER; loop++ )
    {
        timers[loop].del_it = 1;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Present time in seconds unit for busy flag emulation.
*
* \param duration Timer duration.
* \param param Callback function parameters.
* \param func Callback function.
* \return Present time in seconds unit.
*/
/* ******************************************************************************************************************/
void del_timer ( struct_gngeoxtimer_timer* timer )
{
    timer->del_it = 1;
}
/* ******************************************************************************************************************/
/*!
* \brief Executes timers.
*
*/
/* ******************************************************************************************************************/
void my_timer ( void )
{
    static Sint32 init = 1;
    static double inc = 0;

    if ( init )
    {
        //timer_init_save_state();
        init = 0;

        if ( gngeox_config.forcepal )
        {
            /* *(1<<TIMER_SH);*/
            inc = ( ( double ) ( 0.02 ) / NB_INTERLACE );
        }
        else
        {
            /* *(1<<TIMER_SH);*/
            inc = ( ( double ) ( 0.01666 ) / NB_INTERLACE );
        }

        for ( Sint32 i = 0; i < MAX_TIMER; i++ )
        {
            timers[i].del_it = 1;
        }
    }

    /* 16ms par frame */
    timer_count += inc;

    for ( Sint32 i = 0; i < MAX_TIMER; i++ )
    {
        if ( timer_count >= timers[i].time && timers[i].del_it == 0 )
        {
            if ( timers[i].func )
            {
                timers[i].func ( timers[i].param );
            }

            timers[i].del_it = 1;
        }
    }
}

#ifdef _GNGEOX_TIMER_C_
#undef _GNGEOX_TIMER_C_
#endif // _GNGEOX_TIMER_C_
