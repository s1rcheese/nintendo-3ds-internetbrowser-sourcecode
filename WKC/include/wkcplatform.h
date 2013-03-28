/*
 * wkcplatform.h
 *
 * Copyright (C) 2006, 2007, 2008, 2009 Apple Inc.  All rights reserved.
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _WKCPLATFORM_H_
#define _WKCPLATFORM_H_

// prevent to include original wtf/Platform.h
#ifndef WTF_Platform_h
# define WTF_Platform_h
#endif // WTF_Platform_h

/* PLATFORM handles OS, operating environment, graphics API, and CPU */
#define PLATFORM(WTF_FEATURE) (defined WTF_PLATFORM_##WTF_FEATURE  && WTF_PLATFORM_##WTF_FEATURE)
#define COMPILER(WTF_FEATURE) (defined WTF_COMPILER_##WTF_FEATURE  && WTF_COMPILER_##WTF_FEATURE)
#define HAVE(WTF_FEATURE) (defined HAVE_##WTF_FEATURE  && HAVE_##WTF_FEATURE)
#define USE(WTF_FEATURE) (defined WTF_USE_##WTF_FEATURE  && WTF_USE_##WTF_FEATURE)
#define ENABLE(WTF_FEATURE) (defined ENABLE_##WTF_FEATURE  && ENABLE_##WTF_FEATURE)

/* Operating systems - low-level dependencies */

#define WTF_PLATFORM_WKC 1

/* Operating environments */

/* CPU */

/* PLATFORM(ARM) */
#define PLATFORM_ARM_ARCH(N) (PLATFORM(ARM) && ARM_ARCH_VERSION >= N)

#if   defined(arm) \
   || defined(__arm__)
#define WTF_PLATFORM_ARM 1

#if defined(__ARMEB__)
#define WTF_PLATFORM_BIG_ENDIAN 1

#elif !defined(__ARM_EABI__) \
   && !defined(__EABI__) \
   && !defined(__VFP_FP__) \
   && !defined(ANDROID) \
   && (!(defined(__ARMCC_VERSION) && defined(__TARGET_FPU_VFP)))
#define WTF_PLATFORM_MIDDLE_ENDIAN 1

#endif

/* Set ARM_ARCH_VERSION */
#if   defined(__ARM_ARCH_4__) \
   || defined(__ARM_ARCH_4T__) \
   || defined(__MARM_ARMV4__) \
   || defined(_ARMV4I_)
#define ARM_ARCH_VERSION 4

#elif defined(__ARM_ARCH_5__) \
   || defined(__ARM_ARCH_5T__) \
   || defined(__ARM_ARCH_5E__) \
   || defined(__ARM_ARCH_5TE__) \
   || defined(__ARM_ARCH_5TEJ__) \
   || defined(__MARM_ARMV5__)
#define ARM_ARCH_VERSION 5

#elif defined(__ARM_ARCH_6__) \
   || defined(__ARM_ARCH_6J__) \
   || defined(__ARM_ARCH_6K__) \
   || defined(__ARM_ARCH_6Z__) \
   || defined(__ARM_ARCH_6ZK__) \
   || defined(__ARM_ARCH_6T2__) \
   || defined(__ARMV6__)
#define ARM_ARCH_VERSION 6

#elif defined(__ARM_ARCH_7A__) \
   || defined(__ARM_ARCH_7R__)
#define ARM_ARCH_VERSION 7

/* RVCT sets _TARGET_ARCH_ARM */
#elif defined(__TARGET_ARCH_ARM)
#define ARM_ARCH_VERSION __TARGET_ARCH_ARM

#else
#define ARM_ARCH_VERSION 0

#endif

/* Set THUMB_ARM_VERSION */
#if   defined(__ARM_ARCH_4T__)
#define THUMB_ARCH_VERSION 1

#elif defined(__ARM_ARCH_5T__) \
   || defined(__ARM_ARCH_5TE__) \
   || defined(__ARM_ARCH_5TEJ__)
#define THUMB_ARCH_VERSION 2

#elif defined(__ARM_ARCH_6J__) \
   || defined(__ARM_ARCH_6K__) \
   || defined(__ARM_ARCH_6Z__) \
   || defined(__ARM_ARCH_6ZK__) \
   || defined(__ARM_ARCH_6M__)
