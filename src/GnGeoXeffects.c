/*!
*
*   \file    GnGeoXeffects.c
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
#ifndef _GNGEOX_EFFECTS_C_
#define _GNGEOX_EFFECTS_C_
#endif // _GNGEOX_EFFECTS_C_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "zlog.h"
#include "qlibc.h"

#include "GnGeoXeffects.h"
#include "GnGeoXhq2x.h"
#include "GnGeoXxbr2x.h"
#include "GnGeoXscale.h"
#include "GnGeoXscanline.h"
#include "GnGeoXconfig.h"

struct_gngeoxscreen_effect_func effect[] =
{
    {"none", "No effect", 1, 1, effect_none_init, NULL},
    {"scanline", "Scanline effect", 2, 2, effect_scanline_init, effect_scanline_update}, // 1
    {"scanline50", "Scanline 50% effect", 2, 2, effect_scanline_init, effect_scanline50_update}, // 2
    {"scale2x", "Scale2x effect", 2, 2, effect_scale2x_init, effect_scale2x_update}, // 3
    {"scale3x", "Scale3x effect", 3, 3, effect_scale3x_init, effect_scale3x_update}, // 3
    {"scale4x", "Scale4x effect", 4, 4, effect_scale4x_init, effect_scale4x_update}, // 3
    {"hq2x", "HQ2X effect. High quality", 2, 2, effect_hq2x_init, effect_hq2x_update},
    {"xbr2x", "XBR2X effect. High quality", 2, 2, effect_xbr2x_init, effect_xbr2x_update},
    {"doublex", "Double the x resolution (soft blitter only)", 2, 1, effect_scanline_init, effect_doublex_update}, //6
    {NULL, NULL, 0, 0, NULL, NULL}
};
/* ******************************************************************************************************************/
/*!
* \brief  Prints effects list.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
void print_effect_list ( void )
{
    Sint32 i = 0;

    while ( effect[i].name != NULL )
    {
        zlog_info ( gngeox_config.loggingCat, "%-12s : %s", effect[i].name, effect[i].desc );
        i++;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Gets effect by name.
*
* \return  Effect index.
*/
/* ******************************************************************************************************************/
Uint8 get_effect_by_name ( const char* name )
{
    Sint32 i = 0;

    while ( effect[i].name != NULL )
    {
        if ( !strcmp ( effect[i].name, name ) )
        {
            return i;
        }

        i++;
    }

    /* invalid effect */
    zlog_error ( gngeox_config.loggingCat, "Invalid effect" );
    return ( 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes no effects.
*
* \return Always SDL_TRUE.
*/
/* ******************************************************************************************************************/
static SDL_bool effect_none_init ( void )
{
    return ( SDL_TRUE );
}


#ifdef _GNGEOX_EFFECTS_C_
#undef _GNGEOX_EFFECTS_C_
#endif // _GNGEOX_EFFECTS_C_
