/*****************************************************************************
 *
 *	 Z80.c
 *	 Portable Z80 emulator V3.3
 *
 *	 Copyright (C) 1998,1999,2000 Juergen Buchmueller, all rights reserved.
 *
 *	 - This source code is released as freeware for non-commercial purposes.
 *	 - You are free to use and redistribute this code in modified or
 *	   unmodified form, provided you list me in the credits.
 *	 - If you modify this source code, you must add a notice to each modified
 *	   source file that it has been changed.  If you're a nice person, you
 *	   will clearly mark each change too.  :)
 *	 - If you wish to use this for commercial purposes, please contact me at
 *	   pullmoll@t-online.de
 *	 - The author of this copywritten work reserves the right to change the
 *	   terms of its usage and license at any time, including retroactively
 *	 - This entire notice must remain in the source code.
 *
 *	 Changes in 3,3
 *	 - Fixed undocumented flags XF & YF in the non-asm versions of CP,
 *	   and all the 16 bit arithmetic instructions. [Sean Young]
 *	 Changes in 3.2
 *	 - Fixed undocumented flags XF & YF of RRCA, and CF and HF of
 *	   INI/IND/OUTI/OUTD/INIR/INDR/OTIR/OTDR [Sean Young]
 *	 Changes in 3.1
 *	 - removed the REPEAT_AT_ONCE execution of LDIR/CPIR etc. opcodes
 *	   for readabilities sake and because the implementation was buggy
 *	   (and I was not able to find the difference)
 *	 Changes in 3.0
 *	 - 'finished' switch to dynamically overrideable cycle count tables
 *	 Changes in 2.9:
 *	 - added methods to access and override the cycle count tables
 *	 - fixed handling and timing of multiple DD/FD prefixed opcodes
 *	 Changes in 2.8:
 *	 - OUTI/OUTD/OTIR/OTDR also pre-decrement the B register now.
 *	   This was wrong because of a bug fix on the wrong side
 *	   (astrocade sound driver).
 *	 Changes in 2.7:
 *	  - removed z80_vm specific code, it's not needed (and never was).
 *	 Changes in 2.6:
 *	  - BUSY_LOOP_HACKS needed to call change_pc16() earlier, before
 *		checking the opcodes at the new address, because otherwise they
 *		might access the old (wrong or even NULL) banked memory region.
 *		Thanks to Sean Young for finding this nasty bug.
 *	 Changes in 2.5:
 *	  - Burning cycles always adjusts the ICount by a multiple of 4.
 *	  - In REPEAT_AT_ONCE cases the R register wasn't incremented twice
 *		per repetition as it should have been. Those repeated opcodes
 *		could also underflow the ICount.
 *	  - Simplified TIME_LOOP_HACKS for BC and added two more for DE + HL
 *		timing loops. I think those hacks weren't endian safe before too.
 *	 Changes in 2.4:
 *	  - z80_reset zaps the entire context, sets IX and IY to 0xffff(!) and
 *		sets the Z flag. With these changes the Tehkan World Cup driver
 *		_seems_ to work again.
 *	 Changes in 2.3:
 *	  - External termination of the execution loop calls z80_burn() and
 *		z80_vm_burn() to burn an amount of cycles (R adjustment)
 *	  - Shortcuts which burn CPU cycles (BUSY_LOOP_HACKS and TIME_LOOP_HACKS)
 *		now also adjust the R register depending on the skipped opcodes.
 *	 Changes in 2.2:
 *	  - Fixed bugs in CPL, SCF and CCF instructions flag handling.
 *	  - Changed variable Z80.EA and ARG16() function to UINT32; this
 *		produces slightly more efficient code.
 *	  - The DD/FD XY CB opcodes where XY is 40-7F and Y is not 6/E
 *		are changed to calls to the X6/XE opcodes to reduce object size.
 *		They're hardly ever used so this should not yield a speed penalty.
 *	 New in 2.0:
 *	  - Optional more exact Z80 emulation (#define Z80_EXACT 1) according
 *		to a detailed description by Sean Young which can be found at:
 *		http://www.msxnet.org/tech/Z80/z80undoc.txt
 *****************************************************************************/
#ifndef _Z80_C_
#define _Z80_C_
#endif // _Z80_C_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
//#include <signal.h>
#include "Z80.h"

static const UINT8 z80_reg_layout[] =
{
    Z80_PC, Z80_SP, Z80_AF, Z80_BC, Z80_DE, Z80_HL, -1,
    Z80_IX, Z80_IY, Z80_AF2, Z80_BC2, Z80_DE2, Z80_HL2, -1,
    Z80_R,	Z80_I,	Z80_IM, Z80_IFF1, Z80_IFF2, -1,
    Z80_NMI_STATE, Z80_IRQ_STATE, Z80_DC0, Z80_DC1, Z80_DC2, Z80_DC3, 0
};

static const UINT8 z80_win_layout[] =
{
    27, 0, 53, 4,	/* register window (top rows) */
    0, 0, 26, 22,	/* disassembler window (left colums) */
    27, 5, 53, 8,	/* memory #1 window (right, upper middle) */
    27, 14, 53, 8,	/* memory #2 window (right, lower middle) */
    0, 23, 80, 1,	/* command line window (bottom rows) */
};

int z80_ICount = 0;
static Z80_Regs Z80;

static UINT8 SZ[256];		/* zero and sign flags */
static UINT8 SZ_BIT[256];	/* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static UINT8 SZP[256];		/* zero, sign and parity flags */
static UINT8 SZHV_inc[256]; /* zero, sign, half carry and overflow flags INC r8 */
static UINT8 SZHV_dec[256]; /* zero, sign, half carry and overflow flags DEC r8 */

static UINT8 SZHVC_add[2 * 256 * 256]; /* flags for ADD opcode */
static UINT8 SZHVC_sub[2 * 256 * 256]; /* flags for SUB opcode */

/* tmp1 value for ini/inir/outi/otir for [C.1-0][io.1-0] */
static const UINT8 irep_tmp1[4][4] =
{
    {0, 0, 1, 0}, {0, 1, 0, 1}, {1, 0, 1, 1}, {0, 1, 1, 0}
};

/* tmp1 value for ind/indr/outd/otdr for [C.1-0][io.1-0] */
static const UINT8 drep_tmp1[4][4] =
{
    {0, 1, 0, 0}, {1, 0, 0, 1}, {0, 0, 1, 0}, {0, 1, 0, 1}
};

/* tmp2 value for all in/out repeated opcodes for B.7-0 */
static const UINT8 breg_tmp2[256] =
{
    0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
    0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
    1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
    1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
    0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
    1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
    0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
    0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
    1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
    1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
    0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
    0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
    1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1,
    0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
    1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0,
    1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1
};

static const UINT8 cc_op[0x100] =
{
    4, 10, 7, 6, 4, 4, 7, 4, 4, 11, 7, 6, 4, 4, 7, 4,
    8, 10, 7, 6, 4, 4, 7, 4, 12, 11, 7, 6, 4, 4, 7, 4,
    7, 10, 16, 6, 4, 4, 7, 4, 7, 11, 16, 6, 4, 4, 7, 4,
    7, 10, 13, 6, 11, 11, 10, 4, 7, 11, 13, 6, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    7, 7, 7, 7, 7, 7, 4, 7, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    5, 10, 10, 10, 10, 11, 7, 11, 5, 10, 10, 0, 10, 17, 7, 11,
    5, 10, 10, 11, 10, 11, 7, 11, 5, 4, 10, 11, 10, 0, 7, 11,
    5, 10, 10, 19, 10, 11, 7, 11, 5, 4, 10, 4, 10, 0, 7, 11,
    5, 10, 10, 4, 10, 11, 7, 11, 5, 6, 10, 4, 10, 0, 7, 11
};

static const UINT8 cc_cb[0x100] =
{
    8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
    8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
    8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
    8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8,
    8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
    8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
    8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
    8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
    8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
    8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
    8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8,
    8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8
};

