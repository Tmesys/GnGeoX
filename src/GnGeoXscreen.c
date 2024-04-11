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

#include "GnGeoXscreen.h"
#include "GnGeoXvideo.h"
#include "GnGeoXroms.h"
#include "GnGeoXmemory.h"
#include "GnGeoXconfig.h"
#include "GnGeoXhq2x.h"
#include "GnGeoXxbr2x.h"
#include "GnGeoXscale.h"
#include "GnGeoXscanline.h"
#include "GnGeoXsoftblitter.h"
#include "GnGeoXopenglblitter.h"
#include "GnGeoXframeskip.h"
#include "GnGeoXpd4990a.h"

SDL_Surface* sdl_surface_screen = NULL;
SDL_Surface* sdl_surface_buffer = NULL;
SDL_Texture* sdl_texture = NULL;
SDL_Renderer* sdl_renderer = NULL;
SDL_Window* sdl_window = NULL;
SDL_Rect visible_area;
TTF_Font* sys_font = NULL;
Sint32 yscreenpadding = 0;
Uint8 nblitter = 0;
Uint8 neffect = 0;
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

struct_gngeoxscreen_effect_func effect[] =
{
    {"none", "No effect", 1, 1, effect_none_init, NULL},
    {"scanline", "Scanline effect", 2, 2, effect_scanline_init, effect_scanline_update}, // 1
    {"scanline50", "Scanline 50% effect", 2, 2, effect_scanline_init, effect_scanline50_update}, // 2
    {"scale2x", "Scale2x effect", 2, 2, effect_scale2x_init, effect_scale2x_update}, // 3
    {"scale3x", "Scale3x effect", 3, 3, effect_scale3x_init, effect_scale3x_update}, // 3
    {"scale4x", "Scale4x effect", 4, 4, effect_scale4x_init, effect_scale4x_update}, // 3
    {"hq2x", "HQ2X effect. High quality", 2, 2, effect_hq2x_init, effect_hq2x_update},
    {"xbr2x", "XBR2X effect. High quality", 2, 2, effect_xbr2x_init, effect_xbr2x_update},
    {"doublex", "Double the x resolution (soft blitter only)", 2, 1, effect_scanline_init, effect_doublex_update}, //6
    {NULL, NULL, 0, 0, NULL, NULL}
};