#define THUMB_ARCH_VERSION 3

#elif defined(__ARM_ARCH_6T2__) \
   || defined(__ARM_ARCH_7__) \
   || defined(__ARM_ARCH_7A__) \
   || defined(__ARM_ARCH_7R__) \
   || defined(__ARM_ARCH_7M__)
#define THUMB_ARCH_VERSION 4

/* RVCT sets __TARGET_ARCH_THUMB */
#elif defined(__TARGET_ARCH_THUMB)
#define THUMB_ARCH_VERSION __TARGET_ARCH_THUMB

#else
#define THUMB_ARCH_VERSION 0
#endif

/* On ARMv5 and below the natural alignment is required. */
#if !defined(ARM_REQUIRE_NATURAL_ALIGNMENT) && ARM_ARCH_VERSION <= 5
#define ARM_REQUIRE_NATURAL_ALIGNMENT 1
#endif

/* Defines two pseudo-platforms for ARM and Thumb-2 instruction set. */
#if !defined(WTF_PLATFORM_ARM_TRADITIONAL) && !defined(WTF_PLATFORM_ARM_THUMB2)
#  if defined(thumb2) || defined(__thumb2__) \
  || ((defined(__thumb) || defined(__thumb__)) && THUMB_ARCH_VERSION == 4)
#    define WTF_PLATFORM_ARM_TRADITIONAL 0
#    define WTF_PLATFORM_ARM_THUMB2 1
#  elif PLATFORM_ARM_ARCH(4)
#    define WTF_PLATFORM_ARM_TRADITIONAL 1
#    define WTF_PLATFORM_ARM_THUMB2 0
#  else
#    error "Not supported ARM architecture"
#  endif
#elif PLATFORM(ARM_TRADITIONAL) && PLATFORM(ARM_THUMB2) /* Sanity Check */
#  error "Cannot use both of WTF_PLATFORM_ARM_TRADITIONAL and WTF_PLATFORM_ARM_THUMB2 platforms"
#endif // !defined(ARM_TRADITIONAL) && !defined(ARM_THUMB2)
#endif /* ARM */

/* PLATFORM(X86) */
#if   defined(__i386__) \
   || defined(i386)     \
   || defined(_M_IX86)  \
   || defined(_X86_)    \
   || defined(__THW_INTEL)
#define WTF_PLATFORM_X86 1
#endif

/* PLATFORM(X86_64) */
#if   defined(__x86_64__) \
   || defined(_M_X64)
#define WTF_PLATFORM_X86_64 1
#endif

/* Compiler */

/* COMPILER(RVCT) */
#if defined(__CC_ARM) || defined(__ARMCC__) || defined(__ARMCC_VERSION)
#define WTF_COMPILER_RVCT 1
#endif

/* COMPILER(GCC) */
/* --gnu option of the RVCT compiler also defines __GNUC__ */
#if defined(__GNUC__) && !COMPILER(RVCT)
#define WTF_COMPILER_GCC 1
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

/* COMPILER(MSVC) */
#if defined(_MSC_VER)
# define WTF_COMPILER_MSVC 1
# if _MSC_VER < 1400
#  define WTF_COMPILER_MSVC7 1
# endif
# define NOMINMAX
#endif

#define ENABLE_JSC_MULTIPLE_THREADS 0
#define HAVE_ACCESSIBILITY 0
#define HAVE_ERRNO_H 0
#define HAVE_LANGINFO_H 0
#define HAVE_MMAP 0
#define HAVE_SBRK 0
#define HAVE_STRINGS_H 0
#define HAVE_SYS_PARAM_H 0
#define HAVE_SYS_TIME_H 0

/* ENABLE macro defaults */

/* fastMalloc match validation allows for runtime verification that
   new is matched by delete, fastMalloc is matched by fastFree, etc. */