static const UINT8 cc_ed[0x100] =
{
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    12, 12, 15, 20, 8, 8, 8, 9, 12, 12, 15, 20, 8, 8, 8, 9,
    12, 12, 15, 20, 8, 8, 8, 9, 12, 12, 15, 20, 8, 8, 8, 9,
    12, 12, 15, 20, 8, 8, 8, 18, 12, 12, 15, 20, 8, 8, 8, 18,
    12, 12, 15, 20, 8, 8, 8, 8, 12, 12, 15, 20, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    16, 16, 16, 16, 8, 8, 8, 8, 16, 16, 16, 16, 8, 8, 8, 8,
    16, 16, 16, 16, 8, 8, 8, 8, 16, 16, 16, 16, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

static const UINT8 cc_xy[0x100] =
{
    4, 4, 4, 4, 4, 4, 4, 4, 4, 15, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 15, 4, 4, 4, 4, 4, 4,
    4, 14, 20, 10, 9, 9, 9, 4, 4, 15, 20, 10, 9, 9, 9, 4,
    4, 4, 4, 4, 23, 23, 19, 4, 4, 15, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 9, 9, 19, 4, 4, 4, 4, 4, 9, 9, 19, 4,
    4, 4, 4, 4, 9, 9, 19, 4, 4, 4, 4, 4, 9, 9, 19, 4,
    9, 9, 9, 9, 9, 9, 19, 9, 9, 9, 9, 9, 9, 9, 19, 9,
    19, 19, 19, 19, 19, 19, 4, 19, 4, 4, 4, 4, 9, 9, 19, 4,
    4, 4, 4, 4, 9, 9, 19, 4, 4, 4, 4, 4, 9, 9, 19, 4,
    4, 4, 4, 4, 9, 9, 19, 4, 4, 4, 4, 4, 9, 9, 19, 4,
    4, 4, 4, 4, 9, 9, 19, 4, 4, 4, 4, 4, 9, 9, 19, 4,
    4, 4, 4, 4, 9, 9, 19, 4, 4, 4, 4, 4, 9, 9, 19, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 14, 4, 23, 4, 15, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 10, 4, 4, 4, 4, 4, 4
};

static const UINT8 cc_xycb[0x100] =
{
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23
};

/* extra cycles if jr/jp/call taken and 'interrupt latency' on rst 0-7 */
static const UINT8 cc_ex[0x100] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* DJNZ */
    5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,	/* JR NZ/JR Z */
    5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,	/* JR NC/JR C */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0,	/* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
    6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
    6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
    6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
    6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2
};

static const UINT8 *cc[6] = { cc_op, cc_cb, cc_ed, cc_xy, cc_xycb, cc_ex };

static void take_interrupt ( void );

typedef void ( *funcptr ) ( void );

PROTOTYPES ( Z80op, op );
PROTOTYPES ( Z80cb, cb );
PROTOTYPES ( Z80dd, dd );
PROTOTYPES ( Z80ed, ed );
PROTOTYPES ( Z80fd, fd );
PROTOTYPES ( Z80xycb, xycb );

/****************************************************************************/
/* Burn an odd amount of cycles, that is instructions taking something		*/
/* different from 4 T-states per opcode (and R increment)					*/
/****************************************************************************/
INLINE void BURNODD ( int cycles, int opcodes, int cyclesum )
{
    if ( cycles > 0 )
    {
        _R += ( cycles / cyclesum ) * opcodes;
        z80_ICount -= ( cycles / cyclesum ) * cyclesum;
    }
}

/***************************************************************
 * Read a word from given memory location
 ***************************************************************/
INLINE void RM16 ( UINT32 addr, PAIR *r )
{
    r->b.l = RM ( addr );
    r->b.h = RM ( ( addr + 1 ) & 0xffff );
}

/***************************************************************
 * Write a word to given memory location
 ***************************************************************/
INLINE void WM16 ( UINT32 addr, PAIR *r )
{
    WM ( addr, r->b.l );
    WM ( ( addr + 1 ) & 0xffff, r->b.h );
}

/***************************************************************
 * ROP() is identical to RM() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
INLINE UINT8 ROP ( void )
{
    unsigned pc = _PCD;
    _PC++;
    return RM ( pc );
}

/****************************************************************
 * ARG() is identical to ROP() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 ***************************************************************/
INLINE UINT8 ARG ( void )
{
    unsigned pc = _PCD;
    _PC++;
    return RM ( pc );
}

INLINE UINT32 ARG16 ( void )
{
    unsigned pc = _PCD;
    _PC += 2;
    return RM ( pc ) | ( RM ( ( pc + 1 ) & 0xffff ) << 8 );
}

/***************************************************************
 * INC	r8
 ***************************************************************/
INLINE UINT8 INC ( UINT8 value )
{
    UINT8 res = value + 1;
    _F = ( _F & CF ) | SZHV_inc[res];
    return ( UINT8 ) res;
}

/***************************************************************
 * DEC	r8
 ***************************************************************/
INLINE UINT8 DEC ( UINT8 value )
{
    UINT8 res = value - 1;
    _F = ( _F & CF ) | SZHV_dec[res];
    return res;
}

/***************************************************************
 * RLC	r8
 ***************************************************************/
INLINE UINT8 RLC ( UINT8 value )
{
    unsigned res = value;
    unsigned c = ( res & 0x80 ) ? CF : 0;
    res = ( ( res << 1 ) | ( res >> 7 ) ) & 0xff;
    _F = SZP[res] | c;
    return res;
}

/***************************************************************
 * RRC	r8
 ***************************************************************/
INLINE UINT8 RRC ( UINT8 value )
{
    unsigned res = value;
    unsigned c = ( res & 0x01 ) ? CF : 0;
    res = ( ( res >> 1 ) | ( res << 7 ) ) & 0xff;
    _F = SZP[res] | c;
    return res;
}

/***************************************************************
 * RL	r8
 ***************************************************************/
INLINE UINT8 RL ( UINT8 value )
{
    unsigned res = value;
    unsigned c = ( res & 0x80 ) ? CF : 0;
    res = ( ( res << 1 ) | ( _F & CF ) ) & 0xff;
    _F = SZP[res] | c;
    return res;
}

/***************************************************************
 * RR	r8
 ***************************************************************/
INLINE UINT8 RR ( UINT8 value )
{
    unsigned res = value;
    unsigned c = ( res & 0x01 ) ? CF : 0;
    res = ( ( res >> 1 ) | ( _F << 7 ) ) & 0xff;
    _F = SZP[res] | c;
    return res;
}

/***************************************************************
 * SLA	r8
 ***************************************************************/
INLINE UINT8 SLA ( UINT8 value )
{
    unsigned res = value;
    unsigned c = ( res & 0x80 ) ? CF : 0;
    res = ( res << 1 ) & 0xff;
    _F = SZP[res] | c;
    return res;
}

/***************************************************************
 * SRA	r8
 ***************************************************************/
INLINE UINT8 SRA ( UINT8 value )
{
    unsigned res = value;
    unsigned c = ( res & 0x01 ) ? CF : 0;
    res = ( ( res >> 1 ) | ( res & 0x80 ) ) & 0xff;
    _F = SZP[res] | c;
    return res;
}

/***************************************************************
 * SLL	r8
 ***************************************************************/
INLINE UINT8 SLL ( UINT8 value )
{
    unsigned res = value;
    unsigned c = ( res & 0x80 ) ? CF : 0;
    res = ( ( res << 1 ) | 0x01 ) & 0xff;
    _F = SZP[res] | c;
    return res;
}

/***************************************************************
 * SRL	r8
 ***************************************************************/
INLINE UINT8 SRL ( UINT8 value )
{
    unsigned res = value;
    unsigned c = ( res & 0x01 ) ? CF : 0;
    res = ( res >> 1 ) & 0xff;
    _F = SZP[res] | c;
    return res;
}

/***************************************************************
 * RES	bit,r8
 ***************************************************************/
INLINE UINT8 RES ( UINT8 bit, UINT8 value )
{
    return value & ~ ( 1 << bit );
}

/***************************************************************
 * SET	bit,r8
 ***************************************************************/
INLINE UINT8 SET ( UINT8 bit, UINT8 value )
{
    return value | ( 1 << bit );
}

/**********************************************************
 * opcodes with CB prefix
 * rotate, shift and bit operations
 **********************************************************/
OP ( cb, 00 )
{
    _B = RLC ( _B );											   /* RLC  B 		  */
}
OP ( cb, 01 )
{
    _C = RLC ( _C );											   /* RLC  C 		  */
}
OP ( cb, 02 )
{
    _D = RLC ( _D );											   /* RLC  D 		  */
}
OP ( cb, 03 )
{
    _E = RLC ( _E );											   /* RLC  E 		  */
}
OP ( cb, 04 )
{
    _H = RLC ( _H );											   /* RLC  H 		  */
}
OP ( cb, 05 )
{
    _L = RLC ( _L );											   /* RLC  L 		  */
}
OP ( cb, 06 )
{
    WM ( _HL, RLC ( RM ( _HL ) ) );								   /* RLC  (HL)		  */
}
OP ( cb, 07 )
{
    _A = RLC ( _A );											   /* RLC  A 		  */
}

OP ( cb, 08 )
{
    _B = RRC ( _B );											   /* RRC  B 		  */
}
OP ( cb, 09 )
{
    _C = RRC ( _C );											   /* RRC  C 		  */
}
OP ( cb, 0a )
{
    _D = RRC ( _D );											   /* RRC  D 		  */
}
OP ( cb, 0b )
{
    _E = RRC ( _E );											   /* RRC  E 		  */
}
OP ( cb, 0c )
{
    _H = RRC ( _H );											   /* RRC  H 		  */
}
OP ( cb, 0d )
{
    _L = RRC ( _L );											   /* RRC  L 		  */
}
OP ( cb, 0e )
{
    WM ( _HL, RRC ( RM ( _HL ) ) );								   /* RRC  (HL)		  */
}
OP ( cb, 0f )
{
    _A = RRC ( _A );											   /* RRC  A 		  */
}

OP ( cb, 10 )
{
    _B = RL ( _B );											   /* RL   B 		  */
}
OP ( cb, 11 )
{
    _C = RL ( _C );											   /* RL   C 		  */
}
OP ( cb, 12 )
{
    _D = RL ( _D );											   /* RL   D 		  */
}
OP ( cb, 13 )
{
    _E = RL ( _E );											   /* RL   E 		  */
}
OP ( cb, 14 )
{
    _H = RL ( _H );											   /* RL   H 		  */
}
OP ( cb, 15 )
{
    _L = RL ( _L );											   /* RL   L 		  */
}
OP ( cb, 16 )
{
    WM ( _HL, RL ( RM ( _HL ) ) ); 								   /* RL   (HL)		  */
}
OP ( cb, 17 )
{
    _A = RL ( _A );											   /* RL   A 		  */
}

OP ( cb, 18 )
{
    _B = RR ( _B );											   /* RR   B 		  */
}
OP ( cb, 19 )
{
    _C = RR ( _C );											   /* RR   C 		  */
}
OP ( cb, 1a )
{
    _D = RR ( _D );											   /* RR   D 		  */
}
OP ( cb, 1b )
{
    _E = RR ( _E );											   /* RR   E 		  */
}
OP ( cb, 1c )
{
    _H = RR ( _H );											   /* RR   H 		  */
}
OP ( cb, 1d )
{
    _L = RR ( _L );											   /* RR   L 		  */
}
OP ( cb, 1e )
{
    WM ( _HL, RR ( RM ( _HL ) ) ); 								   /* RR   (HL)		  */
}
OP ( cb, 1f )
{
    _A = RR ( _A );											   /* RR   A 		  */
}

OP ( cb, 20 )
{
    _B = SLA ( _B );											   /* SLA  B 		  */
}
OP ( cb, 21 )
{
    _C = SLA ( _C );											   /* SLA  C 		  */
}
OP ( cb, 22 )
{
    _D = SLA ( _D );											   /* SLA  D 		  */
}
OP ( cb, 23 )
{
    _E = SLA ( _E );											   /* SLA  E 		  */
}
OP ( cb, 24 )
{
    _H = SLA ( _H );											   /* SLA  H 		  */
}
OP ( cb, 25 )
{
    _L = SLA ( _L );											   /* SLA  L 		  */
}
OP ( cb, 26 )
{
    WM ( _HL, SLA ( RM ( _HL ) ) );								   /* SLA  (HL)		  */
}
OP ( cb, 27 )
{
    _A = SLA ( _A );											   /* SLA  A 		  */
}

OP ( cb, 28 )
{
    _B = SRA ( _B );											   /* SRA  B 		  */
}
OP ( cb, 29 )
{
    _C = SRA ( _C );											   /* SRA  C 		  */
}
OP ( cb, 2a )
{
    _D = SRA ( _D );											   /* SRA  D 		  */
}
OP ( cb, 2b )
{
    _E = SRA ( _E );											   /* SRA  E 		  */
}
OP ( cb, 2c )
{
    _H = SRA ( _H );											   /* SRA  H 		  */
}
OP ( cb, 2d )
{
    _L = SRA ( _L );											   /* SRA  L 		  */
}
OP ( cb, 2e )
{
    WM ( _HL, SRA ( RM ( _HL ) ) );								   /* SRA  (HL)		  */
}
OP ( cb, 2f )
{
    _A = SRA ( _A );											   /* SRA  A 		  */
}

OP ( cb, 30 )
{
    _B = SLL ( _B );											   /* SLL  B 		  */
}
OP ( cb, 31 )
{
    _C = SLL ( _C );											   /* SLL  C 		  */
}
OP ( cb, 32 )
{
    _D = SLL ( _D );											   /* SLL  D 		  */
}
OP ( cb, 33 )
{
    _E = SLL ( _E );											   /* SLL  E 		  */
}
OP ( cb, 34 )
{
    _H = SLL ( _H );											   /* SLL  H 		  */
}
OP ( cb, 35 )
{
    _L = SLL ( _L );											   /* SLL  L 		  */
}
OP ( cb, 36 )
{
    WM ( _HL, SLL ( RM ( _HL ) ) );								   /* SLL  (HL)		  */
}
OP ( cb, 37 )
{
    _A = SLL ( _A );											   /* SLL  A 		  */
}

OP ( cb, 38 )
{
    _B = SRL ( _B );											   /* SRL  B 		  */
}
OP ( cb, 39 )
{
    _C = SRL ( _C );											   /* SRL  C 		  */
}
OP ( cb, 3a )
{
    _D = SRL ( _D );											   /* SRL  D 		  */
}
OP ( cb, 3b )
{
    _E = SRL ( _E );											   /* SRL  E 		  */
}
OP ( cb, 3c )
{
    _H = SRL ( _H );											   /* SRL  H 		  */
}
OP ( cb, 3d )
{
    _L = SRL ( _L );											   /* SRL  L 		  */
}
OP ( cb, 3e )
{
    WM ( _HL, SRL ( RM ( _HL ) ) );								   /* SRL  (HL)		  */
}
OP ( cb, 3f )
{
    _A = SRL ( _A );											   /* SRL  A 		  */
}

OP ( cb, 40 )
{
    BIT ( 0, _B );												   /* BIT  0,B		  */
}
OP ( cb, 41 )
{
    BIT ( 0, _C );												   /* BIT  0,C		  */
}
OP ( cb, 42 )
{
    BIT ( 0, _D );												   /* BIT  0,D		  */
}
OP ( cb, 43 )
{
    BIT ( 0, _E );												   /* BIT  0,E		  */
}
OP ( cb, 44 )
{
    BIT ( 0, _H );												   /* BIT  0,H		  */
}
OP ( cb, 45 )
{
    BIT ( 0, _L );												   /* BIT  0,L		  */
}
OP ( cb, 46 )
{
    BIT ( 0, RM ( _HL ) ); 										   /* BIT  0,(HL)	  */
}
OP ( cb, 47 )
{
    BIT ( 0, _A );												   /* BIT  0,A		  */
}

OP ( cb, 48 )
{
    BIT ( 1, _B );												   /* BIT  1,B		  */
}
OP ( cb, 49 )
{
    BIT ( 1, _C );												   /* BIT  1,C		  */
}
OP ( cb, 4a )
{
    BIT ( 1, _D );												   /* BIT  1,D		  */
}
OP ( cb, 4b )
{
    BIT ( 1, _E );												   /* BIT  1,E		  */
}
OP ( cb, 4c )
{
    BIT ( 1, _H );												   /* BIT  1,H		  */
}
OP ( cb, 4d )
{
    BIT ( 1, _L );												   /* BIT  1,L		  */
}
OP ( cb, 4e )
{
    BIT ( 1, RM ( _HL ) ); 										   /* BIT  1,(HL)	  */
}
OP ( cb, 4f )
{
    BIT ( 1, _A );												   /* BIT  1,A		  */
}

OP ( cb, 50 )
{
    BIT ( 2, _B );												   /* BIT  2,B		  */
}
OP ( cb, 51 )
{
    BIT ( 2, _C );												   /* BIT  2,C		  */
}
OP ( cb, 52 )
{
    BIT ( 2, _D );												   /* BIT  2,D		  */
}
OP ( cb, 53 )
{
    BIT ( 2, _E );												   /* BIT  2,E		  */
}
OP ( cb, 54 )
{
    BIT ( 2, _H );												   /* BIT  2,H		  */
}
OP ( cb, 55 )
{
    BIT ( 2, _L );												   /* BIT  2,L		  */
}
OP ( cb, 56 )
{
    BIT ( 2, RM ( _HL ) ); 										   /* BIT  2,(HL)	  */
}
OP ( cb, 57 )
{
    BIT ( 2, _A );												   /* BIT  2,A		  */
}

OP ( cb, 58 )
{
    BIT ( 3, _B );												   /* BIT  3,B		  */
}
OP ( cb, 59 )
{
    BIT ( 3, _C );												   /* BIT  3,C		  */
}
OP ( cb, 5a )
{
    BIT ( 3, _D );												   /* BIT  3,D		  */
}
OP ( cb, 5b )
{
    BIT ( 3, _E );												   /* BIT  3,E		  */
}
OP ( cb, 5c )
{
    BIT ( 3, _H );												   /* BIT  3,H		  */
}
OP ( cb, 5d )
{
    BIT ( 3, _L );												   /* BIT  3,L		  */
}
OP ( cb, 5e )
{
    BIT ( 3, RM ( _HL ) ); 										   /* BIT  3,(HL)	  */
}
OP ( cb, 5f )
{
    BIT ( 3, _A );												   /* BIT  3,A		  */
}

OP ( cb, 60 )
{
    BIT ( 4, _B );												   /* BIT  4,B		  */
}
OP ( cb, 61 )
{
    BIT ( 4, _C );												   /* BIT  4,C		  */
}
OP ( cb, 62 )
{
    BIT ( 4, _D );												   /* BIT  4,D		  */
}
OP ( cb, 63 )
{
    BIT ( 4, _E );												   /* BIT  4,E		  */
}
OP ( cb, 64 )
{
    BIT ( 4, _H );												   /* BIT  4,H		  */
}
OP ( cb, 65 )
{
    BIT ( 4, _L );												   /* BIT  4,L		  */
}
OP ( cb, 66 )
{
    BIT ( 4, RM ( _HL ) ); 										   /* BIT  4,(HL)	  */
}
OP ( cb, 67 )
{
    BIT ( 4, _A );												   /* BIT  4,A		  */
}

OP ( cb, 68 )
{
    BIT ( 5, _B );												   /* BIT  5,B		  */
}
OP ( cb, 69 )
{
    BIT ( 5, _C );												   /* BIT  5,C		  */
}
OP ( cb, 6a )
{
    BIT ( 5, _D );												   /* BIT  5,D		  */
}
OP ( cb, 6b )
{
    BIT ( 5, _E );												   /* BIT  5,E		  */
}
OP ( cb, 6c )
{
    BIT ( 5, _H );												   /* BIT  5,H		  */
}
OP ( cb, 6d )
{
    BIT ( 5, _L );												   /* BIT  5,L		  */
}
OP ( cb, 6e )
{
    BIT ( 5, RM ( _HL ) ); 										   /* BIT  5,(HL)	  */
}
OP ( cb, 6f )
{
    BIT ( 5, _A );												   /* BIT  5,A		  */
}

OP ( cb, 70 )
{
    BIT ( 6, _B );												   /* BIT  6,B		  */
}
OP ( cb, 71 )
{
    BIT ( 6, _C );												   /* BIT  6,C		  */
}
OP ( cb, 72 )
{
    BIT ( 6, _D );												   /* BIT  6,D		  */
}
OP ( cb, 73 )
{
    BIT ( 6, _E );												   /* BIT  6,E		  */
}
OP ( cb, 74 )
{
    BIT ( 6, _H );												   /* BIT  6,H		  */
}
OP ( cb, 75 )
{
    BIT ( 6, _L );												   /* BIT  6,L		  */
}
OP ( cb, 76 )
{
    BIT ( 6, RM ( _HL ) ); 										   /* BIT  6,(HL)	  */
}
OP ( cb, 77 )
{
    BIT ( 6, _A );												   /* BIT  6,A		  */
}

OP ( cb, 78 )
{
    BIT ( 7, _B );												   /* BIT  7,B		  */
}
OP ( cb, 79 )
{
    BIT ( 7, _C );												   /* BIT  7,C		  */
}
OP ( cb, 7a )
{
    BIT ( 7, _D );												   /* BIT  7,D		  */
}
OP ( cb, 7b )
{
    BIT ( 7, _E );												   /* BIT  7,E		  */
}
OP ( cb, 7c )
{
    BIT ( 7, _H );												   /* BIT  7,H		  */
}
OP ( cb, 7d )
{
    BIT ( 7, _L );												   /* BIT  7,L		  */
}
OP ( cb, 7e )
{
    BIT ( 7, RM ( _HL ) ); 										   /* BIT  7,(HL)	  */
}
OP ( cb, 7f )
{
    BIT ( 7, _A );												   /* BIT  7,A		  */
}

OP ( cb, 80 )
{
    _B = RES ( 0, _B ); 										   /* RES  0,B		  */
}
OP ( cb, 81 )
{
    _C = RES ( 0, _C ); 										   /* RES  0,C		  */
}
OP ( cb, 82 )
{
    _D = RES ( 0, _D ); 										   /* RES  0,D		  */
}
OP ( cb, 83 )
{
    _E = RES ( 0, _E ); 										   /* RES  0,E		  */
}
OP ( cb, 84 )
{
    _H = RES ( 0, _H ); 										   /* RES  0,H		  */
}
OP ( cb, 85 )
{
    _L = RES ( 0, _L ); 										   /* RES  0,L		  */
}
OP ( cb, 86 )
{
    WM ( _HL, RES ( 0, RM ( _HL ) ) );								   /* RES  0,(HL)	  */
}
OP ( cb, 87 )
{
    _A = RES ( 0, _A ); 										   /* RES  0,A		  */
}

OP ( cb, 88 )
{
    _B = RES ( 1, _B ); 										   /* RES  1,B		  */
}
OP ( cb, 89 )
{
    _C = RES ( 1, _C ); 										   /* RES  1,C		  */
}
OP ( cb, 8a )
{
    _D = RES ( 1, _D ); 										   /* RES  1,D		  */
}
OP ( cb, 8b )
{
    _E = RES ( 1, _E ); 										   /* RES  1,E		  */
}
OP ( cb, 8c )
{
    _H = RES ( 1, _H ); 										   /* RES  1,H		  */
}
OP ( cb, 8d )
{
    _L = RES ( 1, _L ); 										   /* RES  1,L		  */
}
OP ( cb, 8e )
{
    WM ( _HL, RES ( 1, RM ( _HL ) ) );								   /* RES  1,(HL)	  */
}
OP ( cb, 8f )
{
    _A = RES ( 1, _A ); 										   /* RES  1,A		  */
}

OP ( cb, 90 )
{
    _B = RES ( 2, _B ); 										   /* RES  2,B		  */
}
OP ( cb, 91 )
{
    _C = RES ( 2, _C ); 										   /* RES  2,C		  */
}
OP ( cb, 92 )
{
    _D = RES ( 2, _D ); 										   /* RES  2,D		  */
}
OP ( cb, 93 )
{
    _E = RES ( 2, _E ); 										   /* RES  2,E		  */
}
OP ( cb, 94 )
{
    _H = RES ( 2, _H ); 										   /* RES  2,H		  */
}
OP ( cb, 95 )
{
    _L = RES ( 2, _L ); 										   /* RES  2,L		  */
}
OP ( cb, 96 )
{
    WM ( _HL, RES ( 2, RM ( _HL ) ) );								   /* RES  2,(HL)	  */
}
OP ( cb, 97 )
{
    _A = RES ( 2, _A ); 										   /* RES  2,A		  */
}

OP ( cb, 98 )
{
    _B = RES ( 3, _B ); 										   /* RES  3,B		  */
}
OP ( cb, 99 )
{
    _C = RES ( 3, _C ); 										   /* RES  3,C		  */
}
OP ( cb, 9a )
{
    _D = RES ( 3, _D ); 										   /* RES  3,D		  */
}
OP ( cb, 9b )
{
    _E = RES ( 3, _E ); 										   /* RES  3,E		  */
}
OP ( cb, 9c )
{
    _H = RES ( 3, _H ); 										   /* RES  3,H		  */
}
OP ( cb, 9d )
{
    _L = RES ( 3, _L ); 										   /* RES  3,L		  */
}
OP ( cb, 9e )
{
    WM ( _HL, RES ( 3, RM ( _HL ) ) );								   /* RES  3,(HL)	  */
}
OP ( cb, 9f )
{
    _A = RES ( 3, _A ); 										   /* RES  3,A		  */
}

OP ( cb, a0 )
{
    _B = RES ( 4, _B ); 										   /* RES  4,B		  */
}
OP ( cb, a1 )
{
    _C = RES ( 4, _C ); 										   /* RES  4,C		  */
}
OP ( cb, a2 )
{
    _D = RES ( 4, _D ); 										   /* RES  4,D		  */
}
OP ( cb, a3 )
{
    _E = RES ( 4, _E ); 										   /* RES  4,E		  */
}
OP ( cb, a4 )
{
    _H = RES ( 4, _H ); 										   /* RES  4,H		  */
}
OP ( cb, a5 )
{
    _L = RES ( 4, _L ); 										   /* RES  4,L		  */
}
OP ( cb, a6 )
{
    WM ( _HL, RES ( 4, RM ( _HL ) ) );								   /* RES  4,(HL)	  */
}
OP ( cb, a7 )
{
    _A = RES ( 4, _A ); 										   /* RES  4,A		  */
}

OP ( cb, a8 )
{
    _B = RES ( 5, _B ); 										   /* RES  5,B		  */
}
OP ( cb, a9 )
{
    _C = RES ( 5, _C ); 										   /* RES  5,C		  */
}
OP ( cb, aa )
{
    _D = RES ( 5, _D ); 										   /* RES  5,D		  */
}
OP ( cb, ab )
{
    _E = RES ( 5, _E ); 										   /* RES  5,E		  */
}
OP ( cb, ac )
{
    _H = RES ( 5, _H ); 										   /* RES  5,H		  */
}
OP ( cb, ad )
{
    _L = RES ( 5, _L ); 										   /* RES  5,L		  */
}
OP ( cb, ae )
{
    WM ( _HL, RES ( 5, RM ( _HL ) ) );								   /* RES  5,(HL)	  */
}
OP ( cb, af )
{
    _A = RES ( 5, _A ); 										   /* RES  5,A		  */
}

OP ( cb, b0 )
{
    _B = RES ( 6, _B ); 										   /* RES  6,B		  */
}
OP ( cb, b1 )
{
    _C = RES ( 6, _C ); 										   /* RES  6,C		  */
}
OP ( cb, b2 )
{
    _D = RES ( 6, _D ); 										   /* RES  6,D		  */
}
OP ( cb, b3 )
{
    _E = RES ( 6, _E ); 										   /* RES  6,E		  */
}
OP ( cb, b4 )
{
    _H = RES ( 6, _H ); 										   /* RES  6,H		  */
}
OP ( cb, b5 )
{
    _L = RES ( 6, _L ); 										   /* RES  6,L		  */
}
OP ( cb, b6 )
{
    WM ( _HL, RES ( 6, RM ( _HL ) ) );								   /* RES  6,(HL)	  */
}
OP ( cb, b7 )
{
    _A = RES ( 6, _A ); 										   /* RES  6,A		  */
}

OP ( cb, b8 )
{
    _B = RES ( 7, _B ); 										   /* RES  7,B		  */
}
OP ( cb, b9 )
{
    _C = RES ( 7, _C ); 										   /* RES  7,C		  */
}
OP ( cb, ba )
{
    _D = RES ( 7, _D ); 										   /* RES  7,D		  */
}
OP ( cb, bb )
{
    _E = RES ( 7, _E ); 										   /* RES  7,E		  */
}
OP ( cb, bc )
{
    _H = RES ( 7, _H ); 										   /* RES  7,H		  */
}
OP ( cb, bd )
{
    _L = RES ( 7, _L ); 										   /* RES  7,L		  */
}
OP ( cb, be )
{
    WM ( _HL, RES ( 7, RM ( _HL ) ) );								   /* RES  7,(HL)	  */
}
OP ( cb, bf )
{
    _A = RES ( 7, _A ); 										   /* RES  7,A		  */
}

OP ( cb, c0 )
{
    _B = SET ( 0, _B ); 										   /* SET  0,B		  */
}
OP ( cb, c1 )
{
    _C = SET ( 0, _C ); 										   /* SET  0,C		  */
}
OP ( cb, c2 )
{
    _D = SET ( 0, _D ); 										   /* SET  0,D		  */
}
OP ( cb, c3 )
{
    _E = SET ( 0, _E ); 										   /* SET  0,E		  */
}
OP ( cb, c4 )
{
    _H = SET ( 0, _H ); 										   /* SET  0,H		  */
}
OP ( cb, c5 )
{
    _L = SET ( 0, _L ); 										   /* SET  0,L		  */
}
OP ( cb, c6 )
{
    WM ( _HL, SET ( 0, RM ( _HL ) ) );								   /* SET  0,(HL)	  */
}
OP ( cb, c7 )
{
    _A = SET ( 0, _A ); 										   /* SET  0,A		  */
}

OP ( cb, c8 )
{
    _B = SET ( 1, _B ); 										   /* SET  1,B		  */
}
OP ( cb, c9 )
{
    _C = SET ( 1, _C ); 										   /* SET  1,C		  */
}
OP ( cb, ca )
{
    _D = SET ( 1, _D ); 										   /* SET  1,D		  */
}
OP ( cb, cb )
{
    _E = SET ( 1, _E ); 										   /* SET  1,E		  */
}
OP ( cb, cc )
{
    _H = SET ( 1, _H ); 										   /* SET  1,H		  */
}
OP ( cb, cd )
{
    _L = SET ( 1, _L ); 										   /* SET  1,L		  */
}
OP ( cb, ce )
{
    WM ( _HL, SET ( 1, RM ( _HL ) ) );								   /* SET  1,(HL)	  */
}
OP ( cb, cf )
{
    _A = SET ( 1, _A ); 										   /* SET  1,A		  */
}

OP ( cb, d0 )
{
    _B = SET ( 2, _B ); 										   /* SET  2,B		  */
}
OP ( cb, d1 )
{
    _C = SET ( 2, _C ); 										   /* SET  2,C		  */
}
OP ( cb, d2 )
{
    _D = SET ( 2, _D ); 										   /* SET  2,D		  */
}
OP ( cb, d3 )
{
    _E = SET ( 2, _E ); 										   /* SET  2,E		  */
}
OP ( cb, d4 )
{
    _H = SET ( 2, _H ); 										   /* SET  2,H		  */
}
OP ( cb, d5 )
{
    _L = SET ( 2, _L ); 										   /* SET  2,L		  */
}
OP ( cb, d6 )
{
    WM ( _HL, SET ( 2, RM ( _HL ) ) );								   /* SET  2,(HL) 	 */
}
OP ( cb, d7 )
{
    _A = SET ( 2, _A ); 										   /* SET  2,A		  */
}

OP ( cb, d8 )
{
    _B = SET ( 3, _B ); 										   /* SET  3,B		  */
}
OP ( cb, d9 )
{
    _C = SET ( 3, _C ); 										   /* SET  3,C		  */
}
OP ( cb, da )
{
    _D = SET ( 3, _D ); 										   /* SET  3,D		  */
}
OP ( cb, db )
{
    _E = SET ( 3, _E ); 										   /* SET  3,E		  */
}
OP ( cb, dc )
{
    _H = SET ( 3, _H ); 										   /* SET  3,H		  */
}
OP ( cb, dd )
{
    _L = SET ( 3, _L ); 										   /* SET  3,L		  */
}
OP ( cb, de )
{
    WM ( _HL, SET ( 3, RM ( _HL ) ) );								   /* SET  3,(HL)	  */
}
OP ( cb, df )
{
    _A = SET ( 3, _A ); 										   /* SET  3,A		  */
}

OP ( cb, e0 )
{
    _B = SET ( 4, _B ); 										   /* SET  4,B		  */
}
OP ( cb, e1 )
{
    _C = SET ( 4, _C ); 										   /* SET  4,C		  */
}
OP ( cb, e2 )
{
    _D = SET ( 4, _D ); 										   /* SET  4,D		  */
}
OP ( cb, e3 )
{
    _E = SET ( 4, _E ); 										   /* SET  4,E		  */
}
OP ( cb, e4 )
{
    _H = SET ( 4, _H ); 										   /* SET  4,H		  */
}
OP ( cb, e5 )
{
    _L = SET ( 4, _L ); 										   /* SET  4,L		  */
}
OP ( cb, e6 )
{
    WM ( _HL, SET ( 4, RM ( _HL ) ) );								   /* SET  4,(HL)	  */
}
OP ( cb, e7 )
{
    _A = SET ( 4, _A ); 										   /* SET  4,A		  */
}

OP ( cb, e8 )
{
    _B = SET ( 5, _B ); 										   /* SET  5,B		  */
}
OP ( cb, e9 )
{
    _C = SET ( 5, _C ); 										   /* SET  5,C		  */
}
OP ( cb, ea )
{
    _D = SET ( 5, _D ); 										   /* SET  5,D		  */
}
OP ( cb, eb )
{
    _E = SET ( 5, _E ); 										   /* SET  5,E		  */
}
OP ( cb, ec )
{
    _H = SET ( 5, _H ); 										   /* SET  5,H		  */
}
OP ( cb, ed )
{
    _L = SET ( 5, _L ); 										   /* SET  5,L		  */
}
OP ( cb, ee )
{
    WM ( _HL, SET ( 5, RM ( _HL ) ) );								   /* SET  5,(HL)	  */
}
OP ( cb, ef )
{
    _A = SET ( 5, _A ); 										   /* SET  5,A		  */
}

OP ( cb, f0 )
{
    _B = SET ( 6, _B ); 										   /* SET  6,B		  */
}
OP ( cb, f1 )
{
    _C = SET ( 6, _C ); 										   /* SET  6,C		  */
}
OP ( cb, f2 )
{
    _D = SET ( 6, _D ); 										   /* SET  6,D		  */
}
OP ( cb, f3 )
{
    _E = SET ( 6, _E ); 										   /* SET  6,E		  */
}
OP ( cb, f4 )
{
    _H = SET ( 6, _H ); 										   /* SET  6,H		  */
}
OP ( cb, f5 )
{
    _L = SET ( 6, _L ); 										   /* SET  6,L		  */
}
OP ( cb, f6 )
{
    WM ( _HL, SET ( 6, RM ( _HL ) ) );								   /* SET  6,(HL)	  */
}
OP ( cb, f7 )
{
    _A = SET ( 6, _A ); 										   /* SET  6,A		  */
}

OP ( cb, f8 )
{
    _B = SET ( 7, _B ); 										   /* SET  7,B		  */
}
OP ( cb, f9 )
{
    _C = SET ( 7, _C ); 										   /* SET  7,C		  */
}
OP ( cb, fa )
{
    _D = SET ( 7, _D ); 										   /* SET  7,D		  */
}
OP ( cb, fb )
{
    _E = SET ( 7, _E ); 										   /* SET  7,E		  */
}
OP ( cb, fc )
{
    _H = SET ( 7, _H ); 										   /* SET  7,H		  */
}
OP ( cb, fd )
{
    _L = SET ( 7, _L ); 										   /* SET  7,L		  */
}
OP ( cb, fe )
{
    WM ( _HL, SET ( 7, RM ( _HL ) ) );								   /* SET  7,(HL)	  */
}
OP ( cb, ff )
{
    _A = SET ( 7, _A ); 										   /* SET  7,A		  */
}


/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit operations with (IX+o)
**********************************************************/
OP ( xycb, 00 )
{
    _B = RLC ( RM ( Z80.EA ) );    /* RLC  B=(XY+o)	  */
    WM ( Z80.EA, _B );
}
OP ( xycb, 01 )
{
    _C = RLC ( RM ( Z80.EA ) );    /* RLC  C=(XY+o)	  */
    WM ( Z80.EA, _C );
}
OP ( xycb, 02 )
{
    _D = RLC ( RM ( Z80.EA ) );    /* RLC  D=(XY+o)	  */
    WM ( Z80.EA, _D );
}
OP ( xycb, 03 )
{
    _E = RLC ( RM ( Z80.EA ) );    /* RLC  E=(XY+o)	  */
    WM ( Z80.EA, _E );
}
OP ( xycb, 04 )
{
    _H = RLC ( RM ( Z80.EA ) );    /* RLC  H=(XY+o)	  */
    WM ( Z80.EA, _H );
}
OP ( xycb, 05 )
{
    _L = RLC ( RM ( Z80.EA ) );    /* RLC  L=(XY+o)	  */
    WM ( Z80.EA, _L );
}
OP ( xycb, 06 )
{
    WM ( Z80.EA, RLC ( RM ( Z80.EA ) ) );								   /* RLC  (XY+o)	  */
}
OP ( xycb, 07 )
{
    _A = RLC ( RM ( Z80.EA ) );    /* RLC  A=(XY+o)	  */
    WM ( Z80.EA, _A );
}

OP ( xycb, 08 )
{
    _B = RRC ( RM ( Z80.EA ) );    /* RRC  B=(XY+o)	  */
    WM ( Z80.EA, _B );
}
OP ( xycb, 09 )
{
    _C = RRC ( RM ( Z80.EA ) );    /* RRC  C=(XY+o)	  */
    WM ( Z80.EA, _C );
}
OP ( xycb, 0a )
{
    _D = RRC ( RM ( Z80.EA ) );    /* RRC  D=(XY+o)	  */
    WM ( Z80.EA, _D );
}
OP ( xycb, 0b )
{
    _E = RRC ( RM ( Z80.EA ) );    /* RRC  E=(XY+o)	  */
    WM ( Z80.EA, _E );
}
OP ( xycb, 0c )
{
    _H = RRC ( RM ( Z80.EA ) );    /* RRC  H=(XY+o)	  */
    WM ( Z80.EA, _H );
}
OP ( xycb, 0d )
{
    _L = RRC ( RM ( Z80.EA ) );    /* RRC  L=(XY+o)	  */
    WM ( Z80.EA, _L );
}
OP ( xycb, 0e )
{
    WM ( Z80.EA, RRC ( RM ( Z80.EA ) ) );								   /* RRC  (XY+o)	  */
}
OP ( xycb, 0f )
{
    _A = RRC ( RM ( Z80.EA ) );    /* RRC  A=(XY+o)	  */
    WM ( Z80.EA, _A );
}

OP ( xycb, 10 )
{
    _B = RL ( RM ( Z80.EA ) );    /* RL   B=(XY+o)	  */
    WM ( Z80.EA, _B );
}
OP ( xycb, 11 )
{
    _C = RL ( RM ( Z80.EA ) );    /* RL   C=(XY+o)	  */
    WM ( Z80.EA, _C );
}
OP ( xycb, 12 )
{
    _D = RL ( RM ( Z80.EA ) );    /* RL   D=(XY+o)	  */
    WM ( Z80.EA, _D );
}
OP ( xycb, 13 )
{
    _E = RL ( RM ( Z80.EA ) );    /* RL   E=(XY+o)	  */
    WM ( Z80.EA, _E );
}
OP ( xycb, 14 )
{
    _H = RL ( RM ( Z80.EA ) );    /* RL   H=(XY+o)	  */
    WM ( Z80.EA, _H );
}
OP ( xycb, 15 )
{
    _L = RL ( RM ( Z80.EA ) );    /* RL   L=(XY+o)	  */
    WM ( Z80.EA, _L );
}
OP ( xycb, 16 )
{
    WM ( Z80.EA, RL ( RM ( Z80.EA ) ) );								   /* RL   (XY+o)	  */
}
OP ( xycb, 17 )
{
    _A = RL ( RM ( Z80.EA ) );    /* RL   A=(XY+o)	  */
    WM ( Z80.EA, _A );
}

OP ( xycb, 18 )
{
    _B = RR ( RM ( Z80.EA ) );    /* RR   B=(XY+o)	  */
    WM ( Z80.EA, _B );
}
OP ( xycb, 19 )
{
    _C = RR ( RM ( Z80.EA ) );    /* RR   C=(XY+o)	  */
    WM ( Z80.EA, _C );
}
OP ( xycb, 1a )
{
    _D = RR ( RM ( Z80.EA ) );    /* RR   D=(XY+o)	  */
    WM ( Z80.EA, _D );
}
OP ( xycb, 1b )
{
    _E = RR ( RM ( Z80.EA ) );    /* RR   E=(XY+o)	  */
    WM ( Z80.EA, _E );
}
OP ( xycb, 1c )
{
    _H = RR ( RM ( Z80.EA ) );    /* RR   H=(XY+o)	  */
    WM ( Z80.EA, _H );
}
OP ( xycb, 1d )
{
    _L = RR ( RM ( Z80.EA ) );    /* RR   L=(XY+o)	  */
    WM ( Z80.EA, _L );
}
OP ( xycb, 1e )
{
    WM ( Z80.EA, RR ( RM ( Z80.EA ) ) );								   /* RR   (XY+o)	  */
}
OP ( xycb, 1f )
{
    _A = RR ( RM ( Z80.EA ) );    /* RR   A=(XY+o)	  */
    WM ( Z80.EA, _A );
}

OP ( xycb, 20 )
{
    _B = SLA ( RM ( Z80.EA ) );    /* SLA  B=(XY+o)	  */
    WM ( Z80.EA, _B );
}
OP ( xycb, 21 )
{
    _C = SLA ( RM ( Z80.EA ) );    /* SLA  C=(XY+o)	  */
    WM ( Z80.EA, _C );
}
OP ( xycb, 22 )
{
    _D = SLA ( RM ( Z80.EA ) );    /* SLA  D=(XY+o)	  */
    WM ( Z80.EA, _D );
}
OP ( xycb, 23 )
{
    _E = SLA ( RM ( Z80.EA ) );    /* SLA  E=(XY+o)	  */
    WM ( Z80.EA, _E );
}
OP ( xycb, 24 )
{
    _H = SLA ( RM ( Z80.EA ) );    /* SLA  H=(XY+o)	  */
    WM ( Z80.EA, _H );
}
OP ( xycb, 25 )
{
    _L = SLA ( RM ( Z80.EA ) );    /* SLA  L=(XY+o)	  */
    WM ( Z80.EA, _L );
}
OP ( xycb, 26 )
{
    WM ( Z80.EA, SLA ( RM ( Z80.EA ) ) );								   /* SLA  (XY+o)	  */
}
OP ( xycb, 27 )
{
    _A = SLA ( RM ( Z80.EA ) );    /* SLA  A=(XY+o)	  */
    WM ( Z80.EA, _A );
}

OP ( xycb, 28 )
{
    _B = SRA ( RM ( Z80.EA ) );    /* SRA  B=(XY+o)	  */
    WM ( Z80.EA, _B );
}
OP ( xycb, 29 )
{
    _C = SRA ( RM ( Z80.EA ) );    /* SRA  C=(XY+o)	  */
    WM ( Z80.EA, _C );
}
OP ( xycb, 2a )
{
    _D = SRA ( RM ( Z80.EA ) );    /* SRA  D=(XY+o)	  */
    WM ( Z80.EA, _D );
}
OP ( xycb, 2b )
{
    _E = SRA ( RM ( Z80.EA ) );    /* SRA  E=(XY+o)	  */
    WM ( Z80.EA, _E );
}
OP ( xycb, 2c )
{
    _H = SRA ( RM ( Z80.EA ) );    /* SRA  H=(XY+o)	  */
    WM ( Z80.EA, _H );
}
OP ( xycb, 2d )
{
    _L = SRA ( RM ( Z80.EA ) );    /* SRA  L=(XY+o)	  */
    WM ( Z80.EA, _L );
}
OP ( xycb, 2e )
{
    WM ( Z80.EA, SRA ( RM ( Z80.EA ) ) );								   /* SRA  (XY+o)	  */
}
OP ( xycb, 2f )
{
    _A = SRA ( RM ( Z80.EA ) );    /* SRA  A=(XY+o)	  */
    WM ( Z80.EA, _A );
}

OP ( xycb, 30 )
{
    _B = SLL ( RM ( Z80.EA ) );    /* SLL  B=(XY+o)	  */
    WM ( Z80.EA, _B );
}
OP ( xycb, 31 )
{
    _C = SLL ( RM ( Z80.EA ) );    /* SLL  C=(XY+o)	  */
    WM ( Z80.EA, _C );
}
OP ( xycb, 32 )
{
    _D = SLL ( RM ( Z80.EA ) );    /* SLL  D=(XY+o)	  */
    WM ( Z80.EA, _D );
}
OP ( xycb, 33 )
{
    _E = SLL ( RM ( Z80.EA ) );    /* SLL  E=(XY+o)	  */
    WM ( Z80.EA, _E );
}
OP ( xycb, 34 )
{
    _H = SLL ( RM ( Z80.EA ) );    /* SLL  H=(XY+o)	  */
    WM ( Z80.EA, _H );
}
OP ( xycb, 35 )
{
    _L = SLL ( RM ( Z80.EA ) );    /* SLL  L=(XY+o)	  */
    WM ( Z80.EA, _L );
}
OP ( xycb, 36 )
{
    WM ( Z80.EA, SLL ( RM ( Z80.EA ) ) );								   /* SLL  (XY+o)	  */
}
OP ( xycb, 37 )
{
    _A = SLL ( RM ( Z80.EA ) );    /* SLL  A=(XY+o)	  */
    WM ( Z80.EA, _A );
}

OP ( xycb, 38 )
{
    _B = SRL ( RM ( Z80.EA ) );    /* SRL  B=(XY+o)	  */
    WM ( Z80.EA, _B );
}
OP ( xycb, 39 )
{
    _C = SRL ( RM ( Z80.EA ) );    /* SRL  C=(XY+o)	  */
    WM ( Z80.EA, _C );
}
OP ( xycb, 3a )
{
    _D = SRL ( RM ( Z80.EA ) );    /* SRL  D=(XY+o)	  */
    WM ( Z80.EA, _D );
}
OP ( xycb, 3b )
{
    _E = SRL ( RM ( Z80.EA ) );    /* SRL  E=(XY+o)	  */
    WM ( Z80.EA, _E );
}
OP ( xycb, 3c )
{
    _H = SRL ( RM ( Z80.EA ) );    /* SRL  H=(XY+o)	  */
    WM ( Z80.EA, _H );
}
OP ( xycb, 3d )
{
    _L = SRL ( RM ( Z80.EA ) );    /* SRL  L=(XY+o)	  */
    WM ( Z80.EA, _L );
}
OP ( xycb, 3e )
{
    WM ( Z80.EA, SRL ( RM ( Z80.EA ) ) );								   /* SRL  (XY+o)	  */
}
OP ( xycb, 3f )
{
    _A = SRL ( RM ( Z80.EA ) );    /* SRL  A=(XY+o)	  */
    WM ( Z80.EA, _A );
}

OP ( xycb, 40 )
{
    xycb_46();											   /* BIT  0,B=(XY+o)  */
}
OP ( xycb, 41 )
{
    xycb_46();													     /* BIT	0,C=(XY+o)	*/
}
OP ( xycb, 42 )
{
    xycb_46();											   /* BIT  0,D=(XY+o)  */
}
OP ( xycb, 43 )
{
    xycb_46();											   /* BIT  0,E=(XY+o)  */
}
OP ( xycb, 44 )
{
    xycb_46();											   /* BIT  0,H=(XY+o)  */
}
OP ( xycb, 45 )
{
    xycb_46();											   /* BIT  0,L=(XY+o)  */
}
OP ( xycb, 46 )
{
    BIT_XY ( 0, RM ( Z80.EA ) ); 									   /* BIT  0,(XY+o)	  */
}
OP ( xycb, 47 )
{
    xycb_46();											   /* BIT  0,A=(XY+o)  */
}

OP ( xycb, 48 )
{
    xycb_4e();											   /* BIT  1,B=(XY+o)  */
}
OP ( xycb, 49 )
{
    xycb_4e();													     /* BIT	1,C=(XY+o)	*/
}
OP ( xycb, 4a )
{
    xycb_4e();											   /* BIT  1,D=(XY+o)  */
}
OP ( xycb, 4b )
{
    xycb_4e();											   /* BIT  1,E=(XY+o)  */
}
OP ( xycb, 4c )
{
    xycb_4e();											   /* BIT  1,H=(XY+o)  */
}
OP ( xycb, 4d )
{
    xycb_4e();											   /* BIT  1,L=(XY+o)  */
}
OP ( xycb, 4e )
{
    BIT_XY ( 1, RM ( Z80.EA ) ); 									   /* BIT  1,(XY+o)	  */
}
OP ( xycb, 4f )
{
    xycb_4e();											   /* BIT  1,A=(XY+o)  */
}

OP ( xycb, 50 )
{
    xycb_56();											   /* BIT  2,B=(XY+o)  */
}
OP ( xycb, 51 )
{
    xycb_56();													     /* BIT	2,C=(XY+o)	*/
}
OP ( xycb, 52 )
{
    xycb_56();											   /* BIT  2,D=(XY+o)  */
}
OP ( xycb, 53 )
{
    xycb_56();											   /* BIT  2,E=(XY+o)  */
}
OP ( xycb, 54 )
{
    xycb_56();											   /* BIT  2,H=(XY+o)  */
}
OP ( xycb, 55 )
{
    xycb_56();											   /* BIT  2,L=(XY+o)  */
}
OP ( xycb, 56 )
{
    BIT_XY ( 2, RM ( Z80.EA ) ); 									   /* BIT  2,(XY+o)	  */
}
OP ( xycb, 57 )
{
    xycb_56();											   /* BIT  2,A=(XY+o)  */
}

OP ( xycb, 58 )
{
    xycb_5e();											   /* BIT  3,B=(XY+o)  */
}
OP ( xycb, 59 )
{
    xycb_5e();													     /* BIT	3,C=(XY+o)	*/
}
OP ( xycb, 5a )
{
    xycb_5e();											   /* BIT  3,D=(XY+o)  */
}
OP ( xycb, 5b )
{
    xycb_5e();											   /* BIT  3,E=(XY+o)  */
}
OP ( xycb, 5c )
{
    xycb_5e();											   /* BIT  3,H=(XY+o)  */
}
OP ( xycb, 5d )
{
    xycb_5e();											   /* BIT  3,L=(XY+o)  */
}
OP ( xycb, 5e )
{
    BIT_XY ( 3, RM ( Z80.EA ) ); 									   /* BIT  3,(XY+o)	  */
}
OP ( xycb, 5f )
{
    xycb_5e();											   /* BIT  3,A=(XY+o)  */
}

OP ( xycb, 60 )
{
    xycb_66();											   /* BIT  4,B=(XY+o)  */
}
OP ( xycb, 61 )
{
    xycb_66();													     /* BIT	4,C=(XY+o)	*/
}
OP ( xycb, 62 )
{
    xycb_66();											   /* BIT  4,D=(XY+o)  */
}
OP ( xycb, 63 )
{
    xycb_66();											   /* BIT  4,E=(XY+o)  */
}
OP ( xycb, 64 )
{
    xycb_66();											   /* BIT  4,H=(XY+o)  */
}
OP ( xycb, 65 )
{
    xycb_66();											   /* BIT  4,L=(XY+o)  */
}
OP ( xycb, 66 )
{
    BIT_XY ( 4, RM ( Z80.EA ) ); 									   /* BIT  4,(XY+o)	  */
}
OP ( xycb, 67 )
{
    xycb_66();											   /* BIT  4,A=(XY+o)  */
}

OP ( xycb, 68 )
{
    xycb_6e();											   /* BIT  5,B=(XY+o)  */
}
OP ( xycb, 69 )
{
    xycb_6e();													     /* BIT	5,C=(XY+o)	*/
}
OP ( xycb, 6a )
{
    xycb_6e();											   /* BIT  5,D=(XY+o)  */
}
OP ( xycb, 6b )
{
    xycb_6e();											   /* BIT  5,E=(XY+o)  */
}
OP ( xycb, 6c )
{
    xycb_6e();											   /* BIT  5,H=(XY+o)  */
}
OP ( xycb, 6d )
{
    xycb_6e();											   /* BIT  5,L=(XY+o)  */
}
OP ( xycb, 6e )
{
    BIT_XY ( 5, RM ( Z80.EA ) ); 									   /* BIT  5,(XY+o)	  */
}
OP ( xycb, 6f )
{
    xycb_6e();											   /* BIT  5,A=(XY+o)  */
}

OP ( xycb, 70 )
{
    xycb_76();											   /* BIT  6,B=(XY+o)  */
}
OP ( xycb, 71 )
{
    xycb_76();													     /* BIT	6,C=(XY+o)	*/
}
OP ( xycb, 72 )
{
    xycb_76();											   /* BIT  6,D=(XY+o)  */
}
OP ( xycb, 73 )
{
    xycb_76();											   /* BIT  6,E=(XY+o)  */
}
OP ( xycb, 74 )
{
    xycb_76();											   /* BIT  6,H=(XY+o)  */
}
OP ( xycb, 75 )
{
    xycb_76();											   /* BIT  6,L=(XY+o)  */
}
OP ( xycb, 76 )
{
    BIT_XY ( 6, RM ( Z80.EA ) ); 									   /* BIT  6,(XY+o)	  */
}
OP ( xycb, 77 )
{
    xycb_76();											   /* BIT  6,A=(XY+o)  */
}

OP ( xycb, 78 )
{
    xycb_7e();											   /* BIT  7,B=(XY+o)  */
}
OP ( xycb, 79 )
{
    xycb_7e();													     /* BIT	7,C=(XY+o)	*/
}
OP ( xycb, 7a )
{
    xycb_7e();											   /* BIT  7,D=(XY+o)  */
}
OP ( xycb, 7b )
{
    xycb_7e();											   /* BIT  7,E=(XY+o)  */
}
OP ( xycb, 7c )
{
    xycb_7e();											   /* BIT  7,H=(XY+o)  */
}
OP ( xycb, 7d )
{
    xycb_7e();											   /* BIT  7,L=(XY+o)  */
}
OP ( xycb, 7e )
{
    BIT_XY ( 7, RM ( Z80.EA ) ); 									   /* BIT  7,(XY+o)	  */
}
OP ( xycb, 7f )
{
    xycb_7e();											   /* BIT  7,A=(XY+o)  */
}

OP ( xycb, 80 )
{
    _B = RES ( 0, RM ( Z80.EA ) );    /* RES  0,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, 81 )
{
    _C = RES ( 0, RM ( Z80.EA ) );    /* RES  0,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, 82 )
{
    _D = RES ( 0, RM ( Z80.EA ) );    /* RES  0,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, 83 )
{
    _E = RES ( 0, RM ( Z80.EA ) );    /* RES  0,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, 84 )
{
    _H = RES ( 0, RM ( Z80.EA ) );    /* RES  0,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, 85 )
{
    _L = RES ( 0, RM ( Z80.EA ) );    /* RES  0,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, 86 )
{
    WM ( Z80.EA, RES ( 0, RM ( Z80.EA ) ) );								   /* RES  0,(XY+o)	  */
}
OP ( xycb, 87 )
{
    _A = RES ( 0, RM ( Z80.EA ) );    /* RES  0,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, 88 )
{
    _B = RES ( 1, RM ( Z80.EA ) );    /* RES  1,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, 89 )
{
    _C = RES ( 1, RM ( Z80.EA ) );    /* RES  1,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, 8a )
{
    _D = RES ( 1, RM ( Z80.EA ) );    /* RES  1,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, 8b )
{
    _E = RES ( 1, RM ( Z80.EA ) );    /* RES  1,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, 8c )
{
    _H = RES ( 1, RM ( Z80.EA ) );    /* RES  1,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, 8d )
{
    _L = RES ( 1, RM ( Z80.EA ) );    /* RES  1,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, 8e )
{
    WM ( Z80.EA, RES ( 1, RM ( Z80.EA ) ) );								   /* RES  1,(XY+o)	  */
}
OP ( xycb, 8f )
{
    _A = RES ( 1, RM ( Z80.EA ) );    /* RES  1,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, 90 )
{
    _B = RES ( 2, RM ( Z80.EA ) );    /* RES  2,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, 91 )
{
    _C = RES ( 2, RM ( Z80.EA ) );    /* RES  2,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, 92 )
{
    _D = RES ( 2, RM ( Z80.EA ) );    /* RES  2,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, 93 )
{
    _E = RES ( 2, RM ( Z80.EA ) );    /* RES  2,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, 94 )
{
    _H = RES ( 2, RM ( Z80.EA ) );    /* RES  2,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, 95 )
{
    _L = RES ( 2, RM ( Z80.EA ) );    /* RES  2,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, 96 )
{
    WM ( Z80.EA, RES ( 2, RM ( Z80.EA ) ) );								   /* RES  2,(XY+o)	  */
}
OP ( xycb, 97 )
{
    _A = RES ( 2, RM ( Z80.EA ) );    /* RES  2,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, 98 )
{
    _B = RES ( 3, RM ( Z80.EA ) );    /* RES  3,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, 99 )
{
    _C = RES ( 3, RM ( Z80.EA ) );    /* RES  3,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, 9a )
{
    _D = RES ( 3, RM ( Z80.EA ) );    /* RES  3,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, 9b )
{
    _E = RES ( 3, RM ( Z80.EA ) );    /* RES  3,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, 9c )
{
    _H = RES ( 3, RM ( Z80.EA ) );    /* RES  3,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, 9d )
{
    _L = RES ( 3, RM ( Z80.EA ) );    /* RES  3,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, 9e )
{
    WM ( Z80.EA, RES ( 3, RM ( Z80.EA ) ) );								   /* RES  3,(XY+o)	  */
}
OP ( xycb, 9f )
{
    _A = RES ( 3, RM ( Z80.EA ) );    /* RES  3,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, a0 )
{
    _B = RES ( 4, RM ( Z80.EA ) );    /* RES  4,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, a1 )
{
    _C = RES ( 4, RM ( Z80.EA ) );    /* RES  4,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, a2 )
{
    _D = RES ( 4, RM ( Z80.EA ) );    /* RES  4,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, a3 )
{
    _E = RES ( 4, RM ( Z80.EA ) );    /* RES  4,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, a4 )
{
    _H = RES ( 4, RM ( Z80.EA ) );    /* RES  4,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, a5 )
{
    _L = RES ( 4, RM ( Z80.EA ) );    /* RES  4,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, a6 )
{
    WM ( Z80.EA, RES ( 4, RM ( Z80.EA ) ) );								   /* RES  4,(XY+o)	  */
}
OP ( xycb, a7 )
{
    _A = RES ( 4, RM ( Z80.EA ) );    /* RES  4,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, a8 )
{
    _B = RES ( 5, RM ( Z80.EA ) );    /* RES  5,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, a9 )
{
    _C = RES ( 5, RM ( Z80.EA ) );    /* RES  5,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, aa )
{
    _D = RES ( 5, RM ( Z80.EA ) );    /* RES  5,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, ab )
{
    _E = RES ( 5, RM ( Z80.EA ) );    /* RES  5,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, ac )
{
    _H = RES ( 5, RM ( Z80.EA ) );    /* RES  5,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, ad )
{
    _L = RES ( 5, RM ( Z80.EA ) );    /* RES  5,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, ae )
{
    WM ( Z80.EA, RES ( 5, RM ( Z80.EA ) ) );								   /* RES  5,(XY+o)	  */
}
OP ( xycb, af )
{
    _A = RES ( 5, RM ( Z80.EA ) );    /* RES  5,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, b0 )
{
    _B = RES ( 6, RM ( Z80.EA ) );    /* RES  6,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, b1 )
{
    _C = RES ( 6, RM ( Z80.EA ) );    /* RES  6,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, b2 )
{
    _D = RES ( 6, RM ( Z80.EA ) );    /* RES  6,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, b3 )
{
    _E = RES ( 6, RM ( Z80.EA ) );    /* RES  6,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, b4 )
{
    _H = RES ( 6, RM ( Z80.EA ) );    /* RES  6,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, b5 )
{
    _L = RES ( 6, RM ( Z80.EA ) );    /* RES  6,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, b6 )
{
    WM ( Z80.EA, RES ( 6, RM ( Z80.EA ) ) );								   /* RES  6,(XY+o)	  */
}
OP ( xycb, b7 )
{
    _A = RES ( 6, RM ( Z80.EA ) );    /* RES  6,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, b8 )
{
    _B = RES ( 7, RM ( Z80.EA ) );    /* RES  7,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, b9 )
{
    _C = RES ( 7, RM ( Z80.EA ) );    /* RES  7,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, ba )
{
    _D = RES ( 7, RM ( Z80.EA ) );    /* RES  7,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, bb )
{
    _E = RES ( 7, RM ( Z80.EA ) );    /* RES  7,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, bc )
{
    _H = RES ( 7, RM ( Z80.EA ) );    /* RES  7,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, bd )
{
    _L = RES ( 7, RM ( Z80.EA ) );    /* RES  7,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, be )
{
    WM ( Z80.EA, RES ( 7, RM ( Z80.EA ) ) );								   /* RES  7,(XY+o)	  */
}
OP ( xycb, bf )
{
    _A = RES ( 7, RM ( Z80.EA ) );    /* RES  7,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, c0 )
{
    _B = SET ( 0, RM ( Z80.EA ) );    /* SET  0,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, c1 )
{
    _C = SET ( 0, RM ( Z80.EA ) );    /* SET  0,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, c2 )
{
    _D = SET ( 0, RM ( Z80.EA ) );    /* SET  0,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, c3 )
{
    _E = SET ( 0, RM ( Z80.EA ) );    /* SET  0,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, c4 )
{
    _H = SET ( 0, RM ( Z80.EA ) );    /* SET  0,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, c5 )
{
    _L = SET ( 0, RM ( Z80.EA ) );    /* SET  0,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, c6 )
{
    WM ( Z80.EA, SET ( 0, RM ( Z80.EA ) ) );								   /* SET  0,(XY+o)	  */
}
OP ( xycb, c7 )
{
    _A = SET ( 0, RM ( Z80.EA ) );    /* SET  0,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, c8 )
{
    _B = SET ( 1, RM ( Z80.EA ) );    /* SET  1,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, c9 )
{
    _C = SET ( 1, RM ( Z80.EA ) );    /* SET  1,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, ca )
{
    _D = SET ( 1, RM ( Z80.EA ) );    /* SET  1,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, cb )
{
    _E = SET ( 1, RM ( Z80.EA ) );    /* SET  1,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, cc )
{
    _H = SET ( 1, RM ( Z80.EA ) );    /* SET  1,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, cd )
{
    _L = SET ( 1, RM ( Z80.EA ) );    /* SET  1,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, ce )
{
    WM ( Z80.EA, SET ( 1, RM ( Z80.EA ) ) );								   /* SET  1,(XY+o)	  */
}
OP ( xycb, cf )
{
    _A = SET ( 1, RM ( Z80.EA ) );    /* SET  1,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, d0 )
{
    _B = SET ( 2, RM ( Z80.EA ) );    /* SET  2,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, d1 )
{
    _C = SET ( 2, RM ( Z80.EA ) );    /* SET  2,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, d2 )
{
    _D = SET ( 2, RM ( Z80.EA ) );    /* SET  2,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, d3 )
{
    _E = SET ( 2, RM ( Z80.EA ) );    /* SET  2,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, d4 )
{
    _H = SET ( 2, RM ( Z80.EA ) );    /* SET  2,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, d5 )
{
    _L = SET ( 2, RM ( Z80.EA ) );    /* SET  2,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, d6 )
{
    WM ( Z80.EA, SET ( 2, RM ( Z80.EA ) ) );								   /* SET  2,(XY+o)	  */
}
OP ( xycb, d7 )
{
    _A = SET ( 2, RM ( Z80.EA ) );    /* SET  2,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, d8 )
{
    _B = SET ( 3, RM ( Z80.EA ) );    /* SET  3,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, d9 )
{
    _C = SET ( 3, RM ( Z80.EA ) );    /* SET  3,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, da )
{
    _D = SET ( 3, RM ( Z80.EA ) );    /* SET  3,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, db )
{
    _E = SET ( 3, RM ( Z80.EA ) );    /* SET  3,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, dc )
{
    _H = SET ( 3, RM ( Z80.EA ) );    /* SET  3,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, dd )
{
    _L = SET ( 3, RM ( Z80.EA ) );    /* SET  3,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, de )
{
    WM ( Z80.EA, SET ( 3, RM ( Z80.EA ) ) );								   /* SET  3,(XY+o)	  */
}
OP ( xycb, df )
{
    _A = SET ( 3, RM ( Z80.EA ) );    /* SET  3,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, e0 )
{
    _B = SET ( 4, RM ( Z80.EA ) );    /* SET  4,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, e1 )
{
    _C = SET ( 4, RM ( Z80.EA ) );    /* SET  4,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, e2 )
{
    _D = SET ( 4, RM ( Z80.EA ) );    /* SET  4,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, e3 )
{
    _E = SET ( 4, RM ( Z80.EA ) );    /* SET  4,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, e4 )
{
    _H = SET ( 4, RM ( Z80.EA ) );    /* SET  4,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, e5 )
{
    _L = SET ( 4, RM ( Z80.EA ) );    /* SET  4,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, e6 )
{
    WM ( Z80.EA, SET ( 4, RM ( Z80.EA ) ) );								   /* SET  4,(XY+o)	  */
}
OP ( xycb, e7 )
{
    _A = SET ( 4, RM ( Z80.EA ) );    /* SET  4,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, e8 )
{
    _B = SET ( 5, RM ( Z80.EA ) );    /* SET  5,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, e9 )
{
    _C = SET ( 5, RM ( Z80.EA ) );    /* SET  5,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, ea )
{
    _D = SET ( 5, RM ( Z80.EA ) );    /* SET  5,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, eb )
{
    _E = SET ( 5, RM ( Z80.EA ) );    /* SET  5,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, ec )
{
    _H = SET ( 5, RM ( Z80.EA ) );    /* SET  5,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, ed )
{
    _L = SET ( 5, RM ( Z80.EA ) );    /* SET  5,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, ee )
{
    WM ( Z80.EA, SET ( 5, RM ( Z80.EA ) ) );								   /* SET  5,(XY+o)	  */
}
OP ( xycb, ef )
{
    _A = SET ( 5, RM ( Z80.EA ) );    /* SET  5,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, f0 )
{
    _B = SET ( 6, RM ( Z80.EA ) );    /* SET  6,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, f1 )
{
    _C = SET ( 6, RM ( Z80.EA ) );    /* SET  6,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, f2 )
{
    _D = SET ( 6, RM ( Z80.EA ) );    /* SET  6,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, f3 )
{
    _E = SET ( 6, RM ( Z80.EA ) );    /* SET  6,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, f4 )
{
    _H = SET ( 6, RM ( Z80.EA ) );    /* SET  6,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, f5 )
{
    _L = SET ( 6, RM ( Z80.EA ) );    /* SET  6,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, f6 )
{
    WM ( Z80.EA, SET ( 6, RM ( Z80.EA ) ) );								   /* SET  6,(XY+o)	  */
}
OP ( xycb, f7 )
{
    _A = SET ( 6, RM ( Z80.EA ) );    /* SET  6,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( xycb, f8 )
{
    _B = SET ( 7, RM ( Z80.EA ) );    /* SET  7,B=(XY+o)  */
    WM ( Z80.EA, _B );
}
OP ( xycb, f9 )
{
    _C = SET ( 7, RM ( Z80.EA ) );    /* SET  7,C=(XY+o)  */
    WM ( Z80.EA, _C );
}
OP ( xycb, fa )
{
    _D = SET ( 7, RM ( Z80.EA ) );    /* SET  7,D=(XY+o)  */
    WM ( Z80.EA, _D );
}
OP ( xycb, fb )
{
    _E = SET ( 7, RM ( Z80.EA ) );    /* SET  7,E=(XY+o)  */
    WM ( Z80.EA, _E );
}
OP ( xycb, fc )
{
    _H = SET ( 7, RM ( Z80.EA ) );    /* SET  7,H=(XY+o)  */
    WM ( Z80.EA, _H );
}
OP ( xycb, fd )
{
    _L = SET ( 7, RM ( Z80.EA ) );    /* SET  7,L=(XY+o)  */
    WM ( Z80.EA, _L );
}
OP ( xycb, fe )
{
    WM ( Z80.EA, SET ( 7, RM ( Z80.EA ) ) );								   /* SET  7,(XY+o)	  */
}
OP ( xycb, ff )
{
    _A = SET ( 7, RM ( Z80.EA ) );    /* SET  7,A=(XY+o)  */
    WM ( Z80.EA, _A );
}

OP ( illegal, 1 )
{
    /* --- MP ---
       logerror("Z80 #%d ill. opcode $%02x $%02x\n",
       cpu_getactivecpu(), cpu_readop((_PCD-1)&0xffff), cpu_readop(_PCD));
    */
}

/**********************************************************
 * IX register related opcodes (DD prefix)
 **********************************************************/
OP ( dd, 00 )
{
    illegal_1();    /* DB   DD		  */
    op_00();
}
OP ( dd, 01 )
{
    illegal_1();    /* DB   DD		  */
    op_01();
}
OP ( dd, 02 )
{
    illegal_1();    /* DB   DD		  */
    op_02();
}
OP ( dd, 03 )
{
    illegal_1();    /* DB   DD		  */
    op_03();
}
OP ( dd, 04 )
{
    illegal_1();    /* DB   DD		  */
    op_04();
}
OP ( dd, 05 )
{
    illegal_1();    /* DB   DD		  */
    op_05();
}
OP ( dd, 06 )
{
    illegal_1();    /* DB   DD		  */
    op_06();
}
OP ( dd, 07 )
{
    illegal_1();    /* DB   DD		  */
    op_07();
}

OP ( dd, 08 )
{
    illegal_1();    /* DB   DD		  */
    op_08();
}
OP ( dd, 09 )
{
    _R++;    /* ADD  IX,BC 	  */
    ADD16 ( IX, BC );
}
OP ( dd, 0a )
{
    illegal_1();    /* DB   DD		  */
    op_0a();
}
OP ( dd, 0b )
{
    illegal_1();    /* DB   DD		  */
    op_0b();
}
OP ( dd, 0c )
{
    illegal_1();    /* DB   DD		  */
    op_0c();
}
OP ( dd, 0d )
{
    illegal_1();    /* DB   DD		  */
    op_0d();
}
OP ( dd, 0e )
{
    illegal_1();    /* DB   DD		  */
    op_0e();
}
OP ( dd, 0f )
{
    illegal_1();    /* DB   DD		  */
    op_0f();
}

OP ( dd, 10 )
{
    illegal_1();    /* DB   DD		  */
    op_10();
}
OP ( dd, 11 )
{
    illegal_1();    /* DB   DD		  */
    op_11();
}
OP ( dd, 12 )
{
    illegal_1();    /* DB   DD		  */
    op_12();
}
OP ( dd, 13 )
{
    illegal_1();    /* DB   DD		  */
    op_13();
}
OP ( dd, 14 )
{
    illegal_1();    /* DB   DD		  */
    op_14();
}
OP ( dd, 15 )
{
    illegal_1();    /* DB   DD		  */
    op_15();
}
OP ( dd, 16 )
{
    illegal_1();    /* DB   DD		  */
    op_16();
}
OP ( dd, 17 )
{
    illegal_1();    /* DB   DD		  */
    op_17();
}

OP ( dd, 18 )
{
    illegal_1();    /* DB   DD		  */
    op_18();
}
OP ( dd, 19 )
{
    _R++;    /* ADD  IX,DE 	  */
    ADD16 ( IX, DE );
}
OP ( dd, 1a )
{
    illegal_1();    /* DB   DD		  */
    op_1a();
}
OP ( dd, 1b )
{
    illegal_1();    /* DB   DD		  */
    op_1b();
}
OP ( dd, 1c )
{
    illegal_1();    /* DB   DD		  */
    op_1c();
}
OP ( dd, 1d )
{
    illegal_1();    /* DB   DD		  */
    op_1d();
}
OP ( dd, 1e )
{
    illegal_1();    /* DB   DD		  */
    op_1e();
}
OP ( dd, 1f )
{
    illegal_1();    /* DB   DD		  */
    op_1f();
}

OP ( dd, 20 )
{
    illegal_1();    /* DB   DD		  */
    op_20();
}
OP ( dd, 21 )
{
    _R++;    /* LD   IX,w		  */
    _IX = ARG16();
}
OP ( dd, 22 )
{
    _R++;    /* LD   (w),IX	  */
    Z80.EA = ARG16();
    WM16 ( Z80.EA, &Z80.IX );
}
OP ( dd, 23 )
{
    _R++;    /* INC  IX		  */
    _IX++;
}
OP ( dd, 24 )
{
    _R++;    /* INC  HX		  */
    _HX = INC ( _HX );
}
OP ( dd, 25 )
{
    _R++;    /* DEC  HX		  */
    _HX = DEC ( _HX );
}
OP ( dd, 26 )
{
    _R++;    /* LD   HX,n		  */
    _HX = ARG();
}
OP ( dd, 27 )
{
    illegal_1();    /* DB   DD		  */
    op_27();
}

OP ( dd, 28 )
{
    illegal_1();    /* DB   DD		  */
    op_28();
}
OP ( dd, 29 )
{
    _R++;    /* ADD  IX,IX 	  */
    ADD16 ( IX, IX );
}
OP ( dd, 2a )
{
    _R++;    /* LD   IX,(w)	  */
    Z80.EA = ARG16();
    RM16 ( Z80.EA, &Z80.IX );
}
OP ( dd, 2b )
{
    _R++;    /* DEC  IX		  */
    _IX--;
}
OP ( dd, 2c )
{
    _R++;    /* INC  LX		  */
    _LX = INC ( _LX );
}
OP ( dd, 2d )
{
    _R++;    /* DEC  LX		  */
    _LX = DEC ( _LX );
}
OP ( dd, 2e )
{
    _R++;    /* LD   LX,n		  */
    _LX = ARG();
}
OP ( dd, 2f )
{
    illegal_1();    /* DB   DD		  */
    op_2f();
}

OP ( dd, 30 )
{
    illegal_1();    /* DB   DD		  */
    op_30();
}
OP ( dd, 31 )
{
    illegal_1();    /* DB   DD		  */
    op_31();
}
OP ( dd, 32 )
{
    illegal_1();    /* DB   DD		  */
    op_32();
}
OP ( dd, 33 )
{
    illegal_1();    /* DB   DD		  */
    op_33();
}
OP ( dd, 34 )
{
    _R++;    /* INC  (IX+o)	  */
    EAX;
    WM ( Z80.EA, INC ( RM ( Z80.EA ) ) );
}
OP ( dd, 35 )
{
    _R++;    /* DEC  (IX+o)	  */
    EAX;
    WM ( Z80.EA, DEC ( RM ( Z80.EA ) ) );
}
OP ( dd, 36 )
{
    _R++;    /* LD   (IX+o),n	  */
    EAX;
    WM ( Z80.EA, ARG() );
}
OP ( dd, 37 )
{
    illegal_1();    /* DB   DD		  */
    op_37();
}

OP ( dd, 38 )
{
    illegal_1();    /* DB   DD		  */
    op_38();
}
OP ( dd, 39 )
{
    _R++;    /* ADD  IX,SP 	  */
    ADD16 ( IX, SP );
}
OP ( dd, 3a )
{
    illegal_1();    /* DB   DD		  */
    op_3a();
}
OP ( dd, 3b )
{
    illegal_1();    /* DB   DD		  */
    op_3b();
}
OP ( dd, 3c )
{
    illegal_1();    /* DB   DD		  */
    op_3c();
}
OP ( dd, 3d )
{
    illegal_1();    /* DB   DD		  */
    op_3d();
}
OP ( dd, 3e )
{
    illegal_1();    /* DB   DD		  */
    op_3e();
}
OP ( dd, 3f )
{
    illegal_1();    /* DB   DD		  */
    op_3f();
}

OP ( dd, 40 )
{
    illegal_1();    /* DB   DD		  */
    op_40();
}
OP ( dd, 41 )
{
    illegal_1();    /* DB   DD		  */
    op_41();
}
OP ( dd, 42 )
{
    illegal_1();    /* DB   DD		  */
    op_42();
}
OP ( dd, 43 )
{
    illegal_1();    /* DB   DD		  */
    op_43();
}
OP ( dd, 44 )
{
    _R++;    /* LD   B,HX		  */
    _B = _HX;
}
OP ( dd, 45 )
{
    _R++;    /* LD   B,LX		  */
    _B = _LX;
}
OP ( dd, 46 )
{
    _R++;    /* LD   B,(IX+o)	  */
    EAX;
    _B = RM ( Z80.EA );
}
OP ( dd, 47 )
{
    illegal_1();    /* DB   DD		  */
    op_47();
}

OP ( dd, 48 )
{
    illegal_1();    /* DB   DD		  */
    op_48();
}
OP ( dd, 49 )
{
    illegal_1();    /* DB   DD		  */
    op_49();
}
OP ( dd, 4a )
{
    illegal_1();    /* DB   DD		  */
    op_4a();
}
OP ( dd, 4b )
{
    illegal_1();    /* DB   DD		  */
    op_4b();
}
OP ( dd, 4c )
{
    _R++;    /* LD   C,HX		  */
    _C = _HX;
}
OP ( dd, 4d )
{
    _R++;    /* LD   C,LX		  */
    _C = _LX;
}
OP ( dd, 4e )
{
    _R++;    /* LD   C,(IX+o)	  */
    EAX;
    _C = RM ( Z80.EA );
}
OP ( dd, 4f )
{
    illegal_1();    /* DB   DD		  */
    op_4f();
}

OP ( dd, 50 )
{
    illegal_1();    /* DB   DD		  */
    op_50();
}
OP ( dd, 51 )
{
    illegal_1();    /* DB   DD		  */
    op_51();
}
OP ( dd, 52 )
{
    illegal_1();    /* DB   DD		  */
    op_52();
}
OP ( dd, 53 )
{
    illegal_1();    /* DB   DD		  */
    op_53();
}
OP ( dd, 54 )
{
    _R++;    /* LD   D,HX		  */
    _D = _HX;
}
OP ( dd, 55 )
{
    _R++;    /* LD   D,LX		  */
    _D = _LX;
}
OP ( dd, 56 )
{
    _R++;    /* LD   D,(IX+o)	  */
    EAX;
    _D = RM ( Z80.EA );
}
OP ( dd, 57 )
{
    illegal_1();    /* DB   DD		  */
    op_57();
}

OP ( dd, 58 )
{
    illegal_1();    /* DB   DD		  */
    op_58();
}
OP ( dd, 59 )
{
    illegal_1();    /* DB   DD		  */
    op_59();
}
OP ( dd, 5a )
{
    illegal_1();    /* DB   DD		  */
    op_5a();
}
OP ( dd, 5b )
{
    illegal_1();    /* DB   DD		  */
    op_5b();
}
OP ( dd, 5c )
{
    _R++;    /* LD   E,HX		  */
    _E = _HX;
}
OP ( dd, 5d )
{
    _R++;    /* LD   E,LX		  */
    _E = _LX;
}
OP ( dd, 5e )
{
    _R++;    /* LD   E,(IX+o)	  */
    EAX;
    _E = RM ( Z80.EA );
}
OP ( dd, 5f )
{
    illegal_1();    /* DB   DD		  */
    op_5f();
}

OP ( dd, 60 )
{
    _R++;    /* LD   HX,B		  */
    _HX = _B;
}
OP ( dd, 61 )
{
    _R++;    /* LD   HX,C		  */
    _HX = _C;
}
OP ( dd, 62 )
{
    _R++;    /* LD   HX,D		  */
    _HX = _D;
}
OP ( dd, 63 )
{
    _R++;    /* LD   HX,E		  */
    _HX = _E;
}
OP ( dd, 64 ) { 														} /* LD   HX,HX 	  */
OP ( dd, 65 )
{
    _R++;    /* LD   HX,LX 	  */
    _HX = _LX;
}
OP ( dd, 66 )
{
    _R++;    /* LD   H,(IX+o)	  */
    EAX;
    _H = RM ( Z80.EA );
}
OP ( dd, 67 )
{
    _R++;    /* LD   HX,A		  */
    _HX = _A;
}

OP ( dd, 68 )
{
    _R++;    /* LD   LX,B		  */
    _LX = _B;
}
OP ( dd, 69 )
{
    _R++;    /* LD   LX,C		  */
    _LX = _C;
}
OP ( dd, 6a )
{
    _R++;    /* LD   LX,D		  */
    _LX = _D;
}
OP ( dd, 6b )
{
    _R++;    /* LD   LX,E		  */
    _LX = _E;
}
OP ( dd, 6c )
{
    _R++;    /* LD   LX,HX 	  */
    _LX = _HX;
}
OP ( dd, 6d ) { 														} /* LD   LX,LX 	  */
OP ( dd, 6e )
{
    _R++;    /* LD   L,(IX+o)	  */
    EAX;
    _L = RM ( Z80.EA );
}
OP ( dd, 6f )
{
    _R++;    /* LD   LX,A		  */
    _LX = _A;
}

OP ( dd, 70 )
{
    _R++;    /* LD   (IX+o),B	  */
    EAX;
    WM ( Z80.EA, _B );
}
OP ( dd, 71 )
{
    _R++;    /* LD   (IX+o),C	  */
    EAX;
    WM ( Z80.EA, _C );
}
OP ( dd, 72 )
{
    _R++;    /* LD   (IX+o),D	  */
    EAX;
    WM ( Z80.EA, _D );
}
OP ( dd, 73 )
{
    _R++;    /* LD   (IX+o),E	  */
    EAX;
    WM ( Z80.EA, _E );
}
OP ( dd, 74 )
{
    _R++;    /* LD   (IX+o),H	  */
    EAX;
    WM ( Z80.EA, _H );
}
OP ( dd, 75 )
{
    _R++;    /* LD   (IX+o),L	  */
    EAX;
    WM ( Z80.EA, _L );
}
OP ( dd, 76 )
{
    illegal_1();    /* DB   DD		  */
    op_76();
}
OP ( dd, 77 )
{
    _R++;    /* LD   (IX+o),A	  */
    EAX;
    WM ( Z80.EA, _A );
}

OP ( dd, 78 )
{
    illegal_1();    /* DB   DD		  */
    op_78();
}
OP ( dd, 79 )
{
    illegal_1();    /* DB   DD		  */
    op_79();
}
OP ( dd, 7a )
{
    illegal_1();    /* DB   DD		  */
    op_7a();
}
OP ( dd, 7b )
{
    illegal_1();    /* DB   DD		  */
    op_7b();
}
OP ( dd, 7c )
{
    _R++;    /* LD   A,HX		  */
    _A = _HX;
}
OP ( dd, 7d )
{
    _R++;    /* LD   A,LX		  */
    _A = _LX;
}
OP ( dd, 7e )
{
    _R++;    /* LD   A,(IX+o)	  */
    EAX;
    _A = RM ( Z80.EA );
}
OP ( dd, 7f )
{
    illegal_1();    /* DB   DD		  */
    op_7f();
}

OP ( dd, 80 )
{
    illegal_1();    /* DB   DD		  */
    op_80();
}
OP ( dd, 81 )
{
    illegal_1();    /* DB   DD		  */
    op_81();
}
OP ( dd, 82 )
{
    illegal_1();    /* DB   DD		  */
    op_82();
}
OP ( dd, 83 )
{
    illegal_1();    /* DB   DD		  */
    op_83();
}
OP ( dd, 84 )
{
    _R++;    /* ADD  A,HX		  */
    ADD ( _HX );
}
OP ( dd, 85 )
{
    _R++;    /* ADD  A,LX		  */
    ADD ( _LX );
}
OP ( dd, 86 )
{
    _R++;    /* ADD  A,(IX+o)	  */
    EAX;
    ADD ( RM ( Z80.EA ) );
}
OP ( dd, 87 )
{
    illegal_1();    /* DB   DD		  */
    op_87();
}

OP ( dd, 88 )
{
    illegal_1();    /* DB   DD		  */
    op_88();
}
OP ( dd, 89 )
{
    illegal_1();    /* DB   DD		  */
    op_89();
}
OP ( dd, 8a )
{
    illegal_1();    /* DB   DD		  */
    op_8a();
}
OP ( dd, 8b )
{
    illegal_1();    /* DB   DD		  */
    op_8b();
}
OP ( dd, 8c )
{
    _R++;    /* ADC  A,HX		  */
    ADC ( _HX );
}
OP ( dd, 8d )
{
    _R++;    /* ADC  A,LX		  */
    ADC ( _LX );
}
OP ( dd, 8e )
{
    _R++;    /* ADC  A,(IX+o)	  */
    EAX;
    ADC ( RM ( Z80.EA ) );
}
OP ( dd, 8f )
{
    illegal_1();    /* DB   DD		  */
    op_8f();
}

OP ( dd, 90 )
{
    illegal_1();    /* DB   DD		  */
    op_90();
}
OP ( dd, 91 )
{
    illegal_1();    /* DB   DD		  */
    op_91();
}
OP ( dd, 92 )
{
    illegal_1();    /* DB   DD		  */
    op_92();
}
OP ( dd, 93 )
{
    illegal_1();    /* DB   DD		  */
    op_93();
}
OP ( dd, 94 )
{
    _R++;    /* SUB  HX		  */
    SUB ( _HX );
}
OP ( dd, 95 )
{
    _R++;    /* SUB  LX		  */
    SUB ( _LX );
}
OP ( dd, 96 )
{
    _R++;    /* SUB  (IX+o)	  */
    EAX;
    SUB ( RM ( Z80.EA ) );
}
OP ( dd, 97 )
{
    illegal_1();    /* DB   DD		  */
    op_97();
}

OP ( dd, 98 )
{
    illegal_1();    /* DB   DD		  */
    op_98();
}
OP ( dd, 99 )
{
    illegal_1();    /* DB   DD		  */
    op_99();
}
OP ( dd, 9a )
{
    illegal_1();    /* DB   DD		  */
    op_9a();
}
OP ( dd, 9b )
{
    illegal_1();    /* DB   DD		  */
    op_9b();
}
OP ( dd, 9c )
{
    _R++;    /* SBC  A,HX		  */
    SBC ( _HX );
}
OP ( dd, 9d )
{
    _R++;    /* SBC  A,LX		  */
    SBC ( _LX );
}
OP ( dd, 9e )
{
    _R++;    /* SBC  A,(IX+o)	  */
    EAX;
    SBC ( RM ( Z80.EA ) );
}
OP ( dd, 9f )
{
    illegal_1();    /* DB   DD		  */
    op_9f();
}

OP ( dd, a0 )
{
    illegal_1();    /* DB   DD		  */
    op_a0();
}
OP ( dd, a1 )
{
    illegal_1();    /* DB   DD		  */
    op_a1();
}
OP ( dd, a2 )
{
    illegal_1();    /* DB   DD		  */
    op_a2();
}
OP ( dd, a3 )
{
    illegal_1();    /* DB   DD		  */
    op_a3();
}
OP ( dd, a4 )
{
    _R++;    /* AND  HX		  */
    AND ( _HX );
}
OP ( dd, a5 )
{
    _R++;    /* AND  LX		  */
    AND ( _LX );
}
OP ( dd, a6 )
{
    _R++;    /* AND  (IX+o)	  */
    EAX;
    AND ( RM ( Z80.EA ) );
}
OP ( dd, a7 )
{
    illegal_1();    /* DB   DD		  */
    op_a7();
}

OP ( dd, a8 )
{
    illegal_1();    /* DB   DD		  */
    op_a8();
}
OP ( dd, a9 )
{
    illegal_1();    /* DB   DD		  */
    op_a9();
}
OP ( dd, aa )
{
    illegal_1();    /* DB   DD		  */
    op_aa();
}
OP ( dd, ab )
{
    illegal_1();    /* DB   DD		  */
    op_ab();
}
OP ( dd, ac )
{
    _R++;    /* XOR  HX		  */
    XOR ( _HX );
}
OP ( dd, ad )
{
    _R++;    /* XOR  LX		  */
    XOR ( _LX );
}
OP ( dd, ae )
{
    _R++;    /* XOR  (IX+o)	  */
    EAX;
    XOR ( RM ( Z80.EA ) );
}
OP ( dd, af )
{
    illegal_1();    /* DB   DD		  */
    op_af();
}

OP ( dd, b0 )
{
    illegal_1();    /* DB   DD		  */
    op_b0();
}
OP ( dd, b1 )
{
    illegal_1();    /* DB   DD		  */
    op_b1();
}
OP ( dd, b2 )
{
    illegal_1();    /* DB   DD		  */
    op_b2();
}
OP ( dd, b3 )
{
    illegal_1();    /* DB   DD		  */
    op_b3();
}
OP ( dd, b4 )
{
    _R++;    /* OR   HX		  */
    OR ( _HX );
}
OP ( dd, b5 )
{
    _R++;    /* OR   LX		  */
    OR ( _LX );
}
OP ( dd, b6 )
{
    _R++;    /* OR   (IX+o)	  */
    EAX;
    OR ( RM ( Z80.EA ) );
}
OP ( dd, b7 )
{
    illegal_1();    /* DB   DD		  */
    op_b7();
}

OP ( dd, b8 )
{
    illegal_1();    /* DB   DD		  */
    op_b8();
}
OP ( dd, b9 )
{
    illegal_1();    /* DB   DD		  */
    op_b9();
}
OP ( dd, ba )
{
    illegal_1();    /* DB   DD		  */
    op_ba();
}
OP ( dd, bb )
{
    illegal_1();    /* DB   DD		  */
    op_bb();
}
OP ( dd, bc )
{
    _R++;    /* CP   HX		  */
    CP ( _HX );
}
OP ( dd, bd )
{
    _R++;    /* CP   LX		  */
    CP ( _LX );
}
OP ( dd, be )
{
    _R++;    /* CP   (IX+o)	  */
    EAX;
    CP ( RM ( Z80.EA ) );
}
OP ( dd, bf )
{
    illegal_1();    /* DB   DD		  */
    op_bf();
}

OP ( dd, c0 )
{
    illegal_1();    /* DB   DD		  */
    op_c0();
}
OP ( dd, c1 )
{
    illegal_1();    /* DB   DD		  */
    op_c1();
}
OP ( dd, c2 )
{
    illegal_1();    /* DB   DD		  */
    op_c2();
}
OP ( dd, c3 )
{
    illegal_1();    /* DB   DD		  */
    op_c3();
}
OP ( dd, c4 )
{
    illegal_1();    /* DB   DD		  */
    op_c4();
}
OP ( dd, c5 )
{
    illegal_1();    /* DB   DD		  */
    op_c5();
}
OP ( dd, c6 )
{
    illegal_1();    /* DB   DD		  */
    op_c6();
}
OP ( dd, c7 )
{
    illegal_1();    /* DB   DD		  */
    op_c7();
}

OP ( dd, c8 )
{
    illegal_1();    /* DB   DD		  */
    op_c8();
}
OP ( dd, c9 )
{
    illegal_1();    /* DB   DD		  */
    op_c9();
}
OP ( dd, ca )
{
    illegal_1();    /* DB   DD		  */
    op_ca();
}
OP ( dd, cb )
{
    _R++;    /* **   DD CB xx	  */
    EAX;
    EXEC ( xycb, ARG() );
}
OP ( dd, cc )
{
    illegal_1();    /* DB   DD		  */
    op_cc();
}
OP ( dd, cd )
{
    illegal_1();    /* DB   DD		  */
    op_cd();
}
OP ( dd, ce )
{
    illegal_1();    /* DB   DD		  */
    op_ce();
}
OP ( dd, cf )
{
    illegal_1();    /* DB   DD		  */
    op_cf();
}

OP ( dd, d0 )
{
    illegal_1();    /* DB   DD		  */
    op_d0();
}
OP ( dd, d1 )
{
    illegal_1();    /* DB   DD		  */
    op_d1();
}
OP ( dd, d2 )
{
    illegal_1();    /* DB   DD		  */
    op_d2();
}
OP ( dd, d3 )
{
    illegal_1();    /* DB   DD		  */
    op_d3();
}
OP ( dd, d4 )
{
    illegal_1();    /* DB   DD		  */
    op_d4();
}
OP ( dd, d5 )
{
    illegal_1();    /* DB   DD		  */
    op_d5();
}
OP ( dd, d6 )
{
    illegal_1();    /* DB   DD		  */
    op_d6();
}
OP ( dd, d7 )
{
    illegal_1();    /* DB   DD		  */
    op_d7();
}

OP ( dd, d8 )
{
    illegal_1();    /* DB   DD		  */
    op_d8();
}
OP ( dd, d9 )
{
    illegal_1();    /* DB   DD		  */
    op_d9();
}
OP ( dd, da )
{
    illegal_1();    /* DB   DD		  */
    op_da();
}
OP ( dd, db )
{
    illegal_1();    /* DB   DD		  */
    op_db();
}
OP ( dd, dc )
{
    illegal_1();    /* DB   DD		  */
    op_dc();
}
OP ( dd, dd )
{
    illegal_1();    /* DB   DD		  */
    op_dd();
}
OP ( dd, de )
{
    illegal_1();    /* DB   DD		  */
    op_de();
}
OP ( dd, df )
{
    illegal_1();    /* DB   DD		  */
    op_df();
}

OP ( dd, e0 )
{
    illegal_1();    /* DB   DD		  */
    op_e0();
}
OP ( dd, e1 )
{
    _R++;    /* POP  IX		  */
    POP ( IX );
}
OP ( dd, e2 )
{
    illegal_1();    /* DB   DD		  */
    op_e2();
}
OP ( dd, e3 )
{
    _R++;    /* EX   (SP),IX	  */
    EXSP ( IX );
}
OP ( dd, e4 )
{
    illegal_1();    /* DB   DD		  */
    op_e4();
}
OP ( dd, e5 )
{
    _R++;    /* PUSH IX		  */
    PUSH ( IX );
}
OP ( dd, e6 )
{
    illegal_1();    /* DB   DD		  */
    op_e6();
}
OP ( dd, e7 )
{
    illegal_1();    /* DB   DD		  */
    op_e7();
}

OP ( dd, e8 )
{
    illegal_1();    /* DB   DD		  */
    op_e8();
}
OP ( dd, e9 )
{
    _R++;    /* JP   (IX)		  */
    _PC = _IX;
    change_pc16 ( _PCD );
}
OP ( dd, ea )
{
    illegal_1();    /* DB   DD		  */
    op_ea();
}
OP ( dd, eb )
{
    illegal_1();    /* DB   DD		  */
    op_eb();
}
OP ( dd, ec )
{
    illegal_1();    /* DB   DD		  */
    op_ec();
}
OP ( dd, ed )
{
    illegal_1();    /* DB   DD		  */
    op_ed();
}
OP ( dd, ee )
{
    illegal_1();    /* DB   DD		  */
    op_ee();
}
OP ( dd, ef )
{
    illegal_1();    /* DB   DD		  */
    op_ef();
}

OP ( dd, f0 )
{
    illegal_1();    /* DB   DD		  */
    op_f0();
}
OP ( dd, f1 )
{
    illegal_1();    /* DB   DD		  */
    op_f1();
}
OP ( dd, f2 )
{
    illegal_1();    /* DB   DD		  */
    op_f2();
}
OP ( dd, f3 )
{
    illegal_1();    /* DB   DD		  */
    op_f3();
}
OP ( dd, f4 )
{
    illegal_1();    /* DB   DD		  */
    op_f4();
}
OP ( dd, f5 )
{
    illegal_1();    /* DB   DD		  */
    op_f5();
}
OP ( dd, f6 )
{
    illegal_1();    /* DB   DD		  */
    op_f6();
}
OP ( dd, f7 )
{
    illegal_1();    /* DB   DD		  */
    op_f7();
}

OP ( dd, f8 )
{
    illegal_1();    /* DB   DD		  */
    op_f8();
}
OP ( dd, f9 )
{
    _R++;    /* LD   SP,IX 	  */
    _SP = _IX;
}
OP ( dd, fa )
{
    illegal_1();    /* DB   DD		  */
    op_fa();
}
OP ( dd, fb )
{
    illegal_1();    /* DB   DD		  */
    op_fb();
}
OP ( dd, fc )
{
    illegal_1();    /* DB   DD		  */
    op_fc();
}
OP ( dd, fd )
{
    illegal_1();    /* DB   DD		  */
    op_fd();
}
OP ( dd, fe )
{
    illegal_1();    /* DB   DD		  */
    op_fe();
}
OP ( dd, ff )
{
    illegal_1();    /* DB   DD		  */
    op_ff();
}

/**********************************************************
 * IY register related opcodes (FD prefix)
 **********************************************************/
OP ( fd, 00 )
{
    illegal_1();    /* DB   FD		  */
    op_00();
}
OP ( fd, 01 )
{
    illegal_1();    /* DB   FD		  */
    op_01();
}
OP ( fd, 02 )
{
    illegal_1();    /* DB   FD		  */
    op_02();
}
OP ( fd, 03 )
{
    illegal_1();    /* DB   FD		  */
    op_03();
}
OP ( fd, 04 )
{
    illegal_1();    /* DB   FD		  */
    op_04();
}
OP ( fd, 05 )
{
    illegal_1();    /* DB   FD		  */
    op_05();
}
OP ( fd, 06 )
{
    illegal_1();    /* DB   FD		  */
    op_06();
}
OP ( fd, 07 )
{
    illegal_1();    /* DB   FD		  */
    op_07();
}

OP ( fd, 08 )
{
    illegal_1();    /* DB   FD		  */
    op_08();
}
OP ( fd, 09 )
{
    _R++;    /* ADD  IY,BC 	  */
    ADD16 ( IY, BC );
}
OP ( fd, 0a )
{
    illegal_1();    /* DB   FD		  */
    op_0a();
}
OP ( fd, 0b )
{
    illegal_1();    /* DB   FD		  */
    op_0b();
}
OP ( fd, 0c )
{
    illegal_1();    /* DB   FD		  */
    op_0c();
}
OP ( fd, 0d )
{
    illegal_1();    /* DB   FD		  */
    op_0d();
}
OP ( fd, 0e )
{
    illegal_1();    /* DB   FD		  */
    op_0e();
}
OP ( fd, 0f )
{
    illegal_1();    /* DB   FD		  */
    op_0f();
}

OP ( fd, 10 )
{
    illegal_1();    /* DB   FD		  */
    op_10();
}
OP ( fd, 11 )
{
    illegal_1();    /* DB   FD		  */
    op_11();
}
OP ( fd, 12 )
{
    illegal_1();    /* DB   FD		  */
    op_12();
}
OP ( fd, 13 )
{
    illegal_1();    /* DB   FD		  */
    op_13();
}
OP ( fd, 14 )
{
    illegal_1();    /* DB   FD		  */
    op_14();
}
OP ( fd, 15 )
{
    illegal_1();    /* DB   FD		  */
    op_15();
}
OP ( fd, 16 )
{
    illegal_1();    /* DB   FD		  */
    op_16();
}
OP ( fd, 17 )
{
    illegal_1();    /* DB   FD		  */
    op_17();
}

OP ( fd, 18 )
{
    illegal_1();    /* DB   FD		  */
    op_18();
}
OP ( fd, 19 )
{
    _R++;    /* ADD  IY,DE 	  */
    ADD16 ( IY, DE );
}
OP ( fd, 1a )
{
    illegal_1();    /* DB   FD		  */
    op_1a();
}
OP ( fd, 1b )
{
    illegal_1();    /* DB   FD		  */
    op_1b();
}
OP ( fd, 1c )
{
    illegal_1();    /* DB   FD		  */
    op_1c();
}
OP ( fd, 1d )
{
    illegal_1();    /* DB   FD		  */
    op_1d();
}
OP ( fd, 1e )
{
    illegal_1();    /* DB   FD		  */
    op_1e();
}
OP ( fd, 1f )
{
    illegal_1();    /* DB   FD		  */
    op_1f();
}

OP ( fd, 20 )
{
    illegal_1();    /* DB   FD		  */
    op_20();
}
OP ( fd, 21 )
{
    _R++;    /* LD   IY,w		  */
    _IY = ARG16();
}
OP ( fd, 22 )
{
    _R++;    /* LD   (w),IY	  */
    Z80.EA = ARG16();
    WM16 ( Z80.EA, &Z80.IY );
}
OP ( fd, 23 )
{
    _R++;    /* INC  IY		  */
    _IY++;
}
OP ( fd, 24 )
{
    _R++;    /* INC  HY		  */
    _HY = INC ( _HY );
}
OP ( fd, 25 )
{
    _R++;    /* DEC  HY		  */
    _HY = DEC ( _HY );
}
OP ( fd, 26 )
{
    _R++;    /* LD   HY,n		  */
    _HY = ARG();
}
OP ( fd, 27 )
{
    illegal_1();    /* DB   FD		  */
    op_27();
}

OP ( fd, 28 )
{
    illegal_1();    /* DB   FD		  */
    op_28();
}
OP ( fd, 29 )
{
    _R++;    /* ADD  IY,IY 	  */
    ADD16 ( IY, IY );
}
OP ( fd, 2a )
{
    _R++;    /* LD   IY,(w)	  */
    Z80.EA = ARG16();
    RM16 ( Z80.EA, &Z80.IY );
}
OP ( fd, 2b )
{
    _R++;    /* DEC  IY		  */
    _IY--;
}
OP ( fd, 2c )
{
    _R++;    /* INC  LY		  */
    _LY = INC ( _LY );
}
OP ( fd, 2d )
{
    _R++;    /* DEC  LY		  */
    _LY = DEC ( _LY );
}
OP ( fd, 2e )
{
    _R++;    /* LD   LY,n		  */
    _LY = ARG();
}
OP ( fd, 2f )
{
    illegal_1();    /* DB   FD		  */
    op_2f();
}

OP ( fd, 30 )
{
    illegal_1();    /* DB   FD		  */
    op_30();
}
OP ( fd, 31 )
{
    illegal_1();    /* DB   FD		  */
    op_31();
}
OP ( fd, 32 )
{
    illegal_1();    /* DB   FD		  */
    op_32();
}
OP ( fd, 33 )
{
    illegal_1();    /* DB   FD		  */
    op_33();
}
OP ( fd, 34 )
{
    _R++;    /* INC  (IY+o)	  */
    EAY;
    WM ( Z80.EA, INC ( RM ( Z80.EA ) ) );
}
OP ( fd, 35 )
{
    _R++;    /* DEC  (IY+o)	  */
    EAY;
    WM ( Z80.EA, DEC ( RM ( Z80.EA ) ) );
}
OP ( fd, 36 )
{
    _R++;    /* LD   (IY+o),n	  */
    EAY;
    WM ( Z80.EA, ARG() );
}
OP ( fd, 37 )
{
    illegal_1();    /* DB   FD		  */
    op_37();
}

OP ( fd, 38 )
{
    illegal_1();    /* DB   FD		  */
    op_38();
}
OP ( fd, 39 )
{
    _R++;    /* ADD  IY,SP 	  */
    ADD16 ( IY, SP );
}
OP ( fd, 3a )
{
    illegal_1();    /* DB   FD		  */
    op_3a();
}
OP ( fd, 3b )
{
    illegal_1();    /* DB   FD		  */
    op_3b();
}
OP ( fd, 3c )
{
    illegal_1();    /* DB   FD		  */
    op_3c();
}
OP ( fd, 3d )
{
    illegal_1();    /* DB   FD		  */
    op_3d();
}
OP ( fd, 3e )
{
    illegal_1();    /* DB   FD		  */
    op_3e();
}
OP ( fd, 3f )
{
    illegal_1();    /* DB   FD		  */
    op_3f();
}

OP ( fd, 40 )
{
    illegal_1();    /* DB   FD		  */
    op_40();
}
OP ( fd, 41 )
{
    illegal_1();    /* DB   FD		  */
    op_41();
}
OP ( fd, 42 )
{
    illegal_1();    /* DB   FD		  */
    op_42();
}
OP ( fd, 43 )
{
    illegal_1();    /* DB   FD		  */
    op_43();
}
OP ( fd, 44 )
{
    _R++;    /* LD   B,HY		  */
    _B = _HY;
}
OP ( fd, 45 )
{
    _R++;    /* LD   B,LY		  */
    _B = _LY;
}
OP ( fd, 46 )
{
    _R++;    /* LD   B,(IY+o)	  */
    EAY;
    _B = RM ( Z80.EA );
}
OP ( fd, 47 )
{
    illegal_1();    /* DB   FD		  */
    op_47();
}

OP ( fd, 48 )
{
    illegal_1();    /* DB   FD		  */
    op_48();
}
OP ( fd, 49 )
{
    illegal_1();    /* DB   FD		  */
    op_49();
}
OP ( fd, 4a )
{
    illegal_1();    /* DB   FD		  */
    op_4a();
}
OP ( fd, 4b )
{
    illegal_1();    /* DB   FD		  */
    op_4b();
}
OP ( fd, 4c )
{
    _R++;    /* LD   C,HY		  */
    _C = _HY;
}
OP ( fd, 4d )
{
    _R++;    /* LD   C,LY		  */
    _C = _LY;
}
OP ( fd, 4e )
{
    _R++;    /* LD   C,(IY+o)	  */
    EAY;
    _C = RM ( Z80.EA );
}
OP ( fd, 4f )
{
    illegal_1();    /* DB   FD		  */
    op_4f();
}

OP ( fd, 50 )
{
    illegal_1();    /* DB   FD		  */
    op_50();
}
OP ( fd, 51 )
{
    illegal_1();    /* DB   FD		  */
    op_51();
}
OP ( fd, 52 )
{
    illegal_1();    /* DB   FD		  */
    op_52();
}
OP ( fd, 53 )
{
    illegal_1();    /* DB   FD		  */
    op_53();
}
OP ( fd, 54 )
{
    _R++;    /* LD   D,HY		  */
    _D = _HY;
}
OP ( fd, 55 )
{
    _R++;    /* LD   D,LY		  */
    _D = _LY;
}
OP ( fd, 56 )
{
    _R++;    /* LD   D,(IY+o)	  */
    EAY;
    _D = RM ( Z80.EA );
}
OP ( fd, 57 )
{
    illegal_1();    /* DB   FD		  */
    op_57();
}

OP ( fd, 58 )
{
    illegal_1();    /* DB   FD		  */
    op_58();
}
OP ( fd, 59 )
{
    illegal_1();    /* DB   FD		  */
    op_59();
}
OP ( fd, 5a )
{
    illegal_1();    /* DB   FD		  */
    op_5a();
}
OP ( fd, 5b )
{
    illegal_1();    /* DB   FD		  */
    op_5b();
}
OP ( fd, 5c )
{
    _R++;    /* LD   E,HY		  */
    _E = _HY;
}
OP ( fd, 5d )
{
    _R++;    /* LD   E,LY		  */
    _E = _LY;
}
OP ( fd, 5e )
{
    _R++;    /* LD   E,(IY+o)	  */
    EAY;
    _E = RM ( Z80.EA );
}
OP ( fd, 5f )
{
    illegal_1();    /* DB   FD		  */
    op_5f();
}

OP ( fd, 60 )
{
    _R++;    /* LD   HY,B		  */
    _HY = _B;
}
OP ( fd, 61 )
{
    _R++;    /* LD   HY,C		  */
    _HY = _C;
}
OP ( fd, 62 )
{
    _R++;    /* LD   HY,D		  */
    _HY = _D;
}
OP ( fd, 63 )
{
    _R++;    /* LD   HY,E		  */
    _HY = _E;
}
OP ( fd, 64 )
{
    _R++;													   /* LD   HY,HY 	  */
}
OP ( fd, 65 )
{
    _R++;    /* LD   HY,LY 	  */
    _HY = _LY;
}
OP ( fd, 66 )
{
    _R++;    /* LD   H,(IY+o)	  */
    EAY;
    _H = RM ( Z80.EA );
}
OP ( fd, 67 )
{
    _R++;    /* LD   HY,A		  */
    _HY = _A;
}

OP ( fd, 68 )
{
    _R++;    /* LD   LY,B		  */
    _LY = _B;
}
OP ( fd, 69 )
{
    _R++;    /* LD   LY,C		  */
    _LY = _C;
}
OP ( fd, 6a )
{
    _R++;    /* LD   LY,D		  */
    _LY = _D;
}
OP ( fd, 6b )
{
    _R++;    /* LD   LY,E		  */
    _LY = _E;
}
OP ( fd, 6c )
{
    _R++;    /* LD   LY,HY 	  */
    _LY = _HY;
}
OP ( fd, 6d )
{
    _R++;													   /* LD   LY,LY 	  */
}
OP ( fd, 6e )
{
    _R++;    /* LD   L,(IY+o)	  */
    EAY;
    _L = RM ( Z80.EA );
}
OP ( fd, 6f )
{
    _R++;    /* LD   LY,A		  */
    _LY = _A;
}

OP ( fd, 70 )
{
    _R++;    /* LD   (IY+o),B	  */
    EAY;
    WM ( Z80.EA, _B );
}
OP ( fd, 71 )
{
    _R++;    /* LD   (IY+o),C	  */
    EAY;
    WM ( Z80.EA, _C );
}
OP ( fd, 72 )
{
    _R++;    /* LD   (IY+o),D	  */
    EAY;
    WM ( Z80.EA, _D );
}
OP ( fd, 73 )
{
    _R++;    /* LD   (IY+o),E	  */
    EAY;
    WM ( Z80.EA, _E );
}
OP ( fd, 74 )
{
    _R++;    /* LD   (IY+o),H	  */
    EAY;
    WM ( Z80.EA, _H );
}
OP ( fd, 75 )
{
    _R++;    /* LD   (IY+o),L	  */
    EAY;
    WM ( Z80.EA, _L );
}
OP ( fd, 76 )
{
    illegal_1();    /* DB   FD		  */
    op_76();
}
OP ( fd, 77 )
{
    _R++;    /* LD   (IY+o),A	  */
    EAY;
    WM ( Z80.EA, _A );
}

OP ( fd, 78 )
{
    illegal_1();    /* DB   FD		  */
    op_78();
}
OP ( fd, 79 )
{
    illegal_1();    /* DB   FD		  */
    op_79();
}
OP ( fd, 7a )
{
    illegal_1();    /* DB   FD		  */
    op_7a();
}
OP ( fd, 7b )
{
    illegal_1();    /* DB   FD		  */
    op_7b();
}
OP ( fd, 7c )
{
    _R++;    /* LD   A,HY		  */
    _A = _HY;
}
OP ( fd, 7d )
{
    _R++;    /* LD   A,LY		  */
    _A = _LY;
}
OP ( fd, 7e )
{
    _R++;    /* LD   A,(IY+o)	  */
    EAY;
    _A = RM ( Z80.EA );
}
OP ( fd, 7f )
{
    illegal_1();    /* DB   FD		  */
    op_7f();
}

OP ( fd, 80 )
{
    illegal_1();    /* DB   FD		  */
    op_80();
}
OP ( fd, 81 )
{
    illegal_1();    /* DB   FD		  */
    op_81();
}
OP ( fd, 82 )
{
    illegal_1();    /* DB   FD		  */
    op_82();
}
OP ( fd, 83 )
{
    illegal_1();    /* DB   FD		  */
    op_83();
}
OP ( fd, 84 )
{
    _R++;    /* ADD  A,HY		  */
    ADD ( _HY );
}
OP ( fd, 85 )
{
    _R++;    /* ADD  A,LY		  */
    ADD ( _LY );
}
OP ( fd, 86 )
{
    _R++;    /* ADD  A,(IY+o)	  */
    EAY;
    ADD ( RM ( Z80.EA ) );
}
OP ( fd, 87 )
{
    illegal_1();    /* DB   FD		  */
    op_87();
}

OP ( fd, 88 )
{
    illegal_1();    /* DB   FD		  */
    op_88();
}
OP ( fd, 89 )
{
    illegal_1();    /* DB   FD		  */
    op_89();
}
OP ( fd, 8a )
{
    illegal_1();    /* DB   FD		  */
    op_8a();
}
OP ( fd, 8b )
{
    illegal_1();    /* DB   FD		  */
    op_8b();
}
OP ( fd, 8c )
{
    _R++;    /* ADC  A,HY		  */
    ADC ( _HY );
}
OP ( fd, 8d )
{
    _R++;    /* ADC  A,LY		  */
    ADC ( _LY );
}
OP ( fd, 8e )
{
    _R++;    /* ADC  A,(IY+o)	  */
    EAY;
    ADC ( RM ( Z80.EA ) );
}
OP ( fd, 8f )
{
    illegal_1();    /* DB   FD		  */
    op_8f();
}

OP ( fd, 90 )
{
    illegal_1();    /* DB   FD		  */
    op_90();
}
OP ( fd, 91 )
{
    illegal_1();    /* DB   FD		  */
    op_91();
}
OP ( fd, 92 )
{
    illegal_1();    /* DB   FD		  */
    op_92();
}
OP ( fd, 93 )
{
    illegal_1();    /* DB   FD		  */
    op_93();
}
OP ( fd, 94 )
{
    _R++;    /* SUB  HY		  */
    SUB ( _HY );
}
OP ( fd, 95 )
{
    _R++;    /* SUB  LY		  */
    SUB ( _LY );
}
OP ( fd, 96 )
{
    _R++;    /* SUB  (IY+o)	  */
    EAY;
    SUB ( RM ( Z80.EA ) );
}
OP ( fd, 97 )
{
    illegal_1();    /* DB   FD		  */
    op_97();
}

OP ( fd, 98 )
{
    illegal_1();    /* DB   FD		  */
    op_98();
}
OP ( fd, 99 )
{
    illegal_1();    /* DB   FD		  */
    op_99();
}
OP ( fd, 9a )
{
    illegal_1();    /* DB   FD		  */
    op_9a();
}
OP ( fd, 9b )
{
    illegal_1();    /* DB   FD		  */
    op_9b();
}
OP ( fd, 9c )
{
    _R++;    /* SBC  A,HY		  */
    SBC ( _HY );
}
OP ( fd, 9d )
{
    _R++;    /* SBC  A,LY		  */
    SBC ( _LY );
}
OP ( fd, 9e )
{
    _R++;    /* SBC  A,(IY+o)	  */
    EAY;
    SBC ( RM ( Z80.EA ) );
}
OP ( fd, 9f )
{
    illegal_1();    /* DB   FD		  */
    op_9f();
}

OP ( fd, a0 )
{
    illegal_1();    /* DB   FD		  */
    op_a0();
}
OP ( fd, a1 )
{
    illegal_1();    /* DB   FD		  */
    op_a1();
}
OP ( fd, a2 )
{
    illegal_1();    /* DB   FD		  */
    op_a2();
}
OP ( fd, a3 )
{
    illegal_1();    /* DB   FD		  */
    op_a3();
}
OP ( fd, a4 )
{
    _R++;    /* AND  HY		  */
    AND ( _HY );
}
OP ( fd, a5 )
{
    _R++;    /* AND  LY		  */
    AND ( _LY );
}
OP ( fd, a6 )
{
    _R++;    /* AND  (IY+o)	  */
    EAY;
    AND ( RM ( Z80.EA ) );
}
OP ( fd, a7 )
{
    illegal_1();    /* DB   FD		  */
    op_a7();
}

OP ( fd, a8 )
{
    illegal_1();    /* DB   FD		  */
    op_a8();
}
OP ( fd, a9 )
{
    illegal_1();    /* DB   FD		  */
    op_a9();
}
OP ( fd, aa )
{
    illegal_1();    /* DB   FD		  */
    op_aa();
}
OP ( fd, ab )
{
    illegal_1();    /* DB   FD		  */
    op_ab();
}
OP ( fd, ac )
{
    _R++;    /* XOR  HY		  */
    XOR ( _HY );
}
OP ( fd, ad )
{
    _R++;    /* XOR  LY		  */
    XOR ( _LY );
}
OP ( fd, ae )
{
    _R++;    /* XOR  (IY+o)	  */
    EAY;
    XOR ( RM ( Z80.EA ) );
}
OP ( fd, af )
{
    illegal_1();    /* DB   FD		  */
    op_af();
}

OP ( fd, b0 )
{
    illegal_1();    /* DB   FD		  */
    op_b0();
}
OP ( fd, b1 )
{
    illegal_1();    /* DB   FD		  */
    op_b1();
}
OP ( fd, b2 )
{
    illegal_1();    /* DB   FD		  */
    op_b2();
}
OP ( fd, b3 )
{
    illegal_1();    /* DB   FD		  */
    op_b3();
}
OP ( fd, b4 )
{
    _R++;    /* OR   HY		  */
    OR ( _HY );
}
OP ( fd, b5 )
{
    _R++;    /* OR   LY		  */
    OR ( _LY );
}
OP ( fd, b6 )
{
    _R++;    /* OR   (IY+o)	  */
    EAY;
    OR ( RM ( Z80.EA ) );
}
OP ( fd, b7 )
{
    illegal_1();    /* DB   FD		  */
    op_b7();
}

OP ( fd, b8 )
{
    illegal_1();    /* DB   FD		  */
    op_b8();
}
OP ( fd, b9 )
{
    illegal_1();    /* DB   FD		  */
    op_b9();
}
OP ( fd, ba )
{
    illegal_1();    /* DB   FD		  */
    op_ba();
}
OP ( fd, bb )
{
    illegal_1();    /* DB   FD		  */
    op_bb();
}
OP ( fd, bc )
{
    _R++;    /* CP   HY		  */
    CP ( _HY );
}
OP ( fd, bd )
{
    _R++;    /* CP   LY		  */
    CP ( _LY );
}
OP ( fd, be )
{
    _R++;    /* CP   (IY+o)	  */
    EAY;
    CP ( RM ( Z80.EA ) );
}
OP ( fd, bf )
{
    illegal_1();    /* DB   FD		  */
    op_bf();
}

OP ( fd, c0 )
{
    illegal_1();    /* DB   FD		  */
    op_c0();
}
OP ( fd, c1 )
{
    illegal_1();    /* DB   FD		  */
    op_c1();
}
OP ( fd, c2 )
{
    illegal_1();    /* DB   FD		  */
    op_c2();
}
OP ( fd, c3 )
{
    illegal_1();    /* DB   FD		  */
    op_c3();
}
OP ( fd, c4 )
{
    illegal_1();    /* DB   FD		  */
    op_c4();
}
OP ( fd, c5 )
{
    illegal_1();    /* DB   FD		  */
    op_c5();
}
OP ( fd, c6 )
{
    illegal_1();    /* DB   FD		  */
    op_c6();
}
OP ( fd, c7 )
{
    illegal_1();    /* DB   FD		  */
    op_c7();
}

OP ( fd, c8 )
{
    illegal_1();    /* DB   FD		  */
    op_c8();
}
OP ( fd, c9 )
{
    illegal_1();    /* DB   FD		  */
    op_c9();
}
OP ( fd, ca )
{
    illegal_1();    /* DB   FD		  */
    op_ca();
}
OP ( fd, cb )
{
    _R++;    /* **   FD CB xx	  */
    EAY;
    EXEC ( xycb, ARG() );
}
OP ( fd, cc )
{
    illegal_1();    /* DB   FD		  */
    op_cc();
}
OP ( fd, cd )
{
    illegal_1();    /* DB   FD		  */
    op_cd();
}
OP ( fd, ce )
{
    illegal_1();    /* DB   FD		  */
    op_ce();
}
OP ( fd, cf )
{
    illegal_1();    /* DB   FD		  */
    op_cf();
}

OP ( fd, d0 )
{
    illegal_1();    /* DB   FD		  */
    op_d0();
}
OP ( fd, d1 )
{
    illegal_1();    /* DB   FD		  */
    op_d1();
}
OP ( fd, d2 )
{
    illegal_1();    /* DB   FD		  */
    op_d2();
}
OP ( fd, d3 )
{
    illegal_1();    /* DB   FD		  */
    op_d3();
}
OP ( fd, d4 )
{
    illegal_1();    /* DB   FD		  */
    op_d4();
}
OP ( fd, d5 )
{
    illegal_1();    /* DB   FD		  */
    op_d5();
}
OP ( fd, d6 )
{
    illegal_1();    /* DB   FD		  */
    op_d6();
}
OP ( fd, d7 )
{
    illegal_1();    /* DB   FD		  */
    op_d7();
}

OP ( fd, d8 )
{
    illegal_1();    /* DB   FD		  */
    op_d8();
}
OP ( fd, d9 )
{
    illegal_1();    /* DB   FD		  */
    op_d9();
}
OP ( fd, da )
{
    illegal_1();    /* DB   FD		  */
    op_da();
}
OP ( fd, db )
{
    illegal_1();    /* DB   FD		  */
    op_db();
}
OP ( fd, dc )
{
    illegal_1();    /* DB   FD		  */
    op_dc();
}
OP ( fd, dd )
{
    illegal_1();    /* DB   FD		  */
    op_dd();
}
OP ( fd, de )
{
    illegal_1();    /* DB   FD		  */
    op_de();
}
OP ( fd, df )
{
    illegal_1();    /* DB   FD		  */
    op_df();
}

OP ( fd, e0 )
{
    illegal_1();    /* DB   FD		  */
    op_e0();
}
OP ( fd, e1 )
{
    _R++;    /* POP  IY		  */
    POP ( IY );
}
OP ( fd, e2 )
{
    illegal_1();    /* DB   FD		  */
    op_e2();
}
OP ( fd, e3 )
{
    _R++;    /* EX   (SP),IY	  */
    EXSP ( IY );
}
OP ( fd, e4 )
{
    illegal_1();    /* DB   FD		  */
    op_e4();
}
OP ( fd, e5 )
{
    _R++;    /* PUSH IY		  */
    PUSH ( IY );
}
OP ( fd, e6 )
{
    illegal_1();    /* DB   FD		  */
    op_e6();
}
OP ( fd, e7 )
{
    illegal_1();    /* DB   FD		  */
    op_e7();
}

OP ( fd, e8 )
{
    illegal_1();    /* DB   FD		  */
    op_e8();
}
OP ( fd, e9 )
{
    _R++;    /* JP   (IY)		  */
    _PC = _IY;
    change_pc16 ( _PCD );
}
OP ( fd, ea )
{
    illegal_1();    /* DB   FD		  */
    op_ea();
}
OP ( fd, eb )
{
    illegal_1();    /* DB   FD		  */
    op_eb();
}
OP ( fd, ec )
{
    illegal_1();    /* DB   FD		  */
    op_ec();
}
OP ( fd, ed )
{
    illegal_1();    /* DB   FD		  */
    op_ed();
}
OP ( fd, ee )
{
    illegal_1();    /* DB   FD		  */
    op_ee();
}
OP ( fd, ef )
{
    illegal_1();    /* DB   FD		  */
    op_ef();
}

OP ( fd, f0 )
{
    illegal_1();    /* DB   FD		  */
    op_f0();
}
OP ( fd, f1 )
{
    illegal_1();    /* DB   FD		  */
    op_f1();
}
OP ( fd, f2 )
{
    illegal_1();    /* DB   FD		  */
    op_f2();
}
OP ( fd, f3 )
{
    illegal_1();    /* DB   FD		  */
    op_f3();
}
OP ( fd, f4 )
{
    illegal_1();    /* DB   FD		  */
    op_f4();
}
OP ( fd, f5 )
{
    illegal_1();    /* DB   FD		  */
    op_f5();
}
OP ( fd, f6 )
{
    illegal_1();    /* DB   FD		  */
    op_f6();
}
OP ( fd, f7 )
{
    illegal_1();    /* DB   FD		  */
    op_f7();
}

OP ( fd, f8 )
{
    illegal_1();    /* DB   FD		  */
    op_f8();
}
OP ( fd, f9 )
{
    _R++;    /* LD   SP,IY 	  */
    _SP = _IY;
}
OP ( fd, fa )
{
    illegal_1();    /* DB   FD		  */
    op_fa();
}
OP ( fd, fb )
{
    illegal_1();    /* DB   FD		  */
    op_fb();
}
OP ( fd, fc )
{
    illegal_1();    /* DB   FD		  */
    op_fc();
}
OP ( fd, fd )
{
    illegal_1();    /* DB   FD		  */
    op_fd();
}
OP ( fd, fe )
{
    illegal_1();    /* DB   FD		  */
    op_fe();
}
OP ( fd, ff )
{
    illegal_1();    /* DB   FD		  */
    op_ff();
}

OP ( illegal, 2 )
{
    /* --- MP ---
        logerror("Z80 #%d ill. opcode $ed $%02x\n",
        cpu_getactivecpu(), cpu_readop((_PCD-1)&0xffff));
    */
}

/**********************************************************
 * special opcodes (ED prefix)
 **********************************************************/
OP ( ed, 00 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 01 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 02 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 03 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 04 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 05 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 06 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 07 )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, 08 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 09 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 0a )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 0b )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 0c )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 0d )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 0e )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 0f )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, 10 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 11 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 12 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 13 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 14 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 15 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 16 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 17 )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, 18 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 19 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 1a )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 1b )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 1c )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 1d )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 1e )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 1f )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, 20 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 21 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 22 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 23 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 24 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 25 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 26 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 27 )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, 28 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 29 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 2a )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 2b )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 2c )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 2d )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 2e )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 2f )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, 30 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 31 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 32 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 33 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 34 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 35 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 36 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 37 )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, 38 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 39 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 3a )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 3b )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 3c )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 3d )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 3e )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 3f )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, 40 )
{
    _B = IN ( _BC );    /* IN   B,(C) 	  */
    _F = ( _F & CF ) | SZP[_B];
}
OP ( ed, 41 )
{
    OUT ( _BC, _B );											   /* OUT  (C),B 	  */
}
OP ( ed, 42 )
{
    SBC16 ( BC );											   /* SBC  HL,BC 	  */
}
OP ( ed, 43 )
{
    Z80.EA = ARG16();    /* LD   (w),BC	  */
    WM16 ( Z80.EA, &Z80.BC );
}
OP ( ed, 44 )
{
    NEG;													   /* NEG			  */
}
OP ( ed, 45 )
{
    RETN;													   /* RETN;			  */
}
OP ( ed, 46 )
{
    _IM = 0;												   /* IM   0 		  */
}
OP ( ed, 47 )
{
    LD_I_A; 												   /* LD   I,A		  */
}

