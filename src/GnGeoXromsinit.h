/*!
*
*   \file    GnGeoXromsinit.h
*   \brief   ROMS special init functions.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    14/03/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_ROMSINIT_H_
#define _GNGEOX_ROMSINIT_H_

typedef struct
{
    char* name;
    void ( *init ) ( struct_gngeoxroms_game_roms* r );
} struct_gngeoxroms_init_func;

#ifdef _GNGEOX_ROMSINIT_C_
static void init_kof99 ( struct_gngeoxroms_game_roms* );
static void init_kof99n ( struct_gngeoxroms_game_roms* );
static void init_garou ( struct_gngeoxroms_game_roms* );
static void init_garouo ( struct_gngeoxroms_game_roms* );
static void init_garoubl ( struct_gngeoxroms_game_roms* );
static void init_mslug3 ( struct_gngeoxroms_game_roms* );
static void init_mslug3h ( struct_gngeoxroms_game_roms* );
static void init_mslug3b6 ( struct_gngeoxroms_game_roms* );
static void init_kof2000 ( struct_gngeoxroms_game_roms* );
static void init_kof2000n ( struct_gngeoxroms_game_roms* );
static void init_kof2001 ( struct_gngeoxroms_game_roms* );
static void init_mslug4 ( struct_gngeoxroms_game_roms* );
static void init_ms4plus ( struct_gngeoxroms_game_roms* );
static void init_ganryu ( struct_gngeoxroms_game_roms* );
static void init_s1945p ( struct_gngeoxroms_game_roms* );
static void init_preisle2 ( struct_gngeoxroms_game_roms* );
static void init_bangbead ( struct_gngeoxroms_game_roms* );
static void init_nitd ( struct_gngeoxroms_game_roms* );
static void init_zupapa ( struct_gngeoxroms_game_roms* );
static void init_sengoku3 ( struct_gngeoxroms_game_roms* );
static void init_kof98 ( struct_gngeoxroms_game_roms* );
static void init_rotd ( struct_gngeoxroms_game_roms* );
static void init_kof2002 ( struct_gngeoxroms_game_roms* );
static void init_kof2002b ( struct_gngeoxroms_game_roms* );
static void init_kf2k2pls ( struct_gngeoxroms_game_roms* );
static void init_kf2k2mp ( struct_gngeoxroms_game_roms* );
static void init_kof2km2 ( struct_gngeoxroms_game_roms* );
static void init_matrim ( struct_gngeoxroms_game_roms* );
static void init_pnyaa ( struct_gngeoxroms_game_roms* );
static void init_mslug5 ( struct_gngeoxroms_game_roms* );
static void init_ms5pcb ( struct_gngeoxroms_game_roms* );
static void init_ms5plus ( struct_gngeoxroms_game_roms* );
#endif // _GNGEOX_ROMSINIT_C_

void init_roms ( struct_gngeoxroms_game_roms* );

#endif
