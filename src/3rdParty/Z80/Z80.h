#ifndef Z80_H
#define Z80_H

#include <stdint.h>
#define UINT8     uint8_t
#define UINT16    uint16_t
#define UINT32    uint32_t
#define INT8      int8_t
#define INT16     int16_t
#define INT32     int32_t
#ifndef INLINE
#define INLINE static __inline__
#endif

#define CALL_MAME_DEBUG

#define Z80_MAXDAISY    1               /* maximum of daisy chan device */

#define Z80_INT_REQ     0x01    /* interrupt request mask               */
#define Z80_INT_IEO     0x02    /* interrupt disable mask(IEO)  */

#define Z80_VECTOR(device,state) (((device)<<8)|(state))

#define change_pc16(pc) Z80.PC.w.l=pc;

#define VERBOSE 0

#if VERBOSE
#define LOG(x)	logerror x
#else
#define LOG(x)
#endif

/* on JP and JR opcodes check for tight loops */
#define BUSY_LOOP_HACKS 	1

/* check for delay loops counting down BC */
#define TIME_LOOP_HACKS 	1

#define CF	0x01
#define NF	0x02
#define PF	0x04
#define VF	PF
#define XF	0x08
#define HF	0x10
#define YF	0x20
#define ZF	0x40
#define SF	0x80

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

#define _PPC	Z80.PREPC.d 	/* previous program counter */

#define _PCD	Z80.PC.d
#define _PC 	Z80.PC.w.l

#define _SPD	Z80.SP.d
#define _SP 	Z80.SP.w.l

#define _AFD	Z80.AF.d
#define _AF 	Z80.AF.w.l
#define _A		Z80.AF.b.h
#define _F		Z80.AF.b.l

#define _BCD	Z80.BC.d
#define _BC 	Z80.BC.w.l
#define _B		Z80.BC.b.h
#define _C		Z80.BC.b.l

#define _DED	Z80.DE.d
#define _DE 	Z80.DE.w.l
#define _D		Z80.DE.b.h
#define _E		Z80.DE.b.l

#define _HLD	Z80.HL.d
#define _HL 	Z80.HL.w.l
#define _H		Z80.HL.b.h
#define _L		Z80.HL.b.l

#define _IXD	Z80.IX.d
#define _IX 	Z80.IX.w.l
#define _HX 	Z80.IX.b.h
#define _LX 	Z80.IX.b.l

#define _IYD	Z80.IY.d
#define _IY 	Z80.IY.w.l
#define _HY 	Z80.IY.b.h
#define _LY 	Z80.IY.b.l

#define _I		Z80.I
#define _R		Z80.R
#define _R2 	Z80.R2
#define _IM 	Z80.IM
#define _IFF1	Z80.IFF1
#define _IFF2	Z80.IFF2
#define _HALT	Z80.HALT

#define Z80_TABLE_dd	Z80_TABLE_xy
#define Z80_TABLE_fd	Z80_TABLE_xy

