/*!
*
*   \file    GnGeoXneocrypt.h
*   \brief   Encrypting general routines header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.40 (final beta)
*   \date    17/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    Many of the NeoGeo bootlegs use their own form of encryption and
*            protection, presumably to make them harder for other bootleggers to
*            copy.  This encryption often involves non-trivial scrambling of the
*            program roms and the games are protected using an ALTERA chip which
*            provides some kind of rom overlay, patching parts of the code.
*            The graphics roms are usually scrambled in a different way to the
*            official SNK cartridges too.
*
*            1 - NeoGeo 'C' (Graphics) Rom encryption :
*
*            CMC42 and CMC50 protection chips. Also contains 'S' (Text Layer) data on these games
*            M1 (Z80) rom is also encrypted for CMC50.
*
*            The M1 roms contain some additional data at 0xf800.  Some of this
*            is said to be related to the C rom encryption.
*            If CMC50 is used, data located at 0xff00 - 0xffff is required for
*            m1 encryption checksum?.
*
*            Later games use additional basic scrambling on top of the standard
*            CMC scramble.
*
*             Starting with KOF99, all NeoGeo games have encrypted graphics. Additionally
*             to that, the data for the front text layer, which was previously stored in
*             a separate ROM, is stored at the end of the tile data.
*
*             The encryption is one of the nastiest implementation of a XOR scheme ever
*             seen, involving 9 seemingly uncorrelated 256-byte tables. All known games use
*             the same tables except KOF2000 and MS4 which use a different set.
*
*             The 32 data bits of every longword are decrypted in a single step (one byte at
*             a time), but the values to use for the xor are determined in a convoluted way.
*             It's actually so convoluted that it's too difficult to describe - please refer
*             to the source below.
*             Suffice to say that bytes are handled in couples (0&3 and 1&2), and the two xor
*             values are taken from three tables, the indexes inside the tables depending on
*             bits 0-7 and 8-15 of the address, in one case further xored through the table
*             used in step 5) below. Additionally, the bytes in a couple can be swapped,
*             depending either on bit 8 of the address, or on bit 16 xored with the table
*             used in step 4) below.
*
*             The 24 address bits are encrypted in five steps. Each step xors 8 bits with a
*             value taken from a different table; the index inside the table depends on 8
*             other bits.
*             0) xor bits  0-7  with a fixed value that changes from game to game
*             1) xor bits  8-15 depending on bits 16-23
*             2) xor bits  8-15 depending on bits  0-7
*             3) xor bits 16-23 depending on bits  0-7
*             4) xor bits 16-23 depending on bits  8-15
*             5) xor bits  0-7  depending on bits  8-15
*
*             Each step acts on the current value, so e.g. step 4) uses bits 8-15 as modified
*             by step 2).
*
*             [Note: the table used in step 1) is currently incomplete due to lack of data to
*             analyze]
*
*             There are two major weaknesses in this encryption algorithm, that exposed it to
*             a known plain text attack.
*
*             The first weakness is that the data xor depends on the address inside the
*             encrypted ROM instead that on the decrypted address; together with the high
*             concentration of 0x00 and 0xFF in the decrypted data (more than 60% of the
*             total), this exposed easily recognizable patterns in the encrypted data, which
*             could be exploited with some simple statistical checks. The deviousness of the
*             xor scheme was the major difficulty.
*
*             The second weakness is that the address scrambling works on 32-bit words. Since
*             there are a large number of 32-bit values that appear only once in the whole
*             encrypted ROM space, this means that once the xor layer was broken, a large
*             table of encrypted-decrypted address correspondences could be built and
*             analyzed, quickly leading to the algorithm.
*
*            2 - NeoGeo 'P' Rom Encryption :
*
*            Used on various games
*
*            kof98 :
*            - unique early encryption
*            kof99, garou, garouo, mslug3, kof2000 :
*            - complex SMA chip which appears to contain part of the game rom
*              internally and decrypts the 68k code on the board. Also has a
*              random number generator and  custom bankswitching (see machine/neoprot.c)
*            kof2002, matrim, samsho5, samsh5p
*            - some basic block / bank swapping
*            svc, kof2003, mslug5 :
*            - different scrambling with additional xor
*
*            3 - NeoGeo 'V' Rom encryption :
*
*            NEO-PCM2 chip used on various games :
*            type1 used on pnyaa, rotd, mslug4
*            type2 used on kof2002, matrim, mslug5, svc, samsho5, samsh5s, kof2003
*
*/
#ifndef _GNGEOX_NEOCRYPT_H_
#define _GNGEOX_NEOCRYPT_H_

