/*!
*
*   \file    GnGeoXeffects.h
*   \brief   .
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    12/04/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_EFFECTS_H_
#define _GNGEOX_SCREEN_H_

typedef struct
{
    const char* name;
    const char* desc;
    Uint8 x_ratio, y_ratio;
    SDL_bool ( *init ) ();
    void ( *update ) ();
} struct_gngeoxscreen_effect_func;

#ifndef _GNGEOX_EFFECTS_C_
extern struct_gngeoxscreen_effect_func effect[];
#else
static SDL_bool effect_none_init ( void ) __attribute__ ( ( warn_unused_result ) );
#endif // _GNGEOX_EFFECTS_C_

void print_effect_list ( void );
Uint8 get_effect_by_name ( const char* ) __attribute__ ( ( warn_unused_result ) );

#endif
