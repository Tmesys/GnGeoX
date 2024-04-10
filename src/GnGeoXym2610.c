/*!
*
*   \file    GnGeoXym2610.c
*   \brief   YAMAHA YM-2610 sound generator emulation routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/HQ3X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.40 (final beta)
*   \date    04/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_YM2610_C_
#define _GNGEOX_YM2610_C_
#endif // _GNGEOX_YM2610_C_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <zlog.h>

#include "GnGeoXtimer.h"
#include "GnGeoXym2610intf.h"
#include "GnGeoXym2610.h"

/*  TL_TAB_LEN is calculated as:
 *   13 - sinus amplitude bits     (Y axis)
 *   2  - sinus sign bit           (Y axis)
 *   TL_RES_LEN - sinus resolution (X axis)
 */
static Sint32 tl_tab[TL_TAB_LEN];

/* sin waveform table in 'decibel' scale */
static Uint32 sin_tab[SIN_LEN];

/* sustain level table (3dB per step) */
/* bit0, bit1, bit2, bit3, bit4, bit5, bit6 */
/* 1,    2,    4,    8,    16,   32,   64   (value)*/
/* 0.75, 1.5,  3,    6,    12,   24,   48   (dB)*/

/* 0 - 15: 0, 3, 6, 9,12,15,18,21,24,27,30,33,36,39,42,93 (dB)*/
#define SC(db) (Uint32) ( db * (4.0/ENV_STEP) )
static const Uint32 sl_table[16] = { SC ( 0 ), SC ( 1 ), SC ( 2 ), SC ( 3 ),
                                     SC ( 4 ), SC ( 5 ), SC ( 6 ), SC ( 7 ), SC ( 8 ), SC ( 9 ), SC ( 10 ), SC ( 11 ),
                                     SC ( 12 ), SC ( 13 ), SC ( 14 ), SC ( 31 )
                                   };
#undef SC

static const Uint8 eg_inc[19 * RATE_STEPS] =
{

    /*cycle:0 1  2 3  4 5  6 7*/

    /* 0 */0, 1, 0, 1, 0, 1, 0, 1, /* rates 00..11 0 (increment by 0 or 1) */
    /* 1 */0, 1, 0, 1, 1, 1, 0, 1, /* rates 00..11 1 */
    /* 2 */0, 1, 1, 1, 0, 1, 1, 1, /* rates 00..11 2 */
    /* 3 */0, 1, 1, 1, 1, 1, 1, 1, /* rates 00..11 3 */

    /* 4 */1, 1, 1, 1, 1, 1, 1, 1, /* rate 12 0 (increment by 1) */
    /* 5 */1, 1, 1, 2, 1, 1, 1, 2, /* rate 12 1 */
    /* 6 */1, 2, 1, 2, 1, 2, 1, 2, /* rate 12 2 */
    /* 7 */1, 2, 2, 2, 1, 2, 2, 2, /* rate 12 3 */

    /* 8 */2, 2, 2, 2, 2, 2, 2, 2, /* rate 13 0 (increment by 2) */
    /* 9 */2, 2, 2, 4, 2, 2, 2, 4, /* rate 13 1 */
    /*10 */2, 4, 2, 4, 2, 4, 2, 4, /* rate 13 2 */
    /*11 */2, 4, 4, 4, 2, 4, 4, 4, /* rate 13 3 */

    /*12 */4, 4, 4, 4, 4, 4, 4, 4, /* rate 14 0 (increment by 4) */
    /*13 */4, 4, 4, 8, 4, 4, 4, 8, /* rate 14 1 */
    /*14 */4, 8, 4, 8, 4, 8, 4, 8, /* rate 14 2 */
    /*15 */4, 8, 8, 8, 4, 8, 8, 8, /* rate 14 3 */

    /*16 */8, 8, 8, 8, 8, 8, 8, 8, /* rates 15 0, 15 1, 15 2, 15 3 (increment by 8) */
    /*17 */16, 16, 16, 16, 16, 16, 16, 16, /* rates 15 2, 15 3 for attack */
    /*18 */0, 0, 0, 0, 0, 0, 0, 0, /* infinity rates for attack and decay(s) */
};

/* @note (Tmesys#1#12/04/2022): that there is no O(17) in this table - it's directly in the code. */
#define O(a) (a*RATE_STEPS)
static const Uint8 eg_rate_select[32 + 64 + 32] =   /* Envelope Generator rates (32 + 64 rates + 32 RKS) */
{
    /* 32 infinite time rates */
    O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ),
    O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ),
    O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ),
    O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ), O ( 18 ),

    /* rates 00-11 */
    O ( 0 ), O ( 1 ), O ( 2 ), O ( 3 ), O ( 0 ), O ( 1 ), O ( 2 ), O ( 3 ),
    O ( 0 ), O ( 1 ), O ( 2 ), O ( 3 ), O ( 0 ), O ( 1 ), O ( 2 ), O ( 3 ), O ( 0 ),
    O ( 1 ), O ( 2 ), O ( 3 ), O ( 0 ), O ( 1 ), O ( 2 ), O ( 3 ), O ( 0 ), O ( 1 ),
    O ( 2 ), O ( 3 ), O ( 0 ), O ( 1 ), O ( 2 ), O ( 3 ), O ( 0 ), O ( 1 ), O ( 2 ),
    O ( 3 ), O ( 0 ), O ( 1 ), O ( 2 ), O ( 3 ), O ( 0 ), O ( 1 ), O ( 2 ), O ( 3 ),
    O ( 0 ), O ( 1 ), O ( 2 ), O ( 3 ),

    /* rate 12 */
    O ( 4 ), O ( 5 ), O ( 6 ), O ( 7 ),

    /* rate 13 */
    O ( 8 ), O ( 9 ), O ( 10 ), O ( 11 ),

    /* rate 14 */
    O ( 12 ), O ( 13 ), O ( 14 ), O ( 15 ),

    /* rate 15 */
    O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ),

    /* 32 dummy rates (same as 15 3) */
    O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ),
    O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ),
    O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ),
    O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 ), O ( 16 )

};
#undef O

/*rate  0,    1,    2,   3,   4,   5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15*/
/*shift 11,   10,   9,   8,   7,   6,  5,  4,  3,  2, 1,  0,  0,  0,  0,  0 */
/*mask  2047, 1023, 511, 255, 127, 63, 31, 15, 7,  3, 1,  0,  0,  0,  0,  0 */

#define O(a) (a*1)
static const Uint8 eg_rate_shift[32 + 64 + 32] =    /* Envelope Generator counter shifts (32 + 64 rates + 32 RKS) */
{
    /* 32 infinite time rates */O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ),
    O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ),
    O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ),
    O ( 0 ), O ( 0 ),

    /* rates 00-11 */O ( 11 ), O ( 11 ), O ( 11 ), O ( 11 ), O ( 10 ), O ( 10 ), O ( 10 ), O ( 10 ),
    O ( 9 ), O ( 9 ), O ( 9 ), O ( 9 ), O ( 8 ), O ( 8 ), O ( 8 ), O ( 8 ), O ( 7 ),
    O ( 7 ), O ( 7 ), O ( 7 ), O ( 6 ), O ( 6 ), O ( 6 ), O ( 6 ), O ( 5 ), O ( 5 ),
    O ( 5 ), O ( 5 ), O ( 4 ), O ( 4 ), O ( 4 ), O ( 4 ), O ( 3 ), O ( 3 ), O ( 3 ),
    O ( 3 ), O ( 2 ), O ( 2 ), O ( 2 ), O ( 2 ), O ( 1 ), O ( 1 ), O ( 1 ), O ( 1 ),
    O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ),

    /* rate 12 */O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ),

    /* rate 13 */O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ),

    /* rate 14 */O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ),

    /* rate 15 */O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ),

    /* 32 dummy rates (same as 15 3) */O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ),
    O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ),
    O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ),
    O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 ), O ( 0 )

};
#undef O

static const Uint8 dt_tab[4 * 32] =
{
    /* this is YM2151 and YM2612 phase increment data (in 10.10 fixed point format)*/
    /* FD=0 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    /* FD=1 */
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5,
    5, 6, 6, 7, 8, 8, 8, 8,
    /* FD=2 */
    1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 8, 8, 9, 10,
    11, 12, 13, 14, 16, 16, 16, 16,
    /* FD=3 */
    2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 8, 8, 9, 10, 11, 12, 13,
    14, 16, 17, 19, 20, 22, 22, 22, 22
};

/* OPN key frequency number -> key code follow table */
/* fnum higher 4bit -> keycode lower 2bit */
static const Uint8 opn_fktable[16] = { 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 3,
                                       3, 3, 3, 3, 3
                                     };

/* 8 LFO speed parameters */
/* each value represents number of samples that one LFO level will last for */
static const Uint32 lfo_samples_per_step[8] = { 108, 77, 71, 67, 62, 44,
                                                8, 5
                                              };

/*There are 4 different LFO AM depths available, they are:
 0 dB, 1.4 dB, 5.9 dB, 11.8 dB
 Here is how it is generated (in EG steps):

 11.8 dB = 0, 2, 4, 6, 8, 10,12,14,16...126,126,124,122,120,118,....4,2,0
 5.9 dB = 0, 1, 2, 3, 4, 5, 6, 7, 8....63, 63, 62, 61, 60, 59,.....2,1,0
 1.4 dB = 0, 0, 0, 0, 1, 1, 1, 1, 2,...15, 15, 15, 15, 14, 14,.....0,0,0

 (1.4 dB is loosing precision as you can see)

 It's implemented as generator from 0..126 with step 2 then a shift
 right N times, where N is:
 8 for 0 dB
 3 for 1.4 dB
 1 for 5.9 dB
 0 for 11.8 dB
 */
static const Uint8 lfo_ams_depth_shift[4] = { 8, 3, 1, 0 };

/*There are 8 different LFO PM depths available, they are:
 0, 3.4, 6.7, 10, 14, 20, 40, 80 (cents)

 Modulation level at each depth depends on F-NUMBER bits: 4,5,6,7,8,9,10
 (bits 8,9,10 = FNUM MSB from OCT/FNUM register)

 Here we store only first quarter (positive one) of full waveform.
 Full table (lfo_pm_table) containing all 128 waveforms is build
 at run (init) time.

 One value in table below represents 4 (four) basic LFO steps
 (1 PM step = 4 AM steps).

 For example:
 at LFO SPEED=0 (which is 108 samples per basic LFO step)
 one value from "lfo_pm_output" table lasts for 432 consecutive
 samples (4*108=432) and one full LFO waveform cycle lasts for 13824
 samples (32*432=13824; 32 because we store only a quarter of whole
 waveform in the table below)
 */
static const Uint8 lfo_pm_output[7 * 8][8] =   /* 7 bits meaningful (of F-NUMBER), 8 LFO output levels per one depth (out of 32), 8 LFO depths */
{
    /* FNUM BIT 4: 000 0001xxxx */
    /* DEPTH 0 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 1 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 2 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 3 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 4 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 5 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 6 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 7 */
    { 0, 0, 0, 0, 1, 1, 1, 1 },

    /* FNUM BIT 5: 000 0010xxxx */
    /* DEPTH 0 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 1 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 2 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 3 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 4 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 5 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 6 */
    { 0, 0, 0, 0, 1, 1, 1, 1 },
    /* DEPTH 7 */
    { 0, 0, 1, 1, 2, 2, 2, 3 },

    /* FNUM BIT 6: 000 0100xxxx */
    /* DEPTH 0 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 1 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 2 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 3 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 4 */
    { 0, 0, 0, 0, 0, 0, 0, 1 },
    /* DEPTH 5 */
    { 0, 0, 0, 0, 1, 1, 1, 1 },
    /* DEPTH 6 */
    { 0, 0, 1, 1, 2, 2, 2, 3 },
    /* DEPTH 7 */
    { 0, 0, 2, 3, 4, 4, 5, 6 },

    /* FNUM BIT 7: 000 1000xxxx */
    /* DEPTH 0 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 1 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 2 */
    { 0, 0, 0, 0, 0, 0, 1, 1 },
    /* DEPTH 3 */
    { 0, 0, 0, 0, 1, 1, 1, 1 },
    /* DEPTH 4 */
    { 0, 0, 0, 1, 1, 1, 1, 2 },
    /* DEPTH 5 */
    { 0, 0, 1, 1, 2, 2, 2, 3 },
    /* DEPTH 6 */
    { 0, 0, 2, 3, 4, 4, 5, 6 },
    /* DEPTH 7 */
    { 0, 0, 4, 6, 8, 8, 0xa, 0xc },

    /* FNUM BIT 8: 001 0000xxxx */
    /* DEPTH 0 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 1 */
    { 0, 0, 0, 0, 1, 1, 1, 1 },
    /* DEPTH 2 */
    { 0, 0, 0, 1, 1, 1, 2, 2 },
    /* DEPTH 3 */
    { 0, 0, 1, 1, 2, 2, 3, 3 },
    /* DEPTH 4 */
    { 0, 0, 1, 2, 2, 2, 3, 4 },
    /* DEPTH 5 */
    { 0, 0, 2, 3, 4, 4, 5, 6 },
    /* DEPTH 6 */
    { 0, 0, 4, 6, 8, 8, 0xa, 0xc },
    /* DEPTH 7 */
    { 0, 0, 8, 0xc, 0x10, 0x10, 0x14, 0x18 },

    /* FNUM BIT 9: 010 0000xxxx */
    /* DEPTH 0 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 1 */
    { 0, 0, 0, 0, 2, 2, 2, 2 },
    /* DEPTH 2 */
    { 0, 0, 0, 2, 2, 2, 4, 4 },
    /* DEPTH 3 */
    { 0, 0, 2, 2, 4, 4, 6, 6 },
    /* DEPTH 4 */
    { 0, 0, 2, 4, 4, 4, 6, 8 },
    /* DEPTH 5 */
    { 0, 0, 4, 6, 8, 8, 0xa, 0xc },
    /* DEPTH 6 */
    { 0, 0, 8, 0xc, 0x10, 0x10, 0x14, 0x18 },
    /* DEPTH 7 */
    { 0, 0, 0x10, 0x18, 0x20, 0x20, 0x28, 0x30 },

    /* FNUM BIT10: 100 0000xxxx */
    /* DEPTH 0 */
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* DEPTH 1 */
    { 0, 0, 0, 0, 4, 4, 4, 4 },
    /* DEPTH 2 */
    { 0, 0, 0, 4, 4, 4, 8, 8 },
    /* DEPTH 3 */
    { 0, 0, 4, 4, 8, 8, 0xc, 0xc },
    /* DEPTH 4 */
    { 0, 0, 4, 8, 8, 8, 0xc, 0x10 },
    /* DEPTH 5 */
    { 0, 0, 8, 0xc, 0x10, 0x10, 0x14, 0x18 },
    /* DEPTH 6 */
    { 0, 0, 0x10, 0x18, 0x20, 0x20, 0x28, 0x30 },
    /* DEPTH 7 */
    { 0, 0, 0x20, 0x30, 0x40, 0x40, 0x50, 0x60 },

};

