/*!
*
*   \file    GnGeoXym2610intf.h
*   \brief   Yamaha YM2610 sound chip emulation interface header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    01/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    The YM2610 emulator supports up to 2 chips, each chip has the
*            following connections :
*            - Status Read / Control Write A
*            - Port Read / Data Write A
*            - Control Write B
*            - Data Write B
*/
#ifndef _GNGEOX_YM2610_INTERF_H_
#define _GNGEOX_YM2610_INTERF_H_

#ifdef _GNGEOX_YM2610_INTERF_C_
void timer_callback_2610 ( Sint32 );
void FMTimerInit ( void );
void neogeo_sound_irq ( Sint32 );
#endif // _GNGEOX_YM2610_INTERF_C_

void YM2610_sh_start ( void );

#endif
