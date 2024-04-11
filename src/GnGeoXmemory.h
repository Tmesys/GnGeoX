/*!
*
*   \file    GnGeoXmemory.h
*   \brief   Memory routines header.
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
#ifndef _GNGEOX_MEMORY_H_
#define _GNGEOX_MEMORY_H_

#define READ_WORD(a)          (*(Uint16 *)(a))
#define WRITE_WORD(a,d)       (*(Uint16 *)(a) = (d))
#define READ_BYTE(a)          (*(Uint8 *)(a))
#define WRITE_BYTE(a,d)       (*(Uint8 *)(a) = (d))
/* @note (Tmesys#1#12/07/2022): Not used, watch out was long. */
#define SWAP_BYTE_ADDRESS(a)  ((Sint64)(a)^1)
#define SWAP16(y) SDL_Swap16(y)
#define SWAP32(y) SDL_Swap32(y)

/* Programs are stored as BIGENDIAN */
#  if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#    define WRITE_WORD_ROM WRITE_WORD
#    define READ_WORD_ROM READ_WORD
#    define WRITE_BYTE_ROM WRITE_BYTE
#    define READ_BYTE_ROM READ_BYTE
#  else
#    define WRITE_WORD_ROM(a,d) (WRITE_WORD(a,SWAP16(d)))
#    define READ_WORD_ROM(a) (SWAP16(READ_WORD(a)))
#    define WRITE_BYTE_ROM WRITE_BYTE
#    define READ_BYTE_ROM READ_BYTE
#  endif

typedef struct
{
    struct_gngeoxroms_game_roms rom;
    struct_gngeoxvideo_video vid;
    Uint8 ram[0x10000];
    Uint8 sram[0x10000];
    Uint8 memcard[2048];
    Uint8* fix_board_usage;
    Uint8* fix_game_usage;
    Uint8 game_vector[0x80];
    Sint32 current_vector;
    /* Put it in memory.vid? use zoom table in rom */
    Uint8* ng_lo;
    Uint32 nb_of_tiles;
    /* internal representation of inputs */
    Uint8 p1cnt, p2cnt, status_a, status_b;
    Uint8 test_switch;
    /* Sound control */
    Uint8 z80_command;
    Uint8 z80_command_reply;
    /* crypted rom bankswitch system */
    Uint32 bksw_handler;
    Uint8* bksw_unscramble;
    Sint32* bksw_offset;
    Uint16 sma_rng_addr;
    Uint32 watchdog;
} struct_gngeoxmemory_neogeo;

#ifndef _GNGEOX_MEMORY_C_
extern struct_gngeoxmemory_neogeo neogeo_memory;
/* video related */
extern Uint8* current_pal;
extern Uint32* current_pc_pal;
extern Uint8* current_fix;
extern Uint8* fix_usage;
/* sram */
extern Uint8 sram_lock;
/* 68k cpu Banking control */
/* current bank */
extern Uint32 bankaddress;
#endif // _GNGEOX_MEMORY_C_

/* memory handler prototype */
void write_neo_control ( Uint16 );
void write_irq2pos ( Uint32 );
Uint32 convert_pal ( Uint16 ) __attribute__ ( ( warn_unused_result ) );
void update_all_pal ( void );
Uint16 sma_random ( void ) __attribute__ ( ( warn_unused_result ) );
void dump_hardware_reg ( void );
void switch_bank ( Uint32, Uint8 );

extern Uint8 ( *mem68k_fetch_bksw_byte ) ( Uint32 );
extern Uint16 ( *mem68k_fetch_bksw_word ) ( Uint32 );
extern Uint32 ( *mem68k_fetch_bksw_long ) ( Uint32 );
extern void ( *mem68k_store_bksw_byte ) ( Uint32, Uint8 );
extern void ( *mem68k_store_bksw_word ) ( Uint32, Uint16 );
extern void ( *mem68k_store_bksw_long ) ( Uint32, Uint32 );

#endif // _GNGEOX_MEMORY_H_