#define PROTOTYPES(tablename,prefix) \
	INLINE void prefix##_00(void); INLINE void prefix##_01(void); INLINE void prefix##_02(void); INLINE void prefix##_03(void); \
	INLINE void prefix##_04(void); INLINE void prefix##_05(void); INLINE void prefix##_06(void); INLINE void prefix##_07(void); \
	INLINE void prefix##_08(void); INLINE void prefix##_09(void); INLINE void prefix##_0a(void); INLINE void prefix##_0b(void); \
	INLINE void prefix##_0c(void); INLINE void prefix##_0d(void); INLINE void prefix##_0e(void); INLINE void prefix##_0f(void); \
	INLINE void prefix##_10(void); INLINE void prefix##_11(void); INLINE void prefix##_12(void); INLINE void prefix##_13(void); \
	INLINE void prefix##_14(void); INLINE void prefix##_15(void); INLINE void prefix##_16(void); INLINE void prefix##_17(void); \
	INLINE void prefix##_18(void); INLINE void prefix##_19(void); INLINE void prefix##_1a(void); INLINE void prefix##_1b(void); \
	INLINE void prefix##_1c(void); INLINE void prefix##_1d(void); INLINE void prefix##_1e(void); INLINE void prefix##_1f(void); \
	INLINE void prefix##_20(void); INLINE void prefix##_21(void); INLINE void prefix##_22(void); INLINE void prefix##_23(void); \
	INLINE void prefix##_24(void); INLINE void prefix##_25(void); INLINE void prefix##_26(void); INLINE void prefix##_27(void); \
	INLINE void prefix##_28(void); INLINE void prefix##_29(void); INLINE void prefix##_2a(void); INLINE void prefix##_2b(void); \
	INLINE void prefix##_2c(void); INLINE void prefix##_2d(void); INLINE void prefix##_2e(void); INLINE void prefix##_2f(void); \
	INLINE void prefix##_30(void); INLINE void prefix##_31(void); INLINE void prefix##_32(void); INLINE void prefix##_33(void); \
	INLINE void prefix##_34(void); INLINE void prefix##_35(void); INLINE void prefix##_36(void); INLINE void prefix##_37(void); \
	INLINE void prefix##_38(void); INLINE void prefix##_39(void); INLINE void prefix##_3a(void); INLINE void prefix##_3b(void); \
	INLINE void prefix##_3c(void); INLINE void prefix##_3d(void); INLINE void prefix##_3e(void); INLINE void prefix##_3f(void); \
	INLINE void prefix##_40(void); INLINE void prefix##_41(void); INLINE void prefix##_42(void); INLINE void prefix##_43(void); \
	INLINE void prefix##_44(void); INLINE void prefix##_45(void); INLINE void prefix##_46(void); INLINE void prefix##_47(void); \
	INLINE void prefix##_48(void); INLINE void prefix##_49(void); INLINE void prefix##_4a(void); INLINE void prefix##_4b(void); \
	INLINE void prefix##_4c(void); INLINE void prefix##_4d(void); INLINE void prefix##_4e(void); INLINE void prefix##_4f(void); \
	INLINE void prefix##_50(void); INLINE void prefix##_51(void); INLINE void prefix##_52(void); INLINE void prefix##_53(void); \
	INLINE void prefix##_54(void); INLINE void prefix##_55(void); INLINE void prefix##_56(void); INLINE void prefix##_57(void); \
	INLINE void prefix##_58(void); INLINE void prefix##_59(void); INLINE void prefix##_5a(void); INLINE void prefix##_5b(void); \
	INLINE void prefix##_5c(void); INLINE void prefix##_5d(void); INLINE void prefix##_5e(void); INLINE void prefix##_5f(void); \
	INLINE void prefix##_60(void); INLINE void prefix##_61(void); INLINE void prefix##_62(void); INLINE void prefix##_63(void); \
	INLINE void prefix##_64(void); INLINE void prefix##_65(void); INLINE void prefix##_66(void); INLINE void prefix##_67(void); \
	INLINE void prefix##_68(void); INLINE void prefix##_69(void); INLINE void prefix##_6a(void); INLINE void prefix##_6b(void); \
	INLINE void prefix##_6c(void); INLINE void prefix##_6d(void); INLINE void prefix##_6e(void); INLINE void prefix##_6f(void); \
	INLINE void prefix##_70(void); INLINE void prefix##_71(void); INLINE void prefix##_72(void); INLINE void prefix##_73(void); \
	INLINE void prefix##_74(void); INLINE void prefix##_75(void); INLINE void prefix##_76(void); INLINE void prefix##_77(void); \
	INLINE void prefix##_78(void); INLINE void prefix##_79(void); INLINE void prefix##_7a(void); INLINE void prefix##_7b(void); \
	INLINE void prefix##_7c(void); INLINE void prefix##_7d(void); INLINE void prefix##_7e(void); INLINE void prefix##_7f(void); \
	INLINE void prefix##_80(void); INLINE void prefix##_81(void); INLINE void prefix##_82(void); INLINE void prefix##_83(void); \
	INLINE void prefix##_84(void); INLINE void prefix##_85(void); INLINE void prefix##_86(void); INLINE void prefix##_87(void); \
	INLINE void prefix##_88(void); INLINE void prefix##_89(void); INLINE void prefix##_8a(void); INLINE void prefix##_8b(void); \
	INLINE void prefix##_8c(void); INLINE void prefix##_8d(void); INLINE void prefix##_8e(void); INLINE void prefix##_8f(void); \
	INLINE void prefix##_90(void); INLINE void prefix##_91(void); INLINE void prefix##_92(void); INLINE void prefix##_93(void); \
	INLINE void prefix##_94(void); INLINE void prefix##_95(void); INLINE void prefix##_96(void); INLINE void prefix##_97(void); \
	INLINE void prefix##_98(void); INLINE void prefix##_99(void); INLINE void prefix##_9a(void); INLINE void prefix##_9b(void); \
	INLINE void prefix##_9c(void); INLINE void prefix##_9d(void); INLINE void prefix##_9e(void); INLINE void prefix##_9f(void); \
	INLINE void prefix##_a0(void); INLINE void prefix##_a1(void); INLINE void prefix##_a2(void); INLINE void prefix##_a3(void); \
	INLINE void prefix##_a4(void); INLINE void prefix##_a5(void); INLINE void prefix##_a6(void); INLINE void prefix##_a7(void); \
	INLINE void prefix##_a8(void); INLINE void prefix##_a9(void); INLINE void prefix##_aa(void); INLINE void prefix##_ab(void); \
	INLINE void prefix##_ac(void); INLINE void prefix##_ad(void); INLINE void prefix##_ae(void); INLINE void prefix##_af(void); \
	INLINE void prefix##_b0(void); INLINE void prefix##_b1(void); INLINE void prefix##_b2(void); INLINE void prefix##_b3(void); \
	INLINE void prefix##_b4(void); INLINE void prefix##_b5(void); INLINE void prefix##_b6(void); INLINE void prefix##_b7(void); \
	INLINE void prefix##_b8(void); INLINE void prefix##_b9(void); INLINE void prefix##_ba(void); INLINE void prefix##_bb(void); \
	INLINE void prefix##_bc(void); INLINE void prefix##_bd(void); INLINE void prefix##_be(void); INLINE void prefix##_bf(void); \
	INLINE void prefix##_c0(void); INLINE void prefix##_c1(void); INLINE void prefix##_c2(void); INLINE void prefix##_c3(void); \
	INLINE void prefix##_c4(void); INLINE void prefix##_c5(void); INLINE void prefix##_c6(void); INLINE void prefix##_c7(void); \
	INLINE void prefix##_c8(void); INLINE void prefix##_c9(void); INLINE void prefix##_ca(void); INLINE void prefix##_cb(void); \
	INLINE void prefix##_cc(void); INLINE void prefix##_cd(void); INLINE void prefix##_ce(void); INLINE void prefix##_cf(void); \
	INLINE void prefix##_d0(void); INLINE void prefix##_d1(void); INLINE void prefix##_d2(void); INLINE void prefix##_d3(void); \
	INLINE void prefix##_d4(void); INLINE void prefix##_d5(void); INLINE void prefix##_d6(void); INLINE void prefix##_d7(void); \
	INLINE void prefix##_d8(void); INLINE void prefix##_d9(void); INLINE void prefix##_da(void); INLINE void prefix##_db(void); \
	INLINE void prefix##_dc(void); INLINE void prefix##_dd(void); INLINE void prefix##_de(void); INLINE void prefix##_df(void); \
	INLINE void prefix##_e0(void); INLINE void prefix##_e1(void); INLINE void prefix##_e2(void); INLINE void prefix##_e3(void); \
	INLINE void prefix##_e4(void); INLINE void prefix##_e5(void); INLINE void prefix##_e6(void); INLINE void prefix##_e7(void); \
	INLINE void prefix##_e8(void); INLINE void prefix##_e9(void); INLINE void prefix##_ea(void); INLINE void prefix##_eb(void); \
	INLINE void prefix##_ec(void); INLINE void prefix##_ed(void); INLINE void prefix##_ee(void); INLINE void prefix##_ef(void); \
	INLINE void prefix##_f0(void); INLINE void prefix##_f1(void); INLINE void prefix##_f2(void); INLINE void prefix##_f3(void); \
	INLINE void prefix##_f4(void); INLINE void prefix##_f5(void); INLINE void prefix##_f6(void); INLINE void prefix##_f7(void); \
	INLINE void prefix##_f8(void); INLINE void prefix##_f9(void); INLINE void prefix##_fa(void); INLINE void prefix##_fb(void); \
	INLINE void prefix##_fc(void); INLINE void prefix##_fd(void); INLINE void prefix##_fe(void); INLINE void prefix##_ff(void); \
