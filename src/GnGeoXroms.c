/*!
*
*   \file    GnGeoXroms.c
*   \brief   Rom general routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.40 (final beta)
*   \date    04/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    Many of the NeoGeo bootlegs use their own form of encryption and
*            protection, presumably to make them harder for other bootleggers to
*            copy.  This encryption often involves non-trivial scrambling of the
*            program roms and the games are protected using an ALTERA chip which
*            provides some kind of rom overlay, patching parts of the code.
*            The graphics roms are usually scrambled in a different way to the
*            official SNK cartridges too.
*/
#ifndef _GNGEOX_ROMS_C_
#define _GNGEOX_ROMS_C_
#endif // _GNGEOX_ROMS_C_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "zlog.h"
#include "qlibc.h"
#include "bstrlib.h"

#include "GnGeoXdrivers.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXtranspack.h"
#include "GnGeoXconfig.h"
#include "GnGeoXbios.h"
#include "GnGeoXromsinit.h"
#include "GnGeoXromsgno.h"
#include "GnGeoXscreen.h"
#include "GnGeoXemu.h"

Sint32 neogeo_fix_bank_type = 0;
char * region_name[] =
{
    "audio cpu bios",
    "audio cpu cartridge",
    "audio cpu cartridge encrypted",
    "audio data 1",
    "audio data 2",
    "fixed layer bios",
    "fixed layer cartridge",
    "main cpu bios",
    "main cpu cartridge",
    "sprites",
};
/* ******************************************************************************************************************/
/*!
* \brief  Sets up misc patch.
*
* \param  name Game name.
*/
/* ******************************************************************************************************************/
static void setup_misc_patch ( void )
{
    if ( !strcmp ( gngeox_config.gamename, "ssideki" ) )
    {
        WRITE_WORD_ROM ( &neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p[0x2240], 0x4e71 );
    }

    //if (!strcmp(name, "fatfury3")) {
    //  WRITE_WORD_ROM(neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p, 0x0010);
    //}

    if ( !strcmp ( gngeox_config.gamename, "mslugx" ) )
    {
        /* patch out protection checks */
        Uint8* RAM = neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p;

        for ( Sint32 i = 0; i < neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].size; i += 2 )
        {
            if ( ( READ_WORD_ROM ( &RAM[i + 0] ) == 0x0243 )
                    && ( READ_WORD_ROM ( &RAM[i + 2] ) == 0x0001 ) && /* andi.w  #$1, D3 */
                    ( READ_WORD_ROM ( &RAM[i + 4] ) == 0x6600 ) ) /* bne xxxx */
            {

                WRITE_WORD_ROM ( &RAM[i + 4], 0x4e71 );
                WRITE_WORD_ROM ( &RAM[i + 6], 0x4e71 );
            }
        }

        WRITE_WORD_ROM ( &RAM[0x3bdc], 0x4e71 );
        WRITE_WORD_ROM ( &RAM[0x3bde], 0x4e71 );
        WRITE_WORD_ROM ( &RAM[0x3be0], 0x4e71 );
        WRITE_WORD_ROM ( &RAM[0x3c0c], 0x4e71 );
        WRITE_WORD_ROM ( &RAM[0x3c0e], 0x4e71 );
        WRITE_WORD_ROM ( &RAM[0x3c10], 0x4e71 );
        WRITE_WORD_ROM ( &RAM[0x3c36], 0x4e71 );
        WRITE_WORD_ROM ( &RAM[0x3c38], 0x4e71 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Allocates rom regions.
*
* \param  rom_region Game rom.
* \param  size Size to allocate.
* \param  region_type Region type.
*/
/* ******************************************************************************************************************/
SDL_bool allocate_region ( struct_gngeoxroms_rom_region* rom_region, Uint32 rom_region_size, enum_gngeoxroms_region_type region_type )
{
    if ( rom_region_size != 0 )
    {
        rom_region->p = ( Uint8* ) qalloc ( rom_region_size );
        if ( rom_region->p == NULL )
        {
            rom_region->size = 0;
            zlog_error ( gngeox_config.loggingCat, "Not enough memory ! Requesting %d bytes", rom_region_size );
            return ( SDL_FALSE );
        }
    }
    else
    {
        rom_region->p = NULL;
        zlog_warn ( gngeox_config.loggingCat, "Size zero region %d", region_type );
    }

    rom_region->size = rom_region_size;
    rom_region->type = region_type;

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Frees rom regions.
*
* \param  rom_region Game rom.
*/
/* ******************************************************************************************************************/
static void free_region ( struct_gngeoxroms_rom_region* rom_region )
{
    if ( rom_region->p != NULL )
    {
        qalloc_delete ( rom_region->p );
    }

    rom_region->size = 0;
    rom_region->p = NULL;
}
/* ******************************************************************************************************************/
/*!
* \brief  Reads from ZIP rom file.
*
* \param  zip_entry Todo.
* \param  rom_region Todo.
* \param  dest Todo.
* \param  size Todo.
*/
/* ******************************************************************************************************************/
static SDL_bool read_data_i ( qzip_entry_t* zip_entry, struct_gngeoxroms_rom_region* rom_region, Uint32 dest, Uint32 size )
{
    Uint8* p = NULL;
    Uint32 s = LOAD_BUF_SIZE, c = 0;
    Uint8 iloadbuf[LOAD_BUF_SIZE];

    if ( rom_region->p == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Region not allocated" );
        return ( SDL_FALSE );
    }

    p = rom_region->p + dest;

    if ( rom_region->size < ( dest & ~0x1 ) + ( size * 2 ) )
    {
        zlog_error ( gngeox_config.loggingCat, "Region not big enough %d vs requested %d", rom_region->size, dest + ( size * 2 ) );
        return ( SDL_FALSE );
    }

    while ( size )
    {
        c = size;

        if ( c > s )
        {
            c = s;
        }

        c = qzip_read_entry ( zip_entry, &iloadbuf, c );

        if ( c == 0 )
        {
            return ( SDL_TRUE );
        }

        for ( Uint32 i = 0; i < c; i++ )
        {
            *p = iloadbuf[i];
            p += 2;
        }

        size -= c;
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Reads from ZIP rom file.
*
* \param  zip_entry Todo.
* \param  rom_region Todo.
* \param  dest Todo.
* \param  size Todo.
* \return -1 in case of error, 0 otherwise.
*/
/* ******************************************************************************************************************/
static SDL_bool read_data_p ( qzip_entry_t* zip_entry, struct_gngeoxroms_rom_region* rom_region, struct_gngeoxdrivers_rom_file drv_rom )
{
    Uint32 s = LOAD_BUF_SIZE, c = 0, i = 0;

    if ( rom_region->p == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Region not allocated" );
        return ( SDL_FALSE );
    }

    if ( rom_region->size < ( drv_rom.dest + drv_rom.size ) )
    {
        zlog_error ( gngeox_config.loggingCat, "Region not big enough %d vs requested %d", rom_region->size, ( drv_rom.dest + drv_rom.size ) );
        return ( SDL_FALSE );
    }

    while ( drv_rom.size )
    {
        c = drv_rom.size;

        if ( c > s )
        {
            c = s;
        }

        c = qzip_read_entry ( zip_entry, rom_region->p + drv_rom.dest + i, c );

        if ( c == 0 )
        {
            return ( SDL_TRUE );
        }

        i += c;
        drv_rom.size -= c;
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Loads region.
*
* \param  gz_file Todo.
* \param  rom Todo.
* \param  drv Todo.
* \param  index Todo.
* \return 1 error, 0 otherwise Todo.
*/
/* ******************************************************************************************************************/
static SDL_bool load_region ( qzip_file_t* pz_file, struct_gngeoxroms_game_roms* rom, struct_gngeoxdrivers_rom_def* drv, Sint32 index )
{
    qzip_entry_t* zip_entry = NULL;

    zlog_info ( gngeox_config.loggingCat, "Loading file %s in (%s) region", drv->rom[index].filename->data, region_name[drv->rom[index].region] );

    zip_entry = qzip_open_entry ( pz_file, drv->rom[index].filename->data, drv->rom[index].crc );
    if ( zip_entry == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Unable to open entry %s", drv->rom[index].filename->data );
        return ( SDL_FALSE );
    }

    if ( drv->rom[index].src != 0 )
    {
        Uint32 start_loading = 0;

        if ( drv->rom[index].region == REGION_SPRITES )
        {
            start_loading = ( drv->rom[index].src / 2 );
        }
        else
        {
            start_loading = drv->rom[index].src;
        }

        if ( qzip_seek_entry ( zip_entry, start_loading ) == false )
        {
            zlog_error ( gngeox_config.loggingCat, "Unable to seek entry %s", drv->rom[index].filename->data );
            qzip_close_entry ( zip_entry );
            return ( 1 );
        }
    }

    if ( drv->rom[index].region == REGION_SPRITES )
    {
        /* Special interleaved loading  */
        if ( read_data_i ( zip_entry, &rom->rom_region[REGION_SPRITES], drv->rom[index].dest, drv->rom[index].size ) == SDL_FALSE )
        {
            zlog_error ( gngeox_config.loggingCat, "Unable to read file %s", drv->rom[index].filename->data );
            qzip_close_entry ( zip_entry );
            return ( SDL_FALSE );
        }
    }
    else
    {
        if ( read_data_p ( zip_entry, &rom->rom_region[drv->rom[index].region], drv->rom[index] ) == SDL_FALSE )
        {
            zlog_error ( gngeox_config.loggingCat, "Unable to read file %s", drv->rom[index].filename->data );
            qzip_close_entry ( zip_entry );
            return ( SDL_FALSE );
        }
    }

    zlog_info ( gngeox_config.loggingCat, "Loading completed uncompressed size %d bytes : OK ", zip_entry->file_header.uncompressed_size );

    qzip_close_entry ( zip_entry );

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Loads rom.
*
* \param  rom Todo.
* \param  rom_path Todo.
* \param  name Todo.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
static SDL_bool dr_load_roms ( struct_gngeoxroms_game_roms* rom )
{
    struct_gngeoxdrivers_rom_def* drv = NULL;
    qzip_file_t* zip_file = NULL, *gzp_file = NULL;

    memset ( rom, 0, sizeof ( struct_gngeoxroms_game_roms ) );

    drv = neo_driver_load ( gngeox_config.gamename );
    if ( drv == NULL )
    {
        return ( SDL_FALSE );
    }

    rom->info.name = strdup ( drv->name->data );
    rom->info.longname = strdup ( drv->longname->data );
    rom->info.year = drv->year;
    rom->info.flags = 0;

    /* @todo (Tmesys#1#07/04/2024): Nothing special is done to decrypt rom_region[REGION_AUDIO_CPU_CARTRIDGE] ? Concerns m1 roms according to drivers database. */
    for ( Uint32 i = 0; i < REGION_MAX; i++ )
    {
        if ( allocate_region ( &rom->rom_region[i], drv->romsize[i], i ) == SDL_FALSE )
        {
            return ( SDL_FALSE );
        }
    }

    /* @note (Tmesys#1#07/04/2024): Those flags are needed for gno format (audio bios not handled). */
    if ( drv->romsize[REGION_MAIN_CPU_BIOS] != 0 )
    {
        rom->info.flags |= HAS_CUSTOM_CPU_BIOS;
    }

    if ( drv->romsize[REGION_AUDIO_CPU_BIOS] != 0 )
    {
        rom->info.flags |= HAS_CUSTOM_AUDIO_BIOS;
    }

    zip_file = open_rom_zip ( gngeox_config.rompath, gngeox_config.gamename );
    if ( zip_file == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Rom %s/%s.zip not found", gngeox_config.rompath, gngeox_config.gamename );
        return ( SDL_FALSE );
    }

    /* Now, load the roms */
    for ( Uint32 i = 0; i < drv->nb_romfile; i++ )
    {
        if ( load_region ( zip_file, rom, drv, i ) == SDL_FALSE )
        {
            if ( biseqcstrcaseless ( drv->parent, "neogeo" ) == 0 )
            {
                /* File not found in the roms, try the parent */
                zlog_info ( gngeox_config.loggingCat, "Get file from parent" );

                /* Open Parent. For now, only one parent is supported, no recursion */
                gzp_file = open_rom_zip ( gngeox_config.rompath, drv->parent->data );
                if ( gzp_file == NULL )
                {
                    zlog_error ( gngeox_config.loggingCat, "Parent %s/%s.zip not found", gngeox_config.rompath, gngeox_config.gamename );
                    goto error1;
                }

                if ( load_region ( gzp_file, rom, drv, i ) == SDL_FALSE )
                {
                    zlog_error ( gngeox_config.loggingCat, "File %s not found", drv->rom[i].filename->data );
                    goto error1;
                }

                qzip_close_file ( gzp_file );
            }
        }
    }

    /* Close/clean up */
    qzip_close_file ( zip_file );

    neo_driver_free ( drv );

    if ( rom->rom_region[REGION_AUDIO_DATA_2].size == 0 )
    {
        rom->rom_region[REGION_AUDIO_DATA_2].p = rom->rom_region[REGION_AUDIO_DATA_1].p;
        rom->rom_region[REGION_AUDIO_DATA_2].size = rom->rom_region[REGION_AUDIO_DATA_1].size;
    }

    neogeo_memory.nb_of_tiles = rom->rom_region[REGION_SPRITES].size >> 7;

    /* Init rom and bios */
    init_roms ( rom );
    convert_all_tile ( rom );
    return ( neo_bios_load ( rom ) );

error1:
    qzip_close_file ( zip_file );

    if ( gzp_file )
    {
        qzip_close_file ( gzp_file );
    }

    qalloc_delete ( drv );

    return ( SDL_FALSE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Opens rom Zip.
*
* \param  rom_path Todo.
* \param  name Todo.
*/
/* ******************************************************************************************************************/
static qzip_file_t* open_rom_zip ( const char* rom_path, const char* name )
{
    qzip_file_t* zip_file = NULL;
    bstring fpath = NULL;

    fpath = bfromcstr ( rom_path );
    bcatcstr ( fpath, "/" );
    bcatcstr ( fpath, name );
    bcatcstr ( fpath, ".zip" );

    zip_file = qzip_open_file ( fpath->data );

    bdestroy ( fpath );

    return ( zip_file );
}
/* ******************************************************************************************************************/
/*!
* \brief  Converts rom tiles.
*
* \param  gfx Todo.
* \param  tileno Todo.
*/
/* ******************************************************************************************************************/
static Sint32 convert_roms_tile ( Uint8* gfx, Sint32 tileno )
{
    Uint8 swap[128];
    Uint32* gfxdata = NULL;
    Uint32 pen = 0, usage = 0;

    gfxdata = ( Uint32* ) & gfx[tileno << 7];

    memcpy ( swap, gfxdata, 128 );

    //filed=1;
    for ( Sint32 y = 0; y < 16; y++ )
    {
        Uint32 dw = 0;

        for ( Sint32 x = 0; x < 8; x++ )
        {
            pen = ( ( swap[64 + ( y << 2 ) + 3] >> x ) & 1 ) << 3;
            pen |= ( ( swap[64 + ( y << 2 ) + 1] >> x ) & 1 ) << 2;
            pen |= ( ( swap[64 + ( y << 2 ) + 2] >> x ) & 1 ) << 1;
            pen |= ( swap[64 + ( y << 2 )] >> x ) & 1;
            //if (!pen) filed=0;
            dw |= pen << ( ( 7 - x ) << 2 );
            //memory.pen_usage[tileno]  |= (1 << pen);
            usage |= ( 1 << pen );
        }

        * ( gfxdata++ ) = dw;

        dw = 0;

        for ( Uint32 x = 0; x < 8; x++ )
        {
            pen = ( ( swap[ ( y << 2 ) + 3] >> x ) & 1 ) << 3;
            pen |= ( ( swap[ ( y << 2 ) + 1] >> x ) & 1 ) << 2;
            pen |= ( ( swap[ ( y << 2 ) + 2] >> x ) & 1 ) << 1;
            pen |= ( swap[ ( y << 2 )] >> x ) & 1;
            //if (!pen) filed=0;
            dw |= pen << ( ( 7 - x ) << 2 );
            //memory.pen_usage[tileno]  |= (1 << pen);
            usage |= ( 1 << pen );
        }

        * ( gfxdata++ ) = dw;
    }

    //if ((usage & ~1) == 0) pen_usage|=(TILE_INVISIBLE<<((tileno&0xF)*2));
    /* @todo (Tmesys#1#12/16/2022): transpack support ? */
    if ( ( usage & ~1 ) == 0 )
    {
        return ( TILE_INVISIBLE << ( ( tileno & 0xF ) * 2 ) );
    }
    else
    {
        return ( 0 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Converts all rom tiles.
*
* \param  rom Todo.
*/
/* ******************************************************************************************************************/
static void convert_all_tile ( struct_gngeoxroms_game_roms* rom )
{
    allocate_region ( &rom->spr_usage, ( rom->rom_region[REGION_SPRITES].size >> 11 ) * sizeof ( Uint32 ), REGION_SPR_USAGE );

    for ( Uint32 i = 0; i < rom->rom_region[REGION_SPRITES].size >> 7; i++ )
    {
        ( ( Uint32* ) rom->spr_usage.p ) [i >> 4] |= convert_roms_tile ( rom->rom_region[REGION_SPRITES].p, i );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Converts all rom chars.
*
* \param  ptr Todo.
* \param  size Todo.
* \param  usage_ptr Todo.
*/
/* ******************************************************************************************************************/
void convert_all_char ( enum_gngeoxroms_region_type region_type, Uint8* usage_ptr )
{
    Uint8 usage = 0;
    Uint8* src = NULL;
    Uint8* sav_src = NULL;
    Uint8* ptr = neogeo_memory.rom.rom_region[region_type].p;

    src = ( Uint8* ) qalloc ( neogeo_memory.rom.rom_region[region_type].size );
    if ( src == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Not enough memory ! Requesting %d bytes", neogeo_memory.rom.rom_region[region_type].size );
        return;
    }

    sav_src = src;
    memcpy ( src, ptr, neogeo_memory.rom.rom_region[region_type].size );

    for ( Sint32 i = neogeo_memory.rom.rom_region[region_type].size; i > 0; i -= 32 )
    {
        usage = 0;

        for ( Sint32 j = 0; j < 8; j++ )
        {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            *ptr++ = * ( src + 8 );
            usage |= * ( src + 8 );
            *ptr++ = * ( src );
            usage |= * ( src );
            *ptr++ = * ( src + 24 );
            usage |= * ( src + 24 );
            *ptr++ = * ( src + 16 );
            usage |= * ( src + 16 );
            src++;
#else
            *ptr++ = * ( src + 16 );
            usage |= * ( src + 16 );
            *ptr++ = * ( src + 24 );
            usage |= * ( src + 24 );
            *ptr++ = * ( src );
            usage |= * ( src );
            *ptr++ = * ( src + 8 );
            usage |= * ( src + 8 );
            src++;
#endif
        }

        src += 24;
        *usage_ptr++ = usage;
    }

    qalloc_delete ( sav_src );
}
/* ******************************************************************************************************************/
/*!
* \brief  Loads game.
*
* \param  name Todo.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
static SDL_bool dr_load_game ( char* name )
{
    zlog_info ( gngeox_config.loggingCat, "Loading game %s/%s.zip", gngeox_config.rompath, name );

    neogeo_memory.bksw_handler = 0;
    neogeo_memory.bksw_unscramble = NULL;
    neogeo_memory.bksw_offset = NULL;
    neogeo_memory.rom.need_decrypt = SDL_TRUE;

    if ( dr_load_roms ( &neogeo_memory.rom ) == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }

    /* TODO *///neogeo_fix_bank_type =0;
    /* TODO */
    //  set_bankswitchers(0);

    memcpy ( neogeo_memory.game_vector, neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p, 0x80 );
    memcpy ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p, neogeo_memory.rom.rom_region[REGION_MAIN_CPU_BIOS].p, 0x80 );

    neogeo_memory.fix_game_usage = ( Uint8* ) qalloc ( neogeo_memory.rom.rom_region[REGION_FIXED_LAYER_CARTRIDGE].size );
    if ( neogeo_memory.fix_game_usage == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Not enough memory ! Requesting %d bytes", neogeo_memory.rom.rom_region[REGION_FIXED_LAYER_CARTRIDGE].size );
        return ( SDL_FALSE );
    }

    convert_all_char ( REGION_FIXED_LAYER_CARTRIDGE, neogeo_memory.fix_game_usage );

    /* TODO: Move this somewhere else. */
    init_video();

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Frees rom.
*
* \param  rom Todo.
*
* \return Game name.
*/
/* ******************************************************************************************************************/
static void dr_free_roms ( struct_gngeoxroms_game_roms* rom )
{
    free_region ( &rom->rom_region[REGION_MAIN_CPU_CARTRIDGE] );
    free_region ( &rom->rom_region[REGION_AUDIO_CPU_ENCRYPTED] );

    if ( !neogeo_memory.vid.spr_cache.data )
    {
        free_region ( &rom->rom_region[REGION_SPRITES] );
    }
    else
    {
        fclose ( neogeo_memory.vid.spr_cache.gno );
        free_sprite_cache();
        qalloc_delete ( neogeo_memory.vid.spr_cache.offset );
    }

    free_region ( &rom->rom_region[REGION_FIXED_LAYER_CARTRIDGE] );
    free_region ( &rom->rom_region[REGION_AUDIO_CPU_CARTRIDGE] );

    if ( rom->rom_region[REGION_AUDIO_DATA_2].p != rom->rom_region[REGION_AUDIO_DATA_1].p )
    {
        free_region ( &rom->rom_region[REGION_AUDIO_DATA_2] );
    }
    else
    {
        rom->rom_region[REGION_AUDIO_DATA_2].p = NULL;
        rom->rom_region[REGION_AUDIO_DATA_2].size = 0;
    }

    free_region ( &rom->rom_region[REGION_AUDIO_DATA_1] );

    free_region ( &rom->rom_region[REGION_MAIN_CPU_BIOS] );

    qalloc_delete ( neogeo_memory.ng_lo );
    qalloc_delete ( neogeo_memory.fix_board_usage );
    qalloc_delete ( neogeo_memory.fix_game_usage );

    free_region ( &rom->spr_usage );

    qalloc_delete ( rom->info.name );
    qalloc_delete ( rom->info.longname );
}
/* ******************************************************************************************************************/
/*!
* \brief  Closes game.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*
*/
/* ******************************************************************************************************************/
void close_game ( void )
{
    save_nvram ( );
    save_memcard ( );

    dr_free_roms ( &neogeo_memory.rom );

    neo_transpack_close();
}
/* ******************************************************************************************************************/
/*!
* \brief  Loads game config.
*
* \param rom_name Todo.
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*
*/
/* ******************************************************************************************************************/
SDL_bool init_game ( char* rom_name )
{
    /* Open transpack if needed */
    if ( neo_transpack_init ( ) == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }

    if ( strstr ( rom_name, ".gno" ) != NULL )
    {
        if ( dr_open_gno ( rom_name ) == SDL_FALSE )
        {
            return ( SDL_FALSE );
        }
    }
    else
    {
        if ( dr_load_game ( rom_name ) == SDL_FALSE )
        {
            zlog_error ( gngeox_config.loggingCat, "Can't load %s", rom_name );
            return ( SDL_FALSE );
        }
    }

    open_nvram ( );
    open_memcard ( );

    neo_screen_windowtitle_set ( );

    if ( neo_sys_init( ) == SDL_FALSE )
    {
        return ( SDL_TRUE );
    }

    setup_misc_patch ( );

    fix_usage = neogeo_memory.fix_board_usage;
    current_pal = neogeo_memory.vid.pal_neo[0];
    current_fix = neogeo_memory.rom.rom_region[REGION_FIXED_LAYER_BIOS].p;
    current_pc_pal = ( Uint32* ) neogeo_memory.vid.pal_host[0];

    neogeo_memory.vid.currentpal = 0;
    neogeo_memory.vid.currentfix = 0;

    return ( SDL_TRUE );
}

#ifdef _GNGEOX_ROMS_C_
#undef _GNGEOX_ROMS_C_
#endif // _GNGEOX_ROMS_C_
