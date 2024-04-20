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

#ifdef _GNGEOX_DEBUG_C_
static Uint32 gen68k_disassemble ( Sint32 ) __attribute__ ( ( warn_unused_result ) );
static void gen68k_dumpreg ( void );
static void hexdump ( Uint32 );
static Sint32 cpu_68k_debugger ( void );
static void cpu_68k_dbg_step_68k ( void );
#endif // _GNGEOX_DEBUG_C_

void add_bt ( Uint32 );
void show_bt ( void );
void add_cond ( Uint8, Sint32, Uint32 );
SDL_bool check_bp ( Sint32 ) __attribute__ ( ( warn_unused_result ) );
void add_bp ( Sint32 );
void del_bp ( Sint32 );
void clear_bps ( void );
void neo_sys_debug_loop ( void );

#endif // _GNGEOX_DEBUG_H_
