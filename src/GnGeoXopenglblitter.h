/*!
*
*   \file    GnGeoXopenglblitter.h
*   \brief   Opengl blitter routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    03/10/2023
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_OPENGLBLITTER_H_
#define _GNGEOX_OPENGLBLITTER_H_

#define RGB24_PIXELS 1
#define PIXEL_TYPE GL_BGRA
#define PIXEL_SIZE GL_UNSIGNED_BYTE

SDL_bool blitter_opengl_init ( void ) __attribute__ ( ( warn_unused_result ) );
SDL_bool blitter_opengl_resize ( Sint32, Sint32 ) __attribute__ ( ( warn_unused_result ) );
void blitter_opengl_update ( void );
void blitter_opengl_close ( void );
void blitter_opengl_fullscreen ( void );

#endif
