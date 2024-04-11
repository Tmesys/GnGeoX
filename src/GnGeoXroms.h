/*!
*
*   \file    GnGeoXroms.h
*   \brief   Rom general routines header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.40 (final beta)
*   \date    04/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    Many of the NeoGeo bootlegs use their own form of encryption and
*            protection, presumably to make them harder for other bootleggers to
*            copy.  This encryption often involves non-trivial scrambling of the
*            program roms and the games are protected using an ALTERA chip which
*            provides some kind of rom overlay, patching parts of the code.
*            The graphics roms are usually scrambled in a different way to the
*            official SNK cartridges too.
*/
/* Roms/Ram driver interface */
#ifndef _GNGEOX_ROMS_H_
#define _GNGEOX_ROMS_H_

#define HAS_CUSTOM_CPU_BIOS 0x1
#define HAS_CUSTOM_AUDIO_BIOS 0x2

#define LOAD_BUF_SIZE (128*1024)

typedef enum
{
    REGION_AUDIO_CPU_BIOS        = 0,
    REGION_AUDIO_CPU_CARTRIDGE   = 1,
    REGION_AUDIO_CPU_ENCRYPTED   = 2,
    REGION_AUDIO_DATA_1          = 3,
    REGION_AUDIO_DATA_2          = 4,
    REGION_FIXED_LAYER_BIOS      = 5,
    REGION_FIXED_LAYER_CARTRIDGE = 6,
    REGION_MAIN_CPU_BIOS         = 7,
    REGION_MAIN_CPU_CARTRIDGE    = 8,
    REGION_SPRITES               = 9,
    REGION_MAX                   = 10,
    REGION_SPR_USAGE             = 11,
} enum_gngeoxroms_region_type;

typedef struct
{
    char* name;
    char* longname;
    Sint32 year;
    Uint32 flags;
} struct_gngeoxroms_game_info;

typedef struct
{
    Uint8* p;
    Uint32 size;
    enum_gngeoxroms_region_type type;
} struct_gngeoxroms_rom_region;

typedef struct struct_gngeoxroms_game_roms
{
    SDL_bool need_decrypt;
    struct_gngeoxroms_game_info info;
    struct_gngeoxroms_rom_region rom_region[REGION_MAX];
    struct_gngeoxroms_rom_region spr_usage;
} struct_gngeoxroms_game_roms;


#ifdef _GNGEOX_ROMS_C_
static void setup_misc_patch ( void );
static void free_region ( struct_gngeoxroms_rom_region* );
static SDL_bool read_data_i ( qzip_entry_t*, struct_gngeoxroms_rom_region*, Uint32, Uint32 ) __attribute__ ( ( warn_unused_result ) );
static SDL_bool read_data_p ( qzip_entry_t*, struct_gngeoxroms_rom_region*, struct_gngeoxdrivers_rom_file ) __attribute__ ( ( warn_unused_result ) );
static SDL_bool load_region ( qzip_file_t*, struct_gngeoxroms_game_roms*, struct_gngeoxdrivers_rom_def*, Sint32 ) __attribute__ ( ( warn_unused_result ) );
static qzip_file_t* open_rom_zip ( const char*, const char* ) __attribute__ ( ( warn_unused_result ) );
static Sint32 convert_roms_tile ( Uint8*, Sint32 ) __attribute__ ( ( warn_unused_result ) );
static void convert_all_tile ( struct_gngeoxroms_game_roms* );
static SDL_bool dr_load_roms ( struct_gngeoxroms_game_roms* ) __attribute__ ( ( warn_unused_result ) );
static SDL_bool dr_load_game ( char* ) __attribute__ ( ( warn_unused_result ) );
static void dr_free_roms ( struct_gngeoxroms_game_roms* );
static void open_nvram ( void );
static void open_memcard ( void );
static void save_nvram ( void );
static void save_memcard ( void );
#else
extern Sint32 neogeo_fix_bank_type;
#endif // _GNGEOX_ROMS_C_

SDL_bool allocate_region ( struct_gngeoxroms_rom_region*, Uint32, enum_gngeoxroms_region_type ) __attribute__ ( ( warn_unused_result ) );
void convert_all_char ( enum_gngeoxroms_region_type, Uint8* );
SDL_bool init_game ( char* ) __attribute__ ( ( warn_unused_result ) );
void close_game ( void );

#endif
