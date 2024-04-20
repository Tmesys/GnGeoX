/*!
*
*   \file    GnGeoXmamelayer.c
*   \brief   MAME compatibility layer ?
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version).
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
#ifndef _GNGEOX_MAME_LAYER_C_
#define _GNGEOX_MAME_LAYER_C_
#endif // _GNGEOX_MAME_LAYER_C_

#include <string.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include "zlog.h"
#include "qlibc.h"

#include "GnGeoXroms.h"
#include "GnGeoXmamelayer.h"
#include "GnGeoXconfig.h"

/* ******************************************************************************************************************/
/*!
* \brief Retrieves pointer to game memory region.
*
* \param rom Game rom.
* \param region Memory region.
* \return Memory region pointer.
*/
/* ******************************************************************************************************************/
Uint8* memory_region ( struct_gngeoxroms_game_roms* rom, enum_gngeoxmamelayer_memoryregion region )
{
    return ( rom->rom_region[region].p );
}
/* ******************************************************************************************************************/
/*!
* \brief Retrieves game memory region length.
*
* \param rom Game rom.
* \param region Memory region.
* \return Memory region length.
*/
/* ******************************************************************************************************************/
Uint32 memory_region_length ( struct_gngeoxroms_game_roms* rom, enum_gngeoxmamelayer_memoryregion region )
{
    return ( rom->rom_region[region].size );
}
/* ******************************************************************************************************************/
/*!
* \brief Allocates .
*
* \param size Memory size to allocate in bytes.
* \return Pointer to allocated memory, NULL in case of error.
*/
/* ******************************************************************************************************************/
/* @fixme (Tmesys#1#12/04/2022): I should change this to some SDL2 routine. */
void* malloc_or_die ( Uint32 size )
{
    void* memory = qalloc ( size );

    if ( memory )
    {
        return ( memory );
    }

    zlog_error ( gngeox_config.loggingCat, "Not enough memory ! Requesting %d bytes", size );
    exit ( 1 );

    return ( NULL );
}

#ifdef _GNGEOX_MAME_LAYER_C_
#undef _GNGEOX_MAME_LAYER_C_
#endif // _GNGEOX_MAME_LAYER_C_