/* Interpolation */
static SDL_Surface* tmp = NULL, *blend = NULL;
static SDL_Rect left_border = {16, 16, 8, 224};
static SDL_Rect right_border = {16 + 312, 16, 8, 224};
/* ******************************************************************************************************************/
/*!
* \brief Takes a screen shot.
*
*/
/* ******************************************************************************************************************/
void take_screenshot ( void )
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
* \brief  Prints effects list.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
void print_effect_list ( void )
{
    Sint32 i = 0;

    while ( effect[i].name != NULL )
    {
        zlog_info ( gngeox_config.loggingCat, "%-12s : %s", effect[i].name, effect[i].desc );
        i++;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Gets effect by name.
*
* \return  Effect index.
*/
/* ******************************************************************************************************************/
Uint8 get_effect_by_name ( const char* name )
{
    Sint32 i = 0;

    while ( effect[i].name != NULL )
    {
        if ( !strcmp ( effect[i].name, name ) )
        {
            return i;
        }

        i++;
    }

    /* invalid effect */
    zlog_error ( gngeox_config.loggingCat, "Invalid effect" );
    return ( 0 );
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
/* ******************************************************************************************************************/
/*!
* \brief  Initializes screen.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
static SDL_bool init_screen ( void )
{
    visible_area.x = 16;
    visible_area.y = 16;
    visible_area.w = 320;
    visible_area.h = 224;

    /* Initialization of some variables */
    nblitter = get_blitter_by_name ( gngeox_config.blitter );
    neffect = get_effect_by_name ( gngeox_config.effect );
    gngeox_config.res_x = 304;
    gngeox_config.res_y = 224;

    /* Init of video blitter */
    if ( ( *blitter[nblitter].init ) () == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }

    /* Init of effect */
    if ( ( *effect[neffect].init ) () == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }

    /* Interpolation surface */
    blend = SDL_CreateRGBSurface ( SDL_SWSURFACE, 352, 256, 16, 0xF800, 0x7E0, 0x1F, 0 );
    if ( blend == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return ( SDL_FALSE );
    }
    /*
    if ( SDL_ShowCursor ( SDL_QUERY ) == 1 )
    {
        SDL_ShowCursor ( SDL_DISABLE );
    }
    */

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes no effects.
*
* \return Always SDL_TRUE.
*/
/* ******************************************************************************************************************/
static SDL_bool effect_none_init ( void )
{
    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Resizes screen.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise..
*/
/* ******************************************************************************************************************/
SDL_bool screen_resize ( Sint32 width, Sint32 height )
{
    if ( ( *blitter[nblitter].resize ) ( width, height ) == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Interpolates screen.
*
*/
/* ******************************************************************************************************************/
static void do_interpolation ( void )
{
    Uint16* dst = ( Uint16* ) blend->pixels + 16 + ( 352 << 4 );
    Uint16* src = ( Uint16* ) sdl_surface_buffer->pixels + 16 + ( 352 << 4 );
    Uint32 s, d;

    /* we copy pixels from buffer surface to blend surface */
    for ( Uint8 w = 224; w > 0; w-- )
    {
        for ( Uint8 h = 160; h > 0; h-- )
        {
            s = * ( Uint32* ) src;
            d = * ( Uint32* ) dst;

            * ( Uint32* ) dst =
                ( ( d & 0xf7def7de ) >> 1 ) + ( ( s & 0xf7def7de ) >> 1 ) +
                ( s & d & 0x08210821 );

            dst += 2;
            src += 2;
        }

        src += 32; //(visible_area.x<<1);
        dst += 32; //(visible_area.x<<1);
    }

    /* Swap Buffers */
    tmp = blend;
    blend = sdl_surface_buffer;
    sdl_surface_buffer = tmp;
}
/* ******************************************************************************************************************/
/*!
* \brief  Updates screen.
*
*/
/* ******************************************************************************************************************/
void screen_update ( void )
{
    if ( gngeox_config.interpolation == SDL_TRUE )
    {
        do_interpolation();
    }

    /* @note (Tmesys#1#12/18/2022): screen320 ? */
    SDL_FillRect ( sdl_surface_buffer, &left_border, 0 );
    SDL_FillRect ( sdl_surface_buffer, &right_border, 0 );

    if ( effect[neffect].update != NULL )
    {
        ( *effect[neffect].update ) ();
    }

    ( *blitter[nblitter].update ) ();
}
/* ******************************************************************************************************************/
/*!
* \brief  Closes screen.
*
*/
/* ******************************************************************************************************************/
void screen_close ( void )
{
    SDL_FreeSurface ( blend );
}
/* ******************************************************************************************************************/
/*!
* \brief  Sets screen fullscreen.
*
*/
/* ******************************************************************************************************************/
void screen_fullscreen ( void )
{
    gngeox_config.fullscreen ^= 1;
    blitter[nblitter].fullscreen();
}
/* ******************************************************************************************************************/
/*!
* \brief  Sets window title.
*
*/
/* ******************************************************************************************************************/
void sdl_set_title ( void )
{
    char* title = NULL;

    if ( gngeox_config.gamename )
    {
        title = ( char* ) qalloc ( strlen ( "GnGeo-X : " ) + strlen ( gngeox_config.gamename ) + 1 );
        if ( title )
        {
            sprintf ( title, "GnGeo-X : %s", gngeox_config.gamename );
            SDL_SetWindowTitle ( sdl_window, title );
            qalloc_delete ( title );
        }
    }
    else
    {
        SDL_SetWindowTitle ( sdl_window, "GnGeo-X" );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes SDL.
*
*/
/* ******************************************************************************************************************/
SDL_bool init_sdl ( void )
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

    if ( init_screen() == SDL_FALSE )
    {
        zlog_error ( gngeox_config.loggingCat, "Initializing screen" );
        return ( SDL_FALSE );
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
* \brief Updates screen.
*
*/
/* ******************************************************************************************************************/
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

#ifdef _GNGEOX_SCREEN_C_
#undef _GNGEOX_SCREEN_C_
#endif // _GNGEOX_SCREEN_C_
