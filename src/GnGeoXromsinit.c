/*!
*
*   \file    GnGeoXromsinit.c
*   \brief   ROMS special init functions.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    14/03/2024
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_ROMSINIT_C_
#define _GNGEOX_ROMSINIT_C_
#endif // _GNGEOX_ROMSINIT_C_

#include <SDL2/SDL.h>
#include "zlog.h"
#include "qlibc.h"
#include "bstrlib.h"

#include "GnGeoXroms.h"
#include "GnGeoXromsinit.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXneocrypt.h"
#include "GnGeoXneoboot.h"
#include "GnGeoXconfig.h"

static Sint32 bankoffset_kof99[64] =
{
    0x000000, 0x100000, 0x200000, 0x300000, 0x3cc000,
    0x4cc000, 0x3f2000, 0x4f2000, 0x407800, 0x507800, 0x40d000, 0x50d000,
    0x417800, 0x517800, 0x420800, 0x520800, 0x424800, 0x524800, 0x429000,
    0x529000, 0x42e800, 0x52e800, 0x431800, 0x531800, 0x54d000, 0x551000,
    0x567000, 0x592800, 0x588800, 0x581800, 0x599800, 0x594800, 0x598000,
    /* rest not used? */
};

/* addr,uncramblecode,.... */
static Uint8 scramblecode_kof99[7] = {0xF0, 14, 6, 8, 10, 12, 5,};

static Sint32 bankoffset_garou[64] =
{
    0x000000, 0x100000, 0x200000, 0x300000, // 00
    0x280000, 0x380000, 0x2d0000, 0x3d0000, // 04
    0x2f0000, 0x3f0000, 0x400000, 0x500000, // 08
    0x420000, 0x520000, 0x440000, 0x540000, // 12
    0x498000, 0x598000, 0x4a0000, 0x5a0000, // 16
    0x4a8000, 0x5a8000, 0x4b0000, 0x5b0000, // 20
    0x4b8000, 0x5b8000, 0x4c0000, 0x5c0000, // 24
    0x4c8000, 0x5c8000, 0x4d0000, 0x5d0000, // 28
    0x458000, 0x558000, 0x460000, 0x560000, // 32
    0x468000, 0x568000, 0x470000, 0x570000, // 36
    0x478000, 0x578000, 0x480000, 0x580000, // 40
    0x488000, 0x588000, 0x490000, 0x590000, // 44
    0x5d0000, 0x5d8000, 0x5e0000, 0x5e8000, // 48
    0x5f0000, 0x5f8000, 0x600000, /* rest not used? */
};

static Uint8 scramblecode_garou[7] = {0xC0, 5, 9, 7, 6, 14, 12,};

static Sint32 bankoffset_garouo[64] =
{
    0x000000, 0x100000, 0x200000, 0x300000, // 00
    0x280000, 0x380000, 0x2d0000, 0x3d0000, // 04
    0x2c8000, 0x3c8000, 0x400000, 0x500000, // 08
    0x420000, 0x520000, 0x440000, 0x540000, // 12
    0x598000, 0x698000, 0x5a0000, 0x6a0000, // 16
    0x5a8000, 0x6a8000, 0x5b0000, 0x6b0000, // 20
    0x5b8000, 0x6b8000, 0x5c0000, 0x6c0000, // 24
    0x5c8000, 0x6c8000, 0x5d0000, 0x6d0000, // 28
    0x458000, 0x558000, 0x460000, 0x560000, // 32
    0x468000, 0x568000, 0x470000, 0x570000, // 36
    0x478000, 0x578000, 0x480000, 0x580000, // 40
    0x488000, 0x588000, 0x490000, 0x590000, // 44
    0x5d8000, 0x6d8000, 0x5e0000, 0x6e0000, // 48
    0x5e8000, 0x6e8000, 0x6e8000, 0x000000, // 52
    0x000000, 0x000000, 0x000000, 0x000000, // 56
    0x000000, 0x000000, 0x000000, 0x000000, // 60
};

static Uint8 scramblecode_garouo[7] = {0xC0, 4, 8, 14, 2, 11, 13,};

