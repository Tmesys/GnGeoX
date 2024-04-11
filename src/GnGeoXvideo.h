/*!
*
*   \file    GnGeoXvideo.h
*   \brief   Video routines header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    17/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_VIDEO_H_
#define _GNGEOX_VIDEO_H_

#define PIXEL_PITCH (sdl_surface_buffer->pitch >> 2)
#define RASTER_LINES 261
#define PEN_USAGE(tileno) ((((Uint32*) neogeo_memory.rom.spr_usage.p)[tileno>>4]>>((tileno&0xF)*2))&0x3)

#define COLOR_RGB24_B(_X_) (_X_ & 0xFF)
#define COLOR_RGB24_G(_X_) ((_X_>>8) & 0xFF)
#define COLOR_RGB24_R(_X_) ((_X_>>16) & 0xFF)
#define COLOR_RGB24_MAKE(_A_,_R_,_G_,_B_) ((Uint32)( _A_ << 24 ) | (Uint32)( _R_ << 16 ) | (Uint32)( _G_ << 8 ) | (Uint32)(_B_) )

#define BLEND16_50(a,b) alpha_blend(a,b,127)
#define BLEND16_25(a,b) alpha_blend(a,b,63)
#define fix_add(x, y) ((((READ_WORD(neogeo_memory.vid.ram + 0xEA00 + (((y-1)&31)*2 + 64 * (x/6))) >> (5-(x%6))*2) & 3) ^ 3))

typedef struct
{
    Uint8* data;  /* The cache */
    Uint32 size;  /* Tha allocated size of the cache */
    Uint32 total_bank;  /* total number of rom bank */
    Uint8** ptr/*[TOTAL_GFX_BANK]*/; /* ptr[i] Contain a pointer to cached data for bank i */
    Sint32 max_slot; /* Maximal numer of bank that can be cached (depend on cache size) */
    Sint32 slot_size;
    Sint32* usage;   /* contain index to the banks in used order */
    FILE* gno;
    Uint32* offset;
    Uint8* in_buf;
} struct_gngeoxvideo_gfx_cache;

typedef struct
{
    /* Video Ram&Pal */
    Uint8 ram[0x20000];
    Uint8 pal_neo[2][0x2000];
    Uint8 pal_host[2][0x4000];
    Uint8 currentpal;
    Uint8 currentfix; /* 0=bios fix */
    Uint16 rbuf;

    /* Auto anim counter */
    Uint32 fc;
    Uint32 fc_speed;

    Uint32 vptr;
    Sint16 modulo;

    /* IRQ2 related */
    Uint32 irq2control;
    Uint32 irq2taken;
    Uint32 irq2start;
    Uint32 irq2pos;

    struct_gngeoxvideo_gfx_cache spr_cache;
} struct_gngeoxvideo_video;

#ifdef _GNGEOX_VIDEO_C_
static Uint32 alpha_blend ( Uint32, Uint32, Uint8 ) __attribute__ ( ( warn_unused_result ) );
static Uint8* get_cached_sprite_ptr ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static void fix_value_init ( void );
static void draw_fix_char ( Uint8*, Sint32, Sint32 );
#else
extern Uint32 neogeo_frame_counter;
extern Uint32 neogeo_frame_counter_speed;
extern Uint32 frame_counter;
#endif // _GNGEOX_VIDEO_C_

void init_video ( void );
SDL_bool init_sprite_cache ( Uint32, Uint32 ) __attribute__ ( ( warn_unused_result ) );
void free_sprite_cache ( void );
void draw_screen ( void );
void draw_screen_scanline ( Sint32, Sint32, Sint32 );
void SDL_textout ( SDL_Surface*, Sint32, Sint32, const char* );

#endif
