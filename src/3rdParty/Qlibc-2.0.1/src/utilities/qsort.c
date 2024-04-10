/******************************************************************************
 * qLibc
 *
 * Copyright (c) 2010-2015 Seungyoung Kim.
 * Copyright (c) 2020 Adem Budak (libsort).
 * Copyright (c) 2020 Mourad Reggadi.
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
 * @file qsort.c Sorting APIs.
 */

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "utilities/qsort.h"

static void swap ( char * x, char * y, size_t size )
{
    while ( size-- > 0 )
    {
        char tmp = *x;
        *x++ = *y;
        *y++ = tmp;
    }
}

/**
 * Sort object array using bubble algo.
 *
 * @param base  pointer on data array to sort
 * @param num   number of elements in data array
 * @param size  size in bytes of each element
 * @param cmp   comparing routine to sort
 *
 */
void qbubble_sort ( void * base, size_t num, size_t size, int ( *cmp ) ( const void *, const void * ) )
{
    char * pbBase = ( char * ) base;
    bool sorted = true;
    size_t i, j;

    num -= 1;

    if ( num )
    {
        for ( i = 0; i < num; i++ )
        {
            sorted = true;
            for ( j = 0; j < num - i; j++ )
            {
                if ( cmp ( pbBase + ( j + 1 ) * size, pbBase + j * size ) < 0 )
                {
                    swap ( pbBase + ( j + 1 ) * size, pbBase + j * size, size );
                    sorted = false;
                }
            }
            if ( sorted ) break;
        }
    }
}

/**
 * Sort object array using cocktail shaker algo.
 *
 * @param base  pointer on data array to sort
 * @param num   number of elements in data array
 * @param size  size in bytes of each element
 * @param cmp   comparing routine to sort
 *
 */
void qcocktailshaker_sort ( void * base, size_t num, size_t size, int ( *cmp ) ( const void *, const void * ) )
{
    char * pbBase = ( char * ) base;
    size_t i;
    int swapped;

    if ( num )
    {
        do
        {
            swapped = false;
            for ( i = 0; i < num - 2; i++ )
                if ( cmp ( pbBase + ( i + 1 ) * size, pbBase + i * size ) < 0 )
                {
                    swap ( pbBase + ( i + 1 ) * size, pbBase + i * size, size );
                    swapped = true;
                }

            if ( !swapped ) break;

            swapped = false;
            for ( i = num - 2; i > 0; i-- )
                if ( cmp ( pbBase + ( i + 1 ) * size, pbBase + i * size ) < 0 )
                {
                    swap ( pbBase + ( i + 1 ) * size, pbBase + i * size, size );
                    swapped = true;
                }
        }
        while ( swapped );
    }
}

/**
 * Sort object array using comb algo.
 *
 * @param base  pointer on data array to sort
 * @param num   number of elements in data array
 * @param size  size in bytes of each element
 * @param cmp   comparing routine to sort
 *
 */
void qcomb_sort ( void * base, size_t num, size_t size, int ( *cmp ) ( const void *, const void * ) )
{
    char * pbBase = ( char * ) base;
    size_t i, gap;
    int swapped;
    const float shrink = 1.3f;

    gap = num;
    swapped = false;

    while ( gap > 1 || swapped )
    {
        if ( gap > 1 ) gap /= shrink;

        swapped = false;

        for ( i = 0; i + gap < num; i++ )
        {
            if ( cmp ( pbBase + i * size, pbBase + ( i + gap ) * size ) < 0 )
            {
                swap ( pbBase + i * size, pbBase + ( i + gap ) * size, size );
                swapped = true;
            }
            if ( cmp ( pbBase + i * size, pbBase + ( i + gap ) * size ) == 0 ) swapped = true;
        }
    }
}

/**
 * Sort object array using gnome algo.
 *
 * @param base  pointer on data array to sort
 * @param num   number of elements in data array
 * @param size  size in bytes of each element
 * @param cmp   comparing routine to sort
 *
 */
void qgnome_sort ( void * base, size_t num, size_t size, int ( *cmp ) ( const void *, const void * ) )
{
    char * pbBase = ( char * ) base;
    size_t i = 0;

    while ( i < num )
        if ( i == 0 || cmp ( pbBase + ( i - 1 ) * size, pbBase + i * size ) < 0 )
            i++;
        else
        {
            swap ( pbBase + i * size, pbBase + ( i - 1 ) * size, size );
            i--;
        }
}

/**
 * Sort object array using heap algo.
 *
 * @param base  pointer on data array to sort
 * @param num   number of elements in data array
 * @param size  size in bytes of each element
 * @param cmp   comparing routine to sort
 *
 */
