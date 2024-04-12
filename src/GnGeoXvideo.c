/*!
*
*   \file    GnGeoXvideo.c
*   \brief   Video routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    17/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_VIDEO_C_
#define _GNGEOX_VIDEO_C_
#endif // _GNGEOX_VIDEO_C_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "zlog.h"
#include "qlibc.h"

#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXscreen.h"
#include "GnGeoXframeskip.h"
#include "GnGeoXtranspack.h"
#include "GnGeoXconfig.h"

static char ddaxskip[16][16] =
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
    { 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
    { 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
    { 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0},
    { 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0},
    { 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
    { 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0},
    { 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0},
    { 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1},
    { 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1},
    { 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
    { 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
    { 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};
/*
Uint32 ddaxskip_i[16] =
{
    0x0080, 0x0880, 0x0888, 0x2888, 0x288a, 0x2a8a, 0x2aaa, 0xaaaa,
    0xaaea, 0xbaea, 0xbaeb, 0xbbeb, 0xbbef, 0xfbef, 0xfbff, 0xffff
};
*/
Uint32 neogeo_frame_counter = 0;
Uint32 neogeo_frame_counter_speed = 8;
Uint32 frame_counter = 0;
char* dda_x_skip = NULL;
static char dda_y_skip[17];
static char full_y_skip[16] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
static Uint32 dda_y_skip_i = 0;
static Uint16 fix_addr[40][32];
static Uint8 fix_shift[40];