static const funcptr tablename[0x100] = {	\
	prefix##_00,prefix##_01,prefix##_02,prefix##_03,prefix##_04,prefix##_05,prefix##_06,prefix##_07, \
	prefix##_08,prefix##_09,prefix##_0a,prefix##_0b,prefix##_0c,prefix##_0d,prefix##_0e,prefix##_0f, \
	prefix##_10,prefix##_11,prefix##_12,prefix##_13,prefix##_14,prefix##_15,prefix##_16,prefix##_17, \
	prefix##_18,prefix##_19,prefix##_1a,prefix##_1b,prefix##_1c,prefix##_1d,prefix##_1e,prefix##_1f, \
	prefix##_20,prefix##_21,prefix##_22,prefix##_23,prefix##_24,prefix##_25,prefix##_26,prefix##_27, \
	prefix##_28,prefix##_29,prefix##_2a,prefix##_2b,prefix##_2c,prefix##_2d,prefix##_2e,prefix##_2f, \
	prefix##_30,prefix##_31,prefix##_32,prefix##_33,prefix##_34,prefix##_35,prefix##_36,prefix##_37, \
	prefix##_38,prefix##_39,prefix##_3a,prefix##_3b,prefix##_3c,prefix##_3d,prefix##_3e,prefix##_3f, \
	prefix##_40,prefix##_41,prefix##_42,prefix##_43,prefix##_44,prefix##_45,prefix##_46,prefix##_47, \
	prefix##_48,prefix##_49,prefix##_4a,prefix##_4b,prefix##_4c,prefix##_4d,prefix##_4e,prefix##_4f, \
	prefix##_50,prefix##_51,prefix##_52,prefix##_53,prefix##_54,prefix##_55,prefix##_56,prefix##_57, \
	prefix##_58,prefix##_59,prefix##_5a,prefix##_5b,prefix##_5c,prefix##_5d,prefix##_5e,prefix##_5f, \
	prefix##_60,prefix##_61,prefix##_62,prefix##_63,prefix##_64,prefix##_65,prefix##_66,prefix##_67, \
	prefix##_68,prefix##_69,prefix##_6a,prefix##_6b,prefix##_6c,prefix##_6d,prefix##_6e,prefix##_6f, \
	prefix##_70,prefix##_71,prefix##_72,prefix##_73,prefix##_74,prefix##_75,prefix##_76,prefix##_77, \
	prefix##_78,prefix##_79,prefix##_7a,prefix##_7b,prefix##_7c,prefix##_7d,prefix##_7e,prefix##_7f, \
	prefix##_80,prefix##_81,prefix##_82,prefix##_83,prefix##_84,prefix##_85,prefix##_86,prefix##_87, \
	prefix##_88,prefix##_89,prefix##_8a,prefix##_8b,prefix##_8c,prefix##_8d,prefix##_8e,prefix##_8f, \
	prefix##_90,prefix##_91,prefix##_92,prefix##_93,prefix##_94,prefix##_95,prefix##_96,prefix##_97, \
	prefix##_98,prefix##_99,prefix##_9a,prefix##_9b,prefix##_9c,prefix##_9d,prefix##_9e,prefix##_9f, \
	prefix##_a0,prefix##_a1,prefix##_a2,prefix##_a3,prefix##_a4,prefix##_a5,prefix##_a6,prefix##_a7, \
	prefix##_a8,prefix##_a9,prefix##_aa,prefix##_ab,prefix##_ac,prefix##_ad,prefix##_ae,prefix##_af, \
	prefix##_b0,prefix##_b1,prefix##_b2,prefix##_b3,prefix##_b4,prefix##_b5,prefix##_b6,prefix##_b7, \
	prefix##_b8,prefix##_b9,prefix##_ba,prefix##_bb,prefix##_bc,prefix##_bd,prefix##_be,prefix##_bf, \
	prefix##_c0,prefix##_c1,prefix##_c2,prefix##_c3,prefix##_c4,prefix##_c5,prefix##_c6,prefix##_c7, \
	prefix##_c8,prefix##_c9,prefix##_ca,prefix##_cb,prefix##_cc,prefix##_cd,prefix##_ce,prefix##_cf, \
	prefix##_d0,prefix##_d1,prefix##_d2,prefix##_d3,prefix##_d4,prefix##_d5,prefix##_d6,prefix##_d7, \
	prefix##_d8,prefix##_d9,prefix##_da,prefix##_db,prefix##_dc,prefix##_dd,prefix##_de,prefix##_df, \
	prefix##_e0,prefix##_e1,prefix##_e2,prefix##_e3,prefix##_e4,prefix##_e5,prefix##_e6,prefix##_e7, \
	prefix##_e8,prefix##_e9,prefix##_ea,prefix##_eb,prefix##_ec,prefix##_ed,prefix##_ee,prefix##_ef, \
	prefix##_f0,prefix##_f1,prefix##_f2,prefix##_f3,prefix##_f4,prefix##_f5,prefix##_f6,prefix##_f7, \
	prefix##_f8,prefix##_f9,prefix##_fa,prefix##_fb,prefix##_fc,prefix##_fd,prefix##_fe,prefix##_ff  \
}

/***************************************************************
 * define an opcode function
 ***************************************************************/
#define OP(prefix,opcode)  INLINE void prefix##_##opcode(void)

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define CC(prefix,opcode) z80_ICount -= cc[Z80_TABLE_##prefix][opcode]

/***************************************************************
 * execute an opcode
 ***************************************************************/
#define EXEC(prefix,opcode) 									\
{																\
	unsigned op = opcode;										\
	CC(prefix,op);												\
	(*Z80##prefix[op])();										\
}

/***************************************************************
 * Enter HALT state; write 1 to fake port on first execution
 ***************************************************************/
#define ENTER_HALT {											\
	_PC--;														\
	_HALT = 1;													\
	if( !Z80.after_EI ) 											\
		z80_burn( z80_ICount ); 								\
}

/***************************************************************
 * Leave HALT state; write 0 to fake port
 ***************************************************************/
#define LEAVE_HALT {											\
	if( _HALT ) 												\
	{															\
		_HALT = 0;												\
		_PC++;													\
	}															\
}

