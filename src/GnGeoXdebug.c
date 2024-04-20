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
//#include "GnGeoXscreen.h"
#include "GnGeoXscanline.h"
#include "GnGeoXemu.h"

/* TODO: finish it ...... */

static Sint32 nb_breakpoints = 0;
static Sint32 breakpoints[GNGEOXDEBUG_MAX_BREAK_POINT];
static Uint32 backtrace[GNGEOXDEBUG_MAX_BACK_TRACE];
static Uint32 pc = 0;

/* ******************************************************************************************************************/
/*!
* \brief  Adds a back trace.
*
* \param  program_counter Program counter.
*/
/* ******************************************************************************************************************/
void add_bt ( Uint32 program_counter )
{
    for ( Sint32 index = ( GNGEOXDEBUG_MAX_BACK_TRACE - 2 ); index >= 0; index-- )
    {
        backtrace[index + 1] = backtrace[index];
    }

    backtrace[0] = program_counter;
}
/* ******************************************************************************************************************/
/*!
* \brief  Shows back traces.
*
*/
/* ******************************************************************************************************************/
void show_bt ( void )
{
    for ( Sint32 index = GNGEOXDEBUG_MAX_BACK_TRACE - 1; index >= 0; index-- )
    {
        zlog_info ( gngeox_config.loggingCat, "%08x\n", backtrace[index] );
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
void add_cond ( Uint8 type, Sint32 reg, Uint32 value )
{
    /* @todo (Tmesys#1#12/04/2022): Conditions to what ?? Should be implemented */
}
/* ******************************************************************************************************************/
/*!
* \brief  Checks if program counter value is a break point.
*
* \param  program_counter Program counter.
* \return SDL_TRUE when program counter has a break point, SDL_FALSE otherwise.
*/
/* ******************************************************************************************************************/
SDL_bool check_bp ( Sint32 program_counter )
{
    for ( Sint32 index = 0; index < nb_breakpoints; index++ )
    {
        if ( breakpoints[index] == program_counter )
        {
            return ( SDL_TRUE );
        }
    }

    return ( SDL_FALSE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Adds break points condition.
*
* \param  program_counter Program counter.
*/
/* ******************************************************************************************************************/
void add_bp ( Sint32 program_counter )
{
    if ( nb_breakpoints > GNGEOXDEBUG_MAX_BREAK_POINT )
    {
        zlog_error ( gngeox_config.loggingCat, "Too many breakpoints\n" );
        return;
    }

    breakpoints[nb_breakpoints++] = program_counter;
}
/* ******************************************************************************************************************/
/*!
* \brief  Deletes break point.
*
* \param  program_counter Program counter.
*/
/* ******************************************************************************************************************/
void del_bp ( Sint32 program_counter )
{
    for ( Sint32 index = 0; index < nb_breakpoints; index++ )
    {
        if ( breakpoints[index] == program_counter )
        {
            breakpoints[index] = -1;
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Clears all break points.
*
*/
/* ******************************************************************************************************************/
void clear_bps ( void )
{
    for ( Sint32 index = 0; index < nb_breakpoints; index++ )
    {
        breakpoints[index] = -1;
    }

    nb_breakpoints = 0;
}
/* ******************************************************************************************************************/
/*!
* \brief Dumps 68k internal registers with printing.
*
*/
/* ******************************************************************************************************************/
static void gen68k_dumpreg ( void )
{
    zlog_debug ( gngeox_config.loggingCat, "d0=%08x   d4=%08x   a0=%08x   a4=%08x   %c%c%c%c%c %04x",
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
    zlog_debug ( gngeox_config.loggingCat, "d3=%08x   d7=%08x   a3=%08x   a7=%08x   usp=%08x",
                 regs.regs[3], regs.regs[7], regs.regs[11], regs.regs[15], regs.sp );

}
/* ******************************************************************************************************************/
/*!
* \brief Dumps 68k internal registers with printing.
*
* \param program_counter Program counter.
**/
/* ******************************************************************************************************************/
static void hexdump ( Uint32 program_counter )
{
    Uint8 c, tmpchar[16];
    Uint32 tmpaddr;
    Sint32 i, j, k;
    tmpaddr = program_counter & 0xFFFFFFF0;

    for ( i = 0; i < 8; i++ )
    {
        zlog_debug ( gngeox_config.loggingCat, "%08X: %c", tmpaddr, ( program_counter == tmpaddr ) ? '>' : ' ' );

        for ( j = 0; j < 16; j += 2 )
        {
            k = fetchword ( tmpaddr ) & 0xFFFF;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            tmpchar[j + 1] = k >> 8;
            tmpchar[j    ] = k & 0xFF;
#else
            tmpchar[j    ] = k >> 8;
            tmpchar[j + 1] = k & 0xFF;
#endif
            tmpaddr += 2;
            zlog_debug ( gngeox_config.loggingCat, "%02X%02X%c",
                         tmpchar[j], tmpchar[j + 1],
                         ( ( program_counter            == tmpaddr ) && ( j != 14 ) ) ? '>' :
                         ( ( ( program_counter & 0xFFFFFFFE ) == ( tmpaddr - 2 ) ) ? '<' : ' ' )
                       );
        }

        for ( j = 0; j < 16; j++ )
        {
            c = tmpchar[j];

            if ( ( c < 32 ) || ( c > 126 ) )
            {
                c = '.';
            }

            printf ( "%c", c );
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Disassembles 68k opcode beginning at program counter offset with printing.
*
* \param program_counter Program counter.
* \return Program counter after executing indicated instruction number.
*/
/* ******************************************************************************************************************/
static Uint32 gen68k_disassemble ( Sint32 program_counter )
{
    char buf[512];

    diss68k_getdumpline ( program_counter, buf );
    zlog_debug ( gngeox_config.loggingCat, "%s", buf );

    return ( program_counter );
}
/* ******************************************************************************************************************/
/*!
* \brief Main loop for debugging
*
*/
/* ******************************************************************************************************************/
static void cpu_68k_dbg_step_68k ( void )
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
    char buf[200];
    char* args = NULL, *argsend = NULL;
    Uint8 debug_end = 0;
    Uint32 hex = 0;
    Uint32 asmpc = 0;
    Sint32 i = 0;

    do
    {
        printf ( "cpu1> " );
        fflush ( stdout );
        fgets ( buf, 200, stdin );

        args = buf + 1;

        while ( ( *args ) && ( ( *args ) < 32 ) )
        {
            args++;
        }

        switch ( buf[0] )
        {
        case ( '?' ) :
            {
                printf ( "B [address]           Add a break-point at [address]\n"
                         "N [address]           Del break-point at [address]\n"
                         "T                     Show back-trace\n"
                         "R                     Run until break-point\n"
                         "d [address]           Dump memory, starting at [address]\n"
                         "r                     Show register dump and next instruction\n"
                         "t [hex number]        Trace through [hex number] instructions\n"
                         "u [address]           Disassemble code, starting at [address]\n"
                         "q                     Quit\n" );
            }
            break;
        case ( 'B' ) :
            {
                if ( args )
                {
                    pc = strtoul ( args, &argsend, 0 );

                    if ( args != argsend )
                    {
                        add_bp ( pc );
                    }
                    else
                    {
                        zlog_error ( gngeox_config.loggingCat, "Invalid input" );
                    }
                }
            }
            break;
        case ( 'N' ) :
            {
                if ( args )
                {
                    pc = strtoul ( args, &argsend, 0 );

                    if ( args != argsend )
                    {
                        del_bp ( pc );
                    }
                    else
                    {
                        zlog_error ( gngeox_config.loggingCat, "Invalid input" );
                    }
                }
            }
            break;
        case ( 'T' ) :
            {
                show_bt();
            }
            break;
        case ( 'R' ) :
            {
                while ( check_bp ( cpu_68k_getpc() ) != SDL_TRUE )
                {
                    cpu_68k_dbg_step_68k();
                }
            }
            break;
        case ( 'd' ) :
            {
                if ( args )
                {
                    pc = strtoul ( args, &argsend, 0 );

                    if ( args != argsend )
                    {
                        hex = pc;
                    }

                    hexdump ( hex );
                    hex += 0x80;
                }
            }
            break;
        case ( 'r' ) :
            {
                gen68k_dumpreg();
                gen68k_disassemble ( regs.pc );
            }
            break;
        case ( 't' ) :
            {
                if ( args )
                {
                    pc = strtoul ( args, &argsend, 0 );

                    if ( args != argsend )
                    {
                        for ( i = 0; i < pc; i++ )
                        {
                            cpu_68k_dbg_step_68k();
                        }

                        gen68k_dumpreg();
                        gen68k_disassemble ( regs.pc );
                    }
                    else
                    {
                        zlog_error ( gngeox_config.loggingCat, "Invalid input" );
                    }
                }
            }
            break;
        case ( 'u' ) :
            {
                if ( args )
                {
                    pc = strtoul ( args, &argsend, 0 );

                    if ( args != argsend )
                    {
                        asmpc = pc;
                    }

                    /* @fixme (Tmesys#1#20/04/2024): Before disassembling was for 16 instruction. */
                    asmpc = gen68k_disassemble ( asmpc );
                }
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
void neo_sys_debug_loop ( void )
{
    Sint32 a;

    do
    {
        a = cpu_68k_debugger ( );
    }
    while ( a != -1 );
}

#ifdef _GNGEOX_DEBUG_C_
#undef _GNGEOX_DEBUG_C_
#endif // _GNGEOX_DEBUG_C_
