/*!
*
*   \file    GnGeoXpd4990a.c
*   \brief   NEC PD4990A emulation routines.
*   \author  Mathieu Peponas, Espinetes, Ugenn (Original version)
*   \author  James Ponder (68K emulation) / Juergen Buchmueller (Z80 emulation) / Marat Fayzullin (Z80 disassembler).
*   \author  Tatsuyuki Satoh, Jarek Burczynski, NJ pspmvs, ElSemi (YM2610 emulation).
*   \author  Andrea Mazzoleni, Maxim Stepin (Scale/HQ2X/XBR2X effect).
*   \author  Mourad Reggadi (GnGeo-X)
*   \version 01.00
*   \date    04/12/2022
*   \warning Licensed under the terms of the GNU General Public License v2 :
*            https://tldrlegal.com/license/gnu-general-public-license-v2#fulltext
*   \note    The PD4990A is a serial I/O Calendar & Clock IC used in the NEO GEO and probably a couple of other machines.
*/
#ifndef _GNGEOX_PD4990A_C_
#define _GNGEOX_PD4990A_C_
#endif // _GNGEOX_PD4990A_C_

#include <time.h>

#include <SDL2/SDL.h>

#include "GnGeoXroms.h"
#include "GnGeoXvideo.h"
#include "GnGeoXmemory.h"
#include "GnGeoXpd4990a.h"

/* @note (Tmesys#1#12/04/2022): Set the data in the chip to Monday 09/09/73 00:00:00. */
struct_gngeoxpd4990a_date pd4990a =
{
    0x00,   /* seconds BCD */
    0x00,   /* minutes BCD */
    0x00,   /* hours   BCD */
    0x09,   /* days    BCD */
    9,      /* month   Hexadecimal form */
    0x73,   /* year    BCD */
    1       /* weekday BCD */
};

static Uint32 shiftlo = 0, shifthi = 0;
static Sint32 retraces = 0;        /* Assumes 60 retraces a second */
static Sint32 testwaits = 0;
static Sint32 maxwaits = 1;
static Sint32 testbit = 0;     /* Pulses a bit in order to simulate */
/* test output */
static Sint32 outputbit = 0;
static Sint32 bitno = 0;

static char reading = 0;
static char writing = 0;

