/*!
*
*   \file    GnGeoXym2610.h
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
#ifndef _GNGEOX_YM2610_H_
#define _GNGEOX_YM2610_H_

#define YM2610_CLOCK_FREQ_HZ 8000000
#define YM2610_MAX_TIMERS    2

typedef struct struct_gngeoxtimer_timer struct_gngeoxtimer_timer;
struct struct_gngeoxtimer_timer
{
    double target;
};

#ifdef _GNGEOX_YM2610_C_
static void neo_ym2610_callback ( Sint32, Sint32, double );
#endif // _GNGEOX_YM2610_INTERF_C_

SDL_bool neo_ym2610_init ( void ) __attribute__ ( ( warn_unused_result ) );
double neo_ym2610_count ( void );
void neo_ym2610_update ( void );
void neo_ym2610_close ( void );

#endif
