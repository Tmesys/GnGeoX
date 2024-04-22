/*!
*
*   \file    GnGeoXym2610core.h
*   \brief   YAMAHA YM-2610 sound generator emulation routines header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.40 (final beta)
*   \date    04/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    .
*/
#ifndef _GNGEOX_YM2610_CORE_H_
#define _GNGEOX_YM2610_CORE_H_

#define SOUND_SAMPLES 512

#define Limit(val, max, min)                    \
{                                               \
        if (val > max) val = max;               \
        else if (val < min) val = min;          \
}

/* select timer system internal or external */
#define FM_INTERNAL_TIMER 0

#define FREQ_SH         16  /* 16.16 fixed point (frequency calculations) */
#define EG_SH           16  /* 16.16 fixed point (envelope generator timing) */
#define LFO_SH          24  /*  8.24 fixed point (LFO calculations)       */
#define TIMER_SH        16  /* 16.16 fixed point (timers calculations)    */

#define FREQ_MASK       ((1<<FREQ_SH)-1)

#define ENV_BITS        10
#define ENV_LEN         (1<<ENV_BITS)
#define ENV_STEP        (128.0/ENV_LEN)

#define MAX_ATT_INDEX   (ENV_LEN-1) /* 1023 */
#define MIN_ATT_INDEX   (0)         /* 0 */

#define EG_ATT          4
#define EG_DEC          3
#define EG_SUS          2
#define EG_REL          1
#define EG_OFF          0

#define SIN_BITS        10
#define SIN_LEN         (1<<SIN_BITS)
#define SIN_MASK        (SIN_LEN-1)

#define TL_RES_LEN      (256) /* 8 bits addressing (real chip) */

#define FINAL_SH    (0)
#define MAXOUT      (+32767)
#define MINOUT      (-32768)

#define TIMER_SH        16  /* 16.16 fixed point (timers calculations)    */

#define TL_TAB_LEN (13*2*TL_RES_LEN)

#define ENV_QUIET       (TL_TAB_LEN>>3)

#define RATE_STEPS (8)

/*----------------------------------
 for SSG emulator
 -----------------------------------*/

#define SSG_MAX_OUTPUT 0x7fff
#define SSG_STEP 0x8000

/* SSG register ID */
#define SSG_AFINE       (0)
#define SSG_ACOARSE     (1)
#define SSG_BFINE       (2)
#define SSG_BCOARSE     (3)
#define SSG_CFINE       (4)
#define SSG_CCOARSE     (5)
#define SSG_NOISEPER    (6)
#define SSG_ENABLE      (7)
#define SSG_AVOL        (8)
#define SSG_BVOL        (9)
#define SSG_CVOL        (10)
#define SSG_EFINE       (11)
#define SSG_ECOARSE     (12)
#define SSG_ESHAPE      (13)

#define SSG_PORTA       (14)
#define SSG_PORTB       (15)

/* register number to channel number , slot offset */
#define OPN_CHAN(N) (N&3)
#define OPN_SLOT(N) ((N>>2)&3)

/* slot number */
#define SLOT1 0
#define SLOT2 2
#define SLOT3 1
#define SLOT4 3

/* bit0 = Right enable , bit1 = Left enable */
#define OUTD_RIGHT  1
#define OUTD_LEFT   2
#define OUTD_CENTER 3

/* @todo (Tmesys#1#12/05/2022): Suppress this one ? */
#define FM_BUSY_CLEAR(ST) ((ST)->BusyExpire = 0)

#define volume_calc(OP) ((OP)->vol_out + (AM & (OP)->AMmask))

/* frequency step rate   */
#define ADPCM_SHIFT    (16)
/* adpcm A address shift */
#define ADPCMA_ADDRESS_SHIFT 8

/* DELTA-T particle adjuster */
#define ADPCMB_DELTA_MAX (24576)
#define ADPCMB_DELTA_MIN (127)
#define ADPCMB_DELTA_DEF (127)

#define ADPCMB_DECODE_RANGE 32768
#define ADPCMB_DECODE_MIN (-(ADPCMB_DECODE_RANGE))
#define ADPCMB_DECODE_MAX ((ADPCMB_DECODE_RANGE)-1)

typedef void ( *FM_TIMERHANDLER ) ( Sint32, Sint32, double );
typedef void ( *FM_IRQHANDLER ) ( Sint32 );