#define ENABLE_FAST_MALLOC_MATCH_VALIDATION 0
//#define ENABLE_ICONDATABASE 1 // defined in makefile
//#define ENABLE_DATABASE 1 // defined in makefile
//#define ENABLE_JAVASCRIPT_DEBUGGER 1 // defined in makefile
//#define ENABLE_FTPDIR 1
#ifdef ENABLE_JAVASCRIPT_DEBUGGER
# undef ENABLE_JAVASCRIPT_DEBUGGER
# define ENABLE_JAVASCRIPT_DEBUGGER 0
#endif
#define ENABLE_CONTEXT_MENUS 1
#define ENABLE_DRAG_SUPPORT 1
#define ENABLE_DASHBOARD_SUPPORT 0
#define ENABLE_INSPECTOR 0
#define ENABLE_MAC_JAVA_BRIDGE 0
#define ENABLE_NETSCAPE_PLUGIN_API 1
#define WTF_USE_PLUGIN_HOST_PROCESS 0
#define ENABLE_ORIENTATION_EVENTS 0
#define ENABLE_OPCODE_STATS 0

#define ENABLE_SAMPLING_COUNTERS 0
#define ENABLE_SAMPLING_FLAGS 0
#define ENABLE_OPCODE_SAMPLING 0
#define ENABLE_CODEBLOCK_SAMPLING 0
#if ENABLE(CODEBLOCK_SAMPLING) && !ENABLE(OPCODE_SAMPLING)
#error "CODEBLOCK_SAMPLING requires OPCODE_SAMPLING"
#endif
#if ENABLE(OPCODE_SAMPLING) || ENABLE(SAMPLING_FLAGS)
#define ENABLE_SAMPLING_THREAD 1
#endif

#define ENABLE_GEOLOCATION 0
#define ENABLE_NOTIFICATIONS 0
#define ENABLE_TEXT_CARET 1
#define ENABLE_ON_FIRST_TEXTAREA_FOCUS_SELECT_ALL 0

#define WTF_USE_JSVALUE32_64 1
#define ENABLE_REPAINT_THROTTLING 0

#define ENABLE_JIT 0
#define JSC_HOST_CALL

#if COMPILER(GCC) && !ENABLE(JIT)
#define HAVE_COMPUTED_GOTO 1
#endif

#define WTF_USE_INTERPRETER 1

#define WTF_USE_FONT_FAST_PATH 1

#if COMPILER(GCC)
#define WARN_UNUSED_RETURN __attribute__ ((warn_unused_result))
#else
#define WARN_UNUSED_RETURN
#endif

#define ENABLE_PLUGIN_PACKAGE_SIMPLE_HASH 1

/* Set up a define for a common error that is intended to cause a build error -- thus the space after Error. */
#define WTF_PLATFORM_CFNETWORK Error USE_macro_should_be_used_with_CFNETWORK

#define ENABLE_JSC_ZOMBIES 0


// static configurations for WKC

#define WTF_USE_TLSF 1
#define WTF_USE_PTHREADS 0
//#define WTF_USE_JSC 1
#define WTF_USE_V8 0

#define WTF_USE_WKC_UNICODE 1

#define HAVE_PTHREAD_NP_H 0
#undef WTF_FEATURE_WIN_OS
#undef WTF_PLATFORM_WIN_OS
#undef WTF_PLATFORM_WIN
#undef WTF_USE_WININET
#undef HAVE_STRINGS_H
#undef HAVE_SYS_TIME_H
#undef HAVE_MMAP
#undef HAVE_VIRTUALALLOC
#undef HAVE_LANGINFO_H
#undef HAVE_SYS_PARAM_H
#undef ENABLE_JSC_MULTIPLE_THREADS
#undef ENABLE_WREC
#undef ENABLE_JIT
#undef ENABLE_ASSEMBLER
#undef ENABLE_VIDEO
#undef ENABLE_NETSCAPE_PLUGIN_API
#undef WTF_PLATFORM_CAIRO
#undef USE_SYSTEM_MALLOC

//#define ENABLE_IMAGE_DECODER_DOWN_SAMPLING 1
//#define IMAGE_DECODER_DOWN_SAMPLING_MAX_NUMBER_OF_PIXELS (512*512)

#define WTF_USE_CURL 1

#ifndef USE_SETJMP
# define ENABLE_WKC_HIDE_SETJMP 1
#endif

