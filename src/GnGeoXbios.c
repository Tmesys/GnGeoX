/*!
*
*   \file    GnGeoXbios.c
*   \brief   BIOS loading.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    13/03/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_BIOS_C_
#define _GNGEOX_BIOS_C_
#endif // _GNGEOX_BIOS_C_

#include <SDL2/SDL.h>
#include "zlog.h"
#include "qlibc.h"
#include "bstrlib.h"

#include "GnGeoXroms.h"
#include "GnGeoXbios.h"
#include "GnGeoXconfig.h"
#include "GnGeoXemu.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"

/* ******************************************************************************************************************/
/*!
* \brief  Converts all rom chars.
*
* \param  rom Todo.
*
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
SDL_bool neo_bios_load ( struct_gngeoxroms_game_roms* rom )
{
    qzip_file_t *zip_file = NULL;
    Uint32 size = 0;
    bstring fpath = NULL;
    const char* romfile;
    void *buffer = NULL;

    fpath = bfromcstr ( gngeox_config.biospath );
    bcatcstr ( fpath, "/neogeo.zip" );

    zip_file = qzip_open_file ( fpath->data );
    if ( zip_file == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Opening file : %s", fpath->data );
        bdestroy ( fpath );
        return ( SDL_FALSE );
    }

    bdestroy ( fpath );

    buffer = qzip_load_entry ( zip_file, "000-lo.lo", &size );
    if ( buffer == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Could not find 000-lo.lo ! Please check your bios" );
        qzip_close_file ( zip_file );
        return ( SDL_FALSE );
    }
    neogeo_memory.ng_lo = ( Uint8 * ) buffer;
    zlog_info ( gngeox_config.loggingCat, "Load 000-lo ROM OK size %d", size );

    buffer = qzip_load_entry ( zip_file, "sfix.sfix", &rom->rom_region[REGION_FIXED_LAYER_BIOS].size );
    if ( buffer == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Could not find sfix.sfix ! Please check your bios" );
        qzip_close_file ( zip_file );
        return ( SDL_FALSE );
    }
    rom->rom_region[REGION_FIXED_LAYER_BIOS].p = ( Uint8 * ) buffer;

    /* convert bios fix char */
    neogeo_memory.fix_board_usage = ( Uint8* ) qalloc ( neogeo_memory.rom.rom_region[REGION_FIXED_LAYER_BIOS].size );
    if ( neogeo_memory.fix_board_usage == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Not enough memory ! Requesting %d bytes", neogeo_memory.rom.rom_region[REGION_FIXED_LAYER_BIOS].size );
        qzip_close_file ( zip_file );
        return ( SDL_FALSE );
    }

    convert_all_char ( REGION_FIXED_LAYER_BIOS, neogeo_memory.fix_board_usage );
    zlog_info ( gngeox_config.loggingCat, "Load SFIX ROM OK size %d", rom->rom_region[REGION_FIXED_LAYER_BIOS].size );

    if ( rom->rom_region[REGION_MAIN_CPU_BIOS].p == NULL )
    {
        switch ( gngeox_config.systemtype )
        {
        case ( SYS_UNIBIOS ) :
            {
                romfile = "uni-bios.rom";
            }
            break;
        case ( SYS_HOME_AES ) :
            {
                romfile = "aes-bios.bin";
            }
            break;
        case ( SYS_ARCADE_MVS ) :
            {
                switch ( gngeox_config.country )
                {
                case ( CTY_JAPAN ) :
                    {
                        romfile = "vs-bios.rom";
                    }
                    break;

                case ( CTY_USA ) :
                    {
                        romfile = "sp-u2.sp1";
                    }
                    break;

                case ( CTY_ASIA ) :
                    {
                        romfile = "asia-s3.rom";
                    }
                    break;

                default:
                    {
                        romfile = "sp-s2.sp1";
                    }
                    break;
                }
            }
            break;
        case ( SYS_CHECK ) :
            {
                romfile = "checksys.sp1";
            }
            break;
        default:
            {
                zlog_error ( gngeox_config.loggingCat, "Unknown system %d ! Please check your configuration file", gngeox_config.systemtype );
                qzip_close_file ( zip_file );
                return ( SDL_FALSE );
            }
            break;
        }

        buffer = qzip_load_entry ( zip_file, romfile, &rom->rom_region[REGION_MAIN_CPU_BIOS].size );
        if ( buffer == NULL )
        {
            zlog_error ( gngeox_config.loggingCat, "Could not find %s ! Please check your bios", romfile );
            qzip_close_file ( zip_file );
            return ( SDL_FALSE );
        }
        rom->rom_region[REGION_MAIN_CPU_BIOS].p = ( Uint8 * ) buffer;
        zlog_info ( gngeox_config.loggingCat, "Load %s ROM OK size %d", romfile, rom->rom_region[REGION_MAIN_CPU_BIOS].size );
    }

    qzip_close_file ( zip_file );

    return ( SDL_TRUE );
}

#ifdef _GNGEOX_BIOS_C_
#undef _GNGEOX_BIOS_C_
#endif // _GNGEOX_BIOS_C_