/* struct describing a single operator (SLOT) */
typedef struct
{
    Sint32* DT; /* detune          :dt_tab[DT] */
    Uint8 KSR; /* key scale rate  :3-KSR */
    Uint32 ar; /* attack rate  */
    Uint32 d1r; /* decay rate   */
    Uint32 d2r; /* sustain rate */
    Uint32 rr; /* release rate */
    Uint8 ksr; /* key scale rate  :kcode>>(3-KSR) */
    Uint32 mul; /* multiple        :ML_TABLE[ML] */
    /* Phase Generator */
    Uint32 phase; /* phase counter */
    Sint32 Incr; /* phase step */
    /* Envelope Generator */
    Uint8 state; /* phase type */
    Uint32 tl; /* total level: TL << 3 */
    Sint32 volume; /* envelope counter */
    Uint32 sl; /* sustain level:sl_table[SL] */
    Uint32 vol_out; /* current output from EG circuit (without AM from LFO) */
    Uint8 eg_sh_ar; /*  (attack state) */
    Uint8 eg_sel_ar; /*  (attack state) */
    Uint8 eg_sh_d1r; /*  (decay state) */
    Uint8 eg_sel_d1r; /*  (decay state) */
    Uint8 eg_sh_d2r; /*  (sustain state) */
    Uint8 eg_sel_d2r; /*  (sustain state) */
    Uint8 eg_sh_rr; /*  (release state) */
    Uint8 eg_sel_rr; /*  (release state) */
    Uint8 ssg; /* SSG-EG waveform */
    Uint8 ssgn; /* SSG-EG negated output */
    Uint32 key; /* 0=last key was KEY OFF, 1=KEY ON */
    /* LFO */
    Uint32 AMmask; /* AM enable flag */

} FM_SLOT;

typedef struct
{
    FM_SLOT SLOT[4]; /* four SLOTs (operators) */
    Uint8 ALGO; /* algorithm */
    Uint8 FB; /* feedback shift */
    Sint32 op1_out[2]; /* op1 output for feedback */
    Sint32* connect1; /* SLOT1 output pointer */
    Sint32* connect3; /* SLOT3 output pointer */
    Sint32* connect2; /* SLOT2 output pointer */
    Sint32* connect4; /* SLOT4 output pointer */
    Sint32* mem_connect;/* where to put the delayed sample (MEM) */
    Sint32 mem_value; /* delayed sample (MEM) value */
    Sint32 pms; /* channel PMS */
    Uint8 ams; /* channel AMS */
    Uint32 fc; /* fnum,blk:adjusted to sample rate */
    Uint8 kcode; /* key code:                        */
    Uint32 block_fnum; /* current blk/fnum value for this slot (can be different betweeen slots of one channel in 3slot mode) */
} FM_CH;

typedef struct
{
    Sint32 clock; /* master clock  (Hz)   */
    Sint32 rate; /* sampling rate (Hz)   */
    double freqbase; /* frequency base       */
    double TimerBase; /* Timer base time      */
    double BusyExpire; /* ExpireTime of Busy clear */
    Uint8 address; /* address register     */
    Uint8 irq; /* interrupt level      */
    Uint8 irqmask; /* irq mask             */
    Uint8 status; /* status flag          */
    Uint32 mode; /* mode  CSM / 3SLOT    */
    Uint8 prescaler_sel;/* prescaler selector */
    Uint8 fn_h; /* freq latch           */
    Sint32 TA; /* timer a              */
    Sint32 TAC; /* timer a counter      */
    Uint8 TB; /* timer b              */
    Sint32 TBC; /* timer b counter      */
    /* local time tables */
    Sint32 dt_tab[8][32];/* DeTune table       */
    /* Extension Timer and IRQ handler */
    FM_TIMERHANDLER Timer_Handler;
    FM_IRQHANDLER IRQ_Handler;
} FM_ST;

/* OPN 3slot struct */
typedef struct
{
    Uint32 fc[3]; /* fnum3,blk3: calculated */
    Uint8 fn_h; /* freq3 latch */
    Uint8 kcode[3]; /* key code */
    Uint32 block_fnum[3]; /* current fnum value for this slot (can be different betweeen slots of one channel in 3slot mode) */
} FM_3SLOT;

/* OPN/A/B common state */
typedef struct
{
    FM_ST ST; /* general state */
    FM_3SLOT SL3; /* 3 slot mode state */
    FM_CH* P_CH; /* pointer of CH */
    Uint32 pan[6 * 2]; /* fm channels output masks (0xffffffff = enable) */
    Uint32 eg_cnt; /* global envelope generator counter */
    Uint32 eg_timer; /* global envelope generator counter works at frequency = chipclock/64/3 */
    Uint32 eg_timer_add; /* step of eg_timer */
    Uint32 eg_timer_overflow;/* envelope generator timer overlfows every 3 samples (on real chip) */
    /* there are 2048 FNUMs that can be generated using FNUM/BLK registers
     but LFO works with one more bit of a precision so we really need 4096 elements */Uint32 fn_table[4096]; /* fnumber->increment counter */
    /* LFO */Uint32 lfo_cnt;
    Uint32 lfo_inc;
    Uint32 lfo_freq[8]; /* LFO FREQ table */
} FM_OPN;

