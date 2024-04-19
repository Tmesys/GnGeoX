/*!
*
*   \file    GnGeoXemu.c
*   \brief   Emulation routines ?
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version).
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X).
*   \version 01.00
*   \date    02/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_EMU_C_
#define _GNGEOX_EMU_C_
#endif // _GNGEOX_EMU_C_

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "zlog.h"

#include "GnGeoXemu.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXframeskip.h"
#include "GnGeoXpd4990a.h"
#include "GnGeoXprofiler.h"
#include "GnGeoXdebug.h"
#include "GnGeoXym2610.h"
#include "GnGeoXsound.h"
#include "GnGeoXscreen.h"
#include "GnGeoXneocrypt.h"
#include "GnGeoXconfig.h"
#include "GnGeoXcontroller.h"
#include "GnGeoX68k.h"
#include "GnGeoXz80.h"
#include "GnGeoXscanline.h"

Uint32 cpu_68k_timeslice_scanline = 0;
Uint32 cpu_z80_timeslice_interlace = 0;
/* ******************************************************************************************************************/
/*!
* \brief Initializes NeoGeo.
*
*/
/* ******************************************************************************************************************/
SDL_bool neo_sys_init ( void )
{
    /* 12 MHz */
    Uint32 cpu_68k_timeslice = 12000000;
    /* 4 MHz */
    Uint32 cpu_z80_timeslice = 4000000;
    if ( gngeox_config.forcepal )
    {
        cpu_68k_timeslice /= 50;
        cpu_z80_timeslice /= 50;
    }
    else
    {
        cpu_68k_timeslice /= 60;
        cpu_z80_timeslice /= 60;
    }

    cpu_68k_timeslice_scanline = ( cpu_68k_timeslice / 264.0 );
    cpu_z80_timeslice_interlace = cpu_z80_timeslice / ( float ) NB_INTERLACE;

    cpu_68k_init();

    pd4990a_init();

    /* @note (Tmesys#1#13/04/2024): MUST always be after screen initialization. */
    if ( neo_sound_init() == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }
    atexit ( neo_sound_close );

    neo_sys_reset();

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief Resets NeoGeo system.
*
*/
/* ******************************************************************************************************************/
static void neo_sys_reset ( void )
{
    sram_lock = SDL_FALSE;
    neogeo_memory.z80_command = 0;
    neogeo_memory.z80_command_reply = 0;

    if ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].size > 0x100000 )
    {
        cpu_68k_bankswitch ( 0x100000 );
    }
    else
    {
        cpu_68k_bankswitch ( 0 );
    }

    cpu_68k_reset();
}
/* ******************************************************************************************************************/
/*!
* \brief Handles NeoGeo interrupt.
*
*/
/* ******************************************************************************************************************/
void neo_sys_interrupt ( void )
{
    pd4990a_addretrace();

    if ( ! ( neogeo_memory.vid.irq2control & 0x8 ) )
    {
        if ( frame_counter >= neogeo_frame_counter_speed )
        {
            frame_counter = 0;
            neogeo_frame_counter++;
        }

        frame_counter++;
    }

    neo_frame_skip ( );

    if ( !skip_this_frame )
    {
#ifdef ENABLE_PROFILER
        profiler_start ( PROF_VIDEO );
#endif // ENABLE_PROFILER

        draw_screen();

#ifdef ENABLE_PROFILER
        profiler_stop ( PROF_VIDEO );
#endif // ENABLE_PROFILER
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Main loop.
*
*/
/* ******************************************************************************************************************/
void neo_sys_update_events ( void )
{
    SDL_Event event;

    while ( SDL_PollEvent ( &event ) )
    {
        switch ( event.type )
        {
        case ( SDL_CONTROLLERBUTTONDOWN ) :
            {
                neo_controllers_update ( CONTROLLER_STATE_DOWN, event.cdevice.which, event.cbutton.button );
            }
            break;
        case ( SDL_CONTROLLERBUTTONUP ) :
            {
                neo_controllers_update ( CONTROLLER_STATE_UP, event.cdevice.which, event.cbutton.button );
            }
            break;
        case ( SDL_CONTROLLERAXISMOTION ) :
            {
                neo_controllers_update_axis ( event.caxis.which, event.caxis.axis, event.caxis.value );
            }
            break;
        case ( SDL_WINDOWEVENT ) :
            {
                switch ( event.window.event )
                {
                case ( SDL_WINDOWEVENT_RESIZED ) :
                case ( SDL_WINDOWEVENT_SIZE_CHANGED ) :
                    {
                        gngeox_config.res_x = event.window.data1;
                        gngeox_config.res_y = event.window.data2;
                        neo_screen_resize ( event.window.data1, event.window.data2 );
                    }
                    break;
                case ( SDL_WINDOWEVENT_CLOSE ) :
                    {
                        close_game();
                        exit ( EXIT_SUCCESS );
                    }
                }
            }
            break;
        case ( SDL_JOYDEVICEADDED ) :
            {
                zlog_info ( gngeox_config.loggingCat, "Joy plugged %d", event.jdevice.which );
                neo_controllers_plug ( event.cdevice.which );
            }
            break;
        case ( SDL_JOYDEVICEREMOVED ) :
            {
                zlog_info ( gngeox_config.loggingCat, "Joy Unplugged %d", event.jdevice.which );
                neo_controllers_unplug ( event.cdevice.which );
            }
            break;
        case ( SDL_AUDIODEVICEADDED ) :
            {
                zlog_info ( gngeox_config.loggingCat, "New audio device plugged %d", event.adevice.which );
            }
            break;
        case ( SDL_AUDIODEVICEREMOVED ) :
            {
                zlog_info ( gngeox_config.loggingCat, "Audio device Unplugged %d", event.adevice.which );
            }
            break;
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Main loop.
*
*/
/* ******************************************************************************************************************/
void neo_sys_main_loop ( void )
{
    neo_frame_skip_reset();
    neo_ym2610_update();

    while ( 1 )
    {
#ifdef ENABLE_PROFILER
        profiler_start ( PROF_ALL );
#endif // ENABLE_PROFILER

        if ( neogeo_memory.test_switch == 1 )
        {
            neogeo_memory.test_switch = 0;
        }

        neo_sys_update_events();

#ifdef ENABLE_PROFILER
        profiler_start ( PROF_Z80 );
#endif // ENABLE_PROFILER

        for ( Uint32 i = 0; i < NB_INTERLACE; i++ )
        {
            z80_run ( cpu_z80_timeslice_interlace, 0 );
            neo_ym2610_update();
        }

#ifdef ENABLE_PROFILER
        profiler_stop ( PROF_Z80 );
#endif // ENABLE_PROFILER

        current_line = 0;

#ifdef ENABLE_PROFILER
        profiler_start ( PROF_68K );
#endif // ENABLE_PROFILER

        for ( Uint32 i = 0; i < 264; i++ )
        {
            cpu_68k_run ( cpu_68k_timeslice_scanline );

            if ( update_scanline() )
            {
                cpu_68k_interrupt ( 2 );
            }
        }

#ifdef ENABLE_PROFILER
        profiler_stop ( PROF_68K );
#endif // ENABLE_PROFILER
        //cpu_68k_run ( cpu_68k_timeslice_scanline );

        update_screen();
        neogeo_memory.watchdog++;

        if ( neogeo_memory.watchdog > 7 )
        {
            zlog_info ( gngeox_config.loggingCat, "Watchdog Reset" );
            cpu_68k_reset();
        }

        cpu_68k_interrupt ( 1 );

#ifdef ENABLE_PROFILER
        profiler_stop ( PROF_ALL );
        profiler_show_stat();
#endif
    }
}

#ifdef _GNGEOX_EMU_C_
#undef _GNGEOX_EMU_C_
#endif // _GNGEOX_EMU_C_
