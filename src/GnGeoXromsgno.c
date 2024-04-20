/*!
*
*   \file    GnGeoXromsgno.c
*   \brief   ROMS gno format.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    14/03/2024
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_ROMSGNO_C_
#define _GNGEOX_ROMSGNO_C_
#endif // _GNGEOX_ROMSGNO_C_

#include <SDL2/SDL.h>
#include "zlog.h"
#include "qlibc.h"
#include "bstrlib.h"

#include "GnGeoXroms.h"
#include "GnGeoXromsinit.h"
#include "GnGeoXromsgno.h"
#include "GnGeoXbios.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXconfig.h"

/* ******************************************************************************************************************/
/*!
* \brief  Dumps rom region.
*
* \param  gno Todo.
* \param  rom_region Todo.
* \param  idx Todo.
* \param  type Todo.
* \param  block_size Todo.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
static SDL_bool dump_region ( FILE* gno, const struct_gngeoxroms_rom_region* rom_region, Uint8 idx, Uint8 type, Uint32 block_size )
{
    if ( rom_region->p == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Rom region NULL" );
        return ( SDL_FALSE );
    }

    fwrite ( &rom_region->size, sizeof ( Uint32 ), 1, gno );
    fwrite ( &idx, sizeof ( Uint8 ), 1, gno );
    fwrite ( &type, sizeof ( Uint8 ), 1, gno );

    if ( type == 0 )
    {
        zlog_info ( gngeox_config.loggingCat, "Dump region id %d size %d", idx, rom_region->size );
        fwrite ( rom_region->p, rom_region->size, 1, gno );
    }
    else
    {
        Uint32* block_offset = NULL;
        Uint32 nb_block = ( rom_region->size / block_size );
        Uint32 cur_offset = 0;
        Sint64 offset_pos = 0;
        const Uint8* inbuf = rom_region->p;
        Uint8* outbuf = NULL;
        /* @note (Tmesys#1#12/17/2022): Use SDL types. */
        unsigned long outbuf_len = 0;
        unsigned long outlen = 0;
        Uint32 outlen32 = 0;
        Uint32 cmpsize = 0;
        Sint32 return_code = 0;

        fwrite ( &block_size, sizeof ( Uint32 ), 1, gno );

        if ( ( rom_region->size & ( block_size - 1 ) ) != 0 )
        {
            zlog_warn ( gngeox_config.loggingCat, "Incompatible region size %d with Block size %d", rom_region->size, block_size );
        }

        block_offset = ( Uint32* ) qalloc ( nb_block * sizeof ( Uint32 ) );
        /* @note (Tmesys#1#12/17/2022): Zlib compress output buffer need to be at least the size of inbuf + 0.1% + 12 byte. */
        outbuf_len = compressBound ( block_size );
        outbuf = ( Uint8* ) qalloc ( outbuf_len );
        offset_pos = ftell ( gno );
        fseek ( gno, nb_block * 4 + 4, SEEK_CUR ); /* Skip all the offset table + the total compressed size */

        for ( Uint32 i = 0; i < nb_block; i++ )
        {
            cur_offset = ftell ( gno );
            block_offset[i] = cur_offset;
            outlen = outbuf_len;
            return_code = compress ( outbuf, &outlen, inbuf, block_size );
            printf ( "%d %ld\n", return_code, outlen );
            //cur_offset += outlen;
            cmpsize += outlen;
            inbuf += block_size;
            outlen32 = ( Uint32 ) outlen;
            fwrite ( &outlen32, sizeof ( Uint32 ), 1, gno );
            fwrite ( outbuf, outlen, 1, gno );
        }

        qalloc_delete ( outbuf );
        /* Now, write the offset table */
        fseek ( gno, offset_pos, SEEK_SET );
        fwrite ( block_offset, sizeof ( Uint32 ), nb_block, gno );
        qalloc_delete ( block_offset );
        fwrite ( &cmpsize, sizeof ( Uint32 ), 1, gno );
        fseek ( gno, 0, SEEK_END );
        offset_pos = ftell ( gno );
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Saves gno.
*
* \param  rom Todo.
* \param  filename Todo.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
SDL_bool dr_save_gno ( struct_gngeoxroms_game_roms* rom, const char* filename )
{
    FILE* gno_file = NULL;
    const char* fid = "gnodmpv1";
    char fname[9];
    Uint8 nb_sec = 0;

    gno_file = fopen ( filename, "wb" );
    if ( !gno_file )
    {
        return ( SDL_FALSE );
    }

    /* restore game vector */
    memcpy ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p, neogeo_memory.game_vector, 0x80 );

    if ( rom->rom_region[REGION_MAIN_CPU_CARTRIDGE].p )
    {
        nb_sec++;
    }

    if ( rom->rom_region[REGION_AUDIO_CPU_CARTRIDGE].p )
    {
        nb_sec++;
    }

    if ( rom->rom_region[REGION_AUDIO_DATA_1].p )
    {
        nb_sec++;
    }

    if ( rom->rom_region[REGION_AUDIO_DATA_2].p && ( rom->rom_region[REGION_AUDIO_DATA_2].p != rom->rom_region[REGION_AUDIO_DATA_1].p ) )
    {
        nb_sec++;
    }

    if ( rom->rom_region[REGION_FIXED_LAYER_CARTRIDGE].p )
    {
        nb_sec++;
    }

    if ( rom->rom_region[REGION_SPRITES].p )
    {
        /* @note (Tmesys#1#12/17/2022): Sprite + Sprite usage. */
        nb_sec += 2;
    }

    if ( rom->rom_region[REGION_FIXED_LAYER_CARTRIDGE].p )
    {
        nb_sec++;
    }

    /* Do we need Custom Bios? */
    if ( ( rom->info.flags & HAS_CUSTOM_CPU_BIOS ) )
    {
        nb_sec++;
    }

    /* Header information */
    fwrite ( fid, 8, 1, gno_file );
    snprintf ( fname, 9, "%-8s", rom->info.name );
    fwrite ( fname, 8, 1, gno_file );
    fwrite ( &rom->info.flags, sizeof ( Uint32 ), 1, gno_file );
    fwrite ( &nb_sec, sizeof ( Uint8 ), 1, gno_file );

    /* Now each section */
    dump_region ( gno_file, &rom->rom_region[REGION_MAIN_CPU_CARTRIDGE], REGION_MAIN_CPU_CARTRIDGE, 0, 0 );
    dump_region ( gno_file, &rom->rom_region[REGION_AUDIO_CPU_CARTRIDGE], REGION_AUDIO_CPU_CARTRIDGE, 0, 0 );
    dump_region ( gno_file, &rom->rom_region[REGION_AUDIO_DATA_1], REGION_AUDIO_DATA_1, 0, 0 );

    if ( rom->rom_region[REGION_AUDIO_DATA_1].p != rom->rom_region[REGION_AUDIO_DATA_2].p )
    {
        dump_region ( gno_file, &rom->rom_region[REGION_AUDIO_DATA_2], REGION_AUDIO_DATA_2, 0, 0 );
    }

    dump_region ( gno_file, &rom->rom_region[REGION_FIXED_LAYER_CARTRIDGE], REGION_FIXED_LAYER_CARTRIDGE, 0, 0 );
    dump_region ( gno_file, &rom->spr_usage, REGION_SPR_USAGE, 0, 0 );

    if ( ( rom->info.flags & HAS_CUSTOM_CPU_BIOS ) )
    {
        dump_region ( gno_file, &rom->rom_region[REGION_MAIN_CPU_BIOS], REGION_MAIN_CPU_BIOS, 0, 0 );
    }

    /* @fixme (Tmesys#1#12/17/2022): there is a bug in the loading routine, only one compressed (type 1) region can be present at the end of the file. */
    dump_region ( gno_file, &rom->rom_region[REGION_SPRITES], REGION_SPRITES, 1, 4096 );

    fclose ( gno_file );

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Reads region.
*
* \param  gno Todo.
* \param  roms Todo.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
static SDL_bool read_region ( FILE* gno, struct_gngeoxroms_game_roms* roms )
{
    Uint32 size = 0;
    Uint8 lid = 0, type = 0;
    struct_gngeoxroms_rom_region* rom_region = NULL;
    size_t totread = 0;
    Uint32 cache_size[] = {64, 32, 24, 16, 8, 6, 4, 2, 1, 0};

    /* Read region header */
    totread = fread ( &size, sizeof ( Uint32 ), 1, gno );
    totread += fread ( &lid, sizeof ( Uint8 ), 1, gno );
    totread += fread ( &type, sizeof ( Uint8 ), 1, gno );

    switch ( lid )
    {
    case ( REGION_MAIN_CPU_CARTRIDGE ) :
        {
            rom_region = &roms->rom_region[REGION_MAIN_CPU_CARTRIDGE];
        }
        break;

    case ( REGION_AUDIO_CPU_CARTRIDGE ) :
        {
            rom_region = &roms->rom_region[REGION_AUDIO_CPU_CARTRIDGE];
        }
        break;

    case ( REGION_AUDIO_DATA_1 ) :
        {
            rom_region = &roms->rom_region[REGION_AUDIO_DATA_1];
        }
        break;

    case ( REGION_AUDIO_DATA_2 ) :
        {
            rom_region = &roms->rom_region[REGION_AUDIO_DATA_2];
        }
        break;

    case ( REGION_FIXED_LAYER_CARTRIDGE ) :
        {
            rom_region = &roms->rom_region[REGION_FIXED_LAYER_CARTRIDGE];
        }
        break;

    case ( REGION_SPRITES ) :
        {
            rom_region = &roms->rom_region[REGION_SPRITES];
        }
        break;

    case ( REGION_SPR_USAGE ) :
        {
            rom_region = &roms->spr_usage;
        }
        break;

    case ( REGION_FIXED_LAYER_BIOS ) :
        {
            rom_region = &roms->rom_region[REGION_FIXED_LAYER_BIOS];
        }
        break;

    case ( REGION_MAIN_CPU_BIOS ) :
        {
            rom_region = &roms->rom_region[REGION_MAIN_CPU_BIOS];
        }
        break;

    default:
        {
            zlog_error ( gngeox_config.loggingCat, "Unknown region %d", lid );
            return ( SDL_FALSE );
        }
        break;
    }

    if ( type == 0 )
    {
        /* @todo (Tmesys#1#12/17/2022): Support ADPCM streaming for platform with less than 64MB of Mem. */
        allocate_region ( rom_region, size, lid );
        totread += fread ( rom_region->p, rom_region->size, 1, gno );
    }
    else
    {
        Uint32 nb_block = 0, block_size = 0;
        Uint32 cmp_size = 0;

        totread += fread ( &block_size, sizeof ( Uint32 ), 1, gno );
        nb_block = size / block_size;

        rom_region->size = size;

        neogeo_memory.vid.spr_cache.offset = ( Uint32* ) qalloc ( nb_block * sizeof ( Uint32 ) );
        totread += fread ( neogeo_memory.vid.spr_cache.offset, sizeof ( Uint32 ), nb_block, gno );
        neogeo_memory.vid.spr_cache.gno = gno;

        totread += fread ( &cmp_size, sizeof ( Uint32 ), 1, gno );

        fseek ( gno, cmp_size, SEEK_CUR );

        /* TODO: Find the best cache size dynamically! */
        for ( Sint32 i = 0; cache_size[i] != 0; i++ )
        {
            if ( init_sprite_cache ( cache_size[i] * 1024 * 1024, block_size ) == SDL_TRUE )
            {
                break;
            }
        }
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Opens gno.
*
* \param  filename Todo.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
Sint32 dr_open_gno ( const char* filename )
{
    FILE* gno = NULL;
    char fid[9]; // = "gnodmpv1";
    char name[9] = {0,};
    struct_gngeoxroms_game_roms* rom = &neogeo_memory.rom;
    Uint8 nb_sec = 0;
    char* a = NULL;
    size_t totread = 0;

    neogeo_memory.bksw_handler = 0;
    neogeo_memory.bksw_unscramble = NULL;
    neogeo_memory.bksw_offset = NULL;

    rom->need_decrypt = SDL_FALSE;

    gno = fopen ( filename, "rb" );

    if ( !gno )
    {
        return ( SDL_FALSE );
    }

    totread += fread ( fid, 8, 1, gno );

    if ( strncmp ( fid, "gnodmpv1", 8 ) != 0 )
    {
        fclose ( gno );
        zlog_error ( gngeox_config.loggingCat, "Invalid GNO file" );
        return ( SDL_FALSE );
    }

    totread += fread ( name, 8, 1, gno );
    a = strchr ( name, ' ' );

    if ( a )
    {
        a[0] = 0;
    }

    rom->info.name = strdup ( name );

    totread += fread ( &rom->info.flags, sizeof ( Uint32 ), 1, gno );
    totread += fread ( &nb_sec, sizeof ( Uint8 ), 1, gno );

    for ( Sint32 i = 0; i < nb_sec; i++ )
    {
        read_region ( gno, rom );
    }

    if ( rom->rom_region[REGION_AUDIO_DATA_2].p == NULL )
    {
        rom->rom_region[REGION_AUDIO_DATA_2].p = rom->rom_region[REGION_AUDIO_DATA_1].p;
        rom->rom_region[REGION_AUDIO_DATA_2].size = rom->rom_region[REGION_AUDIO_DATA_1].size;
    }

    neogeo_memory.nb_of_tiles = rom->rom_region[REGION_SPRITES].size >> 7;

    /* Init rom and bios */
    init_roms ( rom );

    if ( neo_bios_load ( rom ) == SDL_FALSE )
    {
        return ( SDL_FALSE );
    }

    memcpy ( neogeo_memory.game_vector, neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p, 0x80 );
    memcpy ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p, neogeo_memory.rom.rom_region[REGION_MAIN_CPU_BIOS].p, 0x80 );

    init_video();

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief  Opens gno rom name.
*
* \param  filename Todo.
*
* \return Game name.
*/
/* ******************************************************************************************************************/
char* dr_gno_romname ( const char* filename )
{
    FILE* gno = NULL;
    char fid[9]; // = "gnodmpv1";
    char name[9] = {0,};
    char* space = NULL;
    size_t totread = 0;

    gno = fopen ( filename, "rb" );
    if ( !gno )
    {
        return ( NULL );
    }

    totread += fread ( fid, 8, 1, gno );

    if ( strncmp ( fid, "gnodmpv1", 8 ) != 0 )
    {
        zlog_error ( gngeox_config.loggingCat, "Invalid GNO file" );
        fclose ( gno );
        return ( NULL );
    }

    totread += fread ( name, 8, 1, gno );

    space = strchr ( name, ' ' );

    if ( space != NULL )
    {
        space[0] = 0;
    }

    fclose ( gno );

    return ( strdup ( name ) );
}

#ifdef _GNGEOX_ROMSGNO_C_
#undef _GNGEOX_ROMSGNO_C_
#endif // _GNGEOX_ROMSGNO_C_
