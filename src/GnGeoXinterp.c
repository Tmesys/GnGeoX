/*!
*
*   \file    GnGeoXinterp.c
*   \brief   Interpolation image effect routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    03/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_INTERP_C_
#define _GNGEOX_INTERP_C_
#endif // _GNGEOX_INTERP_C_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "GnGeoXscreen.h"
#include "GnGeoXinterp.h"

Uint32 interp_32_1411 ( Uint32 p1, Uint32 p2, Uint32 p3 )
{
    return INTERP_32_UNMASK_1 ( ( INTERP_32_MASK_1 ( p1 ) * 14 + INTERP_32_MASK_1 ( p2 ) * 1 + INTERP_32_MASK_1 ( p3 ) * 1 ) / 16 )
           | INTERP_32_UNMASK_2 ( ( INTERP_32_MASK_2 ( p1 ) * 14 + INTERP_32_MASK_2 ( p2 ) * 1 + INTERP_32_MASK_2 ( p3 ) * 1 ) / 16 );
}

Uint32 interp_32_11 ( Uint32 p1, Uint32 p2 )
{
#ifdef USE_INTERP_MASK_1
    return INTERP_32_UNMASK_1 ( ( INTERP_32_MASK_1 ( p1 ) + INTERP_32_MASK_1 ( p2 ) ) / 2 )
           | INTERP_32_UNMASK_2 ( ( INTERP_32_MASK_2 ( p1 ) + INTERP_32_MASK_2 ( p2 ) ) / 2 );
#else
    /*
     * This function compute (a + b) / 2 for any rgb nibble, using the
     * the formula (a + b) / 2 = ((a ^ b) >> 1) + (a & b).
     * To extend this formula to a serie of packed nibbles the formula is
     * implemented as (((v0 ^ v1) >> 1) & MASK) + (v0 & v1) where MASK
     * is used to clear the high bit of all the packed nibbles.
     */
    return ( ( ( p1 ^ p2 ) >> 1 ) & INTERP_32_HNMASK ) + ( p1 & p2 );
#endif
}

Uint32 interp_32_611 ( Uint32 p1, Uint32 p2, Uint32 p3 )
{
#ifdef USE_INTERP_MASK_3
    return INTERP_32_UNMASK_1 ( ( INTERP_32_MASK_1 ( p1 ) * 6 + INTERP_32_MASK_1 ( p2 ) + INTERP_32_MASK_1 ( p3 ) ) / 8 )
           | INTERP_32_UNMASK_2 ( ( INTERP_32_MASK_2 ( p1 ) * 6 + INTERP_32_MASK_2 ( p2 ) + INTERP_32_MASK_2 ( p3 ) ) / 8 );
#else
    return interp_32_11 ( p1, interp_32_11 ( p1, interp_32_11 ( p2, p3 ) ) );
#endif
}

Uint32 interp_32_332 ( Uint32 p1, Uint32 p2, Uint32 p3 )
{
#ifdef USE_INTERP_MASK_3
    return INTERP_32_UNMASK_1 ( ( INTERP_32_MASK_1 ( p1 ) * 3 + INTERP_32_MASK_1 ( p2 ) * 3 + INTERP_32_MASK_1 ( p3 ) * 2 ) / 8 )
           | INTERP_32_UNMASK_2 ( ( INTERP_32_MASK_2 ( p1 ) * 3 + INTERP_32_MASK_2 ( p2 ) * 3 + INTERP_32_MASK_2 ( p3 ) * 2 ) / 8 );
#else
    Uint32 t = interp_32_11 ( p1, p2 );
    return interp_32_11 ( t, interp_32_11 ( p3, t ) );
#endif
}

Uint32 interp_32_521 ( Uint32 p1, Uint32 p2, Uint32 p3 )
{
#ifdef USE_INTERP_MASK_3
    return INTERP_32_UNMASK_1 ( ( INTERP_32_MASK_1 ( p1 ) * 5 + INTERP_32_MASK_1 ( p2 ) * 2 + INTERP_32_MASK_1 ( p3 ) ) / 8 )
           | INTERP_32_UNMASK_2 ( ( INTERP_32_MASK_2 ( p1 ) * 5 + INTERP_32_MASK_2 ( p2 ) * 2 + INTERP_32_MASK_2 ( p3 ) ) / 8 );
#else
    return interp_32_11 ( p1, interp_32_11 ( p2, interp_32_11 ( p1, p3 ) ) );
#endif
}

