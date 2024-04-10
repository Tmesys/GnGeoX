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

#include "3rdParty/MameZ80/Z80.h"
#include "GnGeoXym2610intf.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXtimer.h"
#include "GnGeoXz80interf.h"
#include "GnGeoXym2610.h"
#include "GnGeoXconfig.h"

static struct_gngeoxtimer_timer* Timer[2];

/* ******************************************************************************************************************/
/*!
* \brief  TimerHandler from "fm.c".
*
* \param  c Todo
* \param  count Todo
* \param  stepTime Todo
*/
/* ******************************************************************************************************************/
static void TimerHandler ( Sint32 c, Sint32 count, double stepTime )
{
    /* Reset FM Timer */
    if ( count == 0 )
    {
        if ( Timer[c] )
        {
            del_timer ( Timer[c] );
            Timer[c] = 0;
        }
    }
    /* Start FM Timer */
    else
    {
        double timeSec = ( double ) count * stepTime;

        if ( Timer[c] == 0 )
        {
            Timer[c] = insert_timer ( timeSec, c, timer_callback_2610 );
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Timer overflow callback from "timer.c".
*
* \param  param Todo
*/
/* ******************************************************************************************************************/
void timer_callback_2610 ( Sint32 param )
{
    Timer[param] = 0;
    YM2610TimerOver ( param );
}
/* ******************************************************************************************************************/
/*!
* \brief  Timer initialization.
*
*/
/* ******************************************************************************************************************/
void FMTimerInit ( void )
{
    Timer[0] = Timer[1] = 0;
    free_all_timer();
}
/* ******************************************************************************************************************/
/*!
* \brief  Handles NeoGeo sound Irq.
*
* \param  irq Irq flag ?
*/
/* ******************************************************************************************************************/
void neogeo_sound_irq ( Sint32 irq )
{
    if ( irq )
    {
        z80_set_irq_line ( 0, ASSERT_LINE );
    }
    else
    {
        z80_set_irq_line ( 0, CLEAR_LINE );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Timer overflow callback from "timer.c".
*
*/
/* ******************************************************************************************************************/
void YM2610_sh_start ( void )
{
    FMTimerInit();
    /* initialize YM2610 */
    YM2610Init ( 8000000, gngeox_config.samplerate, neogeo_memory.rom.rom_region[REGION_AUDIO_DATA_1].p, neogeo_memory.rom.rom_region[REGION_AUDIO_DATA_1].size,
                 neogeo_memory.rom.rom_region[REGION_AUDIO_DATA_2].p, neogeo_memory.rom.rom_region[REGION_AUDIO_DATA_2].size, TimerHandler, neogeo_sound_irq );
}

#ifdef _GNGEOX_YM2610_INTERF_C_
#undef _GNGEOX_YM2610_INTERF_C_
#endif
