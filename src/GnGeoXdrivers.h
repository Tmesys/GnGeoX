/*!
*
*   \file    GnGeoXdrivers.h
*   \brief   Game description driver and resources loader.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    22/10/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    This effect is a rewritten implementation of the hq2x effect made by Maxim Stepin.
*/
#ifndef _GNGEOX_DRIVERS_H_
#define _GNGEOX_DRIVERS_H_

typedef struct
{
    bstring filename;
    Uint8 region;
    Uint32 src;
    Uint32 dest;
    Uint32 size;
    Uint32 crc;
} struct_gngeoxdrivers_rom_file;

typedef struct
{
    bstring name;
    bstring parent;
    bstring longname;
    Uint32 year;
    Uint32 romsize[10];
    Uint32 nb_romfile;
    struct_gngeoxdrivers_rom_file rom[32];
} struct_gngeoxdrivers_rom_def;

struct_gngeoxdrivers_rom_def* neo_driver_load ( char* );

#endif
