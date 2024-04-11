/*!
*
*   \file    GnGeoXromsgno.h
*   \brief   ROMS gno format.
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
#ifndef _GNGEOX_ROMSGNO_H_
#define _GNGEOX_ROMSGNO_H_

#ifdef _GNGEOX_ROMSGNO_C_
static SDL_bool dump_region ( FILE*, const struct_gngeoxroms_rom_region*, Uint8, Uint8, Uint32 ) __attribute__ ( ( warn_unused_result ) );
static SDL_bool read_region ( FILE*, struct_gngeoxroms_game_roms* ) __attribute__ ( ( warn_unused_result ) );
#endif // _GNGEOX_ROMSGNO_C_

Sint32 dr_open_gno ( const char* ) __attribute__ ( ( warn_unused_result ) );
SDL_bool dr_save_gno ( struct_gngeoxroms_game_roms*, const char* ) __attribute__ ( ( warn_unused_result ) );
char* dr_gno_romname ( const char* );

#endif
