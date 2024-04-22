/******************************************************************************
 * qLibc
 *
 * Copyright (c) 2010-2015 Seungyoung Kim.
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
 * @file qsystem.c System APIs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#ifdef __linux__
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#else
#ifdef __MINGW32__
#include <Winsock2.h>
#endif // __MINGW32__
#endif // _linux_
#include "qinternal.h"
#include "utilities/qfile.h"
#include "utilities/qsystem.h"

/**
 * Get system environment variable
 *
 * @param envname   environment name
 * @param defstr    if not found, return this string
 *
 * @return a pointer of environment variable
 */
const char * qsys_getenv ( const char * envname, const char * defstr )
{
    const char * envstr = getenv ( envname );
    return ( envstr ) ? envstr : defstr;
}

/**
 * Get the result string of external command execution
 *
 * @param cmd       external command
 *
 * @return malloced string pointer which contains result if successful,
 *         otherwise returns NULL
 *
 * @note
 *  If the command does not report result but it is executed successfully,
 *  this will returns empty string(not null)
 */
char * qsys_cmd ( const char * cmd )
{
    FILE * fp = popen ( cmd, "r" );
    if ( fp == NULL )
    {
        return NULL;
    }

    char * str = qfile_read ( fp, NULL );
    pclose ( fp );

    if ( str == NULL )
    {
        str = strdup ( "" );
    }

    return str;
}

/**
 * Get the operating system name.
 *
 * @return Name of this implementation of the operating system.
 *
 */
char * qsys_info_osname ( void )
{
    static struct utsname buf1;
    if ( uname ( &buf1 ) != 0 )
    {
        return NULL;
    }

    return buf1.sysname;
}

/**
 * Get the system node.
 *
 * @return Name of this node within an implementation-defined communications network.
 *
 */
char * qsys_info_node ( void )
{
    static struct utsname buf1;
    if ( uname ( &buf1 ) != 0 )
    {
        return NULL;
    }

    return buf1.nodename;
}

/**
 * Get the system version.
 *
 * @return Current version level of this operating system release.
 *
 */
char * qsys_info_version ( void )
{
    static struct utsname buf1;
    if ( uname ( &buf1 ) != 0 )
    {
        return NULL;
    }

    return buf1.version;
}

/**
 * Get the system release.
 *
 * @return Current version level of this release.
 *
 */
char * qsys_info_release ( void )
{
    static struct utsname buf1;
    if ( uname ( &buf1 ) != 0 )
    {
        return NULL;
    }

    return buf1.release;
}

/**
 * Get the system machine.
 *
 * @return Name of the hardware type on which the system is running.
 *
 */
char * qsys_info_machine ( void )
{
    static struct utsname buf1;
    if ( uname ( &buf1 ) != 0 )
    {
        return NULL;
    }

    return buf1.machine;
}

/**
 * Clears terminal screen.
 *
 * @return Name of the hardware type on which the system is running.
 *
 */
void qsys_clrscr ( void )
{
    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
    #else
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #endif
    #endif
}