/***************************************************************
 * Input a byte from given I/O port
 ***************************************************************/
#define IN(port)   ((UINT8)z80_readport16(port))

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
#define OUT(port,value) z80_writeport16(port,value)

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
//#define RM(addr) (UINT8)z80_readmem16(addr)
#define RM(addr) (mame_z80mem[addr])

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
//#define WM(addr,value) z80_writemem16(addr,value)
/* @todo (Tmesys#1#12/05/22): Original : check write to ROM */
#define WM(addr,value) (mame_z80mem[addr]=value)

/***************************************************************
 * Calculate the effective address Z80.EA of an opcode using
 * IX+offset resp. IY+offset addressing.
 ***************************************************************/
#define EAX Z80.EA = (UINT32)(UINT16)(_IX+(INT8)ARG())
#define EAY Z80.EA = (UINT32)(UINT16)(_IY+(INT8)ARG())

/***************************************************************
 * POP
 ***************************************************************/
#define POP(DR) { RM16( _SPD, &Z80.DR ); _SP += 2; }

/***************************************************************
 * PUSH
 ***************************************************************/
#define PUSH(SR) { _SP -= 2; WM16( _SPD, &Z80.SR ); }

/***************************************************************
 * JP
 ***************************************************************/
#if BUSY_LOOP_HACKS
#define JP {													\
	unsigned oldpc = _PCD-1;									\
	_PCD = ARG16(); 											\
	change_pc16(_PCD);											\
	/* speed up busy loop */									\
	if( _PCD == oldpc ) 										\
	{															\
		if( !Z80.after_EI ) 										\
			BURNODD( z80_ICount, 1, cc[Z80_TABLE_op][0xc3] );	\
	}															\
	else														\
	{															\
		UINT8 op = RM(_PCD);							\
		if( _PCD == oldpc-1 )									\
		{														\
			/* NOP - JP $-1 or EI - JP $-1 */					\
			if ( op == 0x00 || op == 0xfb ) 					\
			{													\
				if( !Z80.after_EI ) 								\
					BURNODD( z80_ICount-cc[Z80_TABLE_op][0x00], \
						2, cc[Z80_TABLE_op][0x00]+cc[Z80_TABLE_op][0xc3]); \
			}													\
		}														\
		else													\
		/* LD SP,#xxxx - JP $-3 (Galaga) */ 					\
		if( _PCD == oldpc-3 && op == 0x31 ) 					\
		{														\
			if( !Z80.after_EI ) 									\
				BURNODD( z80_ICount-cc[Z80_TABLE_op][0x31], 	\
					2, cc[Z80_TABLE_op][0x31]+cc[Z80_TABLE_op][0xc3]); \
		}														\
	}															\
}
#else
#define JP {													\
	_PCD = ARG16(); 											\
	change_pc16(_PCD);											\
}
#endif

/***************************************************************
 * JP_COND
 ***************************************************************/

#define JP_COND(cond)											\
	if( cond )													\
	{															\
		_PCD = ARG16(); 										\
		change_pc16(_PCD);										\
	}															\
	else														\
	{															\
		_PC += 2;												\
	}

/***************************************************************
 * JR
 ***************************************************************/
#define JR()													\
{																\
	unsigned oldpc = _PCD-1;									\
	INT8 arg = (INT8)ARG(); /* ARG() also increments _PC */ 	\
	_PC += arg; 			/* so don't do _PC += ARG() */      \
	change_pc16(_PCD);											\
	/* speed up busy loop */									\
	if( _PCD == oldpc ) 										\
	{															\
		if( !Z80.after_EI ) 										\
			BURNODD( z80_ICount, 1, cc[Z80_TABLE_op][0x18] );	\
	}															\
	else														\
	{															\
		UINT8 op = RM(_PCD);							\
		if( _PCD == oldpc-1 )									\
		{														\
			/* NOP - JR $-1 or EI - JR $-1 */					\
			if ( op == 0x00 || op == 0xfb ) 					\
			{													\
				if( !Z80.after_EI ) 								\
				   BURNODD( z80_ICount-cc[Z80_TABLE_op][0x00],	\
					   2, cc[Z80_TABLE_op][0x00]+cc[Z80_TABLE_op][0x18]); \
			}													\
		}														\
		else													\
		/* LD SP,#xxxx - JR $-3 */								\
		if( _PCD == oldpc-3 && op == 0x31 ) 					\
		{														\
			if( !Z80.after_EI ) 									\
			   BURNODD( z80_ICount-cc[Z80_TABLE_op][0x31],		\
				   2, cc[Z80_TABLE_op][0x31]+cc[Z80_TABLE_op][0x18]); \
		}														\
	}															\
}

/***************************************************************
 * JR_COND
 ***************************************************************/
#define JR_COND(cond,opcode)									\
	if( cond )													\
	{															\
		INT8 arg = (INT8)ARG(); /* ARG() also increments _PC */ \
		_PC += arg; 			/* so don't do _PC += ARG() */  \
		CC(ex,opcode);											\
		change_pc16(_PCD);										\
	}															\
	else _PC++; 												\

/***************************************************************
 * CALL
 ***************************************************************/
#define CALL()													\
	Z80.EA = ARG16();												\
	PUSH( PC ); 												\
	_PCD = Z80.EA;													\
	change_pc16(_PCD)

/***************************************************************
 * CALL_COND
 ***************************************************************/
#define CALL_COND(cond,opcode)									\
	if( cond )													\
	{															\
		Z80.EA = ARG16();											\
		PUSH( PC ); 											\
		_PCD = Z80.EA;												\
		CC(ex,opcode);											\
		change_pc16(_PCD);										\
	}															\
	else														\
	{															\
		_PC+=2; 												\
	}

