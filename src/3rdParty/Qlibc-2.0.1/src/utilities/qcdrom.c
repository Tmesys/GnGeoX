/******************************************************************************
 * qLibc
 *
 * Copyright (c) 2010-2015 Seungyoung Kim.
 * Copyright (c) 2021 Mourad Reggadi.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

/**
 * @file qcdrom.c CdRom utilities APIs.
 */

#include "utilities/qcdrom.h"

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
int qcdrom_itob ( int value )
{
    return ( ( value ) / 10 * 16 + ( value ) % 10 );
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
int qcdrom_btoi ( int value )
{
    return ( ( value ) / 16 * 10 + ( value ) % 16 );
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
int qcdrom_msf2sec ( char * msf )
{
    return ( ( ( msf[0] * 60 ) + msf[1] ) * 75 + msf[2] );
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
void qcdrom_sec2msf ( int sector, char * msf )
{
    msf[0] = sector / 75 / 60;
    sector = sector - msf[0] * 75 * 60;
    msf[1] = sector / 75;
    sector = sector - msf[1] * 75;
    msf[2] = sector;
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
void qcdrom_addr2time ( int sector, char * msf )
{
    msf[2] = ( char ) ( sector % 75 );
    sector /= 75;
    msf[1] = ( char ) ( sector % 60 );
    msf[0] = ( char ) ( sector / 60 );
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
void qcdrom_addr2timeb ( int sector, char * msf )
{
    msf[2] = qcdrom_itob ( ( char ) ( sector % 75 ) );
    sector /= 75;
    msf[1] = qcdrom_itob ( ( char ) ( sector % 60 ) );
    msf[0] = qcdrom_itob ( ( char ) ( sector / 60 ) );
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
int qcdrom_time2addr ( char * msf )
{
    int addr = 0;

    addr = msf[0] * 60;
    addr = ( addr + msf[1] ) * 75;
    addr += msf[2];

    return ( addr );
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
int qcdrom_time2addrb ( char * msf )
{
    int addr = 0;

    addr = qcdrom_btoi ( msf[0] ) * 60;
    addr = ( addr + qcdrom_btoi ( msf[1] ) ) * 75;
    addr += qcdrom_btoi ( msf[2] );

    return ( addr );
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
void qcdrom_block2time ( char * block, char * msf )
{
    int m = 0, s = 0, d = 0;
    int bblock = * ( ( int * ) block );

    bblock += 150;
    m = bblock / 4500;               // minutes
    bblock = bblock - m * 4500;       // minutes rest
    s = bblock / 75;                 // seconds
    d = bblock - s * 75;             // seconds rest

    m = ( ( m / 10 ) << 4 ) | m % 10;
    s = ( ( s / 10 ) << 4 ) | s % 10;
    d = ( ( d / 10 ) << 4 ) | d % 10;

    msf[0] = ( char ) qcdrom_btoi ( m );
    msf[1] = ( char ) qcdrom_btoi ( s );
    msf[2] = ( char ) qcdrom_btoi ( d );
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
int qcdrom_block2addr ( char * block )
{
    char msf[3] = {0, 0, 0};

    qcdrom_block2time ( block, msf );

    return ( qcdrom_time2addr ( msf ) );
}
