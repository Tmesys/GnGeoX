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
 * qlibc header file.
 *
 * @file qlibc.h
 */

#ifndef QLIBC_H
#define QLIBC_H

/* _X_=target variable, _Y_=bit number to act upon 0-n */
#define QBIT_SET( _X_, _Y_ ) \
    ({                      \
        const typeof(_X_) __C_ = 1U;  \
        const typeof(_X_) __Y_ = (__C_<<(_Y_));  \
        (_X_) |= __Y_;   \
    })

#define QBIT_CLEAR( _X_, _Y_ ) \
    ({                      \
        const typeof(_X_) __C_ = 1U;  \
        const typeof(_X_) __Y_ = ~(__C_<<(_Y_));  \
        (_X_) &= __Y_;   \
    })

#define QBIT_FLIP( _X_, _Y_ ) \
    ({                      \
        const typeof(_X_) __C_ = 1U;  \
        const typeof(_X_) __Y_ = (__C_<<(_Y_));  \
        (_X_) ^= __Y_;   \
    })

#define QBIT_CHECK( _X_, _Y_ ) \
    ({                      \
        const typeof(_X_) __C_ = 1U;  \
        const typeof(_X_) __Y_ = (__C_<<(_Y_));  \
        (_X_) & __Y_;   \
    })

#define QBIT_POS( _X_ )      ( 1U << (_X_) )
#define QBIT_RANGE( _X_, _Y_ ) ( ( BIT_POS((_Y_ + 1) - _X_) - 1 ) << _X_ )

/* _X_=source variable, _Y_=bit start 0-n, , _Z_=bit length 1-n */
#define QBIT_RANGE_EXTRACT( _X_, _Y_, _Z_ ) \
    ({                      \
        const typeof(_X_) __C_ = 1U;  \
        const typeof(_X_) __Y_ = (_Y_);  \
        const typeof(_X_) __Z_ = (__C_<<(_Z_));  \
        (_X_ >> __Y_) & (__Z_ - 1);   \
    })

#define QBIT_RANGE_SET( _X_, _W_, _Y_, _Z_ ) \
    _X_ = ( ( _X_ & ( ~ ( BIT_RANGE ( _Y_, ( _Y_ + ( _Z_ - 1 ) ) ) ) ) ) | ( _W_ << _Y_ ) );

/* Checks if a variable is a power of 2 */
#define QBIT_POW2_CHECK( _X_ ) (_X_ && !(_X_ & (_X_ - 1)))

/* _X_=target variable, _Y_=mask */
#define QBITMASK_SET( _X_, _Y_ ) \
    ({                      \
        const typeof(_X_) __Y_ = (_Y_);  \
        (_X_) |= __Y_;   \
    })

#define QBITMASK_CLEAR( _X_, _Y_ ) \
    ({                      \
        const typeof(_X_) __Y_ = (_Y_);  \
        (_X_) &= ~__Y_;   \
    })

#define QBITMASK_FLIP( _X_, _Y_ ) \
    ({                      \
        const typeof(_X_) __Y_ = (_Y_);  \
        (_X_) ^= __Y_;   \
    })

#define QBITMASK_CHECK( _X_, _Y_ ) \
    ({                      \
        const typeof(_X_) __Y_ = (_Y_);  \
        (_X_) & __Y_;   \
    })

/* Swap integer values with no need to extra variable */
#define QSWAPVAL( _X_, _Y_ ) (((_X_) == (_Y_)) || (((_X_) ^= (_Y_)), ((_Y_) ^= (_X_)), ((_X_) ^= (_Y_))))

/* MIN */
#ifndef MIN
#define MIN( _X_, _Y_ ) \
    ({                      \
        const typeof(_X_) __X_ = (_X_);  \
        const typeof(_Y_) __Y_ = (_Y_);  \
        (void) (&__X_ == &__Y_); \
        (__X_ < __Y_ ? __X_ : __Y_);   \
    })
#endif // MIN

/* MAX */
#ifndef MAX
#define MAX( _X_, _Y_ ) \
    ({                       \
        const typeof(_X_) __X_ = (_X_);   \
        const typeof(_Y_) __Y_ = (_Y_);   \
        (void) (&__X_ == &__Y_);  \
        (__X_ > __Y_ ? __X_ : __Y_);    \
    })
#endif // MAX

/* Clamp value _X_ by _MIN_ and _MAX_ */
#define QCLAMP(_X_,_MIN_,_MAX_) \
    ({ \
        const typeof (_X_) __X_ = (_X_); \
        const typeof (_MIN_) __MIN_ = (_MIN_); \
        const typeof (_MAX_) __MAX_ = (_MAX_); \
        (__X_ > __MAX_ ? __MAX_ : (__X_ < __MIN_ ? __MIN_ : __X_)); \
    })

#define QCLAMP_16(_X_) (s16) CLAMP(_X_,INT16_MIN,INT16_MAX)
#define QCLAMP_32(_X_) (s32) CLAMP(_X_,INT32_MIN,INT32_MAX)

