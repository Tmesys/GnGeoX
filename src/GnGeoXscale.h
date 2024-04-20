/*!
*
*   \file    GnGeoXscale.h
*   \brief   Scale image effect routines header http://scale2x.sourceforge.net/.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 04.00
*   \date    03/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    This effect is a rewritten implementation of the scale effect made by Andrea Mazzoleni.
*/
#ifndef _GNGEOX_SCALE_H_
#define _GNGEOX_SCALE_H_

#ifdef _GNGEOX_SCALE_C_
static void scale2x_32_def_single ( Uint32* dst, const Uint32* src0, const Uint32* src1, const Uint32* src2, Uint32 count );
static void scale2x_32_def ( Uint32* dst0, Uint32* dst1, const Uint32* src0, const Uint32* src1, const Uint32* src2, Uint32 count );
static void scale3x_32_def_single ( Uint32* dst, const Uint32* src0, const Uint32* src1, const Uint32* src2, Uint32 count );
static void scale3x_32_def_fill ( Uint32* dst, const Uint32* src, Uint32 count );
static void scale3x_32_def ( Uint32* dst0, Uint32* dst1, Uint32* dst2, const Uint32* src0, const Uint32* src1, const Uint32* src2, Uint32 count );
#endif // _GNGEOX_SCALE_C_

SDL_bool effect_scale2x_init ( void ) __attribute__ ( ( warn_unused_result ) );
SDL_bool effect_scale3x_init ( void ) __attribute__ ( ( warn_unused_result ) );
SDL_bool effect_scale4x_init ( void ) __attribute__ ( ( warn_unused_result ) );
void effect_scale2x_update ( void );
void effect_scale4x_update ( void );
void effect_scale3x_update ( void );
#endif // _GNGEOX_SCALE_H_
