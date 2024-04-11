/*!
*
*   \file    GnGeoXtranspack.c
*   \brief   Transparency pack routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.40 (final beta)
*   \date    04/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    Transparency pack have been introduced with Nebula and Kawaks.
*            This is basicly a file that define which sprite should be draw with
*            transparency. For example, explosion in Blazing Star or special effect
*            move in King Of Fighters.
*            You can find some pack in the nebula distribution (they have .bld extension)
*            It's pretty simple to use it with Gngeo, you just have to use the -t
*            option (ex: gngeo -t /config/kof96DS.bld kof96).
*/
#ifndef _GNGEOX_TRANSPACK_C_
#define _GNGEOX_TRANSPACK_C_
#endif // _GNGEOX_TRANSPACK_C_

#include <SDL2/SDL.h>
#include "zlog.h"
#include "qlibc.h"
#include "sqlite3.h"
#include "bstrlib.h"

#include "GnGeoXtranspack.h"
#include "GnGeoXconfig.h"

/* @fixme (Tmesys#1#12/07/2022): Routines ares implemented but are not used anywhere in emulator. */
sqlite3 * db_handle = NULL;
sqlite3_stmt * db_stmt = NULL;
struct_gngeoxtranspack_range *range_list = NULL;
Uint32 range_list_size = 0;
/* ******************************************************************************************************************/
/*!
* \brief Loads a Nebula transparency pack.
*
* \param filename Transparency pack path and name.
* \return Todo.
* \note Old codification : Type 1 is for 25% transparency, 2 for 50%.
*/
/* ******************************************************************************************************************/
SDL_bool neo_transpack_init ( void )
{
    bstring bsql = NULL;
    Sint32 result_code = 0;
    qlist_t * transpack = NULL;
    struct_gngeoxtranspack_range range_info;

    result_code = sqlite3_open ( "./transpack_db", &db_handle );
    if ( result_code != SQLITE_OK )
    {
        zlog_error ( gngeox_config.loggingCat, "Opening transpack : %d (%s)", result_code, sqlite3_errmsg ( db_handle ) );
        return ( SDL_FALSE );
    }

    /* Preparing SQL query */
    bsql = bfromcstr ( "SELECT begin, end, type \
                    FROM transpack \
                    WHERE short_name = ? \
                    ORDER BY begin ASC;" );

    result_code = sqlite3_prepare_v2 ( db_handle, bsql->data, bsql->slen, &db_stmt, NULL );
    if ( result_code != SQLITE_OK )
    {
        zlog_error ( gngeox_config.loggingCat, "Preparing Fetch : %d (%s)", result_code, sqlite3_errmsg ( db_handle ) );
        bdestroy ( bsql );
        return ( SDL_FALSE );
    }

    bdestroy ( bsql );

    result_code = sqlite3_bind_text ( db_stmt, 1, gngeox_config.gamename, -1, SQLITE_TRANSIENT );
    if ( result_code != SQLITE_OK )
    {
        sqlite3_clear_bindings ( db_stmt );
        sqlite3_reset ( db_stmt );
        zlog_error ( gngeox_config.loggingCat, "Unable to bind game %s : %d (%s)", gngeox_config.gamename, result_code, sqlite3_errmsg ( db_handle ) );

        return ( SDL_FALSE );
    }

    transpack = qlist ( QLIST_THREADSAFE );
    if ( transpack == NULL )
    {
        sqlite3_clear_bindings ( db_stmt );
        sqlite3_reset ( db_stmt );
        zlog_error ( gngeox_config.loggingCat, "Unable to create list" );

        return ( SDL_FALSE );
    }

    while ( sqlite3_step ( db_stmt ) == SQLITE_ROW )
    {
        SDL_zero ( range_info );

        range_info.begin = sqlite3_column_int ( db_stmt, 0 );
        range_info.end = sqlite3_column_int ( db_stmt, 1 );
        range_info.type = sqlite3_column_int ( db_stmt, 2 );

        if ( qlist_addlast ( transpack, &range_info, sizeof ( range_info ) ) == false )
        {
            sqlite3_clear_bindings ( db_stmt );
            sqlite3_reset ( db_stmt );
            zlog_error ( gngeox_config.loggingCat, "Unable to create list" );

            return ( SDL_FALSE );
        }
    }

    sqlite3_clear_bindings ( db_stmt );
    sqlite3_reset ( db_stmt );

    range_list_size = qlist_size ( transpack );

    range_list = (struct_gngeoxtranspack_range *) qlist_toarray ( transpack, NULL );
    if ( range_list == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Unable to create list" );

        return ( SDL_FALSE );
    }

    qlist_free ( transpack );

    result_code = sqlite3_finalize ( db_stmt );
    if ( result_code != SQLITE_OK )
    {
        zlog_error ( gngeox_config.loggingCat, "Finalizing SQL statement : %d (%s)", result_code, sqlite3_errmsg ( db_handle ) );
        return ( SDL_FALSE );
    }
    db_stmt = NULL;

    result_code = sqlite3_close ( db_handle );
    if ( result_code != SQLITE_OK )
    {
        zlog_error ( gngeox_config.loggingCat, "Closing transpack : %d (%s)", result_code, sqlite3_errmsg ( db_handle ) );
        return ( SDL_FALSE );
    }
    db_handle = NULL;

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief Finds transparency tweak for tile.
*
* \param tile Todo.
* \return Todo.
*/
/* ******************************************************************************************************************/
enum_gngeoxtranspack_tile_type neo_transpack_find ( Uint32 tile )
{
    for ( Uint32 loop = 0; loop < range_list_size ; loop++ )
    {
        if ( range_list[loop].begin <= tile && tile <= range_list[loop].end )
        {
            return ( range_list[loop].type );
        }

        if (  tile < range_list[loop].begin )
        {
            break;
        }
    }

    return ( TILE_TRANS_UNKNOWN );
}
/* ******************************************************************************************************************/
/*!
* \brief Frees a transparency pack.
*
*/
/* ******************************************************************************************************************/
SDL_bool neo_transpack_close ( void )
{
    Sint32 result_code = 0;

    if ( range_list != NULL )
    {
        free ( range_list );
    }

    if ( db_stmt != NULL )
    {
        result_code = sqlite3_finalize ( db_stmt );
        if ( result_code != SQLITE_OK )
        {
            zlog_error ( gngeox_config.loggingCat, "Finalizing SQL statement : %d (%s)", result_code, sqlite3_errmsg ( db_handle ) );
            return ( SDL_FALSE );
        }
    }

    if ( db_handle != NULL )
    {
        result_code = sqlite3_close ( db_handle );
        if ( result_code != SQLITE_OK )
        {
            zlog_error ( gngeox_config.loggingCat, "Closing transpack : %d (%s)", result_code, sqlite3_errmsg ( db_handle ) );
            return ( SDL_FALSE );
        }
    }

    return ( SDL_TRUE );
}

#ifdef _GNGEOX_TRANSPACK_C_
#undef _GNGEOX_TRANSPACK_C_
#endif // _GNGEOX_TRANSPACK_C_