/* Prevents zero division */
#define QDIVZERO_GUARD(_X_) ((_X_) == 0 ? 1 : (_X_))

/* Checks if number is odd */
#define QCHECK_ODD(_X_) ((_X_) & 1)

/* Checks if number is even */
#define QCHECK_EVEN(_X_) (!((__X__) & 1))

/* Cheks if a given number is in the range */
#define QCHECK_RANGE_INCLUSIVE(_X_, _MIN_, _MAX_) (((_X_) - (_MIN_)) * ( (_MAX_) - (_X_) ) >= 0 ? TRUE : FALSE)
#define QCHECK_RANGE_EXCLUSIVE(_X_, _MIN_, _MAX_) (((_X_) - (_MIN_)) * ( (_MAX_) - (_X_) ) > 0 ? TRUE : FALSE)

#ifdef QLOWORD
#undef QLOWORD
#endif // QLOWORD
#define QLOWORD(_X_)   ( (uint16_t) ( ( (uint32_t) (_X_) ) & 0xFFFF ) )

#ifdef QLOWORD_SIGNED
#undef QLOWORD_SIGNED
#endif // QLOWORD_SIGNED
#define QLOWORD_SIGNED(_X_)   ( (sint16_t) ( ( (sint32_t) (_X_) ) & 0xFFFF ) )

#ifdef QHIWORD
#undef QHIWORD
#endif // QHIWORD
#define QHIWORD(_X_)   ( (uint16_t) ( ( ( (uint32_t) (_X_) ) >> 16 ) & 0xFFFF ) )

#ifdef QHIWORD_SIGNED
#undef QHIWORD_SIGNED
#endif // QHIWORD_SIGNED
#define QHIWORD_SIGNED(_X_)   ( (sint16_t) ( ( (sint32_t) (_X_) ) >> 16 ) )

#ifdef QLOBYTE
#undef QLOBYTE
#endif // QLOBYTE
#define QLOBYTE(_X_)   ( (uint8_t)  ( ( (uint16_t) (_X_) ) & 0xFF ) )

#ifdef QHIBYTE
#undef QHIBYTE
#endif // QHIBYTE
#define QHIBYTE(_X_)   ( (uint8_t)  ( ( ( (uint16_t) (_X_) ) >> 8 ) & 0xFF ) )

#ifdef QLONIBBLE
#undef QLONIBBLE
#endif // QLONIBBLE
#define QLONIBBLE(_X_) ( (uint8_t)  ( ( (uint8_t) (_X_) ) & 0xF ) )

#ifdef QHINIBBLE
#undef QHINIBBLE
#endif // QHINIBBLE
#define QHINIBBLE(_X_) ( (uint8_t)  ( ( ( (uint8_t) (_X_) ) >> 4 ) & 0xF ) )

#ifdef QMAKEWORD16
#undef QMAKEWORD16
#endif // QMAKEWORD16
#define QMAKEWORD16(LOW,HIGH) ( (uint16_t) ( ( (uint8_t) (LOW) ) | ( ( (uint16_t) ( (uint8_t) (HIGH) ) ) << 8 ) ) )

#ifdef QMAKEWORD32
#undef QMAKEWORD32
#endif // QMAKEWORD32
#define QMAKEWORD32(LOW,HIGH) ( (uint32_t) ( ( (uint16_t) (LOW) ) | ( ( (uint32_t) ( (uint16_t) (HIGH) ) ) << 16 ) ) )

#ifdef QWORD32ALIGNED
#undef QWORD32ALIGNED
#endif // QWORD32ALIGNED
#define QWORD32ALIGNED(WORD) !( WORD & 0x3 )

/* containers */
#include "containers/qtreetbl.h"
#include "containers/qhashtbl.h"
#include "containers/qhasharr.h"
#include "containers/qlisttbl.h"
#include "containers/qlist.h"
#include "containers/qvector.h"
#include "containers/qqueue.h"
#include "containers/qstack.h"
#include "containers/qgrow.h"

/* extensions */
#include "extensions/qaconf.h"
#include "extensions/qconfig.h"
#include "extensions/qdatabase.h"
//#include "extensions/qhttpclient.h"
#include "extensions/qlog.h"
#include "extensions/qtokenbucket.h"

/* utilities */
#include "utilities/qcount.h"
#include "utilities/qencode.h"
#include "utilities/qfile.h"
#include "utilities/qhash.h"
#include "utilities/qio.h"
#ifndef _WIN32
#include "utilities/qsocket.h"
#endif // _WIN32
#include "utilities/qstring.h"
#include "utilities/qsystem.h"
#include "utilities/qtime.h"
#include "utilities/qsort.h"
#include "utilities/qcdrom.h"
#include "utilities/qfastmem.h"
#include "utilities/qoptfetch.h"
#include "utilities/qalloc.h"
#include "utilities/qzip.h"

/* ipc */
#include "ipc/qsem.h"
#include "ipc/qshm.h"

#endif /* QLIBC_H */