Uint32 interp_32_31 ( Uint32 p1, Uint32 p2 )
{
#ifdef USE_INTERP_MASK_2
    return INTERP_32_UNMASK_1 ( ( INTERP_32_MASK_1 ( p1 ) * 3 + INTERP_32_MASK_1 ( p2 ) ) / 4 )
           | INTERP_32_UNMASK_2 ( ( INTERP_32_MASK_2 ( p1 ) * 3 + INTERP_32_MASK_2 ( p2 ) ) / 4 );
#else
    return interp_32_11 ( p1, interp_32_11 ( p1, p2 ) );
#endif
}

Uint32 interp_32_211 ( Uint32 p1, Uint32 p2, Uint32 p3 )
{
#ifdef USE_INTERP_MASK_2
    return INTERP_32_UNMASK_1 ( ( INTERP_32_MASK_1 ( p1 ) * 2 + INTERP_32_MASK_1 ( p2 ) + INTERP_32_MASK_1 ( p3 ) ) / 4 )
           | INTERP_32_UNMASK_2 ( ( INTERP_32_MASK_2 ( p1 ) * 2 + INTERP_32_MASK_2 ( p2 ) + INTERP_32_MASK_2 ( p3 ) ) / 4 );
#else
    return interp_32_11 ( p1, interp_32_11 ( p2, p3 ) );
#endif
}

Uint32 interp_32_71 ( Uint32 p1, Uint32 p2 )
{
    return INTERP_32_UNMASK_1 ( ( INTERP_32_MASK_1 ( p1 ) * 7 + INTERP_32_MASK_1 ( p2 ) * 1 ) / 16 )
           | INTERP_32_UNMASK_2 ( ( INTERP_32_MASK_2 ( p1 ) * 7 + INTERP_32_MASK_2 ( p2 ) * 1 ) / 16 );
}

Uint32 interp_32_diff ( Uint32 p1, Uint32 p2 )
{
    int r, g, b;
    int y, u, v;
    int i1, i2;

    /* assume standard rgb formats */
    if ( ( p1 & 0xF8F8F8 ) == ( p2 & 0xF8F8F8 ) )
        return 0;

    i1 = p1;
    i2 = p2;

    b = ( ( i1 & 0xFF ) - ( i2 & 0xFF ) );
    g = ( ( i1 & 0xFF00 ) - ( i2 & 0xFF00 ) ) >> 8;
    r = ( ( i1 & 0xFF0000 ) - ( i2 & 0xFF0000 ) ) >> 16;

    /* not exact, but fast */
    y = r + g + b;
    u = r - b;
    v = -r + 2 * g - b;

    if ( y < -INTERP_Y_LIMIT_S2 || y > INTERP_Y_LIMIT_S2 )
        return 1;

    if ( u < -INTERP_U_LIMIT_S2 || u > INTERP_U_LIMIT_S2 )
        return 1;

    if ( v < -INTERP_V_LIMIT_S3 || v > INTERP_V_LIMIT_S3 )
        return 1;

    return 0;
}

Uint32 interp_32_dist ( Uint32 p1, Uint32 p2 )
{
    int r, g, b;
    int y, u, v;
    int i1, i2;

    /* assume standard rgb formats */
    if ( ( p1 & 0xF8F8F8 ) == ( p2 & 0xF8F8F8 ) )
        return 0;

    i1 = p1;
    i2 = p2;

    b = ( ( i1 & 0xFF ) - ( i2 & 0xFF ) );
    g = ( ( i1 & 0xFF00 ) - ( i2 & 0xFF00 ) ) >> 8;
    r = ( ( i1 & 0xFF0000 ) - ( i2 & 0xFF0000 ) ) >> 16;

    if ( b < 0 ) b = -b;
    if ( g < 0 ) g = -g;
    if ( r < 0 ) r = -r;

    return 3 * r + 4 * g + 2 * b;
}

Uint32 interp_32_dist3 ( Uint32 p1, Uint32 p2, Uint32 p3 )
{
    return interp_32_dist ( p1, p2 ) + interp_32_dist ( p2, p3 );
}

#ifdef _GNGEOX_INTERP_C_
#undef _GNGEOX_INTERP_C_
#endif // _GNGEOX_INTERP_C_
