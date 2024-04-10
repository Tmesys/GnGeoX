/*!
*
*   \file    GnGeoXscale.c
*   \brief   Scale image effect routines http://scale2x.sourceforge.net/.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 04.00
*   \date    03/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    This effect is a rewritten implementation of the scale effect made by Andrea Mazzoleni.
*/
#ifndef _GNGEOX_SCALE_C_
#define _GNGEOX_SCALE_C_
#endif // _GNGEOX_SCALE_C_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "GnGeoXscreen.h"
#include "GnGeoXvideo.h"
#include "GnGeoXscale.h"

/* @note (Tmesys#1#12/10/2022): Modified for GnGeoX: since our src duffer is bigger than dest, we can skip first/last pixel test. */

static SDL_Surface* scale4xtmp = NULL;

/* ******************************************************************************************************************/
/*!
* \brief  Initializes scale2x effect.
*
* \return Always SDL_TRUE.
*/
/* ******************************************************************************************************************/
SDL_bool effect_scale2x_init ( void )
{
    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes scale3x effect.
*
* \return Always SDL_TRUE.
*/
/* ******************************************************************************************************************/
SDL_bool effect_scale3x_init ( void )
{
    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes scale4x effect.
*
* \return Always SDL_TRUE.
*/
/* ******************************************************************************************************************/
SDL_bool effect_scale4x_init ( void )
{
    if ( !scale4xtmp )
    {
        scale4xtmp = SDL_CreateRGBSurface ( SDL_SWSURFACE, visible_area.w << 2, ( visible_area.h << 2 ) + 16, 16, 0xF800, 0x7E0, 0x1F, 0 );
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Applies scale2x effect.
*
* \param dst0 Todo.
* \param dst1 Todo.
* \param src0 Todo.
* \param src1 Todo.
* \param src2 Todo.
* \param count Todo.
*/
/* ******************************************************************************************************************/
static void internal_scale2x_16_def ( Uint16* dst0, Uint16* dst1, const Uint16* src0, const Uint16* src1, const Uint16* src2, Uint32 count )
{
    while ( count )
    {
        if ( src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0] )
        {
            dst0[0] = src0[0];
        }
        else
        {
            dst0[0] = src1[0];
        }

        if ( src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0] )
        {
            dst0[1] = src0[0];
        }
        else
        {
            dst0[1] = src1[0];
        }

        if ( src1[-1] == src2[0] && src0[0] != src2[0] && src1[1] != src2[0] )
        {
            dst1[0] = src2[0];
        }
        else
        {
            dst1[0] = src1[0];
        }

        if ( src1[1] == src2[0] && src0[0] != src2[0] && src1[-1] != src2[0] )
        {
            dst1[1] = src2[0];
        }
        else
        {
            dst1[1] = src1[0];
        }

        ++src0;
        ++src1;
        ++src2;
        dst0 += 2;
        dst1 += 2;
        --count;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Applies scale2x 50% variant effect.
*
* \param dst0 Todo.
* \param dst1 Todo.
* \param src0 Todo.
* \param src1 Todo.
* \param src2 Todo.
* \param count Todo.
*/
/* ******************************************************************************************************************/
static void internal_scale2x_16_def_50 ( Uint16* dst0, Uint16* dst1, const Uint16* src0, const Uint16* src1, const Uint16* src2, Uint32 count )
{
    while ( count )
    {
        if ( src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0] )
        {
            dst0[0] = src0[0];
        }
        else
        {
            dst0[0] = src1[0];
        }

        if ( src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0] )
        {
            dst0[1] = src0[0];
        }
        else
        {
            dst0[1] = src1[0];
        }

        if ( src1[-1] == src2[0] && src0[0] != src2[0] && src1[1] != src2[0] )
        {
            dst1[0] = ( src2[0] & 0xf7de ) >> 1;
        }
        else
        {
            dst1[0] = ( src1[0] & 0xf7de ) >> 1;
        }

        if ( src1[1] == src2[0] && src0[0] != src2[0] && src1[-1] != src2[0] )
        {
            dst1[1] = ( src2[0] & 0xf7de ) >> 1;
        }
        else
        {
            dst1[1] = ( src1[0] & 0xf7de ) >> 1;
        }

        ++src0;
        ++src1;
        ++src2;
        dst0 += 2;
        dst1 += 2;
        --count;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Applies scale2x 75% variant effect.
*
* \param dst0 Todo.
* \param dst1 Todo.
* \param src0 Todo.
* \param src1 Todo.
* \param src2 Todo.
* \param count Todo.
*/
/* ******************************************************************************************************************/
static void internal_scale2x_16_def_75 ( Uint16* dst0, Uint16* dst1, const Uint16* src0, const Uint16* src1, const Uint16* src2, Uint32 count )
{
    while ( count )
    {
        if ( src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0] )
        {
            dst0[0] = src0[0];
        }
        else
        {
            dst0[0] = src1[0];
        }

        if ( src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0] )
        {
            dst0[1] = src0[0];
        }
        else
        {
            dst0[1] = src1[0];
        }

        if ( src1[-1] == src2[0] && src0[0] != src2[0] && src1[1] != src2[0] )
        {
            dst1[0] = ( src2[0] & 0xf7de ) >> 1;
            dst1[0] = ( ( src2[0] & 0xf7de ) >> 1 ) + ( ( dst1[0] & 0xf7de ) >> 1 );
        }
        else
        {
            dst1[0] = ( src1[0] & 0xf7de ) >> 1;
            dst1[0] = ( ( src1[0] & 0xf7de ) >> 1 ) + ( ( dst1[0] & 0xf7de ) >> 1 );
        }

        if ( src1[1] == src2[0] && src0[0] != src2[0] && src1[-1] != src2[0] )
        {
            dst1[1] = ( src2[0] & 0xf7de ) >> 1;
            dst1[1] = ( ( src2[0] & 0xf7de ) >> 1 ) + ( ( dst1[1] & 0xf7de ) >> 1 );
        }
        else
        {
            dst1[1] = ( src1[0] & 0xf7de ) >> 1;
            dst1[1] = ( ( src1[0] & 0xf7de ) >> 1 ) + ( ( dst1[1] & 0xf7de ) >> 1 );
        }

        ++src0;
        ++src1;
        ++src2;
        dst0 += 2;
        dst1 += 2;
        --count;
    }
}

static void scale2x_8_def_single ( Uint8* dst, const Uint8* src0, const Uint8* src1, const Uint8* src2, Uint32 count )
{
    if ( count < 2 )
    {
        return;
    }

    while ( count )
    {
        if ( src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0] )
        {
            dst[0] = src0[0];
        }
        else
        {
            dst[0] = src1[0];
        }

        if ( src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0] )
        {
            dst[1] = src0[0];
        }
        else
        {
            dst[1] = src1[0];
        }

        ++src0;
        ++src1;
        ++src2;
        dst += 2;
        --count;
    }
}

