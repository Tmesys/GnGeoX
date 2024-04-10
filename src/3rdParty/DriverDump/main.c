#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include "qlibc.h"
#include "sqlite3.h"
#include "bstrlib.h"

typedef struct
{
    char filename[32];
    Uint8 region;
    Uint32 src;
    Uint32 dest;
    Uint32 size;
    Uint32 crc;
} struct_gngeoxdrivers_rom_file;

typedef struct
{
    char name[32];
    char parent[32];
    char longname[128];
    Uint32 year;
    Uint32 romsize[10];
    Uint32 nb_romfile;
    struct_gngeoxdrivers_rom_file rom[32];
} struct_gngeoxdrivers_rom_def;

struct_gngeoxdrivers_rom_def* load_drv ( void* buffer )
{
    struct_gngeoxdrivers_rom_def* drv = NULL;
    void *buffer_save = NULL;

    buffer_save = buffer;

    drv = ( struct_gngeoxdrivers_rom_def* ) calloc ( 1, sizeof ( struct_gngeoxdrivers_rom_def ) );
    if ( drv == NULL )
    {
        return ( NULL );
    }

    memcpy ( drv->name, buffer, sizeof ( drv->name ) );
    buffer += sizeof ( drv->name );
    memcpy ( drv->parent, buffer, sizeof ( drv->parent ) );
    buffer += sizeof ( drv->parent );
    memcpy ( drv->longname, buffer, sizeof ( drv->longname ) );
    buffer += sizeof ( drv->longname );
    drv->year = * ( Uint32 * ) buffer;

    for ( Uint32 loop = 0; loop < 10; loop++ )
    {
        buffer += sizeof ( Uint32 );
        drv->romsize[loop] = * ( Uint32 * ) buffer;
    }

    buffer += sizeof ( Uint32 );
    drv->nb_romfile = * ( Uint32 * ) buffer;
    buffer += sizeof ( Uint32 );

    /* @fixme (Tmesys#1#22/10/2023): P1 file is declared twice for mslug ? */
    for ( Uint32 loop = 0; loop < drv->nb_romfile; loop++ )
    {
        memcpy ( drv->rom[loop].filename, buffer, sizeof ( drv->rom[loop].filename ) );
        buffer += sizeof ( drv->rom[loop].filename );
        drv->rom[loop].region = * ( Uint8 * ) buffer;
        buffer += sizeof ( Uint8 );
        drv->rom[loop].src = * ( Uint32 * ) buffer;
        buffer += sizeof ( Uint32 );
        drv->rom[loop].dest = * ( Uint32 * ) buffer;
        buffer += sizeof ( Uint32 );
        drv->rom[loop].size = * ( Uint32 * ) buffer;
        buffer += sizeof ( Uint32 );
        drv->rom[loop].crc = * ( Uint32 * ) buffer;
        buffer += sizeof ( Uint32 );
    }

    free ( buffer_save );

    return ( drv );
}

