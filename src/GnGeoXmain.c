/*!
*
*   \file    GnGeoXmain.c
*   \brief   Main program.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    08/10/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    This effect is a rewritten implementation of the hq2x effect made by Maxim Stepin.
*/

//#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "zlog.h"
#include "qlibc.h"

#include "GnGeoXversion.h"
#include "GnGeoXym2610.h"
#include "GnGeoXvideo.h"
#include "GnGeoXscreen.h"
#include "GnGeoXemu.h"
#include "GnGeoXsound.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXdebug.h"
#include "GnGeoXscale.h"
#include "GnGeoXscanline.h"
#include "GnGeoXtranspack.h"
#include "GnGeoXcontroller.h"
#include "GnGeoXframeskip.h"
#include "GnGeoXconfig.h"
#include "GnGeoXromsgno.h"

Sint32 main ( Sint32 argc, char* argv[] )
{
    printf ( "GnGeoX version %d.%d.%d.rev%d\n\n"
             , GNGEOX_MAJOR
             , GNGEOX_MINOR
             , GNGEOX_BUILD
             , GNGEOX_REVISION );

    printf ( "-> Operating system : %s\n", qsys_info_osname () );
    printf ( "-> Operating system version : %s\n", qsys_info_version () );
    printf ( "-> Operating system release : %s\n", qsys_info_release () );
    printf ( "-> System node : %s\n", qsys_info_node () );
    printf ( "-> System machine : %s\n\n", qsys_info_machine () );

    SDL_zero ( gngeox_config );

    if ( qalloc_init ( ) == false )
    {
        exit ( EXIT_FAILURE );
    }
    atexit ( qalloc_exit );

    if ( zlog_init ( "./GNGEOX-LOG-UNIX.conf" ) )
    {
        exit ( EXIT_FAILURE );
    }
    atexit ( zlog_fini );

    gngeox_config.loggingCat = zlog_get_category ( "CAT0" );
    if ( gngeox_config.loggingCat == NULL )
    {
        exit ( EXIT_FAILURE );
    }

    /* Open Default configuration file */
    if ( neo_config_init ( "./gngeox.ini" ) == SDL_FALSE )
    {
        exit ( EXIT_FAILURE );
    }
    atexit ( neo_config_close );

    /* Command line options overrides default configuration file */
    if ( neo_config_parse_options ( &argc, &argv ) == SDL_FALSE )
    {
        exit ( EXIT_FAILURE );
    }

    if ( neo_screen_init() == SDL_FALSE )
    {
        exit ( EXIT_FAILURE );
    }
    zlog_info ( gngeox_config.loggingCat, "SDL initialization OK" );

    if ( neo_controllers_init() == SDL_FALSE )
    {
        exit ( EXIT_FAILURE );
    }
    atexit ( neo_controllers_close );
    zlog_info ( gngeox_config.loggingCat, "Controllers initialization OK" );

    neo_frame_skip_reset();

    if ( init_game ( gngeox_config.gamename ) == SDL_FALSE )
    {
        zlog_error ( gngeox_config.loggingCat, "Can't init %s", gngeox_config.gamename );
        exit ( EXIT_FAILURE );
    }
    zlog_info ( gngeox_config.loggingCat, "Game initialization OK" );

    /* If asked, do a .gno dump and exit*/
    if ( gngeox_config.dump == SDL_TRUE )
    {
        char dump[8 + 4 + 1];
        sprintf ( dump, "%s.gno", gngeox_config.gamename );
        dr_save_gno ( &neogeo_memory.rom, dump );
        close_game();
        exit ( EXIT_SUCCESS );
    }

    if ( gngeox_config.debug )
    {
        neo_sys_debug_loop();
    }
    else
    {
        neo_sys_main_loop();
    }

    close_game();

    exit ( EXIT_SUCCESS );
}
