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
 * @file qalloc.c Memory allocation APIs.
 */
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "utilities/qalloc.h"
#include "containers/qlisttbl.h"
#include "utilities/qstring.h"

static qlisttbl_t * tbl = NULL;

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
bool qalloc_init ( void )
{
    tbl = qlisttbl ( QLISTTBL_THREADSAFE );
    if ( tbl == NULL )
    {
        errno = ENOMEM;
        return ( false );
    }

    return ( true );
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
void * qalloc_new ( unsigned int bytesize, char * filename, unsigned int line )
{
    char * memory = NULL;
    char key[255];
    qalloc_obj_t mnode;

    if ( bytesize == 0 )
    {
        errno = EINVAL;
        return ( NULL );
    }

    if ( filename == NULL )
    {
        errno = EINVAL;
        return ( NULL );
    }

    if ( line == 0 )
    {
        errno = EINVAL;
        return ( NULL );
    }

    /* Allocate two extra bytes as overflow guards */
    memory = ( char * ) calloc ( 1, ( bytesize + 2 ) );
    if ( memory == NULL )
    {
        errno = ENOMEM;
        return ( NULL );
    }

    sprintf ( key, "%x", ( int ) ( memory + 1 ) );
    qstrupper ( key );

    memset ( &mnode, 0, sizeof ( qalloc_obj_t ) );
    mnode.startpointer = memory;
    mnode.size = bytesize;
    mnode.filename = filename;
    mnode.line = line;

    if ( qlisttbl_put ( tbl, key, &mnode, sizeof ( qalloc_obj_t ) ) == false )
    {
        free ( memory );
        return ( NULL );
    }

    return ( ( void * ) ( memory + 1 ) );
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
void * qalloc_add ( void * memory, unsigned int bytesize, char * filename, unsigned int line )
{
    char key[255];
    qalloc_obj_t mnode;

    if ( memory == NULL )
    {
        errno = EINVAL;
        return ( NULL );
    }

    if ( bytesize == 0 )
    {
        errno = EINVAL;
        return ( NULL );
    }

    if ( filename == NULL )
    {
        errno = EINVAL;
        return ( NULL );
    }

    if ( line == 0 )
    {
        errno = EINVAL;
        return ( NULL );
    }

    sprintf ( key, "%x", ( int ) ( memory ) );
    qstrupper ( key );

    memset ( &mnode, 0, sizeof ( qalloc_obj_t ) );
    mnode.startpointer = memory;
    mnode.size = bytesize;
    mnode.filename = filename;
    mnode.line = line;

    if ( qlisttbl_put ( tbl, key, &mnode, sizeof ( qalloc_obj_t ) ) == false )
    {
        free ( memory );
        return ( NULL );
    }

    return ( memory );
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
bool qalloc_delete ( void * memory )
{
    char key[255];
    qalloc_obj_t * mnode = NULL;
    char * cstartpointer = NULL;

    if ( memory == NULL )
    {
        errno = EINVAL;
        return ( false );
    }

    sprintf ( key, "%x", ( int ) memory );
    qstrupper ( key );

    mnode = qlisttbl_get ( tbl, key, NULL, false );
    if ( mnode == NULL )
    {
        errno = ENOKEY;
        return ( false );
    }

    if ( mnode->startpointer != NULL || mnode->size != 0 )
    {
        cstartpointer = ( char * ) mnode->startpointer;
        if ( cstartpointer[0] != 0 || cstartpointer[mnode->size + 1] != 0 )
        {
            fprintf ( stdout, "Memory corruption at address %s, file %s, line %d \n", key, mnode->filename, mnode->line );
        }

        free ( mnode->startpointer );
    }
    else
    {
        errno = EFAULT;
        return ( false );
    }

    if ( qlisttbl_remove ( tbl, key ) != 1 )
    {
        errno = EPERM;
        return ( false );
    }

    return ( true );
}

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
void qalloc_exit ( void )
{
    qlisttbl_obj_t * obj = NULL;
    qalloc_obj_t * mnode = NULL;
    char * cstartpointer = NULL;

    for ( obj = tbl->first; obj; obj = obj->next )
    {
        mnode = qlisttbl_get ( tbl, obj->name, NULL, false );
        if ( mnode == NULL )
        {
            fprintf ( stderr, "Unreachable address %s\n", obj->name );
            errno = ENOKEY;
        }

        fprintf ( stdout, "Memory leak at address %s, file %s, line %d \n", obj->name, mnode->filename, mnode->line );

        if ( mnode->startpointer != NULL || mnode->size != 0 )
        {
            cstartpointer = ( char * ) mnode->startpointer;
            if ( cstartpointer[0] != 0 || cstartpointer[mnode->size + 1] != 0 )
            {
                fprintf ( stdout, "Memory corruption at address %s, file %s, line %d \n", obj->name, mnode->filename, mnode->line );
            }

            free ( mnode->startpointer );
        }
        else
        {
            fprintf ( stderr, "Incoherent address %s\n", obj->name );
            errno = EFAULT;
        }
    }

    qlisttbl_free ( tbl );
}