/* all 128 LFO PM waveforms */
/* 128 combinations of 7 bits meaningful (of F-NUMBER), 8 LFO depths, 32 LFO output levels per one depth */
static Sint32 lfo_pm_table[128 * 8 * 32];

static SSG_t SSG;

static ym2610_t YM2610;

/* current chip state */
static Sint32 m2 = 0, c1 = 0, c2 = 0; /* Phase Modulation input for operators 2,3,4 */
static Sint32 mem = 0; /* one sample delay memory */

static Sint32 out_fm[8]; /* outputs of working channels */
static Sint32 out_ssg = 0; /* channel output CHENTER only for SSG */
static Sint32 out_adpcma[4]; /* channel output NONE,LEFT,RIGHT or CENTER for YM2608/YM2610 ADPCM */
static Sint32 out_delta[4]; /* channel output NONE,LEFT,RIGHT or CENTER for YM2608/YM2610 DELTAT*/

static Uint32 LFO_AM = 0; /* runtime LFO calculations helper */
static Sint32 LFO_PM = 0; /* runtime LFO calculations helper */

static Uint8* pcmbufA = NULL;
static Uint32 pcmsizeA = 0;

/* Algorithm and tables verified on real YM2610 */

/* usual ADPCM table (16 * 1.1^N) */
static Sint32 steps[49] = { 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55,
                            60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230,
                            253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876,
                            963, 1060, 1166, 1282, 1411, 1552
                          };

/* different from the usual ADPCM table */
static Sint32 step_inc[8] = { -1 * 16, -1 * 16, -1 * 16, -1 * 16, 2 * 16, 5 * 16, 7
                              * 16, 9 * 16
                            };

/* speedup purposes only */
static Sint32 jedi_table[49 * 16];

static FM_TIMERHANDLER sav_TimerHandler;
static FM_IRQHANDLER sav_IRQHandler;

static Uint8* pcmbufB = 0;
static Uint32 pcmsizeB = 0;

/* Forecast to next Forecast (rate = *8) */
/* 1/8 , 3/8 , 5/8 , 7/8 , 9/8 , 11/8 , 13/8 , 15/8 */
static const Sint32 adpcmb_decode_table1[16] = { 1, 3, 5, 7, 9, 11, 13, 15, -1, -3,
                                                 -5, -7, -9, -11, -13, -15,
                                                 };
/* delta to next delta (rate= *64) */
/* 0.9 , 0.9 , 0.9 , 0.9 , 1.2 , 1.6 , 2.0 , 2.4 */
static const Sint32 adpcmb_decode_table2[16] = { 57, 57, 57, 57, 77, 102, 128, 153,
                                                 57, 57, 57, 57, 77, 102, 128, 153
                                               };

