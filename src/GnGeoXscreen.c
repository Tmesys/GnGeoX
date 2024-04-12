/*!
*
*   \file    GnGeoXscreen.c
*   \brief   Screen routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    18/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_SCREEN_C_
#define _GNGEOX_SCREEN_C_
#endif // _GNGEOX_SCREEN_C_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "zlog.h"
#include "qlibc.h"
#include "bstrlib.h"

#include "GnGeoXscreen.h"
#include "GnGeoXvideo.h"
#include "GnGeoXroms.h"
#include "GnGeoXmemory.h"
#include "GnGeoXconfig.h"
#include "GnGeoXsoftblitter.h"
#include "GnGeoXopenglblitter.h"
#include "GnGeoXframeskip.h"
#include "GnGeoXpd4990a.h"
#include "GnGeoXeffects.h"

SDL_Surface* sdl_surface_screen = NULL;
SDL_Surface* sdl_surface_buffer = NULL;
/* Interpolation */
SDL_Surface* sdl_surface_blend = NULL;
SDL_Renderer* sdl_renderer = NULL;
SDL_Window* sdl_window = NULL;
SDL_Rect visible_area;
TTF_Font* sys_font = NULL;
Sint32 yscreenpadding = 0;
Sint32 last_line = 0;

static blitter_func blitter[] =
{
    {
        "soft", "Software blitter", blitter_soft_init, NULL, blitter_soft_update, blitter_soft_fullscreen,
        blitter_soft_close
    },

    {
        "opengl", "Opengl blitter", blitter_opengl_init, blitter_opengl_resize, blitter_opengl_update,
        blitter_opengl_fullscreen, blitter_opengl_close
    },

#if (defined(HAVE_GL_GL_H) || defined(HAVE_OPENGL_GL_H)) && defined(USE_GLSL)
    {
        "glsl", "OpenGL shading language (GLSL) blitter",
        blitter_glsl_init, blitter_glsl_resize, blitter_glsl_update, blitter_glsl_fullscreen, blitter_glsl_close
    },
#endif
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};



