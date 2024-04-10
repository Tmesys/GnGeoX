/*!
*
*   \file    GnGeoXinterp.c
*   \brief   Interpolation image effect routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
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

/* ******************************************************************************************************************/
/*!
* \brief  Interpolate 16 521.
*
* \return Todo.
*/
/* ******************************************************************************************************************/
Uint16 interp_16_521 ( Uint16 p1, Uint16 p2, Uint16 p3 )
{
    return INTERP_16_UNMASK_1 ( ( INTERP_16_MASK_1 ( p1 ) * 5 + INTERP_16_MASK_1 ( p2 ) * 2 + INTERP_16_MASK_1 ( p3 ) * 1 ) / 8 )
           | INTERP_16_UNMASK_2 ( ( INTERP_16_MASK_2 ( p1 ) * 5 + INTERP_16_MASK_2 ( p2 ) * 2 + INTERP_16_MASK_2 ( p3 ) * 1 ) / 8 );
}

Uint16 interp_16_332 ( Uint16 p1, Uint16 p2, Uint16 p3 )
{
    return INTERP_16_UNMASK_1 ( ( INTERP_16_MASK_1 ( p1 ) * 3 + INTERP_16_MASK_1 ( p2 ) * 3 + INTERP_16_MASK_1 ( p3 ) * 2 ) / 8 )
           | INTERP_16_UNMASK_2 ( ( INTERP_16_MASK_2 ( p1 ) * 3 + INTERP_16_MASK_2 ( p2 ) * 3 + INTERP_16_MASK_2 ( p3 ) * 2 ) / 8 );
}

Uint16 interp_16_611 ( Uint16 p1, Uint16 p2, Uint16 p3 )
{
    return INTERP_16_UNMASK_1 ( ( INTERP_16_MASK_1 ( p1 ) * 6 + INTERP_16_MASK_1 ( p2 ) + INTERP_16_MASK_1 ( p3 ) ) / 8 )
           | INTERP_16_UNMASK_2 ( ( INTERP_16_MASK_2 ( p1 ) * 6 + INTERP_16_MASK_2 ( p2 ) + INTERP_16_MASK_2 ( p3 ) ) / 8 );
}

Uint16 interp_16_71 ( Uint16 p1, Uint16 p2 )
{
    return INTERP_16_UNMASK_1 ( ( INTERP_16_MASK_1 ( p1 ) * 7 + INTERP_16_MASK_1 ( p2 ) ) / 8 )
           | INTERP_16_UNMASK_2 ( ( INTERP_16_MASK_2 ( p1 ) * 7 + INTERP_16_MASK_2 ( p2 ) ) / 8 );
}

Uint16 interp_16_211 ( Uint16 p1, Uint16 p2, Uint16 p3 )
{
    return INTERP_16_UNMASK_1 ( ( INTERP_16_MASK_1 ( p1 ) * 2 + INTERP_16_MASK_1 ( p2 ) + INTERP_16_MASK_1 ( p3 ) ) / 4 )
           | INTERP_16_UNMASK_2 ( ( INTERP_16_MASK_2 ( p1 ) * 2 + INTERP_16_MASK_2 ( p2 ) + INTERP_16_MASK_2 ( p3 ) ) / 4 );
}

Uint16 interp_16_772 ( Uint16 p1, Uint16 p2, Uint16 p3 )
{
    return INTERP_16_UNMASK_1 ( ( ( INTERP_16_MASK_1 ( p1 ) + INTERP_16_MASK_1 ( p2 ) ) * 7 + INTERP_16_MASK_1 ( p3 ) * 2 ) / 16 )
           | INTERP_16_UNMASK_2 ( ( ( INTERP_16_MASK_2 ( p1 ) + INTERP_16_MASK_2 ( p2 ) ) * 7 + INTERP_16_MASK_2 ( p3 ) * 2 ) / 16 );
}

Uint16 interp_16_11 ( Uint16 p1, Uint16 p2 )
{
    return INTERP_16_UNMASK_1 ( ( INTERP_16_MASK_1 ( p1 ) + INTERP_16_MASK_1 ( p2 ) ) / 2 )
           | INTERP_16_UNMASK_2 ( ( INTERP_16_MASK_2 ( p1 ) + INTERP_16_MASK_2 ( p2 ) ) / 2 );
}