static void scale2x_16_def_single ( Uint16* dst, const Uint16* src0, const Uint16* src1, const Uint16* src2, Uint32 count )
{
    if ( count < 2 )
    {
        return;
    }

    while ( count )
    {
        if ( src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0] )
        {
            dst[0] = src0[0];
        }
        else
        {
            dst[0] = src1[0];
        }

        if ( src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0] )
        {
            dst[1] = src0[0];
        }
        else
        {
            dst[1] = src1[0];
        }

        ++src0;
        ++src1;
        ++src2;
        dst += 2;
        --count;
    }
}

static void scale2x_32_def_single ( Uint32* dst, const Uint32* src0, const Uint32* src1, const Uint32* src2, Uint32 count )
{
    if ( count < 2 )
    {
        return;
    }

    while ( count )
    {
        if ( src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0] )
        {
            dst[0] = src0[0];
        }
        else
        {
            dst[0] = src1[0];
        }

        if ( src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0] )
        {
            dst[1] = src0[0];
        }
        else
        {
            dst[1] = src1[0];
        }

        ++src0;
        ++src1;
        ++src2;
        dst += 2;
        --count;
    }
}

/**
 * Scale by a factor of 2 a row of pixels of 8 bits.
 * The function is implemented in C.
 * The pixels over the left and right borders are assumed of the same color of
 * the pixels on the border.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
static void scale2x_8_def ( Uint8* dst0, Uint8* dst1, const Uint8* src0, const Uint8* src1, const Uint8* src2, Uint32 count )
{
    if ( count < 2 )
    {
        return;
    }

    scale2x_8_def_single ( dst0, src0, src1, src2, count );
    scale2x_8_def_single ( dst1, src2, src1, src0, count );
}

/**
 * Scale by a factor of 2 a row of pixels of 16 bits.
 * This function operates like scale2x_8_def() but for 16 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
static void scale2x_16_def ( Uint16* dst0, Uint16* dst1, const Uint16* src0, const Uint16* src1, const Uint16* src2, Uint32 count )
{
    if ( count < 2 )
    {
        return;
    }

    scale2x_16_def_single ( dst0, src0, src1, src2, count );
    scale2x_16_def_single ( dst1, src2, src1, src0, count );
}

/**
 * Scale by a factor of 2 a row of pixels of 32 bits.
 * This function operates like scale2x_8_def() but for 32 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, double length in pixels.
 * \param dst1 Second destination row, double length in pixels.
 */
