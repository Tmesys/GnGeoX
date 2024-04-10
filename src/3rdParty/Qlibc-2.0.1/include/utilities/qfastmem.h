/******************************************************************************
 * qLibc
 *
 * Copyright (c) 2010-2015 Seungyoung Kim.
 * Copyright (c) 2014 Trevor Herselman (qmemmove).
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
 * qcdrom header file.
 *
 * @file qmem.h
 */

#ifndef QMEM_H
#define QMEM_H

//	FORCE `CDECL` calling convention on 32-bit builds on our function pointers, because we need it to match the original `std::memmove` definition; in-case the user specified a different default function calling convention! (I specified __fastcall as my default calling convention and got errors! So I needed to add this!)
#if !defined(__x86_64__) && !defined(_M_X64) && (defined(__i386) || defined(_M_IX86)) && (defined(_MSC_VER) || defined(__GNUC__))
#if defined(_MSC_VER)
#define APEXCALL __cdecl						//	32-bit on Visual Studio
#else
#define APEXCALL __attribute__((__cdecl__))		//	32-bit on GCC / LLVM (Clang)
#endif
#else
#define APEXCALL									//	64-bit - __fastcall is default on 64-bit!
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void * ( APEXCALL *qmemcpy ) ( void *dst, const void *src, size_t size );
extern void * ( APEXCALL *qmemmove ) ( void *dst, const void *src, size_t size );

#ifdef __cplusplus
}
#endif

#endif /* QMEM_H */
