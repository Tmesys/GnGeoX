/*!
*
*   \file    GnGeoXdebug.c
*   \brief   68K debugging routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    01/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_DEBUG_C_
#define _GNGEOX_DEBUG_C_
#endif // _GNGEOX_DEBUG_C_

#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "zlog.h"
#include "bstrlib.h"
#include "qlibc.h"
#include "Z80.h"

#include "generator.h"
#include "cpu68k.h"
#include "reg68k.h"
#include "mem68k.h"
#include "diss68k.h"

#include "GnGeoXdebug.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXconfig.h"
#include "GnGeoX68k.h"
#include "GnGeoXscreen.h"
#include "GnGeoXscanline.h"
#include "GnGeoXemu.h"
#include "GnGeoXym2610.h"
#include "GnGeoXroms.h"
#include "version.h"

static qvector_t *breakpoints = NULL;
static qvector_t *backtrace = NULL;
/* ******************************************************************************************************************/
/*!
* \brief  Adds a back trace.
*
* \param  program_counter Program counter.
*/
/* ******************************************************************************************************************/
static SDL_bool neo_debug_back_trace_add ( Uint32 program_counter )
{
    if ( qvector_size ( backtrace ) == GNGEOXDEBUG_MAX_BACK_TRACE )
    {
        if ( qvector_removefirst ( backtrace ) == false )
        {
            zlog_error ( gngeox_config.loggingCat, "Can not make room for new back-trace" );
            return ( SDL_FALSE );
        }
    }

    if ( qvector_addlast ( backtrace, &program_counter ) == false )
    {
        zlog_error ( gngeox_config.loggingCat, "Can not add back-trace" );
        return ( SDL_FALSE );
    }
    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Shows back traces.
*
*/
/* ******************************************************************************************************************/
static void neo_debug_back_trace_print ( void )
{
    qvector_obj_t obj;
    SDL_zero ( obj );

    while ( qvector_getnext ( backtrace, &obj, false ) == true )
    {
        gen68k_disassemble ( * ( ( Uint32 * ) obj.data ) );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Adds conditions.
*
* \param  type Todo.
* \param  reg Todo.
* \param  value Todo.
*/
/* ******************************************************************************************************************/
static void add_cond ( Uint8 type, Sint32 reg, Uint32 value )
{
    /* @todo (Tmesys#1#12/04/2022): Conditions to what ?? Should be implemented */
}
/* ******************************************************************************************************************/
/*!
* \brief  Adds break points condition.
*
* \param  program_counter Program counter.
*/
/* ******************************************************************************************************************/
/* 12655316 */
static SDL_bool neo_debug_break_point_add ( Sint32 program_counter )
{
    if ( qvector_size ( breakpoints ) == GNGEOXDEBUG_MAX_BREAK_POINT )
    {
        if ( qvector_removefirst ( breakpoints ) == false )
        {
            zlog_error ( gngeox_config.loggingCat, "Can not make room for new break-point" );
            return ( SDL_FALSE );
        }
    }

    if ( qvector_addlast ( breakpoints, &program_counter ) == false )
    {
        zlog_error ( gngeox_config.loggingCat, "Can not feed break-point" );
        return ( SDL_FALSE );
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Checks if program counter value is a break point.
*
* \param  program_counter Program counter.
* \return SDL_TRUE when program counter has a break point, SDL_FALSE otherwise.
*/
/* ******************************************************************************************************************/
static SDL_bool neo_debug_break_point_check ( Sint32 program_counter )
{
    qvector_obj_t obj;
    SDL_zero ( obj );

    while ( qvector_getnext ( breakpoints, &obj, false ) == true )
    {
        if ( * ( ( Uint32 * ) obj.data ) == program_counter )
        {
            return ( SDL_TRUE );
        }
    }

    return ( SDL_FALSE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Deletes break point.
*
* \param  program_counter Program counter.
*/
/* ******************************************************************************************************************/
static SDL_bool neo_debug_break_point_delete ( Sint32 program_counter )
{
    qvector_obj_t obj;
    Uint32 index = 0;

    SDL_zero ( obj );

    while ( qvector_getnext ( breakpoints, &obj, false ) == true )
    {
        if ( * ( ( Uint32 * ) obj.data ) == program_counter )
        {
            index = obj.index;
            break;
        }
    }

    if ( index != 0 )
    {
        if ( qvector_removeat ( breakpoints, index ) == false )
        {
            zlog_error ( gngeox_config.loggingCat, "Can not remove break-point %d", program_counter );
            return ( SDL_FALSE );
        }
    }
    else
    {
        zlog_error ( gngeox_config.loggingCat, "Break point %d not found", program_counter );
        return ( SDL_FALSE );
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Clears all break points.
*
*/
/* ******************************************************************************************************************/
static void neo_debug_break_point_clear ( void )
{
    qvector_clear ( breakpoints );
}
/* ******************************************************************************************************************/
/*!
* \brief  Prints all break points.
*
*/
/* ******************************************************************************************************************/
static void neo_debug_break_point_print ( void )
{
    qvector_obj_t obj;
    SDL_zero ( obj );

    while ( qvector_getnext ( breakpoints, &obj, false ) == true )
    {
        zlog_debug ( gngeox_config.loggingCat, "[%d]", * ( ( Uint32 * ) obj.data ) );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Dumps 68k internal registers with printing.
*
*/
/* ******************************************************************************************************************/
static void gen68k_dumpreg ( void )
{
    zlog_debug ( gngeox_config.loggingCat, "d0=%08x   d4=%08x   a0=%08x   a4=%08x   CCR=%c%c%c%c%c %04x",
                 regs.regs[0], regs.regs[4], regs.regs[8], regs.regs[12],
                 ( ( regs.sr.sr_int >> 4 ) & 1 ? 'X' : '-' ),
                 ( ( regs.sr.sr_int >> 3 ) & 1 ? 'N' : '-' ),
                 ( ( regs.sr.sr_int >> 2 ) & 1 ? 'Z' : '-' ),
                 ( ( regs.sr.sr_int >> 1 ) & 1 ? 'V' : '-' ),
                 ( ( regs.sr.sr_int ) & 1 ? 'C' : '-' ), regs.sr.sr_int );
    zlog_debug ( gngeox_config.loggingCat, "d1=%08x   d5=%08x   a1=%08x   a5=%08x",
                 regs.regs[1], regs.regs[5], regs.regs[9], regs.regs[13] );
    zlog_debug ( gngeox_config.loggingCat, "d2=%08x   d6=%08x   a2=%08x   a6=%08x",
                 regs.regs[2], regs.regs[6], regs.regs[10], regs.regs[14] );
    zlog_debug ( gngeox_config.loggingCat, "d3=%08x   d7=%08x   a3=%08x   a7=%08x   SP=%08x",
                 regs.regs[3], regs.regs[7], regs.regs[11], regs.regs[15], regs.sp );

}
/* ******************************************************************************************************************/
/*!
* \brief Disassembles 68k opcode beginning at program counter offset with printing.
*
* \param program_counter Program counter.
* \return Program counter after executing indicated instruction number.
*/
/* ******************************************************************************************************************/
static void gen68k_disassemble ( Sint32 program_counter )
{
    char buf[512];

    diss68k_getdumpline ( program_counter, buf );
    zlog_debug ( gngeox_config.loggingCat, "%s", buf );
}
/* ******************************************************************************************************************/
/*!
* \brief 68k instruction step by step.
*
*/
/* ******************************************************************************************************************/
static void cpu_68k_dbg_step ( void )
{
    static Uint32 current_slice = 0;

    if ( current_slice == 0 )
    {
        if ( neogeo_memory.test_switch == 1 )
        {
            neogeo_memory.test_switch = 0;
        }

        neo_sys_update_events();

        for ( Uint32 i = 0; i < EMU_NB_INTERLACE; i++ )
        {
            z80_run ( cpu_z80_timeslice_interlace, 0 );
            neo_ym2610_update();
        }

        current_line = 0;
    }

    reg68k_external_step();
    gen68k_disassemble ( cpu_68k_getpc() );

    if ( update_scanline() )
    {
        cpu_68k_interrupt ( 2 );
    }

    current_slice++;

    if ( current_slice == ( cpu_68k_timeslice_scanline * EMU_NB_SCANLINES_MVS ) )
    {
        neo_screen_update();
        neogeo_memory.watchdog++;

        if ( neogeo_memory.watchdog > 7 )
        {
            zlog_info ( gngeox_config.loggingCat, "Watchdog Reset" );
            cpu_68k_reset();
        }

        cpu_68k_interrupt ( 1 );

        current_slice = 0;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
* \param execstep Todo.
* \param dump Todo.
* \return Todo.
**/
/* ******************************************************************************************************************/
static Sint32 cpu_68k_debugger ( void )
{
    char buffer[50];
    char command = '0';
    Uint32 program_counter = 0;
    Uint32 instruction_counter = 0;

    bstring command_line = NULL;
    bstringList command_parse = NULL;
    Uint8 debug_end = 0;

    qsys_clrscr();
    printf ( "GnGeoX 68k live debugger v%d.%d\n\n"
             , GNGEOX_MAJOR
             , GNGEOX_MINOR );
    printf ( "Cartridge : %s [%s]\n"
             , neogeo_memory.rom.info.name
             , neogeo_memory.rom.info.longname );

    for ( Uint32 i = 0; i < REGION_MAX; i++ )
    {
        printf ( "->ROM type [%s] size : %d bytes\n\n", neo_rom_region_name[i], neogeo_memory.rom.rom_region[i].size );
    }

    do
    {
        printf ( "68k> " );
        fflush ( stdout );
        SDL_zero ( buffer );
        fgets ( buffer, 50, stdin );

        command_line = bfromcstr ( ( const char * ) buffer );
        btrimws ( command_line );

        if ( command_line->slen == 0 )
        {
            zlog_error ( gngeox_config.loggingCat, "Invalid debugger command" );
            bdestroy ( command_line );
            continue;
        }

        command_parse = bsplit ( command_line, ' ' );;
        bdestroy ( command_line );

        command = * ( command_parse->entry[DEBUG_COMMAND]->data );

        /* @todo (Tmesys#1#21/04/2024): Parameters checking. */
        switch ( command )
        {
        case ( '?' ) :
            {
                printf ( "Command list :\n"
                         "  a [address]           Add a break-point at [address]\n"
                         "  d [address]           Del break-point at [address]\n"
                         "  c                     Clear all break-points\n"
                         "  p                     Print break-points\n"
                         "  T                     Print back-trace\n"
                         "  R                     Run until break-point\n"
                         "  r                     Print register dump actual state\n"
                         "  t [number]            Trace through [number] instructions and stop\n"
                         "  u [address]           Disassemble code, starting at [address]\n"
                         "  q                     Quit\n" );
            }
            break;
        case ( 'a' ) :
            {
                program_counter = strtoul ( ( const char * ) command_parse->entry[DEBUG_PARAM1]->data, NULL, 0 );
                if ( neo_debug_break_point_add ( program_counter ) == SDL_TRUE )
                {
                    printf ( "OK\n" ) ;
                }
            }
            break;
        case ( 'd' ) :
            {
                program_counter = strtoul ( ( const char * ) command_parse->entry[DEBUG_PARAM1]->data, NULL, 0 );
                if ( neo_debug_break_point_delete ( program_counter ) == SDL_TRUE )
                {
                    printf ( "OK\n" ) ;
                }
            }
            break;
        case ( 'c' ) :
            {
                neo_debug_break_point_clear();
            }
            break;
        case ( 'p' ) :
            {
                neo_debug_break_point_print();
            }
            break;
        case ( 'T' ) :
            {
                neo_debug_back_trace_print();
            }
            break;
        case ( 'R' ) :
            {
                while ( neo_debug_break_point_check ( cpu_68k_getpc() ) != SDL_TRUE )
                {
                    neo_debug_back_trace_add ( cpu_68k_getpc() );
                    cpu_68k_dbg_step();
                }
            }
            break;
        case ( 'r' ) :
            {
                gen68k_dumpreg();
            }
            break;
        case ( 't' ) :
            {
                instruction_counter = strtoul ( ( const char * ) command_parse->entry[DEBUG_PARAM1]->data, NULL, 0 );
                while ( instruction_counter-- )
                {
                    neo_debug_back_trace_add ( cpu_68k_getpc() );
                    cpu_68k_dbg_step();
                }
            }
            break;
        /* @todo (Tmesys#1#21/04/2024): Should have end instruction parameter. */
        case ( 'u' ) :
            {
                program_counter = strtoul ( ( const char * ) command_parse->entry[DEBUG_PARAM1]->data, NULL, 0 );
                neo_debug_break_point_add ( program_counter );
                while ( neo_debug_break_point_check ( cpu_68k_getpc() ) != SDL_TRUE )
                {
                    cpu_68k_dbg_step();
                }
                neo_debug_break_point_delete ( program_counter );
            }
            break;
        case ( 'q' ) :
            {
                debug_end = 1;
            }
            break;
        default :
            {
                zlog_error ( gngeox_config.loggingCat, "Invalid debugger command" );
            }
            break;
        }

        bstrListDestroy ( command_parse );
    }
    while ( debug_end == 0 );

    return -1;
}
/* ******************************************************************************************************************/
/*!
* \brief Debug loop.
*
*/
/* ******************************************************************************************************************/
void neo_debug_loop ( void )
{
    SDL_MinimizeWindow ( sdl_window );

    backtrace = qvector ( GNGEOXDEBUG_MAX_BACK_TRACE, sizeof ( Uint32 ), QVECTOR_THREADSAFE | QVECTOR_RESIZE_NONE );
    if ( backtrace == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Can not allocate back-trace" );
        return;
    }

    breakpoints = qvector ( GNGEOXDEBUG_MAX_BREAK_POINT, sizeof ( Uint32 ), QVECTOR_THREADSAFE | QVECTOR_RESIZE_NONE );
    if ( breakpoints == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Can not allocate back-trace" );
        return;
    }

    while ( cpu_68k_debugger ( ) != -1 );

    qvector_free ( backtrace );
    qvector_free ( breakpoints );
}

#ifdef _GNGEOX_DEBUG_C_
#undef _GNGEOX_DEBUG_C_
#endif // _GNGEOX_DEBUG_C_