OP ( ed, 48 )
{
    _C = IN ( _BC );    /* IN   C,(C) 	  */
    _F = ( _F & CF ) | SZP[_C];
}
OP ( ed, 49 )
{
    OUT ( _BC, _C );											   /* OUT  (C),C 	  */
}
OP ( ed, 4a )
{
    ADC16 ( BC );											   /* ADC  HL,BC 	  */
}
OP ( ed, 4b )
{
    Z80.EA = ARG16();    /* LD   BC,(w)	  */
    RM16 ( Z80.EA, &Z80.BC );
}
OP ( ed, 4c )
{
    NEG;													   /* NEG			  */
}
OP ( ed, 4d )
{
    RETI;													   /* RETI			  */
}
OP ( ed, 4e )
{
    _IM = 0;												   /* IM   0 		  */
}
OP ( ed, 4f )
{
    LD_R_A; 												   /* LD   R,A		  */
}

OP ( ed, 50 )
{
    _D = IN ( _BC );    /* IN   D,(C) 	  */
    _F = ( _F & CF ) | SZP[_D];
}
OP ( ed, 51 )
{
    OUT ( _BC, _D );											   /* OUT  (C),D 	  */
}
OP ( ed, 52 )
{
    SBC16 ( DE );											   /* SBC  HL,DE 	  */
}
OP ( ed, 53 )
{
    Z80.EA = ARG16();    /* LD   (w),DE	  */
    WM16 ( Z80.EA, &Z80.DE );
}
OP ( ed, 54 )
{
    NEG;													   /* NEG			  */
}
OP ( ed, 55 )
{
    RETN;													   /* RETN;			  */
}
OP ( ed, 56 )
{
    _IM = 1;												   /* IM   1 		  */
}
OP ( ed, 57 )
{
    LD_A_I; 												   /* LD   A,I		  */
}

