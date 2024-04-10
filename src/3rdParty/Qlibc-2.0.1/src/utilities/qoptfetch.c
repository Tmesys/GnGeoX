/******************************************************************************
 * qLibc
 *
 * Copyright (c) 2010-2015 Seungyoung Kim.
 * Copyright (c) 2017 Elijah Stone (OptFetch).
 * Copyright (c) 2023 Mourad Reggadi.
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
#include <stdio.h>
#include <string.h>

#include "utilities/qoptfetch.h"

/**
 * @file qoptfetch.c Sorting APIs.
 */

#ifndef _DOXYGEN_SKIP

static int countopts ( const struct opttype *opts )
{
    unsigned int i = 0;

    for ( i = 0;
            /* longname and short name are both 0, OR */
            ! ( ( ( opts[i].longname == NULL ) && ( opts[i].shortname == '\0' ) ) ||
                /* output type was unspecified OR */
                ( ( opts[i].type == 0 ) || ( opts[i].type > OPTTYPE_STRING ) ) ||
                /* nowhere to output data */
                ( opts[i].outdata == NULL ) );

            i++ );

    return i;
}

static int get_option_index_short ( char opt, const struct opttype *potentialopts, unsigned int len )
{
    unsigned int i = 0;

    for ( i = 0; i < len; i++ )
    {
        if ( !potentialopts[i].shortname )
        {
            continue;
        }
        else if ( potentialopts[i].shortname == opt )
        {
            return i;
        }
    }
    return -1;
}

static int get_option_index_long ( const char *opt, const struct opttype *potentialopts, unsigned int len )
{
    unsigned int i = 0;

    for ( i = 0; i < len; i++ )
    {
        if ( !potentialopts[i].longname )
        {
            continue;
        }
        else if ( !strcmp ( potentialopts[i].longname, opt ) )
        {
            return i;
        }
    }
    return -1;
}

#endif /* _DOXYGEN_SKIP */

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 * @code
 * int main(int argc, char **argv) {
 * 	bool debug = false;
 * 	char *name = NULL;
 * 	float boat = 0.0;
 *
 * 	struct opttype opts[] = {
 * 		{"debug",	'd',	OPTTYPE_BOOL,	&debug},
 * 		{"name",	'n',	OPTTYPE_STRING,	&name},
 * 		{"boat",	'b',	OPTTYPE_FLOAT,	&boat},
 * 	{0}};
 *
 * 	fetchopts(&argc, &argv, opts);
 *
 * 	if (debug) {
 * 		printf("Did debug.\n");
 * 	}
 *
 * 	printf("Hi.  My name is %s.  What's yours?\n", name);
 *
 * 	printf("My boat is %f feet long.  How about yours?\n", boat);
 *
 * 	printf("Looks like I have %d argument%s left over.  Fancy that now!\n", argc, (argc == 1) ? "" : "s");
 *
 * 	if (argc) {
 * 		printf("They are:\n");
 * 		for (int i = 1; i <= argc; i++) {
 * 			printf("* %s\n", argv[i]);
 * 		}
 * 	}
 *
 * 	return 0;
 * }
 * @endcode
 *
 */
void qoptfetch ( int *argc, char ***argv, struct opttype *opts )
{
    unsigned int argindex = 0;
    unsigned int numopts = countopts ( opts );
    int newargc = 1;
    struct opttype *wasinarg = NULL;
    char *curropt = NULL;

    /* start at 1 because index 0 is the executable name */
    for ( argindex = 1; argindex < *argc; argindex++ )
    {
        if ( ( curropt = ( *argv ) [argindex] ) == NULL ) continue;

        /* Last argument was an option, now we're setting the actual value of that option */
        if ( wasinarg != NULL )
        {
            char *format_specifier;

            switch ( wasinarg->type )
            {
            /* We set the format specifier here then make
             * one sscanf call with it.  We don't even need
             * to cast it because it's already a pointer
             * unless the user fucked something up which is
             * their fault!
             */
            case OPTTYPE_CHAR:
                format_specifier = "%c";
                break;
            case OPTTYPE_SHORT:
                format_specifier = "%hi";
                break;
            case OPTTYPE_USHORT:
                format_specifier = "%hu";
                break;
            case OPTTYPE_INT:
                format_specifier = "%d";
                break;
            case OPTTYPE_UINT:
                format_specifier = "%u";
                break;
            case OPTTYPE_LONG:
                format_specifier = "%ld";
                break;
            case OPTTYPE_ULONG:
                format_specifier = "%lu";
                break;
#ifdef _WIN32
            case OPTTYPE_LONGLONG:
                format_specifier = "%l64d";
                break;
            case OPTTYPE_ULONGLONG:
                format_specifier = "%l64u";
                break;
#else
            case OPTTYPE_LONGLONG:
                format_specifier = "%lld";
                break;
            case OPTTYPE_ULONGLONG:
                format_specifier = "%llu";
                break;
#endif
            case OPTTYPE_FLOAT:
                format_specifier = "%f";
                break;
            case OPTTYPE_DOUBLE:
                format_specifier = "%lf";
                break;
            case OPTTYPE_LONGDOUBLE:
                format_specifier = "%Lf";
                break;
            case OPTTYPE_STRING:
                * ( char** ) ( wasinarg->outdata ) = curropt;
                wasinarg = NULL;
                format_specifier = NULL;
                continue;
                /* OPTTYPE_BOOL already handled */
            }
            sscanf ( curropt, format_specifier, wasinarg->outdata );
            wasinarg = NULL;
            format_specifier = NULL;
        }
        else
        {
            /* Has the user manually demanded that the option-parsing end now? */
            if ( !strcmp ( curropt, "--" ) )
            {
                argindex++;

                goto end;
            }

            /* in an option, getting warmer */
            if ( curropt[0] == '-' )
            {
                int option_index = -1;
                unsigned char oneoffset;

                /* was it a --foo or just a -foo? */
                if ( curropt[1] == '-' )
                {
                    oneoffset = 2;
                }
                else
                {
                    oneoffset = 1;
                }

                /* is it a short opt (e.g. -f) or a long one (e.g. -foo)? */
                if ( strlen ( curropt + oneoffset ) == 1 )
                {
                    option_index = get_option_index_short ( curropt[oneoffset], opts, numopts );
                }

                /* long opt OR nothing matched for short opt ('f' is a valid longname) */
                if ( strlen ( curropt + oneoffset ) > 1 || option_index == -1 )
                {
                    option_index = get_option_index_long ( curropt + oneoffset, opts, numopts );
                }

                /* not an option */
                if ( option_index == -1 )
                {
                    ( *argv ) [newargc++] = curropt;
                    continue;
                }
                else
                {
                    /* it's a boolean option, so the next loop doesn't want to know about it */
                    if ( ( opts[option_index] ).type == OPTTYPE_BOOL )
                    {
#if __STDC_VERSION__ >= 199901L
                        * ( _Bool* ) opts[option_index].outdata = 1;
#else
                        * ( int* ) opts[option_index].outdata = 1;
#endif
                        /* let the next loop iteration get the value */
                    }
                    else
                    {
                        wasinarg = &opts[option_index];
                    }
                }
            }
            else
            {
                ( *argv ) [newargc++] = curropt;
            }
        }
    }

end:
    {
        int i;

        for ( i = argindex; i < *argc; i++ )
        {
            ( *argv ) [newargc++] = ( *argv ) [i];
        }
    }

    *argc = newargc;
}
