/*!
*
*   \file    GnGeoXdebug.h
*   \brief   68K debugging routines header.
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
#ifndef _GNGEOX_DEBUG_H_
#define _GNGEOX_DEBUG_H_

#define GNGEOXDEBUG_MAX_BREAK_POINT 500
#define GNGEOXDEBUG_MAX_BACK_TRACE 20

enum
{
    DEBUG_COMMAND = 0,
    DEBUG_PARAM1 = 1,
    DEBUG_PARAM2 = 2,
};

#ifdef _GNGEOX_DEBUG_C_
static void gen68k_disassemble ( Sint32 );
static void gen68k_dumpreg ( void );
static Sint32 cpu_68k_debugger ( void );
static void cpu_68k_dbg_step ( void );

static SDL_bool neo_debug_back_trace_add ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static void neo_debug_back_trace_print ( void );
static SDL_bool neo_debug_break_point_check ( Sint32 ) __attribute__ ( ( warn_unused_result ) );
static SDL_bool neo_debug_break_point_add ( Sint32 ) __attribute__ ( ( warn_unused_result ) );
static SDL_bool neo_debug_break_point_delete ( Sint32 ) __attribute__ ( ( warn_unused_result ) );
static void neo_debug_break_point_clear ( void );
static void neo_debug_break_point_print ( void );
static void add_cond ( Uint8, Sint32, Uint32 );
#endif // _GNGEOX_DEBUG_C_

void neo_debug_loop ( void );

#endif // _GNGEOX_DEBUG_H_
