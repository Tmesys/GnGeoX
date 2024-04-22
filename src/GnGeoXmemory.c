/*!
*
*   \file    GnGeoXmemory.c
*   \brief   Memory routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    03/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_MEMORY_C_
#define _GNGEOX_MEMORY_C_
#endif // _GNGEOX_MEMORY_C_

#include <SDL2/SDL.h>
#include "zlog.h"
#include "qlibc.h"
#include "bstrlib.h"

#include "GnGeoXvideo.h"
#include "GnGeoXym2610.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXpd4990a.h"
#include "GnGeoXtranspack.h"
#include "GnGeoX68k.h"
#include "GnGeoXz80.h"
#include "GnGeoXscanline.h"
#include "GnGeoXconfig.h"

struct_gngeoxmemory_neogeo neogeo_memory;

/* @fixme (Tmesys#1#12/04/2022): I think that some of these should be incorporated into neogeo_memory struct */
Uint8* current_pal = NULL;
Uint32* current_pc_pal = NULL;
Uint8* current_fix = NULL;
Uint8* fix_usage = NULL;
SDL_bool sram_lock = SDL_FALSE;

/* @note (Tmesys#1#12/04/2022): This one is heavily used in neoboot but commented. */
Uint32 cpu_68k_bankaddress = 0;