static void scale2x_32_def ( Uint32* dst0, Uint32* dst1, const Uint32* src0, const Uint32* src1, const Uint32* src2, Uint32 count )
{
    if ( count < 2 )
    {
        return;
    }

    scale2x_32_def_single ( dst0, src0, src1, src2, count );
    scale2x_32_def_single ( dst1, src2, src1, src0, count );
}

static void scale3x_8_def_single ( Uint8* dst, const Uint8* src0, const Uint8* src1, const Uint8* src2, Uint32 count )
{
    if ( count < 2 )
    {
        return;
    }

    /* first pixel */
    dst[0] = src1[0];
    dst[1] = src1[0];

    if ( src1[1] == src0[0] && src2[0] != src0[0] )
    {
        dst[2] = src0[0];
    }
    else
    {
        dst[2] = src1[0];
    }

    ++src0;
    ++src1;
    ++src2;
    dst += 3;

    /* central pixels */
    count -= 2;

    while ( count )
    {
        if ( src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0] )
        {
            dst[0] = src0[0];
        }
        else
        {
            dst[0] = src1[0];
        }

        dst[1] = src1[0];

        if ( src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0] )
        {
            dst[2] = src0[0];
        }
        else
        {
            dst[2] = src1[0];
        }

        ++src0;
        ++src1;
        ++src2;

        dst += 3;
        --count;
    }

    /* last pixel */
    if ( src1[-1] == src0[0] && src2[0] != src0[0] )
    {
        dst[0] = src0[0];
    }
    else
    {
        dst[0] = src1[0];
    }

    dst[1] = src1[0];
    dst[2] = src1[0];
}

static void scale3x_8_def_fill ( Uint8* dst, const Uint8* src, Uint32 count )
{
    while ( count )
    {
        dst[0] = src[0];
        dst[1] = src[0];
        dst[2] = src[0];

        ++src;
        dst += 3;
        --count;
    }
}

static void scale3x_16_def_single ( Uint16* dst, const Uint16* src0, const Uint16* src1, const Uint16* src2, Uint32 count )
{
    if ( count < 2 )
    {
        return;
    }

    /* first pixel */
    dst[0] = src1[0];
    dst[1] = src1[0];

    if ( src1[1] == src0[0] && src2[0] != src0[0] )
    {
        dst[2] = src0[0];
    }
    else
    {
        dst[2] = src1[0];
    }

    ++src0;
    ++src1;
    ++src2;
    dst += 3;

    /* central pixels */
    count -= 2;

    while ( count )
    {
        if ( src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0] )
        {
            dst[0] = src0[0];
        }
        else
        {
            dst[0] = src1[0];
        }

        dst[1] = src1[0];

        if ( src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0] )
        {
            dst[2] = src0[0];
        }
        else
        {
            dst[2] = src1[0];
        }

        ++src0;
        ++src1;
        ++src2;

        dst += 3;
        --count;
    }

    /* last pixel */
    if ( src1[-1] == src0[0] && src2[0] != src0[0] )
    {
        dst[0] = src0[0];
    }
    else
    {
        dst[0] = src1[0];
    }

    dst[1] = src1[0];
    dst[2] = src1[0];
}

static void scale3x_16_def_fill ( Uint16* dst, const Uint16* src, Uint32 count )
{
    while ( count )
    {
        dst[0] = src[0];
        dst[1] = src[0];
        dst[2] = src[0];

        ++src;
        dst += 3;
        --count;
    }
}

static void scale3x_32_def_single ( Uint32* dst, const Uint32* src0, const Uint32* src1, const Uint32* src2, Uint32 count )
{
    if ( count < 2 )
    {
        return;
    }

    /* first pixel */
    dst[0] = src1[0];
    dst[1] = src1[0];

    if ( src1[1] == src0[0] && src2[0] != src0[0] )
    {
        dst[2] = src0[0];
    }
    else
    {
        dst[2] = src1[0];
    }

    ++src0;
    ++src1;
    ++src2;
    dst += 3;

    /* central pixels */
    count -= 2;

    while ( count )
    {
        if ( src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0] )
        {
            dst[0] = src0[0];
        }
        else
        {
            dst[0] = src1[0];
        }

        dst[1] = src1[0];

        if ( src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0] )
        {
            dst[2] = src0[0];
        }
        else
        {
            dst[2] = src1[0];
        }

        ++src0;
        ++src1;
        ++src2;

        dst += 3;
        --count;
    }

    /* last pixel */
    if ( src1[-1] == src0[0] && src2[0] != src0[0] )
    {
        dst[0] = src0[0];
    }
    else
    {
        dst[0] = src1[0];
    }

    dst[1] = src1[0];
    dst[2] = src1[0];
}

