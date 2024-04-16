/*!
*
*   \file    GnGeoX68k.h
*   \brief   Interface to the generator68k emulator header.
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

#ifndef _GNGEOX_68K_H_
#define _GNGEOX_68K_H_

/* System registers */
enum
{
    /* Normal video output */
    REG_NOSHADOW = 0x3A0001,
    /* Darken video output */
    REG_SHADOW = 0x3A0011,
    /* BIOS vector table */
    REG_SWPBIOS = 0x3A0003,
    /* Use the cart's vector table */
    REG_SWPROM = 0x3A0013,
    /* Enable writes to memory card (unused in CD systems) */
    REG_CRDUNLOCK1 = 0x3A0005,
    /* Disable writes to memory card (unused in CD systems) */
    REG_CRDLOCK1 = 0x3A0015,
    /* Disable writes to memory card (unused in CD systems) */
    REG_CRDLOCK2 = 0x3A0007,
    /* Enable writes to memory card (unused in CD systems) */
    REG_CRDUNLOCK2 = 0x3A0017,
    /* Enable "Register select" for memory card */
    REG_CRDREGSEL = 0x3A0009,
    /* Disable "Register select" for memory card */
    REG_CRDNORMAL = 0x3A0019,
    /* Use the embedded SFIX ROM and SM1 ROM */
    REG_BRDFIX = 0x3A000B,
    /* Use the cart's S ROM and M1 ROM */
    REG_CRTFIX = 0x3A001B,
    /* Write-protects backup RAM (MVS) */
    REG_SRAMLOCK = 0x3A000D,
    /* Unprotects backup RAM (MVS) */
    REG_SRAMUNLOCK = 0x3A001D,
    /* Use palette bank 1 */
    REG_PALBANK1 = 0x3A000F,
    /* Use palette bank 0 */
    REG_PALBANK0 = 0x3A001F,
};

/* Video registers */
enum
{
    /* Read : Read from VRAM (address doesn't change) */
    /* Write : Sets VRAM address */
    REG_VRAMADDR = 0x3C0000,
    /* Read : Read from VRAM (address doesn't change) */
    /* Write : Write to VRAM (modulo is applied after) */
    REG_VRAMRW = 0x3C0002,
    /* Read : Reads VRAM address modulo */
    /* Write : Sets VRAM address modulo */
    REG_VRAMMOD = 0x3C0004,
    /* Read :  */
    /* Write :  */
    REG_LSPCMODE = 0x3C0006,
    /* Read : Like REG_VRAMADDR. */
    /* Write : 16 highest bits of the timer reload value. */
    REG_TIMERHIGH = 0x3C0008,
    /* Read : Like REG_VRAMRW. */
    /* Write : 16 lowest bits of the timer reload value. */
    REG_TIMERLOW = 0x3C000A,
    /* Read : Like REG_VRAMMOD. */
    /* Write : Interrupt acknowledge. */
    REG_IRQACK = 0x3C000C,
    /* Read : Like REG_LSPCMODE. */
    /* Write : Bit 0=1: Stops timer counter during first and last 16 lines (32 total) when in PAL mode. */
    REG_TIMERSTOP = 0x3C000E,
};

/* REG_IRQACK */
enum
{
    REG_IRQACK_IRQ3 = 0,
    REG_IRQACK_TIMER = 1,
    REG_IRQACK_VBLANK = 2,
    REG_IRQACK_NUSED1 = 3,
    REG_IRQACK_NUSED2 = 4,
    REG_IRQACK_NUSED3 = 5,
    REG_IRQACK_NUSED4 = 6,
    REG_IRQACK_NUSED5 = 7,
};

#ifdef _GNGEOX_68K_C_
static Uint8 mem68k_fetch_invalid_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_invalid_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_invalid_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8 mem68k_fetch_ram_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_ram_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_ram_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8 mem68k_fetch_cpu_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_cpu_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_cpu_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8 mem68k_fetch_bios_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_bios_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_bios_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8 mem68k_fetch_sram_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_sram_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_sram_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8 mem68k_fetch_pal_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_pal_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_pal_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8 mem68k_fetch_video_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_video_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_video_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8 mem68k_fetch_ctl1_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_ctl1_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_ctl1_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8 mem68k_fetch_ctl2_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_ctl2_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_ctl2_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8 mem68k_fetch_ctl3_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_ctl3_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_ctl3_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8 mem68k_fetch_coin_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_coin_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_coin_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8 mem68k_fetch_memcrd_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_memcrd_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_memcrd_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8 mem68k_fetch_bk_normal_byte ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint16 mem68k_fetch_bk_normal_word ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint32 mem68k_fetch_bk_normal_long ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static void mem68k_store_invalid_byte ( Uint32, Uint8 );
static void mem68k_store_invalid_word ( Uint32, Uint16 );
static void mem68k_store_invalid_long ( Uint32, Uint32 );
static void mem68k_store_ram_byte ( Uint32, Uint8 );
static void mem68k_store_ram_word ( Uint32, Uint16 );
static void mem68k_store_ram_long ( Uint32, Uint32 );
static void mem68k_store_bk_normal_byte ( Uint32, Uint8 );
static void mem68k_store_bk_normal_word ( Uint32, Uint16 );
static void mem68k_store_bk_normal_long ( Uint32, Uint32 );
static void mem68k_store_sram_byte ( Uint32, Uint8 );
static void mem68k_store_sram_word ( Uint32, Uint16 );
static void mem68k_store_sram_long ( Uint32, Uint32 );
static void mem68k_store_pal_byte ( Uint32, Uint8 );
static void mem68k_store_pal_word ( Uint32, Uint16 );
static void mem68k_store_pal_long ( Uint32, Uint32 );
static void mem68k_store_video_byte ( Uint32, Uint8 );
static void mem68k_store_video_word ( Uint32, Uint16 );
static void mem68k_store_video_long ( Uint32, Uint32 );
static void mem68k_store_pd4990_byte ( Uint32, Uint8 );
static void mem68k_store_pd4990_word ( Uint32, Uint16 );
static void mem68k_store_pd4990_long ( Uint32, Uint32 );
static void mem68k_store_z80_byte ( Uint32, Uint8 );
static void mem68k_store_z80_word ( Uint32, Uint16 );
static void mem68k_store_z80_long ( Uint32, Uint32 );
static void mem68k_store_setting_byte ( Uint32, Uint8 );
static void mem68k_store_setting_word ( Uint32, Uint16 );
static void mem68k_store_setting_long ( Uint32, Uint32 );
static void mem68k_store_memcrd_byte ( Uint32, Uint8 );
static void mem68k_store_memcrd_word ( Uint32, Uint16 );
static void mem68k_store_memcrd_long ( Uint32, Uint32 );
static Uint8* mem68k_memptr_bad ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8* mem68k_memptr_cpu ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8* mem68k_memptr_bios ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8* mem68k_memptr_cpu_bk ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
static Uint8* mem68k_memptr_ram ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
#endif // _GNGEOX_68K_C_

void cpu_68k_bankswitch ( Uint32 );
void cpu_68k_reset ( void );
void cpu_68k_init ( void );
Sint32 cpu_68k_run ( Uint32 ) __attribute__ ( ( warn_unused_result ) );
Uint32 cpu_68k_getpc ( void ) __attribute__ ( ( warn_unused_result ) );
void cpu_68k_interrupt ( Sint32 );
Sint32 cpu_68k_getcycle ( void ) __attribute__ ( ( warn_unused_result ) );

#endif // _GNGEOX_68K_H_
