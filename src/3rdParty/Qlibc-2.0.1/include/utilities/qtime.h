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
 * qtime header file.
 *
 * @file qtime.h
 */

#ifndef QTIME_H
#define QTIME_H

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qtimer_s qtimer_t;

extern void qtime_timespec_diff ( struct timespec t1, struct timespec t2,
                                  struct timespec * diff );
extern void qtime_timeval_diff ( struct timeval t1, struct timeval t2,
                                 struct timeval * diff );

extern long qtime_current_milli ( void );

extern char * qtime_localtime_strf ( char * buf, int size, time_t utctime,
                                     const char * format );
extern char * qtime_localtime_str ( time_t utctime );
extern const char * qtime_localtime_staticstr ( time_t utctime );
extern char * qtime_gmt_strf ( char * buf, int size, time_t utctime,
                               const char * format );
extern char * qtime_gmt_str ( time_t utctime );
extern const char * qtime_gmt_staticstr ( time_t utctime );
extern time_t qtime_parse_gmtstr ( const char * gmtstr );
extern int qtime_sleep_milli ( const int delay, bool interrupt );
extern void qtime_start_timer ( qtimer_t * );
extern double qtime_end_timer ( qtimer_t * );

struct qtimer_s
{
    struct timeval starttime;
    struct timeval endtime;
};

#ifdef __cplusplus
}
#endif

#endif /* QTIME_H */