/* SSG struct */
typedef struct
{
    Sint32 lastEnable;
    Uint32 step;
    Sint32 period[3];
    Sint32 PeriodN;
    Sint32 PeriodE;
    Sint32 count[3];
    Sint32 CountN;
    Sint32 CountE;
    Uint32 vol[3];
    Uint32 VolE;
    Uint8 envelope[3];
    Uint8 output[3];
    Uint8 OutputN;
    Sint8 count_env;
    Uint8 hold;
    Uint8 alternate;
    Uint8 attack;
    Uint8 holding;
    Sint32 RNG;
    Uint32 vol_table[32];
} SSG_t;

/* ADPCM type A channel struct */
typedef struct
{
    Uint8 flag; /* port state              */
    Uint8 flagMask; /* arrived flag mask       */
    Uint8 now_data; /* current ROM data            */
    Uint32 now_addr; /* current ROM address        */
    Uint32 now_step;
    Uint32 step;
    Uint32 start; /* sample data start address*/
    Uint32 end; /* sample data end address */
    Uint8 IL; /* Instrument Level          */
    Sint32 adpcma_acc; /* accumulator              */
    Sint32 adpcma_step; /* step                        */
    Sint32 adpcma_out; /* (speedup) hiro-shi!!     */
    Sint8 vol_mul; /* volume in "0.75dB" steps */
    Uint8 vol_shift; /* volume in "-6dB" steps */
    Sint32* pan; /* &out_adpcma[OPN_xxxx]  */
} ADPCMA;

/* ADPCM type B struct */
typedef struct adpcmb_state
{
    Sint32* pan; /* pan : &output_pointer[pan]   */
    double freqbase;
    Sint32 output_range;
    Uint32 now_addr; /* current address      */
    Uint32 now_step; /* currect step         */
    Uint32 step; /* step                 */
    Uint32 start; /* start address        */
    Uint32 limit; /* limit address        */
    Uint32 end; /* end address          */
    Uint32 delta; /* delta scale          */
    Sint32 volume; /* current volume       */
    Sint32 acc; /* shift Measurement value*/
    Sint32 adpcmd; /* next Forecast        */
    Sint32 adpcml; /* current value        */
    Sint32 prev_acc; /* leveling value       */
    Uint8 now_data; /* current rom data     */
    Uint8 CPU_data; /* current data from reg 08 */
    Uint8 portstate; /* port status          */
    Uint8 control2; /* control reg: SAMPLE, DA/AD, RAM TYPE (x8bit / x1bit), ROM/RAM */
    Uint8 portshift; /* address bits shift-left:
     ** 8 for YM2610,
     ** 5 for Y8950 and YM2608 */
    Uint8 DRAMportshift; /* address bits shift-right:
     ** 0 for ROM and x8bit DRAMs,
     ** 3 for x1 DRAMs */
    Uint8 memread; /* needed for reading/writing external memory */
    /* note that different chips have these flags on different
     ** bits of the status register
     */Uint8 status_change_EOS_bit; /* 1 on End Of Sample (record/playback/cycle time of AD/DA converting has passed)*/
    Uint8 status_change_BRDY_bit; /* 1 after recording 2 datas (2x4bits) or after reading/writing 1 data */
    /* neither Y8950 nor YM2608 can generate IRQ when PCMBSY bit changes, so instead of above,
     ** the statusflag gets ORed with PCM_BSY (below) (on each read of statusflag of Y8950 and YM2608)
     */Uint8 PCM_BSY; /* 1 when ADPCM is playing; Y8950/YM2608 only */

} ADPCMB;

/* here's the virtual YM2610 */
typedef struct
{
    Uint8 regs[512]; /* registers            */
    FM_OPN OPN; /* OPN state            */
    FM_CH CH[6]; /* channel state        */
    Uint8 addr_A1; /* address line A1      */
    /* ADPCM-A unit */Uint8 adpcmaTL; /* adpcmA total level   */
    ADPCMA adpcma[6]; /* adpcm channels       */
    Uint8 adpcm_arrivedEndAddress;

    /* ADPCM-B unit */
    ADPCMB adpcmb; /* Delta-T ADPCM unit   */

} ym2610_t;

#ifdef _GNGEOX_YM2610_CORE_C_
static void FM_STATUS_SET ( FM_ST*, Sint32 );
static void FM_STATUS_RESET ( FM_ST*, Sint32 );
static void FM_IRQMASK_SET ( FM_ST*, Sint32 );
static void set_timers ( FM_ST*, Sint32 );
static void TimerAOver ( FM_ST* );
static void TimerBOver ( FM_ST* );
#endif // _GNGEOX_YM2610_CORE_C_

void YM2610Init ( Sint32 baseclock, Sint32, void*, Sint32, void*, Sint32, FM_TIMERHANDLER, FM_IRQHANDLER );
void YM2610ChangeSamplerate ( Sint32 );
void YM2610Reset ( void );
void YM2610Write ( Sint32, Uint8 );
Uint8 YM2610Read ( Sint32 ) __attribute__ ( ( warn_unused_result ) );
void YM2610TimerOver ( Sint32 );
void YM2610Update_stream ( Sint32, Uint16* );

#endif // _GNGEOX_YM2610_CORE_H_