static Sint32 bankoffset_mslug3[64] =
{
    0x000000, 0x020000, 0x040000, 0x060000, // 00
    0x070000, 0x090000, 0x0b0000, 0x0d0000, // 04
    0x0e0000, 0x0f0000, 0x120000, 0x130000, // 08
    0x140000, 0x150000, 0x180000, 0x190000, // 12
    0x1a0000, 0x1b0000, 0x1e0000, 0x1f0000, // 16
    0x200000, 0x210000, 0x240000, 0x250000, // 20
    0x260000, 0x270000, 0x2a0000, 0x2b0000, // 24
    0x2c0000, 0x2d0000, 0x300000, 0x310000, // 28
    0x320000, 0x330000, 0x360000, 0x370000, // 32
    0x380000, 0x390000, 0x3c0000, 0x3d0000, // 36
    0x400000, 0x410000, 0x440000, 0x450000, // 40
    0x460000, 0x470000, 0x4a0000, 0x4b0000, // 44
    0x4c0000, /* rest not used? */
};

static Uint8 scramblecode_mslug3[7] = {0xE4, 14, 12, 15, 6, 3, 9,};

static Sint32 bankoffset_kof2000[64] =
{
    0x000000, 0x100000, 0x200000, 0x300000, // 00
    0x3f7800, 0x4f7800, 0x3ff800, 0x4ff800, // 04
    0x407800, 0x507800, 0x40f800, 0x50f800, // 08
    0x416800, 0x516800, 0x41d800, 0x51d800, // 12
    0x424000, 0x524000, 0x523800, 0x623800, // 16
    0x526000, 0x626000, 0x528000, 0x628000, // 20
    0x52a000, 0x62a000, 0x52b800, 0x62b800, // 24
    0x52d000, 0x62d000, 0x52e800, 0x62e800, // 28
    0x618000, 0x619000, 0x61a000, 0x61a800, // 32
};

static Uint8 scramblecode_kof2000[7] = {0xEC, 15, 14, 7, 3, 10, 5,};

