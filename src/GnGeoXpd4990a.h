/*!
*
*   \file    GnGeoXpd4990a.h
*   \brief   NEC PD4990A emulation routines header.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    03/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    The PD4990A is a serial I/O Calendar & Clock IC used in the NEO GEO and probably a couple of other machines.
*/
#ifndef _GNGEOX_PD4990A_H_
#define _GNGEOX_PD4990A_H_

#define DATA_BIT    0x1
#define CLOCK_BIT   0x2
#define END_BIT     0x4

typedef struct
{
    Sint32 seconds;
    Sint32 minutes;
    Sint32 hours;
    Sint32 days;
    Sint32 month;
    Sint32 year;
    Sint32 weekday;
} struct_gngeoxpd4990a_date;

#ifdef _GNGEOX_PD4990A_C_
static void pd4990a_readbit ( void );
static void pd4990a_resetbitstream ( void );
static void pd4990a_writebit ( Uint8 );
static void pd4990a_nextbit ( void );
static Uint8 pd4990a_getcommand ( void ) __attribute__ ( ( warn_unused_result ) );
static void pd4990a_update_date ( void );
static void pd4990a_process_command ( void );
static void pd4990a_serial_control ( Uint8 );
#endif // _GNGEOX_PD4990A_C_

void pd4990a_init ( void );
//void pd4990a_init_save_state(void);
void pd4990a_addretrace ( void );
Sint32 read_4990_testbit ( void ) __attribute__ ( ( warn_unused_result ) );
Sint32 read_4990_databit ( void ) __attribute__ ( ( warn_unused_result ) );
void pd4990a_increment_day ( void );
void pd4990a_increment_month ( void );
void write_4990_control_w ( Uint32, Uint32 );

#endif // _GNGEOX_PD4990A_H_