/***************************************************************
 * RET_COND
 ***************************************************************/
#define RET_COND(cond,opcode)									\
	if( cond )													\
	{															\
		POP(PC);												\
		change_pc16(_PCD);										\
		CC(ex,opcode);											\
	}

/***************************************************************
 * RETN
 ***************************************************************/
#define RETN	{												\
	LOG(("Z80 #%d RETN IFF1:%d IFF2:%d\n", cpu_getactivecpu(), _IFF1, _IFF2)); \
	POP(PC);													\
	change_pc16(_PCD);											\
	if( _IFF1 == 0 && _IFF2 == 1 )								\
	{															\
		_IFF1 = 1;												\
		if( Z80.irq_state != CLEAR_LINE ||						\
			Z80.request_irq >= 0 )								\
		{														\
			LOG(("Z80 #%d RETN takes IRQ\n",                    \
				cpu_getactivecpu()));							\
			take_interrupt();									\
		}														\
	}															\
	else _IFF1 = _IFF2; 										\
}

/***************************************************************
 * RETI
 ***************************************************************/
#define RETI	{												\
	int device = Z80.service_irq;								\
	POP(PC);													\
	change_pc16(_PCD);											\
/* according to http://www.msxnet.org/tech/Z80/z80undoc.txt */	\
/*	_IFF1 = _IFF2;	*/											\
	if( device >= 0 )											\
	{															\
		LOG(("Z80 #%d RETI device %d: $%02x\n",                 \
			cpu_getactivecpu(), device, Z80.irq[device].irq_param)); \
		Z80.irq[device].interrupt_reti(Z80.irq[device].irq_param); \
	}															\
}

/***************************************************************
 * LD	R,A
 ***************************************************************/
#define LD_R_A {												\
	_R = _A;													\
	_R2 = _A & 0x80;				/* keep bit 7 of R */		\
}

/***************************************************************
 * LD	A,R
 ***************************************************************/
#define LD_A_R {												\
	_A = (_R & 0x7f) | _R2; 									\
	_F = (_F & CF) | SZ[_A] | ( _IFF2 << 2 );					\
}

/***************************************************************
 * LD	I,A
 ***************************************************************/
#define LD_I_A {												\
	_I = _A;													\
}

/***************************************************************
 * LD	A,I
 ***************************************************************/
#define LD_A_I {												\
	_A = _I;													\
	_F = (_F & CF) | SZ[_A] | ( _IFF2 << 2 );					\
}

/***************************************************************
 * RST
 ***************************************************************/
#define RST(addr)												\
	PUSH( PC ); 												\
	_PCD = addr;												\
	change_pc16(_PCD)

/***************************************************************
 * RLCA
 ***************************************************************/
#define RLCA													\
	_A = (_A << 1) | (_A >> 7); 								\
	_F = (_F & (SF | ZF | PF)) | (_A & (YF | XF | CF))

/***************************************************************
 * RRCA
 ***************************************************************/
#define RRCA													\
	_F = (_F & (SF | ZF | PF)) | (_A & CF); 					\
	_A = (_A >> 1) | (_A << 7); 								\
	_F |= (_A & (YF | XF) )

/***************************************************************
 * RLA
 ***************************************************************/
#define RLA {													\
	UINT8 res = (_A << 1) | (_F & CF);							\
	UINT8 c = (_A & 0x80) ? CF : 0; 							\
	_F = (_F & (SF | ZF | PF)) | c | (res & (YF | XF)); 		\
	_A = res;													\
}

/***************************************************************
 * RRA
 ***************************************************************/
#define RRA {													\
	UINT8 res = (_A >> 1) | (_F << 7);							\
	UINT8 c = (_A & 0x01) ? CF : 0; 							\
	_F = (_F & (SF | ZF | PF)) | c | (res & (YF | XF)); 		\
	_A = res;													\
}

/***************************************************************
 * RRD
 ***************************************************************/
#define RRD {													\
	UINT8 n = RM(_HL);											\
	WM( _HL, (n >> 4) | (_A << 4) );							\
	_A = (_A & 0xf0) | (n & 0x0f);								\
	_F = (_F & CF) | SZP[_A];									\
}

/***************************************************************
 * RLD
 ***************************************************************/
#define RLD {													\
	UINT8 n = RM(_HL);											\
	WM( _HL, (n << 4) | (_A & 0x0f) );							\
	_A = (_A & 0xf0) | (n >> 4);								\
	_F = (_F & CF) | SZP[_A];									\
}

/***************************************************************
 * ADD	A,n
 ***************************************************************/
#define ADD(value)												\
{																\
	UINT32 ah = _AFD & 0xff00;									\
	UINT32 res = (UINT8)((ah >> 8) + value);					\
	_F = SZHVC_add[ah | res];									\
	_A = res;													\
}

/***************************************************************
 * ADC	A,n
 ***************************************************************/
#define ADC(value)												\
{																\
	UINT32 ah = _AFD & 0xff00, c = _AFD & 1;					\
	UINT32 res = (UINT8)((ah >> 8) + value + c);				\
	_F = SZHVC_add[(c << 16) | ah | res];						\
	_A = res;													\
}

/***************************************************************
 * SUB	n
 ***************************************************************/
#define SUB(value)												\
{																\
	UINT32 ah = _AFD & 0xff00;									\
	UINT32 res = (UINT8)((ah >> 8) - value);					\
	_F = SZHVC_sub[ah | res];									\
	_A = res;													\
}

/***************************************************************
 * SBC	A,n
 ***************************************************************/
#define SBC(value)												\
{																\
	UINT32 ah = _AFD & 0xff00, c = _AFD & 1;					\
	UINT32 res = (UINT8)((ah >> 8) - value - c);				\
	_F = SZHVC_sub[(c<<16) | ah | res]; 						\
	_A = res;													\
}

/***************************************************************
 * NEG
 ***************************************************************/
#define NEG {													\
	UINT8 value = _A;											\
	_A = 0; 													\
	SUB(value); 												\
}

/***************************************************************
 * DAA
 ***************************************************************/
