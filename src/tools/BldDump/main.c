#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include "qlibc.h"
#include "sqlite3.h"
#include "bstrlib.h"

typedef struct struct_gngeoxtranspack_pack struct_gngeoxtranspack_pack;
struct struct_gngeoxtranspack_pack
{
    Uint32 begin, end;
    Uint8 type;
    struct_gngeoxtranspack_pack* next;
};

sqlite3 * db_handle = NULL;

SDL_bool trans_pack_add ( char* game_name, Uint32 begin, Uint32 end, Uint32 type )
{
    bstring bsql = NULL;
    char * sql_err = NULL;
    int return_code = 0;
    /*
        printf ( "-> name = %s\n", game_name );
        printf ( "-> begin = %d\n", begin );
        printf ( "-> end = %d\n", end );
        printf ( "-> type = %d\n", type );
    */
    bsql = bformat ( "INSERT INTO TRANSPACK (short_name,begin,end,type) VALUES ('%s', '%d', '%d', %d);"
                     , game_name
                     , begin
                     , end
                     , type
                   );

    if ( bsql == NULL )
    {
        return ( SDL_FALSE );
    }

    return_code = sqlite3_exec ( db_handle, bsql->data, 0, 0, &sql_err );
    if ( return_code != SQLITE_OK && return_code != SQLITE_CONSTRAINT )
    {
        printf ( "SQL error %s\n", sql_err );
        sqlite3_free ( sql_err );
        return ( SDL_FALSE );
    }

    if ( return_code == SQLITE_CONSTRAINT )
    {
        printf ( "already exists %s\n", game_name );
        return ( SDL_FALSE );
    }

    bdestroy ( bsql );

    return ( SDL_TRUE );
}

SDL_bool trans_pack_open ( char* file_path )
{
    FILE* file_transpack_io = NULL;
    char * file_basename = NULL;

    char line_buffer[255];
    bstring line_parse = NULL;
    bstringList list_global = NULL;
    bstringList list_range = NULL;
    Uint32 type = 0, begin = 0, end = 0, swap = 0;
    bstring game_name = NULL;
    bstring tmp = NULL;

    file_transpack_io = fopen ( file_path, "r" );
    if ( file_transpack_io == NULL )
    {
        printf ( "Can't open trans pack file %s\n", file_path );
        return ( SDL_FALSE );
    }

    printf ( "---------------------------------\n" );
    printf ( "Processing %s : \n", file_path );

    while ( fgets ( line_buffer, 255, file_transpack_io ) )
    {
        line_parse = bfromcstr ( ( const char * ) line_buffer );
        btrimws ( line_parse );

        /* empty line */
        if ( line_parse->slen <= 0 )
        {
            bdestroy ( line_parse );
            continue;
        }

        /* comment line */
        if ( line_parse->data[0] == ';' )
        {
            bdestroy ( line_parse );
            continue;
        }

        list_global = bsplit ( line_parse, ' ' );
        if ( list_global->qty == 0 )
        {
            bstrListDestroy ( list_global );
            bdestroy ( line_parse );
            continue;
        }

        if ( biseqcstrcaseless ( list_global->entry[0], "game" ) != 0 )
        {
            game_name = bstrcpy ( list_global->entry[1] );
            btolower ( game_name );

            file_basename = qfile_get_name ( file_path );
            if ( strcasecmp ( file_basename, game_name->data ) != 0 )
            {
                printf ( "Tag name : %s different from file name : %s\n", game_name->data, file_basename );
            }

            tmp = bfromcstr ( "../roms" );
            bcatcstr ( tmp, "/" );
            bcatcstr ( tmp, file_basename );
            bcatcstr ( tmp, ".zip" );

            free ( file_basename );

            bstrListDestroy ( list_global );
            bdestroy ( line_parse );

            if ( qfile_exist ( tmp->data ) == false )
            {
                printf ( "Skipping, NeoGeo ROM not found : %s\n", tmp->data );
                bdestroy ( tmp );
                return ( SDL_TRUE );
            }

            bdestroy ( tmp );
            continue;
        }

        if ( biseqcstrcaseless ( list_global->entry[0], "name" ) != 0 )
        {
            bstrListDestroy ( list_global );
            bdestroy ( line_parse );
            continue;
        }

        bfindreplacecstr ( list_global->entry[0], "O", "0", 0 );
        bfindreplacecstr ( list_global->entry[0], "o", "0", 0 );
        list_range = bsplit ( list_global->entry[0], '-' );
        sscanf ( list_global->entry[1]->data, "%d", &type );
        sscanf ( list_range->entry[0]->data, "%x", &begin );

        if ( list_range->qty > 1 )
        {
            sscanf ( list_range->entry[1]->data, "%x", &end );
            if ( end == 335 )
            {
                printf ( "Check ^point\n" );
            }
        }
        else
        {
            end = begin;
        }

        if ( end == 0 )
        {
            end = begin;
        }

        if ( end < begin )
        {
            printf ( "Swap incoherent boundaries : begin %d / end %d\n", begin, end );
            swap = begin;
            begin = end;
            end = swap;
        }

        if ( type == 1 )
        {
            type = 2;
        }
        else
        {
            type = 3;
        }

        bstrListDestroy ( list_range );
        bstrListDestroy ( list_global );
        bdestroy ( line_parse );

        if ( trans_pack_add ( game_name->data, begin, end, type ) == SDL_FALSE )
        {
            return ( SDL_FALSE );
        }
    }

    fclose ( file_transpack_io );

    printf ( "Processing OK\n" );

    return ( SDL_TRUE );
}

int main()
{
    DIR *directory = NULL;
    struct dirent *dir = NULL;

    if ( sqlite3_open ( "./transpack_db", &db_handle ) != SQLITE_OK )
    {
        return ( EXIT_FAILURE );
    }

    if ( qfile_change_dir ( "./transpack" ) == false )
    {
        return ( EXIT_FAILURE );
    }

    directory = opendir ( "." );

    if ( directory )
    {
        while ( ( dir = readdir ( directory ) ) != NULL )
        {
            if ( dir->d_type == DT_REG
                    && ( strcasecmp ( qfile_get_ext ( dir->d_name ), "bld" ) == 0 ) )
            {
                if ( trans_pack_open ( dir->d_name ) == SDL_FALSE )
                {
                    return ( EXIT_FAILURE );
                }
            }
        }

        closedir ( directory );
    }

    if ( sqlite3_close ( db_handle ) != SQLITE_OK )
    {
        return ( EXIT_FAILURE );
    }

    return ( EXIT_SUCCESS );
}
