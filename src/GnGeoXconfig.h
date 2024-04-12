/*!
*
*   \file    GnGeoXconfig.h
*   \brief   New configuration routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    09/10/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_CONFIG_H_
#define _GNGEOX_CONFIG_H_

typedef enum
{
    SYS_ARCADE_MVS = 0,
    SYS_HOME_AES = 1,
    SYS_UNIBIOS = 2,
    SYS_MAX = 3,
} SYSTEM;

typedef enum COUNTRY
{
    CTY_JAPAN = 0,
    CTY_EUROPE = 1,
    CTY_USA = 2,
    CTY_ASIA = 3,
    CTY_MAX = 4
} COUNTRY;

typedef struct
{
    char* gamename;
    char* rompath;
    char* biospath;
    char* shaderpath;
    char* nvrampath;
    char* savespath;
    char* blitter;
    char* effect;
    Uint16 scale;
    SDL_bool fullscreen;
    SDL_bool interpolation;
    SDL_bool showfps;
    SDL_bool autoframeskip;
    SDL_bool vsync;
    SDL_bool raster;
    SDL_bool forcepal;
    SDL_bool transpack;
    Uint16 country;
    Uint16 systemtype;
    Uint16 samplerate;
    SDL_bool debug;
    SDL_bool dump;
    /* @todo (Tmesys#1#10/04/2024): Not implemented. */
    SDL_bool joystick;
    zlog_category_t* loggingCat;
    Uint16 res_x;
    Uint16 res_y;
    Uint8 extra_xor;
    Uint8 blitter_index;
    Uint8 effect_index;
} struct_gngeoxconfig_params;

#ifndef _GNGEOX_CONFIG_C_
extern struct_gngeoxconfig_params gngeox_config;
#endif // _GNGEOX_CONFIG_C_

SDL_bool neo_config_init ( char* ) __attribute__ ( ( warn_unused_result ) );
SDL_bool neo_config_parse_options ( int *, char *** ) __attribute__ ( ( warn_unused_result ) );
void neo_config_close ( void );

#endif