static SDL_Rect left_border = {16, 16, 8, 224};
static SDL_Rect right_border = {16 + 312, 16, 8, 224};
/* ******************************************************************************************************************/
/*!
* \brief Takes a screen shot.
*
*/
/* ******************************************************************************************************************/
void neo_screen_capture ( void )
{
    time_t ltime = 0;
    struct tm* today = NULL;
    char buf[256];
    char date_str[101];
    //static SDL_Rect buf_rect    = {16, 16, 304, 224};
    static SDL_Rect screen_rect = { 0, 0, 304, 224 };
    static SDL_Surface* shoot = NULL;

    screen_rect.w = visible_area.w;
    screen_rect.h = visible_area.h;

    if ( shoot == NULL )
    {
        shoot = SDL_CreateRGBSurface ( SDL_SWSURFACE, visible_area.w, visible_area.h, 16, 0xF800, 0x7E0, 0x1F, 0 );
    }

    time ( &ltime );
    today = localtime ( &ltime );
    strftime ( date_str, 100, "%a_%b_%d_%T_%Y", today );
    snprintf ( buf, 255, "%s/%s_%s.bmp", getenv ( "HOME" ), gngeox_config.gamename, date_str );

    SDL_BlitSurface ( sdl_surface_buffer, &visible_area, shoot, &screen_rect );
    SDL_SaveBMP ( shoot, buf );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes SDL.
*
*/
/* ******************************************************************************************************************/
SDL_bool neo_screen_init ( void )
{
    if ( SDL_Init ( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER ) < 0 )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return ( SDL_FALSE );
    }
    atexit ( SDL_Quit );

    if ( TTF_Init() < 0 )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", TTF_GetError() );
        return ( SDL_FALSE );
    }
    atexit ( TTF_Quit );

    sys_font = TTF_OpenFont ( "sys.ttf", 16 );
    if ( sys_font == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Initializing system font" );
        return ( SDL_FALSE );
    }

    TTF_SetFontStyle ( sys_font, TTF_STYLE_NORMAL );

    visible_area.x = 16;
    visible_area.y = 16;
    visible_area.w = 320;
    visible_area.h = 224;

    gngeox_config.res_x = 304;
    gngeox_config.res_y = 224;

    /* Init of video blitter */
    if ( ( *blitter[gngeox_config.blitter_index].init ) () == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }

    /* Init of effect */
    if ( ( *effect[gngeox_config.effect_index].init ) () == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }

    /* Interpolation surface */
    if ( gngeox_config.interpolation == SDL_TRUE )
    {
        sdl_surface_blend = SDL_CreateRGBSurface ( SDL_SWSURFACE, 352, 256, 16, 0xF800, 0x7E0, 0x1F, 0 );
        if ( sdl_surface_blend == NULL )
        {
            zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
            return ( SDL_FALSE );
        }
    }

    sdl_surface_buffer = SDL_CreateRGBSurface ( SDL_SWSURFACE, 352, 256, 32, 0, 0, 0, 0 );
    if ( sdl_surface_buffer == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return ( SDL_FALSE );
    }

    if ( SDL_FillRect ( sdl_surface_buffer, NULL, SDL_MapRGB ( sdl_surface_buffer->format, 0xE5, 0xE5, 0xE5 ) ) < 0 )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return ( SDL_FALSE );
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Resizes screen.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise..
*/
/* ******************************************************************************************************************/
SDL_bool neo_screen_resize ( Sint32 width, Sint32 height )
{
    if ( ( *blitter[gngeox_config.blitter_index].resize ) ( width, height ) == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Closes screen.
*
*/
/* ******************************************************************************************************************/
void neo_screen_close ( void )
{
    if ( gngeox_config.interpolation == SDL_TRUE )
    {
        SDL_FreeSurface ( sdl_surface_blend );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Sets screen fullscreen.
*
*/
/* ******************************************************************************************************************/
void neo_screen_fullscreen ( void )
{
    gngeox_config.fullscreen ^= 1;
    blitter[gngeox_config.blitter_index].fullscreen();
}
/* ******************************************************************************************************************/
/*!
* \brief  Sets window title.
*
*/
/* ******************************************************************************************************************/
void neo_screen_windowtitle_set ( void )
{
    bstring title = NULL;

    if ( gngeox_config.gamename )
    {
        title = bfromcstr ( "GnGeo-X : " );
        bcatcstr ( title, neogeo_memory.rom.info.longname );

        SDL_SetWindowTitle ( sdl_window, title->data );

        bdestroy ( title );
    }
    else
    {
        zlog_error ( gngeox_config.loggingCat, "Game name not set ?" );
        SDL_SetWindowTitle ( sdl_window, "GnGeo-X" );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Updates screen.
*
*/
/* ******************************************************************************************************************/
/* @todo (Tmesys#1#12/04/2024): This is more to me like effects update */
void neo_screen_update ( void )
{
    if ( gngeox_config.interpolation == SDL_TRUE )
    {
        interp_32_screen();
    }

    /* @note (Tmesys#1#12/18/2022): Does not seem to have any effect ? */
    SDL_FillRect ( sdl_surface_buffer, &left_border, 0 );
    SDL_FillRect ( sdl_surface_buffer, &right_border, 0 );

    if ( effect[gngeox_config.effect_index].update != NULL )
    {
        ( *effect[gngeox_config.effect_index].update ) ();
    }

    ( *blitter[gngeox_config.blitter_index].update ) ();
}
/* ******************************************************************************************************************/
/*!
* \brief Updates screen.
*
*/
/* ******************************************************************************************************************/
/* @todo (Tmesys#1#12/04/2024): Rename function */
void update_screen ( void )
{
    if ( neogeo_memory.vid.irq2control & 0x40 )
    {
        neogeo_memory.vid.irq2start = ( neogeo_memory.vid.irq2pos + 3 ) / 0x180;    /* ridhero gives 0x17d */
    }
    else
    {
        neogeo_memory.vid.irq2start = 1000;
    }

    if ( !skip_this_frame )
    {
        if ( last_line < 21 )
        {
            /* there was no IRQ2 while the beam was in the
                                 * visible area -> no need for scanline rendering */
            draw_screen();
        }
        else
        {
            draw_screen_scanline ( last_line - 21, 262, 1 );
        }
    }

    last_line = 0;

    pd4990a_addretrace();

    if ( frame_counter >= neogeo_frame_counter_speed )
    {
        frame_counter = 0;
        neogeo_frame_counter++;
    }

    frame_counter++;

    neo_frame_skip ( );
}
/* ******************************************************************************************************************/
/*!
* \brief  Prints blitter list.
*
*/
/* ******************************************************************************************************************/
void print_blitter_list ( void )
{
    Sint32 i = 0;

    while ( blitter[i].name != NULL )
    {
        zlog_info ( gngeox_config.loggingCat, "%-12s : %s", blitter[i].name, blitter[i].desc );
        i++;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Gets blitter by name.
*
* \return  Blitter index.
*/
/* ******************************************************************************************************************/
Uint8 get_blitter_by_name ( const char* name )
{
    Sint32 i = 0;

    while ( blitter[i].name != NULL )
    {
        if ( !strcmp ( blitter[i].name, name ) )
        {
            return ( i );
        }

        i++;
    }

    /* invalid blitter */
    zlog_error ( gngeox_config.loggingCat, "Invalid blitter." );
    zlog_warn ( gngeox_config.loggingCat, "Forcing use of soft blitter." );

    return ( 0 );
}

#ifdef _GNGEOX_SCREEN_C_
#undef _GNGEOX_SCREEN_C_
#endif // _GNGEOX_SCREEN_C_