// spatial navigation
#define ENABLE_WKC_SPATIAL_NAVI 1
// enable to notify viewport changing
#define ENABLE_WKC_META_VIEWPORT 1
// pagesave
#define ENABLE_WKC_PAGESAVE_MHTML 1
// enable to change frame setting
#define ENABLE_WKC_FRAME_FLATTENING 1
/* enable to change column-fit layout and small-screen-rendering layout */
#define ENABLE_WKC_ANDROID_LAYOUT 1
// Ensure that the fixed elements are always relative to the top document.
#define ENABLE_WKC_ANDROID_FIXED_ELEMENTS 1
// enable to notify scroll position even if scroll position is not changed
#define ENABLE_WKC_FORCE_NOTIFY_SCROLL 1
// enable shrink decode
#define ENABLE_WKC_IMAGE_DECODER_DOWN_SAMPLING 1
// enable optimization to ignore fixed background images when scrolling a page.
#define ENABLE_FAST_MOBILE_SCROLLING 1

#ifndef DISABLE_WKC_REPLACE_NEWDELETE_WITH_FASTMALLOC
# define ENABLE_WKC_REPLACE_NEWDELETE_WITH_FASTMALLOC 1
#endif

#if defined(WKC_ENABLE_FASTMALLOC_SMALL_CLASS_BY_TABLE) && WKC_ENABLE_FASTMALLOC_SMALL_CLASS_BY_TABLE
#define ENABLE_WKC_FASTMALLOC_SMALL_CLASS_BY_TABLE 1
#endif // defined(WKC_ENABLE_FASTMALLOC_SMALL_CLASS_BY_TABLE) && WKC_ENABLE_FASTMALLOC_SMALL_CLASS_BY_TABLE

//#define ENABLE_DOM_STORAGE 1 // defined in makefile
//#define ENABLE_OFFLINE_WEB_APPLICATIONS 1 // defined in makefile
#define JS_EXPORTDATA
#define __N p__N
#define WTF_USE_JSVALUE32_64 1

#ifndef __BUILDING_IN_VS__
typedef unsigned int size_t;
#endif

typedef void* LPVOID;
#define _CRT_TERMINATE_DEFINED
#define MAX_PATH            260
#if !defined(_TRUNCATE)
#define _TRUNCATE ((size_t)-1)
#endif

extern "C" const unsigned short* wkcSystemGetNavigatorPlatformPeer(void);
extern "C" const unsigned short* wkcSystemGetNavigatorProductPeer(void);
extern "C" const unsigned short* wkcSystemGetNavigatorProductSubPeer(void);
extern "C" const unsigned short* wkcSystemGetNavigatorVendorPeer(void);
extern "C" const unsigned short* wkcSystemGetNavigatorVendorSubPeer(void);
#define WEBCORE_NAVIGATOR_PLATFORM wkcSystemGetNavigatorPlatformPeer()
#define WEBCORE_NAVIGATOR_PRODUCT wkcSystemGetNavigatorProductPeer()
#define WEBCORE_NAVIGATOR_PRODUCT_SUB wkcSystemGetNavigatorProductSubPeer()
#define WEBCORE_NAVIGATOR_VENDOR wkcSystemGetNavigatorVendorPeer()
#define WEBCORE_NAVIGATOR_VENDOR_SUB wkcSystemGetNavigatorVendorSubPeer()

// for Memory Debug
#if defined(WKC_ENABLE_FASTMALLOC_USED_MEMORY_INFO) && WKC_ENABLE_FASTMALLOC_USED_MEMORY_INFO
#define ENABLE_WKC_FASTMALLOC_USED_MEMORY_INFO 1
#endif // defined(WKC_ENABLE_FASTMALLOC_USED_MEMORY_INFO) && WKC_ENABLE_FASTMALLOC_USED_MEMORY_INFO

#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
#if defined(WKC_ENABLE_FASTMALLOC_DYNAMIC_ALLOCATION) && WKC_ENABLE_FASTMALLOC_DYNAMIC_ALLOCATION
#define ENABLE_WKC_FASTMALLOC_DYNAMIC_ALLOCATION 1
#endif // WKC_ENABLE_FASTMALLOC_DYNAMIC_ALLOCATION
#if defined(WKC_ENABLE_FASTMALLOC_STACK_TRACE) && WKC_ENABLE_FASTMALLOC_STACK_TRACE
#define ENABLE_WKC_FASTMALLOC_STACK_TRACE 1
#endif // defined(WKC_ENABLE_FASTMALLOC_STACK_TRACE) && WKC_ENABLE_FASTMALLOC_STACK_TRACE
#endif // ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)

#endif // _WKCPLATFORM_H_