/* 0-DRAM x1, 1-ROM, 2-DRAM x8, 3-ROM (3 is bad setting - not allowed by the manual) */
static Uint8 dram_rightshift[4] = { 3, 0, 0, 0 };
/* ******************************************************************************************************************/
/*!
* \brief Sets status and handles IRQ.
*
* \param status Todo.
* \param flag Todo.
*/
/* ******************************************************************************************************************/
static void FM_STATUS_SET ( FM_ST* status, Sint32 flag )
{
    /* set status flag */
    status->status |= flag;

    if ( ! ( status->irq ) && ( status->status & status->irqmask ) )
    {
        status->irq = 1;
        /* callback user interrupt handler (IRQ is OFF to ON) */
        ( status->IRQ_Handler ) ( 1 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Resets status and handles IRQ.
*
* \param status Todo.
* \param flag Todo.
*/
/* ******************************************************************************************************************/
static void FM_STATUS_RESET ( FM_ST* status, Sint32 flag )
{
    /* reset status flag */
    status->status &= ~flag;

    if ( ( status->irq ) && ! ( status->status & status->irqmask ) )
    {
        status->irq = 0;
        /* callback user interrupt handler (IRQ is ON to OFF) */
        ( status->IRQ_Handler ) ( 0 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Sets IRQ mask.
*
* \param status Todo.
* \param flag Todo.
*/
/* ******************************************************************************************************************/
static void FM_IRQMASK_SET ( FM_ST* status, Sint32 flag )
{
    status->irqmask = flag;
    /* IRQ handling check */
    FM_STATUS_SET ( status, 0 );
    FM_STATUS_RESET ( status, 0 );
}
/* ******************************************************************************************************************/
/*!
* \brief Writes OPN Mode Register.
*
* \param status Todo.
* \param value Todo.
*/
/* ******************************************************************************************************************/
static void set_timers ( FM_ST* status, Sint32 value )
{
    /* b7 = CSM MODE */
    /* b6 = 3 slot mode */
    /* b5 = reset b */
    /* b4 = reset a */
    /* b3 = timer enable b */
    /* b2 = timer enable a */
    /* b1 = load b */
    /* b0 = load a */
    status->mode = value;

    /* reset Timer b flag */
    if ( value & 0x20 )
    {
        FM_STATUS_RESET ( status, 0x02 );
    }

    /* reset Timer a flag */
    if ( value & 0x10 )
    {
        FM_STATUS_RESET ( status, 0x01 );
    }

    /* load b */
    if ( value & 0x02 )
    {
        if ( status->TBC == 0 )
        {
            status->TBC = ( 256 - status->TB ) << 4;
            /* External timer handler */
#if FM_INTERNAL_TIMER==0
            ( status->Timer_Handler ) ( 1, status->TBC, status->TimerBase );
#endif
        }
    }
    else     /* stop timer b */
    {
        if ( status->TBC != 0 )
        {
            status->TBC = 0;
#if FM_INTERNAL_TIMER==0
            ( status->Timer_Handler ) ( 1, 0, status->TimerBase );
#endif
        }
    }

    /* load a */
    if ( value & 0x01 )
    {
        if ( status->TAC == 0 )
        {
            status->TAC = ( 1024 - status->TA );
            /* External timer handler */
#if FM_INTERNAL_TIMER==0
            ( status->Timer_Handler ) ( 0, status->TAC, status->TimerBase );
#endif
        }
    }
    else     /* stop timer a */
    {
        if ( status->TAC != 0 )
        {
            status->TAC = 0;
#if FM_INTERNAL_TIMER==0
            ( status->Timer_Handler ) ( 0, 0, status->TimerBase );
#endif
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Handles timer A Overflow.
*
* \param status Todo.
*/
/* ******************************************************************************************************************/
static void TimerAOver ( FM_ST* status )
{
    /* set status (if enabled) */
    if ( status->mode & 0x04 )
    {
        FM_STATUS_SET ( status, 0x01 );
    }

    /* clear or reload the counter */
    status->TAC = ( 1024 - status->TA );
#if FM_INTERNAL_TIMER==0
    ( status->Timer_Handler ) ( 0, status->TAC, status->TimerBase );
#endif
}
/* ******************************************************************************************************************/
/*!
* \brief Handles timer B Overflow.
*
* \param status Todo.
*/
/* ******************************************************************************************************************/
static void TimerBOver ( FM_ST* status )
{
    /* set status (if enabled) */
    if ( status->mode & 0x08 )
    {
        FM_STATUS_SET ( status, 0x02 );
    }

    /* clear or reload the counter */
    status->TBC = ( 256 - status->TB ) << 4;
#if FM_INTERNAL_TIMER==0
    ( status->Timer_Handler ) ( 1, status->TBC, status->TimerBase );
#endif
}
/* @todo (Tmesys#1#12/05/2022): Analyze further to see if we keep it like this. */
#if FM_INTERNAL_TIMER
/* ----- internal timer mode , update timer */

/* ---------- calculate timer A ---------- */
#define INTERNAL_TIMER_A(ST,CSM_CH)                 \
    {                                                   \
        if( ST.TAC /*&&  (ST.Timer_Handler==0) */)      \
            /*if( (ST.TAC -= (Sint32)(ST.freqbase*4096)) <= 0 )*/  \
            if( (ST.TAC -= (Sint32)((1000.0/ST.rate)*4096)) <= 0 ) \
            {                                           \
                TimerAOver( &ST );                      \
                /* CSM mode total level latch and auto key on */    \
                if( ST.mode & 0x80 )                    \
                    CSMKeyControll( CSM_CH );           \
            }                                           \
    }
/* ---------- calculate timer B ---------- */
#define INTERNAL_TIMER_B(ST,step)                       \
    {                                                       \
        if( ST.TBC /*&& (ST.Timer_Handler==0) */)               \
            /*if( (ST.TBC -= (Sint32)(ST.freqbase*4096*step)) <= 0 )*/ \
            if( (ST.TBC -= (Sint32)((1000.0/ST.rate)*4096*step)) <= 0 )    \
                TimerBOver( &ST );                          \
    }
#else /* FM_INTERNAL_TIMER */
/* external timer mode */
#define INTERNAL_TIMER_A(ST,CSM_CH)
#define INTERNAL_TIMER_B(ST,step)
#endif /* FM_INTERNAL_TIMER */
/* ******************************************************************************************************************/
/*!
* \brief Handles status flag ?
*
* \param status Todo.
* \return Status Todo.
*/
/* ******************************************************************************************************************/
static Uint8 FM_STATUS_FLAG ( FM_ST* status )
{
    if ( status->BusyExpire )
    {
        if ( ( status->BusyExpire - timer_get_time() ) > 0 )
        {
            return status->status | 0x80;    /* with busy */
        }

        /* expire */
        status->BusyExpire = 0;
    }

    return status->status;
}
/* ******************************************************************************************************************/
/*!
* \brief Sets busy flag ?
*
* \param status Todo.
* \param busyclock Todo.
*/
/* ******************************************************************************************************************/
static void FM_BUSY_SET ( FM_ST* status, Sint32 busyclock )
{
    status->BusyExpire = timer_get_time() + ( status->TimerBase * busyclock );
}
/* ******************************************************************************************************************/
/*!
* \brief Sets busy flag ?
*
* \param status Todo.
* \param busyclock Todo.
*/
/* ******************************************************************************************************************/
static void FM_KEYON ( FM_CH* channels, Sint32 slot )
{
    FM_SLOT* SLOT = &channels->SLOT[slot];

    if ( !SLOT->key )
    {
        SLOT->key = 1;
        SLOT->phase = 0; /* restart Phase Generator */
        SLOT->state = EG_ATT; /* phase -> Attack */
    }
}
/* ******************************************************************************************************************/
/*!
* \brief ?
*
* \param channels Todo.
* \param slot Todo.
*/
/* ******************************************************************************************************************/
static void FM_KEYOFF ( FM_CH* channels, Sint32 slot )
{
    FM_SLOT* SLOT = &channels->SLOT[slot];

    if ( SLOT->key )
    {
        SLOT->key = 0;

        if ( SLOT->state > EG_REL )
        {
            SLOT->state = EG_REL;    /* phase -> Release */
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Sets algorithm connection.
*
* \param channels Todo.
* \param slot Todo.
*/
/* ******************************************************************************************************************/
static void setup_connection ( FM_CH* channels, Sint32 channel )
{
    Sint32* carrier = &out_fm[channel];

    Sint32** om1 = &channels->connect1;
    Sint32** om2 = &channels->connect3;
    Sint32** oc1 = &channels->connect2;

    Sint32** memc = &channels->mem_connect;

    switch ( channels->ALGO )
    {
    case ( 0 ) :
        {
            /* M1---C1---MEM---M2---C2---OUT */
            *om1 = &c1;
            *oc1 = &mem;
            *om2 = &c2;
            *memc = &m2;
        }
        break;

    case ( 1 ) :
        {
            /* M1------+-MEM---M2---C2---OUT */
            /*      C1-+                     */
            *om1 = &mem;
            *oc1 = &mem;
            *om2 = &c2;
            *memc = &m2;
        }
        break;

    case ( 2 ) :
        {
            /* M1-----------------+-C2---OUT */
            /*      C1---MEM---M2-+          */
            *om1 = &c2;
            *oc1 = &mem;
            *om2 = &c2;
            *memc = &m2;
        }
        break;

    case ( 3 ) :
        {
            /* M1---C1---MEM------+-C2---OUT */
            /*                 M2-+          */
            *om1 = &c1;
            *oc1 = &mem;
            *om2 = &c2;
            *memc = &c2;
        }
        break;

    case ( 4 ) :
        {
            /* M1---C1-+-OUT */
            /* M2---C2-+     */
            /* MEM: not used */
            *om1 = &c1;
            *oc1 = carrier;
            *om2 = &c2;
            *memc = &mem; /* store it anywhere where it will not be used */
        }
        break;

    case ( 5 ) :
        {
            /*    +----C1----+     */
            /* M1-+-MEM---M2-+-OUT */
            /*    +----C2----+     */
            *om1 = 0; /* special mark */
            *oc1 = carrier;
            *om2 = carrier;
            *memc = &m2;
        }
        break;

    case ( 6 ) :
        {
            /* M1---C1-+     */
            /*      M2-+-OUT */
            /*      C2-+     */
            /* MEM: not used */
            *om1 = &c1;
            *oc1 = carrier;
            *om2 = carrier;
            *memc = &mem; /* store it anywhere where it will not be used */
        }
        break;

    case ( 7 ) :
        {
            /* M1-+     */
            /* C1-+-OUT */
            /* M2-+     */
            /* C2-+     */
            /* MEM: not used*/
            *om1 = carrier;
            *oc1 = carrier;
            *om2 = carrier;
            *memc = &mem; /* store it anywhere where it will not be used */
        }
        break;
    }

    channels->connect4 = carrier;
}
/* ******************************************************************************************************************/
/*!
* \brief Sets detune & multiple.
*
* \param status Todo.
* \param channel Todo.
* \param slot Todo.
* \param value Todo.
*/
/* ******************************************************************************************************************/
static void set_det_mul ( FM_ST* status, FM_CH* channel, FM_SLOT* slot, Sint32 value )
{
    slot->mul = ( value & 0x0f ) ? ( value & 0x0f ) * 2 : 1;
    slot->DT = status->dt_tab[ ( value >> 4 ) & 7];
    channel->SLOT[SLOT1].Incr = -1;
}
/* ******************************************************************************************************************/
/*!
* \brief Sets total level.
*
* \param slot Todo.
* \param value Todo.
*/
/* ******************************************************************************************************************/
static void set_tl ( FM_SLOT* slot, Sint32 value )
{
    /* 7bit TL */
    slot->tl = ( value & 0x7f ) << ( ENV_BITS - 7 );
}
/* ******************************************************************************************************************/
/*!
* \brief Sets attack rate & key scale.
*
* \param channel Todo.
* \param slot Todo.
* \param value Todo.
*/
/* ******************************************************************************************************************/
static void set_ar_ksr ( FM_CH* channel, FM_SLOT* slot, Sint32 value )
{
    Uint8 old_KSR = slot->KSR;

    slot->ar = ( value & 0x1f ) ? 32 + ( ( value & 0x1f ) << 1 ) : 0;

    slot->KSR = 3 - ( value >> 6 );

    if ( slot->KSR != old_KSR )
    {
        channel->SLOT[SLOT1].Incr = -1;
    }
    else
    {
        /* refresh Attack rate */
        if ( ( slot->ar + slot->ksr ) < 32 + 62 )
        {
            slot->eg_sh_ar = eg_rate_shift[slot->ar + slot->ksr];
            slot->eg_sel_ar = eg_rate_select[slot->ar + slot->ksr];
        }
        else
        {
            slot->eg_sh_ar = 0;
            slot->eg_sel_ar = 17 * RATE_STEPS;
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Sets decay rate.
*
* \param slot Todo.
* \param value Todo.
*/
/* ******************************************************************************************************************/
static void set_dr ( FM_SLOT* slot, Sint32 value )
{
    slot->d1r = ( value & 0x1f ) ? 32 + ( ( value & 0x1f ) << 1 ) : 0;

    slot->eg_sh_d1r = eg_rate_shift[slot->d1r + slot->ksr];
    slot->eg_sel_d1r = eg_rate_select[slot->d1r + slot->ksr];
}
/* ******************************************************************************************************************/
/*!
* \brief Sets sustain rate.
*
* \param slot Todo.
* \param value Todo.
*/
/* ******************************************************************************************************************/
static void set_sr ( FM_SLOT* slot, Sint32 value )
{
    slot->d2r = ( value & 0x1f ) ? 32 + ( ( value & 0x1f ) << 1 ) : 0;

    slot->eg_sh_d2r = eg_rate_shift[slot->d2r + slot->ksr];
    slot->eg_sel_d2r = eg_rate_select[slot->d2r + slot->ksr];
}
/* ******************************************************************************************************************/
/*!
* \brief Sets release rate.
*
* \param slot Todo.
* \param value Todo.
*/
/* ******************************************************************************************************************/
static void set_sl_rr ( FM_SLOT* slot, Sint32 value )
{
    slot->sl = sl_table[value >> 4];

    slot->rr = 34 + ( ( value & 0x0f ) << 2 );

    slot->eg_sh_rr = eg_rate_shift[slot->rr + slot->ksr];
    slot->eg_sel_rr = eg_rate_select[slot->rr + slot->ksr];
}
/* ******************************************************************************************************************/
/*!
* \brief ???
*
* \param phase Todo.
* \param env Todo.
* \param pm Todo.
*/
/* ******************************************************************************************************************/
static Sint32 op_calc ( Uint32 phase, Uint32 env, Sint32 pm )
{
    Uint32 p = 0;

    p = ( env << 3 )
        + sin_tab[ ( ( ( Sint32 ) ( ( phase & ~FREQ_MASK ) + ( pm << 15 ) ) )
                                    >> FREQ_SH ) & SIN_MASK];

    if ( p >= TL_TAB_LEN )
    {
        return 0;
    }

    return ( tl_tab[p] );
}
/* ******************************************************************************************************************/
/*!
* \brief ???
*
* \param phase Todo.
* \param env Todo.
* \param pm Todo.
*/
/* ******************************************************************************************************************/
static Sint32 op_calc1 ( Uint32 phase, Uint32 env, Sint32 pm )
{
    Uint32 p = 0;

    p = ( env << 3 )
        + sin_tab[ ( ( ( Sint32 ) ( ( phase & ~FREQ_MASK ) + pm ) ) >> FREQ_SH )
                                  & SIN_MASK];

    if ( p >= TL_TAB_LEN )
    {
        return ( 0 );
    }

    return ( tl_tab[p] );
}
/* ******************************************************************************************************************/
/*!
* \brief Advances LFO to next sample.
*
* \param opn Todo.
*/
/* ******************************************************************************************************************/
static void advance_lfo ( FM_OPN* opn )
{
    Uint8 pos = 0;
    Uint8 prev_pos = 0;

    /* LFO enabled ? */
    if ( opn->lfo_inc )
    {
        prev_pos = opn->lfo_cnt >> LFO_SH & 127;

        opn->lfo_cnt += opn->lfo_inc;

        pos = ( opn->lfo_cnt >> LFO_SH ) & 127;

        /* update AM when LFO output changes */

        /*if (prev_pos != pos)*/
        /* actually I can't optimize is this way without rewritting chan_calc()
         to use chip->lfo_am instead of global lfo_am */
        {

            /* triangle */
            /* AM: 0 to 126 step +2, 126 to 0 step -2 */
            if ( pos < 64 )
            {
                LFO_AM = ( pos & 63 ) * 2;
            }

            else
            {
                LFO_AM = 126 - ( ( pos & 63 ) * 2 );
            }
        }

        /* PM works with 4 times slower clock */
        prev_pos >>= 2;
        pos >>= 2;
        /* update PM when LFO output changes */
        /*if (prev_pos != pos)*//* can't use global lfo_pm for this optimization, must be chip->lfo_pm instead*/
        {
            LFO_PM = pos;
        }

    }
    else
    {
        LFO_AM = 0;
        LFO_PM = 0;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief ???
*
* \param opn Todo.
* \param slot Todo.
*/
/* ******************************************************************************************************************/
static void advance_eg_channel ( const FM_OPN* opn, FM_SLOT* slot )
{
    Uint32 out = 0;
    Uint32 swap_flag = 0;
    /* four operators per channel */
    Uint32 i = 4;

    do
    {
        switch ( slot->state )
        {
        /* attack phase */
        case ( EG_ATT ) :
            {
                if ( ! ( opn->eg_cnt & ( ( 1 << slot->eg_sh_ar ) - 1 ) ) )
                {
                    slot->volume += ( ~slot->volume
                                      * ( eg_inc[slot->eg_sel_ar + ( ( opn->eg_cnt >> slot->eg_sh_ar ) & 7 )] ) ) >> 4;

                    if ( slot->volume <= MIN_ATT_INDEX )
                    {
                        slot->volume = MIN_ATT_INDEX;
                        slot->state = EG_DEC;
                    }
                }
            }
            break;
        /* decay phase */
        case ( EG_DEC ) :
            {
                if ( slot->ssg & 0x08 ) /* SSG EG type envelope selected */
                {
                    if ( ! ( opn->eg_cnt & ( ( 1 << slot->eg_sh_d1r ) - 1 ) ) )
                    {
                        slot->volume += ( eg_inc[slot->eg_sel_d1r + ( ( opn->eg_cnt >> slot->eg_sh_d1r ) & 7 )] << 2 );

                        if ( slot->volume >= slot->sl )
                        {
                            slot->state = EG_SUS;
                        }
                    }
                }
                else
                {
                    if ( ! ( opn->eg_cnt & ( ( 1 << slot->eg_sh_d1r ) - 1 ) ) )
                    {
                        slot->volume += eg_inc[slot->eg_sel_d1r + ( ( opn->eg_cnt >> slot->eg_sh_d1r ) & 7 )];

                        if ( slot->volume >= slot->sl )
                        {
                            slot->state = EG_SUS;
                        }
                    }
                }

            }
            break;
        /* sustain phase */
        case ( EG_SUS ) :
            {
                if ( slot->ssg & 0x08 ) /* SSG EG type envelope selected */
                {
                    if ( ! ( opn->eg_cnt & ( ( 1 << slot->eg_sh_d2r ) - 1 ) ) )
                    {
                        slot->volume += ( eg_inc[slot->eg_sel_d2r + ( ( opn->eg_cnt >> slot->eg_sh_d2r ) & 7 )] << 2 );

                        if ( slot->volume >= MAX_ATT_INDEX )
                        {
                            slot->volume = MAX_ATT_INDEX;

                            if ( slot->ssg & 0x01 ) /* bit 0 = hold */
                            {
                                if ( slot->ssgn & 1 ) /* have we swapped once ??? */
                                {
                                    /* yes, so do nothing, just hold current level */
                                }
                                else
                                {
                                    swap_flag = ( slot->ssg & 0x02 ) | 1;    /* bit 1 = alternate */
                                }

                            }
                            else
                            {
                                /* same as KEY-ON operation */

                                /* restart of the Phase Generator should be here,
                                 only if AR is not maximum ??? */
                                /*slot->phase = 0;*/

                                /* phase -> Attack */
                                slot->state = EG_ATT;

                                swap_flag = ( slot->ssg & 0x02 ); /* bit 1 = alternate */
                            }
                        }
                    }
                }
                else
                {
                    if ( ! ( opn->eg_cnt & ( ( 1 << slot->eg_sh_d2r ) - 1 ) ) )
                    {
                        slot->volume += eg_inc[slot->eg_sel_d2r
                                               + ( ( opn->eg_cnt >> slot->eg_sh_d2r ) & 7 )];

                        if ( slot->volume >= MAX_ATT_INDEX )
                        {
                            slot->volume = MAX_ATT_INDEX;
                            /* do not change slot->state (verified on real chip) */
                        }
                    }

                }
            }
            break;
        /* release phase */
        case ( EG_REL ) :
            {
                if ( ! ( opn->eg_cnt & ( ( 1 << slot->eg_sh_rr ) - 1 ) ) )
                {
                    slot->volume += eg_inc[slot->eg_sel_rr
                                           + ( ( opn->eg_cnt >> slot->eg_sh_rr ) & 7 )];

                    if ( slot->volume >= MAX_ATT_INDEX )
                    {
                        slot->volume = MAX_ATT_INDEX;
                        slot->state = EG_OFF;
                    }
                }
            }
            break;
            /*
                    default :
                        {
                            zlog_error ( gngeox_config.loggingCat, "Unknown slot state %d", slot->state );
                        }
                        break;
            */
        }

        out = slot->tl + ( ( Uint32 ) slot->volume );

        if ( ( slot->ssg & 0x08 ) && ( slot->ssgn & 2 ) ) /* negate output (changes come from alternate bit, init comes from attack bit) */
        {
            out ^= ( ( 1 << ENV_BITS ) - 1 );    /* 1023 */
        }

        /* we need to store the result here because we are going to change ssgn
         in next instruction */
        slot->vol_out = out;

        slot->ssgn ^= swap_flag;

        /* @fixme (Tmesys#1#12/06/2022): Well, increment pointer on structure ? */
        slot++;
        i--;
    }
    while ( i );
}
/* ******************************************************************************************************************/
/*!
* \brief ???
*
* \param opn Todo.
* \param slot Todo.
*/
/* ******************************************************************************************************************/
static void chan_calc ( FM_OPN* OPN, FM_CH* CH )
{
    Uint32 eg_out = 0;

    Uint32 AM = LFO_AM >> CH->ams;

    m2 = c1 = c2 = mem = 0;

    *CH->mem_connect = CH->mem_value; /* restore delayed sample (MEM) value to m2 or c2 */

    eg_out = volume_calc ( &CH->SLOT[SLOT1] );
    {
        Sint32 out = CH->op1_out[0] + CH->op1_out[1];
        CH->op1_out[0] = CH->op1_out[1];

        if ( !CH->connect1 )
        {
            /* algorithm 5  */
            mem = c1 = c2 = CH->op1_out[0];
        }
        else
        {
            /* other algorithms */
            *CH->connect1 += CH->op1_out[0];
        }

        CH->op1_out[1] = 0;

        if ( eg_out < ENV_QUIET ) /* SLOT 1 */
        {
            if ( !CH->FB )
            {
                out = 0;
            }

            CH->op1_out[1] = op_calc1 ( CH->SLOT[SLOT1].phase, eg_out, ( out << CH->FB ) );
        }
    }

    eg_out = volume_calc ( &CH->SLOT[SLOT3] );

    if ( eg_out < ENV_QUIET ) /* SLOT 3 */
    {
        *CH->connect3 += op_calc ( CH->SLOT[SLOT3].phase, eg_out, m2 );
    }

    eg_out = volume_calc ( &CH->SLOT[SLOT2] );

    if ( eg_out < ENV_QUIET ) /* SLOT 2 */
    {
        *CH->connect2 += op_calc ( CH->SLOT[SLOT2].phase, eg_out, c1 );
    }

    eg_out = volume_calc ( &CH->SLOT[SLOT4] );

    if ( eg_out < ENV_QUIET ) /* SLOT 4 */
    {
        *CH->connect4 += op_calc ( CH->SLOT[SLOT4].phase, eg_out, c2 );
    }

    /* store current MEM */
    CH->mem_value = mem;

    /* update phase counters AFTER output calculations */
    if ( CH->pms )
    {
        /* add support for 3 slot mode */
        Uint32 block_fnum = CH->block_fnum;

        Uint32 fnum_lfo = ( ( block_fnum & 0x7f0 ) >> 4 ) * 32 * 8;
        Sint32 lfo_fn_table_index_offset = lfo_pm_table[fnum_lfo + CH->pms + LFO_PM];

        /* LFO phase modulation active */
        if ( lfo_fn_table_index_offset )
        {
            Uint8 blk = 0;
            Uint32 fn = 0;
            Sint32 kc = 0, fc = 0;

            block_fnum = block_fnum * 2 + lfo_fn_table_index_offset;

            blk = ( block_fnum & 0x7000 ) >> 12;
            fn = block_fnum & 0xfff;

            /* keyscale code */
            kc = ( blk << 2 ) | opn_fktable[fn >> 8];
            /* phase increment counter */
            fc = OPN->fn_table[fn] >> ( 7 - blk );

            CH->SLOT[SLOT1].phase += ( ( fc + CH->SLOT[SLOT1].DT[kc] )
                                       * CH->SLOT[SLOT1].mul ) >> 1;
            CH->SLOT[SLOT2].phase += ( ( fc + CH->SLOT[SLOT2].DT[kc] )
                                       * CH->SLOT[SLOT2].mul ) >> 1;
            CH->SLOT[SLOT3].phase += ( ( fc + CH->SLOT[SLOT3].DT[kc] )
                                       * CH->SLOT[SLOT3].mul ) >> 1;
            CH->SLOT[SLOT4].phase += ( ( fc + CH->SLOT[SLOT4].DT[kc] )
                                       * CH->SLOT[SLOT4].mul ) >> 1;
        }
        else
        {
            /* LFO phase modulation  = zero */
            CH->SLOT[SLOT1].phase += CH->SLOT[SLOT1].Incr;
            CH->SLOT[SLOT2].phase += CH->SLOT[SLOT2].Incr;
            CH->SLOT[SLOT3].phase += CH->SLOT[SLOT3].Incr;
            CH->SLOT[SLOT4].phase += CH->SLOT[SLOT4].Incr;
        }
    }
    else
    {
        /* no LFO phase modulation */
        CH->SLOT[SLOT1].phase += CH->SLOT[SLOT1].Incr;
        CH->SLOT[SLOT2].phase += CH->SLOT[SLOT2].Incr;
        CH->SLOT[SLOT3].phase += CH->SLOT[SLOT3].Incr;
        CH->SLOT[SLOT4].phase += CH->SLOT[SLOT4].Incr;
    }
}

/* update phase increment and envelope generator */
static void refresh_fc_eg_slot ( FM_SLOT* SLOT, Sint32 fc, Sint32 kc )
{
    Sint32 ksr = 0;

    /* (frequency) phase increment counter */
    SLOT->Incr = ( ( fc + SLOT->DT[kc] ) * SLOT->mul ) >> 1;

    ksr = kc >> SLOT->KSR;

    if ( SLOT->ksr != ksr )
    {
        SLOT->ksr = ksr;

        /* calculate envelope generator rates */
        if ( ( SLOT->ar + SLOT->ksr ) < 32 + 62 )
        {
            SLOT->eg_sh_ar = eg_rate_shift[SLOT->ar + SLOT->ksr];
            SLOT->eg_sel_ar = eg_rate_select[SLOT->ar + SLOT->ksr];
        }

        else
        {
            SLOT->eg_sh_ar = 0;
            SLOT->eg_sel_ar = 17 * RATE_STEPS;
        }

        SLOT->eg_sh_d1r = eg_rate_shift[SLOT->d1r + SLOT->ksr];
        SLOT->eg_sel_d1r = eg_rate_select[SLOT->d1r + SLOT->ksr];

        SLOT->eg_sh_d2r = eg_rate_shift[SLOT->d2r + SLOT->ksr];
        SLOT->eg_sel_d2r = eg_rate_select[SLOT->d2r + SLOT->ksr];

        SLOT->eg_sh_rr = eg_rate_shift[SLOT->rr + SLOT->ksr];
        SLOT->eg_sel_rr = eg_rate_select[SLOT->rr + SLOT->ksr];
    }
}

/* update phase increment counters */
static void refresh_fc_eg_chan ( FM_CH* CH )
{
    if ( CH->SLOT[SLOT1].Incr == -1 )
    {
        Sint32 fc = CH->fc;
        Sint32 kc = CH->kcode;
        refresh_fc_eg_slot ( &CH->SLOT[SLOT1], fc, kc );
        refresh_fc_eg_slot ( &CH->SLOT[SLOT2], fc, kc );
        refresh_fc_eg_slot ( &CH->SLOT[SLOT3], fc, kc );
        refresh_fc_eg_slot ( &CH->SLOT[SLOT4], fc, kc );
    }
}

/* initialize time tables */
static void init_timetables ( FM_ST* ST, const Uint8* dttable )
{
    double rate = 0;

#if 0
    logerror ( "FM.C: samplerate=%8i chip clock=%8i  freqbase=%f  \n",
               ST->rate, ST->clock, ST->freqbase );
#endif

    /* DeTune table */
    for ( Sint32 d = 0; d <= 3; d++ )
    {
        for ( Sint32 i = 0; i <= 31; i++ )
        {
            rate = ( ( double ) dttable[d * 32 + i] ) * SIN_LEN * ST->freqbase
                   * ( 1 << FREQ_SH ) / ( ( double ) ( 1 << 20 ) );
            ST->dt_tab[d][i] = ( Sint32 ) rate;
            ST->dt_tab[d + 4][i] = -ST->dt_tab[d][i];
#if 0
            logerror ( "FM.C: DT [%2i %2i] = %8x  \n", d, i, ST->dt_tab[d][i] );
#endif
        }
    }

}

static void reset_channels ( FM_ST* ST, FM_CH* CH, Sint32 num )
{
    //Sint32 c = 0, s = 0;

    /* normal mode */
    ST->mode = 0;
    ST->TA = 0;
    ST->TAC = 0;
    ST->TB = 0;
    ST->TBC = 0;

    for ( Sint32 c = 0; c < num; c++ )
    {
        CH[c].fc = 0;

        for ( Sint32 s = 0; s < 4; s++ )
        {
            CH[c].SLOT[s].ssg = 0;
            CH[c].SLOT[s].ssgn = 0;
            CH[c].SLOT[s].state = EG_OFF;
            CH[c].SLOT[s].volume = MAX_ATT_INDEX;
            CH[c].SLOT[s].vol_out = MAX_ATT_INDEX;
        }
    }
}

/* initialize generic tables */
static void OPNInitTable ( void )
{
    Sint32 i, x;
    Sint32 n;
    double o, m;

    for ( x = 0; x < TL_RES_LEN; x++ )
    {
        m = ( 1 << 16 ) / pow ( 2, ( x + 1 ) * ( ENV_STEP / 4.0 ) / 8.0 );
        m = floor ( m );

        /* we never reach (1<<16) here due to the (x+1) */
        /* result fits within 16 bits at maximum */

        n = ( Sint32 ) m; /* 16 bits here */
        n >>= 4; /* 12 bits here */

        if ( n & 1 ) /* round to nearest */
        {
            n = ( n >> 1 ) + 1;
        }
        else
        {
            n = n >> 1;
        }

        /* 11 bits here (rounded) */
        n <<= 2; /* 13 bits here (as in real chip) */
        tl_tab[x * 2 + 0] = n;
        tl_tab[x * 2 + 1] = -tl_tab[x * 2 + 0];

        for ( i = 1; i < 13; i++ )
        {
            tl_tab[x * 2 + 0 + i * 2 * TL_RES_LEN] = tl_tab[x * 2 + 0] >> i;
            tl_tab[x * 2 + 1 + i * 2 * TL_RES_LEN] = -tl_tab[x * 2 + 0 + i * 2 * TL_RES_LEN];
        }

#if 0
        logerror ( "tl %04i", x );

        for ( i = 0; i < 13; i++ )
        {
            logerror ( ", [%02i] %4x", i * 2, tl_tab[ x * 2 /*+1*/ + i * 2 * TL_RES_LEN ] );
        }

        logerror ( "\n" );
#endif
    }

    /*logerror("FM.C: TL_TAB_LEN = %i elements (%i bytes)\n",TL_TAB_LEN, (Sint32)sizeof(tl_tab));*/

    for ( i = 0; i < SIN_LEN; i++ )
    {
        /* non-standard sinus */
        m = sin ( ( ( i * 2 ) + 1 ) * M_PI / SIN_LEN ); /* checked against the real chip */

        /* we never reach zero here due to ((i*2)+1) */

        if ( m > 0.0 )
        {
            o = 8 * log ( 1.0 / m ) / log ( 2 );    /* convert to 'decibels' */
        }

        else
        {
            o = 8 * log ( -1.0 / m ) / log ( 2 );    /* convert to 'decibels' */
        }

        o = o / ( ENV_STEP / 4 );

        n = ( Sint32 ) ( 2.0 * o );

        if ( n & 1 ) /* round to nearest */
        {
            n = ( n >> 1 ) + 1;
        }
        else
        {
            n = n >> 1;
        }

        sin_tab[i] = n * 2 + ( m >= 0.0 ? 0 : 1 );
        /*logerror("FM.C: sin [%4i]= %4i (tl_tab value=%5i)\n", i, sin_tab[i],tl_tab[sin_tab[i]]);*/
    }

    /*logerror("FM.C: ENV_QUIET= %08x\n",ENV_QUIET );*/

    /* build LFO PM modulation table */
    for ( i = 0; i < 8; i++ ) /* 8 PM depths */
    {
        Uint8 fnum;

        for ( fnum = 0; fnum < 128; fnum++ ) /* 7 bits meaningful of F-NUMBER */
        {
            Uint8 value;
            Uint8 step;
            Uint32 offset_depth = i;
            Uint32 offset_fnum_bit;
            Uint32 bit_tmp;

            for ( step = 0; step < 8; step++ )
            {
                value = 0;

                for ( bit_tmp = 0; bit_tmp < 7; bit_tmp++ ) /* 7 bits */
                {
                    if ( fnum & ( 1 << bit_tmp ) ) /* only if bit "bit_tmp" is set */
                    {
                        offset_fnum_bit = bit_tmp * 8;
                        value +=
                            lfo_pm_output[offset_fnum_bit + offset_depth][step];
                    }
                }

                lfo_pm_table[ ( fnum * 32 * 8 ) + ( i * 32 ) + step + 0] = value;
                lfo_pm_table[ ( fnum * 32 * 8 ) + ( i * 32 ) + ( step ^ 7 ) + 8] = value;
                lfo_pm_table[ ( fnum * 32 * 8 ) + ( i * 32 ) + step + 16] = -value;
                lfo_pm_table[ ( fnum * 32 * 8 ) + ( i * 32 ) + ( step ^ 7 ) + 24] = -value;
            }

#if 0
            logerror ( "LFO depth=%1x FNUM=%04x (<<4=%4x): ", i, fnum, fnum << 4 );

            for ( step = 0; step < 16; step++ ) /* dump only positive part of waveforms */
            {
                logerror ( "%02x ", lfo_pm_table[ ( fnum * 32 * 8 ) + ( i * 32 ) + step] );
            }

            logerror ( "\n" );
#endif
        }
    }
}

/* CSM Key Controll */
static void CSMKeyControll ( FM_CH* CH )
{
    /* this is wrong, atm */

    /* all key on */
    FM_KEYON ( CH, SLOT1 );
    FM_KEYON ( CH, SLOT2 );
    FM_KEYON ( CH, SLOT3 );
    FM_KEYON ( CH, SLOT4 );
}

/* prescaler set (and make time tables) */
static void OPNSetPres ( FM_OPN* OPN, Sint32 pres, Sint32 TimerPres, Sint32 SSGpres )
{
    //Sint32 i;

    /* frequency base */
    OPN->ST.freqbase = ( OPN->ST.rate ) ? ( ( double ) OPN->ST.clock / OPN->ST.rate ) / pres : 0;

#if 0
    OPN->ST.rate = ( double ) OPN->ST.clock / pres;
    OPN->ST.freqbase = 1.0;
#endif

    OPN->eg_timer_add = ( 1 << EG_SH ) * OPN->ST.freqbase;
    OPN->eg_timer_overflow = ( 3 ) * ( 1 << EG_SH );

    /* Timer base time */
    OPN->ST.TimerBase = 1.0 / ( ( double ) OPN->ST.clock / ( double ) TimerPres );

    /* SSG part  prescaler set */
    if ( SSGpres )
    {
        SSG.step = ( ( double ) SSG_STEP * OPN->ST.rate * 8 ) / ( OPN->ST.clock * 2 / SSGpres );
    }

    /* make time tables */
    init_timetables ( &OPN->ST, dt_tab );

    /* there are 2048 FNUMs that can be generated using FNUM/BLK registers
     but LFO works with one more bit of a precision so we really need 4096 elements */
    /* calculate fnumber -> increment counter table */
    for ( Sint32 i = 0; i < 4096; i++ )
    {
        /* freq table for octave 7 */
        /* OPN phase increment counter = 20bit */
        /* FREQ_SH-10 because chip works with 10.10 fixed point, while we use 16.16 */
        OPN->fn_table[i] = ( Uint32 ) ( ( double ) i * 32 * OPN->ST.freqbase * ( 1 << ( FREQ_SH - 10 ) ) );
#if 0
        logerror ( "FM.C: fn_table[%4i] = %08x (dec=%8i)\n",
                   i, OPN->fn_table[i] >> 6, OPN->fn_table[i] >> 6 );
#endif
    }

    /* LFO freq. table */
    for ( Sint32 i = 0; i < 8; i++ )
    {
        /* Amplitude modulation: 64 output levels (triangle waveform); 1 level lasts for one of "lfo_samples_per_step" samples */
        /* Phase modulation: one entry from lfo_pm_output lasts for one of 4 * "lfo_samples_per_step" samples  */
        OPN->lfo_freq[i] = ( 1.0 / lfo_samples_per_step[i] ) * ( 1 << LFO_SH ) * OPN->ST.freqbase;
#if 0
        logerror ( "FM.C: lfo_freq[%i] = %08x (dec=%8i)\n",
                   i, OPN->lfo_freq[i], OPN->lfo_freq[i] );
#endif
    }
}

/* write a OPN mode register 0x20-0x2f */
static void OPNWriteMode ( FM_OPN* OPN, Sint32 r, Sint32 v )
{
    switch ( r )
    {
    /* Test */
    case ( 0x21 ) :
        break;

    /* LFO FREQ (YM2608/YM2610/YM2610B/YM2612) */
    case ( 0x22 ) :
        {
            if ( v & 0x08 ) /* LFO enabled ? */
            {
                OPN->lfo_inc = OPN->lfo_freq[v & 7];
            }
            else
            {
                OPN->lfo_inc = 0;
            }
        }
        break;

    /* timer A High 8*/
    case ( 0x24 ) :
        {
            OPN->ST.TA = ( OPN->ST.TA & 0x03 ) | ( ( ( Sint32 ) v ) << 2 );
        }
        break;

    /* timer A Low 2*/
    case ( 0x25 ) :
        {
            OPN->ST.TA = ( OPN->ST.TA & 0x3fc ) | ( v & 3 );
        }
        break;

    /* timer B */
    case ( 0x26 ) :
        {
            OPN->ST.TB = v;
        }
        break;

    /* mode, timer control */
    case ( 0x27 ) :
        {
            set_timers ( & ( OPN->ST ), v );
        }
        break;

    /* key on / off */
    case ( 0x28 ) :
        {
            Uint8 c = 0;
            FM_CH* CH = NULL;

            c = v & 0x03;

            if ( c == 3 )
            {
                break;
            }

            if ( v & 0x04 )
            {
                c += 3;
            }

            CH = OPN->P_CH;
            CH = &CH[c];

            if ( v & 0x10 )
            {
                FM_KEYON ( CH, SLOT1 );
            }
            else
            {
                FM_KEYOFF ( CH, SLOT1 );
            }

            if ( v & 0x20 )
            {
                FM_KEYON ( CH, SLOT2 );
            }
            else
            {
                FM_KEYOFF ( CH, SLOT2 );
            }

            if ( v & 0x40 )
            {
                FM_KEYON ( CH, SLOT3 );
            }
            else
            {
                FM_KEYOFF ( CH, SLOT3 );
            }

            if ( v & 0x80 )
            {
                FM_KEYON ( CH, SLOT4 );
            }
            else
            {
                FM_KEYOFF ( CH, SLOT4 );
            }
        }

        break;
    }
}

/* write a OPN register (0x30-0xff) */
static void OPNWriteReg ( FM_OPN* OPN, Sint32 r, Sint32 v )
{
    FM_CH* CH = NULL;
    FM_SLOT* SLOT = NULL;

    Uint8 c = OPN_CHAN ( r );

    if ( c == 3 )
    {
        /* 0xX3,0xX7,0xXB,0xXF */
        return;
    }

    if ( r >= 0x100 )
    {
        c += 3;
    }

    CH = OPN->P_CH;
    CH = &CH[c];

    SLOT = & ( CH->SLOT[OPN_SLOT ( r )] );

    switch ( r & 0xf0 )
    {
    /* DET , MUL */
    case ( 0x30 ) :
        {
            set_det_mul ( &OPN->ST, CH, SLOT, v );
        }
        break;

    /* TL */
    case ( 0x40 ) :
        {
            set_tl ( SLOT, v );
        }
        break;

    /* KS, AR */
    case ( 0x50 ) :
        {
            set_ar_ksr ( CH, SLOT, v );
        }
        break;

    /* bit7 = AM ENABLE, DR */
    case ( 0x60 ) :
        {
            set_dr ( SLOT, v );
            SLOT->AMmask = ( v & 0x80 ) ? ~0 : 0;
        }
        break;

    /* SR */
    case ( 0x70 ) :
        {
            set_sr ( SLOT, v );
        }
        break;

    /* SL, RR */
    case ( 0x80 ) :
        {
            set_sl_rr ( SLOT, v );
        }
        break;

    /* SSG-EG */
    case ( 0x90 ) :
        {
            SLOT->ssg = v & 0x0f;
            SLOT->ssgn = ( v & 0x04 ) >> 1; /* bit 1 in ssgn = attack */

            /* SSG-EG envelope shapes :

             E AtAlH
             1 0 0 0  \\\\

            1 0 0 1  \___

             1 0 1 0  \/\/
             ___
             1 0 1 1
             1 1 0 0  ////
             ___
             1 1 0 1  /

             1 1 1 0  /\/
             1 1 1 1  /___

             E = SSG-EG enable

             The shapes are generated using Attack, Decay and Sustain phases.

             Each single character in the diagrams above represents this whole
             sequence:

             - when KEY-ON = 1, normal Attack phase is generated (*without* any
             difference when compared to normal mode),

             - later, when envelope level reaches minimum level (max volume),
             the EG switches to Decay phase (which works with bigger steps
             when compared to normal mode - see below),

             - later when envelope level passes the SL level,
             the EG swithes to Sustain phase (which works with bigger steps
             when compared to normal mode - see below),

             - finally when envelope level reaches maximum level (min volume),
             the EG switches to Attack phase again (depends on actual waveform).

             Important is that when switch to Attack phase occurs, the phase counter
             of that operator will be zeroed-out (as in normal KEY-ON) but not always.
             (I havent found the rule for that - perhaps only when the output level is low)

             The difference (when compared to normal Envelope Generator mode) is
             that the resolution in Decay and Sustain phases is 4 times lower;
             this results in only 256 steps instead of normal 1024.
             In other words:
             when SSG-EG is disabled, the step inside of the EG is one,
             when SSG-EG is enabled, the step is four (in Decay and Sustain phases).

             Times between the level changes are the same in both modes.


             Important:
             Decay 1 Level (so called SL) is compared to actual SSG-EG output, so
             it is the same in both SSG and no-SSG modes, with this exception:

             when the SSG-EG is enabled and is generating raising levels
             (when the EG output is inverted) the SL will be found at wrong level !!!
             For example, when SL=02:
             0 -6 = -6dB in non-inverted EG output
             96-6 = -90dB in inverted EG output
             Which means that EG compares its level to SL as usual, and that the
             output is simply inverted afterall.


             The Yamaha's manuals say that AR should be set to 0x1f (max speed).
             That is not necessary, but then EG will be generating Attack phase.
             */
        }
        break;

    case ( 0xa0 ) :
        {
            switch ( OPN_SLOT ( r ) )
            {
            /* 0xa0-0xa2 : FNUM1 */
            case ( 0 ) :
                {
                    Uint32 fn = ( ( ( Uint32 ) ( ( OPN->ST.fn_h ) & 7 ) ) << 8 ) + v;
                    Uint8 blk = OPN->ST.fn_h >> 3;
                    /* keyscale code */
                    CH->kcode = ( blk << 2 ) | opn_fktable[fn >> 7];
                    /* phase increment counter */
                    CH->fc = OPN->fn_table[fn * 2] >> ( 7 - blk );

                    /* store fnum in clear form for LFO PM calculations */
                    CH->block_fnum = ( blk << 11 ) | fn;

                    CH->SLOT[SLOT1].Incr = -1;
                }
                break;

            /* 0xa4-0xa6 : FNUM2,BLK */
            case ( 1 ) :
                {
                    OPN->ST.fn_h = v & 0x3f;
                }
                break;

            /* 0xa8-0xaa : 3CH FNUM1 */
            case ( 2 ) :
                {
                    if ( r < 0x100 )
                    {
                        Uint32 fn = ( ( ( Uint32 ) ( OPN->SL3.fn_h & 7 ) ) << 8 ) + v;
                        Uint8 blk = OPN->SL3.fn_h >> 3;
                        /* keyscale code */
                        OPN->SL3.kcode[c] = ( blk << 2 ) | opn_fktable[fn >> 7];
                        /* phase increment counter */
                        OPN->SL3.fc[c] = OPN->fn_table[fn * 2] >> ( 7 - blk );
                        OPN->SL3.block_fnum[c] = fn;
                        ( OPN->P_CH ) [2].SLOT[SLOT1].Incr = -1;
                    }
                }
                break;

            /* 0xac-0xae : 3CH FNUM2,BLK */
            case ( 3 ) :
                {
                    if ( r < 0x100 )
                    {
                        OPN->SL3.fn_h = v & 0x3f;
                    }
                }
                break;
            }
        }
        break;

    case ( 0xb0 ) :
        switch ( OPN_SLOT ( r ) )
        {
        /* 0xb0-0xb2 : FB,ALGO */
        case ( 0 ) :
            {
                Sint32 feedback = ( v >> 3 ) & 7;
                CH->ALGO = v & 7;
                CH->FB = feedback ? feedback + 6 : 0;
                setup_connection ( CH, c );
            }
            break;

        /* 0xb4-0xb6 : L , R , AMS , PMS (YM2612/YM2610B/YM2610/YM2608) */
        case ( 1 ) :
            {
                /* b0-2 PMS */
                /* CH->pms = PM depth * 32 (index in lfo_pm_table) */
                CH->pms = ( v & 7 ) * 32;

                /* b4-5 AMS */
                CH->ams = lfo_ams_depth_shift[ ( v >> 4 ) & 0x03];

                /* PAN :  b7 = L, b6 = R */
                OPN->pan[ c * 2 ] = ( v & 0x80 ) ? ~0 : 0;
                OPN->pan[ c * 2 + 1 ] = ( v & 0x40 ) ? ~0 : 0;
            }
            break;
        }

        break;
    }
}

/*********************************************************************************************/

/* SSG */

static void SSGWriteReg ( Sint32 r, Sint32 v )
{
    Sint32 old = 0;

    YM2610.regs[r] = v;

    switch ( r )
    {
    case ( 0x00 ) :
    case ( 0x02 ) :

    /* Channel A/B/C Fine Tune */
    case ( 0x04 ) :
    case ( 0x01 ) :
    case ( 0x03 ) :

    /* Channel A/B/C Coarse */
    case ( 0x05 ) :
        {
            Sint32 ch = r >> 1;

            r &= ~1;
            YM2610.regs[r + 1] &= 0x0f;
            old = SSG.period[ch];
            SSG.period[ch] = ( YM2610.regs[r] + 256 * YM2610.regs[r + 1] ) * SSG.step;

            if ( SSG.period[ch] == 0 )
            {
                SSG.period[ch] = SSG.step;
            }

            SSG.count[ch] += SSG.period[ch] - old;

            if ( SSG.count[ch] <= 0 )
            {
                SSG.count[ch] = 1;
            }
        }
        break;

    /* Noise percent */
    case ( 0x06 ) :
        {
            YM2610.regs[SSG_NOISEPER] &= 0x1f;
            old = SSG.PeriodN;
            SSG.PeriodN = YM2610.regs[SSG_NOISEPER] * SSG.step;

            if ( SSG.PeriodN == 0 )
            {
                SSG.PeriodN = SSG.step;
            }

            SSG.CountN += SSG.PeriodN - old;

            if ( SSG.CountN <= 0 )
            {
                SSG.CountN = 1;
            }
        }
        break;

    /* Enable */
    case ( 0x07 ) :
        {
            SSG.lastEnable = YM2610.regs[SSG_ENABLE];
        }
        break;

    case ( 0x08 ) :
    case ( 0x09 ) :

    /* Channel A/B/C Volume */
    case ( 0x0a ) :
        {
            Sint32 ch = r & 3;

            YM2610.regs[r] &= 0x1f;
            SSG.envelope[ch] = YM2610.regs[r] & 0x10;
            SSG.vol[ch] =
                SSG.envelope[ch] ?
                SSG.VolE :
                SSG.vol_table[
                    YM2610.regs[r] ? YM2610.regs[r] * 2 + 1 : 0];
        }
        break;

    /* Envelope Fine */
    case ( SSG_EFINE ) :

    /* Envelope Coarse */
    case ( SSG_ECOARSE ) :
        {
            old = SSG.PeriodE;
            SSG.PeriodE = ( YM2610.regs[SSG_EFINE] + 256 * YM2610.regs[SSG_ECOARSE] )
                          * SSG.step;

            if ( SSG.PeriodE == 0 )
            {
                SSG.PeriodE = SSG.step / 2;
            }

            SSG.CountE += SSG.PeriodE - old;

            if ( SSG.CountE <= 0 )
            {
                SSG.CountE = 1;
            }
        }
        break;

    /* Envelope Shapes */
    case ( SSG_ESHAPE ) :
        {
            YM2610.regs[SSG_ESHAPE] &= 0x0f;
            SSG.attack = ( YM2610.regs[SSG_ESHAPE] & 0x04 ) ? 0x1f : 0x00;

            if ( ( YM2610.regs[SSG_ESHAPE] & 0x08 ) == 0 )
            {
                /* if Continue = 0, map the shape to the equivalent one which has Continue = 1 */
                SSG.hold = 1;
                SSG.alternate = SSG.attack;
            }

            else
            {
                SSG.hold = YM2610.regs[SSG_ESHAPE] & 0x01;
                SSG.alternate = YM2610.regs[SSG_ESHAPE] & 0x02;
            }

            SSG.CountE = SSG.PeriodE;
            SSG.count_env = 0x1f;
            SSG.holding = 0;
            SSG.VolE = SSG.vol_table[SSG.count_env ^ SSG.attack];

            if ( SSG.envelope[0] )
            {
                SSG.vol[0] = SSG.VolE;
            }

            if ( SSG.envelope[1] )
            {
                SSG.vol[1] = SSG.VolE;
            }

            if ( SSG.envelope[2] )
            {
                SSG.vol[2] = SSG.VolE;
            }
        }
        break;

    /* @todo (Tmesys#1#12/06/2022): Needs some implementation ? */
    case ( SSG_PORTA ) : // Port A
    case ( SSG_PORTB ) : // Port B
        break;
    }
}

static Sint32 SSG_calc_count ( Sint32 length )
{
    /* calc SSG count */
    for ( Sint32 i = 0; i < 3; i++ )
    {
        if ( YM2610.regs[SSG_ENABLE] & ( 0x01 << i ) )
        {
            if ( SSG.count[i] <= length * SSG_STEP )
            {
                SSG.count[i] += length * SSG_STEP;
            }

            SSG.output[i] = 1;
        }

        else if ( YM2610.regs[0x08 + i] == 0 )
        {
            if ( SSG.count[i] <= length * SSG_STEP )
            {
                SSG.count[i] += length * SSG_STEP;
            }
        }
    }

    /* for the noise channel we must not touch OutputN - it's also not necessary */
    /* since we use outn. */
    if ( ( YM2610.regs[SSG_ENABLE] & 0x38 ) == 0x38 ) /* all off */
    {
        if ( SSG.CountN <= length * SSG_STEP )
        {
            SSG.CountN += length * SSG_STEP;
        }
    }

    return ( SSG.OutputN | YM2610.regs[SSG_ENABLE] );
}

static Sint32 SSG_CALC ( Sint32 outn )
{
    Sint32 ch = 0;
    Sint32 vol[3] = {0, 0, 0};
    Sint32 left = 0;

    /* vola, volb and volc keep track of how long each square wave stays */
    /* in the 1 position during the sample period. */
    vol[0] = vol[1] = vol[2] = 0;

    left = SSG_STEP;

    do
    {
        Sint32 nextevent = 0;

        nextevent = ( SSG.CountN < left ) ? SSG.CountN : left;

        for ( ch = 0; ch < 3; ch++ )
        {
            if ( outn & ( 0x08 << ch ) )
            {
                if ( SSG.output[ch] )
                {
                    vol[ch] += SSG.count[ch];
                }

                SSG.count[ch] -= nextevent;

                while ( SSG.count[ch] <= 0 )
                {
                    SSG.count[ch] += SSG.period[ch];

                    if ( SSG.count[ch] > 0 )
                    {
                        SSG.output[ch] ^= 1;

                        if ( SSG.output[ch] )
                        {
                            vol[ch] += SSG.period[ch];
                        }

                        break;
                    }

                    SSG.count[ch] += SSG.period[ch];
                    vol[ch] += SSG.period[ch];
                }

                if ( SSG.output[ch] )
                {
                    vol[ch] -= SSG.count[ch];
                }
            }

            else
            {
                SSG.count[ch] -= nextevent;

                while ( SSG.count[ch] <= 0 )
                {
                    SSG.count[ch] += SSG.period[ch];

                    if ( SSG.count[ch] > 0 )
                    {
                        SSG.output[ch] ^= 1;
                        break;
                    }

                    SSG.count[ch] += SSG.period[ch];
                }
            }
        }

        SSG.CountN -= nextevent;

        if ( SSG.CountN <= 0 )
        {
            /* Is noise output going to change? */
            if ( ( SSG.RNG + 1 ) & 2 ) /* (bit0^bit1)? */
            {
                SSG.OutputN = ~SSG.OutputN;
                outn = ( SSG.OutputN | YM2610.regs[SSG_ENABLE] );
            }

            if ( SSG.RNG & 1 )
            {
                SSG.RNG ^= 0x24000;
            }

            SSG.RNG >>= 1;
            SSG.CountN += SSG.PeriodN;
        }

        left -= nextevent;
    }

    while ( left > 0 );

    /* update envelope */
    if ( SSG.holding == 0 )
    {
        SSG.CountE -= SSG_STEP;

        if ( SSG.CountE <= 0 )
        {
            do
            {
                SSG.count_env--;
                SSG.CountE += SSG.PeriodE;
            }
            while ( SSG.CountE <= 0 );

            /* check envelope current position */
            if ( SSG.count_env < 0 )
            {
                if ( SSG.hold )
                {
                    if ( SSG.alternate )
                    {
                        SSG.attack ^= 0x1f;
                    }

                    SSG.holding = 1;
                    SSG.count_env = 0;
                }

                else
                {
                    /* if count_env has looped an odd number of times (usually 1), */
                    /* invert the output. */
                    if ( SSG.alternate && ( SSG.count_env & 0x20 ) )
                    {
                        SSG.attack ^= 0x1f;
                    }

                    SSG.count_env &= 0x1f;
                }
            }

            SSG.VolE = SSG.vol_table[SSG.count_env ^ SSG.attack];

            /* reload volume */
            if ( SSG.envelope[0] )
            {
                SSG.vol[0] = SSG.VolE;
            }

            if ( SSG.envelope[1] )
            {
                SSG.vol[1] = SSG.VolE;
            }

            if ( SSG.envelope[2] )
            {
                SSG.vol[2] = SSG.VolE;
            }
        }
    }

    out_ssg = ( ( ( vol[0] * SSG.vol[0] ) + ( vol[1] * SSG.vol[1] )
                  + ( vol[2] * SSG.vol[2] ) ) / SSG_STEP ) / 3;

    return ( outn );
}

static void SSG_init_table ( void )
{
    double out = 0;

    /* calculate the volume->voltage conversion table */
    /* The AY-3-8910 has 16 levels, in a logarithmic scale (3dB per step) */
    /* The YM2149 still has 16 levels for the tone generators, but 32 for */
    /* the envelope generator (1.5dB per step). */
    out = SSG_MAX_OUTPUT;

    for ( Sint32 i = 31; i > 0; i-- )
    {
        SSG.vol_table[i] = out + 0.5; /* round to nearest */

        out /= 1.188502227; /* = 10 ^ (1.5/20) = 1.5dB */
    }

    SSG.vol_table[0] = 0;
}

static void SSG_reset ( void )
{
    SSG.RNG = 1;
    SSG.output[0] = 0;
    SSG.output[1] = 0;
    SSG.output[2] = 0;
    SSG.OutputN = 0xff;
    SSG.lastEnable = -1;

    for ( Sint32 i = 0; i < SSG_PORTA; i++ )
    {
        YM2610.regs[i] = 0x00;
        SSGWriteReg ( i, 0x00 );
    }
}

static void SSG_write ( Sint32 r, Sint32 v )
{
    SSGWriteReg ( r, v );
}

/*********************************************************************************************/

static void OPNB_ADPCMA_init_table ( void )
{
    for ( Sint32 step = 0; step < 49; step++ )
    {
        /* loop over all nibbles and compute the difference */
        for ( Sint32 nib = 0; nib < 16; nib++ )
        {
            Sint32 value = ( 2 * ( nib & 0x07 ) + 1 ) * steps[step] / 8;
            jedi_table[step * 16 + nib] = ( nib & 0x08 ) ? -value : value;
        }
    }

}

/* ADPCM A (Non control type) : calculate one channel output */
static void OPNB_ADPCMA_calc_chan ( ADPCMA* ch )
{
    Uint32 step = 0;
    Uint8 data = 0;

    ch->now_step += ch->step;

    if ( ch->now_step >= ( 1 << ADPCM_SHIFT ) )
    {
        step = ch->now_step >> ADPCM_SHIFT;
        ch->now_step &= ( 1 << ADPCM_SHIFT ) - 1;

        do
        {
            /* end check */
            /* 11-06-2001 JB: corrected comparison. Was > instead of == */
            /* YM2610 checks lower 20 bits only, the 4 MSB bits are sample bank */
            /* Here we use 1<<21 to compensate for nibble calculations */

            if ( ( ch->now_addr & ( ( 1 << 21 ) - 1 ) )
                    == ( ( ch->end << 1 ) & ( ( 1 << 21 ) - 1 ) ) )
            {
                ch->flag = 0;
                YM2610.adpcm_arrivedEndAddress |= ch->flagMask;
                return;
            }

            if ( ch->now_addr & 1 )
            {
                data = ch->now_data & 0x0f;
            }
            else
            {
                ch->now_data = * ( pcmbufA + ( ch->now_addr >> 1 ) );
                data = ( ch->now_data >> 4 ) & 0x0f;
            }

            ch->now_addr++;

            ch->adpcma_acc += jedi_table[ch->adpcma_step + data];
            /* extend 12-bit Sint32 */

            if ( ch->adpcma_acc & 0x800 )
            {
                ch->adpcma_acc |= ~0xfff;
            }
            else
            {
                ch->adpcma_acc &= 0xfff;
            }

            ch->adpcma_step += step_inc[data & 7];
            Limit ( ch->adpcma_step, 48 * 16, 0 * 16 );

        }
        while ( --step );

        /* calc pcm * volume data */
        /* multiply, shift and mask out 2 LSB bits */
        ch->adpcma_out = ( ( ( Sint16 ) ch->adpcma_acc * ch->vol_mul ) >> ch->vol_shift ) & ~3;
    }

    /* output for work of output channels (out_adpcma[OPNxxxx]) */
    *ch->pan += ch->adpcma_out;
}

/* ADPCM type A Write */
static void OPNB_ADPCMA_write ( Sint32 r, Sint32 v )
{
    ADPCMA* adpcma = YM2610.adpcma;
    Uint8 c = r & 0x07;

    YM2610.regs[r] = v & 0xff; /* stock data */

    switch ( r )
    {
    /* DM,--,C5,C4,C3,C2,C1,C0 */
    case ( 0x100 ) :
        {
            if ( ! ( v & 0x80 ) )
            {
                /* KEY ON */
                for ( c = 0; c < 6; c++ )
                {
                    if ( ( v >> c ) & 1 )
                    {
                        /**** start adpcm ****/
                        adpcma[c].step = ( Uint32 ) ( ( float ) ( 1 << ADPCM_SHIFT )
                                                      * ( ( float ) YM2610.OPN.ST.freqbase ) / 3.0 );
                        adpcma[c].now_addr = adpcma[c].start << 1;
                        adpcma[c].now_step = 0;
                        adpcma[c].adpcma_acc = 0;
                        adpcma[c].adpcma_step = 0;
                        adpcma[c].adpcma_out = 0;
                        adpcma[c].flag = 1;

                        if ( pcmbufA == NULL )
                        {
                            /* Check ROM Mapped */
//                      logerror("YM2610: ADPCM-A rom not mapped\n");
                            adpcma[c].flag = 0;
                        }

                        else
                        {
                            if ( adpcma[c].end >= pcmsizeA )
                            {
                                /* Check End in Range */
//                          logerror("YM2610: ADPCM-A end out of range: $%08x\n", adpcma[c].end);
                                /* adpcma[c].end = pcmsizeA - 1; *//* JB: DO NOT uncomment this, otherwise you will break the comparison in the ADPCM_CALC_CHA() */
                            }

                            if ( adpcma[c].start >= pcmsizeA ) /* Check Start in Range */
                            {
//                          logerror("YM2610: ADPCM-A start out of range: $%08x\n", adpcma[c].start);
                                adpcma[c].flag = 0;
                            }
                        }
                    }
                }
            }
            else
            {
                /* KEY OFF */
                for ( c = 0; c < 6; c++ )
                    if ( ( v >> c ) & 1 )
                    {
                        adpcma[c].flag = 0;
                    }
            }
        }
        break;

    /* B0-5 = TL */
    case ( 0x101 ) :
        {
            YM2610.adpcmaTL = ( v & 0x3f ) ^ 0x3f;

            for ( c = 0; c < 6; c++ )
            {
                Sint32 volume = YM2610.adpcmaTL + adpcma[c].IL;

                if ( volume >= 63 ) /* This is correct, 63 = quiet */
                {
                    adpcma[c].vol_mul = 0;
                    adpcma[c].vol_shift = 0;
                }

                else
                {
                    adpcma[c].vol_mul = 15 - ( volume & 7 ); /* so called 0.75 dB */
                    adpcma[c].vol_shift = 1 + ( volume >> 3 ); /* Yamaha engineers used the approximation: each -6 dB is close to divide by two (shift right) */
                }

                /* calc pcm * volume data */
                adpcma[c].adpcma_out = ( ( adpcma[c].adpcma_acc * adpcma[c].vol_mul )
                                         >> adpcma[c].vol_shift ) & ~3; /* multiply, shift and mask out low 2 bits */
            }
        }
        break;

    default:
        {
            c = r & 0x07;

            if ( c >= 0x06 )
            {
                return;
            }

            switch ( r & 0x138 )
            {
            /* B7=L,B6=R,B4-0=IL */
            case ( 0x108 ) :
                {
                    Sint32 volume;

                    adpcma[c].IL = ( v & 0x1f ) ^ 0x1f;

                    volume = YM2610.adpcmaTL + adpcma[c].IL;

                    if ( volume >= 63 ) /* This is correct, 63 = quiet */
                    {
                        adpcma[c].vol_mul = 0;
                        adpcma[c].vol_shift = 0;
                    }

                    else
                    {
                        adpcma[c].vol_mul = 15 - ( volume & 7 ); /* so called 0.75 dB */
                        adpcma[c].vol_shift = 1 + ( volume >> 3 ); /* Yamaha engineers used the approximation: each -6 dB is close to divide by two (shift right) */
                    }

                    adpcma[c].pan = &out_adpcma[ ( v >> 6 ) & 0x03];

                    /* calc pcm * volume data */
                    adpcma[c].adpcma_out = ( ( adpcma[c].adpcma_acc * adpcma[c].vol_mul )
                                             >> adpcma[c].vol_shift ) & ~3; /* multiply, shift and mask out low 2 bits */
                }
                break;

            case ( 0x110 ) :
            case ( 0x118 ) :
                {
                    adpcma[c].start = ( ( YM2610.regs[0x118 + c] << 8 ) | YM2610.regs[0x110 + c] ) << ADPCMA_ADDRESS_SHIFT;
                }
                break;

            case ( 0x120 ) :
            case ( 0x128 ) :
                {
                    adpcma[c].end = ( ( YM2610.regs[0x128 + c] << 8 ) | YM2610.regs[0x120 + c] ) << ADPCMA_ADDRESS_SHIFT;
                    adpcma[c].end += ( 1 << ADPCMA_ADDRESS_SHIFT ) - 1;
                }
                break;
            }
        }
        break;
    }
}

/*********************************************************************************************/

/* DELTA-T-ADPCM write register */
static void OPNB_ADPCMB_write ( ADPCMB* adpcmb, Sint32 r, Sint32 v )
{
    if ( r >= 0x20 )
    {
        return;
    }

    YM2610.regs[r] = v; /* stock data */

    switch ( r )
    {
    case 0x10:
        /*
         START:
         Accessing *external* memory is started when START bit (D7) is set to "1", so
         you must set all conditions needed for recording/playback before starting.
         If you access *CPU-managed* memory, recording/playback starts after
         read/write of ADPCM data register $08.

         REC:
         0 = ADPCM synthesis (playback)
         1 = ADPCM analysis (record)

         MEMDATA:
         0 = processor (*CPU-managed*) memory (means: using register $08)
         1 = external memory (using start/end/limit registers to access memory: RAM or ROM)


         SPOFF:
         controls output pin that should disable the speaker while ADPCM analysis

         RESET and REPEAT only work with external memory.


         some examples:
         value:   START, REC, MEMDAT, REPEAT, SPOFF, x,x,RESET   meaning:
         C8     1      1    0       0       1      0 0 0       Analysis (recording) from AUDIO to CPU (to reg $08), sample rate in PRESCALER register
         E8     1      1    1       0       1      0 0 0       Analysis (recording) from AUDIO to EXT.MEMORY,       sample rate in PRESCALER register
         80     1      0    0       0       0      0 0 0       Synthesis (playing) from CPU (from reg $08) to AUDIO,sample rate in DELTA-N register
         a0     1      0    1       0       0      0 0 0       Synthesis (playing) from EXT.MEMORY to AUDIO,        sample rate in DELTA-N register

         60     0      1    1       0       0      0 0 0       External memory write via ADPCM data register $08
         20     0      0    1       0       0      0 0 0       External memory read via ADPCM data register $08

         */
        v |= 0x20; /*  YM2610 always uses external memory and doesn't even have memory flag bit. */
        adpcmb->portstate = v & ( 0x80 | 0x40 | 0x20 | 0x10 | 0x01 ); /* start, rec, memory mode, repeat flag copy, reset(bit0) */

        if ( adpcmb->portstate & 0x80 ) /* START,REC,MEMDATA,REPEAT,SPOFF,--,--,RESET */
        {
            /* set PCM BUSY bit */
            adpcmb->PCM_BSY = 1;

            /* start ADPCM */
            adpcmb->now_step = 0;
            adpcmb->acc = 0;
            adpcmb->prev_acc = 0;
            adpcmb->adpcml = 0;
            adpcmb->adpcmd = ADPCMB_DELTA_DEF;
            adpcmb->now_data = 0;
        }

//      if (adpcmb->portstate & 0x20) /* do we access external memory? */
        {
            adpcmb->now_addr = adpcmb->start << 1;
            adpcmb->memread = 2; /* two dummy reads needed before accesing external memory via register $08*/

            /* if yes, then let's check if ADPCM memory is mapped and big enough */
            if ( !pcmbufB )
            {
//              logerror("YM2610: Delta-T ADPCM rom not mapped\n");
                adpcmb->portstate = 0x00;
                adpcmb->PCM_BSY = 0;
            }

            else
            {
                if ( adpcmb->end >= pcmsizeB ) /* Check End in Range */
                {
//                  logerror("YM2610: Delta-T ADPCM end out of range: $%08x\n", adpcmb->end);
                    adpcmb->end = pcmsizeB - 1;
                }

                if ( adpcmb->start >= pcmsizeB ) /* Check Start in Range */
                {
//                  logerror("YM2610: Delta-T ADPCM start out of range: $%08x\n", adpcmb->start);
                    adpcmb->portstate = 0x00;
                    adpcmb->PCM_BSY = 0;
                }
            }
        }
#if 0

        else /* we access CPU memory (ADPCM data register $08) so we only reset now_addr here */
        {
            adpcmb->now_addr = 0;
        }

#endif

        if ( adpcmb->portstate & 0x01 )
        {
            adpcmb->portstate = 0x00;

            /* clear PCM BUSY bit (in status register) */
            adpcmb->PCM_BSY = 0;

            /* set BRDY flag */
            if ( adpcmb->status_change_BRDY_bit )
                YM2610.adpcm_arrivedEndAddress |=
                    adpcmb->status_change_BRDY_bit;
        }

        break;

    case 0x11: /* L,R,-,-,SAMPLE,DA/AD,RAMTYPE,ROM */
        v |= 0x01; /*  YM2610 always uses ROM as an external memory and doesn't tave ROM/RAM memory flag bit. */
        adpcmb->pan = &out_delta[ ( v >> 6 ) & 0x03];

        if ( ( adpcmb->control2 & 3 ) != ( v & 3 ) )
        {
            /*0-DRAM x1, 1-ROM, 2-DRAM x8, 3-ROM (3 is bad setting - not allowed by the manual) */
            if ( adpcmb->DRAMportshift != dram_rightshift[v & 3] )
            {
                adpcmb->DRAMportshift = dram_rightshift[v & 3];

                /* final shift value depends on chip type and memory type selected:
                 8 for YM2610 (ROM only),
                 5 for ROM for Y8950 and YM2608,
                 5 for x8bit DRAMs for Y8950 and YM2608,
                 2 for x1bit DRAMs for Y8950 and YM2608.
                 */

                /* refresh addresses */
                adpcmb->start = ( ( YM2610.regs[0x13] << 8 ) | YM2610.regs[0x12] )
                                << adpcmb->portshift;
                adpcmb->end = ( ( YM2610.regs[0x15] << 8 ) | YM2610.regs[0x14] )
                              << adpcmb->portshift;
                adpcmb->end += ( 1 << adpcmb->portshift ) - 1;
                adpcmb->limit = ( ( YM2610.regs[0x1d] << 8 ) | YM2610.regs[0x1c] )
                                << adpcmb->portshift;
            }
        }

        adpcmb->control2 = v;
        break;

    case 0x12: /* Start Address L */
    case 0x13: /* Start Address H */
        adpcmb->start = ( ( YM2610.regs[0x13] << 8 ) | YM2610.regs[0x12] )
                        << adpcmb->portshift;
        /*logerror("DELTAT start: 02=%2x 03=%2x addr=%8x\n", YM2610.regs[0x12], YM2610.regs[0x13], adpcmb->start);*/
        break;

    case 0x14: /* Stop Address L */
    case 0x15: /* Stop Address H */
        adpcmb->end = ( ( YM2610.regs[0x15] << 8 ) | YM2610.regs[0x14] )
                      << adpcmb->portshift;
        adpcmb->end += ( 1 << adpcmb->portshift ) - 1;
        /*logerror("DELTAT end  : 04=%2x 05=%2x addr=%8x\n", YM2610.regs[0x14], YM2610.reg[0x15], adpcmb->end);*/
        break;

    case 0x19: /* DELTA-N L (ADPCM Playback Prescaler) */
    case 0x1a: /* DELTA-N H */
        adpcmb->delta = ( YM2610.regs[0x1a] << 8 ) | YM2610.regs[0x19];
        adpcmb->step =
            ( Uint32 ) ( ( double ) ( adpcmb->delta /* * (1 << (ADPCMb_SHIFT - 16)) */ )
                         * ( adpcmb->freqbase ) );
        /*logerror("DELTAT deltan:09=%2x 0a=%2x\n", YM2610.regs[0x19], YM2610.regs[0x1a]);*/
        break;

    case 0x1b: /* Output level control (volume, linear) */
        {
            Sint32 oldvol = adpcmb->volume;
            adpcmb->volume = ( v & 0xff )
                             * ( adpcmb->output_range / 256 ) / ADPCMB_DECODE_RANGE;

//                              v     *     ((1<<16)>>8)        >>  15;
//                      thus:   v     *     (1<<8)              >>  15;
//                      thus: output_range must be (1 << (15+8)) at least
//                              v     *     ((1<<23)>>8)        >>  15;
//                              v     *     (1<<15)             >>  15;
            /*logerror("DELTAT vol = %2x\n", v & 0xff);*/
            if ( oldvol != 0 )
            {
                adpcmb->adpcml = ( Sint32 ) ( ( double ) adpcmb->adpcml / ( double ) oldvol
                                              * ( double ) adpcmb->volume );
            }
        }
        break;
    }
}

static void OPNB_ADPCMB_CALC ( ADPCMB* adpcmb )
{
    Uint32 step;
    Sint32 data;

    adpcmb->now_step += adpcmb->step;

    if ( adpcmb->now_step >= ( 1 << ADPCM_SHIFT ) )
    {
        step = adpcmb->now_step >> ADPCM_SHIFT;
        adpcmb->now_step &= ( 1 << ADPCM_SHIFT ) - 1;

        do
        {
            if ( adpcmb->now_addr == ( adpcmb->limit << 1 ) )
            {
                adpcmb->now_addr = 0;
            }

            if ( adpcmb->now_addr == ( adpcmb->end << 1 ) )
            {
                /* 12-06-2001 JB: corrected comparison. Was > instead of == */
                if ( adpcmb->portstate & 0x10 )
                {
                    /* repeat start */
                    adpcmb->now_addr = adpcmb->start << 1;
                    adpcmb->acc = 0;
                    adpcmb->adpcmd = ADPCMB_DELTA_DEF;
                    adpcmb->prev_acc = 0;
                }

                else
                {
                    /* set EOS bit in status register */
                    if ( adpcmb->status_change_EOS_bit )
                        YM2610.adpcm_arrivedEndAddress |=
                            adpcmb->status_change_EOS_bit;

                    /* clear PCM BUSY bit (reflected in status register) */
                    adpcmb->PCM_BSY = 0;

                    adpcmb->portstate = 0;
                    adpcmb->adpcml = 0;
                    adpcmb->prev_acc = 0;
                    return;
                }
            }

            if ( adpcmb->now_addr & 1 )
            {
                data = adpcmb->now_data & 0x0f;
            }

            else
            {
                adpcmb->now_data = * ( pcmbufB + ( adpcmb->now_addr >> 1 ) );
                data = adpcmb->now_data >> 4;
            }

            adpcmb->now_addr++;
            /* 12-06-2001 JB: */
            /* YM2610 address register is 24 bits wide.*/
            /* The "+1" is there because we use 1 bit more for nibble calculations.*/
            /* WARNING: */
            /* Side effect: we should take the size of the mapped ROM into account */
            adpcmb->now_addr &= ( ( 1 << ( 24 + 1 ) ) - 1 );

            /* store accumulator value */
            adpcmb->prev_acc = adpcmb->acc;

            /* Forecast to next Forecast */
            adpcmb->acc += ( adpcmb_decode_table1[data] * adpcmb->adpcmd / 8 );
            Limit ( adpcmb->acc, ADPCMB_DECODE_MAX, ADPCMB_DECODE_MIN );

            /* delta to next delta */
            adpcmb->adpcmd = ( adpcmb->adpcmd * adpcmb_decode_table2[data] ) / 64;
            Limit ( adpcmb->adpcmd, ADPCMB_DELTA_MAX, ADPCMB_DELTA_MIN );

            /* ElSemi: Fix interpolator. */
            /*adpcmb->prev_acc = prev_acc + ((adpcmb->acc - prev_acc) / 2);*/

        }
        while ( --step );

    }

    /* ElSemi: Fix interpolator. */
#if 1
    adpcmb->adpcml = adpcmb->prev_acc * ( Sint32 ) ( ( 1 << ADPCM_SHIFT ) - adpcmb->now_step );
    adpcmb->adpcml += ( adpcmb->acc * ( Sint32 ) adpcmb->now_step );
    adpcmb->adpcml = ( adpcmb->adpcml >> ADPCM_SHIFT ) * ( Sint32 ) adpcmb->volume;
#else
    adpcmb->adpcml = ( ( adpcmb->acc * ( Sint32 ) adpcmb->now_step ) >> ADPCM_SHIFT ) * ( Sint32 ) adpcmb->volume;;
#endif

    /* output for work of output channels (outd[OPNxxxx])*/
    *adpcmb->pan += adpcmb->adpcml;
}

/*********************************************************************************************/

/* YM2610(OPNB) */
void YM2610Init ( Sint32 clock, Sint32 rate, void* pcmroma, Sint32 pcmsizea, void* pcmromb,
                  Sint32 pcmsizeb, FM_TIMERHANDLER TimerHandler, FM_IRQHANDLER IRQHandler )
{
    /*
     sound->stack    = 0x10000;
     sound->stereo   = 1;
     sound->callback = YM2610Update;
     */

    /* clear */
    memset ( &YM2610, 0, sizeof ( YM2610 ) );
    memset ( &SSG, 0, sizeof ( SSG ) );

    OPNInitTable();
    SSG_init_table();
    OPNB_ADPCMA_init_table();

    /* FM */
    YM2610.OPN.P_CH = YM2610.CH;
    YM2610.OPN.ST.clock = clock;
    YM2610.OPN.ST.rate = rate;
    /* Extend handler */
    YM2610.OPN.ST.Timer_Handler = TimerHandler;
    YM2610.OPN.ST.IRQ_Handler = IRQHandler;
    sav_TimerHandler = TimerHandler;
    sav_IRQHandler = IRQHandler;
    /* SSG */
    SSG.step = ( ( double ) SSG_STEP * rate * 8 ) / clock;
    /* ADPCM-A */
    pcmbufA = ( Uint8* ) pcmroma;
    pcmsizeA = pcmsizea;
    /* ADPCM-B */
    pcmbufB = ( Uint8* ) pcmromb;
    pcmsizeB = pcmsizeb;

    YM2610.adpcmb.status_change_EOS_bit = 0x80; /* status flag: set bit7 on End Of Sample */

    YM2610Reset();
}

void YM2610ChangeSamplerate ( Sint32 rate )
{
    YM2610.OPN.ST.rate = rate;
    SSG.step = ( ( double ) SSG_STEP * rate * 8 ) / YM2610.OPN.ST.clock;
    OPNSetPres ( &YM2610.OPN, 6 * 24, 6 * 24, 4 * 2 ); /* OPN 1/6, SSG 1/4 */

    for ( Sint32 i = 0; i < 6; i++ )
    {
        YM2610.adpcma[i].step = ( Uint32 ) ( ( float ) ( 1 << ADPCM_SHIFT ) * ( ( float ) YM2610.OPN.ST.freqbase ) / 3.0 );
    }

    YM2610.adpcmb.freqbase = YM2610.OPN.ST.freqbase;
}

/* reset one of chip */
void YM2610Reset ( void )
{
    FM_OPN* OPN = &YM2610.OPN;

    /* Reset Prescaler */
    OPNSetPres ( OPN, 6 * 24, 6 * 24, 4 * 2 ); /* OPN 1/6, SSG 1/4 */
    /* reset SSG section */
    SSG_reset();
    /* status clear */
    FM_IRQMASK_SET ( &OPN->ST, 0x03 );
    FM_BUSY_CLEAR ( &OPN->ST );
    OPNWriteMode ( OPN, 0x27, 0x30 ); /* mode 0, timer reset */

    OPN->eg_timer = 0;
    OPN->eg_cnt = 0;

    FM_STATUS_RESET ( &OPN->ST, 0xff );

    reset_channels ( &OPN->ST, YM2610.CH, 6 );

    /* reset OPerator paramater */
    for ( Sint32 i = 0xb6; i >= 0xb4; i-- )
    {
        OPNWriteReg ( OPN, i, 0xc0 );
        OPNWriteReg ( OPN, i | 0x100, 0xc0 );
    }

    for ( Sint32 i = 0xb2; i >= 0x30; i-- )
    {
        OPNWriteReg ( OPN, i, 0x00 );
        OPNWriteReg ( OPN, i | 0x100, 0x00 );
    }

    for ( Sint32 i = 0x26; i >= 0x20; i-- )
    {
        OPNWriteReg ( OPN, i, 0x00 );
    }

    /**** ADPCM work initial ****/
    for ( Sint32 i = 0; i < 6; i++ )
    {
        YM2610.adpcma[i].step = ( Uint32 ) ( ( float ) ( 1 << ADPCM_SHIFT ) * ( ( float ) YM2610.OPN.ST.freqbase ) / 3.0 );
        YM2610.adpcma[i].now_addr = 0;
        YM2610.adpcma[i].now_step = 0;
        YM2610.adpcma[i].start = 0;
        YM2610.adpcma[i].end = 0;
        YM2610.adpcma[i].vol_mul = 0;
        YM2610.adpcma[i].pan = &out_adpcma[OUTD_CENTER]; /* default center */
        YM2610.adpcma[i].flagMask = 1 << i;
        YM2610.adpcma[i].flag = 0;
        YM2610.adpcma[i].adpcma_acc = 0;
        YM2610.adpcma[i].adpcma_step = 0;
        YM2610.adpcma[i].adpcma_out = 0;
    }

    YM2610.adpcmaTL = 0x3f;

    YM2610.adpcm_arrivedEndAddress = 0;

    /* ADPCM-B unit */
    YM2610.adpcmb.freqbase = OPN->ST.freqbase;
    YM2610.adpcmb.portshift = 8; /* allways 8bits shift */
    YM2610.adpcmb.output_range = 1 << 23;

    YM2610.adpcmb.now_addr = 0;
    YM2610.adpcmb.now_step = 0;
    YM2610.adpcmb.step = 0;
    YM2610.adpcmb.start = 0;
    YM2610.adpcmb.end = 0;
    YM2610.adpcmb.limit = ~0; /* this way YM2610 and Y8950 (both of which don't have limit address reg) will still work */
    YM2610.adpcmb.volume = 0;
    YM2610.adpcmb.pan = &out_delta[OUTD_CENTER];
    YM2610.adpcmb.acc = 0;
    YM2610.adpcmb.prev_acc = 0;
    YM2610.adpcmb.adpcmd = 127;
    YM2610.adpcmb.adpcml = 0;
    YM2610.adpcmb.portstate = 0x20;
    YM2610.adpcmb.control2 = 0x01;

    /* The flag mask register disables the BRDY after the reset, however
     ** as soon as the mask is enabled the flag needs to be set. */

    /* set BRDY bit in status register */
    if ( YM2610.adpcmb.status_change_BRDY_bit )
    {
        YM2610.adpcm_arrivedEndAddress |= YM2610.adpcmb.status_change_BRDY_bit;
    }
}

/* YM2610 write */
/* a = address */
/* v = value   */
Sint32 YM2610Write ( Sint32 a, Uint8 v )
{
    FM_OPN* OPN = &YM2610.OPN;
    Sint32 addr = 0;
    Sint32 ch = 0;

    v &= 0xff; /* adjust to 8 bit bus */

    switch ( a & 3 )
    {
    /* address port 0 */
    case ( 0 ) :
        {
            OPN->ST.address = v;
            YM2610.addr_A1 = 0;

            /* Write register to SSG emulator */
            /* if (v < 16) SSG_write(0, v); */
        }
        break;
    /* data port 0    */
    case ( 1 ) :
        {
            if ( YM2610.addr_A1 != 0 )
            {
                break;    /* verified on real YM2608 */
            }

            addr = OPN->ST.address;
            YM2610.regs[addr] = v;

            switch ( addr & 0xf0 )
            {
            /* SSG section */
            case ( 0x00 ) :
                {
                    /* Write data to SSG emulator */
                    SSG_write ( addr, v );
                }
                break;
            /* DeltaT ADPCM */
            case ( 0x10 ) :
                {
                    switch ( addr )
                    {
                    /* control 1 */
                    case ( 0x10 ) :

                    /* control 2 */
                    case ( 0x11 ) :

                    /* start address L */
                    case ( 0x12 ) :

                    /* start address H */
                    case ( 0x13 ) :

                    /* stop address L */
                    case ( 0x14 ) :

                    /* stop address H */
                    case ( 0x15 ) :

                    /* delta-n L */
                    case ( 0x19 ) :

                    /* delta-n H */
                    case ( 0x1a ) :

                    /* volume */
                    case ( 0x1b ) :
                        {
                            OPNB_ADPCMB_write ( &YM2610.adpcmb, addr, v );
                        }
                        break;

                    /*  FLAG CONTROL : Extend Status Clear/Mask */
                    case ( 0x1c ) :
                        {
                            Uint8 statusmask = ~v;

                            /* set arrived flag mask */
                            for ( ch = 0; ch < 6; ch++ )
                            {
                                YM2610.adpcma[ch].flagMask = statusmask & ( 1 << ch );
                            }

                            /* status flag: set bit7 on End Of Sample */
                            YM2610.adpcmb.status_change_EOS_bit = statusmask & 0x80;

                            /* clear arrived flag */
                            YM2610.adpcm_arrivedEndAddress &= statusmask;
                        }
                        break;
                    }
                }
                break;
            /* Mode Register */
            case ( 0x20 ) :
                {
                    OPNWriteMode ( OPN, addr, v );
                }
                break;
            /* OPN section */
            default:
                {
                    /* write register */
                    OPNWriteReg ( OPN, addr, v );
                }
                break;
            }
        }
        break;
    /* address port 1 */
    case ( 2 ) :
        {
            OPN->ST.address = v;
            YM2610.addr_A1 = 1;
        }
        break;
    /* data port 1    */
    case ( 3 ) :
        {
            if ( YM2610.addr_A1 != 1 )
            {
                /* verified on real YM2608 */
                break;
            }

            addr = YM2610.OPN.ST.address | 0x100;
            YM2610.regs[addr | 0x100] = v;

            /* 100-12f : ADPCM A section */
            if ( addr < 0x130 )
            {
                OPNB_ADPCMA_write ( addr, v );
            }
            else
            {
                OPNWriteReg ( OPN, addr, v );
            }
        }
        break;
    }

    return ( OPN->ST.irq );
}

Uint8 YM2610Read ( Sint32 a )
{
    Sint32 addr = YM2610.OPN.ST.address;

    switch ( a & 3 )
    {
    /* status 0 : YM2203 compatible */
    case ( 0 ) :
        {
            return ( FM_STATUS_FLAG ( &YM2610.OPN.ST ) & 0x83 );
        }
        break;

    /* data 0 */
    case ( 1 ) :
        {
            if ( addr < SSG_PORTA )
            {
                return ( YM2610.regs[addr] );
            }

            if ( addr == 0xff )
            {
                return ( 0x01 );
            }
        }
        break;

    /* status 1 : ADPCM status */
    case ( 2 ) :
        {
            /* ADPCM STATUS (arrived End Address) */
            /* B, --, A5, A4, A3, A2, A1, A0 */
            /* B     = ADPCM-B(DELTA-T) arrived end address */
            /* A0-A5 = ADPCM-A          arrived end address */
            return ( YM2610.adpcm_arrivedEndAddress );
        }
        break;
    }

    return ( 0 );
}

Sint32 YM2610TimerOver ( Sint32 ch )
{
    FM_ST* ST = &YM2610.OPN.ST;

    if ( ch )
    {
        /* Timer B */
        TimerBOver ( ST );
    }
    else
    {
        /* timer A update */
        TimerAOver ( ST );

        /* CSM mode key, TL controll */
        if ( ST->mode & 0x80 )
        {
            /* CSM mode total level latch and auto key on */
            CSMKeyControll ( &YM2610.CH[2] );
        }
    }

    return ( ST->irq );
}

/* Generate samples for one of the YM2610s */
void YM2610Update_stream ( Sint32 length, Uint16* buffer )
{
    FM_OPN* OPN = &YM2610.OPN;
    Sint32 outn = 0;
    Sint32 lt = 0, rt = 0;
    FM_CH* cch[6];

    cch[0] = &YM2610.CH[1];
    cch[1] = &YM2610.CH[2];
    cch[2] = &YM2610.CH[4];
    cch[3] = &YM2610.CH[5];

    /* update frequency counter */
    refresh_fc_eg_chan ( cch[0] );

    if ( OPN->ST.mode & 0xc0 )
    {
        /* 3SLOT MODE */
        if ( cch[1]->SLOT[SLOT1].Incr == -1 )
        {
            /* 3 slot mode */
            refresh_fc_eg_slot ( &cch[1]->SLOT[SLOT1], OPN->SL3.fc[1], OPN->SL3.kcode[1] );
            refresh_fc_eg_slot ( &cch[1]->SLOT[SLOT2], OPN->SL3.fc[2], OPN->SL3.kcode[2] );
            refresh_fc_eg_slot ( &cch[1]->SLOT[SLOT3], OPN->SL3.fc[0], OPN->SL3.kcode[0] );
            refresh_fc_eg_slot ( &cch[1]->SLOT[SLOT4], cch[1]->fc, cch[1]->kcode );
        }
    }
    else
    {
        refresh_fc_eg_chan ( cch[1] );
    }

    refresh_fc_eg_chan ( cch[2] );
    refresh_fc_eg_chan ( cch[3] );

    /* calc SSG count */
    outn = SSG_calc_count ( length );

    /* buffering */
    for ( Sint32 i = 0; i < length; i++ )
    {
        advance_lfo ( OPN );

        /* clear output acc. */
        out_adpcma[OUTD_LEFT] = out_adpcma[OUTD_RIGHT] = out_adpcma[OUTD_CENTER] = 0;
        out_delta[OUTD_LEFT] = out_delta[OUTD_RIGHT] = out_delta[OUTD_CENTER] = 0;

        /* clear outputs */
        out_fm[1] = 0;
        out_fm[2] = 0;
        out_fm[4] = 0;
        out_fm[5] = 0;

        /* clear outputs SSG */
        out_ssg = 0;

        /* advance envelope generator */
        OPN->eg_timer += OPN->eg_timer_add;

        while ( OPN->eg_timer >= OPN->eg_timer_overflow )
        {
            OPN->eg_timer -= OPN->eg_timer_overflow;
            OPN->eg_cnt++;
            advance_eg_channel ( OPN, &cch[0]->SLOT[SLOT1] );
            advance_eg_channel ( OPN, &cch[1]->SLOT[SLOT1] );
            advance_eg_channel ( OPN, &cch[2]->SLOT[SLOT1] );
            advance_eg_channel ( OPN, &cch[3]->SLOT[SLOT1] );
        }

        /* calculate FM */
        chan_calc ( OPN, cch[0] ); /*remapped to 1*/
        chan_calc ( OPN, cch[1] ); /*remapped to 2*/
        chan_calc ( OPN, cch[2] ); /*remapped to 4*/
        chan_calc ( OPN, cch[3] ); /*remapped to 5*/

        /* calculate SSG */
        outn = SSG_CALC ( outn );

        /* deltaT ADPCM */
        if ( YM2610.adpcmb.portstate & 0x80 )
        {
            OPNB_ADPCMB_CALC ( &YM2610.adpcmb );
        }

        for ( Sint32 j = 0; j < 6; j++ )
        {
            /* ADPCM */
            if ( YM2610.adpcma[j].flag )
            {
                OPNB_ADPCMA_calc_chan ( &YM2610.adpcma[j] );
            }
        }

        /* buffering */
        lt = out_adpcma[OUTD_LEFT] + out_adpcma[OUTD_CENTER];
        rt = out_adpcma[OUTD_RIGHT] + out_adpcma[OUTD_CENTER];

        lt += ( out_delta[OUTD_LEFT] + out_delta[OUTD_CENTER] ) >> 9;
        rt += ( out_delta[OUTD_RIGHT] + out_delta[OUTD_CENTER] ) >> 9;

        lt += out_ssg;
        rt += out_ssg;

        lt += ( ( out_fm[1] >> 1 ) & OPN->pan[2] ); /* the shift right was verified on real chip */
        rt += ( ( out_fm[1] >> 1 ) & OPN->pan[3] );
        lt += ( ( out_fm[2] >> 1 ) & OPN->pan[4] );
        rt += ( ( out_fm[2] >> 1 ) & OPN->pan[5] );

        lt += ( ( out_fm[4] >> 1 ) & OPN->pan[8] );
        rt += ( ( out_fm[4] >> 1 ) & OPN->pan[9] );
        lt += ( ( out_fm[5] >> 1 ) & OPN->pan[10] );
        rt += ( ( out_fm[5] >> 1 ) & OPN->pan[11] );

        lt <<= 1;
        rt <<= 1;

        Limit ( lt, MAXOUT, MINOUT );
        Limit ( rt, MAXOUT, MINOUT );

        *buffer++ = lt;
        *buffer++ = rt;

        /* my_timer(); */

        INTERNAL_TIMER_A ( OPN->ST, cch[1] );
    }

    INTERNAL_TIMER_B ( OPN->ST, length );
}

#ifdef _GNGEOX_YM2610_C_
#undef _GNGEOX_YM2610_C_
#endif // _GNGEOX_YM2610_C_
