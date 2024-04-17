/*!
*
*   \file    GnGeoXz80.c
*   \brief   Interface to the mamez80 emulator.
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
#ifndef _GNGEOX_Z80_C_
#define _GNGEOX_Z80_C_
#endif // _GNGEOX_Z80_C_

#include <SDL2/SDL.h>
#include "zlog.h"
#include "qlibc.h"

#include "3rdParty/Z80/Z80.h"
#include "GnGeoXz80.h"
#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXym2610.h"
#include "GnGeoXym2610core.h"
#include "GnGeoXconfig.h"

#define GNGEOX_Z80_DEBUG

static Uint8* z80map0 = NULL, *z80map1 = NULL, *z80map2 = NULL, *z80map3 = NULL;
/* The NMI is disabled immediately after the system is reset. */
static SDL_bool enable_nmi = SDL_FALSE;
Uint8 mame_z80mem[0x10000];

/* ******************************************************************************************************************/
/*!
* \brief Switches Z80 memory banks.
*
* \param bank Memory bank to switch to.
* \param port Z80 port ?.
**/
/* ******************************************************************************************************************/
static void cpu_z80_switchbank ( Uint8 bank, Uint16 port )
{
    Uint16 window_address_mult = QHIBYTE ( port );
    Uint32 new_start_offset = 0;

    switch ( bank )
    {
    case ( 0 ) :
        {
            /* @note (Tmesys#1#10/04/2024): Why ? */
            window_address_mult &= 0x7f;
            new_start_offset = ( Z80_BANK_0_WINDOW_SIZE * window_address_mult );

            if ( new_start_offset < neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].size )
            {
                z80map0 = neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p + new_start_offset;
                memcpy ( mame_z80mem + Z80_BANK_0_OFFSET, z80map0, Z80_BANK_0_WINDOW_SIZE );
            }
            else
            {
                zlog_error ( gngeox_config.loggingCat, "Bank address out of range %p / size %d"
                             , neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p + new_start_offset
                             , neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].size );
            }
        }
        break;
    case ( 1 ) :
        {
            window_address_mult &= 0x3f;
            new_start_offset = ( Z80_BANK_1_WINDOW_SIZE * window_address_mult );

            if ( new_start_offset < neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].size )
            {
                z80map1 = neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p + new_start_offset;
                memcpy ( mame_z80mem + Z80_BANK_1_OFFSET, z80map1, Z80_BANK_1_WINDOW_SIZE );
            }
            else
            {
                zlog_error ( gngeox_config.loggingCat, "Bank address out of range %p / size %d"
                             , neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p + new_start_offset
                             , neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].size );
            }
        }
        break;
    case ( 2 ) :
        {
            window_address_mult &= 0x1f;
            new_start_offset = ( Z80_BANK_2_WINDOW_SIZE * window_address_mult );

            if ( new_start_offset < neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].size )
            {
                z80map2 = neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p + new_start_offset;
                memcpy ( mame_z80mem + Z80_BANK_2_OFFSET, z80map2, Z80_BANK_2_WINDOW_SIZE );
            }
            else
            {
                zlog_error ( gngeox_config.loggingCat, "Bank address out of range %p / size %d"
                             , neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p + new_start_offset
                             , neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].size );
            }
        }
        break;
    case ( 3 ) :
        {
            window_address_mult &= 0x0F;
            new_start_offset = ( Z80_BANK_3_WINDOW_SIZE * window_address_mult );

            if ( new_start_offset < neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].size )
            {
                z80map3 = neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p + new_start_offset;
                memcpy ( mame_z80mem + Z80_BANK_3_OFFSET, z80map3, Z80_BANK_3_WINDOW_SIZE );
            }
            else
            {
                zlog_error ( gngeox_config.loggingCat, "Bank address out of range %p / size %d"
                             , neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p + ( 0x4000 * window_address_mult )
                             , neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].size );
            }
        }
        break;
    default:
        {
            zlog_error ( gngeox_config.loggingCat, "Invalid bank : %d", bank );
        }
        break;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Z80 Irq callback : Fired when IRQ changes state.
