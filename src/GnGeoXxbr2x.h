/*!
*
*   \file    GnGeoXhq3x.h
*   \brief   HQ3X image effect routines header https://en.wikipedia.org//wiki/Hqx.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    03/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    This effect is a rewritten implementation of the hq3x effect made by Maxim Stepin.
*/

#ifndef _GNGEOX_XBR2X_H_
#define _GNGEOX_XBR2X_H_

#define XBR(type, PE, PI, PH, PF, PG, PC, PD, PB, PA, G5, C4, G0, D0, C1, B1, F4, I4, H5, I5, A0, A1, N0, N1, N2, N3) \
	if (PE != PH && PE != PF) { \
		unsigned e = df3(PC, PE, PG) + df3(H5, PI, F4) + 4 * df(PH, PF); \
		unsigned i = df3(PD, PH, I5) + df3(I4, PF, PB) + 4 * df(PE, PI); \
		if (e < i) { \
			int ex2 = PE != PC && PB != PC; \
			int ex3 = PE != PG && PD != PG; \
			unsigned ke = df(PF, PG); \
			unsigned ki = df(PH, PC); \
			type px = df(PE, PF) <= df(PE, PH) ? PF : PH; \
			if (ke == 0 && ki == 0 && ex3 && ex2) { \
				LEFT_UP_2_2X(N3, N2, N1, px); \
			} else if (2 * ke <= ki && ex3) { \
				LEFT_2_2X(N3, N2, px); \
			} else if (ke >= 2 * ki && ex2) { \
				UP_2_2X(N3, N1, px); \
			} else { \
				DIA_2X(N3, px); \
			} \
		} \
	}

#define LEFT_UP_2_2X(N3, N2, N1, PIXEL) \
	E[N3] = interp_32_71(PIXEL, E[N3]); \
	E[N1] = E[N2] = interp_32_31(E[N2], PIXEL);

#define LEFT_2_2X(N3, N2, PIXEL) \
	E[N3] = interp_32_31(PIXEL, E[N3]); \
	E[N2] = interp_32_31(E[N2], PIXEL);

#define UP_2_2X(N3, N1, PIXEL) \
	E[N3] = interp_32_31(PIXEL, E[N3]); \
	E[N1] = interp_32_31(E[N1], PIXEL);

#define DIA_2X(N3, PIXEL) \
	E[N3] = interp_32_11(E[N3], PIXEL);


#define df(A, B) interp_32_dist(A, B)
#define df3(A, B, C) interp_32_dist3(A, B, C)

#ifdef _GNGEOX_XBR2X_C_
static void hq3x_16_def ( Uint16*, Uint16*, Uint16*, Uint16*, Uint16*, Uint16*, Uint32 );
#endif // _GNGEOX_XBR2X_C_

SDL_bool effect_xbr2x_init ( void ) __attribute__ ( ( warn_unused_result ) );
void effect_xbr2x_update ( void );

#endif // _GNGEOX_HQ3X_H_