static Uint16 neogeo_rng = 0x2345;
/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
* \param data Todo.
*/
/* ******************************************************************************************************************/
void write_neo_control ( Uint16 data )
{
    neogeo_frame_counter_speed = ( ( ( data >> 8 ) & 0xff ) + 1 );
    neogeo_memory.vid.irq2control = data & 0xff;
}
/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
* \param data Todo.
*/
/* ******************************************************************************************************************/
void write_irq2pos ( Uint32 data )
{
    neogeo_memory.vid.irq2pos = data;

    if ( neogeo_memory.vid.irq2control & 0x20 )
    {
        /* @note (Tmesys#1#05/04/2024): turfmast goes as low as 0x145 */
        Sint32 line = ( neogeo_memory.vid.irq2pos + 0x3b ) / 0x180;
        neogeo_memory.vid.irq2start = line + current_line;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
* \param npal Todo.
*/
/* ******************************************************************************************************************/
Uint32 convert_pal ( Uint16 npal )
{
    Sint32 r = 0, g = 0, b = 0, dark = 0;
    r = ( ( npal >> 7 ) & 0x1e ) | ( ( npal >> 14 ) & 0x01 );
    g = ( ( npal >> 3 ) & 0x1e ) | ( ( npal >> 13 ) & 0x01 );
    b = ( ( npal << 1 ) & 0x1e ) | ( ( npal >> 12 ) & 0x01 );
    dark = ( ( npal >> 15 ) & 0x01 ) ^ 1;

    /* 2^6 bits components (neogeo) -> 2^8 bits components (RGB24) */
    r = ( r << 1 | dark ) << 2;
    g = ( g << 1 | dark ) << 2;
    b = ( b << 1 | dark ) << 2;
    return ( r << 16 ) + ( g << 8 ) + b;
}
/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
*/
/* ******************************************************************************************************************/
void update_all_pal ( void )
{
    Sint32 i;
    Uint32* pc_pal1 = ( Uint32* ) neogeo_memory.vid.pal_host[0];
    Uint32* pc_pal2 = ( Uint32* ) neogeo_memory.vid.pal_host[1];

    for ( i = 0; i < 0x1000; i++ )
    {
        //pc_pal1[i] = convert_pal(READ_WORD_ROM(&memory.pal1[i<<1]));
        //pc_pal2[i] = convert_pal(READ_WORD_ROM(&memory.pal2[i<<1]));
        pc_pal1[i] = convert_pal ( READ_WORD ( &neogeo_memory.vid.pal_neo[0][i << 1] ) );
        pc_pal2[i] = convert_pal ( READ_WORD ( &neogeo_memory.vid.pal_neo[1][i << 1] ) );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
* \return old Todo.
*/
/* ******************************************************************************************************************/
Uint16 sma_random ( void )
{
    Uint16 old = neogeo_rng;

    Uint16 newbit = ( ( neogeo_rng >> 2 ) ^ ( neogeo_rng >> 3 ) ^ ( neogeo_rng >> 5 )
                      ^ ( neogeo_rng >> 6 ) ^ ( neogeo_rng >> 7 ) ^ ( neogeo_rng >> 11 )
                      ^ ( neogeo_rng >> 12 ) ^ ( neogeo_rng >> 15 ) ) & 1;

    neogeo_rng = ( neogeo_rng << 1 ) | newbit;

    return old;
}
/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
* \param address Todo.
* \param data Todo.
*/
/* ******************************************************************************************************************/
void switch_bank ( Uint32 address, Uint8 data )
{
    if ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].size <= 0x100000 )
    {
        return;
    }

    if ( address >= 0x2FFFF0 )
    {
        data = data & 0x7;
        cpu_68k_bankaddress = ( data + 1 ) * 0x100000;
    }
    else
    {
        return;
    }

    if ( cpu_68k_bankaddress >= neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].size )
    {
        cpu_68k_bankaddress = 0x100000;
    }

    cpu_68k_bankswitch ( cpu_68k_bankaddress );
}
/* ******************************************************************************************************************/
/*!
* \brief  Opens memcard.
*
* \param  name Todo.
*
*/
/* ******************************************************************************************************************/
void open_memcard ( void )
{
    Uint8 * buffer = NULL;
    bstring fpath = NULL;

    fpath = bfromcstr ( gngeox_config.savespath );
    bcatcstr ( fpath, "/" );
    bcatcstr ( fpath, gngeox_config.gamename );
    bcatcstr ( fpath, ".sav" );

    if ( qfile_exist ( ( const char * ) fpath->data ) == false )
    {
        SDL_zero ( neogeo_memory.memcard );

        if ( qfile_save ( ( const char * ) fpath->data, neogeo_memory.memcard, sizeof ( neogeo_memory.memcard ), false ) == false )
        {
            zlog_error ( gngeox_config.loggingCat, "Can not create file %s", fpath->data );
        }

        bdestroy ( fpath );
        return;
    }

    buffer = qfile_load ( ( const char * ) fpath->data, NULL );
    if ( buffer == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Can not load file %s", fpath->data );
        bdestroy ( fpath );
        return;
    }

    memcpy ( neogeo_memory.memcard, buffer, sizeof ( neogeo_memory.memcard ) );
    free ( buffer );
}
/* ******************************************************************************************************************/
/*!
* \brief  Saves memcard.
*
* \param  name Todo.
*
*/
/* ******************************************************************************************************************/
void save_memcard ( void )
{
    bstring fpath = NULL;

    fpath = bfromcstr ( gngeox_config.savespath );
    bcatcstr ( fpath, "/" );
    bcatcstr ( fpath, gngeox_config.gamename );
    bcatcstr ( fpath, ".sav" );

    if ( qfile_save ( fpath->data, neogeo_memory.memcard, sizeof ( neogeo_memory.memcard ), false ) == false )
    {
        zlog_error ( gngeox_config.loggingCat, "Can not save file %s", fpath->data );
    }

    bdestroy ( fpath );
}
/* ******************************************************************************************************************/
/*!
* \brief  Opens nvram.
*
* \param  name Todo.
*
*/
/* ******************************************************************************************************************/
void open_nvram ( void )
{
    Uint8 * buffer = NULL;
    bstring fpath = NULL;

    fpath = bfromcstr ( gngeox_config.nvrampath );
    bcatcstr ( fpath, "/" );
    bcatcstr ( fpath, gngeox_config.gamename );
    bcatcstr ( fpath, ".nv" );

    if ( qfile_exist ( ( const char* ) fpath->data ) == false )
    {
        SDL_zero ( neogeo_memory.sram );

        if ( qfile_save ( ( const char* ) fpath->data, neogeo_memory.sram, sizeof ( neogeo_memory.sram ), false ) == false )
        {
            zlog_error ( gngeox_config.loggingCat, "Can not create file %s", fpath->data );
        }

        bdestroy ( fpath );
        return;
    }

    buffer = qfile_load ( ( const char* ) fpath->data, NULL );
    if ( buffer == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "Can not load file %s", fpath->data );
        bdestroy ( fpath );
        return;
    }

    memcpy ( neogeo_memory.sram, buffer, sizeof ( neogeo_memory.sram ) );
    free ( buffer );
}
/* ******************************************************************************************************************/
/*!
* \brief  Saves nvram.
*
* \param  name Todo.
*
*/
/* ******************************************************************************************************************/
void save_nvram ( void )
{
    bstring fpath = NULL;

    fpath = bfromcstr ( gngeox_config.nvrampath );
    bcatcstr ( fpath, "/" );
    bcatcstr ( fpath, gngeox_config.gamename );
    bcatcstr ( fpath, ".nv" );

    if ( qfile_save ( ( const char* ) fpath->data, neogeo_memory.sram, sizeof ( neogeo_memory.sram ), false ) == false )
    {
        zlog_error ( gngeox_config.loggingCat, "Can not save file %s", fpath->data );
    }

    bdestroy ( fpath );
}

#ifdef _GNGEOX_MEMORY_C_
#undef _GNGEOX_MEMORY_C_
#endif // _GNGEOX_MEMORY_C_