void qheap_sort ( void * base, size_t num, size_t size, int ( *cmp ) ( const void *, const void * ) )
{
    char * pbBase = ( char * ) base;
    int i = ( num / 2 - 1 ) * size;
    int n = num * size;
    int c, r;

    while ( i >= 0 )
    {
        for ( r = i; r * 2 + size < n; r = c )
        {
            c = r * 2 + size;
            if ( c < n - size && cmp ( pbBase + c, pbBase + c + size ) < 0 ) c += size;
            if ( cmp ( pbBase + r, pbBase + c ) >= 0 ) break;
            swap ( pbBase + r, pbBase + c, size );
        }
        i -= size;
    }

    for ( i = n - size; i > 0; i -= size )
    {
        swap ( pbBase, pbBase + i, size );

        for ( r = 0; r * 2 + size < i; r = c )
        {
            c = r * 2 + size;
            if ( c < i - size && cmp ( pbBase + c, pbBase + c + size ) < 0 ) c += size;
            if ( cmp ( pbBase + r, pbBase + c ) >= 0 ) break;
            swap ( pbBase + r, pbBase + c, size );
        }
    }
}

/**
 * Sort object array using insertion algo.
 *
 * @param base  pointer on data array to sort
 * @param num   number of elements in data array
 * @param size  size in bytes of each element
 * @param cmp   comparing routine to sort
 *
 */
void qinsertion_sort ( void * base, size_t num, size_t size, int ( *cmp ) ( const void *, const void * ) )
{
    char * pbBase = ( char * ) base;
    size_t i, j;

    if ( num )
        for ( i = 1; i < num; i++ )
            for ( j = i; j > 0 && cmp ( pbBase + j * size, pbBase + ( j - 1 ) * size ) < 0; j-- )
                swap ( pbBase + j * size, pbBase + ( j - 1 ) * size, size );
}

/**
 * Sort object array using 3 way algo.
 *
 * @param base  pointer on data array to sort
 * @param num   number of elements in data array
 * @param size  size in bytes of each element
 * @param cmp   comparing routine to sort
 *
 */
void q3way_sort ( void * base, size_t n, size_t size, int ( *cmp ) ( const void *, const void * ) )
{
    char * ptr = ( char * ) base;

    while ( n > 1 )
    {
        int i = 1, lt = 0, gt = n;
        while ( i < gt )
        {
            int c = cmp ( ptr + lt * size, ptr + i * size );
            if ( c > 0 )
            {
                swap ( ptr + lt * size, ptr + i * size, size );
                lt++;
                i++;
            }
            else if ( c < 0 )
            {
                gt--;
                swap ( ptr + i * size, ptr + gt * size, size );
            }
            else
            {
                i++;
            }
        }

        if ( lt < n - gt )
        {
            q3way_sort ( ptr, lt, size, cmp );
            ptr += gt * size;
            n -= gt;
        }
        else
        {
            q3way_sort ( ptr + gt * size, n - gt, size, cmp );
            n = lt;
        }
    }
}

/**
 * Sort object array using selection algo.
 *
 * @param base  pointer on data array to sort
 * @param num   number of elements in data array
 * @param size  size in bytes of each element
 * @param cmp   comparing routine to sort
 *
 */
void qselection_sort ( void * base, size_t num, size_t size, int ( *cmp ) ( const void *, const void * ) )
{
    char * pvBase = ( char * ) base;
    size_t i, j;
    size_t m;

    if ( num )
    {
        for ( i = 0; i < num; i++ )
        {
            m = i;

            for ( j = i + 1; j < num; j++ )
                if ( cmp ( pvBase + j * size, pvBase + m * size ) < 0 ) m = j;

            swap ( pvBase + i * size, pvBase + m * size, size );
        }
    }
}

/**
 * Sort object array using shell algo.
 *
 * @param base  pointer on data array to sort
 * @param num   number of elements in data array
 * @param size  size in bytes of each element
 * @param cmp   comparing routine to sort
 *
 */
void qshell_sort ( void * base, size_t num, size_t size, int ( *cmp ) ( const void *, const void * ) )
{
    char * pbBase = ( char * ) base;
    size_t i, j;
    size_t h = 1;

    if ( num )
    {
        while ( h < num / 3 )
            h = 3 * h + 1;

        while ( h >= 1 )
        {
            for ( i = 1; i < num; i++ )
                for ( j = i; j > 0 && cmp ( pbBase + j * size, pbBase + ( j - 1 ) * size ) < 0;
                        j-- )
                    swap ( pbBase + j * size, pbBase + ( j - 1 ) * size, size );
            h /= 3;
        }
    }
}
