/*!
*
*   \file    GnGeoXscanline.c
*   \brief   Scanline image effect routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    11/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_SCANLINE_C_
#define _GNGEOX_SCANLINE_C_
#endif // _GNGEOX_SCANLINE_C_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "GnGeoXscreen.h"
#include "GnGeoXvideo.h"
#include "GnGeoXscanline.h"
#include "GnGeoXroms.h"
#include "GnGeoXmemory.h"
#include "GnGeoXframecap.h"

Sint32 current_line = 0;

/* ******************************************************************************************************************/
/*!
* \brief  Initializes scanline effect.
*
*/
/* ******************************************************************************************************************/
SDL_bool effect_scanline_init ( void )
{
    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Updates scanline effect.
*
* \param  program_counter Program counter.
*/
/* ******************************************************************************************************************/
void effect_scanline_update ( void )
{
    Uint16* src = NULL, *dst = NULL;
    Uint32 src_pixel = 0, dst_pixel = 0;

    /* LeftBorder + RowLength * UpperBorder */
    src = ( Uint16* ) sdl_surface_buffer->pixels + visible_area.x + ( 352 << 4 );
    dst = ( Uint16* ) sdl_surface_screen->pixels + yscreenpadding;

    for ( Uint32 height = visible_area.h; height > 0; height-- )
    {
        for ( Uint32 weight = visible_area.w >> 1; weight > 0; weight-- )
        {
            src_pixel = * ( Uint32* ) src;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            dst_pixel = ( src_pixel & 0xFFFF0000 ) + ( ( src_pixel & 0xFFFF0000 ) >> 16 );
            * ( Uint32* ) ( dst ) = dst_pixel;

            dst_pixel = ( src_pixel & 0x0000FFFF ) + ( ( src_pixel & 0x0000FFFF ) << 16 );
            * ( Uint32* ) ( dst + 2 ) = dst_pixel;
#else
            dst_pixel = ( src_pixel & 0xFFFF0000 ) + ( ( src_pixel & 0xFFFF0000 ) >> 16 );
            * ( Uint32* ) ( dst + 2 ) = dst_pixel;

            dst_pixel = ( src_pixel & 0x0000FFFF ) + ( ( src_pixel & 0x0000FFFF ) << 16 );
            * ( Uint32* ) ( dst ) = dst_pixel;
#endif

            dst += 4;
            src += 2;
        }

        src += ( visible_area.x << 1 );
        dst += ( visible_area.w << 1 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Updates 50% scanline effect.
*
*/
/* ******************************************************************************************************************/
void effect_scanline50_update ( void )
{
    Uint16* src = NULL, *dst = NULL;
    Uint32 src_pixel = 0, dst_pixel = 0;

    /* LeftBorder + RowLength * UpperBorder */
    src = ( Uint16* ) sdl_surface_buffer->pixels + visible_area.x + ( 352 << 4 );
    dst = ( Uint16* ) sdl_surface_screen->pixels + yscreenpadding;

    for ( Uint32 height = visible_area.h; height > 0; height-- )
    {
        for ( Uint32 weight = visible_area.w >> 1; weight > 0; weight-- )
        {
            src_pixel = * ( Uint32* ) src;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            dst_pixel = ( src_pixel & 0xFFFF0000 ) + ( ( src_pixel & 0xFFFF0000 ) >> 16 );
            * ( Uint32* ) ( dst ) = dst_pixel;
            * ( Uint32* ) ( dst + ( visible_area.w << 1 ) ) = ( dst_pixel & 0xf7def7de ) >> 1;

            dst_pixel = ( src_pixel & 0x0000FFFF ) + ( ( src_pixel & 0x0000FFFF ) << 16 );
            * ( Uint32* ) ( dst + 2 ) = dst_pixel;
            * ( Uint32* ) ( dst + ( visible_area.w << 1 ) + 2 ) = ( dst_pixel & 0xf7def7de ) >> 1;
#else
            dst_pixel = ( src_pixel & 0xFFFF0000 ) + ( ( src_pixel & 0xFFFF0000 ) >> 16 );
            * ( Uint32* ) ( dst + 2 ) = dst_pixel;
            * ( Uint32* ) ( dst + ( visible_area.w << 1 ) + 2 ) = ( dst_pixel & 0xf7def7de ) >> 1;

            dst_pixel = ( src_pixel & 0x0000FFFF ) + ( ( src_pixel & 0x0000FFFF ) << 16 );
            * ( Uint32* ) ( dst ) = dst_pixel;
            * ( Uint32* ) ( dst + ( visible_area.w << 1 ) ) = ( dst_pixel & 0xf7def7de ) >> 1;
#endif
            dst += 4;
            src += 2;
        }

        src += ( visible_area.x << 1 );
        dst += ( visible_area.w << 1 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Updates double resolution scanline effect.
*
* \note Works only with software blitter.
*/
/* ******************************************************************************************************************/
void effect_doublex_update ( void )
{
    Uint16* src = NULL, *dst = NULL;
    Uint32 src_pixel = 0, dst_pixel = 0;

    /* LeftBorder + RowLength * UpperBorder */
    src = ( Uint16* ) sdl_surface_buffer->pixels + visible_area.x + ( 352 << 4 );
    dst = ( Uint16* ) sdl_surface_screen->pixels + ( yscreenpadding >> 1 );

    for ( Uint32 height = visible_area.h; height > 0; height-- )
    {
        for ( Uint32 weight = visible_area.w >> 1; weight > 0; weight-- )
        {
            src_pixel = * ( Uint32* ) src;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            dst_pixel = ( src_pixel & 0xFFFF0000 ) + ( ( src_pixel & 0xFFFF0000 ) >> 16 );
            * ( Uint32* ) ( dst ) = dst_pixel;

            dst_pixel = ( src_pixel & 0x0000FFFF ) + ( ( src_pixel & 0x0000FFFF ) << 16 );
            * ( Uint32* ) ( dst + 2 ) = dst_pixel;
#else
            dst_pixel = ( src_pixel & 0xFFFF0000 ) + ( ( src_pixel & 0xFFFF0000 ) >> 16 );
            * ( Uint32* ) ( dst + 2 ) = dst_pixel;

            dst_pixel = ( src_pixel & 0x0000FFFF ) + ( ( src_pixel & 0x0000FFFF ) << 16 );
            * ( Uint32* ) ( dst ) = dst_pixel;
#endif
            dst += 4;
            src += 2;
        }

        src += ( visible_area.x << 1 );
        //dst += 608;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Updates scanline.
*
*/
/* ******************************************************************************************************************/
Sint32 update_scanline ( void )
{
    neogeo_memory.vid.irq2taken = 0;

    if ( neogeo_memory.vid.irq2control & 0x10 )
    {
        if ( current_line == neogeo_memory.vid.irq2start )
        {
            if ( neogeo_memory.vid.irq2control & 0x80 )
            {
                neogeo_memory.vid.irq2start += ( neogeo_memory.vid.irq2pos + 3 ) / 0x180;
            }

            neogeo_memory.vid.irq2taken = 1;
        }
    }

    if ( neogeo_memory.vid.irq2taken )
    {
        if ( ( last_line >= 21 ) && ( current_line >= 20 ) )
        {
            draw_screen_scanline ( last_line - 21, current_line - 20, SDL_FALSE );
        }

        last_line = current_line;
    }

    current_line++;

    return ( neogeo_memory.vid.irq2taken );
}

#ifdef _GNGEOX_SCANLINE_C_
#undef _GNGEOX_SCANLINE_C_
#endif // _GNGEOX_SCANLINE_C_
