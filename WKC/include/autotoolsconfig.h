/*
 autotoolsconfig.h

 Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _AUTOTOOLSCONFIG_H_
#define _AUTOTOOLSCONFIG_H_

// remove pre-defined values
#ifdef __CYGWIN__
# undef __CYGWIN__
# define __int8_t_defined
# define __uint32_t_defined
# define __intptr_t_defined
typedef signed char int8_t;
typedef short int16_t;
typedef signed int int32_t;
typedef long long int64_t;
typedef unsigned int uint32_t;
typedef int intptr_t;
typedef unsigned long __uid32_t;
typedef unsigned long __gid32_t;
#ifndef TRUE
# define TRUE true
#endif
# define __BUILDING_IN_CYGWIN__
#endif // __CYGWIN__

#ifdef __CYGWIN32__
# undef __CYGWIN32__
#endif // __CYGWIN32__

#ifdef _MSC_VER
# define __BUILDING_IN_VS__
#endif // _MSC_VER


#include <wkc/wkcconfig.h>
#include "wkcplatform.h"
#include "wkcclibwrapper.h"

#endif // _AUTOTOOLSCONFIG_H_
