/*!
*
*   \file    GnGeoXdebug.c
*   \brief   68K debugging routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
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

#include "3rdParty/Generator68k/generator.h"
#include "3rdParty/Generator68k/cpu68k.h"
#include "3rdParty/Generator68k/reg68k.h"
#include "3rdParty/Generator68k/mem68k.h"
#include "GnGeoXdebug.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXconfig.h"
#include "GnGeoX68k.h"
#include "GnGeoXscreen.h"
#include "GnGeoXscanline.h"
#include "GnGeoXemu.h"

/* TODO: finish it ...... */

/* @fixme (Tmesys#1#12/04/2022): Prototype should be in some gen68k library header */
Sint32 diss68k_getdumpline ( Uint32 addr68k, Uint8* addr, char* dumpline );

static Sint32 gngeoxdebug_debug_step = 0;
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
* \param nb_instr Number of instructions to disassemble.
* \return Program counter after executing indicated instruction number.
*/
/* ******************************************************************************************************************/
static Uint32 gen68k_disassemble ( Sint32 program_counter, Sint32 nb_instr )
{
    char buf[512];
    Uint8* addr = NULL;

    for ( Sint32 index = 0; index < nb_instr; index++ )
    {
        addr = mem68k_memptr[program_counter >> 12] ( program_counter );
        program_counter += diss68k_getdumpline ( program_counter, addr, buf ) * 2;
        zlog_debug ( gngeox_config.loggingCat, "%s", buf );
    }

    return ( program_counter );
}
/* ******************************************************************************************************************/
/*!
* \brief Dumps 68k internal registers with printing.
*
*/
/* ******************************************************************************************************************/
static void gen68k_dumpreg ( void )
{
    zlog_debug ( gngeox_config.loggingCat, "d0=%08x   d4=%08x   a0=%08x   a4=%08x   %c%c%c%c%c %04x\n",
                 regs.regs[0], regs.regs[4], regs.regs[8], regs.regs[12],
                 ( ( regs.sr.sr_int >> 4 ) & 1 ? 'X' : '-' ),
                 ( ( regs.sr.sr_int >> 3 ) & 1 ? 'N' : '-' ),
                 ( ( regs.sr.sr_int >> 2 ) & 1 ? 'Z' : '-' ),
                 ( ( regs.sr.sr_int >> 1 ) & 1 ? 'V' : '-' ),
                 ( ( regs.sr.sr_int ) & 1 ? 'C' : '-' ), regs.sr.sr_int );
    zlog_debug ( gngeox_config.loggingCat, "d1=%08x   d5=%08x   a1=%08x   a5=%08x\n",
                 regs.regs[1], regs.regs[5], regs.regs[9], regs.regs[13] );
    zlog_debug ( gngeox_config.loggingCat, "d2=%08x   d6=%08x   a2=%08x   a6=%08x\n",
                 regs.regs[2], regs.regs[6], regs.regs[10], regs.regs[14] );
    zlog_debug ( gngeox_config.loggingCat, "d3=%08x   d7=%08x   a3=%08x   a7=%08x   usp=%08x\n",
                 regs.regs[3], regs.regs[7], regs.regs[11], regs.regs[15], regs.sp );

}
/* ******************************************************************************************************************/
/*!
* \brief Debugs 68k step by step.
*
*/
/* ******************************************************************************************************************/
void cpu_68k_dpg_step ( void )
{
    static Uint32 nb_cycle = 0;
    static Uint32 line_cycle = 0;
    Uint32 cpu_68k_timeslice = 200000;
    Uint32 cpu_68k_timeslice_scanline = 200000 / ( float ) 262;
    Uint32 cycle = 0;

    if ( nb_cycle == 0 )
    {
        /* update event etc. */
        neo_sys_main_loop();
    }

    cycle = cpu_68k_run_step();
    add_bt ( cpu_68k_getpc() );
    line_cycle += cycle;
    nb_cycle += cycle;

    if ( nb_cycle >= cpu_68k_timeslice )
    {
        nb_cycle = line_cycle = 0;

        if ( gngeox_config.raster )
        {
            update_screen();
        }
        else
        {
            neo_sys_interrupt();
        }

        //state_handling(pending_save_state, pending_load_state);
        cpu_68k_interrupt ( 1 );
    }
    else
    {
        if ( line_cycle >= cpu_68k_timeslice_scanline )
        {
            line_cycle = 0;

            if ( gngeox_config.raster )
            {
                if ( update_scanline() )
                {
                    cpu_68k_interrupt ( 2 );
                }
            }
        }
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
static Sint32 cpu_68k_debuger ( void ( *execstep ) ( void ), void ( *dump ) ( void ) )
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
                printf ( "B [address]           Add a breakpoint at [address]\n"
                         "T                     Show backtrace\n"
                         "N [address]           Del breakpoint at [address]\n"
                         "R                     Run until breakpoint\n"
                         "b [address]           Run continuously, break at PC=[address]\n"
                         "d [address]           Dump memory, starting at [address]\n"
                         "h                     Hardware dump\n"
                         "i [number]            Generate hardware interrupt [number]\n"
                         "j [address]           Jump directly to [address]\n"
                         "q                     Quit\n"
                         "r                     Show register dump and next instruction\n"
                         "t [hex number]        Trace through [hex number] instructions\n"
                         "u [address]           Disassemble code, starting at [address]\n" );
            }
            break;

        case ( 'T' ) :
            {
                show_bt();
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

        case ( 'R' ) :
            {
                while ( check_bp ( cpu_68k_getpc() ) != SDL_TRUE && gngeoxdebug_debug_step == 0 )
                {
                    cpu_68k_dpg_step();
                }

                if ( gngeoxdebug_debug_step )
                {
                    gngeoxdebug_debug_step = 0;
                }

                gen68k_dumpreg();
                gen68k_disassemble ( regs.pc, 1 );
            }
            break;

        case ( 'b' ) :
            {
                if ( args )
                {
                    pc = strtoul ( args, &argsend, 0 );

                    if ( args != argsend )
                    {
                        cpu_68k_dpg_step(); /* trace 1 */

                        while ( cpu_68k_getpc() != pc && gngeoxdebug_debug_step == 0 )
                        {
                            cpu_68k_dpg_step();
                            //if (regs.regs[8]==0xf0f0f0f0) gngeoxdebug_debug_step=1;
                        }

                        if ( gngeoxdebug_debug_step )
                        {
                            gngeoxdebug_debug_step = 0;
                        }

                        gen68k_dumpreg();
                        gen68k_disassemble ( regs.pc, 1 );

                    }
                    else
                    {
                        zlog_error ( gngeox_config.loggingCat, "Invalid input" );
                    }
                }
            }
            break;

        case ( 'j' ) :
            {
                if ( args )
                {
                    pc = strtoul ( args, &argsend, 0 );

                    if ( args != argsend )
                    {
                        regs.pc = pc;
                    }
                    else
                    {
                        zlog_error ( gngeox_config.loggingCat, "Invalid input" );
                    }
                }
            }
            break;

        case ( 'r' ) :
            {
                gen68k_dumpreg();
                gen68k_disassemble ( regs.pc, 1 );
            }
            break;

        case ( 't' ) :
            {
                if ( args )
                {
                    pc = strtoul ( args, &argsend, 0 );

                    if ( args != argsend )
                    {
                        for ( i = 0; i < pc && gngeoxdebug_debug_step == 0; i++ )
                        {
                            cpu_68k_dpg_step();
                        }

                        if ( gngeoxdebug_debug_step )
                        {
                            gngeoxdebug_debug_step = 0;
                        }

                        gen68k_dumpreg();
                        gen68k_disassemble ( regs.pc, 1 );

                    }
                    else
                    {
                        zlog_error ( gngeox_config.loggingCat, "Invalid input" );
                    }
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

        case ( 'u' ) :
            {
                if ( args )
                {
                    pc = strtoul ( args, &argsend, 0 );

                    if ( args != argsend )
                    {
                        asmpc = pc;
                    }

                    asmpc = gen68k_disassemble ( asmpc, 16 );
                }
            }
            break;

        case ( 'h' ) :
            {
                //dump_hardware_reg();
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
void debug_loop ( void )
{
    Sint32 a;

    do
    {
        a = cpu_68k_debuger ( cpu_68k_dpg_step, /*dump_hardware_reg*/NULL );
    }
    while ( a != -1 );
}

/*
void debug_interf ( void )
{
    char in_buf[256];
    char val[32];
    char cmd;
    Sint32 in_a, in_b, in_c;
    Sint32 i, j;

    while ( 1 )
    {
        printf ( "> " );
        fflush ( stdout );
        //    in_buf[0]=0;
        // memset(in_buf,0,255);
        //scanf("%s %s",in_buf);
        fgets ( in_buf, 255, stdin );

        switch ( in_buf[0] )
        {
            case 'q':
            case 'c':
                gngeoxdebug_debug_step = 0;
                return;
                break;

            case 's':
                gngeoxdebug_debug_step = 1;
                return;

            case 'b':
                sscanf ( in_buf, "%c %x", &cmd, &in_a );
                add_bp ( in_a );
                break;

            case 'B':
                sscanf ( in_buf, "%c %x", &cmd, &in_a );
                del_bp ( in_a );
                break;

            case 'd':
                sscanf ( in_buf, "%c %s", &cmd, val );
                in_a = strtol ( val, NULL, 0 );
                cpu_68k_disassemble ( in_a, 10 );
                break;

            case 'p':
                cpu_68k_dumpreg();
                break;
        }

        printf ( "\n" );
    }
}

Sint32 dbg_68k_run ( Uint32 nbcycle )
{
    Sint32 i = 0;

    while ( i < nbcycle )
    {
        if ( check_bp ( cpu_68k_getpc() ) )
        {
            cpu_68k_disassemble ( cpu_68k_getpc(), 1 );
            debug_interf();
        }

        i += cpu_68k_run_step();
    }

    return i;
}
*/
#ifdef _GNGEOX_DEBUG_C_
#undef _GNGEOX_DEBUG_C_
#endif // _GNGEOX_DEBUG_C_
