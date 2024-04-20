/*!
*
*   \file    GnGeoXxbr2x.c
*   \brief   xbr2x image effect routines https://en.wikipedia.org//wiki/Hqx.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    11/04/2024
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_XBR2X_C_
#define _GNGEOX_XBR2X_C_
#endif // _GNGEOX_XBR2X_C_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "GnGeoXscreen.h"
#include "GnGeoXvideo.h"
#include "GnGeoXinterp.h"
#include "GnGeoXxbr2x.h"

/* ******************************************************************************************************************/
/*!
* \brief  Applies xbr2x effect.
*
* \param dst0 Todo.
* \param dst1 Todo.
* \param src0 Todo.
* \param src1 Todo.
* \param src2 Todo.
* \param count Todo.
*/
/* ******************************************************************************************************************/
static void xbr2x_32_def ( Uint32* restrict volatile dst0, Uint32* restrict volatile dst1, const Uint32* restrict src0, const Uint32* restrict src1, const Uint32* restrict src2, const Uint32* restrict src3, const Uint32* restrict src4, unsigned count )
{
    unsigned i;

    for ( i = 0; i < count; ++i )
    {
        Uint32 PA, PB, PC, PD, PE, PF, PG, PH, xPI;
        Uint32 A0, D0, G0, A1, B1, C1, C4, F4, I4, G5, H5, I5;
        Uint32 E[4];

        /* first two columns */
        if ( i > 1 )
        {
            A1 = src0[-1];
            PA = src1[-1];
            PD = src2[-1];
            PG = src3[-1];
            G5 = src4[-1];

            A0 = src1[-2];
            D0 = src2[-2];
            G0 = src3[-2];
        }
        else if ( i > 0 )
        {
            A1 = src0[-1];
            PA = src1[-1];
            PD = src2[-1];
            PG = src3[-1];
            G5 = src4[-1];

            A0 = src1[-1];
            D0 = src2[-1];
            G0 = src3[-1];
        }
        else
        {
            A1 = src0[0];
            PA = src1[0];
            PD = src2[0];
            PG = src3[0];
            G5 = src4[0];

            A0 = src1[0];
            D0 = src2[0];
            G0 = src3[0];
        }

        /* central */
        B1 = src0[0];
        PB = src1[0];
        PE = src2[0];
        PH = src3[0];
        H5 = src4[0];

        /* last two columns */
        if ( i + 2 < count )
        {
            C1 = src0[1];
            PC = src1[1];
            PF = src2[1];
            xPI = src3[1];
            I5 = src4[1];

            C4 = src1[2];
            F4 = src2[2];
            I4 = src3[2];
        }
        else if ( i + 1 < count )
        {
            C1 = src0[1];
            PC = src1[1];
            PF = src2[1];
            xPI = src3[1];
            I5 = src4[1];

            C4 = src1[1];
            F4 = src2[1];
            I4 = src3[1];
        }
        else
        {
            C1 = src0[0];
            PC = src1[0];
            PF = src2[0];
            xPI = src3[0];
            I5 = src4[0];

            C4 = src1[0];
            F4 = src2[0];
            I4 = src3[0];
        }

        /* default pixels */
        E[0] = PE;
        E[1] = PE;
        E[2] = PE;
        E[3] = PE;

        XBR ( Uint32, PE, xPI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, 0, 1, 2, 3 );
        XBR ( Uint32, PE, PC, PF, PB, xPI, PA, PH, PD, PG, I4, A1, I5, H5, A0, D0, B1, C1, F4, C4, G5, G0, 2, 0, 3, 1 );
        XBR ( Uint32, PE, PA, PB, PD, PC, PG, PF, PH, xPI, C1, G0, C4, F4, G5, H5, D0, A0, B1, A1, I4, I5, 3, 2, 1, 0 );
        XBR ( Uint32, PE, PG, PD, PH, PA, xPI, PB, PF, PC, A0, I5, A1, B1, I4, F4, H5, G5, D0, G0, C1, C4, 1, 3, 0, 2 );

        /* copy resulting pixel into dst */
        dst0[0] = E[0];
        dst0[1] = E[1];
        dst1[0] = E[2];
        dst1[1] = E[3];

        src0 += 1;
        src1 += 1;
        src2 += 1;
        src3 += 1;
        src4 += 1;
        dst0 += 2;
        dst1 += 2;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes xbr2x effect.
*
* \return Always SDL_TRUE.
*/
/* ******************************************************************************************************************/
SDL_bool effect_xbr2x_init ( void )
{
    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Updates xbr2x effect.
*
*/
/* ******************************************************************************************************************/
void effect_xbr2x_update ( void )
{
    Uint32* dst0 = NULL, *dst1 = NULL, *src0 = NULL, *src1 = NULL, *src2 = NULL, *src3 = NULL, *src4 = NULL;
    Uint32 height = 0;

    height = visible_area.h;

    dst0 = ( Uint32* ) sdl_surface_screen->pixels + yscreenpadding;
    dst1 = ( Uint32* ) dst0 + ( visible_area.w * 2 );

    src0 = ( Uint32* ) sdl_surface_buffer->pixels + 352 * visible_area.y + visible_area.x;
    src1 = ( Uint32* ) src0 + 352;
    src2 = ( Uint32* ) src1 + 352;
    src3 = ( Uint32* ) src2 + 352;
    src4 = ( Uint32* ) src3 + 352;

    SDL_LockSurface ( sdl_surface_buffer );
    SDL_LockSurface ( sdl_surface_screen );

    while ( height-- )
    {

        xbr2x_32_def ( dst0, dst1, src0, src1, src2, src3, src4, visible_area.w );

        dst0 += ( visible_area.w * 4 );
        dst1 += ( visible_area.w * 4 );

        src0 += 352;
        src1 += 352;
        src2 += 352;
        src3 += 352;
        src4 += 352;
    }

    SDL_UnlockSurface ( sdl_surface_buffer );
    SDL_UnlockSurface ( sdl_surface_screen );
}

#ifdef _GNGEOX_XBR2X_C_
#undef _GNGEOX_XBR2X_C_
#endif // _GNGEOX_XBR2X_C_
