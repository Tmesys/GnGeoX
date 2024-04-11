/*!
*
*   \file    GnGeoXopenglblitter.c
*   \brief   Opengl blitter routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    03/10/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_OPENGLBLITTER_C_
#define _GNGEOX_OPENGLBLITTER_C_
#endif // _GNGEOX_OPENGLBLITTER_C_

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_ttf.h>
#include "zlog.h"

#include "GnGeoXscreen.h"
#include "GnGeoXvideo.h"
#include "GnGeoXscale.h"
#include "GnGeoXscanline.h"
#include "GnGeoXconfig.h"
#include "GnGeoXopenglblitter.h"

static float a = 0;
static float b = 0;
static float c = 0;
static float d = 0;

static SDL_Surface* tex_opengl = NULL;
static SDL_Rect glrectef;
static SDL_GLContext context;

/* ******************************************************************************************************************/
/*!
* \brief  Initializes OpenGL blitter.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
SDL_bool blitter_opengl_init ( void )
{
    Uint32 sdl_flags = 0;
    Uint32 width = visible_area.w;
    Uint32 height = visible_area.h;

    if ( sdl_window != NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "SDL window already initialized" );
        return ( SDL_FALSE );
    }

    if ( SDL_GL_SetAttribute ( SDL_GL_DOUBLEBUFFER, 1 ) < 0 )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return ( SDL_FALSE );
    }

    /* @fixme (Tmesys#1#11/04/2024): This control should be handled in configuration parsing i think. */
    if ( ( effect[neffect].x_ratio != 2 || effect[neffect].y_ratio != 2 ) &&
            ( effect[neffect].x_ratio != 1 || effect[neffect].y_ratio != 1 ) )
    {
        zlog_error ( gngeox_config.loggingCat, "Opengl support only effect with a ratio of 2x2 or 1x1" );
        return ( SDL_FALSE );
    }

    /*
      if (gngeox_config.res_x==304 && gngeox_config.res_y==224) {
    */
    if ( gngeox_config.scale < 2 )
    {
        width *= effect[neffect].x_ratio;
        height *= effect[neffect].y_ratio;
    }

    width *= gngeox_config.scale;
    height *= gngeox_config.scale;
    zlog_info ( gngeox_config.loggingCat, "Width : %d / Height : %d / Scale : %d / Visible width : %d / Visible height : %d", width, height, gngeox_config.scale, visible_area.w, visible_area.h );
    /*
        } else {
            width = gngeox_config.res_x;
            height=gngeox_config.res_y;
        }

    */
    gngeox_config.res_x = width;
    gngeox_config.res_y = height;

    sdl_flags = ( gngeox_config.fullscreen ? SDL_WINDOW_FULLSCREEN : 0 ) | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_MAXIMIZED;

    sdl_window = SDL_CreateWindow ( "GnGeo-X",
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    width,
                                    height,
                                    sdl_flags );

    if ( sdl_window == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return ( SDL_FALSE );
    }

    /* Create our opengl context and attach it to our window */
    context = SDL_GL_CreateContext ( sdl_window );
    if ( context == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return ( SDL_FALSE );
    }

    /* This makes our buffer swap synchronized with the monitor's vertical refresh */
    if ( SDL_GL_SetSwapInterval ( 1 ) < 0 )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return ( SDL_FALSE );
    }

    glClearColor ( 0, 0, 0, 0 );
    glClear ( GL_COLOR_BUFFER_BIT );

    glEnable ( GL_TEXTURE_2D );
    glViewport ( 0, 0, width, height );

    /* Linear Filter */
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    /*
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    */
    /* Texture Mode */
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    if ( neffect == 0 )
    {
        /* Texture limits */
        /*
                a = (240.0/304.0);
                b = (48.0/256.0);

                c = (240.0/256.0);
        */

        tex_opengl = SDL_CreateRGBSurface ( SDL_SWSURFACE, 512, 256, 32, 0, 0, 0, 0 );
    }
    else
    {
        /* Texture limits */
        a = ( ( 256.0 / ( float ) visible_area.w ) - 1.0f ) * effect[neffect].x_ratio / 2.0;
        b = ( ( 512.0 / ( float ) visible_area.w ) - 1.0f ) * effect[neffect].x_ratio / 2.0;
        c = ( ( ( float ) visible_area.h / 256.0 ) ) * effect[neffect].y_ratio / 2.0;
        d = ( ( ( float ) ( ( visible_area.w << 1 ) - 512 ) / 256.0 ) ) * effect[neffect].y_ratio / 2.0;
        sdl_surface_screen = SDL_CreateRGBSurface ( SDL_SWSURFACE, ( visible_area.w * 2 ), ( visible_area.h * 2 ) /*512*/, 32, 0, 0, 0, 0 );
        if ( sdl_surface_screen == NULL )
        {
            zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
            return ( SDL_FALSE );
        }

        tex_opengl = SDL_CreateRGBSurface ( SDL_SWSURFACE, 1024, 512, 32, 0, 0, 0, 0 );
        if ( tex_opengl == NULL )
        {
            zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
            return ( SDL_FALSE );
        }

        if ( visible_area.w == 320 )
        {
            glrectef.x = 0;
            glrectef.y = 0;
            glrectef.w = 320 * 2;
            glrectef.h = 224 * 2;
        }

        else
        {
            glrectef.x = 0;
            glrectef.y = 0;
            glrectef.w = 304 * 2;
            glrectef.h = 224 * 2;
        }
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Resizes OpenGL viewport.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
SDL_bool blitter_opengl_resize ( Sint32 w, Sint32 h )
{
    glEnable ( GL_TEXTURE_2D );
    glViewport ( 0, 0, w, h );

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Updates OpenGL blitter.
*
*/
/* ******************************************************************************************************************/
void blitter_opengl_update ( void )
{
    if ( neffect == 0 )
    {
        SDL_BlitSurface ( sdl_surface_buffer, &visible_area, tex_opengl, NULL );
        glTexImage2D ( GL_TEXTURE_2D, 0, 3, 512, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex_opengl->pixels );

        glBegin ( GL_QUADS );
        glTexCoord2f ( 0.0f, 0.0f );
        glVertex2f ( -1.0f, 1.0f );

        glTexCoord2f ( ( float ) visible_area.w / 512.0f, 0.0f );
        glVertex2f ( 1.0f, 1.0f );

        glTexCoord2f ( ( float ) visible_area.w / 512.0f, 1.0f - 32.0f / 256.0f );
        glVertex2f ( 1.0f, -1.0f );

        glTexCoord2f ( 0.0f, 1.0f - 32.0f / 256.0f );
        glVertex2f ( -1.0f, -1.0f );
        glEnd();
    }
    else
    {
        SDL_BlitSurface ( sdl_surface_screen, &glrectef, tex_opengl, NULL );
        glTexImage2D ( GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_BGRA, GL_UNSIGNED_BYTE, tex_opengl->pixels );

        glBegin ( GL_QUADS );
        glTexCoord2f ( 0.0f, 0.0f );
        glVertex2f ( -1.0f, 1.0f );

        glTexCoord2f ( ( float ) visible_area.w * 2 / 1024.0f, 0.0f );
        glVertex2f ( 1.0f, 1.0f );

        glTexCoord2f ( ( float ) visible_area.w * 2 / 1024.0f, 448.0f / 512.0f );
        glVertex2f ( 1.0f, -1.0f );

        glTexCoord2f ( 0.0f, 448.0f / 512.0f );
        glVertex2f ( -1.0f, -1.0f );
        glEnd();
    }

    SDL_GL_SwapWindow ( sdl_window );
}
/* ******************************************************************************************************************/
/*!
* \brief  Closes OpenGL blitter.
*
*/
/* ******************************************************************************************************************/
void blitter_opengl_close ( void )
{
    //if (sdl_surface_screen != NULL)
    //  SDL_FreeSurface(sdl_surface_screen);
}
/* ******************************************************************************************************************/
/*!
* \brief  Sets OpenGL blitter in full screen mode.
*
*/
/* ******************************************************************************************************************/
void blitter_opengl_fullscreen ( void )
{
    SDL_DisplayMode mode;
    SDL_SetWindowFullscreen ( sdl_window, gngeox_config.fullscreen ? SDL_WINDOW_FULLSCREEN : 0 );

    if ( gngeox_config.fullscreen )
    {
        SDL_GetWindowDisplayMode ( sdl_window, &mode );
    }

    else
    {
        SDL_GetWindowSize ( sdl_window, &gngeox_config.res_x, &gngeox_config.res_y );
    }

    glEnable ( GL_TEXTURE_2D );
    glViewport ( 0, 0, gngeox_config.res_x, gngeox_config.res_y );
}
