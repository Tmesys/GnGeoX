/*!
*
*   \file    GnGeoXprofiler.h
*   \brief   Profiler routines header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    18/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_PROFILER_H_
#define _GNGEOX_PROFILER_H_

typedef enum
{
    PROF_ALL = 0,
    PROF_VIDEO = 1,
    PROF_68K = 2,
    PROF_Z80 = 3,
    PROF_SDLBLIT = 4,
    PROF_SOUND = 5,
    MAX_BLOCK = 6
} enum_gngeoxroms_profiler_type;

void profiler_start ( enum_gngeoxroms_profiler_type );
void profiler_stop ( enum_gngeoxroms_profiler_type );
void profiler_show_stat ( void );

#endif