#define DAA {                                       \
  UINT8 a = _A;                                      \
  if (_F & NF) {                                     \
    if ((_F&HF) | ((_A&0xf)>9)) a-=6;                 \
    if ((_F&CF) | (_A>0x99)) a-=0x60;                 \
  }                                                 \
  else {                                            \
    if ((_F&HF) | ((_A&0xf)>9)) a+=6;                 \
    if ((_F&CF) | (_A>0x99)) a+=0x60;                 \
  }                                                 \
                                                    \
  _F = (_F&(CF|NF)) | (_A>0x99) | ((_A^a)&HF) | SZP[a]; \
  _A = a;                                            \
}

/***************************************************************
 * AND	n
 ***************************************************************/
#define AND(value)												\
	_A &= value;												\
	_F = SZP[_A] | HF

/***************************************************************
 * OR	n
 ***************************************************************/
#define OR(value)												\
	_A |= value;												\
	_F = SZP[_A]

/***************************************************************
 * XOR	n
 ***************************************************************/
#define XOR(value)												\
	_A ^= value;												\
	_F = SZP[_A]

/***************************************************************
 * CP	n
 ***************************************************************/
#define CP(value)												\
{																\
	unsigned val = value;										\
	UINT32 ah = _AFD & 0xff00;									\
	UINT32 res = (UINT8)((ah >> 8) - val);						\
	_F = (SZHVC_sub[ah | res] & ~(YF | XF)) |					\
		(val & (YF | XF));										\
}

/***************************************************************
 * EX	AF,AF'
 ***************************************************************/
#define EX_AF { 												\
	PAIR tmp;													\
	tmp = Z80.AF; Z80.AF = Z80.AF2; Z80.AF2 = tmp;				\
}

/***************************************************************
 * EX	DE,HL
 ***************************************************************/
#define EX_DE_HL {												\
	PAIR tmp;													\
	tmp = Z80.DE; Z80.DE = Z80.HL; Z80.HL = tmp;				\
}

/***************************************************************
 * EXX
 ***************************************************************/
#define EXX {													\
	PAIR tmp;													\
	tmp = Z80.BC; Z80.BC = Z80.BC2; Z80.BC2 = tmp;				\
	tmp = Z80.DE; Z80.DE = Z80.DE2; Z80.DE2 = tmp;				\
	tmp = Z80.HL; Z80.HL = Z80.HL2; Z80.HL2 = tmp;				\
}

/***************************************************************
 * EX	(SP),r16
 ***************************************************************/
#define EXSP(DR)												\
{																\
	PAIR tmp = { { 0, 0, 0, 0 } };								\
	RM16( _SPD, &tmp ); 										\
	WM16( _SPD, &Z80.DR );										\
	Z80.DR = tmp;												\
}


/***************************************************************
 * ADD16
 ***************************************************************/
#define ADD16(DR,SR)											\
{																\
	UINT32 res = Z80.DR.d + Z80.SR.d;							\
	_F = (_F & (SF | ZF | VF)) |								\
		(((Z80.DR.d ^ res ^ Z80.SR.d) >> 8) & HF) | 			\
		((res >> 16) & CF) | ((res >> 8) & (YF | XF)); 			\
	Z80.DR.w.l = (UINT16)res;									\
}

/***************************************************************
 * ADC	r16,r16
 ***************************************************************/
#define ADC16(Reg)												\
{																\
	UINT32 res = _HLD + Z80.Reg.d + (_F & CF);					\
	_F = (((_HLD ^ res ^ Z80.Reg.d) >> 8) & HF) |				\
		((res >> 16) & CF) |									\
		((res >> 8) & (SF | YF | XF)) |							\
		((res & 0xffff) ? 0 : ZF) | 							\
		(((Z80.Reg.d ^ _HLD ^ 0x8000) & (Z80.Reg.d ^ res) & 0x8000) >> 13); \
	_HL = (UINT16)res;											\
}

/***************************************************************
 * SBC	r16,r16
 ***************************************************************/
#define SBC16(Reg)												\
{																\
	UINT32 res = _HLD - Z80.Reg.d - (_F & CF);					\
	_F = (((_HLD ^ res ^ Z80.Reg.d) >> 8) & HF) | NF |			\
		((res >> 16) & CF) |									\
		((res >> 8) & (SF | YF | XF)) | 						\
		((res & 0xffff) ? 0 : ZF) | 							\
		(((Z80.Reg.d ^ _HLD) & (_HLD ^ res) &0x8000) >> 13);	\
	_HL = (UINT16)res;											\
}

/***************************************************************
 * BIT	bit,r8
 ***************************************************************/
#define BIT(bit,reg)											\
	_F = (_F & CF) | HF | SZ_BIT[reg & (1<<bit)]

/***************************************************************
 * BIT	bit,(IX/Y+o)
 ***************************************************************/
#define BIT_XY(bit,reg) 										\
	_F = (_F & CF) | HF | (SZ_BIT[reg & (1<<bit)] & ~(YF|XF)) | ((Z80.EA>>8) & (YF|XF))

/***************************************************************
 * LDI
 ***************************************************************/
#define LDI {													\
	UINT8 io = RM(_HL); 										\
	WM( _DE, io );												\
	_F &= SF | ZF | CF; 										\
	if( (_A + io) & 0x02 ) _F |= YF; /* bit 1 -> flag 5 */		\
	if( (_A + io) & 0x08 ) _F |= XF; /* bit 3 -> flag 3 */		\
	_HL++; _DE++; _BC--;										\
	if( _BC ) _F |= VF; 										\
}

/***************************************************************
 * CPI
 ***************************************************************/
#define CPI {													\
	UINT8 val = RM(_HL);										\
	UINT8 res = _A - val;										\
	_HL++; _BC--;												\
	_F = (_F & CF) | (SZ[res] & ~(YF|XF)) | ((_A ^ val ^ res) & HF) | NF;  \
	if( _F & HF ) res -= 1; 									\
	if( res & 0x02 ) _F |= YF; /* bit 1 -> flag 5 */			\
	if( res & 0x08 ) _F |= XF; /* bit 3 -> flag 3 */			\
	if( _BC ) _F |= VF; 										\
}

/***************************************************************
 * INI
 ***************************************************************/
