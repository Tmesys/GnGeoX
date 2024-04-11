/*!
*
*   \file    GnGeoXgen68kinterf.c
*   \brief   Interface to the generator68k emulator.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    03/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_GEN68K_INTERF_C_
#define _GNGEOX_GEN68K_INTERF_C_
#endif // _GNGEOX_GEN68K_INTERF_C_

#include <stdlib.h>

#include <SDL2/SDL.h>
#include "zlog.h"
#include "qlibc.h"

#include "3rdParty/Generator68k/generator.h"
#include "3rdParty/Generator68k/cpu68k.h"
#include "3rdParty/Generator68k/reg68k.h"
#include "3rdParty/Generator68k/mem68k.h"
#include "GnGeoXgen68kinterf.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXemu.h"
#include "GnGeoXdebug.h"
#include "GnGeoXconfig.h"
#include "GnGeoXpd4990a.h"
#include "GnGeoXz80interf.h"
#include "GnGeoXscanline.h"


t_mem68k_def mem68k_def[] =
{
    {
        0x000, 0x1000, mem68k_memptr_bad,
        mem68k_fetch_invalid_byte, mem68k_fetch_invalid_word,
        mem68k_fetch_invalid_long,
        mem68k_store_invalid_byte, mem68k_store_invalid_word,
        mem68k_store_invalid_long
    },

    /* RAM */
    {
        0x100, 0x1FF, mem68k_memptr_ram,
        mem68k_fetch_ram_byte, mem68k_fetch_ram_word, mem68k_fetch_ram_long,
        mem68k_store_ram_byte, mem68k_store_ram_word, mem68k_store_ram_long
    },

    /* BANKED CPU */
    {
        0x200, 0x2FF, mem68k_memptr_cpu_bk,
        NULL, NULL, NULL, NULL, NULL, NULL
    },

    /* CPU BANK 0 */
    {
        0x000, 0x0FF, mem68k_memptr_cpu,
        mem68k_fetch_cpu_byte, mem68k_fetch_cpu_word, mem68k_fetch_cpu_long,
        mem68k_store_invalid_byte, mem68k_store_invalid_word,
        mem68k_store_invalid_long
    },

    /* BIOS */
    {
        0xc00, 0xcFF, mem68k_memptr_bios,
        mem68k_fetch_bios_byte, mem68k_fetch_bios_word,
        mem68k_fetch_bios_long,
        mem68k_store_invalid_byte, mem68k_store_invalid_word,
        mem68k_store_invalid_long
    },

    /* SRAM */
    {
        0xd00, 0xdFF, mem68k_memptr_bad,
        mem68k_fetch_sram_byte, mem68k_fetch_sram_word,
        mem68k_fetch_sram_long,
        mem68k_store_sram_byte, mem68k_store_sram_word,
        mem68k_store_sram_long
    },

    /* PAL */
    {
        0x400, 0x401, mem68k_memptr_bad,
        mem68k_fetch_pal_byte, mem68k_fetch_pal_word, mem68k_fetch_pal_long,
        mem68k_store_pal_byte, mem68k_store_pal_word, mem68k_store_pal_long
    },

    /* VIDEO */
    {
        0x3c0, 0x3c0, mem68k_memptr_bad,
        mem68k_fetch_video_byte, mem68k_fetch_video_word,
        mem68k_fetch_video_long,
        mem68k_store_video_byte, mem68k_store_video_word,
        mem68k_store_video_long
    },

    /* CONTROLER 1 */
    {
        0x300, 0x300, mem68k_memptr_bad,
        mem68k_fetch_ctl1_byte, mem68k_fetch_ctl1_word,
        mem68k_fetch_ctl1_long,
        mem68k_store_invalid_byte, mem68k_store_invalid_word,
        mem68k_store_invalid_long
    },

    /* CONTROLER 2 */
    {
        0x340, 0x340, mem68k_memptr_bad,
        mem68k_fetch_ctl2_byte, mem68k_fetch_ctl2_word,
        mem68k_fetch_ctl2_long,
        mem68k_store_invalid_byte, mem68k_store_invalid_word,
        mem68k_store_invalid_long
    },

    /* CONTROLER 3 + PD4990 */
    {
        0x380, 0x380, mem68k_memptr_bad,
        mem68k_fetch_ctl3_byte, mem68k_fetch_ctl3_word,
        mem68k_fetch_ctl3_long,
        mem68k_store_pd4990_byte, mem68k_store_pd4990_word,
        mem68k_store_pd4990_long
    },

    /* COIN + Z80 */
    {
        0x320, 0x320, mem68k_memptr_bad,
        mem68k_fetch_coin_byte, mem68k_fetch_coin_word,
        mem68k_fetch_coin_long,
        mem68k_store_z80_byte, mem68k_store_z80_word, mem68k_store_z80_long
    },

    /* MEMCARD */
    {
        0x800, 0x800, mem68k_memptr_bad,
        mem68k_fetch_memcrd_byte, mem68k_fetch_memcrd_word,
        mem68k_fetch_memcrd_long,
        mem68k_store_memcrd_byte, mem68k_store_memcrd_word,
        mem68k_store_memcrd_long
    },

    /* SETTING DIVER */
    {
        0x3A0, 0x3a0, mem68k_memptr_bad,
        mem68k_fetch_invalid_byte, mem68k_fetch_invalid_word,
        mem68k_fetch_invalid_long,
        mem68k_store_setting_byte, mem68k_store_setting_word,
        mem68k_store_setting_long
    },

    {0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL}

};