static void scale3x_32_def_fill ( Uint32* dst, const Uint32* src, Uint32 count )
{
    while ( count )
    {
        dst[0] = src[0];
        dst[1] = src[0];
        dst[2] = src[0];

        ++src;
        dst += 3;
        --count;
    }
}

/**
 * Scale by a factor of 3 a row of pixels of 8 bits.
 * The function is implemented in C.
 * The pixels over the left and right borders are assumed of the same color of
 * the pixels on the border.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, triple length in pixels.
 * \param dst1 Second destination row, triple length in pixels.
 * \param dst2 Third destination row, triple length in pixels.
 */
static void scale3x_8_def ( Uint8* dst0, Uint8* dst1, Uint8* dst2, const Uint8* src0, const Uint8* src1, const Uint8* src2, Uint32 count )
{
    if ( count < 2 )
    {
        return;
    }

    scale3x_8_def_single ( dst0, src0, src1, src2, count );
    scale3x_8_def_fill ( dst1, src1, count );
    scale3x_8_def_single ( dst2, src2, src1, src0, count );
}

/**
 * Scale by a factor of 3 a row of pixels of 16 bits.
 * This function operates like scale3x_8_def() but for 16 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, triple length in pixels.
 * \param dst1 Second destination row, triple length in pixels.
 * \param dst2 Third destination row, triple length in pixels.
 */
static void scale3x_16_def ( Uint16* dst0, Uint16* dst1, Uint16* dst2, const Uint16* src0, const Uint16* src1, const Uint16* src2, Uint32 count )
{
    if ( count < 2 )
    {
        return;
    }

    scale3x_16_def_single ( dst0, src0, src1, src2, count );
    scale3x_16_def_fill ( dst1, src1, count );
    scale3x_16_def_single ( dst2, src2, src1, src0, count );
}

/**
 * Scale by a factor of 3 a row of pixels of 32 bits.
 * This function operates like scale3x_8_def() but for 32 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, triple length in pixels.
 * \param dst1 Second destination row, triple length in pixels.
 * \param dst2 Third destination row, triple length in pixels.
 */
static void scale3x_32_def ( Uint32* dst0, Uint32* dst1, Uint32* dst2, const Uint32* src0, const Uint32* src1, const Uint32* src2, Uint32 count )
{
    if ( count < 2 )
    {
        return;
    }

    scale3x_32_def_single ( dst0, src0, src1, src2, count );
    scale3x_32_def_fill ( dst1, src1, count );
    scale3x_32_def_single ( dst2, src2, src1, src0, count );
}

void effect_scale2x_update ( void )
{
    Uint16* dst0 = NULL, *dst1 = NULL, *src0 = NULL, *src1 = NULL, *src2 = NULL;
    Uint16 height = 0;

    height = visible_area.h;

    dst0 = ( Uint16* ) sdl_surface_screen->pixels + yscreenpadding;
    dst1 = ( Uint16* ) dst0 + ( visible_area.w << 1 );

    src1 = ( Uint16* ) sdl_surface_buffer->pixels + 352 * visible_area.y + visible_area.x;
    src0 = ( Uint16* ) src1 - 352;
    src2 = ( Uint16* ) src1 + 352;

    while ( height-- )
    {
        scale2x_16_def ( dst0, dst1, src0, src1, src2, visible_area.w );

        dst0 += ( visible_area.w << 2 );
        dst1 += ( visible_area.w << 2 );

        src0 += 352;
        src1 += 352;
        src2 += 352;
    }
}