OP ( ed, 58 )
{
    _E = IN ( _BC );    /* IN   E,(C) 	  */
    _F = ( _F & CF ) | SZP[_E];
}
OP ( ed, 59 )
{
    OUT ( _BC, _E );											   /* OUT  (C),E 	  */
}
OP ( ed, 5a )
{
    ADC16 ( DE );											   /* ADC  HL,DE 	  */
}
OP ( ed, 5b )
{
    Z80.EA = ARG16();    /* LD   DE,(w)	  */
    RM16 ( Z80.EA, &Z80.DE );
}
OP ( ed, 5c )
{
    NEG;													   /* NEG			  */
}
OP ( ed, 5d )
{
    RETI;													   /* RETI			  */
}
OP ( ed, 5e )
{
    _IM = 2;												   /* IM   2 		  */
}
OP ( ed, 5f )
{
    LD_A_R; 												   /* LD   A,R		  */
}

OP ( ed, 60 )
{
    _H = IN ( _BC );    /* IN   H,(C) 	  */
    _F = ( _F & CF ) | SZP[_H];
}
OP ( ed, 61 )
{
    OUT ( _BC, _H );											   /* OUT  (C),H 	  */
}
OP ( ed, 62 )
{
    SBC16 ( HL );											   /* SBC  HL,HL 	  */
}
OP ( ed, 63 )
{
    Z80.EA = ARG16();    /* LD   (w),HL	  */
    WM16 ( Z80.EA, &Z80.HL );
}
OP ( ed, 64 )
{
    NEG;													   /* NEG			  */
}
OP ( ed, 65 )
{
    RETN;													   /* RETN;			  */
}
OP ( ed, 66 )
{
    _IM = 0;												   /* IM   0 		  */
}
OP ( ed, 67 )
{
    RRD;													   /* RRD  (HL)		  */
}