Uint16 interp_16_31 ( Uint16 p1, Uint16 p2 )
{
    return INTERP_16_UNMASK_1 ( ( INTERP_16_MASK_1 ( p1 ) * 3 + INTERP_16_MASK_1 ( p2 ) ) / 4 )
           | INTERP_16_UNMASK_2 ( ( INTERP_16_MASK_2 ( p1 ) * 3 + INTERP_16_MASK_2 ( p2 ) ) / 4 );
}

Uint16 interp_16_1411 ( Uint16 p1, Uint16 p2, Uint16 p3 )
{
    return INTERP_16_UNMASK_1 ( ( INTERP_16_MASK_1 ( p1 ) * 14 + INTERP_16_MASK_1 ( p2 ) + INTERP_16_MASK_1 ( p3 ) ) / 16 )
           | INTERP_16_UNMASK_2 ( ( INTERP_16_MASK_2 ( p1 ) * 14 + INTERP_16_MASK_2 ( p2 ) + INTERP_16_MASK_2 ( p3 ) ) / 16 );
}

Uint16 interp_16_431 ( Uint16 p1, Uint16 p2, Uint16 p3 )
{
    return INTERP_16_UNMASK_1 ( ( INTERP_16_MASK_1 ( p1 ) * 4 + INTERP_16_MASK_1 ( p2 ) * 3 + INTERP_16_MASK_1 ( p3 ) ) / 8 )
           | INTERP_16_UNMASK_2 ( ( INTERP_16_MASK_2 ( p1 ) * 4 + INTERP_16_MASK_2 ( p2 ) * 3 + INTERP_16_MASK_2 ( p3 ) ) / 8 );
}

Uint16 interp_16_53 ( Uint16 p1, Uint16 p2 )
{
    return INTERP_16_UNMASK_1 ( ( INTERP_16_MASK_1 ( p1 ) * 5 + INTERP_16_MASK_1 ( p2 ) * 3 ) / 8 )
           | INTERP_16_UNMASK_2 ( ( INTERP_16_MASK_2 ( p1 ) * 5 + INTERP_16_MASK_2 ( p2 ) * 3 ) / 8 );
}

Uint16 interp_16_151 ( Uint16 p1, Uint16 p2 )
{
    return INTERP_16_UNMASK_1 ( ( INTERP_16_MASK_1 ( p1 ) * 15 + INTERP_16_MASK_1 ( p2 ) ) / 16 )
           | INTERP_16_UNMASK_2 ( ( INTERP_16_MASK_2 ( p1 ) * 15 + INTERP_16_MASK_2 ( p2 ) ) / 16 );
}

Uint16 interp_16_97 ( Uint16 p1, Uint16 p2 )
{
    return INTERP_16_UNMASK_1 ( ( INTERP_16_MASK_1 ( p1 ) * 9 + INTERP_16_MASK_1 ( p2 ) * 7 ) / 16 )
           | INTERP_16_UNMASK_2 ( ( INTERP_16_MASK_2 ( p1 ) * 9 + INTERP_16_MASK_2 ( p2 ) * 7 ) / 16 );
}

Sint32 interp_16_diff ( Uint16 p1, Uint16 p2 )
{
    Sint32 r = 0, g = 0, b = 0;
    Sint32 y = 0, u = 0, v = 0;

    if ( p1 == p2 )
    {
        /* @todo (Tmesys#1#12/17/2022): Use SDL types. */
        return 0;
    }

    b = ( Sint32 ) ( ( p1 & 0x1F ) - ( p2 & 0x1F ) ) << 3;
    g = ( Sint32 ) ( ( p1 & 0x7E0 ) - ( p2 & 0x7E0 ) ) >> 3;
    r = ( Sint32 ) ( ( p1 & 0xF800 ) - ( p2 & 0xF800 ) ) >> 8;

    y = r + g + b;

    if ( y < -INTERP_Y_LIMIT || y > INTERP_Y_LIMIT )
    {
        return 1;
    }

    u = r - b;

    if ( u < -INTERP_U_LIMIT || u > INTERP_U_LIMIT )
    {
        return 1;
    }

    v = -r + 2 * g - b;

    if ( v < -INTERP_V_LIMIT || v > INTERP_V_LIMIT )
    {
        return 1;
    }

    return 0;
}

#ifdef _GNGEOX_INTERP_C_
#undef _GNGEOX_INTERP_C_
#endif // _GNGEOX_INTERP_C_
