/*!
*
*   \file    GnGeoXconfig.c
*   \brief   New configuration routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 02.00
*   \date    09/10/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_CONFIG_C_
#define _GNGEOX_CONFIG_C_
#endif // _GNGEOX_CONFIG_C_

#include <SDL2/SDL.h>

#include "zlog.h"
#include "qlibc.h"
#include "GnGeoXconfig.h"
#include "GnGeoXemu.h"

struct_gngeoxconfig_params gngeox_config;

/* ******************************************************************************************************************/
/*!
* \brief  Load default configuration file.
*
* \param  filename Full path to configuration file.
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
SDL_bool neo_config_init ( char* filename )
{
    qlisttbl_t * tbl = NULL;

    tbl = qconfig_parse_file ( filename, '=' );
    if ( tbl == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Loading configuration file %s", filename );
        return ( SDL_FALSE );
    }

    gngeox_config.rompath = qlisttbl_getstr ( tbl, "path.rompath", true );
    if ( gngeox_config.rompath == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Please specify rom directory" );
        return ( SDL_FALSE );
    }

    gngeox_config.biospath = qlisttbl_getstr ( tbl, "path.biospath", true );
    if ( gngeox_config.biospath == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Please specify bios directory (neogeo.zip)" );
        return ( SDL_FALSE );
    }

    gngeox_config.shaderpath = qlisttbl_getstr ( tbl, "path.shaderpath", true );
    if ( gngeox_config.shaderpath == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Please specify NeoGeoX shader files directory" );
        return ( SDL_FALSE );
    }

    gngeox_config.transpackpath = qlisttbl_getstr ( tbl, "path.transpackpath", true );
    if ( gngeox_config.transpackpath == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Please specify NeoGeoX transpack files directory" );
        return ( SDL_FALSE );
    }

    gngeox_config.nvrampath = qlisttbl_getstr ( tbl, "path.nvrampath", true );
    if ( gngeox_config.nvrampath == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Please specify NeoGeoX nvram files directory" );
        return ( SDL_FALSE );
    }

    gngeox_config.savespath = qlisttbl_getstr ( tbl, "path.savespath", true );
    if ( gngeox_config.savespath == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Please specify NeoGeoX saves files directory" );
        return ( SDL_FALSE );
    }

    gngeox_config.blitter = qlisttbl_getstr ( tbl, "graphics.blitter", true );
    if ( gngeox_config.blitter == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Please specify blitter" );
        return ( SDL_FALSE );
    }

    gngeox_config.effect = qlisttbl_getstr ( tbl, "graphics.effect", true );
    if ( gngeox_config.blitter == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Please specify effect" );
        return ( SDL_FALSE );
    }

    gngeox_config.scale = qlisttbl_getint ( tbl, "graphics.scale" );
    if ( gngeox_config.scale == 0 )
    {
        gngeox_config.scale = 1;
    }

    gngeox_config.fullscreen = qlisttbl_getint ( tbl, "graphics.fullscreen" );

    gngeox_config.interpolation = qlisttbl_getint ( tbl, "graphics.interpolation" );

    gngeox_config.showfps = qlisttbl_getint ( tbl, "graphics.showfps" );

    gngeox_config.autoframeskip = qlisttbl_getint ( tbl, "graphics.autoframeskip" );

    gngeox_config.vsync = qlisttbl_getint ( tbl, "graphics.vsync" );

    gngeox_config.raster = qlisttbl_getint ( tbl, "system.raster" );

    gngeox_config.forcepal = qlisttbl_getint ( tbl, "system.forcepal" );

    gngeox_config.country = qlisttbl_getint ( tbl, "system.country" );
    switch ( gngeox_config.country )
    {
    case ( CTY_JAPAN ) :
    case ( CTY_EUROPE ) :
    case ( CTY_USA ) :
    case ( CTY_ASIA ) :
        {
        }
        break;
    default:
        {
            zlog_error ( gngeox_config.loggingCat, "Unknown country option value %d", gngeox_config.country );
            return ( SDL_FALSE );
        }
        break;
    }

    gngeox_config.systemtype = qlisttbl_getint ( tbl, "system.type" );
    switch ( gngeox_config.systemtype )
    {
    case ( SYS_ARCADE_MVS ) :
    case ( SYS_HOME_AES ) :
    case ( SYS_UNIBIOS ) :
        {
        }
        break;
    default:
        {
            zlog_error ( gngeox_config.loggingCat, "Unknown system type configuration value %d", gngeox_config.systemtype );
            return ( SDL_FALSE );
        }
        break;
    }

    gngeox_config.samplerate = qlisttbl_getint ( tbl, "system.samplerate" );

    gngeox_config.debug = qlisttbl_getint ( tbl, "system.debug" );

    gngeox_config.dump = qlisttbl_getint ( tbl, "system.dump" );

    gngeox_config.joystick = qlisttbl_getint ( tbl, "input.joystick" );

    qlisttbl_free ( tbl );

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Parse command line arguments.
*
* \param  argc Arguemnt count.
* \param  argv Arguments.
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
SDL_bool neo_config_parse_options ( int *argc, char ***argv )
{
    struct opttype opts[] =
    {
        {"rompath", 'r', OPTTYPE_STRING, &gngeox_config.rompath},
        {"biospath", 'b', OPTTYPE_STRING, &gngeox_config.biospath},
        {"shaderpath", 's', OPTTYPE_STRING, &gngeox_config.shaderpath},
        {"transpackpath", 't', OPTTYPE_STRING, &gngeox_config.transpackpath},
        {"nvrampath", 'v', OPTTYPE_STRING, &gngeox_config.transpackpath},
        {"savespath", 'x', OPTTYPE_STRING, &gngeox_config.transpackpath},
        {"blitter", 'a', OPTTYPE_STRING, &gngeox_config.blitter},
        {"effect", 'e', OPTTYPE_STRING, &gngeox_config.effect},
        {"scale", 'h', OPTTYPE_UINT, &gngeox_config.scale},
        {"fullscreen", 'c', OPTTYPE_BOOL, &gngeox_config.fullscreen},
        {"interpolation", 'i', OPTTYPE_BOOL, &gngeox_config.interpolation},
        {"showfps", 'p', OPTTYPE_BOOL, &gngeox_config.showfps},
        {"autoframeskip", 'k', OPTTYPE_BOOL, &gngeox_config.autoframeskip},
        {"vsync", 'y', OPTTYPE_BOOL, &gngeox_config.vsync},
        {"raster", 't', OPTTYPE_BOOL, &gngeox_config.raster},
        {"forcepal", 'u', OPTTYPE_BOOL, &gngeox_config.forcepal},
        {"country", 'n', OPTTYPE_UINT, &gngeox_config.country},
        {"systemtype", 'm', OPTTYPE_UINT, &gngeox_config.systemtype},
        {"samplerate", 'l', OPTTYPE_UINT, &gngeox_config.samplerate},
        {"debug", 'g', OPTTYPE_BOOL, &gngeox_config.debug},
        {"dump", 'p', OPTTYPE_BOOL, &gngeox_config.dump},
        {"joystick", 'j', OPTTYPE_BOOL, &gngeox_config.joystick},
        {"gamename", 'f', OPTTYPE_STRING, &gngeox_config.gamename},
        {0}
    };

    qoptfetch ( argc, argv, opts );

    if ( gngeox_config.gamename == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Please specify a game to load" );
        return ( SDL_FALSE );
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Clean configuration objects.
*
*/
/* ******************************************************************************************************************/
void neo_config_close ( void )
{
    if ( gngeox_config.biospath != NULL )
    {
        free ( gngeox_config.biospath );
    }

    if ( gngeox_config.blitter != NULL )
    {
        free ( gngeox_config.blitter );
    }

    if ( gngeox_config.effect != NULL )
    {
        free ( gngeox_config.effect );
    }

    if ( gngeox_config.rompath != NULL )
    {
        free ( gngeox_config.rompath );
    }

    if ( gngeox_config.shaderpath != NULL )
    {
        free ( gngeox_config.shaderpath );
    }

    if ( gngeox_config.transpackpath != NULL )
    {
        free ( gngeox_config.transpackpath );
    }

    if ( gngeox_config.nvrampath != NULL )
    {
        free ( gngeox_config.nvrampath );
    }

    if ( gngeox_config.savespath != NULL )
    {
        free ( gngeox_config.savespath );
    }
}
#ifdef _GNGEOX_CONFIG_C_
#undef _GNGEOX_CONFIG_C_
#endif // _GNGEOX_CONFIG_C_