OP ( ed, 68 )
{
    _L = IN ( _BC );    /* IN   L,(C) 	  */
    _F = ( _F & CF ) | SZP[_L];
}
OP ( ed, 69 )
{
    OUT ( _BC, _L );											   /* OUT  (C),L 	  */
}
OP ( ed, 6a )
{
    ADC16 ( HL );											   /* ADC  HL,HL 	  */
}
OP ( ed, 6b )
{
    Z80.EA = ARG16();    /* LD   HL,(w)	  */
    RM16 ( Z80.EA, &Z80.HL );
}
OP ( ed, 6c )
{
    NEG;													   /* NEG			  */
}
OP ( ed, 6d )
{
    RETI;													   /* RETI			  */
}
OP ( ed, 6e )
{
    _IM = 0;												   /* IM   0 		  */
}
OP ( ed, 6f )
{
    RLD;													   /* RLD  (HL)		  */
}

OP ( ed, 70 )
{
    UINT8 res = IN ( _BC );    /* IN   0,(C) 	  */
    _F = ( _F & CF ) | SZP[res];
}
OP ( ed, 71 )
{
    OUT ( _BC, 0 ); 											   /* OUT  (C),0 	  */
}
OP ( ed, 72 )
{
    SBC16 ( SP );											   /* SBC  HL,SP 	  */
}
OP ( ed, 73 )
{
    Z80.EA = ARG16();    /* LD   (w),SP	  */
    WM16 ( Z80.EA, &Z80.SP );
}
OP ( ed, 74 )
{
    NEG;													   /* NEG			  */
}
OP ( ed, 75 )
{
    RETN;													   /* RETN;			  */
}
OP ( ed, 76 )
{
    _IM = 1;												   /* IM   1 		  */
}
OP ( ed, 77 )
{
    illegal_2();											   /* DB   ED,77 	  */
}