void effect_scale4x_update ( void )
{
    Uint16* dst0 = NULL, *dst1 = NULL, *src0 = NULL, *src1 = NULL, *src2 = NULL;
    Uint16 height = 0;

    height = visible_area.h;

    dst0 = ( Uint16* ) scale4xtmp->pixels;
    dst1 = ( Uint16* ) dst0 + ( visible_area.w << 1 );

    src1 = ( Uint16* ) sdl_surface_buffer->pixels + 352 * visible_area.y + visible_area.x;
    src0 = ( Uint16* ) src1 - 352;
    src2 = ( Uint16* ) src1 + 352;

    while ( height-- )
    {
        scale2x_16_def ( dst0, dst1, src0, src1, src2, visible_area.w );

        dst0 += ( visible_area.w << 2 );
        dst1 += ( visible_area.w << 2 );

        src0 += 352;
        src1 += 352;
        src2 += 352;
    }

    height = ( visible_area.h << 1 );
    dst0 = ( Uint16* ) sdl_surface_screen->pixels + yscreenpadding;
    dst1 = ( Uint16* ) dst0 + ( visible_area.w << 2 );

    src1 = ( Uint16* ) scale4xtmp->pixels + ( visible_area.w << 1 );
    src0 = ( Uint16* ) src1 - ( visible_area.w << 1 );
    src2 = ( Uint16* ) src1 + ( visible_area.w << 1 );

    while ( height-- )
    {
        scale2x_16_def ( dst0, dst1, src0, src1, src2, ( visible_area.w << 1 ) );

        dst0 += ( visible_area.w << 3 );
        dst1 += ( visible_area.w << 3 );

        src0 += ( visible_area.w << 1 );
        src1 += ( visible_area.w << 1 );
        src2 += ( visible_area.w << 1 );
    }
}

void effect_scale3x_update ( void )
{
    Uint16* dst0 = NULL, *dst1 = NULL, *dst2 = NULL, *src0 = NULL, *src1 = NULL, *src2 = NULL;
    Uint16 height = 0;

    height = visible_area.h;

    dst0 = ( Uint16* ) sdl_surface_screen->pixels + yscreenpadding;
    dst1 = ( Uint16* ) dst0 + visible_area.w * 3;
    dst2 = ( Uint16* ) dst1 + visible_area.w * 3;

    src1 = ( Uint16* ) sdl_surface_buffer->pixels + 352 * visible_area.y + visible_area.x;
    src0 = ( Uint16* ) src1 - 352;
    src2 = ( Uint16* ) src1 + 352;

    while ( height-- )
    {

        scale3x_16_def ( dst0, dst1, dst2, src0, src1, src2, visible_area.w );

        dst0 += ( visible_area.w * 9 );
        dst1 += ( visible_area.w * 9 );
        dst2 += ( visible_area.w * 9 );

        src0 += 352;
        src1 += 352;
        src2 += 352;
    }
}

void effect_scale2x50_update ( void )
{
    Uint16* dst0 = NULL, *dst1 = NULL, *src0 = NULL, *src1 = NULL, *src2 = NULL;
    Uint16 height = 0;

    height = visible_area.h;

    dst0 = ( Uint16* ) sdl_surface_screen->pixels + yscreenpadding;
    dst1 = ( Uint16* ) dst0 + ( visible_area.w << 1 );

    src1 = ( Uint16* ) sdl_surface_buffer->pixels + 352 * visible_area.y + visible_area.x;
    src0 = ( Uint16* ) src1 - 352;
    src2 = ( Uint16* ) src1 + 352;

    while ( height-- )
    {
        internal_scale2x_16_def_50 ( dst0, dst1, src0, src1, src2, visible_area.w );

        dst0 += ( visible_area.w << 2 );
        dst1 += ( visible_area.w << 2 );

        src0 += 352;
        src1 += 352;
        src2 += 352;
    }
}

void effect_scale2x75_update ( void )
{
    Uint16* dst0 = NULL, *dst1 = NULL, *src0 = NULL, *src1 = NULL, *src2 = NULL;
    Uint16 height = 0;

    height = visible_area.h;

    dst0 = ( Uint16* ) sdl_surface_screen->pixels + yscreenpadding;
    dst1 = ( Uint16* ) dst0 + ( visible_area.w << 1 );

    src1 = ( Uint16* ) sdl_surface_buffer->pixels + 352 * visible_area.y + visible_area.x;
    src0 = ( Uint16* ) src1 - 352;
    src2 = ( Uint16* ) src1 + 352;

    while ( height-- )
    {
        internal_scale2x_16_def_75 ( dst0, dst1, src0, src1, src2, visible_area.w );

        dst0 += ( visible_area.w << 2 );
        dst1 += ( visible_area.w << 2 );

        src0 += 352;
        src1 += 352;
        src2 += 352;
    }
}

#ifdef _GNGEOX_SCALE_C_
#undef _GNGEOX_SCALE_C_
#endif // _GNGEOX_SCALE_C_