int main()
{
    DIR *directory = NULL;
    struct dirent *dir = NULL;
    void * file_content = NULL;
    struct_gngeoxdrivers_rom_def* drv = NULL;
    sqlite3 * db_handle = NULL;
    bstring bsql = NULL;
    bstring btmp = NULL;
    int return_code = 0;
    char * sql_err = NULL;

    if ( sqlite3_open ( "./drivers_db", &db_handle ) != SQLITE_OK )
    {
        return ( -1 );
    }

    qfile_change_dir ( "./drivers" );
    directory = opendir ( "." );

    if ( directory )
    {
        while ( ( dir = readdir ( directory ) ) != NULL )
        {
            if ( dir->d_type == DT_REG )
            {
                printf ( "---------------------------------\n" );
                printf ( "%s\n", dir->d_name );
                printf ( "---------------------------------\n" );
                file_content = qfile_load ( dir->d_name, NULL );
                drv = load_drv ( file_content );
                printf ( "-> name = %s\n", drv->name );
                printf ( "-> longname = %s\n", drv->longname );
                printf ( "-> nb_romfile = %d\n", drv->nb_romfile );
                printf ( "-> parent = %s\n", drv->parent );
                printf ( "-> year = %d\n", drv->year );

                btmp = bfromcstr ( drv->longname );
                bfindreplacecstr ( btmp, "(", "[", 0 );
                bfindreplacecstr ( btmp, ")", "]", 0 );
                bfindreplacecstr ( btmp, "'", "", 0 );

                bsql = bformat ( "INSERT INTO ROM VALUES ('%s', '%s', %d, '%s');"
                                 , drv->name
                                 , btmp->data
                                 , drv->year
                                 , drv->parent
                               );

                if ( bsql == NULL )
                {
                    return ( -1 );
                }

                return_code = sqlite3_exec ( db_handle, bsql->data, 0, 0, &sql_err );
                if ( return_code != SQLITE_OK && return_code != SQLITE_CONSTRAINT )
                {
                    return ( -1 );
                }

                if ( return_code == SQLITE_CONSTRAINT )
                {
                    printf ( "already exists %s : %s\n", dir->d_name, drv->name );
                }

                sqlite3_free ( sql_err );
                bdestroy ( bsql );
                bdestroy ( btmp );

                for ( Uint32 loop = 0; loop < 10; loop++ )
                {
                    printf ( "-> romsize(%d) = %d\n", loop, drv->romsize[loop] );
                    bsql = bformat ( "INSERT INTO ROM_SIZE(region,size,short_name_fk) VALUES (%d, %d, '%s');"
                                     , loop
                                     , drv->romsize[loop]
                                     , drv->name
                                   );
                    if ( bsql == NULL )
                    {
                        return ( -1 );
                    }

                    return_code = sqlite3_exec ( db_handle, bsql->data, 0, 0, &sql_err );
                    if ( return_code != SQLITE_OK && return_code != SQLITE_CONSTRAINT )
                    {
                        return ( -1 );
                    }

                    sqlite3_free ( sql_err );
                    bdestroy ( bsql );
                }

                for ( Uint32 loop = 0; loop < drv->nb_romfile; loop++ )
                {
                    printf ( "--> rom filename = %s\n", drv->rom[loop].filename );
                    printf ( "---> rom region = %d\n", drv->rom[loop].region );
                    printf ( "---> rom size = %d\n", drv->rom[loop].size );
                    printf ( "---> rom src = %d\n", drv->rom[loop].src );
                    printf ( "---> rom dest = %d\n", drv->rom[loop].dest );
                    printf ( "---> rom crc = %x\n", drv->rom[loop].crc );

                    bsql = bformat ( "INSERT INTO ROM_FILES(filename,short_name_fk,region,src,dest,size,crc) VALUES ('%s', '%s', %d, %d, %d, %d, '%x');"
                                     , drv->rom[loop].filename
                                     , drv->name
                                     , drv->rom[loop].region
                                     , drv->rom[loop].src
                                     , drv->rom[loop].dest
                                     , drv->rom[loop].size
                                     , drv->rom[loop].crc
                                   );

                    return_code = sqlite3_exec ( db_handle, bsql->data, 0, 0, &sql_err );
                    if ( return_code != SQLITE_OK && return_code != SQLITE_CONSTRAINT )
                    {
                        return ( -1 );
                    }

                    if ( return_code == SQLITE_CONSTRAINT )
                    {
                        printf ( "already exists %s : %s\n", dir->d_name, drv->name );
                    }

                    sqlite3_free ( sql_err );
                    bdestroy ( bsql );
                }

                free ( drv );
            }
        }

        closedir ( directory );
    }

    if ( sqlite3_close ( db_handle ) != SQLITE_OK )
    {
        return ( -1 );
    }

    return ( 0 );
}