#define INI {													\
	UINT8 io = IN(_BC); 										\
	_B--;														\
	WM( _HL, io );												\
	_HL++;														\
	_F = SZ[_B];												\
	if( io & SF ) _F |= NF; 									\
	if( ( ( (_C + 1) & 0xff) + io) & 0x100 ) _F |= HF | CF; 	\
	if( (irep_tmp1[_C & 3][io & 3] ^							\
		 breg_tmp2[_B] ^										\
		 (_C >> 2) ^											\
		 (io >> 2)) & 1 )										\
		_F |= PF;												\
}

/***************************************************************
 * OUTI
 ***************************************************************/
#define OUTI {													\
	UINT8 io = RM(_HL); 										\
	_B--;														\
	OUT( _BC, io ); 											\
	_HL++;														\
	_F = SZ[_B];												\
	if( io & SF ) _F |= NF; 									\
	if( ( ( (_C + 1) & 0xff) + io) & 0x100 ) _F |= HF | CF; 	\
	if( (irep_tmp1[_C & 3][io & 3] ^							\
		 breg_tmp2[_B] ^										\
		 (_C >> 2) ^											\
		 (io >> 2)) & 1 )										\
		_F |= PF;												\
}

/***************************************************************
 * LDD
 ***************************************************************/
#define LDD {													\
	UINT8 io = RM(_HL); 										\
	WM( _DE, io );												\
	_F &= SF | ZF | CF; 										\
	if( (_A + io) & 0x02 ) _F |= YF; /* bit 1 -> flag 5 */		\
	if( (_A + io) & 0x08 ) _F |= XF; /* bit 3 -> flag 3 */		\
	_HL--; _DE--; _BC--;										\
	if( _BC ) _F |= VF; 										\
}

/***************************************************************
 * CPD
 ***************************************************************/
#define CPD {													\
	UINT8 val = RM(_HL);										\
	UINT8 res = _A - val;										\
	_HL--; _BC--;												\
	_F = (_F & CF) | (SZ[res] & ~(YF|XF)) | ((_A ^ val ^ res) & HF) | NF;  \
	if( _F & HF ) res -= 1; 									\
	if( res & 0x02 ) _F |= YF; /* bit 1 -> flag 5 */			\
	if( res & 0x08 ) _F |= XF; /* bit 3 -> flag 3 */			\
	if( _BC ) _F |= VF; 										\
}


/***************************************************************
 * IND
 ***************************************************************/
#define IND {													\
	UINT8 io = IN(_BC); 										\
	_B--;														\
	WM( _HL, io );												\
	_HL--;														\
	_F = SZ[_B];												\
	if( io & SF ) _F |= NF; 									\
	if( ( ( (_C - 1) & 0xff) + io) & 0x100 ) _F |= HF | CF; 	\
	if( (drep_tmp1[_C & 3][io & 3] ^							\
		 breg_tmp2[_B] ^										\
		 (_C >> 2) ^											\
		 (io >> 2)) & 1 )										\
		_F |= PF;												\
}

/***************************************************************
 * OUTD
 ***************************************************************/
#define OUTD {													\
	UINT8 io = RM(_HL); 										\
	_B--;														\
	OUT( _BC, io ); 											\
	_HL--;														\
	_F = SZ[_B];												\
	if( io & SF ) _F |= NF; 									\
	if( ( ( (_C - 1) & 0xff) + io) & 0x100 ) _F |= HF | CF; 	\
	if( (drep_tmp1[_C & 3][io & 3] ^							\
		 breg_tmp2[_B] ^										\
		 (_C >> 2) ^											\
		 (io >> 2)) & 1 )										\
		_F |= PF;												\
}

/***************************************************************
 * LDIR
 ***************************************************************/
#define LDIR													\
	LDI;														\
	if( _BC )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xb0);											\
	}

/***************************************************************
 * CPIR
 ***************************************************************/
#define CPIR													\
	CPI;														\
	if( _BC && !(_F & ZF) ) 									\
	{															\
		_PC -= 2;												\
		CC(ex,0xb1);											\
	}

/***************************************************************
 * INIR
 ***************************************************************/
#define INIR													\
	INI;														\
	if( _B )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xb2);											\
	}

/***************************************************************
 * OTIR
 ***************************************************************/
#define OTIR													\
	OUTI;														\
	if( _B )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xb3);											\
	}

/***************************************************************
 * LDDR
 ***************************************************************/
#define LDDR													\
	LDD;														\
	if( _BC )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xb8);											\
	}

/***************************************************************
 * CPDR
 ***************************************************************/
#define CPDR													\
	CPD;														\
	if( _BC && !(_F & ZF) ) 									\
	{															\
		_PC -= 2;												\
		CC(ex,0xb9);											\
	}

/***************************************************************
 * INDR
 ***************************************************************/
#define INDR													\
	IND;														\
	if( _B )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xba);											\
	}

/***************************************************************
 * OTDR
 ***************************************************************/
#define OTDR													\
	OUTD;														\
	if( _B )													\
	{															\
		_PC -= 2;												\
		CC(ex,0xbb);											\
	}

/***************************************************************
 * EI
 ***************************************************************/
#define EI {													\
	/* If interrupts were disabled, execute one more			\
	 * instruction and check the IRQ line.						\
	 * If not, simply set interrupt flip-flop 2 				\
	 */ 														\
	if( _IFF1 == 0 )											\
	{															\
		_IFF1 = _IFF2 = 1;										\
		_PPC = _PCD;											\
		CALL_MAME_DEBUG;										\
		_R++;													\
		while( RM(_PCD) == 0xfb ) /* more EIs? */		\
		{														\
			LOG(("Z80 #%d multiple EI opcodes at %04X\n",       \
				cpu_getactivecpu(), _PC));						\
			CC(op,0xfb);										\
			_PPC =_PCD; 										\
			CALL_MAME_DEBUG;									\
			_PC++;												\
			_R++;												\
		}														\
		if( Z80.irq_state != CLEAR_LINE ||						\
			Z80.request_irq >= 0 )								\
		{														\
			Z80.after_EI = 1;	/* avoid cycle skip hacks */		\
			EXEC(op,ROP()); 									\
			Z80.after_EI = 0;										\
			LOG(("Z80 #%d EI takes irq\n", cpu_getactivecpu())); \
			take_interrupt();									\
		} else EXEC(op,ROP());									\
	} else _IFF2 = 1;											\
}