/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
*/
/* ******************************************************************************************************************/
void pd4990a_addretrace ( void )
{
    ++testwaits;

    if ( testwaits >= maxwaits )
    {
        testbit ^= 1;
        testwaits = 0;
    }

    retraces++;

    if ( retraces < 60 )
    {
        return;
    }

    retraces = 0;
    pd4990a.seconds++;

    if ( ( pd4990a.seconds & 0x0f ) < 10 )
    {
        return;
    }

    pd4990a.seconds &= 0xf0;
    pd4990a.seconds += 0x10;

    if ( pd4990a.seconds < 0x60 )
    {
        return;
    }

    pd4990a.seconds = 0;
    pd4990a.minutes++;

    if ( ( pd4990a.minutes & 0x0f ) < 10 )
    {
        return;
    }

    pd4990a.minutes &= 0xf0;
    pd4990a.minutes += 0x10;

    if ( pd4990a.minutes < 0x60 )
    {
        return;
    }

    pd4990a.minutes = 0;
    pd4990a.hours++;

    if ( ( pd4990a.hours & 0x0f ) < 10 )
    {
        return;
    }

    pd4990a.hours &= 0xf0;
    pd4990a.hours += 0x10;

    if ( pd4990a.hours < 0x24 )
    {
        return;
    }

    pd4990a.hours = 0;
    pd4990a_increment_day();
}
/* ******************************************************************************************************************/
/*!
* \brief Initializes PD4990A.
*
*/
/* ******************************************************************************************************************/
void pd4990a_init ( void )
{
    time_t ltime;
    struct tm* today;

    pd4990a.seconds = 0x00;     /* BCD */
    pd4990a.minutes = 0x00;     /* BCD */
    pd4990a.hours = 0x00;       /* BCD */
    pd4990a.days = 0x09;        /* BCD */
    pd4990a.month = 9;          /* Hexadecimal form */
    pd4990a.year = 0x73;        /* BCD */
    pd4990a.weekday = 1;        /* BCD */

    /* Assumes 60 retraces a second */
    retraces = 0;
    /* Pulses a bit in order to simulate */
    testbit = 0;
    reading = 0;
    writing = 0;
    shiftlo = 0;
    shifthi = 0;
    /* test output */
    outputbit = 0;
    bitno = 0;

    time ( &ltime );
    today = localtime ( &ltime );

    pd4990a.seconds = ( ( today->tm_sec / 10 ) << 4 ) + ( today->tm_sec % 10 );
    pd4990a.minutes = ( ( today->tm_min / 10 ) << 4 ) + ( today->tm_min % 10 );
    pd4990a.hours = ( ( today->tm_hour / 10 ) << 4 ) + ( today->tm_hour % 10 );
    pd4990a.days = ( ( today->tm_mday / 10 ) << 4 ) + ( today->tm_mday % 10 );
    pd4990a.month = ( today->tm_mon + 1 );
    pd4990a.year = ( ( today->tm_year / 10 ) << 4 ) + ( today->tm_year % 10 );
    pd4990a.weekday = today->tm_wday;
    //pd4990a_init_save_state();
}
/* ******************************************************************************************************************/
/*!
* \brief Increments day number.
*
*/
/* ******************************************************************************************************************/
void pd4990a_increment_day ( void )
{
    Sint32 real_year;

    pd4990a.days++;

    if ( ( pd4990a.days & 0x0f ) >= 10 )
    {
        pd4990a.days &= 0xf0;
        pd4990a.days += 0x10;
    }

    pd4990a.weekday++;

    if ( pd4990a.weekday == 7 )
    {
        pd4990a.weekday = 0;
    }

    switch ( pd4990a.month )
    {
    case ( 1 ) :
    case ( 3 ) :
    case ( 5 ) :
    case ( 7 ) :
    case ( 8 ) :
    case ( 10 ) :
    case ( 12 ) :
        {
            if ( pd4990a.days == 0x32 )
            {
                pd4990a.days = 1;
                pd4990a_increment_month();
            }
        }
        break;

    case ( 2 ) :
        {
            real_year = ( pd4990a.year >> 4 ) * 10 + ( pd4990a.year & 0xf );

            if ( ( real_year % 4 ) && ( ! ( real_year % 100 ) || ( real_year % 400 ) ) )
            {
                if ( pd4990a.days == 0x29 )
                {
                    pd4990a.days = 1;
                    pd4990a_increment_month();
                }
            }
            else
            {
                if ( pd4990a.days == 0x30 )
                {
                    pd4990a.days = 1;
                    pd4990a_increment_month();
                }
            }
        }
        break;

    case ( 4 ) :
    case ( 6 ) :
    case ( 9 ) :
    case ( 11 ) :
        {
            if ( pd4990a.days == 0x31 )
            {
                pd4990a.days = 1;
                pd4990a_increment_month();
            }
        }
        break;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Increments month number.
*
*/
/* ******************************************************************************************************************/
void pd4990a_increment_month ( void )
{
    pd4990a.month++;

    if ( pd4990a.month == 13 )
    {
        pd4990a.month = 1;
        pd4990a.year++;

        if ( ( pd4990a.year & 0x0f ) >= 10 )
        {
            pd4990a.year &= 0xf0;
            pd4990a.year += 0x10;
        }

        if ( pd4990a.year == 0xA0 )
        {
            pd4990a.year = 0;
        }
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Retrieves PD4990A test bit.
*
* \return Test bit.
*/
/* ******************************************************************************************************************/
Sint32 read_4990_testbit ( void )
{
    return ( testbit );
}
/* ******************************************************************************************************************/
/*!
* \brief Retrieves PD4990A data bit.
*
* \return Data bit.
*/
/* ******************************************************************************************************************/
Sint32 read_4990_databit ( void )
{
    return ( outputbit );
}
/* ******************************************************************************************************************/
/*!
* \brief Loads PD4990A data bit.
*
*/
/* ******************************************************************************************************************/
static void pd4990a_readbit ( void )
{
    switch ( bitno )
    {
    case ( 0x00 ) :
    case ( 0x01 ) :
    case ( 0x02 ) :
    case ( 0x03 ) :
    case ( 0x04 ) :
    case ( 0x05 ) :
    case ( 0x06 ) :
    case ( 0x07 ) :
        {
            outputbit = ( pd4990a.seconds >> bitno ) & 0x01;
        }
        break;

    case ( 0x08 ) :
    case ( 0x09 ) :
    case ( 0x0a ) :
    case ( 0x0b ) :
    case ( 0x0c ) :
    case ( 0x0d ) :
    case ( 0x0e ) :
    case ( 0x0f ) :
        {
            outputbit = ( pd4990a.minutes >> ( bitno - 0x08 ) ) & 0x01;
        }
        break;

    case ( 0x10 ) :
    case ( 0x11 ) :
    case ( 0x12 ) :
    case ( 0x13 ) :
    case ( 0x14 ) :
    case ( 0x15 ) :
    case ( 0x16 ) :
    case ( 0x17 ) :
        {
            outputbit = ( pd4990a.hours >> ( bitno - 0x10 ) ) & 0x01;
        }
        break;

    case ( 0x18 ) :
    case ( 0x19 ) :
    case ( 0x1a ) :
    case ( 0x1b ) :
    case ( 0x1c ) :
    case ( 0x1d ) :
    case ( 0x1e ) :
    case ( 0x1f ) :
        {
            outputbit = ( pd4990a.days >> ( bitno - 0x18 ) ) & 0x01;
        }
        break;

    case ( 0x20 ) :
    case ( 0x21 ) :
    case ( 0x22 ) :
    case ( 0x23 ) :
        {
            outputbit = ( pd4990a.weekday >> ( bitno - 0x20 ) ) & 0x01;
        }
        break;

    case ( 0x24 ) :
    case ( 0x25 ) :
    case ( 0x26 ) :
    case ( 0x27 ) :
        {
            outputbit = ( pd4990a.month >> ( bitno - 0x24 ) ) & 0x01;
        }
        break;

    case ( 0x28 ) :
    case ( 0x29 ) :
    case ( 0x2a ) :
    case ( 0x2b ) :
    case ( 0x2c ) :
    case ( 0x2d ) :
    case ( 0x2e ) :
    case ( 0x2f ) :
        {
            outputbit = ( pd4990a.year >> ( bitno - 0x28 ) ) & 0x01;
        }
        break;

    /* @todo (Tmesys#1#12/04/2022): Some implementation maybe ? */
    case ( 0x30 ) :
    case ( 0x31 ) :
    case ( 0x32 ) :
    case ( 0x33 ) :
        /*unknown */
        break;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Resets bits.
*
*/
/* ******************************************************************************************************************/
static void pd4990a_resetbitstream ( void )
{
    shiftlo = 0;
    shifthi = 0;
    bitno = 0;
}
/* ******************************************************************************************************************/
/*!
* \brief Writes bits.
*
*/
/* ******************************************************************************************************************/
static void pd4990a_writebit ( Uint8 bit )
{
    if ( bitno <= 31 )  /*low part */
    {
        shiftlo |= bit << bitno;
    }

    else    /*high part */
    {
        shifthi |= bit << ( bitno - 32 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
*/
/* ******************************************************************************************************************/
static void pd4990a_nextbit ( void )
{
    ++bitno;

    if ( reading )
    {
        pd4990a_readbit();
    }

    if ( reading && bitno == 0x34 )
    {
        reading = 0;
        pd4990a_resetbitstream();
    }

}
/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
* \return Test bit.
*/
/* ******************************************************************************************************************/
static Uint8 pd4990a_getcommand ( void )
{
    /*Warning: problems if the 4 bits are in different */
    /*parts, It's very strange that this case could happen. */
    if ( bitno <= 31 )
    {
        return shiftlo >> ( bitno - 4 );
    }

    else
    {
        return shifthi >> ( bitno - 32 - 4 );
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Updates date.
*
*/
/* ******************************************************************************************************************/
static void pd4990a_update_date ( void )
{
    pd4990a.seconds = ( shiftlo >> 0 ) & 0xff;
    pd4990a.minutes = ( shiftlo >> 8 ) & 0xff;
    pd4990a.hours  = ( shiftlo >> 16 ) & 0xff;
    pd4990a.days   = ( shiftlo >> 24 ) & 0xff;
    pd4990a.weekday = ( shifthi >> 0 ) & 0x0f;
    pd4990a.month  = ( shifthi >> 4 ) & 0x0f;
    pd4990a.year   = ( shifthi >> 8 ) & 0xff;
}
/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
*/
/* ******************************************************************************************************************/
static void pd4990a_process_command ( void )
{
    switch ( pd4990a_getcommand() )
    {
    /*load output register */
    case ( 0x1 ) :
        {
            bitno = 0;

            if ( reading )
            {
                /*prepare first bit */
                pd4990a_readbit();
            }

            shiftlo = 0;
            shifthi = 0;
        }
        break;

    case ( 0x2 ) :
        {
            /*store register to current date */
            writing = 0;
            pd4990a_update_date();
        }
        break;

    /*start reading */
    case ( 0x3 ) :
        {
            reading = 1;
        }
        break;

    /*switch testbit every frame */
    case ( 0x7 ) :
        {
            maxwaits = 1;
        }
        break;

    /*switch testbit every half-second */
    case ( 0x8 ) :
        {
            maxwaits = 30;
        }
        break;
    }
}
/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
* \param data Todo.
*/
/* ******************************************************************************************************************/
static void pd4990a_serial_control ( Uint8 data )
{
    static Sint32 clock_line = 0;
    static Sint32 command_line = 0;    /* ?? */

    /*Check for command end */
    if ( command_line && ! ( data & END_BIT ) ) /*end of command */
    {
        pd4990a_process_command();
        pd4990a_resetbitstream();
    }

    command_line = data & END_BIT;

    if ( clock_line && ! ( data & CLOCK_BIT ) ) /*clock lower edge */
    {
        pd4990a_writebit ( data & DATA_BIT );
        pd4990a_nextbit();
    }

    clock_line = data & CLOCK_BIT;
}
/* ******************************************************************************************************************/
/*!
* \brief Todo.
*
* \param address Todo.
* \param data Todo.
*/
/* ******************************************************************************************************************/
void write_4990_control_w ( Uint32 address, Uint32 data )
{
    pd4990a_serial_control ( data & 0x7 );
}

#ifdef _GNGEOX_PD4990A_C_
#undef _GNGEOX_PD4990A_C_
#endif // _GNGEOX_PD4990A_C_
