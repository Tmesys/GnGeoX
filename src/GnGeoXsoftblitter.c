/*!
*
*   \file    GnGeoXsoftblitter.c
*   \brief   Soft blitter routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    18/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_SOFTBLITTER_C_
#define _GNGEOX_SOFTBLITTER_C_
#endif // _GNGEOX_SOFTBLITTER_C_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "zlog.h"

#include "GnGeoXscreen.h"
#include "GnGeoXvideo.h"
#include "GnGeoXscale.h"
#include "GnGeoXscanline.h"
#include "GnGeoXconfig.h"
#include "GnGeoXsoftblitter.h"
#include "GnGeoXeffects.h"

SDL_Texture* sdl_texture = NULL;
static SDL_Rect screen_rect =   { 0,  0, 304, 224};

/* ******************************************************************************************************************/
/*!
* \brief  Initializes software blitter.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
SDL_bool blitter_soft_init ( void )
{
    Uint32 width = visible_area.w;
    Uint32 height = visible_area.h;

    zlog_info ( gngeox_config.loggingCat, "Soft driver" );

    if ( sdl_window != NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "SDL window already initialized" );
        return ( SDL_FALSE );
    }

    if ( gngeox_config.vsync )
    {
        height = 240;
        screen_rect.y = 8;
    }
    else
    {
        height = visible_area.h;
        screen_rect.y = 0;
        yscreenpadding = 0;
    }

    screen_rect.w = visible_area.w;
    screen_rect.h = visible_area.h;

    if ( gngeox_config.scale == 1 )
    {
        width *= effect[gngeox_config.effect_index].x_ratio;
        height *= effect[gngeox_config.effect_index].y_ratio;
    }
    else
    {
        if ( gngeox_config.scale > 3 )
        {
            gngeox_config.scale = 3;
        }

        width *= gngeox_config.scale;
        height *= gngeox_config.scale;
    }

    sdl_window = SDL_CreateWindow ( "GnGeo-X",
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    width, height,
                                    ( gngeox_config.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0 ) );
    sdl_renderer = SDL_CreateRenderer ( sdl_window, -1, gngeox_config.vsync ? SDL_RENDERER_PRESENTVSYNC : 0 );
    /* for preserving aspect when scaling */
    SDL_RenderSetLogicalSize ( sdl_renderer, width, height );
    //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    sdl_texture = SDL_CreateTexture ( sdl_renderer,
                                      SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      width, height );
    sdl_surface_screen = SDL_CreateRGBSurface ( SDL_SWSURFACE, width, height, 32, 0, 0, 0, 0 );
    if ( !sdl_surface_screen )
    {
        return ( SDL_FALSE );
    }

    if ( gngeox_config.vsync )
    {
        yscreenpadding = screen_rect.y * sdl_surface_screen->pitch;
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Updates software blitter for scale x2.
*
*/
/* ******************************************************************************************************************/
static void update_double ( void )
{
    Uint32* src = NULL, *dst = NULL;
    Uint32 src_pixels = 0;

    /* LeftBorder + RowLength * UpperBorder */
    src = ( Uint32* ) sdl_surface_buffer->pixels + visible_area.x + ( sdl_surface_buffer->w << 4 );
    dst = ( Uint32* ) sdl_surface_screen->pixels + yscreenpadding;

    for ( Uint32 h = visible_area.h; h > 0; h-- )
    {
        for ( Uint32 w = UPDATE_VISIBLE_AREA; w > 0; w-- )
        {
            src_pixels = * ( Uint32* ) src;
            * ( Uint32* ) ( dst ) = src_pixels;
            * ( Uint32* ) ( dst + 1 ) = src_pixels;
            * ( Uint32* ) ( dst + ( visible_area.w << 1 ) ) = src_pixels;
            * ( Uint32* ) ( dst + ( visible_area.w << 1 ) + 1 ) = src_pixels;
            dst += 2;
            src += 1;
        }

        src += ( visible_area.x << 1 );
        dst += ( visible_area.w << 1 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Updates software blitter for scale x3.
*
*/
/* ******************************************************************************************************************/
static void update_triple ( void )
{
    Uint32* src = NULL, *dst = NULL;
    Uint32 src_pixels = 0;

    /* LeftBorder + RowLength * UpperBorder */
    src = ( Uint32* ) sdl_surface_buffer->pixels + visible_area.x + ( sdl_surface_buffer->w << 4 );
    dst = ( Uint32* ) sdl_surface_screen->pixels + yscreenpadding;

    for ( Uint32 h = visible_area.h; h > 0; h-- )
    {
        for ( Uint32 w = UPDATE_VISIBLE_AREA; w > 0; w-- )
        {
            src_pixels = * ( Uint32* ) src;
            dst += 6;
            src += 2;
            * ( Uint32* ) ( dst ) = src_pixels;
            * ( Uint32* ) ( dst + 1 ) = src_pixels;
            * ( Uint32* ) ( dst + 2 ) = src_pixels;
            * ( Uint32* ) ( dst + ( visible_area.w * 3 ) ) = src_pixels;
            * ( Uint32* ) ( dst + ( visible_area.w * 3 ) + 1 ) = src_pixels;
            * ( Uint32* ) ( dst + ( visible_area.w * 3 ) + 2 ) = src_pixels;
            * ( Uint32* ) ( dst + ( visible_area.w * 6 ) ) = src_pixels;
            * ( Uint32* ) ( dst + ( visible_area.w * 6 ) + 1 ) = src_pixels;
            * ( Uint32* ) ( dst + ( visible_area.w * 6 ) + 2 ) = src_pixels;
            dst += 3;
            src += 1;
        }

        src += ( visible_area.x << 1 );
        dst += ( visible_area.w * 6 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Updates software blitter.
*
*/
/* ******************************************************************************************************************/
void blitter_soft_update()
{
    if ( gngeox_config.effect_index == 0 )
    {
        switch ( gngeox_config.scale )
        {
        case ( 2 ) :
            {
                update_double();
            }
            break;

        case ( 3 ) :
            {
                update_triple();
            }
            break;

        default:
            {
                SDL_BlitSurface ( sdl_surface_buffer, &visible_area, sdl_surface_screen, &screen_rect );
            }
            break;
        }
    }

    SDL_UpdateTexture ( sdl_texture, NULL, sdl_surface_screen->pixels, sdl_surface_screen->w * 4 );
    SDL_RenderClear ( sdl_renderer );
    SDL_RenderCopy ( sdl_renderer, sdl_texture, NULL, NULL );
    SDL_RenderPresent ( sdl_renderer );
}
/* ******************************************************************************************************************/
/*!
* \brief  Closes software blitter.
*
*/
/* ******************************************************************************************************************/
void blitter_soft_close()
{
    if ( sdl_texture != NULL )
    {
        SDL_DestroyTexture ( sdl_texture );
        sdl_texture = NULL;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Sets software blitter in full screen mode.
*
*/
/* ******************************************************************************************************************/
void blitter_soft_fullscreen()
{
    SDL_SetWindowFullscreen ( sdl_window, gngeox_config.fullscreen ? SDL_WINDOW_FULLSCREEN : 0 );
}

#ifdef _GNGEOX_SOFTBLITTER_C_
#undef _GNGEOX_SOFTBLITTER_C_
#endif // _GNGEOX_SOFTBLITTER_C_