*
* \param irq_line Concerned IRQ line.
* \return Corresponding IRQ vector.
**/
/* ******************************************************************************************************************/
static Sint32 neo_z80_irq_callback ( Sint32 irq_line )
{
    return ( 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Initializes Z80 cpu.
* \note Having a 16-bit address bus, the Z80 CPU allows access to 64KiB of memory at most.
*       In NeoGeo cartridges, the NEO-ZMC or NEO-ZMC2 chips provide a way to access more
*       memory for M1 ROMs up to 4MiB. Four different switchable zones ("windows") are
*       available.
*       The bank that appear in these windows are configured by reading from 4 of the Z80's
*       ports (see page for explanation).
*       Bank offsets numbering depend on the window sizes:
*       Window 0 counts in 2KiB increments
*       Window 1 in 4KiB
*       Window 2 in 8KiB
*       Window 3 in 16KiB
*       On cartridge systems, all windows are initialized to 0 (verified on hardware).
*       Many sound drivers start up by initializing the windows properly like indicated above.
*       Start	End	    Size	Description
*       -------------------------------------------------------------------
*       $0000	$7FFF	32KiB	Static main code bank (start of the M1 ROM)
*       $8000	$BFFF	16KiB	Switching window 3
*       $C000	$DFFF	8KiB	Switching window 2
*       $E000	$EFFF	4KiB	Switching window 1
*       $F000	$F7FF	2KiB	Switching window 0
*       $F800	$FFFF	2KiB	Work RAM
*/
/* ******************************************************************************************************************/
void neo_z80_init ( void )
{
    z80_init ( neo_z80_irq_callback );

    /* memory bank initialization */
    z80map0 = neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p + Z80_BANK_0_OFFSET;
    z80map1 = neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p + Z80_BANK_1_OFFSET;
    z80map2 = neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p + Z80_BANK_2_OFFSET;
    z80map3 = neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p + Z80_BANK_3_OFFSET;

    SDL_zero ( mame_z80mem );
    memcpy ( mame_z80mem, neogeo_memory.rom.rom_region[REGION_AUDIO_CPU_CARTRIDGE].p, 0xF800 );
}
/* ******************************************************************************************************************/
/*!
* \brief Writes 16 bits value to designated port.
*
* \param port Z80 port to write to.
* \param value 16 bits value to write.
* \note SNK used an obscure feature of the Z80: when accessing ports, the top address bus byte
*       is set to register C.
*       Address	Write
*       $00	    Clear sound code from 68k to $00.
*       $04-$07	YM2610 I/O
*       $08	    --
*       $09      |
*       $0A       > Enable NMIs
*       $0B     --
*       $0C	    Reply to 68k
*       $18	    Disable NMIs
*       Pair A (Z80 ports 4 and 5) concerns the SSG, ADPCM-B, and FM channels 1 and 2.
*       Pair B (Z80 ports 6 and 7) concerns the ADPCM-A, and FM channels 3 and 4.
**/
/* ******************************************************************************************************************/
void z80_writeport16 ( Uint16 port, Uint8 value )
{
    switch ( QLOBYTE ( port ) )
    {
    case ( 0x0 ) :
        {
            neogeo_memory.z80_command = 0;
        }
        break;
    /* Address A */
    case ( 0x4 ) :
        {
            YM2610Write ( 0, value );
        }
        break;
    /* Data A */
    /* IRQs are acknowledged by clearing reset flags in mode register */
    case ( 0x5 ) :
        {
            YM2610Write ( 1, value );
        }
        break;
    /* Address B */
    case ( 0x6 ) :
        {
            YM2610Write ( 2, value );
        }
        break;
    /* Data B */
    case ( 0x7 ) :
        {
            YM2610Write ( 3, value );
        }
        break;
    case ( 0x08 ) :
    case ( 0x09 ) :
    case ( 0x0a ) :
    case ( 0x0b ) :
        {
            enable_nmi = SDL_TRUE;
        }
        break;
    case ( 0xC ) :
        {
            neogeo_memory.z80_command_reply = value;
        }
        break;
    case ( 0x18 ) :
        {
            enable_nmi = SDL_FALSE;
        }
        break;
    default:
        {
            zlog_error ( gngeox_config.loggingCat, "Unknown port %x, value %x", QLOBYTE ( port ), value );
        }
        break;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Reads 16 bits value from designated port.
*
* \param port Z80 port to read from.
* \return 16 bits value read.
* \note M1 ROM bankswitching is done by reading ports.
*       SNK used an obscure feature of the Z80: when accessing ports, the top address bus byte
*       is set to register C.
*       The banks can be chosen by putting the bank number in C, and doing IN A,(port).
*       This is handled by NEO-ZMC in cartridges.
*       Address	Read
*       $00	    Read sound code from 68k / Acknowledge NMI
*       $04-$07	YM2610 I/O
*       $08	    Set window 0 bank
*       $09	    Set window 1 bank
*       $0A	    Set window 2 bank
*       $0B	    Set window 3 bank
*       $0C	    See SDRD1
*       $18	    See address $08
*       The only writable registers that can be read back are the SSG ones.
*       To read register X, write X to Z80 port 4, then read Z80 port 5 (needs clarification).
**/
/* ******************************************************************************************************************/
Uint8 z80_readport16 ( Uint16 port )
{
    Uint8 return_value = 0;

    switch ( QLOBYTE ( port ) )
    {
    /* NMI are acknowledged by reading port $00 */
    case ( 0x0 ) :
        {
            return_value = neogeo_memory.z80_command;
            /* @note (Tmesys#1#17/04/2024): Disabling NMI do not disables acknowledge (ninjamas) ! */
            z80_set_nmi_line ( CLEAR_LINE );
        }
        break;
    case ( 0x4 ) :
        {
            return_value = YM2610Read ( 0 );
        }
        break;
    case ( 0x5 ) :
        {
            return_value = YM2610Read ( 1 );
        }
        break;
    case ( 0x6 ) :
        {
            return_value = YM2610Read ( 2 );
        }
        break;
    case ( 0x08 ) :
        {
            cpu_z80_switchbank ( 0, port );
        }
        break;
    case ( 0x09 ) :
        {
            cpu_z80_switchbank ( 1, port );
        }
        break;
    case ( 0x0a ) :
        {
            cpu_z80_switchbank ( 2, port );
        }
        break;
    case ( 0x0b ) :
        {
            cpu_z80_switchbank ( 3, port );
        }
        break;
    /* @note (Tmesys#1#16/04/2024): This is some undocumented port used by ninjamas
    sound driver :
    First write to port 0xf some value 0x89
    Then read port 0xe, and with 0xc0 (keep two upper bits),
    return from routine if result is non zero. */
    /*
        case ( 0x0e ) :
            {
                return_value = 0xc0;
            }
            break;
    */
    default:
        {
            zlog_error ( gngeox_config.loggingCat, "Unknown port %x", QLOBYTE ( port ) );
        }
        break;
    };

    return ( return_value );
}
/* ******************************************************************************************************************/
/*!
* \brief  Z80 IRQs can be generated by the YM2610's timers, and are mostly used to time
*         music playback.
*         The vector address is $0038. They can be disabled in software (DI/EI) and must be
*         acknowledged via the YM2610.
*
* \param  irq IRQ flag.
*/
/* ******************************************************************************************************************/
void neo_z80_irq ( Sint32 irq )
{
    /* IRQ is OFF to ON */
    if ( irq )
    {
        z80_set_irq_line ( 0, ASSERT_LINE );
    }
    else
    {
        z80_set_irq_line ( 0, CLEAR_LINE );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Non Maskable Interrupts are triggered when the 68k sends a sound command.
*        The vector address is $0066.
*        They can be enabled or disabled through ports $08 and $18. They are disabled after
*        a reset.
*        They are acknowledged by reading port $00.
*
*/
/* ******************************************************************************************************************/
void neo_z80_nmi ( void )
{
    if ( enable_nmi == SDL_TRUE )
    {
        z80_set_nmi_line ( ASSERT_LINE );
    }
}

#ifdef _GNGEOX_Z80_C_
#undef _GNGEOX_Z80_C_
#endif // _GNGEOX_Z80_C_