OP ( ed, 78 )
{
    _A = IN ( _BC );    /* IN   E,(C) 	  */
    _F = ( _F & CF ) | SZP[_A];
}
OP ( ed, 79 )
{
    OUT ( _BC, _A );											   /* OUT  (C),E 	  */
}
OP ( ed, 7a )
{
    ADC16 ( SP );											   /* ADC  HL,SP 	  */
}
OP ( ed, 7b )
{
    Z80.EA = ARG16();    /* LD   SP,(w)	  */
    RM16 ( Z80.EA, &Z80.SP );
}
OP ( ed, 7c )
{
    NEG;													   /* NEG			  */
}
OP ( ed, 7d )
{
    RETI;													   /* RETI			  */
}
OP ( ed, 7e )
{
    _IM = 2;												   /* IM   2 		  */
}
OP ( ed, 7f )
{
    illegal_2();											   /* DB   ED,7F 	  */
}

OP ( ed, 80 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 81 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 82 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 83 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 84 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 85 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 86 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 87 )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, 88 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 89 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 8a )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 8b )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 8c )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 8d )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 8e )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 8f )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, 90 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 91 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 92 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 93 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 94 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 95 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 96 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 97 )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, 98 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 99 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 9a )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 9b )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 9c )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 9d )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 9e )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, 9f )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, a0 )
{
    LDI;													   /* LDI			  */
}
OP ( ed, a1 )
{
    CPI;													   /* CPI			  */
}
OP ( ed, a2 )
{
    INI;													   /* INI			  */
}
OP ( ed, a3 )
{
    OUTI;													   /* OUTI			  */
}
OP ( ed, a4 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, a5 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, a6 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, a7 )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, a8 )
{
    LDD;													   /* LDD			  */
}
OP ( ed, a9 )
{
    CPD;													   /* CPD			  */
}
OP ( ed, aa )
{
    IND;													   /* IND			  */
}
OP ( ed, ab )
{
    OUTD;													   /* OUTD			  */
}
OP ( ed, ac )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, ad )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, ae )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, af )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, b0 )
{
    LDIR;													   /* LDIR			  */
}
OP ( ed, b1 )
{
    CPIR;													   /* CPIR			  */
}
OP ( ed, b2 )
{
    INIR;													   /* INIR			  */
}
OP ( ed, b3 )
{
    OTIR;													   /* OTIR			  */
}
OP ( ed, b4 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, b5 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, b6 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, b7 )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, b8 )
{
    LDDR;													   /* LDDR			  */
}
OP ( ed, b9 )
{
    CPDR;													   /* CPDR			  */
}
OP ( ed, ba )
{
    INDR;													   /* INDR			  */
}
OP ( ed, bb )
{
    OTDR;													   /* OTDR			  */
}
OP ( ed, bc )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, bd )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, be )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, bf )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, c0 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, c1 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, c2 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, c3 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, c4 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, c5 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, c6 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, c7 )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, c8 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, c9 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, ca )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, cb )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, cc )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, cd )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, ce )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, cf )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, d0 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, d1 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, d2 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, d3 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, d4 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, d5 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, d6 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, d7 )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, d8 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, d9 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, da )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, db )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, dc )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, dd )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, de )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, df )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, e0 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, e1 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, e2 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, e3 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, e4 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, e5 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, e6 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, e7 )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, e8 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, e9 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, ea )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, eb )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, ec )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, ed )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, ee )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, ef )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, f0 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, f1 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, f2 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, f3 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, f4 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, f5 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, f6 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, f7 )
{
    illegal_2();											   /* DB   ED		  */
}

OP ( ed, f8 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, f9 )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, fa )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, fb )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, fc )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, fd )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, fe )
{
    illegal_2();											   /* DB   ED		  */
}
OP ( ed, ff )
{
    illegal_2();											   /* DB   ED		  */
}

#if TIME_LOOP_HACKS

#define CHECK_BC_LOOP												\
if( _BC > 1 && _PCD < 0xfffc ) {									\
	UINT8 op1 = RM(_PCD);									\
	UINT8 op2 = RM(_PCD+1); 								\
	if( (op1==0x78 && op2==0xb1) || (op1==0x79 && op2==0xb0) )		\
	{																\
		UINT8 op3 = RM(_PCD+2); 							\
		UINT8 op4 = RM(_PCD+3); 							\
		if( op3==0x20 && op4==0xfb )								\
		{															\
			int cnt =												\
				cc[Z80_TABLE_op][0x78] +							\
				cc[Z80_TABLE_op][0xb1] +							\
				cc[Z80_TABLE_op][0x20] +							\
				cc[Z80_TABLE_ex][0x20]; 							\
			while( _BC > 0 && z80_ICount > cnt )					\
			{														\
				BURNODD( cnt, 4, cnt ); 							\
				_BC--;												\
			}														\
		}															\
		else														\
		if( op3 == 0xc2 )											\
		{															\
			UINT8 ad1 = RM(_PCD+3); 					\
			UINT8 ad2 = RM(_PCD+4); 					\
			if( (ad1 + 256 * ad2) == (_PCD - 1) )					\
			{														\
				int cnt =											\
					cc[Z80_TABLE_op][0x78] +						\
					cc[Z80_TABLE_op][0xb1] +						\
					cc[Z80_TABLE_op][0xc2] +						\
					cc[Z80_TABLE_ex][0xc2]; 						\
				while( _BC > 0 && z80_ICount > cnt )				\
				{													\
					BURNODD( cnt, 4, cnt ); 						\
					_BC--;											\
				}													\
			}														\
		}															\
	}																\
}

#define CHECK_DE_LOOP												\
if( _DE > 1 && _PCD < 0xfffc ) {									\
	UINT8 op1 = RM(_PCD);									\
	UINT8 op2 = RM(_PCD+1); 								\
	if( (op1==0x7a && op2==0xb3) || (op1==0x7b && op2==0xb2) )		\
	{																\
		UINT8 op3 = RM(_PCD+2); 							\
		UINT8 op4 = RM(_PCD+3); 							\
		if( op3==0x20 && op4==0xfb )								\
		{															\
			int cnt =												\
				cc[Z80_TABLE_op][0x7a] +							\
				cc[Z80_TABLE_op][0xb3] +							\
				cc[Z80_TABLE_op][0x20] +							\
				cc[Z80_TABLE_ex][0x20]; 							\
			while( _DE > 0 && z80_ICount > cnt )					\
			{														\
				BURNODD( cnt, 4, cnt ); 							\
				_DE--;												\
			}														\
		}															\
		else														\
		if( op3==0xc2 ) 											\
		{															\
			UINT8 ad1 = RM(_PCD+3); 					\
			UINT8 ad2 = RM(_PCD+4); 					\
			if( (ad1 + 256 * ad2) == (_PCD - 1) )					\
			{														\
				int cnt =											\
					cc[Z80_TABLE_op][0x7a] +						\
					cc[Z80_TABLE_op][0xb3] +						\
					cc[Z80_TABLE_op][0xc2] +						\
					cc[Z80_TABLE_ex][0xc2]; 						\
				while( _DE > 0 && z80_ICount > cnt )				\
				{													\
					BURNODD( cnt, 4, cnt ); 						\
					_DE--;											\
				}													\
			}														\
		}															\
	}																\
}

#define CHECK_HL_LOOP												\
if( _HL > 1 && _PCD < 0xfffc ) {									\
	UINT8 op1 = RM(_PCD);									\
	UINT8 op2 = RM(_PCD+1); 								\
	if( (op1==0x7c && op2==0xb5) || (op1==0x7d && op2==0xb4) )		\
	{																\
		UINT8 op3 = RM(_PCD+2); 							\
		UINT8 op4 = RM(_PCD+3); 							\
		if( op3==0x20 && op4==0xfb )								\
		{															\
			int cnt =												\
				cc[Z80_TABLE_op][0x7c] +							\
				cc[Z80_TABLE_op][0xb5] +							\
				cc[Z80_TABLE_op][0x20] +							\
				cc[Z80_TABLE_ex][0x20]; 							\
			while( _HL > 0 && z80_ICount > cnt )					\
			{														\
				BURNODD( cnt, 4, cnt ); 							\
				_HL--;												\
			}														\
		}															\
		else														\
		if( op3==0xc2 ) 											\
		{															\
			UINT8 ad1 = RM(_PCD+3); 					\
			UINT8 ad2 = RM(_PCD+4); 					\
			if( (ad1 + 256 * ad2) == (_PCD - 1) )					\
			{														\
				int cnt =											\
					cc[Z80_TABLE_op][0x7c] +						\
					cc[Z80_TABLE_op][0xb5] +						\
					cc[Z80_TABLE_op][0xc2] +						\
					cc[Z80_TABLE_ex][0xc2]; 						\
				while( _HL > 0 && z80_ICount > cnt )				\
				{													\
					BURNODD( cnt, 4, cnt ); 						\
					_HL--;											\
				}													\
			}														\
		}															\
	}																\
}

#else

#define CHECK_BC_LOOP
#define CHECK_DE_LOOP
#define CHECK_HL_LOOP

#endif

