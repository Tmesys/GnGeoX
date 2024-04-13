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

#include "GnGeoXversion.h"
#include "GnGeoXemu.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXframeskip.h"
#include "GnGeoXpd4990a.h"
#include "GnGeoXprofiler.h"
#include "GnGeoXdebug.h"
#include "GnGeoXym2610intf.h"
#include "GnGeoXsound.h"
#include "GnGeoXscreen.h"
#include "GnGeoXneocrypt.h"
#include "GnGeoXconfig.h"
#include "GnGeoXcontroller.h"
#include "GnGeoX68k.h"
#include "GnGeoXz80.h"
#include "GnGeoXscanline.h"

/* ******************************************************************************************************************/
/*!
* \brief Initializes NeoGeo.
*
*/
/* ******************************************************************************************************************/
void neo_sys_init ( void )
{
    /*
        char * test = NULL;
        printf ( "GnGeoX version %d.%d.%d.rev%d\n"
                 , GNGEOX_MAJOR
                 , GNGEOX_MINOR
                 , GNGEOX_BUILD
                 , GNGEOX_REVISION );
        test = qsys_info_osname ();
        printf ( "-> Operating system %s\n", qsys_info_osname () );
        printf ( "-> Operating system version %s\n", qsys_info_version () );
        printf ( "-> Operating system release %s\n", qsys_info_release () );
        printf ( "-> System node %s\n", qsys_info_node () );
        printf ( "-> System machine %s\n", qsys_info_machine () );
    */
    cpu_68k_init();

    pd4990a_init();

    init_sound();

    neo_sys_reset();
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
        profiler_start ( PROF_VIDEO );

        draw_screen();

        profiler_stop ( PROF_VIDEO );
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
                zlog_info ( gngeox_config.loggingCat, "Axis number %d axis %d value %d", event.caxis.which, event.caxis.axis, event.caxis.value );
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
    Sint32 neo_emu_done = 0;
    Sint32 interrupt = 0;

    Uint32 cpu_68k_timeslice = 200000;
    Uint32 cpu_68k_timeslice_scanline = ( cpu_68k_timeslice / 264.0 );
    Uint32 cpu_z80_timeslice = 73333;
    Uint32 tm_cycle = 0;

    Uint32 cpu_z80_timeslice_interlace = cpu_z80_timeslice / ( float ) NB_INTERLACE;

    neo_frame_skip_reset();
    neo_ym2610_update();

    while ( !neo_emu_done )
    {
        if ( neogeo_memory.test_switch == 1 )
        {
            neogeo_memory.test_switch = 0;
        }

        neo_sys_update_events();

        profiler_start ( PROF_Z80 );

        for ( Uint32 i = 0; i < NB_INTERLACE; i++ )
        {
            neo_z80_run ( cpu_z80_timeslice_interlace );
            neo_ym2610_update();
        }

        profiler_stop ( PROF_Z80 );

        if ( !gngeox_config.debug )
        {
            if ( gngeox_config.raster )
            {
                current_line = 0;

                for ( Uint32 i = 0; i < 264; i++ )
                {
                    tm_cycle = cpu_68k_run ( cpu_68k_timeslice_scanline - tm_cycle );

                    if ( update_scanline() )
                    {
                        cpu_68k_interrupt ( 2 );
                    }
                }

                tm_cycle = cpu_68k_run ( cpu_68k_timeslice_scanline - tm_cycle );
                //state_handling(pending_save_state, pending_load_state);

                update_screen();
                neogeo_memory.watchdog++;

                if ( neogeo_memory.watchdog > 7 )
                {
                    zlog_info ( gngeox_config.loggingCat, "Watchdog Reset" );
                    cpu_68k_reset();
                }

                cpu_68k_interrupt ( 1 );
            }
            else
            {
                profiler_start ( PROF_68K );
                tm_cycle = cpu_68k_run ( cpu_68k_timeslice - tm_cycle );
                profiler_stop ( PROF_68K );

                neo_sys_interrupt();

                /* state handling (we save/load before interrupt) */
                //state_handling(pending_save_state, pending_load_state);

                neogeo_memory.watchdog++;

                /* Watchdog reset after ~0.13 == ~7.8 frames */
                if ( neogeo_memory.watchdog > 7 )
                {
                    zlog_info ( gngeox_config.loggingCat, "Watchdog Reset %d", neogeo_memory.watchdog );
                    cpu_68k_reset();
                }

                cpu_68k_interrupt ( interrupt );
            }
        }
        else
        {
            /* we are in debug mode -> we are just here for event handling */
            neo_emu_done = 1;
        }

#ifdef ENABLE_PROFILER
        profiler_show_stat();
#endif
        profiler_start ( PROF_ALL );
    }

    pause_audio ( 1 );
}

#ifdef _GNGEOX_EMU_C_
#undef _GNGEOX_EMU_C_
#endif // _GNGEOX_EMU_C_