struct_gngeoxroms_init_func init_func_table[] =
{
    { "kof99", init_kof99},
    { "kof99n", init_kof99n},
    { "garou", init_garou},
    { "garouo", init_garouo},
    { "garoubl", init_garoubl},
    { "mslug3", init_mslug3},
    { "mslug3h", init_mslug3h},
    { "mslug3n", init_mslug3h},
    { "mslug3b6", init_mslug3b6},
    { "kof2000", init_kof2000},
    { "kof2000n", init_kof2000n},
    { "kof2001", init_kof2001},
    { "mslug4", init_mslug4},
    { "ms4plus", init_ms4plus},
    { "ganryu", init_ganryu},
    { "s1945p", init_s1945p},
    { "preisle2", init_preisle2},
    { "bangbead", init_bangbead},
    { "nitd", init_nitd},
    { "zupapa", init_zupapa},
    { "sengoku3", init_sengoku3},
    { "kof98", init_kof98},
    { "rotd", init_rotd},
    { "kof2002", init_kof2002},
    { "kof2002b", init_kof2002b},
    { "kf2k2pls", init_kf2k2pls},
    { "kf2k2mp", init_kf2k2mp},
    { "kof2km2", init_kof2km2},
    { "matrim", init_matrim},
    { "pnyaa", init_pnyaa},
    { "mslug5", init_mslug5},
    { "ms5pcb", init_ms5pcb},
    { "ms5plus", init_ms5plus},
    { NULL, NULL}
};
/* ******************************************************************************************************************/
/*!
* \brief  Initializes kof99.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_kof99 ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        kof99_decrypt_68k ( rom );
        kof99_neogeo_gfx_decrypt ( rom, 0x00 );
    }

    neogeo_fix_bank_type = 0;
    neogeo_memory.bksw_offset = bankoffset_kof99;
    neogeo_memory.bksw_unscramble = scramblecode_kof99;
    neogeo_memory.sma_rng_addr = 0xF8FA;
    //kof99_install_protection(machine);
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes kof99n.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_kof99n ( struct_gngeoxroms_game_roms* rom )
{
    neogeo_fix_bank_type = 1;

    if ( rom->need_decrypt )
    {
        kof99_neogeo_gfx_decrypt ( rom, 0x00 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes garou.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_garou ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        garou_decrypt_68k ( rom );
        kof99_neogeo_gfx_decrypt ( rom, 0x06 );
    }

    neogeo_fix_bank_type = 1;
    neogeo_memory.bksw_offset = bankoffset_garou;
    neogeo_memory.bksw_unscramble = scramblecode_garou;
    neogeo_memory.sma_rng_addr = 0xCCF0;
    //garou_install_protection(machine);
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes garouo.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_garouo ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        garouo_decrypt_68k ( rom );
        kof99_neogeo_gfx_decrypt ( rom, 0x06 );
    }

    neogeo_fix_bank_type = 1;
    neogeo_memory.bksw_offset = bankoffset_garouo;
    neogeo_memory.bksw_unscramble = scramblecode_garouo;
    neogeo_memory.sma_rng_addr = 0xCCF0;

    //garouo_install_protection(machine);
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes garoubl.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_garoubl ( struct_gngeoxroms_game_roms* rom )
{
    /* @todo (Tmesys#1#20/10/2023): Bootleg support */
    if ( rom->need_decrypt )
    {
        neogeo_bootleg_sx_decrypt ( rom, 2 );
        neogeo_bootleg_cx_decrypt ( rom );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes mslug3.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_mslug3 ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        mslug3_decrypt_68k ( rom );
        kof99_neogeo_gfx_decrypt ( rom, 0xad );
    }

    neogeo_fix_bank_type = 1;
    neogeo_memory.bksw_offset = bankoffset_mslug3;
    neogeo_memory.bksw_unscramble = scramblecode_mslug3;
    //neogeo_memory.sma_rng_addr=0xF8FA;
    neogeo_memory.sma_rng_addr = 0;

    //mslug3_install_protection(r);
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes mslug3h.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_mslug3h ( struct_gngeoxroms_game_roms* rom )
{
    neogeo_fix_bank_type = 1;

    if ( rom->need_decrypt )
    {
        kof99_neogeo_gfx_decrypt ( rom, 0xad );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes mslug3b6.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_mslug3b6 ( struct_gngeoxroms_game_roms* rom )
{
    /* TODO: Bootleg support */
    if ( rom->need_decrypt )
    {
        neogeo_bootleg_sx_decrypt ( rom, 2 );
        cmc42_neogeo_gfx_decrypt ( rom, 0xad );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes mslug4.
*
* \param  rom Game rom.
* \note USA violent content screen is wrong : Not a bug, confirmed on real hardware!
*/
/* ******************************************************************************************************************/
static void init_mslug4 ( struct_gngeoxroms_game_roms* rom )
{
    neogeo_fix_bank_type = 1;

    if ( rom->need_decrypt )
    {
        neogeo_cmc50_m1_decrypt ( rom );
        kof2000_neogeo_gfx_decrypt ( rom, 0x31 );

        neo_pcm2_snk_1999 ( rom, 8 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes ms4plus.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_ms4plus ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        cmc50_neogeo_gfx_decrypt ( rom, 0x31 );
        neo_pcm2_snk_1999 ( rom, 8 );
        neogeo_cmc50_m1_decrypt ( rom );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes kof2000.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_kof2000 ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        kof2000_decrypt_68k ( rom );
        neogeo_cmc50_m1_decrypt ( rom );
        kof2000_neogeo_gfx_decrypt ( rom, 0x00 );
    }

    neogeo_fix_bank_type = 2;
    neogeo_memory.bksw_offset = bankoffset_kof2000;
    neogeo_memory.bksw_unscramble = scramblecode_kof2000;
    neogeo_memory.sma_rng_addr = 0xD8DA;
    //kof2000_install_protection(r);
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes kof2000.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_kof2000n ( struct_gngeoxroms_game_roms* rom )
{
    neogeo_fix_bank_type = 2;

    if ( rom->need_decrypt )
    {
        neogeo_cmc50_m1_decrypt ( rom );
        kof2000_neogeo_gfx_decrypt ( rom, 0x00 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes kof2001.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_kof2001 ( struct_gngeoxroms_game_roms* rom )
{
    neogeo_fix_bank_type = 1;

    if ( rom->need_decrypt )
    {
        kof2000_neogeo_gfx_decrypt ( rom, 0x1e );
        neogeo_cmc50_m1_decrypt ( rom );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes ganryu.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_ganryu ( struct_gngeoxroms_game_roms* rom )
{
    neogeo_fix_bank_type = 1;

    if ( rom->need_decrypt )
    {
        kof99_neogeo_gfx_decrypt ( rom, 0x07 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes s1945p.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_s1945p ( struct_gngeoxroms_game_roms* rom )
{
    neogeo_fix_bank_type = 1;

    if ( rom->need_decrypt )
    {
        kof99_neogeo_gfx_decrypt ( rom, 0x05 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes preisle2.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_preisle2 ( struct_gngeoxroms_game_roms* rom )
{
    neogeo_fix_bank_type = 1;

    if ( rom->need_decrypt )
    {
        kof99_neogeo_gfx_decrypt ( rom, 0x9f );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes bangbead.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_bangbead ( struct_gngeoxroms_game_roms* rom )
{
    neogeo_fix_bank_type = 1;

    if ( rom->need_decrypt )
    {
        kof99_neogeo_gfx_decrypt ( rom, 0xf8 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes nitd.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_nitd ( struct_gngeoxroms_game_roms* rom )
{
    neogeo_fix_bank_type = 1;

    if ( rom->need_decrypt )
    {
        kof99_neogeo_gfx_decrypt ( rom, 0xff );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes zupapa.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_zupapa ( struct_gngeoxroms_game_roms* rom )
{
    neogeo_fix_bank_type = 1;

    if ( rom->need_decrypt )
    {
        kof99_neogeo_gfx_decrypt ( rom, 0xbd );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes sengoku3.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_sengoku3 ( struct_gngeoxroms_game_roms* rom )
{
    neogeo_fix_bank_type = 1;

    if ( rom->need_decrypt )
    {
        kof99_neogeo_gfx_decrypt ( rom, 0xfe );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes sengoku3.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_kof98 ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        kof98_decrypt_68k ( rom );
    }

    //install_kof98_protection(r);
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes rotd.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_rotd ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        neo_pcm2_snk_1999 ( rom, 16 );
        neogeo_cmc50_m1_decrypt ( rom );
        kof2000_neogeo_gfx_decrypt ( rom, 0x3f );
    }

    neogeo_fix_bank_type = 1;
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes kof2002.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_kof2002 ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        kof2002_decrypt_68k ( rom );
        neo_pcm2_swap ( rom, 0 );
        neogeo_cmc50_m1_decrypt ( rom );
        kof2000_neogeo_gfx_decrypt ( rom, 0xec );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes kof2002b.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_kof2002b ( struct_gngeoxroms_game_roms* rom )
{
    /* TODO: Bootleg */
    if ( rom->need_decrypt )
    {
        kof2002_decrypt_68k ( rom );
        neo_pcm2_swap ( rom, 0 );
        neogeo_cmc50_m1_decrypt ( rom );
        //kof2002b_gfx_decrypt(r, r->rom_region[REGION_SPRITES].p,0x4000000);
        //kof2002b_gfx_decrypt(r, r->rom_region[REGION_FIXED_LAYER_CARTRIDGE].p,0x20000);
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes kf2k2pls.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_kf2k2pls ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        kof2002_decrypt_68k ( rom );
        neo_pcm2_swap ( rom, 0 );
        neogeo_cmc50_m1_decrypt ( rom );
        cmc50_neogeo_gfx_decrypt ( rom, 0xec );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes kf2k2mp.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_kf2k2mp ( struct_gngeoxroms_game_roms* rom )
{
    /* TODO: Bootleg */
    if ( rom->need_decrypt )
    {
        //kf2k2mp_decrypt(r);
        neo_pcm2_swap ( rom, 0 );
        //neogeo_bootleg_sx_decrypt(r, 2);
        cmc50_neogeo_gfx_decrypt ( rom, 0xec );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes kof2km2.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_kof2km2 ( struct_gngeoxroms_game_roms* rom )
{
    /* TODO: Bootleg */
    if ( rom->need_decrypt )
    {
        //kof2km2_px_decrypt(r);
        neo_pcm2_swap ( rom, 0 );
        //neogeo_bootleg_sx_decrypt(r, 1);
        cmc50_neogeo_gfx_decrypt ( rom, 0xec );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes kof2km2.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_matrim ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        matrim_decrypt_68k ( rom );
        neo_pcm2_swap ( rom, 1 );
        neogeo_cmc50_m1_decrypt ( rom );
        kof2000_neogeo_gfx_decrypt ( rom, 0x6a );
    }

    neogeo_fix_bank_type = 2;
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes pnyaa.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_pnyaa ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        neo_pcm2_snk_1999 ( rom, 4 );
        neogeo_cmc50_m1_decrypt ( rom );
        kof2000_neogeo_gfx_decrypt ( rom, 0x2e );
    }

    neogeo_fix_bank_type = 1;
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes mslug5.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_mslug5 ( struct_gngeoxroms_game_roms* rom )
{
    if ( rom->need_decrypt )
    {
        mslug5_decrypt_68k ( rom );
        neo_pcm2_swap ( rom, 2 );
        neogeo_cmc50_m1_decrypt ( rom );
        kof2000_neogeo_gfx_decrypt ( rom, 0x19 );
    }

    neogeo_fix_bank_type = 1;
    //install_pvc_protection(r);
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes ms5pcb.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_ms5pcb ( struct_gngeoxroms_game_roms* rom )
{
    /* @todo (Tmesys#1#12/13/2022): Start a timer that will check the BIOS select DIP every second. */
    //timer_set(machine, attotime_zero, NULL, 0, ms5pcb_bios_timer_callback);
    //timer_pulse(machine, ATTOTIME_IN_MSEC(1000), NULL, 0, ms5pcb_bios_timer_callback);
    if ( rom->need_decrypt )
    {
        mslug5_decrypt_68k ( rom );
        svcpcb_gfx_decrypt ( rom );
        neogeo_cmc50_m1_decrypt ( rom );
        kof2000_neogeo_gfx_decrypt ( rom, 0x19 );
        svcpcb_s1data_decrypt ( rom );
        neo_pcm2_swap ( rom, 2 );
    }

    neogeo_fix_bank_type = 2;
    //install_pvc_protection(r);
}
/* ******************************************************************************************************************/
/*!
* \brief  Initializes ms5plus.
*
* \param  rom Game rom.
*/
/* ******************************************************************************************************************/
static void init_ms5plus ( struct_gngeoxroms_game_roms* rom )
{
    /* TODO: Bootleg */
    if ( rom->need_decrypt )
    {
        cmc50_neogeo_gfx_decrypt ( rom, 0x19 );
        neo_pcm2_swap ( rom, 2 );
        //neogeo_bootleg_sx_decrypt(r, 1);
    }

    neogeo_fix_bank_type = 1;

    //install_ms5plus_protection(r);
}
/* ******************************************************************************************************************/
/*!
* \brief  Converts all rom chars.
*
* \param  rom Todo.
*/
/* ******************************************************************************************************************/
void init_roms ( struct_gngeoxroms_game_roms* rom )
{
    Sint32 i = 0;
    neogeo_fix_bank_type = 0;
    neogeo_memory.bksw_handler = 0;
    neogeo_memory.bksw_unscramble = NULL;
    neogeo_memory.bksw_offset = NULL;
    neogeo_memory.sma_rng_addr = 0;

    while ( init_func_table[i].name )
    {
        if ( strcmp ( init_func_table[i].name, rom->info.name ) == 0
                && init_func_table[i].init != NULL )
        {
            zlog_warn ( gngeox_config.loggingCat, "Special init func" );
            init_func_table[i].init ( rom );
        }

        i++;
    }
}

#ifdef _GNGEOX_ROMSINIT_C_
#undef _GNGEOX_ROMSINIT_C_
#endif // _GNGEOX_ROMSINIT_C_