#ifdef _GNGEOX_NEOCRYPT_C_
static void decrypt ( Uint8*, Uint8*, Uint8,  Uint8, const Uint8*, const Uint8*, const Uint8*, Sint32, Sint32 );
static void neogeo_gfx_decrypt ( struct_gngeoxroms_game_roms*, Sint32 );
static void neogeo_sfix_decrypt ( struct_gngeoxroms_game_roms* );
static void load_cmc42_table ( void );
static void load_cmc50_table ( void );
static Uint16 generate_cs16 ( Uint8*, Sint32 ) __attribute__ ( ( warn_unused_result ) );
static Sint32 m1_address_scramble ( Sint32, Uint16 ) __attribute__ ( ( warn_unused_result ) );
#endif // _GNGEOX_NEOCRYPT_C_

void kof99_neogeo_gfx_decrypt ( struct_gngeoxroms_game_roms*, Sint32 );
void kof2000_neogeo_gfx_decrypt ( struct_gngeoxroms_game_roms*, Sint32 );
void cmc42_neogeo_gfx_decrypt ( struct_gngeoxroms_game_roms*, Sint32 );
void cmc50_neogeo_gfx_decrypt ( struct_gngeoxroms_game_roms*, Sint32 );
void svcpcb_gfx_decrypt ( struct_gngeoxroms_game_roms* );
void svcpcb_s1data_decrypt ( struct_gngeoxroms_game_roms* );
void kf2k3pcb_gfx_decrypt ( struct_gngeoxroms_game_roms* );
void kf2k3pcb_decrypt_s1data ( struct_gngeoxroms_game_roms* );
void neogeo_cmc50_m1_decrypt ( struct_gngeoxroms_game_roms* );
void kof98_decrypt_68k ( struct_gngeoxroms_game_roms* );
void kof99_decrypt_68k ( struct_gngeoxroms_game_roms* );
void garou_decrypt_68k ( struct_gngeoxroms_game_roms* );
void garouo_decrypt_68k ( struct_gngeoxroms_game_roms* );
void mslug3_decrypt_68k ( struct_gngeoxroms_game_roms* );
void kof2000_decrypt_68k ( struct_gngeoxroms_game_roms* );
void kof2002_decrypt_68k ( struct_gngeoxroms_game_roms* );
void matrim_decrypt_68k ( struct_gngeoxroms_game_roms* );
void samsho5_decrypt_68k ( struct_gngeoxroms_game_roms* );
void samsh5sp_decrypt_68k ( struct_gngeoxroms_game_roms* );
void mslug5_decrypt_68k ( struct_gngeoxroms_game_roms* );
void svc_px_decrypt ( struct_gngeoxroms_game_roms* );
void kf2k3pcb_decrypt_68k ( struct_gngeoxroms_game_roms* );
void kof2003_decrypt_68k ( struct_gngeoxroms_game_roms* );
void kof2003h_decrypt_68k ( struct_gngeoxroms_game_roms* );
void neo_pcm2_snk_1999 ( struct_gngeoxroms_game_roms*, Sint32 );
void neo_pcm2_swap ( struct_gngeoxroms_game_roms*, Sint32 );
void kof2003biosdecode ( struct_gngeoxroms_game_roms* );
void samsh5p_decrypt_68k ( struct_gngeoxroms_game_roms* );

#endif