/* ******************************************************************************************************************/
/*!
* \brief  Applies alpha blending.
*
* \param  dest Destination pixel in RGB24.
* \param  src Source pixel in RGB24.
* \param  Alpha value.
* \return Alpha blended pixel.
*/
/* ******************************************************************************************************************/
static Uint32 alpha_blend ( Uint32 dest, Uint32 src, Uint8 alpha )
{
    Uint32 or = 0, og = 0, ob = 0, dr = 0, dg = 0, db = 0, sr = 0, sg = 0, sb = 0;
    Uint32 beta = ( 255 - alpha );
    Uint32 blended_color = 0;

    dr = COLOR_RGB24_R ( dest );
    dg = COLOR_RGB24_G ( dest );
    db = COLOR_RGB24_B ( dest );

    sr = COLOR_RGB24_R ( src );
    sg = COLOR_RGB24_G ( src );
    sb = COLOR_RGB24_B ( src );

    or = ( ( sr * alpha ) + ( dr * beta ) ) / 255;
    og = ( ( sg * alpha ) + ( dg * beta ) ) / 255;
    ob = ( ( sb * alpha ) + ( db * beta ) ) / 255;

    blended_color = COLOR_RGB24_MAKE ( alpha, or, og, ob );
    return ( blended_color );
}
/* ******************************************************************************************************************/
/*!
* \brief Initializes sprite cache.
*
* \param size Cache size.
* \param bsize Block size.
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
void SDL_textout ( SDL_Surface* dest, Sint32 x, Sint32 y, const char* string )
{
    SDL_Surface* sdl_surface_text = NULL;
    SDL_Rect dest_area;
    SDL_Color sys_font_color = {255, 150, 0, 1};

    if ( ( sdl_surface_text = TTF_RenderText_Solid ( sys_font, string, sys_font_color ) ) == NULL )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return;
    }

    dest_area.x = x;
    dest_area.y = y;
    dest_area.h = sdl_surface_text->h;
    dest_area.w = sdl_surface_text->w;

    if ( SDL_BlitSurface ( sdl_surface_text, NULL, dest, &dest_area ) < 0 )
    {
        zlog_error ( gngeox_config.loggingCat, "%s", SDL_GetError() );
        return;
    }

    SDL_FreeSurface ( sdl_surface_text );
}
/* ******************************************************************************************************************/
/*!
* \brief Initializes sprite cache.
*
* \param size Cache size.
* \param bsize Block size.
* \return SDL_FALSE when error, SDL_TRUE otherwise.
*/
/* ******************************************************************************************************************/
SDL_bool init_sprite_cache ( Uint32 size, Uint32 bsize )
{
    struct_gngeoxvideo_gfx_cache* gcache = &neogeo_memory.vid.spr_cache;

    if ( gcache->data != NULL ) /* We already have a cache, just reset it */
    {
        memset ( gcache->ptr, 0, gcache->total_bank * sizeof ( Uint8* ) );

        for ( Sint32 i = 0; i < gcache->max_slot; i++ )
        {
            gcache->usage[i] = -1;
        }

        return ( SDL_FALSE );
    }

    /* Create our video cache */
    gcache->slot_size = bsize;
    gcache->total_bank = neogeo_memory.rom.rom_region[REGION_SPRITES].size / gcache->slot_size;
    gcache->ptr = ( Uint8** ) qalloc ( gcache->total_bank * sizeof ( Uint8* ) );
    if ( gcache->ptr == NULL )
    {
        return ( SDL_FALSE );
    }

    gcache->size = size;
    gcache->data = ( Uint8* ) qalloc ( gcache->size );
    if ( gcache->data == NULL )
    {
        qalloc_delete ( gcache->ptr );

        return ( SDL_FALSE );
    }

    //gcache->max_slot=((float)gcache->size/0x4000000)*TOTAL_GFX_BANK;
    //gcache->max_slot=((float)gcache->size/neogeo_memory.rom.rom_region[REGION_SPRITES].size)*gcache->total_bank;
    gcache->max_slot = size / gcache->slot_size;
    //gcache->slot_size=0x4000000/TOTAL_GFX_BANK;
    gcache->usage = ( Sint32* ) qalloc ( gcache->max_slot * sizeof ( Uint32 ) );
    if ( gcache->usage == NULL )
    {
        qalloc_delete ( gcache->ptr );
        qalloc_delete ( gcache->data );
        return ( SDL_FALSE );
    }

    for ( Sint32 i = 0; i < gcache->max_slot; i++ )
    {
        gcache->usage[i] = -1;
    }

    gcache->in_buf = ( Uint8* ) qalloc ( compressBound ( bsize ) );
    if ( gcache->in_buf == NULL )
    {
        qalloc_delete ( gcache->ptr );
        qalloc_delete ( gcache->data );
        qalloc_delete ( gcache->usage );
        return ( SDL_FALSE );
    }

    return ( SDL_TRUE );
}
/* ******************************************************************************************************************/
/*!
* \brief Frees sprite cache.
*
*/
/* ******************************************************************************************************************/
void free_sprite_cache ( void )
{
    struct_gngeoxvideo_gfx_cache* gcache = &neogeo_memory.vid.spr_cache;

    if ( gcache->data )
    {
        qalloc_delete ( gcache->data );
        gcache->data = NULL;
    }

    if ( gcache->ptr )
    {
        qalloc_delete ( gcache->ptr );
        gcache->ptr = NULL;
    }

    if ( gcache->usage )
    {
        qalloc_delete ( gcache->usage );
        gcache->usage = NULL;
    }

    if ( gcache->in_buf )
    {
        qalloc_delete ( gcache->in_buf );
        gcache->in_buf = NULL;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes sprite cache.
*
* \param  tileno Tile number.
* \return Pointer to cached sprite.
*/
/* ******************************************************************************************************************/
static Uint8* get_cached_sprite_ptr ( Uint32 tileno )
{
    struct_gngeoxvideo_gfx_cache* gcache = &neogeo_memory.vid.spr_cache;
    static Sint32 pos = 0;
    Sint32 tile_sh = ~ ( ( gcache->slot_size >> 7 ) - 1 );
    Sint32 bank = ( ( tileno & tile_sh ) / ( gcache->slot_size >> 7 ) );
    Sint32 a = 0;
    Uint32 cmp_size = 0;
    Uint32 dst_size = 0;

    if ( gcache->ptr[bank] )
    {
        /* The bank is present in the cache */
        return gcache->ptr[bank];
    }

    /* We have to find a slot for this bank */
    a = pos;
    pos++;

    if ( pos >= gcache->max_slot )
    {
        pos = 0;
    }

    fseek ( gcache->gno, gcache->offset[bank], SEEK_SET );
    fread ( &cmp_size, sizeof ( Uint32 ), 1, gcache->gno );
    fread ( gcache->in_buf, cmp_size, 1, gcache->gno );
    dst_size = gcache->slot_size;
    uncompress ( gcache->data + a * gcache->slot_size, &dst_size, gcache->in_buf, cmp_size );

    gcache->ptr[bank] = gcache->data + a * gcache->slot_size;

    if ( gcache->usage[a] != -1 )
    {
        gcache->ptr[gcache->usage[a]] = 0;
    }

    gcache->usage[a] = bank;

    return ( gcache->ptr[bank] );
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes sprite cache.
*
*/
/* ******************************************************************************************************************/
static void fix_value_init ( void )
{
    for ( Sint32 x = 0; x < 40; x++ )
    {
        for ( Sint32 y = 0; y < 32; y++ )
        {
            fix_addr[x][y] = 0xea00 + ( y << 1 ) + 64 * ( x / 6 );
        }

        fix_shift[x] = ( 5 - ( x % 6 ) );
    }
}
/* @todo (Tmesys#1#12/17/2022): Drawing function generation : Should be refactored using maybe function pointers. */
/* ******************************************************************************************************************/
/*!
* \brief  .
*
*/
/* ******************************************************************************************************************/
#define RENAME(name) name##_tile
#define PUTPIXEL(dst,src) dst=src
#include "GnGeoXvideotemplate.h"
/* ******************************************************************************************************************/
/*!
* \brief  .
*
*/
/* ******************************************************************************************************************/
#define RENAME(name) name##_tile_50
#define PUTPIXEL(dst,src) dst=BLEND16_50(src,dst)
#include "GnGeoXvideotemplate.h"
/* ******************************************************************************************************************/
/*!
* \brief  .
*
*/
/* ******************************************************************************************************************/
#define RENAME(name) name##_tile_25
#define PUTPIXEL(dst,src) dst=BLEND16_25(src,dst)
#include "GnGeoXvideotemplate.h"
/* ******************************************************************************************************************/
/*!
* \brief  Draws fix char.
*
* \param  buf Tile number.
* \param  start Tile number.
* \param  end Tile number.
*/
/* ******************************************************************************************************************/
static void draw_fix_char ( Uint8* buf, Sint32 start, Sint32 end )
{
    Uint32* gfxdata = NULL, myword = 0;
    Sint32 y = 0;
    Uint32* brp = NULL;
    Uint32* paldata = NULL;
    Uint32 byte1 = 0, byte2 = 0;
    Sint32 banked = 0, garouoffsets[32];
    SDL_Rect clip;
    Sint32 ystart = 1, yend = 32;

    banked = ( current_fix == neogeo_memory.rom.rom_region[REGION_FIXED_LAYER_CARTRIDGE].p && neogeo_fix_bank_type && neogeo_memory.rom.rom_region[REGION_FIXED_LAYER_CARTRIDGE].size > 0x1000 ) ? 1 : 0;
    if ( banked && neogeo_fix_bank_type == 1 )
    {
        Sint32 garoubank = 0;
        Sint32 k = 0;
        y = 0;

        while ( y < 32 )
        {
            if ( READ_WORD ( &neogeo_memory.vid.ram[0xea00 + ( k << 1 )] ) == 0x0200 &&
                    ( READ_WORD ( &neogeo_memory.vid.ram[0xeb00 + ( k << 1 )] ) & 0xff00 ) == 0xff00 )
            {

                garoubank = READ_WORD ( &neogeo_memory.vid.ram[0xeb00 + ( k << 1 )] ) & 3;
                garouoffsets[y++] = garoubank;
            }

            garouoffsets[y++] = garoubank;
            k += 2;
        }
    }

    if ( start != 0 && end != 0 )
    {
        ystart = start >> 3;
        yend = ( end >> 3 ) + 1;

        if ( ystart < 1 )
        {
            ystart = 1;
        }

        clip.x = 0;
        clip.y = start + 16;
        clip.w = sdl_surface_buffer->w;
        clip.h = ( end - start ) + 16;
        SDL_SetClipRect ( sdl_surface_buffer, &clip );
    }

    for ( y = ystart; y < yend; y++ )
    {
        for ( Sint32 x = 1; x < 39; x++ )
        {
            byte1 = ( READ_WORD ( &neogeo_memory.vid.ram[0xE000 + ( ( y + ( x << 5 ) ) << 1 )] ) );
            byte2 = byte1 >> 12;
            byte1 = byte1 & 0xfff;

            if ( banked )
            {
                switch ( neogeo_fix_bank_type )
                {
                /* Garou, MSlug 3 */
                case ( 1 ) :
                    {
                        byte1 += 0x1000 * ( garouoffsets[ ( y - 2 ) & 31] ^ 3 );
                    }
                    break;
                case ( 2 ) :
                    {
                        byte1 += 0x1000 * fix_add ( x, y );
                    }
                    break;
                default :
                    {
                        zlog_error ( gngeox_config.loggingCat, "Unknown fix bank type %d", neogeo_fix_bank_type );
                    }
                    break;
                }
            }

            if ( ( byte1 >= ( neogeo_memory.rom.rom_region[REGION_FIXED_LAYER_CARTRIDGE].size >> 5 ) ) || ( fix_usage[byte1] == 0x00 ) )
            {
                continue;
            }

            brp = ( Uint32* ) buf + ( ( y << 3 ) ) * sdl_surface_buffer->w + ( x << 3 ) + 16;

            paldata = ( Uint32* ) &current_pc_pal[16 * byte2];
            gfxdata = ( Uint32* ) &current_fix[ byte1 << 5];

            for ( Sint32 yy = 0; yy < 8; yy++ )
            {
                Uint8 col = 0;

                myword = gfxdata[yy];
                col = ( myword >> 28 ) & 0xf;

                if ( col )
                {
                    brp[7] = paldata[col];
                }

                col = ( myword >> 24 ) & 0xf;

                if ( col )
                {
                    brp[6] = paldata[col];
                }

                col = ( myword >> 20 ) & 0xf;

                if ( col )
                {
                    brp[5] = paldata[col];
                }

                col = ( myword >> 16 ) & 0xf;

                if ( col )
                {
                    brp[4] = paldata[col];
                }

                col = ( myword >> 12 ) & 0xf;

                if ( col )
                {
                    brp[3] = paldata[col];
                }

                col = ( myword >> 8 ) & 0xf;

                if ( col )
                {
                    brp[2] = paldata[col];
                }

                col = ( myword >> 4 ) & 0xf;

                if ( col )
                {
                    brp[1] = paldata[col];
                }

                col = ( myword >> 0 ) & 0xf;

                if ( col )
                {
                    brp[0] = paldata[col];
                }

                brp += sdl_surface_buffer->w;
            }
        }
    }

    if ( start != 0 && end != 0 )
    {
        SDL_SetClipRect ( sdl_surface_buffer, NULL );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Draws screen.
*
*/
/* ******************************************************************************************************************/
void draw_screen ( void )
{
    Sint32 sx = 0, sy = 0, oy = 0, my = 0, zx = 1, rzy = 1;
    Uint32 offs, /*i, count,*/ y;
    Uint32 tileno, tileatr, t1, t2, t3;
    char fullmode = 0;
    Sint32 dday = 0, rzx = 15, yskip = 0;
    Uint8* vidram = neogeo_memory.vid.ram;
    Uint8 penusage;

    SDL_FillRect ( sdl_surface_buffer, NULL, current_pc_pal[4095] );
    SDL_LockSurface ( sdl_surface_buffer );

    /* Draw sprites */
    for ( Uint32 count = 0; count < 0x300; count += 2 )
    {
        t3 = READ_WORD ( &vidram[0x10000 + count] );
        t1 = READ_WORD ( &vidram[0x10400 + count] );
        t2 = READ_WORD ( &vidram[0x10800 + count] );

        /* If this bit is set, this new column is placed next to last one */
        if ( t1 & 0x40 )
        {
            /* new x */
            sx += rzx;

            //            if ( sx >= 0x1F0 )    /* x>496 => x-=512 */
            //                sx -= 0x200;

            /* Get new zoom for this column */
            zx = ( t3 >> 8 ) & 0x0f;
            /* y pos = old y pos */
            sy = oy;
        }
        /* nope it is a new block */
        else
        {
            /* Sprite scaling */
            zx = ( t3 >> 8 ) & 0x0f; /* zoom x */
            rzy = t3 & 0xff; /* zoom y */

            if ( rzy == 0 )
            {
                continue;
            }

            /* x pos */
            sx = ( t2 >> 7 );

            /* Number of tiles in this strip */
            my = t1 & 0x3f;

            if ( my == 0x20 )
            {
                fullmode = 1;
            }
            else
            {
                if ( my >= 0x21 )
                {
                    /* most games use 0x21, but */
                    fullmode = 2;
                }
                /* Alpha Mission II uses 0x3f */
                else
                {
                    fullmode = 0;
                }
            }

            /* sprite bank position */
            sy = 0x200 - ( t1 >> 7 );

            if ( sy > 0x110 )
            {
                sy -= 0x200;
            }

            if ( fullmode == 2 || ( fullmode == 1 && rzy == 0xff ) )
            {
                while ( sy < 0 )
                {
                    sy += ( ( rzy + 1 ) << 1 );
                }
            }

            /* on se souvient de la position y */
            oy = sy;

            if ( rzy < 0xff && my < 0x10 && my )
            {
                //my = (my<<8)/(rzy+1);
                my = my * 255 / rzy;

                if ( my > 0x10 )
                {
                    my = 0x10;
                }
            }

            if ( my > 0x20 )
            {
                my = 0x20;
            }
        }

        /* No point doing anything if tile strip is 0 */
        if ( my == 0 )
        {
            continue;
        }

        /* Process x zoom */
        if ( zx != 15 )
        {
            dda_x_skip = ddaxskip[zx];
            rzx = zx + 1;

        }
        else
        {
            rzx = 16;
        }

        if ( sx >= 0x1F0 )
        {
            sx -= 0x200;
        }

        if ( sx >= 320 )
        {
            continue;
        }

        //if (sx<-16) continue;

        /* Setup y zoom */
        if ( rzy == 255 )
        {
            yskip = 16;
        }
        else
        {
            dday = 0;    /* =256; NS990105 mslug fix */
        }

        offs = count << 6;

        /* my holds the number of tiles in each vertical multisprite block */
        for ( y = 0; y < my; y++ )
        {
            tileno = READ_WORD ( &vidram[offs] );
            offs += 2;
            tileatr = READ_WORD ( &vidram[offs] );
            offs += 2;

            if ( neogeo_memory.nb_of_tiles > 0x10000 && ( tileatr & 0x10 ) )
            {
                tileno += 0x10000;
            }

            if ( neogeo_memory.nb_of_tiles > 0x20000 && ( tileatr & 0x20 ) )
            {
                tileno += 0x20000;
            }

            if ( neogeo_memory.nb_of_tiles > 0x40000 && ( tileatr & 0x40 ) )
            {
                tileno += 0x40000;
            }

            /* animation automatique */
            if ( tileatr & 0x8 )
            {
                tileno = ( tileno & ~7 ) + ( ( tileno + neogeo_frame_counter ) & 7 );
            }
            else
            {
                if ( tileatr & 0x4 )
                {
                    tileno = ( tileno & ~3 ) + ( ( tileno + neogeo_frame_counter ) & 3 );
                }
            }

            if ( tileno > neogeo_memory.nb_of_tiles )
            {
                continue;
            }

            if ( fullmode == 2 || ( fullmode == 1 && rzy == 0xff ) )
            {
                if ( sy >= 248 )
                {
                    sy -= ( ( rzy + 1 ) << 1 );
                }
            }
            else
            {
                if ( fullmode == 1 )
                {
                    if ( y == 0x10 )
                    {
                        sy -= ( ( rzy + 1 ) << 1 );
                    }
                }
                else
                {
                    if ( sy > 0x110 )
                    {
                        sy -= 0x200;    // NS990105 mslug2 fix
                    }
                }
            }

            if ( rzy != 255 )
            {
                yskip = 0;
                dda_y_skip_i = 0;
                dda_y_skip[0] = 0;

                for ( Uint32 i = 0; i < 16; i++ )
                {
                    dda_y_skip[i + 1] = 0;
                    dday -= ( rzy + 1 );

                    if ( dday <= 0 )
                    {
                        dday += 256;
                        yskip++;
                        dda_y_skip[yskip]++;
                    }
                    else
                    {
                        dda_y_skip[yskip]++;
                    }

                    //if (dda_y_skip[i])
                    //       dda_y_skip_i=dda_y_skip_i|(1<<i);
                }
            }

            if ( sx >= -16 && sx + 15 < 336 && sy >= 0 && sy + 15 < 256 )
            {
                penusage = neo_transpack_find ( tileno );
                if ( penusage == TILE_TRANS_UNKNOWN )
                {
                    penusage = PEN_USAGE ( tileno );
                }

                if ( neogeo_memory.vid.spr_cache.data )
                {
                    neogeo_memory.rom.rom_region[REGION_SPRITES].p = get_cached_sprite_ptr ( tileno );
                    tileno = ( tileno & ( ( neogeo_memory.vid.spr_cache.slot_size >> 7 ) - 1 ) );
                }

                switch ( penusage )
                {
                case ( TILE_NORMAL ) :
                    {
                        draw_tile ( tileno, sx + 16, sy, rzx, yskip, tileatr >> 8,
                                    tileatr & 0x01, tileatr & 0x02,
                                    ( Uint8* ) sdl_surface_buffer->pixels );
                    }
                    break;
                case ( TILE_TRANSPARENT25 ) :
                    {
                        draw_tile_25 ( tileno, sx + 16, sy, rzx, yskip, tileatr >> 8,
                                       tileatr & 0x01, tileatr & 0x02,
                                       ( Uint8* ) sdl_surface_buffer->pixels );
                    }
                    break;
                case ( TILE_TRANSPARENT50 ) :
                    {
                        draw_tile_50 ( tileno, sx + 16, sy, rzx, yskip, tileatr >> 8,
                                       tileatr & 0x01, tileatr & 0x02,
                                       ( Uint8* ) sdl_surface_buffer->pixels );
                    }
                    break;
                    /*
                      default:
                          {
                              SDL_Rect r={sx+16,sy,rzx,yskip};
                              SDL_FillRect(sdl_surface_buffer,&r,0xFFAA);
                          }
                          //((Uint16*)(sdl_surface_buffer->pixels))[sx+16+sy*356]=0xFFFF;

                          break;
                     */
                }
            }

            sy += yskip;
        } /* for y */
    } /* for count */

    draw_fix_char ( sdl_surface_buffer->pixels, 0, 0 );
    SDL_UnlockSurface ( sdl_surface_buffer );

    neo_frame_skip_display();

    neo_screen_update();
}
/* ******************************************************************************************************************/
/*!
* \brief  Draws screen with scanlines.
*
* \param  start_line Tile number.
* \param  end_line Tile number.
* \param  refresh Tile number.
*/
/* ******************************************************************************************************************/
void draw_screen_scanline ( Sint32 start_line, Sint32 end_line, Sint32 refresh )
{
    Sint32 sx = 0, sy = 0, my = 0, zx = 1, zy = 1;
    Sint32 offs = 0, /*count,*/ y = 0;
    Sint32 tileno = 0, tileatr = 0;
    Sint32 tctl1 = 0, tctl2 = 0, tctl3 = 0;
    Uint8* vidram = neogeo_memory.vid.ram;
    static SDL_Rect clear_rect;
    Sint32 yy = 0;
    Sint32 tile = 0, yoffs = 0;
    Sint32 zoom_line = 0;
    Sint32 invert = 0;
    Uint8* zoomy_rom = NULL;
    Uint8 penusage = 0;

    if ( start_line > 255 )
    {
        start_line = 255;
    }

    if ( end_line > 255 )
    {
        end_line = 255;
    }

    clear_rect.x = visible_area.x;
    clear_rect.w = visible_area.w;
    clear_rect.y = start_line;
    clear_rect.h = end_line - start_line + 1;

    SDL_FillRect ( sdl_surface_buffer, &clear_rect, current_pc_pal[4095] );

    /* Draw sprites */
    for ( Sint32 count = 0; count < 0x300; count += 2 )
    {
        tctl3 = READ_WORD ( &vidram[0x10000 + count] );
        tctl1 = READ_WORD ( &vidram[0x10400 + count] );
        tctl2 = READ_WORD ( &vidram[0x10800 + count] );

        /* If this bit is set this new column is placed next to last one */
        if ( tctl1 & 0x40 )
        {
            /* new x */
            sx += zx + 1;
            /* Get new zoom for this column */
            zx = ( tctl3 >> 8 ) & 0x0f;
        }
        /* nope it is a new block */
        else
        {
            /* Sprite scaling */
            zx = ( tctl3 >> 8 ) & 0x0f; /* zomm x */
            zy = tctl3 & 0xff; /* zoom y */

            /* x pos 0 - 512  */
            sx = ( tctl2 >> 7 );

            /* Number of tiles in this strip */
            my = tctl1 & 0x3f;

            /* y pos 512 - 0 */
            sy = 512 - ( tctl1 >> 7 );

            if ( my > 0x20 )
            {
                my = 0x20;
            }
        }

        /* No point doing anything if tile strip is 0 */
        if ( my == 0 )
        {
            continue;
        }

        if ( sx >= 496 ) /* after 496, we consider sx negative */
        {
            sx -= 512;
            //continue;
        }

        if ( sx > 320 )
        {
            continue;
            //sx-=16;
        }

        if ( sx < -16 )
        {
            continue;
        }

        //sx&=0x1ff;

        /* Process x zoom */
        if ( zx != 16 )
        {
            dda_x_skip = ddaxskip[zx];
        }
        else
        {
            zx = 16;
        }

        offs = count << 6;
        zoomy_rom = neogeo_memory.ng_lo + ( zy << 8 );

        for ( yy = start_line; yy <= end_line; yy++ )
        {
            y = yy - sy; /* y: 0 -> my*16 */

            if ( y < 0 )
            {
                y += 512;
            }

            if ( y >= ( my << 4 ) )
            {
                continue;
            }

            invert = 0;

            zoom_line = y & 0xff;

            if ( y & 0x100 )
            {
                zoom_line ^= 0xff; /* zoom_line = 255 - zoom_line */
                invert = 1;
            }

            if ( my == 0x20 ) /* fix for joyjoy, trally... */
            {
                if ( zy )
                {
                    zoom_line %= ( zy << 1 );

                    if ( zoom_line >= zy )
                    {
                        zoom_line = ( zy << 1 ) - 1 - zoom_line;
                        invert ^= 1;
                    }
                }
            }

            yoffs = zoomy_rom[zoom_line] & 0x0f;
            tile = zoomy_rom[zoom_line] >> 4;

            if ( invert )
            {
                tile ^= 0x1f; // tile=31 - tile;
                yoffs ^= 0x0f; // yoffs= 15 - yoffs;
            }

            tileno = READ_WORD ( &vidram[offs + ( tile << 2 )] );
            tileatr = READ_WORD ( &vidram[offs + ( tile << 2 ) + 2] );

            if ( neogeo_memory.nb_of_tiles > 0x10000 && ( tileatr & 0x10 ) )
            {
                tileno += 0x10000;
            }

            if ( neogeo_memory.nb_of_tiles > 0x20000 && ( tileatr & 0x20 ) )
            {
                tileno += 0x20000;
            }

            if ( neogeo_memory.nb_of_tiles > 0x40000 && ( tileatr & 0x40 ) )
            {
                tileno += 0x40000;
            }

            /* animation automatique */
            if ( tileatr & 0x8 )
            {
                tileno = ( tileno & ~7 ) + ( ( tileno + neogeo_frame_counter ) & 7 );
            }
            else
            {
                if ( tileatr & 0x4 )
                {
                    tileno = ( tileno & ~3 ) + ( ( tileno + neogeo_frame_counter ) & 3 );
                }
            }

            if ( tileatr & 0x02 )
            {
                yoffs ^= 0x0f;    /* flip y */
            }

            penusage = neo_transpack_find ( tileno );
            if ( penusage == TILE_TRANS_UNKNOWN )
            {
                penusage = PEN_USAGE ( tileno );
            }

            if ( neogeo_memory.vid.spr_cache.data )
            {
                neogeo_memory.rom.rom_region[REGION_SPRITES].p = get_cached_sprite_ptr ( tileno );
                tileno = ( tileno & ( ( neogeo_memory.vid.spr_cache.slot_size >> 7 ) - 1 ) );
            }

            switch ( penusage )
            {
            case ( TILE_NORMAL ) :
                {
                    draw_scanline_tile ( tileno, yoffs, sx + 16, yy, zx, tileatr >> 8,
                                         tileatr & 0x01, ( Uint8* ) sdl_surface_buffer->pixels );
                }
                break;

            case ( TILE_TRANSPARENT50 ) :
                {
                    draw_scanline_tile_50 ( tileno, yoffs, sx + 16, yy, zx, tileatr >> 8,
                                            tileatr & 0x01, ( Uint8* ) sdl_surface_buffer->pixels );
                }
                break;

            case ( TILE_TRANSPARENT25 ) :
                {
                    draw_scanline_tile_25 ( tileno, yoffs, sx + 16, yy, zx, tileatr >> 8,
                                            tileatr & 0x01, ( Uint8* ) sdl_surface_buffer->pixels );
                }
                break;
            }
        }
    } /* for count */

    if ( refresh )
    {
        draw_fix_char ( sdl_surface_buffer->pixels, 0, 0 );

        neo_frame_skip_display();

        neo_screen_update();
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes video.
*
*/
/* ******************************************************************************************************************/
void init_video ( void )
{
    fix_value_init();
    neogeo_memory.vid.modulo = 1;
}

#ifdef _GNGEOX_VIDEO_C_
#undef _GNGEOX_VIDEO_C_
#endif // _GNGEOX_VIDEO_C_