/**********************************************************
 * main opcodes
 **********************************************************/
OP ( op, 00 ) { 														} /* NOP			  */
OP ( op, 01 )
{
    _BC = ARG16();											   /* LD   BC,w		  */
}
OP ( op, 02 )
{
    WM ( _BC, _A );											   /* LD   (BC),A	  */
}
OP ( op, 03 )
{
    _BC++;													   /* INC  BC		  */
}
OP ( op, 04 )
{
    _B = INC ( _B );											   /* INC  B 		  */
}
OP ( op, 05 )
{
    _B = DEC ( _B );											   /* DEC  B 		  */
}
OP ( op, 06 )
{
    _B = ARG(); 											   /* LD   B,n		  */
}
OP ( op, 07 )
{
    RLCA;													   /* RLCA			  */
}

OP ( op, 08 )
{
    EX_AF;													   /* EX   AF,AF'      */
}
OP ( op, 09 )
{
    ADD16 ( HL, BC );											   /* ADD  HL,BC 	  */
}
OP ( op, 0a )
{
    _A = RM ( _BC );											   /* LD   A,(BC)	  */
}
OP ( op, 0b )
{
    _BC--;    /* DEC  BC		  */
    CHECK_BC_LOOP;
}
OP ( op, 0c )
{
    _C = INC ( _C );											   /* INC  C 		  */
}
OP ( op, 0d )
{
    _C = DEC ( _C );											   /* DEC  C 		  */
}
OP ( op, 0e )
{
    _C = ARG(); 											   /* LD   C,n		  */
}
OP ( op, 0f )
{
    RRCA;													   /* RRCA			  */
}

OP ( op, 10 )
{
    _B--;    /* DJNZ o 		  */
    JR_COND ( _B, 0x10 );
}
OP ( op, 11 )
{
    _DE = ARG16();											   /* LD   DE,w		  */
}
OP ( op, 12 )
{
    WM ( _DE, _A );											   /* LD   (DE),A	  */
}
OP ( op, 13 )
{
    _DE++;													   /* INC  DE		  */
}
OP ( op, 14 )
{
    _D = INC ( _D );											   /* INC  D 		  */
}
OP ( op, 15 )
{
    _D = DEC ( _D );											   /* DEC  D 		  */
}
OP ( op, 16 )
{
    _D = ARG(); 											   /* LD   D,n		  */
}
OP ( op, 17 )
{
    RLA;													   /* RLA			  */
}

OP ( op, 18 )
{
    JR();													   /* JR   o 		  */
}
OP ( op, 19 )
{
    ADD16 ( HL, DE );											   /* ADD  HL,DE 	  */
}
OP ( op, 1a )
{
    _A = RM ( _DE );											   /* LD   A,(DE)	  */
}
OP ( op, 1b )
{
    _DE--;    /* DEC  DE		  */
    CHECK_DE_LOOP;
}
OP ( op, 1c )
{
    _E = INC ( _E );											   /* INC  E 		  */
}
OP ( op, 1d )
{
    _E = DEC ( _E );											   /* DEC  E 		  */
}
OP ( op, 1e )
{
    _E = ARG(); 											   /* LD   E,n		  */
}
OP ( op, 1f )
{
    RRA;													   /* RRA			  */
}

OP ( op, 20 )
{
    JR_COND ( ! ( _F & ZF ), 0x20 );							   /* JR   NZ,o		  */
}
OP ( op, 21 )
{
    _HL = ARG16();											   /* LD   HL,w		  */
}
OP ( op, 22 )
{
    Z80.EA = ARG16();    /* LD   (w),HL	  */
    WM16 ( Z80.EA, &Z80.HL );
}
OP ( op, 23 )
{
    _HL++;													   /* INC  HL		  */
}
OP ( op, 24 )
{
    _H = INC ( _H );											   /* INC  H 		  */
}
OP ( op, 25 )
{
    _H = DEC ( _H );											   /* DEC  H 		  */
}
OP ( op, 26 )
{
    _H = ARG(); 											   /* LD   H,n		  */
}
OP ( op, 27 )
{
    DAA;													   /* DAA			  */
}

OP ( op, 28 )
{
    JR_COND ( _F & ZF, 0x28 );								   /* JR   Z,o		  */
}
OP ( op, 29 )
{
    ADD16 ( HL, HL );											   /* ADD  HL,HL 	  */
}
OP ( op, 2a )
{
    Z80.EA = ARG16();    /* LD   HL,(w)	  */
    RM16 ( Z80.EA, &Z80.HL );
}
OP ( op, 2b )
{
    _HL--;    /* DEC  HL		  */
    CHECK_HL_LOOP;
}
OP ( op, 2c )
{
    _L = INC ( _L );											   /* INC  L 		  */
}
OP ( op, 2d )
{
    _L = DEC ( _L );											   /* DEC  L 		  */
}
OP ( op, 2e )
{
    _L = ARG(); 											   /* LD   L,n		  */
}
OP ( op, 2f )
{
    _A ^= 0xff;    /* CPL			  */
    _F = ( _F & ( SF | ZF | PF | CF ) ) | HF | NF | ( _A & ( YF | XF ) );
}

OP ( op, 30 )
{
    JR_COND ( ! ( _F & CF ), 0x30 );							   /* JR   NC,o		  */
}
OP ( op, 31 )
{
    _SP = ARG16();											   /* LD   SP,w		  */
}
OP ( op, 32 )
{
    Z80.EA = ARG16();    /* LD   (w),A 	  */
    WM ( Z80.EA, _A );
}
OP ( op, 33 )
{
    _SP++;													   /* INC  SP		  */
}
OP ( op, 34 )
{
    WM ( _HL, INC ( RM ( _HL ) ) );								   /* INC  (HL)		  */
}
OP ( op, 35 )
{
    WM ( _HL, DEC ( RM ( _HL ) ) );								   /* DEC  (HL)		  */
}
OP ( op, 36 )
{
    WM ( _HL, ARG() );										   /* LD   (HL),n	  */
}
OP ( op, 37 )
{
    _F = ( _F & ( SF | ZF | PF ) ) | CF | ( _A & ( YF | XF ) );			   /* SCF			  */
}

OP ( op, 38 )
{
    JR_COND ( _F & CF, 0x38 );								   /* JR   C,o		  */
}
OP ( op, 39 )
{
    ADD16 ( HL, SP );											   /* ADD  HL,SP 	  */
}
OP ( op, 3a )
{
    Z80.EA = ARG16();    /* LD   A,(w) 	  */
    _A = RM ( Z80.EA );
}
OP ( op, 3b )
{
    _SP--;													   /* DEC  SP		  */
}
OP ( op, 3c )
{
    _A = INC ( _A );											   /* INC  A 		  */
}
OP ( op, 3d )
{
    _A = DEC ( _A );											   /* DEC  A 		  */
}
OP ( op, 3e )
{
    _A = ARG(); 											   /* LD   A,n		  */
}
OP ( op, 3f )
{
    _F = ( ( _F & ( SF | ZF | PF | CF ) ) | ( ( _F & CF ) << 4 ) | ( _A & ( YF | XF ) ) ) ^CF;    /* CCF			  */
}
/*OP(op,3f) { _F = ((_F & ~(HF|NF)) | ((_F & CF)<<4)) ^ CF; 		  }    CCF				   */

OP ( op, 40 ) { 														} /* LD   B,B		  */
OP ( op, 41 )
{
    _B = _C;												   /* LD   B,C		  */
}
OP ( op, 42 )
{
    _B = _D;												   /* LD   B,D		  */
}
OP ( op, 43 )
{
    _B = _E;												   /* LD   B,E		  */
}
OP ( op, 44 )
{
    _B = _H;												   /* LD   B,H		  */
}
OP ( op, 45 )
{
    _B = _L;												   /* LD   B,L		  */
}
OP ( op, 46 )
{
    _B = RM ( _HL );											   /* LD   B,(HL)	  */
}
OP ( op, 47 )
{
    _B = _A;												   /* LD   B,A		  */
}

OP ( op, 48 )
{
    _C = _B;												   /* LD   C,B		  */
}
OP ( op, 49 ) { 														} /* LD   C,C		  */
OP ( op, 4a )
{
    _C = _D;												   /* LD   C,D		  */
}
OP ( op, 4b )
{
    _C = _E;												   /* LD   C,E		  */
}
OP ( op, 4c )
{
    _C = _H;												   /* LD   C,H		  */
}
OP ( op, 4d )
{
    _C = _L;												   /* LD   C,L		  */
}
OP ( op, 4e )
{
    _C = RM ( _HL );											   /* LD   C,(HL)	  */
}
OP ( op, 4f )
{
    _C = _A;												   /* LD   C,A		  */
}

OP ( op, 50 )
{
    _D = _B;												   /* LD   D,B		  */
}
OP ( op, 51 )
{
    _D = _C;												   /* LD   D,C		  */
}
OP ( op, 52 ) { 														} /* LD   D,D		  */
OP ( op, 53 )
{
    _D = _E;												   /* LD   D,E		  */
}
OP ( op, 54 )
{
    _D = _H;												   /* LD   D,H		  */
}
OP ( op, 55 )
{
    _D = _L;												   /* LD   D,L		  */
}
OP ( op, 56 )
{
    _D = RM ( _HL );											   /* LD   D,(HL)	  */
}
OP ( op, 57 )
{
    _D = _A;												   /* LD   D,A		  */
}

OP ( op, 58 )
{
    _E = _B;												   /* LD   E,B		  */
}
OP ( op, 59 )
{
    _E = _C;												   /* LD   E,C		  */
}
OP ( op, 5a )
{
    _E = _D;												   /* LD   E,D		  */
}
OP ( op, 5b ) { 														} /* LD   E,E		  */
OP ( op, 5c )
{
    _E = _H;												   /* LD   E,H		  */
}
OP ( op, 5d )
{
    _E = _L;												   /* LD   E,L		  */
}
OP ( op, 5e )
{
    _E = RM ( _HL );											   /* LD   E,(HL)	  */
}
OP ( op, 5f )
{
    _E = _A;												   /* LD   E,A		  */
}

OP ( op, 60 )
{
    _H = _B;												   /* LD   H,B		  */
}
OP ( op, 61 )
{
    _H = _C;												   /* LD   H,C		  */
}
OP ( op, 62 )
{
    _H = _D;												   /* LD   H,D		  */
}
OP ( op, 63 )
{
    _H = _E;												   /* LD   H,E		  */
}
OP ( op, 64 ) { 														} /* LD   H,H		  */
OP ( op, 65 )
{
    _H = _L;												   /* LD   H,L		  */
}
OP ( op, 66 )
{
    _H = RM ( _HL );											   /* LD   H,(HL)	  */
}
OP ( op, 67 )
{
    _H = _A;												   /* LD   H,A		  */
}

OP ( op, 68 )
{
    _L = _B;												   /* LD   L,B		  */
}
OP ( op, 69 )
{
    _L = _C;												   /* LD   L,C		  */
}
OP ( op, 6a )
{
    _L = _D;												   /* LD   L,D		  */
}
OP ( op, 6b )
{
    _L = _E;												   /* LD   L,E		  */
}
OP ( op, 6c )
{
    _L = _H;												   /* LD   L,H		  */
}
OP ( op, 6d ) { 														} /* LD   L,L		  */
OP ( op, 6e )
{
    _L = RM ( _HL );											   /* LD   L,(HL)	  */
}
OP ( op, 6f )
{
    _L = _A;												   /* LD   L,A		  */
}

OP ( op, 70 )
{
    WM ( _HL, _B );											   /* LD   (HL),B	  */
}
OP ( op, 71 )
{
    WM ( _HL, _C );											   /* LD   (HL),C	  */
}
OP ( op, 72 )
{
    WM ( _HL, _D );											   /* LD   (HL),D	  */
}
OP ( op, 73 )
{
    WM ( _HL, _E );											   /* LD   (HL),E	  */
}
OP ( op, 74 )
{
    WM ( _HL, _H );											   /* LD   (HL),H	  */
}
OP ( op, 75 )
{
    WM ( _HL, _L );											   /* LD   (HL),L	  */
}
OP ( op, 76 )
{
    ENTER_HALT; 											   /* HALT			  */
}
OP ( op, 77 )
{
    WM ( _HL, _A );											   /* LD   (HL),A	  */
}

OP ( op, 78 )
{
    _A = _B;												   /* LD   A,B		  */
}
OP ( op, 79 )
{
    _A = _C;												   /* LD   A,C		  */
}
OP ( op, 7a )
{
    _A = _D;												   /* LD   A,D		  */
}
OP ( op, 7b )
{
    _A = _E;												   /* LD   A,E		  */
}
OP ( op, 7c )
{
    _A = _H;												   /* LD   A,H		  */
}
OP ( op, 7d )
{
    _A = _L;												   /* LD   A,L		  */
}
OP ( op, 7e )
{
    _A = RM ( _HL );											   /* LD   A,(HL)	  */
}
OP ( op, 7f ) { 														} /* LD   A,A		  */

OP ( op, 80 )
{
    ADD ( _B );												   /* ADD  A,B		  */
}
OP ( op, 81 )
{
    ADD ( _C );												   /* ADD  A,C		  */
}
OP ( op, 82 )
{
    ADD ( _D );												   /* ADD  A,D		  */
}
OP ( op, 83 )
{
    ADD ( _E );												   /* ADD  A,E		  */
}
OP ( op, 84 )
{
    ADD ( _H );												   /* ADD  A,H		  */
}
OP ( op, 85 )
{
    ADD ( _L );												   /* ADD  A,L		  */
}
OP ( op, 86 )
{
    ADD ( RM ( _HL ) );											   /* ADD  A,(HL)	  */
}
OP ( op, 87 )
{
    ADD ( _A );												   /* ADD  A,A		  */
}

OP ( op, 88 )
{
    ADC ( _B );												   /* ADC  A,B		  */
}
OP ( op, 89 )
{
    ADC ( _C );												   /* ADC  A,C		  */
}
OP ( op, 8a )
{
    ADC ( _D );												   /* ADC  A,D		  */
}
OP ( op, 8b )
{
    ADC ( _E );												   /* ADC  A,E		  */
}
OP ( op, 8c )
{
    ADC ( _H );												   /* ADC  A,H		  */
}
OP ( op, 8d )
{
    ADC ( _L );												   /* ADC  A,L		  */
}
OP ( op, 8e )
{
    ADC ( RM ( _HL ) );											   /* ADC  A,(HL)	  */
}
OP ( op, 8f )
{
    ADC ( _A );												   /* ADC  A,A		  */
}

OP ( op, 90 )
{
    SUB ( _B );												   /* SUB  B 		  */
}
OP ( op, 91 )
{
    SUB ( _C );												   /* SUB  C 		  */
}
OP ( op, 92 )
{
    SUB ( _D );												   /* SUB  D 		  */
}
OP ( op, 93 )
{
    SUB ( _E );												   /* SUB  E 		  */
}
OP ( op, 94 )
{
    SUB ( _H );												   /* SUB  H 		  */
}
OP ( op, 95 )
{
    SUB ( _L );												   /* SUB  L 		  */
}
OP ( op, 96 )
{
    SUB ( RM ( _HL ) );											   /* SUB  (HL)		  */
}
OP ( op, 97 )
{
    SUB ( _A );												   /* SUB  A 		  */
}

OP ( op, 98 )
{
    SBC ( _B );												   /* SBC  A,B		  */
}
OP ( op, 99 )
{
    SBC ( _C );												   /* SBC  A,C		  */
}
OP ( op, 9a )
{
    SBC ( _D );												   /* SBC  A,D		  */
}
OP ( op, 9b )
{
    SBC ( _E );												   /* SBC  A,E		  */
}
OP ( op, 9c )
{
    SBC ( _H );												   /* SBC  A,H		  */
}
OP ( op, 9d )
{
    SBC ( _L );												   /* SBC  A,L		  */
}
OP ( op, 9e )
{
    SBC ( RM ( _HL ) );											   /* SBC  A,(HL)	  */
}
OP ( op, 9f )
{
    SBC ( _A );												   /* SBC  A,A		  */
}

OP ( op, a0 )
{
    AND ( _B );												   /* AND  B 		  */
}
OP ( op, a1 )
{
    AND ( _C );												   /* AND  C 		  */
}
OP ( op, a2 )
{
    AND ( _D );												   /* AND  D 		  */
}
OP ( op, a3 )
{
    AND ( _E );												   /* AND  E 		  */
}
OP ( op, a4 )
{
    AND ( _H );												   /* AND  H 		  */
}
OP ( op, a5 )
{
    AND ( _L );												   /* AND  L 		  */
}
OP ( op, a6 )
{
    AND ( RM ( _HL ) );											   /* AND  (HL)		  */
}
OP ( op, a7 )
{
    AND ( _A );												   /* AND  A 		  */
}

OP ( op, a8 )
{
    XOR ( _B );												   /* XOR  B 		  */
}
OP ( op, a9 )
{
    XOR ( _C );												   /* XOR  C 		  */
}
OP ( op, aa )
{
    XOR ( _D );												   /* XOR  D 		  */
}
OP ( op, ab )
{
    XOR ( _E );												   /* XOR  E 		  */
}
OP ( op, ac )
{
    XOR ( _H );												   /* XOR  H 		  */
}
OP ( op, ad )
{
    XOR ( _L );												   /* XOR  L 		  */
}
OP ( op, ae )
{
    XOR ( RM ( _HL ) );											   /* XOR  (HL)		  */
}
OP ( op, af )
{
    XOR ( _A );												   /* XOR  A 		  */
}

OP ( op, b0 )
{
    OR ( _B ); 												   /* OR   B 		  */
}
OP ( op, b1 )
{
    OR ( _C ); 												   /* OR   C 		  */
}
OP ( op, b2 )
{
    OR ( _D ); 												   /* OR   D 		  */
}
OP ( op, b3 )
{
    OR ( _E ); 												   /* OR   E 		  */
}
OP ( op, b4 )
{
    OR ( _H ); 												   /* OR   H 		  */
}
OP ( op, b5 )
{
    OR ( _L ); 												   /* OR   L 		  */
}
OP ( op, b6 )
{
    OR ( RM ( _HL ) );											   /* OR   (HL)		  */
}
OP ( op, b7 )
{
    OR ( _A ); 												   /* OR   A 		  */
}

OP ( op, b8 )
{
    CP ( _B ); 												   /* CP   B 		  */
}
OP ( op, b9 )
{
    CP ( _C ); 												   /* CP   C 		  */
}
OP ( op, ba )
{
    CP ( _D ); 												   /* CP   D 		  */
}
OP ( op, bb )
{
    CP ( _E ); 												   /* CP   E 		  */
}
OP ( op, bc )
{
    CP ( _H ); 												   /* CP   H 		  */
}
OP ( op, bd )
{
    CP ( _L ); 												   /* CP   L 		  */
}
OP ( op, be )
{
    CP ( RM ( _HL ) );											   /* CP   (HL)		  */
}
OP ( op, bf )
{
    CP ( _A ); 												   /* CP   A 		  */
}

OP ( op, c0 )
{
    RET_COND ( ! ( _F & ZF ), 0xc0 );							   /* RET  NZ		  */
}
OP ( op, c1 )
{
    POP ( BC );												   /* POP  BC		  */
}
OP ( op, c2 )
{
    JP_COND ( ! ( _F & ZF ) );									   /* JP   NZ,a		  */
}
OP ( op, c3 )
{
    JP; 													   /* JP   a 		  */
}
OP ( op, c4 )
{
    CALL_COND ( ! ( _F & ZF ), 0xc4 );							   /* CALL NZ,a		  */
}
OP ( op, c5 )
{
    PUSH ( BC ); 											   /* PUSH BC		  */
}
OP ( op, c6 )
{
    ADD ( ARG() ); 											   /* ADD  A,n		  */
}
OP ( op, c7 )
{
    RST ( 0x00 );												   /* RST  0 		  */
}

OP ( op, c8 )
{
    RET_COND ( _F & ZF, 0xc8 );								   /* RET  Z 		  */
}
OP ( op, c9 )
{
    POP ( PC );    /* RET			  */
    change_pc16 ( _PCD );
}
OP ( op, ca )
{
    JP_COND ( _F & ZF ); 									   /* JP   Z,a		  */
}
OP ( op, cb )
{
    _R++;    /* **** CB xx 	  */
    EXEC ( cb, ROP() );
}
OP ( op, cc )
{
    CALL_COND ( _F & ZF, 0xcc ); 							   /* CALL Z,a		  */
}
OP ( op, cd )
{
    CALL(); 												   /* CALL a 		  */
}
OP ( op, ce )
{
    ADC ( ARG() ); 											   /* ADC  A,n		  */
}
OP ( op, cf )
{
    RST ( 0x08 );												   /* RST  1 		  */
}

OP ( op, d0 )
{
    RET_COND ( ! ( _F & CF ), 0xd0 );							   /* RET  NC		  */
}
OP ( op, d1 )
{
    POP ( DE );												   /* POP  DE		  */
}
OP ( op, d2 )
{
    JP_COND ( ! ( _F & CF ) );									   /* JP   NC,a		  */
}
OP ( op, d3 )
{
    unsigned n = ARG() | ( _A << 8 );    /* OUT  (n),A 	  */
    OUT ( n, _A );
}
OP ( op, d4 )
{
    CALL_COND ( ! ( _F & CF ), 0xd4 );							   /* CALL NC,a		  */
}
OP ( op, d5 )
{
    PUSH ( DE ); 											   /* PUSH DE		  */
}
OP ( op, d6 )
{
    SUB ( ARG() ); 											   /* SUB  n 		  */
}
OP ( op, d7 )
{
    RST ( 0x10 );												   /* RST  2 		  */
}

OP ( op, d8 )
{
    RET_COND ( _F & CF, 0xd8 );								   /* RET  C 		  */
}
OP ( op, d9 )
{
    EXX;													   /* EXX			  */
}
OP ( op, da )
{
    JP_COND ( _F & CF ); 									   /* JP   C,a		  */
}
OP ( op, db )
{
    unsigned n = ARG() | ( _A << 8 );    /* IN   A,(n) 	  */
    _A = IN ( n );
}
OP ( op, dc )
{
    CALL_COND ( _F & CF, 0xdc ); 							   /* CALL C,a		  */
}
OP ( op, dd )
{
    _R++;    /* **** DD xx 	  */
    EXEC ( dd, ROP() );
}
OP ( op, de )
{
    SBC ( ARG() ); 											   /* SBC  A,n		  */
}
OP ( op, df )
{
    RST ( 0x18 );												   /* RST  3 		  */
}

OP ( op, e0 )
{
    RET_COND ( ! ( _F & PF ), 0xe0 );							   /* RET  PO		  */
}
OP ( op, e1 )
{
    POP ( HL );												   /* POP  HL		  */
}
OP ( op, e2 )
{
    JP_COND ( ! ( _F & PF ) );									   /* JP   PO,a		  */
}
OP ( op, e3 )
{
    EXSP ( HL );												   /* EX   HL,(SP)	  */
}
OP ( op, e4 )
{
    CALL_COND ( ! ( _F & PF ), 0xe4 );							   /* CALL PO,a		  */
}
OP ( op, e5 )
{
    PUSH ( HL ); 											   /* PUSH HL		  */
}
OP ( op, e6 )
{
    AND ( ARG() ); 											   /* AND  n 		  */
}
OP ( op, e7 )
{
    RST ( 0x20 );												   /* RST  4 		  */
}

OP ( op, e8 )
{
    RET_COND ( _F & PF, 0xe8 );								   /* RET  PE		  */
}
OP ( op, e9 )
{
    _PC = _HL;    /* JP   (HL)		  */
    change_pc16 ( _PCD );
}
OP ( op, ea )
{
    JP_COND ( _F & PF ); 									   /* JP   PE,a		  */
}
OP ( op, eb )
{
    EX_DE_HL;												   /* EX   DE,HL 	  */
}
OP ( op, ec )
{
    CALL_COND ( _F & PF, 0xec ); 							   /* CALL PE,a		  */
}
OP ( op, ed )
{
    _R++;    /* **** ED xx 	  */
    EXEC ( ed, ROP() );
}
OP ( op, ee )
{
    XOR ( ARG() ); 											   /* XOR  n 		  */
}
OP ( op, ef )
{
    RST ( 0x28 );												   /* RST  5 		  */
}

OP ( op, f0 )
{
    RET_COND ( ! ( _F & SF ), 0xf0 );							   /* RET  P 		  */
}
OP ( op, f1 )
{
    POP ( AF );												   /* POP  AF		  */
}
OP ( op, f2 )
{
    JP_COND ( ! ( _F & SF ) );									   /* JP   P,a		  */
}
OP ( op, f3 )
{
    _IFF1 = _IFF2 = 0;										   /* DI 			  */
}
OP ( op, f4 )
{
    CALL_COND ( ! ( _F & SF ), 0xf4 );							   /* CALL P,a		  */
}
OP ( op, f5 )
{
    PUSH ( AF ); 											   /* PUSH AF		  */
}
OP ( op, f6 )
{
    OR ( ARG() );												   /* OR   n 		  */
}
OP ( op, f7 )
{
    RST ( 0x30 );												   /* RST  6 		  */
}

