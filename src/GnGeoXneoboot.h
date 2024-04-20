/*!
*
*   \file    GnGeoXneoboot.h
*   \brief   General Bootleg Functions, used by more than 1 game.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.40 (final beta)
*   \date    04/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    Many of the NeoGeo bootlegs use their own form of encryption and
*            protection, presumably to make them harder for other bootleggser to
*            copy.  This encryption often involves non-trivial scrambling of the
*            program roms and the games are protected using an Altera chip which
*            provides some kind of rom overlay, patching parts of the code.
*            The graphics roms are usually scrambled in a different way to the
*            official SNK cartridges too.
*/
#ifndef _GNGEOX_NEOBOOT_H_
#define _GNGEOX_NEOBOOT_H_

#define MATRIMBLZ80( i ) ( i^(BITSWAP8(i&0x3,4,3,1,2,0,7,6,5)<<8) )

#ifdef _GNGEOX_NEOBOOT_C_
static void kf2k5uni_px_decrypt ( struct_gngeoxroms_game_roms* );
static void kf2k5uni_sx_decrypt ( struct_gngeoxroms_game_roms* );
static void kf2k5uni_mx_decrypt ( struct_gngeoxroms_game_roms* );
static void cthd2003_neogeo_gfx_address_fix_do ( struct_gngeoxroms_game_roms*, Sint32, Sint32, Sint32, Sint32, Sint32, Sint32 );
static void cthd2003_c ( struct_gngeoxroms_game_roms* );
static void ct2k3sp_sx_decrypt ( struct_gngeoxroms_game_roms* );
#endif // _GNGEOX_NEOBOOT_C_

void neogeo_bootleg_cx_decrypt ( struct_gngeoxroms_game_roms* );
void neogeo_bootleg_sx_decrypt ( struct_gngeoxroms_game_roms*, Sint32 );
void kog_px_decrypt ( struct_gngeoxroms_game_roms* );
void decrypt_kof10th ( struct_gngeoxroms_game_roms* );
void decrypt_kf10thep ( struct_gngeoxroms_game_roms* );
void decrypt_kf2k5uni ( struct_gngeoxroms_game_roms* );
void kof2002b_gfx_decrypt ( struct_gngeoxroms_game_roms*, Uint8*, Sint32 );
/* @fixme (Tmesys#1#12/10/2022): Commented. */
void kf2k2mp_decrypt ( struct_gngeoxroms_game_roms* );
/* @fixme (Tmesys#1#12/10/2022): Not used. */
void kf2k2mp2_px_decrypt ( struct_gngeoxroms_game_roms* );
void decrypt_cthd2003 ( struct_gngeoxroms_game_roms* );
void decrypt_ct2k3sp ( struct_gngeoxroms_game_roms* );
void decrypt_ct2k3sa ( struct_gngeoxroms_game_roms* );
void patch_ct2k3sa ( struct_gngeoxroms_game_roms* );
void decrypt_kof2k4se_68k ( struct_gngeoxroms_game_roms* );
void lans2004_vx_decrypt ( struct_gngeoxroms_game_roms* );
void lans2004_decrypt_68k ( struct_gngeoxroms_game_roms* );
void svcboot_px_decrypt ( struct_gngeoxroms_game_roms* );
void svcboot_cx_decrypt ( struct_gngeoxroms_game_roms* );
void svcplus_px_decrypt ( struct_gngeoxroms_game_roms* );
void svcplus_px_hack ( struct_gngeoxroms_game_roms* );
void svcplusa_px_decrypt ( struct_gngeoxroms_game_roms* );
void svcsplus_px_decrypt ( struct_gngeoxroms_game_roms* );
void svcsplus_px_hack ( struct_gngeoxroms_game_roms* );
void kf2k3pl_px_decrypt ( struct_gngeoxroms_game_roms* );
void kf2k3upl_px_decrypt ( struct_gngeoxroms_game_roms* );
void samsho5b_px_decrypt ( struct_gngeoxroms_game_roms* );
void samsho5b_vx_decrypt ( struct_gngeoxroms_game_roms* );
void matrimbl_decrypt ( struct_gngeoxroms_game_roms* );

#endif // _GNGEOX_NEOBOOT_H_
