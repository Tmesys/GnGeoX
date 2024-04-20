/*!
*
*   \file    GnGeoXemu.h
*   \brief   Emulation routines header ?
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version).
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X).
*   \version 01.00
*   \date    02/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_EMU_H_
#define _GNGEOX_EMU_H_

#define EMU_NB_INTERLACE          256
#define EMU_NB_SCANLINES_MVS      264
#define EMU_NB_SCANLINES_AES_PAL  312
#define EMU_NB_SCANLINES_AES_NTSC 264

#ifdef _GNGEOX_EMU_C_
static void neo_sys_reset ( void );
#else
extern Uint32 cpu_z80_timeslice_interlace;
extern Uint32 cpu_68k_timeslice_scanline;
#endif // _GNGEOX_EMU_C_

SDL_bool neo_sys_init ( void )  __attribute__ ( ( warn_unused_result ) );
void neo_sys_main_loop ( void );
void neo_sys_interrupt ( void );
void neo_sys_update_events ( void );

#endif
