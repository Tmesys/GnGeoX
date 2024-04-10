/*!
*
*   \file    GnGeoXinterp.H
*   \brief   Interpolation image effect routines header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
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

Uint16 interp_16_521 ( Uint16, Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Uint16 interp_16_332 ( Uint16, Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Uint16 interp_16_611 ( Uint16, Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Uint16 interp_16_71 ( Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Uint16 interp_16_211 ( Uint16, Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Uint16 interp_16_772 ( Uint16, Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Uint16 interp_16_11 ( Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Uint16 interp_16_31 ( Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Uint16 interp_16_1411 ( Uint16, Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Uint16 interp_16_431 ( Uint16, Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Uint16 interp_16_53 ( Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Uint16 interp_16_151 ( Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Uint16 interp_16_97 ( Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );
Sint32 interp_16_diff ( Uint16, Uint16 ) __attribute__ ( ( warn_unused_result ) );

#endif // _GNGEOX_INTERP_H_
