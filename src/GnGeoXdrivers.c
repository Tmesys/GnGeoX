/*!
*
*   \file    GnGeoXdrivers.c
*   \brief   Game description driver and resources loader.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 03.00
*   \date    22/10/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    This effect is a rewritten implementation of the hq2x effect made by Maxim Stepin.
*/
#ifndef _GNGEOX_DRIVERS_C_
#define _GNGEOX_DRIVERS_C_
#endif // _GNGEOX_DRIVERS_C_

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>

#include <SDL2/SDL.h>
#include "zlog.h"
#include "qlibc.h"
#include "sqlite3.h"
#include "bstrlib.h"

#include "GnGeoXdrivers.h"
#include "GnGeoXroms.h"
#include "GnGeoXconfig.h"

/* ******************************************************************************************************************/
/*!
* \brief  Load a parses a rom definition file from gngeo data file.
*
* \param  game_name Game name.
* \return NULL when error, struct_gngeoxdrivers_rom_def* otherwise.
*/
/* ******************************************************************************************************************/
struct_gngeoxdrivers_rom_def* neo_driver_load ( char* game_name )
{
    struct_gngeoxdrivers_rom_def* drv = NULL;
    sqlite3 * db_handle = NULL;
    bstring bsql = NULL;
    sqlite3_stmt * db_stmt = NULL;
    Sint32 result_code = 0;

    result_code = sqlite3_open ( "./drivers_db", &db_handle );
    if ( result_code != SQLITE_OK )
    {
        return ( NULL );
    }

    drv = ( struct_gngeoxdrivers_rom_def* ) qalloc ( sizeof ( struct_gngeoxdrivers_rom_def ) );
    if ( drv == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Memory allocation fail" );
        return ( NULL );
    }

    /* Preparing SQL query : Get rom information */
    bsql = bformat ( "SELECT long_name, \
                    year, \
                    short_name_parent \
                    FROM rom \
                    WHERE short_name = '%s';"
                     , game_name );

    result_code = sqlite3_prepare_v2 ( db_handle, bsql->data, bsql->slen, &db_stmt, NULL );
    if ( result_code != SQLITE_OK )
    {
        zlog_error ( gngeox_config.loggingCat, "Preparing Fetch : %d (%s)", result_code, sqlite3_errmsg ( db_handle ) );
        bdestroy ( bsql );
        return ( NULL );
    }

    bdestroy ( bsql );

    result_code = sqlite3_step ( db_stmt );
    if ( result_code == SQLITE_ROW )
    {
        /* short_name */
        drv->name = bfromcstr ( game_name );
        /* long_name */
        drv->longname = bfromcstr ( sqlite3_column_text ( db_stmt, 0 ) );
        /* year */
        drv->year = sqlite3_column_int ( db_stmt, 1 );
        /* short_name_parent */
        drv->parent = bfromcstr ( sqlite3_column_text ( db_stmt, 2 ) );
    }
    else
    {
        zlog_error ( gngeox_config.loggingCat, "Driver for %s not found", game_name );
        return ( NULL );
    }

    sqlite3_finalize ( db_stmt );

    /* Preparing SQL query : Get rom region sizes */
    bsql = bformat ( "SELECT region, \
                    size \
                    FROM rom_size \
                    WHERE short_name_fk = '%s' \
                    ORDER BY region ASC;"
                     , game_name );

    result_code = sqlite3_prepare_v2 ( db_handle, bsql->data, bsql->slen, &db_stmt, NULL );
    if ( result_code != SQLITE_OK )
    {
        zlog_error ( gngeox_config.loggingCat, "Preparing Fetch : %d (%s)", result_code, sqlite3_errmsg ( db_handle ) );
        bdestroy ( bsql );
        return ( NULL );
    }

    bdestroy ( bsql );

    while ( sqlite3_step ( db_stmt ) == SQLITE_ROW )
    {
        Sint32 size = 0, region = 0;

        /* region */
        region = sqlite3_column_int ( db_stmt, 0 );
        /* size */
        size = sqlite3_column_int ( db_stmt, 1 );

        drv->romsize[region] = size;
    }

    sqlite3_finalize ( db_stmt );

    /* Preparing SQL query : Get rom files information */
    bsql = bformat ( "SELECT filename, \
                    region, \
                    src, \
                    dest, \
                    size, \
                    crc \
                    FROM rom_files \
                    WHERE short_name_fk = '%s' \
                    ORDER BY region ASC;"
                     , game_name );

    result_code = sqlite3_prepare_v2 ( db_handle, bsql->data, bsql->slen, &db_stmt, NULL );
    if ( result_code != SQLITE_OK )
    {
        zlog_error ( gngeox_config.loggingCat, "Preparing Fetch : %d (%s)", result_code, sqlite3_errmsg ( db_handle ) );
        bdestroy ( bsql );
        return ( NULL );
    }

    bdestroy ( bsql );

    while ( sqlite3_step ( db_stmt ) == SQLITE_ROW )
    {
        /* filename */
        drv->rom[drv->nb_romfile].filename = bfromcstr ( sqlite3_column_text ( db_stmt, 0 ) );
        /* region */
        drv->rom[drv->nb_romfile].region = sqlite3_column_int ( db_stmt, 1 );
        /* src */
        drv->rom[drv->nb_romfile].src = sqlite3_column_int ( db_stmt, 2 );
        /* dest */
        drv->rom[drv->nb_romfile].dest = sqlite3_column_int ( db_stmt, 3 );
        /* size */
        drv->rom[drv->nb_romfile].size = sqlite3_column_int ( db_stmt, 4 );
        /* crc */
        drv->rom[drv->nb_romfile].crc = ( Uint32 ) strtol ( sqlite3_column_text ( db_stmt, 5 ), NULL, 16 ) ;

        drv->nb_romfile++;
    }

    result_code = sqlite3_finalize ( db_stmt );
    if ( result_code != SQLITE_OK )
    {
        zlog_error ( gngeox_config.loggingCat, "Finalizing SQL statement : %d (%s)", result_code, sqlite3_errmsg ( db_handle ) );
        return ( NULL );
    }

    result_code = sqlite3_close ( db_handle );
    if ( result_code != SQLITE_OK )
    {
        zlog_error ( gngeox_config.loggingCat, "Closing transpack : %d (%s)", result_code, sqlite3_errmsg ( db_handle ) );
        return ( NULL );
    }

    return ( drv );
}
/* ******************************************************************************************************************/
/*!
* \brief  Frees ROM definition ressources.
*
* \param  drv ROM definition.
*/
/* ******************************************************************************************************************/
void neo_driver_free ( struct_gngeoxdrivers_rom_def* drv )
{
    for ( Uint32 loop = 0; loop < 32; loop++ )
    {
        if ( drv->rom[loop].filename != NULL )
        {
            bdestroy ( drv->rom[loop].filename );
        }
    }

    bdestroy ( drv->longname );
    bdestroy ( drv->name );
    bdestroy ( drv->parent );
    qalloc_delete ( drv );
}

#ifdef _GNGEOX_DRIVERS_C_
#undef _GNGEOX_DRIVERS_C_
#endif // _GNGEOX_DRIVERS_C_