OP ( op, f8 )
{
    RET_COND ( _F & SF, 0xf8 );								   /* RET  M 		  */
}
OP ( op, f9 )
{
    _SP = _HL;												   /* LD   SP,HL 	  */
}
OP ( op, fa )
{
    JP_COND ( _F & SF );										   /* JP   M,a		  */
}
OP ( op, fb )
{
    EI; 													   /* EI 			  */
}
OP ( op, fc )
{
    CALL_COND ( _F & SF, 0xfc ); 							   /* CALL M,a		  */
}
OP ( op, fd )
{
    _R++;    /* **** FD xx 	  */
    EXEC ( fd, ROP() );
}
OP ( op, fe )
{
    CP ( ARG() );												   /* CP   n 		  */
}
OP ( op, ff )
{
    RST ( 0x38 );												   /* RST  7 		  */
}

static void take_interrupt ( void )
{
    if ( _IFF1 )
    {
        int irq_vector = 0;

        /* there isn't a valid previous program counter */
        _PPC = -1;

        /* Check if processor was halted */
        LEAVE_HALT;

        if ( Z80.irq_max )			/* daisy chain mode */
        {
            if ( Z80.request_irq >= 0 )
            {
                /* Clear both interrupt flip flops */
                _IFF1 = _IFF2 = 0;
                irq_vector = Z80.irq[Z80.request_irq].interrupt_entry ( Z80.irq[Z80.request_irq].irq_param );
                LOG ( ( "Z80 #%d daisy chain irq_vector $%02x\n", cpu_getactivecpu(), irq_vector ) );
                Z80.request_irq = -1;
            }
            else { return; }
        }
        else
        {
            /* Clear both interrupt flip flops */
            _IFF1 = _IFF2 = 0;
            /* call back the cpu interface to retrieve the vector */
            irq_vector = ( *Z80.irq_callback ) ( 0 );
            LOG ( ( "Z80 #%d single int. irq_vector $%02x\n", cpu_getactivecpu(), irq_vector ) );
        }

        /* Interrupt mode 2. Call [Z80.I:databyte] */
        if ( _IM == 2 )
        {
            irq_vector = ( irq_vector & 0xff ) | ( _I << 8 );
            PUSH ( PC );
            RM16 ( irq_vector, &Z80.PC );
            LOG ( ( "Z80 #%d IM2 [$%04x] = $%04x\n", cpu_getactivecpu(), irq_vector, _PCD ) );
            /* CALL opcode timing */
            Z80.extra_cycles += cc[Z80_TABLE_op][0xcd];
        }
        else
            /* Interrupt mode 1. RST 38h */
            if ( _IM == 1 )
            {
                LOG ( ( "Z80 #%d IM1 $0038\n", cpu_getactivecpu() ) );
                PUSH ( PC );
                _PCD = 0x0038;
                /* RST $38 + 'interrupt latency' cycles */
                Z80.extra_cycles += cc[Z80_TABLE_op][0xff] + cc[Z80_TABLE_ex][0xff];
            }
            else
            {
                /* Interrupt mode 0. We check for CALL and JP instructions, */
                /* if neither of these were found we assume a 1 byte opcode */
                /* was placed on the databus								*/
                LOG ( ( "Z80 #%d IM0 $%04x\n", cpu_getactivecpu(), irq_vector ) );
                switch ( irq_vector & 0xff0000 )
                {
                case 0xcd0000:	/* call */
                    PUSH ( PC );
                    _PCD = irq_vector & 0xffff;
                    /* CALL $xxxx + 'interrupt latency' cycles */
                    Z80.extra_cycles += cc[Z80_TABLE_op][0xcd] + cc[Z80_TABLE_ex][0xff];
                    break;
                case 0xc30000:	/* jump */
                    _PCD = irq_vector & 0xffff;
                    /* JP $xxxx + 2 cycles */
                    Z80.extra_cycles += cc[Z80_TABLE_op][0xc3] + cc[Z80_TABLE_ex][0xff];
                    break;
                default:		/* rst (or other opcodes?) */
                    PUSH ( PC );
                    _PCD = irq_vector & 0x0038;
                    /* RST $xx + 2 cycles */
                    Z80.extra_cycles += cc[Z80_TABLE_op][_PCD] + cc[Z80_TABLE_ex][_PCD];
                    break;
                }
            }
        change_pc16 ( _PCD );
    }
}
/****************************************************************************
 * Processor initialization
 ****************************************************************************/
void z80_init ( int ( *callback ) ( int ) )
{
    /* --- MP ---
       int cpu = cpu_getactivecpu();
    */
    int i = 0, p = 0;
    int oldval = 0, newval = 0, val = 0;
    UINT8 *padd, *padc, *psub, *psbc;

    padd = &SZHVC_add[	0 * 256];
    padc = &SZHVC_add[256 * 256];
    psub = &SZHVC_sub[	0 * 256];
    psbc = &SZHVC_sub[256 * 256];

    for ( oldval = 0; oldval < 256; oldval++ )
    {
        for ( newval = 0; newval < 256; newval++ )
        {
            /* add or adc w/o carry set */
            val = newval - oldval;
            *padd = ( newval ) ? ( ( newval & 0x80 ) ? SF : 0 ) : ZF;
            *padd |= ( newval & ( YF | XF ) );	/* undocumented flag bits 5+3 */
            if ( ( newval & 0x0f ) < ( oldval & 0x0f ) ) { *padd |= HF; }
            if ( newval < oldval ) { *padd |= CF; }
            if ( ( val ^ oldval ^ 0x80 ) & ( val ^ newval ) & 0x80 ) { *padd |= VF; }
            padd++;

            /* adc with carry set */
            val = newval - oldval - 1;
            *padc = ( newval ) ? ( ( newval & 0x80 ) ? SF : 0 ) : ZF;
            *padc |= ( newval & ( YF | XF ) );	/* undocumented flag bits 5+3 */
            if ( ( newval & 0x0f ) <= ( oldval & 0x0f ) ) { *padc |= HF; }
            if ( newval <= oldval ) { *padc |= CF; }
            if ( ( val ^ oldval ^ 0x80 ) & ( val ^ newval ) & 0x80 ) { *padc |= VF; }
            padc++;

            /* cp, sub or sbc w/o carry set */
            val = oldval - newval;
            *psub = NF | ( ( newval ) ? ( ( newval & 0x80 ) ? SF : 0 ) : ZF );
            *psub |= ( newval & ( YF | XF ) );	/* undocumented flag bits 5+3 */
            if ( ( newval & 0x0f ) > ( oldval & 0x0f ) ) { *psub |= HF; }
            if ( newval > oldval ) { *psub |= CF; }
            if ( ( val ^ oldval ) & ( oldval ^ newval ) & 0x80 ) { *psub |= VF; }
            psub++;

            /* sbc with carry set */
            val = oldval - newval - 1;
            *psbc = NF | ( ( newval ) ? ( ( newval & 0x80 ) ? SF : 0 ) : ZF );
            *psbc |= ( newval & ( YF | XF ) );	/* undocumented flag bits 5+3 */
            if ( ( newval & 0x0f ) >= ( oldval & 0x0f ) ) { *psbc |= HF; }
            if ( newval >= oldval ) { *psbc |= CF; }
            if ( ( val ^ oldval ) & ( oldval ^ newval ) & 0x80 ) { *psbc |= VF; }
            psbc++;
        }
    }

    for ( i = 0; i < 256; i++ )
    {
        p = 0;
        if ( i & 0x01 ) { ++p; }
        if ( i & 0x02 ) { ++p; }
        if ( i & 0x04 ) { ++p; }
        if ( i & 0x08 ) { ++p; }
        if ( i & 0x10 ) { ++p; }
        if ( i & 0x20 ) { ++p; }
        if ( i & 0x40 ) { ++p; }
        if ( i & 0x80 ) { ++p; }
        SZ[i] = i ? i & SF : ZF;
        SZ[i] |= ( i & ( YF | XF ) );		/* undocumented flag bits 5+3 */
        SZ_BIT[i] = i ? i & SF : ZF | PF;
        SZ_BIT[i] |= ( i & ( YF | XF ) );	/* undocumented flag bits 5+3 */
        SZP[i] = SZ[i] | ( ( p & 1 ) ? 0 : PF );
        SZHV_inc[i] = SZ[i];
        if ( i == 0x80 ) { SZHV_inc[i] |= VF; }
        if ( ( i & 0x0f ) == 0x00 ) { SZHV_inc[i] |= HF; }
        SZHV_dec[i] = SZ[i] | NF;
        if ( i == 0x7f ) { SZHV_dec[i] |= VF; }
        if ( ( i & 0x0f ) == 0x0f ) { SZHV_dec[i] |= HF; }
    }

    z80_reset ( NULL );

    Z80.irq_callback = callback;

    /* daisy chain needs to be saved by z80ctc.c somehow */
}
/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
void z80_reset ( void *param )
{
    Z80_DaisyChain *daisy_chain = ( Z80_DaisyChain * ) param;
    memset ( &Z80, 0, sizeof ( Z80 ) );
    _IX = _IY = 0xffff; /* IX and IY are FFFF after a reset! */
    _F = ZF;			/* Zero flag is set */
    Z80.request_irq = -1;
    Z80.service_irq = -1;
    Z80.nmi_state = CLEAR_LINE;
    Z80.irq_state = CLEAR_LINE;

    if ( daisy_chain )
    {
        while ( daisy_chain->irq_param != -1 && Z80.irq_max < Z80_MAXDAISY )
        {
            /* set callbackhandler after reti */
            Z80.irq[Z80.irq_max] = *daisy_chain;
            /* device reset */
            if ( Z80.irq[Z80.irq_max].reset )
            { Z80.irq[Z80.irq_max].reset ( Z80.irq[Z80.irq_max].irq_param ); }
            Z80.irq_max++;
            daisy_chain++;
        }
    }

    change_pc16 ( _PCD );
}
/****************************************************************************
 * Execute 'cycles' T-states. Return number of T-states really executed
 ****************************************************************************/
int z80_run ( int cycles, int debug )
{
    char buf[512];
    z80_ICount = ( cycles - Z80.extra_cycles );
    Z80.extra_cycles = 0;

    do
    {
        _PPC = _PCD;
        //CALL_MAME_DEBUG;
        if ( debug == 1 )
        {
            z80_dasm ( buf, _PCD );
            printf ( "PC[%X] : %s\n", _PCD, buf );
        }
        _R++;
        EXEC ( op, ROP() );
    }
    while ( z80_ICount > 0 );

    z80_ICount -= Z80.extra_cycles;
    Z80.extra_cycles = 0;

    return cycles - z80_ICount;
}
/****************************************************************************
 * Burn 'cycles' T-states. Adjust R register for the lost time
 ****************************************************************************/
void z80_burn ( int cycles )
{
    if ( cycles > 0 )
    {
        /* NOP takes 4 cycles per instruction */
        int n = ( cycles + 3 ) / 4;
        _R += n;
        z80_ICount -= 4 * n;
    }
}
/****************************************************************************
 * Get all registers in given buffer
 ****************************************************************************/
unsigned z80_get_context ( void *dst )
{
    if ( dst )
    { * ( Z80_Regs* ) dst = Z80; }
    return sizeof ( Z80_Regs );
}
/****************************************************************************
 * Set all registers to given values
 ****************************************************************************/
void z80_set_context ( void *src )
{
    if ( src )
    { Z80 = * ( Z80_Regs* ) src; }
    change_pc16 ( _PCD );
}
/****************************************************************************
 * Get a pointer to a cycle count table
 ****************************************************************************/
const void *z80_get_cycle_table ( int which )
{
    if ( which >= 0 && which <= Z80_TABLE_xycb )
    { return cc[which]; }
    return NULL;
}

/****************************************************************************
 * Set a new cycle count table
 ****************************************************************************/
void z80_set_cycle_table ( int which, void *new_table )
{
    if ( which >= 0 && which <= Z80_TABLE_ex )
    { cc[which] = new_table; }
}
/****************************************************************************
 * Return a specific register
 ****************************************************************************/
unsigned z80_get_reg ( int regnum )
{
    switch ( regnum )
    {
    case REG_PC:
        return _PCD;
    case Z80_PC:
        return Z80.PC.w.l;
    case REG_SP:
        return _SPD;
    case Z80_SP:
        return Z80.SP.w.l;
    case Z80_AF:
        return Z80.AF.w.l;
    case Z80_BC:
        return Z80.BC.w.l;
    case Z80_DE:
        return Z80.DE.w.l;
    case Z80_HL:
        return Z80.HL.w.l;
    case Z80_IX:
        return Z80.IX.w.l;
    case Z80_IY:
        return Z80.IY.w.l;
    case Z80_R:
        return ( Z80.R & 0x7f ) | ( Z80.R2 & 0x80 );
    case Z80_I:
        return Z80.I;
    case Z80_AF2:
        return Z80.AF2.w.l;
    case Z80_BC2:
        return Z80.BC2.w.l;
    case Z80_DE2:
        return Z80.DE2.w.l;
    case Z80_HL2:
        return Z80.HL2.w.l;
    case Z80_IM:
        return Z80.IM;
    case Z80_IFF1:
        return Z80.IFF1;
    case Z80_IFF2:
        return Z80.IFF2;
    case Z80_HALT:
        return Z80.HALT;
    case Z80_NMI_STATE:
        return Z80.nmi_state;
    case Z80_IRQ_STATE:
        return Z80.irq_state;
    case Z80_DC0:
        return Z80.int_state[0];
#if Z80_MAXDAISY > 1
    case Z80_DC1:
        return Z80.int_state[1];
    case Z80_DC2:
        return Z80.int_state[2];
    case Z80_DC3:
        return Z80.int_state[3];
#endif
    case REG_PREVIOUSPC:
        return Z80.PREPC.w.l;
    default:
        if ( regnum <= REG_SP_CONTENTS )
        {
            unsigned offset = _SPD + 2 * ( REG_SP_CONTENTS - regnum );
            if ( offset < 0xffff )
            { return RM ( offset ) | ( RM ( offset + 1 ) << 8 ); }
        }
    }
    return 0;
}
/****************************************************************************
 * Set a specific register
 ****************************************************************************/
void z80_set_reg ( int regnum, unsigned val )
{
    switch ( regnum )
    {
    case REG_PC:
        _PC = val;
        change_pc16 ( _PCD );
        break;
    case Z80_PC:
        Z80.PC.w.l = val;
        break;
    case REG_SP:
        _SP = val;
        break;
    case Z80_SP:
        Z80.SP.w.l = val;
        break;
    case Z80_AF:
        Z80.AF.w.l = val;
        break;
    case Z80_BC:
        Z80.BC.w.l = val;
        break;
    case Z80_DE:
        Z80.DE.w.l = val;
        break;
    case Z80_HL:
        Z80.HL.w.l = val;
        break;
    case Z80_IX:
        Z80.IX.w.l = val;
        break;
    case Z80_IY:
        Z80.IY.w.l = val;
        break;
    case Z80_R:
        Z80.R = val;
        Z80.R2 = val & 0x80;
        break;
    case Z80_I:
        Z80.I = val;
        break;
    case Z80_AF2:
        Z80.AF2.w.l = val;
        break;
    case Z80_BC2:
        Z80.BC2.w.l = val;
        break;
    case Z80_DE2:
        Z80.DE2.w.l = val;
        break;
    case Z80_HL2:
        Z80.HL2.w.l = val;
        break;
    case Z80_IM:
        Z80.IM = val;
        break;
    case Z80_IFF1:
        Z80.IFF1 = val;
        break;
    case Z80_IFF2:
        Z80.IFF2 = val;
        break;
    case Z80_HALT:
        Z80.HALT = val;
        break;
    case Z80_NMI_STATE:
        z80_set_nmi_line ( val );
        break;
    case Z80_IRQ_STATE:
        z80_set_irq_line ( 0, val );
        break;
    case Z80_DC0:
        Z80.int_state[0] = val;
        break;
#if Z80_MAXDAISY > 1
    case Z80_DC1:
        Z80.int_state[1] = val;
        break;
    case Z80_DC2:
        Z80.int_state[2] = val;
        break;
    case Z80_DC3:
        Z80.int_state[3] = val;
        break;
#endif
    default:
        if ( regnum <= REG_SP_CONTENTS )
        {
            unsigned offset = _SPD + 2 * ( REG_SP_CONTENTS - regnum );
            if ( offset < 0xffff )
            {
                WM ( offset, val & 0xff );
                WM ( offset + 1, ( val >> 8 ) & 0xff );
            }
        }
    }
}
/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
void z80_set_irq_line ( unsigned int irqline, unsigned int state )
{
    LOG ( ( "Z80 #%d set_irq_line %d\n", cpu_getactivecpu(), state ) );
    Z80.irq_state = state;

    if ( state != CLEAR_LINE )
    {
        if ( Z80.irq_max )
        {
            int daisychain, device, int_state;
            daisychain = ( *Z80.irq_callback ) ( irqline );
            device = daisychain >> 8;
            int_state = daisychain & 0xff;
            LOG ( ( "Z80 #%d daisy chain $%04x -> device %d, state $%02x", cpu_getactivecpu(), daisychain, device, int_state ) );

            if ( Z80.int_state[device] != int_state )
            {
                LOG ( ( " change\n" ) );
                /* set new interrupt status */
                Z80.int_state[device] = int_state;
                /* check interrupt status */
                Z80.request_irq = Z80.service_irq = -1;

                /* search higher IRQ or IEO */
                for ( device = 0 ; device < Z80.irq_max ; device ++ )
                {
                    /* IEO = disable ? */
                    if ( Z80.int_state[device] & Z80_INT_IEO )
                    {
                        Z80.request_irq = -1;		/* if IEO is disable , masking lower IRQ */
                        Z80.service_irq = device;	/* set highest interrupt service device */
                    }
                    /* IRQ = request ? */
                    if ( Z80.int_state[device] & Z80_INT_REQ )
                    { Z80.request_irq = device; }
                }
                LOG ( ( "Z80 #%d daisy chain service_irq $%02x, request_irq $%02x\n", cpu_getactivecpu(), Z80.service_irq, Z80.request_irq ) );
                if ( Z80.request_irq < 0 ) { return; }
            }
            else
            {
                LOG ( ( " no change\n" ) );
                return;
            }
        }
    }

    take_interrupt();
}
/****************************************************************************
 * Set NMI line state
 ****************************************************************************/
void z80_set_nmi_line ( unsigned int state )
{
    if ( Z80.nmi_state != state )
    {
        LOG ( ( "Z80 #%d set_irq_line (NMI) %d\n", cpu_getactivecpu(), state ) );
        Z80.nmi_state = state;

        if ( state != CLEAR_LINE )
        {
            LOG ( ( "Z80 #%d take NMI\n", cpu_getactivecpu() ) );
            _PPC = -1;			/* there isn't a valid previous program counter */
            LEAVE_HALT; 		/* Check if processor was halted */

            _IFF1 = 0;
            PUSH ( PC );
            _PCD = 0x0066;
            Z80.extra_cycles += 11;
        }
    }
}
/****************************************************************************
 * Set IRQ vector callback
 ****************************************************************************/
void z80_set_irq_callback ( int ( *callback ) ( int ) )
{
    LOG ( ( "Z80 #%d set_irq_callback $%08x\n", cpu_getactivecpu(), ( int ) callback ) );
    Z80.irq_callback = callback;
}
/****************************************************************************
 * Return a formatted string for a register
 ****************************************************************************/
const char *z80_info ( void *context, int regnum )
{
    static char buffer[32][47 + 1];
    static int which = 0;
    Z80_Regs *r = context;

    which = ( which + 1 ) % 32;
    buffer[which][0] = '\0';
    if ( !context )
    { r = &Z80; }

    switch ( regnum )
    {
    case CPU_INFO_REG+Z80_PC:
        sprintf ( buffer[which], "PC:%04X", r->PC.w.l );
        break;
    case CPU_INFO_REG+Z80_SP:
        sprintf ( buffer[which], "SP:%04X", r->SP.w.l );
        break;
    case CPU_INFO_REG+Z80_AF:
        sprintf ( buffer[which], "AF:%04X", r->AF.w.l );
        break;
    case CPU_INFO_REG+Z80_BC:
        sprintf ( buffer[which], "BC:%04X", r->BC.w.l );
        break;
    case CPU_INFO_REG+Z80_DE:
        sprintf ( buffer[which], "DE:%04X", r->DE.w.l );
        break;
    case CPU_INFO_REG+Z80_HL:
        sprintf ( buffer[which], "HL:%04X", r->HL.w.l );
        break;
    case CPU_INFO_REG+Z80_IX:
        sprintf ( buffer[which], "IX:%04X", r->IX.w.l );
        break;
    case CPU_INFO_REG+Z80_IY:
        sprintf ( buffer[which], "IY:%04X", r->IY.w.l );
        break;
    case CPU_INFO_REG+Z80_R:
        sprintf ( buffer[which], "R:%02X", ( r->R & 0x7f ) | ( r->R2 & 0x80 ) );
        break;
    case CPU_INFO_REG+Z80_I:
        sprintf ( buffer[which], "I:%02X", r->I );
        break;
    case CPU_INFO_REG+Z80_AF2:
        sprintf ( buffer[which], "AF'%04X", r->AF2.w.l );
        break;
    case CPU_INFO_REG+Z80_BC2:
        sprintf ( buffer[which], "BC'%04X", r->BC2.w.l );
        break;
    case CPU_INFO_REG+Z80_DE2:
        sprintf ( buffer[which], "DE'%04X", r->DE2.w.l );
        break;
    case CPU_INFO_REG+Z80_HL2:
        sprintf ( buffer[which], "HL'%04X", r->HL2.w.l );
        break;
    case CPU_INFO_REG+Z80_IM:
        sprintf ( buffer[which], "IM:%X", r->IM );
        break;
    case CPU_INFO_REG+Z80_IFF1:
        sprintf ( buffer[which], "IFF1:%X", r->IFF1 );
        break;
    case CPU_INFO_REG+Z80_IFF2:
        sprintf ( buffer[which], "IFF2:%X", r->IFF2 );
        break;
    case CPU_INFO_REG+Z80_HALT:
        sprintf ( buffer[which], "HALT:%X", r->HALT );
        break;
    case CPU_INFO_REG+Z80_NMI_STATE:
        sprintf ( buffer[which], "NMI:%X", r->nmi_state );
        break;
    case CPU_INFO_REG+Z80_IRQ_STATE:
        sprintf ( buffer[which], "IRQ:%X", r->irq_state );
        break;
    case CPU_INFO_REG+Z80_DC0:
        if ( Z80.irq_max >= 1 ) { sprintf ( buffer[which], "DC0:%X", r->int_state[0] ); }
        break;
#if Z80_MAXDAISY > 1
    case CPU_INFO_REG+Z80_DC1:
        if ( Z80.irq_max >= 2 ) { sprintf ( buffer[which], "DC1:%X", r->int_state[1] ); }
        break;
    case CPU_INFO_REG+Z80_DC2:
        if ( Z80.irq_max >= 3 ) { sprintf ( buffer[which], "DC2:%X", r->int_state[2] ); }
        break;
    case CPU_INFO_REG+Z80_DC3:
        if ( Z80.irq_max >= 4 ) { sprintf ( buffer[which], "DC3:%X", r->int_state[3] ); }
        break;
#endif
    case CPU_INFO_FLAGS:
        sprintf ( buffer[which], "%c%c%c%c%c%c%c%c",
                  r->AF.b.l & 0x80 ? 'S' : '.',
                  r->AF.b.l & 0x40 ? 'Z' : '.',
                  r->AF.b.l & 0x20 ? '5' : '.',
                  r->AF.b.l & 0x10 ? 'H' : '.',
                  r->AF.b.l & 0x08 ? '3' : '.',
                  r->AF.b.l & 0x04 ? 'P' : '.',
                  r->AF.b.l & 0x02 ? 'N' : '.',
                  r->AF.b.l & 0x01 ? 'C' : '.' );
        break;
    case CPU_INFO_NAME:
        return "Z80";
    case CPU_INFO_FAMILY:
        return "Zilog Z80";
    case CPU_INFO_VERSION:
        return "3.3";
    case CPU_INFO_FILE:
        return __FILE__;
    case CPU_INFO_CREDITS:
        return "Copyright (C) 1998,1999 Juergen Buchmueller, all rights reserved.";
    case CPU_INFO_REG_LAYOUT:
        return ( const char * ) z80_reg_layout;
    case CPU_INFO_WIN_LAYOUT:
        return ( const char * ) z80_win_layout;
    }
    return buffer[which];
}
/*
unsigned z80_dasm ( char *buffer, unsigned pc )
{
#ifdef MAME_DEBUG
    return DasmZ80 ( buffer, pc );
#else
    sprintf ( buffer, "$%02X", RM ( pc ) );
    return 1;
#endif
}
*/

#ifdef _Z80_C_
#undef _Z80_C_
#endif // _Z80_C_