/* @note (Tmesys#1#12/04/2022): These variables are used to communicate with gen68k library, they can not be static. */
Uint8* ( *mem68k_memptr[0x1000] ) ( Uint32 addr );
Uint8 ( *mem68k_fetch_byte[0x1000] ) ( Uint32 addr );
Uint16 ( *mem68k_fetch_word[0x1000] ) ( Uint32 addr );
Uint32 ( *mem68k_fetch_long[0x1000] ) ( Uint32 addr );
void ( *mem68k_store_byte[0x1000] ) ( Uint32 addr, Uint8 data );
void ( *mem68k_store_word[0x1000] ) ( Uint32 addr, Uint16 data );
void ( *mem68k_store_long[0x1000] ) ( Uint32 addr, Uint32 data );

/* ******************************************************************************************************************/
/*!
* \brief Byte Invalid fetching.
*
* \param address Memory address.
* \return Always 0xF0.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_invalid_byte ( Uint32 address )
{
    zlog_error ( gngeox_config.loggingCat, "Invalid fetch at address %x", address );
    return ( 0xF0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Word Invalid fetching.
*
* \param address Memory address.
* \return Always 0xF0F0.
*/
/* ******************************************************************************************************************/
static Uint16 mem68k_fetch_invalid_word ( Uint32 address )
{
    zlog_error ( gngeox_config.loggingCat, "Invalid fetch at address %x", address );
    return ( 0xF0F0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Long Invalid fetching.
*
* \param address Memory address.
* \return Always 0xF0F0F0F0.
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_invalid_long ( Uint32 address )
{
    zlog_error ( gngeox_config.loggingCat, "Invalid fetch at address %x", address );
    return ( 0xF0F0F0F0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches byte located at a specified memory address.
*
* \param address Memory address to fetch from.
* \return Fetched Byte.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_ram_byte ( Uint32 address )
{
    return ( READ_BYTE_ROM ( neogeo_memory.ram + QLOWORD ( address ) ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in ram.
*
* \param address Memory address to fetch from.
* \return Fetched Byte.
*/
/* ******************************************************************************************************************/
static Uint16 mem68k_fetch_ram_word ( Uint32 address )
{
    return ( READ_WORD_ROM ( neogeo_memory.ram + QLOWORD ( address ) ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches long located in ram.
*
* \param address Memory address to fetch from.
* \return Fetched long.
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_ram_long ( Uint32 address )
{
    return ( QMAKEWORD32 ( mem68k_fetch_ram_word ( address + 2 ), mem68k_fetch_ram_word ( address ) ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches byte located in cpu.
*
* \param address Memory address to fetch from.
* \return Fetched byte.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_cpu_byte ( Uint32 address )
{
    address &= 0xFFFFF;

    return ( READ_BYTE_ROM ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p + address ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in cpu.
*
* \param address Memory address to fetch from.
* \return Fetched byte.
*/
/* ******************************************************************************************************************/
Uint16 mem68k_fetch_cpu_word ( Uint32 address )
{
    address &= 0xFFFFF;

    return ( READ_WORD_ROM ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p + address ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches long located in cpu.
*
* \param address Memory address to fetch from.
* \return Fetched long.
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_cpu_long ( Uint32 address )
{
    return ( QMAKEWORD32 ( mem68k_fetch_cpu_word ( address + 2 ), mem68k_fetch_cpu_word ( address ) ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches byte located in bios.
*
* \param address Memory address to fetch from.
* \return Fetched byte.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_bios_byte ( Uint32 address )
{
    address &= 0x1FFFF;

    return ( READ_BYTE_ROM ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_BIOS].p + address ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in bios.
*
* \param address Memory address to fetch from.
* \return Fetched word.
*/
/* ******************************************************************************************************************/
static Uint16 mem68k_fetch_bios_word ( Uint32 address )
{
    address &= 0x1FFFF;

    return ( READ_WORD_ROM ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_BIOS].p + address ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in bios.
*
* \param address Memory address to fetch from.
* \return Fetched word.
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_bios_long ( Uint32 address )
{
    return ( mem68k_fetch_bios_word ( address ) << 16 ) | mem68k_fetch_bios_word ( address + 2 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches byte located in sram.
*
* \param address Memory address to fetch from.
* \return Fetched byte.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_sram_byte ( Uint32 address )
{
    return ( neogeo_memory.sram[address - 0xd00000] );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in sram.
*
* \param address Memory address to fetch from.
* \return Fetched word.
*/
/* ******************************************************************************************************************/
static Uint16 mem68k_fetch_sram_word ( Uint32 address )
{
    address -= 0xd00000;

    return ( neogeo_memory.sram[address] << 8 ) | ( neogeo_memory.sram[address + 1] & 0xff );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches long located in sram.
*
* \param address Memory address to fetch from.
* \return Fetched long.
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_sram_long ( Uint32 address )
{
    return ( mem68k_fetch_sram_word ( address ) << 16 ) | mem68k_fetch_sram_word ( address + 2 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches byte located in palette.
*
* \param address Memory address to fetch from.
* \return Fetched byte.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_pal_byte ( Uint32 address )
{
    address &= 0xffff;

    if ( address <= 0x1fff )
    {
        return ( current_pal[address] );
    }

    return ( 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in palette.
*
* \param address Memory address to fetch from.
* \return Fetched word.
*/
/* ******************************************************************************************************************/
static Uint16 mem68k_fetch_pal_word ( Uint32 address )
{
    address &= 0xffff;

    if ( address <= 0x1fff )
    {
        return ( READ_WORD ( &current_pal[address] ) );
    }

    return ( 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches long located in palette.
*
* \param address Memory address to fetch from.
* \return Fetched long.
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_pal_long ( Uint32 addr )
{
    return ( mem68k_fetch_pal_word ( addr ) << 16 ) | mem68k_fetch_pal_word ( addr + 2 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches byte located in video.
*
* \param address Memory address to fetch from.
* \return Fetched byte.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_video_byte ( Uint32 address )
{
    if ( ! ( address & 0x1 ) )
    {
        return ( mem68k_fetch_video_word ( address ) >> 8 );
    }
    else
    {
        Uint32 lpc = cpu_68k_getpc() + 2;

        switch ( ( lpc & 0xF00000 ) >> 20 )
        {
        case ( 0x0 ) :
            {
                return ( READ_WORD ( &neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p + ( lpc & 0xFFFFF ) ) );
            }
            break;

        case ( 0x2 ) :
            {
                return ( READ_WORD ( &neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p + bankaddress + ( lpc & 0xFFFFF ) ) );
            }
            break;

        case ( 0xC ) :
            {
                if ( lpc <= 0xc1FFff )
                {
                    return ( READ_WORD ( &neogeo_memory.rom.rom_region[REGION_MAIN_CPU_BIOS].p + ( lpc & 0xFFFFF ) ) );
                }
            }
            break;
        }
    }

    return ( 0xFF );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in video.
*
* \param address Memory address to fetch from.
* \return Fetched word.
*/
/* ******************************************************************************************************************/
static Uint16 mem68k_fetch_video_word ( Uint32 address )
{
    address &= 0x7;

    if ( address == 0x00 || address == 0x02 || address == 0x0a )
    {
        return ( neogeo_memory.vid.rbuf );  //READ_WORD(&neogeo_memory.vid.ram[neogeo_memory.vid.vptr << 1]);
    }

    if ( address == 0x04 )
    {
        return ( neogeo_memory.vid.modulo );
    }

    if ( address == 0x06 )
    {
        /* @note (Tmesys#1#12/04/2022): inlined read_neo_control */
        Uint32 scan = 0;

        if ( !gngeox_config.raster )
        {
            /* current scan-line */
            scan = cpu_68k_getcycle() / 766.28;

            //  scan+=0x100;
            //  if (scan >=0x200) scan=scan-0x108;
            scan += 0xF8;

            return ( scan << 7 ) | ( gngeox_config.forcepal << 3 ) | ( neogeo_frame_counter & 0x0007 ); /* frame counter */
        }
        else
        {
            scan = current_line /*+ 22*/; /* current scanline */
            //scan+=0x110;
            //if (scan >=0x200) scan=scan-0x108;
            scan += 0xF8;

            return ( scan << 7 ) | ( gngeox_config.forcepal << 3 ) | ( neogeo_frame_counter & 0x0007 ); /* frame counter */
        }
    }

    return ( 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches long located in video.
*
* \param address Memory address to fetch from.
* \return Fetched long.
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_video_long ( Uint32 address )
{
    return ( mem68k_fetch_video_word ( address ) << 16 ) | mem68k_fetch_video_word ( address + 2 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches byte located in control 1.
*
* \param address Memory address to fetch from.
* \return Fetched byte.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_ctl1_byte ( Uint32 address )
{
    Uint8 return_value = 0;

    switch ( QLOWORD ( address ) )
    {
    case ( 0x00 ) :
        {
            return_value = neogeo_memory.p1cnt;
        }
        break;
    case ( 0x01 ) :
        {
            return_value = neogeo_memory.test_switch ? 0xFE : 0xFF;
        }
        break;
    case ( 0x81 ) :
        {
            return_value = neogeo_memory.test_switch ? 0x00 : 0x80;
        }
        break;
    default:
        {
            zlog_error ( gngeox_config.loggingCat, "Unknown address %x", address );
        }
        break;
    }

    return ( return_value );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in control 1.
*
* \param address Memory address to fetch from.
* \return Fetched word.
*/
/* ******************************************************************************************************************/
static Uint16 mem68k_fetch_ctl1_word ( Uint32 address )
{
    /* @note (Tmesys#1#10/04/2024): Experimental used by diggerma */
    return ( QMAKEWORD16 ( mem68k_fetch_ctl1_byte ( address ), mem68k_fetch_ctl1_byte ( address ) ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches long located in control 1.
*
* \param address Memory address to fetch from.
* \return Fetched long.
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_ctl1_long ( Uint32 address )
{
    zlog_error ( gngeox_config.loggingCat, "Not implemented at address %x", address );

    return ( 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches byte located in control 2.
*
* \param address Memory address to fetch from.
* \return Fetched byte.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_ctl2_byte ( Uint32 address )
{
    Uint8 return_value = 0;

    switch ( QLOWORD ( address ) )
    {
    case ( 0x00 ) :
        {
            return_value = neogeo_memory.p2cnt;
        }
        break;
    case ( 0x01 ) :
        {
            return_value = 0xFF;
        }
        break;
    default:
        {
            zlog_error ( gngeox_config.loggingCat, "Unknown address %x", address );
        }
        break;
    }

    return ( return_value );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in control 2.
*
* \param address Memory address to fetch from.
* \return Fetched word.
*/
/* ******************************************************************************************************************/
static Uint16 mem68k_fetch_ctl2_word ( Uint32 address )
{
    /* @note (Tmesys#1#10/04/2024): Experimental used by diggerma */
    return ( QMAKEWORD16 ( mem68k_fetch_ctl2_byte ( address ), mem68k_fetch_ctl2_byte ( address ) ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches long located in control 2.
*
* \param address Memory address to fetch from.
* \return Fetched long.
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_ctl2_long ( Uint32 address )
{
    zlog_error ( gngeox_config.loggingCat, "Not implemented at address %x", address );

    return ( 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches byte located in control 2.
*
* \param address Memory address to fetch from.
* \return Fetched byte.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_ctl3_byte ( Uint32 address )
{
    Uint8 return_value = 0;

    if ( QLOWORD ( address ) == 0x0 )
    {
        return_value = neogeo_memory.status_b;
    }
    else
    {
        return_value = 0;
        zlog_error ( gngeox_config.loggingCat, "Unknown address %x", address );
    }

    return ( return_value );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in control 3.
*
* \param address Memory address to fetch from.
* \return Fetched word.
*/
/* ******************************************************************************************************************/
static Uint16 mem68k_fetch_ctl3_word ( Uint32 address )
{
    /* @note (Tmesys#1#10/04/2024): Experimental used by diggerma */
    return ( QMAKEWORD16 ( mem68k_fetch_ctl3_byte ( address ), mem68k_fetch_ctl3_byte ( address ) ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches long located in control 3.
*
* \param address Memory address to fetch from.
* \return Fetched long.
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_ctl3_long ( Uint32 address )
{
    zlog_error ( gngeox_config.loggingCat, "Not implemented at address %x", address );

    return ( 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches byte located in coin.
*
* \param address Memory address to fetch from.
* \return Fetched byte.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_coin_byte ( Uint32 address )
{
    Uint8 result_value = 0;

    switch ( QLOWORD ( address ) )
    {
    case ( 0x0 ) :
        {
            result_value = neogeo_memory.z80_command_reply;
        }
        break;
    case ( 0x1 ) :
        {
            Sint32 rtc_time_pulse = read_4990_testbit();
            Sint32 rtc_databit = read_4990_databit();
            result_value = ( neogeo_memory.status_a ^ ( rtc_time_pulse << 6 ) ^ ( rtc_databit << 7 ) );
        }
        break;
    default:
        {
            zlog_warn ( gngeox_config.loggingCat, "Unknown Address at %x", address );
            result_value = 0;
        }
        break;
    }

    return ( result_value );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in coin.
*
* \param address Memory address to fetch from.
* \return Fetched word.
*/
/* ******************************************************************************************************************/
static Uint16 mem68k_fetch_coin_word ( Uint32 address )
{
    zlog_error ( gngeox_config.loggingCat, "Not implemented at address %x", address );

    return ( 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches long located in coin.
*
* \param address Memory address to fetch from.
* \return Fetched long.
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_coin_long ( Uint32 address )
{
    zlog_error ( gngeox_config.loggingCat, "Not implemented at address %x", address );

    return ( 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches byte located in memory card.
*
* \attention Even byte are FF, Odd  byte are data
* \param address Memory address to fetch from.
* \return Fetched byte.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_memcrd_byte ( Uint32 address )
{
    address &= 0xFFF;

    if ( address & 1 )
    {
        return 0xFF;
    }
    else
    {
        return ( neogeo_memory.memcard[address >> 1] );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in memory card.
*
* \attention Even byte are FF, Odd  byte are data
* \param address Memory address to fetch from.
* \return Fetched word.
*/
/* ******************************************************************************************************************/
static Uint16 mem68k_fetch_memcrd_word ( Uint32 address )
{
    address &= 0xFFF;

    return ( neogeo_memory.memcard[address >> 1] | 0xff00 );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in memory card.
*
* \attention Even byte are FF, Odd  byte are data
* \param address Memory address to fetch from.
* \return Fetched word.
*
* \note used by UNIBIOS
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_memcrd_long ( Uint32 address )
{
    zlog_error ( gngeox_config.loggingCat, "Not implemented at address %x", address );

    return ( 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Byte Invalid store.
*
* \param address Memory address.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_invalid_byte ( Uint32 address, Uint8 data )
{
    switch ( address )
    {
    case ( 0x300001 ) :
        {
            neogeo_memory.watchdog = 0;
        }
        break;
    default:
        {
        }
        zlog_error ( gngeox_config.loggingCat, "Invalid : Address at %x, value = %x", address, data );
        break;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Word Invalid store.
*
* \param address Memory address.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_invalid_word ( Uint32 address, Uint16 data )
{
    zlog_error ( gngeox_config.loggingCat, "Invalid write at address %x, value = %x", address, data );
}
/* ******************************************************************************************************************/
/*!
* \brief Long Invalid store.
*
* \param address Memory address.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_invalid_long ( Uint32 address, Uint32 data )
{
    zlog_error ( gngeox_config.loggingCat, "Invalid write at address %x, value = %x", address, data );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores byte at a specified memory address in ram.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_ram_byte ( Uint32 address, Uint8 data )
{
    address &= 0xffff;
    WRITE_BYTE_ROM ( neogeo_memory.ram + address, data );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores word at a specified memory address in ram.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_ram_word ( Uint32 address, Uint16 data )
{
    address &= 0xffff;
    WRITE_WORD_ROM ( neogeo_memory.ram + address, data );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores long at a specified memory address in ram.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_ram_long ( Uint32 address, Uint32 data )
{
    mem68k_store_ram_word ( address, data >> 16 );
    mem68k_store_ram_word ( address + 2, data & 0xffff );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores byte at a specified memory address in memory bank.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_bk_normal_byte ( Uint32 address, Uint8 data )
{
    //if (address<0x2FFFF0)
    switch_bank ( address, data );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores word at a specified memory address in memory bank.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_bk_normal_word ( Uint32 address, Uint16 data )
{
    //if (address<0x2FFFF0)
    if ( neogeo_memory.bksw_unscramble && ( address & 0xFF ) == neogeo_memory.bksw_unscramble[0] )
    {
        /* unscramble bank number */
        data =
            ( ( ( data >> neogeo_memory.bksw_unscramble[1] ) & 1 ) << 0 ) +
            ( ( ( data  >> neogeo_memory.bksw_unscramble[2] ) & 1 ) << 1 ) +
            ( ( ( data  >> neogeo_memory.bksw_unscramble[3] ) & 1 ) << 2 ) +
            ( ( ( data  >> neogeo_memory.bksw_unscramble[4] ) & 1 ) << 3 ) +
            ( ( ( data  >> neogeo_memory.bksw_unscramble[5] ) & 1 ) << 4 ) +
            ( ( ( data  >> neogeo_memory.bksw_unscramble[6] ) & 1 ) << 5 );

        bankaddress = 0x100000 + neogeo_memory.bksw_offset[data];
        cpu_68k_bankswitch ( bankaddress );
    }
    else
    {
        switch_bank ( address, data );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Stores long at a specified memory address in memory bank.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_bk_normal_long ( Uint32 address, Uint32 data )
{
    mem68k_store_bk_normal_word ( address, QHIWORD ( data ) );
    mem68k_store_bk_normal_word ( address + 2, QLOWORD ( data ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores byte at a specified memory address in sram.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_sram_byte ( Uint32 address, Uint8 data )
{
    if ( sram_lock == SDL_FALSE )
    {
        neogeo_memory.sram[address - 0xd00000] = data;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Stores word at a specified memory address in sram.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_sram_word ( Uint32 address, Uint16 data )
{
    if ( sram_lock == SDL_FALSE )
    {
        /* @todo (Tmesys#1#07/04/2024): Why ? */
        address -= 0xd00000;
        neogeo_memory.sram[address] = QHIBYTE ( data );
        neogeo_memory.sram[address + 1] = QLOBYTE ( data );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Stores long at a specified memory address in sram.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
void mem68k_store_sram_long ( Uint32 address, Uint32 data )
{
    mem68k_store_sram_word ( address, QHIWORD ( data ) );
    mem68k_store_sram_word ( address + 2, QLOWORD ( data ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores byte at a specified memory address in palette.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_pal_byte ( Uint32 address, Uint8 data )
{
    /* @todo (Tmesys#1#12/07/2022): Verify this */
    address &= 0xffff;

    if ( address <= 0x1fff )
    {
        Uint16 a = READ_WORD ( &current_pal[address & 0xfffe] );

        if ( address & 0x1 )
        {
            a = data | ( a & 0xff00 );
        }
        else
        {
            a = ( a & 0xff ) | ( data << 8 );
        }

        WRITE_WORD ( &current_pal[address & 0xfffe], a );

        if ( ( address >> 1 ) & 0xF )
        {
            current_pc_pal[ ( address ) >> 1] = convert_pal ( a );
        }
        else
        {
            current_pc_pal[ ( address ) >> 1] = 0xF81F;
        }
    }
    else
    {
        zlog_error ( gngeox_config.loggingCat, "Address out of bound ? %x", address );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Stores word at a specified memory address in palette.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_pal_word ( Uint32 address, Uint16 data )
{
    address &= 0xffff;

    if ( address <= 0x1fff )
    {
        WRITE_WORD ( &current_pal[address], data );

        if ( ( address >> 1 ) & 0xF )
        {
            current_pc_pal[ ( address ) >> 1] = convert_pal ( data );
        }
        else
        {
            current_pc_pal[ ( address ) >> 1] = 0xF81F;
        }
    }
    else
    {
        zlog_error ( gngeox_config.loggingCat, "Address out of bound ? %x", address );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Stores long at a specified memory address in palette.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
void mem68k_store_pal_long ( Uint32 address, Uint32 data )
{
    mem68k_store_pal_word ( address, QHIWORD ( data ) );
    mem68k_store_pal_word ( address + 2, QLOWORD ( data ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores byte at a specified memory address in palette.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_video_byte ( Uint32 address, Uint8 data )
{
    /* @note (Tmesys#1#12/20/2022): Watchout garou / shocktr2 write at 3c001f, 3c000f, 3c0015 / wjammers write, and fetch at 3c0000 .... */
    zlog_warn ( gngeox_config.loggingCat, "Write at address %x, value = %x", address, data );
    mem68k_store_video_word ( address, QMAKEWORD16 ( data, data ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores word at a specified memory address in video.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
/* @fixme (Tmesys#1#12/04/2022): Some undefined references in neo_control and irq2pos. */
static void mem68k_store_video_word ( Uint32 address, Uint16 data )
{
    switch ( address )
    {
    case ( REG_VRAMADDR ) :
        {
            neogeo_memory.vid.vptr = data & 0xffff;
            neogeo_memory.vid.rbuf = READ_WORD ( &neogeo_memory.vid.ram[neogeo_memory.vid.vptr << 1] );
        }
        break;
    case ( REG_VRAMRW ) :
        {
            WRITE_WORD ( &neogeo_memory.vid.ram[neogeo_memory.vid.vptr << 1], data );
            neogeo_memory.vid.vptr = ( neogeo_memory.vid.vptr & 0x8000 ) + ( ( neogeo_memory.vid.vptr
                                     + neogeo_memory.vid.modulo ) & 0x7fff );
            neogeo_memory.vid.rbuf = READ_WORD ( &neogeo_memory.vid.ram[neogeo_memory.vid.vptr << 1] );
        }
        break;
    case ( REG_VRAMMOD ) :
        {
            if ( data & 0x4000 )
            {
                data |= 0x8000;
            }
            else
            {
                data &= 0x7FFF;
            }

            neogeo_memory.vid.modulo = ( Sint32 ) data;
        }
        break;
    case ( REG_LSPCMODE ) :
        {
            write_neo_control ( data );
        }
        break;
    case ( REG_TIMERHIGH ) :
        {
            write_irq2pos ( ( neogeo_memory.vid.irq2pos & 0xffff ) | ( ( Uint32 ) data << 16 ) );
        }
        break;
    case ( REG_TIMERLOW ) :
        {
            write_irq2pos ( ( neogeo_memory.vid.irq2pos & 0xffff0000 ) | ( Uint32 ) data );
        }
        break;
    case ( REG_IRQACK ) :
        {
            /* @todo (Tmesys#1#05/04/2024): Implement IRQ acknowledge */
        }
        break;
    default :
        {
            /* @fixme (Tmesys#1#12/24/2022): $3C000E is REG_TIMERSTOP : Bit 0=1: Stops timer counter during first and last 16 lines (32 total) when in PAL mode. */
            zlog_error ( gngeox_config.loggingCat, "Unknown write at address %x, value = %x", address, data );
        }
        break;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Stores long at a specified memory address in video.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_video_long ( Uint32 address, Uint32 data )
{
    mem68k_store_video_word ( address, QHIWORD ( data ) );
    mem68k_store_video_word ( address + 2, QLOWORD ( data ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores byte at a specified memory address in pd4990 controller.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
/* @todo (Tmesys#1#12/04/2022): This should be replaced directly by write_4990_control_w */
static void mem68k_store_pd4990_byte ( Uint32 address, Uint8 data )
{
    write_4990_control_w ( address, data );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores word at a specified memory address in pd4990 controller.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
/* @todo (Tmesys#1#12/04/2022): This should be replaced directly by write_4990_control_w */
static void mem68k_store_pd4990_word ( Uint32 address, Uint16 data )
{
    write_4990_control_w ( address, data );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores long at a specified memory address in pd4990 controller.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
/* @todo (Tmesys#1#12/04/2022): This should be replaced directly by write_4990_control_w */
static void mem68k_store_pd4990_long ( Uint32 address, Uint32 data )
{
    write_4990_control_w ( address, data );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores byte at a specified memory address in Z80.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_z80_byte ( Uint32 address, Uint8 data )
{
    switch ( address )
    {
    /*
    The sound command is read from 0000H but a sound acknowledge (or return
    data) is written to 000CH. On the 68000 side, the single location 320000H is
    written or read.
    */
    case ( 0x320000 ) :
        {
            neogeo_memory.z80_command = data;

            neo_z80_nmi();
            neo_z80_run ( 300 );
        }
        break;
    default:
        {
            zlog_error ( gngeox_config.loggingCat, "Unhandled address : %x", address );
        }
        break;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Stores word at a specified memory address in Z80.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_z80_word ( Uint32 address, Uint16 data )
{
    switch ( address )
    {
    /*
    The sound command is read from 0000H but a sound acknowledge (or return
    data) is written to 000CH. On the 68000 side, the single location 320000H is
    written or read.
    */
    /* @note (Tmesys#1#04/04/2024): tpgolf use word store for sound */
    case ( 0x320000 ) :
        {
            neogeo_memory.z80_command = QHIBYTE ( data );

            neo_z80_nmi();
            neo_z80_run ( 300 );
        }
        break;
    default:
        {
            zlog_error ( gngeox_config.loggingCat, "Unhandled address : %x", address );
        }
        break;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Stores long at a specified memory address in Z80.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_z80_long ( Uint32 address, Uint32 data )
{
    /* I don't think any game will use long store for sound.... */
    zlog_warn ( gngeox_config.loggingCat, "Write at address %x, value = %x", address, data );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores byte at a specified memory address in settings.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_setting_byte ( Uint32 address, Uint8 data )
{
    switch ( address )
    {
    case ( REG_NOSHADOW ) :
        {
        }
        break;
    case ( REG_SWPBIOS ) :
        {
            zlog_info ( gngeox_config.loggingCat, "Selecting Bios Vector" );
            memcpy ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p, neogeo_memory.rom.rom_region[REGION_MAIN_CPU_BIOS].p, 0x80 );
            neogeo_memory.current_vector = 0;
        }
        break;
    case ( REG_SWPROM ) :
        {
            zlog_info ( gngeox_config.loggingCat, "Selecting Game Vector" );
            memcpy ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p, neogeo_memory.game_vector, 0x80 );
            neogeo_memory.current_vector = 1;
        }
        break;
    /* select board fix */
    case ( REG_BRDFIX ) :
        {
            zlog_info ( gngeox_config.loggingCat, "Selecting board fix" );
            current_fix = neogeo_memory.rom.rom_region[REGION_FIXED_LAYER_BIOS].p;
            fix_usage = neogeo_memory.fix_board_usage;
            neogeo_memory.vid.currentfix = 0;
        }
        break;
    /* select game fix */
    case ( REG_CRTFIX ) :
        {
            zlog_info ( gngeox_config.loggingCat, "Selecting game fix" );
            current_fix = neogeo_memory.rom.rom_region[REGION_FIXED_LAYER_CARTRIDGE].p;
            fix_usage = neogeo_memory.fix_game_usage;
            neogeo_memory.vid.currentfix = 1;
        }
        break;
    /* sram lock */
    case ( REG_SRAMLOCK ) :
        {
            sram_lock = SDL_TRUE;
        }
        break;
    /* sram unlock */
    case ( REG_SRAMUNLOCK ) :
        {
            sram_lock = SDL_FALSE;
        }
        break;
    /* set palette 2 */
    case ( REG_PALBANK1 ) :
        {
            current_pal = neogeo_memory.vid.pal_neo[1];
            current_pc_pal = ( Uint32* ) neogeo_memory.vid.pal_host[1];
            neogeo_memory.vid.currentpal = 1;
        }
        break;
    /* set palette 1 */
    case ( REG_PALBANK0 ) :
        {
            current_pal = neogeo_memory.vid.pal_neo[0];
            current_pc_pal = ( Uint32* ) neogeo_memory.vid.pal_host[0];
            neogeo_memory.vid.currentpal = 0;
        }
        break;
    default:
        {
            /* garou write 0 to 3a0001 -> enable display, 3a0011 -> disable display */
            zlog_error ( gngeox_config.loggingCat, "Unknown write at address %x, value = %x", address, data );
        }
        break;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Stores word at a specified memory address in settings.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_setting_word ( Uint32 address, Uint16 data )
{
    /* @todo (Tmesys#1#12/07/2022): Some game use it ? */
    zlog_warn ( gngeox_config.loggingCat, "Write at address %x, value = %x", address, data );
    mem68k_store_setting_byte ( address, QLOBYTE ( data ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores long at a specified memory address in settings.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_setting_long ( Uint32 address, Uint32 data )
{
    zlog_error ( gngeox_config.loggingCat, "Not implemented at address %x : data %x", address, data );
}
/* ******************************************************************************************************************/
/*!
* \brief Stores byte at a specified memory address in memory card.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_memcrd_byte ( Uint32 address, Uint8 data )
{
    address &= 0xFFF;
    neogeo_memory.memcard[address >> 1] = data;
}
/* ******************************************************************************************************************/
/*!
* \brief Stores word at a specified memory address in memory card.
*
* \param address Memory address where to store.
* \param data Data to store.
*/
/* ******************************************************************************************************************/
static void mem68k_store_memcrd_word ( Uint32 address, Uint16 data )
{
    address &= 0xFFF;
    neogeo_memory.memcard[address >> 1] = data & 0xff;
}
/* ******************************************************************************************************************/
/*!
* \brief Stores long at a specified memory address in memory card.
*
* \param address Memory address where to store.
* \param data Data to store.
*
* \note used by UNIBIOS
*/
/* ******************************************************************************************************************/
static void mem68k_store_memcrd_long ( Uint32 address, Uint32 data )
{
    zlog_error ( gngeox_config.loggingCat, "Not implemented at address %x : data %x", address, data );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches byte located in memory bank ?
*
* \note Normal bankswitcher.
* \param address Memory address to fetch from.
* \return Fetched byte.
*/
/* ******************************************************************************************************************/
static Uint8 mem68k_fetch_bk_normal_byte ( Uint32 address )
{
    /* SMA prot & random number generator */
    if ( neogeo_memory.bksw_unscramble )
    {
        Uint32 a = address & 0xFFFFE;

        if ( a == 0xfe446 )
        {
            return ( address & 0x1 ? 0x9a : 0x37 );
        }

        if ( neogeo_memory.sma_rng_addr && address >= 0x2fff00 &&
                ( ( ( a & 0xFF ) == ( neogeo_memory.sma_rng_addr & 0xFF ) ) ||
                  ( ( a & 0xFF ) == neogeo_memory.sma_rng_addr >> 8 ) ) )
        {
            zlog_warn ( gngeox_config.loggingCat, "SMA random at address %x", address );
            return ( address & 0x1 ? sma_random() >> 8 : sma_random() & 0xFF );
        }
    }

    address &= 0xFFFFF;
    return ( READ_BYTE_ROM ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p + bankaddress + address ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches word located in memory bank ?
*
* \note Normal bankswitcher.
* \param address Memory address to fetch from.
* \return Fetched word.
*/
/* ******************************************************************************************************************/
static Uint16 mem68k_fetch_bk_normal_word ( Uint32 address )
{
    /* SMA prot & random number generator */
    if ( neogeo_memory.bksw_unscramble )
    {
        if ( ( address & 0xFFFFF ) == 0xfe446 )
        {
            return ( 0x9a37 );
        }

        if ( neogeo_memory.sma_rng_addr && address >= 0x2fff00 &&
                ( ( ( address & 0xFF ) == ( neogeo_memory.sma_rng_addr & 0xFF ) ) ||
                  ( ( address & 0xFF ) == neogeo_memory.sma_rng_addr >> 8 ) ) )
        {
            zlog_warn ( gngeox_config.loggingCat, "SMA random at address %x", address );
            return ( sma_random() );
        }
    }

    address &= 0xFFFFF;
    return ( READ_WORD_ROM ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p + bankaddress + address ) );
}
/* ******************************************************************************************************************/
/*!
* \brief Fetches long located in memory bank ?
*
* \note Normal bankswitcher.
* \param address Memory address to fetch from.
* \return Fetched long.
*/
/* ******************************************************************************************************************/
static Uint32 mem68k_fetch_bk_normal_long ( Uint32 address )
{
    return ( mem68k_fetch_bk_normal_word ( address ) << 16 ) | mem68k_fetch_bk_normal_word ( address + 2 );
}
/* ******************************************************************************************************************/
/*!
* \brief Swaps a memory area.
*
* \param mem Pointer to memory to swap.
* \param length Memory length.
*/
/* ******************************************************************************************************************/
static void swap_memory ( Uint8* mem, Uint32 length )
{
    Sint32 mem_tmp = 0;

    /* swap bytes in each word */
    for ( Uint32 index = 0; index < length; index += 2 )
    {
        mem_tmp = mem[index];
        mem[index] = mem[index + 1];
        mem[index + 1] = mem_tmp;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Initializes memory tables.
*
* \return Pointer to memory to swap.
*/
/* ******************************************************************************************************************/
/* @note (Tmesys#1#12/04/2022): Never prototype as static or change, declared in gen68k library (even if no need for a return,
keep it like this). */
Sint32 mem68k_init ( void )
{
    Sint32 index2 = 0;

    mem68k_def[2].fetch_byte = mem68k_fetch_bk_normal_byte;
    mem68k_def[2].fetch_word = mem68k_fetch_bk_normal_word;
    mem68k_def[2].fetch_long = mem68k_fetch_bk_normal_long;
    mem68k_def[2].store_byte = mem68k_store_bk_normal_byte;
    mem68k_def[2].store_word = mem68k_store_bk_normal_word;
    mem68k_def[2].store_long = mem68k_store_bk_normal_long;

    do
    {
        for ( Sint32 index1 = mem68k_def[index2].start; index1 <= mem68k_def[index2].end; index1++ )
        {
            mem68k_memptr[index1] = mem68k_def[index2].memptr;
            mem68k_fetch_byte[index1] = mem68k_def[index2].fetch_byte;
            mem68k_fetch_word[index1] = mem68k_def[index2].fetch_word;
            mem68k_fetch_long[index1] = mem68k_def[index2].fetch_long;
            mem68k_store_byte[index1] = mem68k_def[index2].store_byte;
            mem68k_store_word[index1] = mem68k_def[index2].store_word;
            mem68k_store_long[index1] = mem68k_def[index2].store_long;
        }

        index2++;
    }
    while ( ( mem68k_def[index2].start != 0 ) || ( mem68k_def[index2].end != 0 ) );

    return ( 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Returns a pointer to the start of some rom memory region ?
*
* \param address Unused address.
* \return Pointer to rom region memory.
* \note called for IPC generation so speed is not vital.
*/
/* ******************************************************************************************************************/
static Uint8* mem68k_memptr_bad ( Uint32 address )
{
    zlog_error ( gngeox_config.loggingCat, "Bad address %x ?", address );
    return neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p;
}
/* ******************************************************************************************************************/
/*!
* \brief Computes a pointer of some rom memory region ?
*
* \param address Address to compute.
* \return Pointer to rom region memory.
*/
/* ******************************************************************************************************************/
static Uint8* mem68k_memptr_cpu ( Uint32 address )
{
    return ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p + address );
}
/* ******************************************************************************************************************/
/*!
* \brief Computes a pointer of bios memory region from some address ?
*
* \param address Address to compute.
* \return Pointer to bios region memory.
*/
/* ******************************************************************************************************************/
static Uint8* mem68k_memptr_bios ( Uint32 address )
{
    address &= 0x1FFFF;
    return ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_BIOS].p + address );
}
/* ******************************************************************************************************************/
/*!
* \brief Returns a pointer to the start of some rom memory region ?
*
* \param address Address to compute.
* \return Pointer to bios region memory.
*/
/* ******************************************************************************************************************/
static Uint8* mem68k_memptr_cpu_bk ( Uint32 addr )
{
    addr &= 0xFFFFF;
    return ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p + addr + bankaddress );
}
/* ******************************************************************************************************************/
/*!
* \brief Returns a pointer to the start of ram memory region ?
*
* \param address Address to compute.
* \return Pointer to ram region memory.
*/
/* ******************************************************************************************************************/
static Uint8* mem68k_memptr_ram ( Uint32 address )
{
    address &= 0xFFFF;
    return ( neogeo_memory.ram + address );
}
/* ******************************************************************************************************************/
/*!
* \brief Switches memory bank.
*
* \param address Address to new memory bank.
*/
/* ******************************************************************************************************************/
void cpu_68k_bankswitch ( Uint32 address )
{
    bankaddress = address;
};
/* ******************************************************************************************************************/
/*!
* \brief Resets 68k emulator.
*
*/
/* ******************************************************************************************************************/
void cpu_68k_reset ( void )
{
    cpu68k_reset();
}
/* ******************************************************************************************************************/
/*!
* \brief Initializes 68k cpu.
*
*/
/* ******************************************************************************************************************/
void cpu_68k_init ( void )
{
    cpu68k_clearcache();

    if ( !gngeox_config.dump )
    {
        swap_memory ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].p, neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].size );

        if ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_BIOS].p[0] == 0x10 )
        {
            zlog_info ( gngeox_config.loggingCat, "Bios byte 1 value %x", neogeo_memory.rom.rom_region[REGION_MAIN_CPU_BIOS].p[0] );
            swap_memory ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_BIOS].p, neogeo_memory.rom.rom_region[REGION_MAIN_CPU_BIOS].size );
        }

        swap_memory ( neogeo_memory.game_vector, 0x80 );
    }

    cpu68k_ram = neogeo_memory.ram;

    mem68k_init();
    cpu68k_init();

    if ( neogeo_memory.rom.rom_region[REGION_MAIN_CPU_CARTRIDGE].size > 0x100000 )
    {
        cpu_68k_bankswitch ( 0 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Runs 68k cpu.
*
* \param nb_cycles Number of clock cycles to execute at least.
* \return Number of clocks executed too much.
*/
/* ******************************************************************************************************************/
Sint32 cpu_68k_run ( Uint32 nb_cycles )
{
    Uint32 excedent_cycles = 0;

    excedent_cycles = reg68k_external_execute ( nb_cycles );

    cpu68k_endfield();

    return ( excedent_cycles );
}
/* ******************************************************************************************************************/
/*!
* \brief Gives current program counter value.
*
* \return Program counter.
**/
/* ******************************************************************************************************************/
Uint32 cpu_68k_getpc ( void )
{
    return regs.pc;
}
/* ******************************************************************************************************************/
/*!
* \brief Executes one instruction.
*
* \return Number of clock cycles elapsed.
**/
/* ******************************************************************************************************************/
Sint32 cpu_68k_run_step ( void )
{
    return reg68k_external_step();
}

/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
* \param auto_vector Todo.
**/
/* ******************************************************************************************************************/
void cpu_68k_interrupt ( Sint32 auto_vector )
{
    reg68k_external_autovector ( auto_vector );
}
/* ******************************************************************************************************************/
/*!
* \brief Retrieve current clock cycles value.
*
* \return Current clock cycles value.
**/
/* ******************************************************************************************************************/
Sint32 cpu_68k_getcycle ( void )
{
    return cpu68k_clocks;
}

#ifdef _GNGEOX_GEN68K_INTERF_C_
#undef _GNGEOX_GEN68K_INTERF_C_
#endif // _GNGEOX_GEN68K_INTERF_C_