/* ----- Extracted from MAME cpuintrf --------- */

enum
{
    /* line states */
    CLEAR_LINE = 0,  /* clear (a fired, held or pulsed) line */
    ASSERT_LINE,     /* assert an interrupt immediately */
    HOLD_LINE,       /* hold interrupt line until acknowledged */
    PULSE_LINE,      /* pulse interrupt line -for one instruction */

    /* internal flags (not for use by drivers!) */
    INTERNAL_CLEAR_LINE = 100 + CLEAR_LINE,
    INTERNAL_ASSERT_LINE = 100 + ASSERT_LINE,

    /* interrupt parameters */
    MAX_IRQ_LINES = 16,                     /* maximum number of IRQ lines per CPU */
};

enum
{
    MAX_REGS = 128,                         /* maximum number of register of any CPU */

    /* This value is passed to activecpu_get_reg to retrieve the previous
     * program counter value, ie. before a CPU emulation started
     * to fetch opcodes and arguments for the current instrution. */
    REG_PREVIOUSPC = -1,

    /* This value is passed to activecpu_get_reg to retrieve the current
     * program counter value. */
    REG_PC = -2,

    /* This value is passed to activecpu_get_reg to retrieve the current
     * stack pointer value. */
    REG_SP = -3,

    /* This value is passed to activecpu_get_reg/activecpu_set_reg, instead of one of
     * the names from the enum a CPU core defines for it's registers,
     * to get or set the contents of the memory pointed to by a stack pointer.
     * You can specify the n'th element on the stack by (REG_SP_CONTENTS-n),
     * ie. lower negative values. The actual element size (UINT16 or UINT32)
     * depends on the CPU core. */
    REG_SP_CONTENTS = -4
};


/* ---- END of mame extract ----- */

enum
{
    CPU_INFO_REG,
    CPU_INFO_FLAGS = MAX_REGS,
    CPU_INFO_NAME,
    CPU_INFO_FAMILY,
    CPU_INFO_VERSION,
    CPU_INFO_FILE,
    CPU_INFO_CREDITS,
    CPU_INFO_REG_LAYOUT,
    CPU_INFO_WIN_LAYOUT
};

enum
{
    Z80_PC = 1, Z80_SP, Z80_AF, Z80_BC, Z80_DE, Z80_HL,
    Z80_IX, Z80_IY,	Z80_AF2, Z80_BC2, Z80_DE2, Z80_HL2,
    Z80_R, Z80_I, Z80_IM, Z80_IFF1, Z80_IFF2, Z80_HALT,
    Z80_NMI_STATE, Z80_IRQ_STATE, Z80_DC0, Z80_DC1, Z80_DC2, Z80_DC3
};

enum
{
    Z80_TABLE_op,
    Z80_TABLE_cb,
    Z80_TABLE_ed,
    Z80_TABLE_xy,
    Z80_TABLE_xycb,
    Z80_TABLE_ex	/* cycles counts for taken jr/jp/call and interrupt latency (rst opcodes) */
};

typedef union
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    struct
    {
        UINT8 l, h, h2, h3;
    } b;
    struct
    {
        UINT16 l, h;
    } w;
#else
    struct
    {
        UINT8 h3, h2, h, l;
    } b;
    struct
    {
        UINT16 h, l;
    } w;
#endif
    UINT32 d;
}       PAIR;

typedef struct
{
    void ( *reset ) ( int );                /* reset callback         */
    int ( *interrupt_entry ) ( int ); /* entry callback         */
    void ( *interrupt_reti ) ( int ); /* reti callback          */
    int irq_param;                                  /* callback paramater */
} Z80_DaisyChain;

/****************************************************************************/
/* The Z80 registers. HALT is set to 1 when the CPU is halted, the refresh	*/
/* register is calculated as follows: refresh=(Regs.R&127)|(Regs.R2&128)	*/
/****************************************************************************/
typedef struct
{
    PAIR	PREPC, PC, SP, AF, BC, DE, HL, IX, IY;
    PAIR	AF2, BC2, DE2, HL2;
    UINT8	R, R2, IFF1, IFF2, HALT, IM, I;
    UINT8	irq_max;			/* number of daisy chain devices */
    UINT32 EA;
    int after_EI;
    INT8	request_irq;		/* daisy chain next request device*/
    INT8	service_irq;		/* daisy chain next reti handling device */
    UINT8	nmi_state;			/* nmi line state */
    UINT8	irq_state;			/* irq line state */
    UINT8	int_state[Z80_MAXDAISY];
    int extra_cycles;		/* extra cycles for interrupts */
    Z80_DaisyChain irq[Z80_MAXDAISY];
    int	( *irq_callback ) ( int irqline );
}	Z80_Regs;

extern UINT8 mame_z80mem[0x10000];

void z80_init ( int ( *callback ) ( int ) );
void z80_reset ( void *param );
int z80_run ( int cycles, int debug );
void z80_burn ( int cycles );
void z80_set_irq_line ( unsigned int irqline, unsigned int state );
void z80_set_nmi_line ( unsigned int state );
const char *z80_info ( void *context, int regnum );
unsigned z80_get_context ( void *dst );
void z80_set_context ( void *src );
const void *z80_get_cycle_table ( int which );
void z80_set_cycle_table ( int which, void *new_tbl );
unsigned z80_get_reg ( int regnum );
void z80_set_reg ( int regnum, unsigned val );
void z80_set_irq_callback ( int ( *irq_callback ) ( int ) );
int z80_dasm ( char *buffer, unsigned pc );

/* interface */
extern void z80_writemem16 ( UINT16 addr, UINT8 val );
extern UINT8 z80_readmem16 ( UINT16 addr );
extern UINT8 z80_readop ( UINT16 addr );
extern UINT8 z80_readop_arg ( UINT16 addr );
extern UINT8 z80_readport16 ( UINT16 port );
extern void z80_writeport16 ( UINT16 port, UINT8 value );

#ifdef MAME_DEBUG
extern unsigned DasmZ80 ( char *buffer, unsigned pc );
#endif

#endif

