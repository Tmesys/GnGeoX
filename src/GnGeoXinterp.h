/*!
*
*   \file    GnGeoXinterp.H
*   \brief   Interpolation image effect routines header.
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
#ifndef _GNGEOX_INTERP_H_
#define _GNGEOX_INTERP_H_

#define INTERP_16_MASK_1(v) ((v) & 0xf81F)
#define INTERP_16_MASK_2(v) ((v) & 0x7E0)
#define INTERP_16_UNMASK_1(v) ((v) & 0xf81F)
#define INTERP_16_UNMASK_2(v) ((v) & 0x7E0)

#define INTERP_Y_LIMIT (0x30 * 4)
#define INTERP_U_LIMIT (0x07 * 4)
#define INTERP_V_LIMIT (0x06 * 8)

/* diff */
#define INTERP_Y_LIMIT 0x30
#define INTERP_U_LIMIT 0x07
#define INTERP_V_LIMIT 0x06
/* Multipled version */
#define INTERP_Y_LIMIT_S2 (INTERP_Y_LIMIT << 2)
#define INTERP_U_LIMIT_S2 (INTERP_U_LIMIT << 2)
#define INTERP_V_LIMIT_S3 (INTERP_V_LIMIT << 3)

#define INTERP_32_MASK_1(v) ((v) & 0xFF00FFU)
#define INTERP_32_MASK_2(v) ((v) & 0x00FF00U)
#define INTERP_32_UNMASK_1(v) ((v) & 0xFF00FFU)
#define INTERP_32_UNMASK_2(v) ((v) & 0x00FF00U)
#define INTERP_32_HNMASK (~0x808080U)

Uint32 interp_32_1411 ( Uint32 p1, Uint32 p2, Uint32 p3 );
Uint32 interp_32_11 ( Uint32 p1, Uint32 p2 );
Uint32 interp_32_611 ( Uint32 p1, Uint32 p2, Uint32 p3 );
Uint32 interp_32_332 ( Uint32 p1, Uint32 p2, Uint32 p3 );
Uint32 interp_32_521 ( Uint32 p1, Uint32 p2, Uint32 p3 );
Uint32 interp_32_211 ( Uint32 p1, Uint32 p2, Uint32 p3 );
Uint32 interp_32_31 ( Uint32 p1, Uint32 p2 );
Uint32 interp_32_71 ( Uint32 p1, Uint32 p2 );
Uint32 interp_32_diff ( Uint32, Uint32 ) __attribute__ ( ( warn_unused_result ) );
Uint32 interp_32_dist ( Uint32 p1, Uint32 p2 ) __attribute__ ( ( warn_unused_result ) );
Uint32 interp_32_dist3(Uint32 p1, Uint32 p2, Uint32 p3);

#endif // _GNGEOX_INTERP_H_
