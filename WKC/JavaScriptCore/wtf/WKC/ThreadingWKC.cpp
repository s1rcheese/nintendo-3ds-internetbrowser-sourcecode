/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Justin Haygood (jhaygood@reaktix.com)
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "Threading.h"

#include <wkc/wkcpeer.h>

#include "CurrentTime.h"
#include "HashMap.h"
#include "MainThread.h"
#include "RandomNumberSeed.h"

//#define USE_LOCK 1

namespace WTF {

static Mutex* atomicallyInitializedStaticMutex = NULL;
static ThreadIdentifier mainThreadIdentifier = NULL;

static Mutex* gMutex = 0;

static Mutex& threadMapMutex()
{
    if (!gMutex) {
        gMutex = new Mutex;
    }
    return *gMutex;
}

void initializeThreading()
{
    if (!atomicallyInitializedStaticMutex) {
        atomicallyInitializedStaticMutex = new Mutex;
        threadMapMutex();
        initializeRandomNumberGenerator();
        mainThreadIdentifier = currentThread();
        initializeMainThread();
    }
}

void lockAtomicallyInitializedStaticMutex()
{
    atomicallyInitializedStaticMutex->lock();
}

void unlockAtomicallyInitializedStaticMutex()
{
    atomicallyInitializedStaticMutex->unlock();
}

ThreadIdentifier
createThreadInternal(ThreadFunction entryPoint, void* data, const char*)
{
    void* instance = NULL;
    wkcThreadProc proc = (wkcThreadProc)entryPoint;
    instance = wkcThreadCreatePeer(proc, data);
    return (ThreadIdentifier)instance;
}

void setThreadNameInternal(const char*)
{
}

int waitForThreadCompletion(ThreadIdentifier threadID, void** result)
{
    void* instance = (void *)threadID;

    return wkcThreadJoinPeer(instance, result);
}

void detachThread(ThreadIdentifier in_threadid)
{
    void* instance = (void *)in_threadid;
    wkcThreadDetachPeer(instance);
}

ThreadIdentifier currentThread()
{
    void* instance = NULL;

    instance = wkcThreadCurrentThreadPeer();
    return (ThreadIdentifier)instance;
}

bool isMainThread()
{
    return currentThread() == mainThreadIdentifier;
}

Mutex::Mutex()
     : m_mutex(0)
{
}

Mutex::~Mutex()
{
}

#ifdef USE_LOCK

static void SlowLock(volatile unsigned int* lockword)
{
  wkcThreadYieldPeer();        // Yield immediately since fast path failed
  while (true) {
    int r;
#if COMPILER(GCC)
#if PLATFORM(X86)
    __asm__ __volatile__
      ("xchgl %0, %1"
       : "=r"(r), "=m"(*lockword)
       : "0"(1), "m"(*lockword)
       : "memory");

#else
    int tmp = 1;
    __asm__ __volatile__
        ("1: lwarx %0, 0, %1\n\t"
         "stwcx. %2, 0, %1\n\t"
         "bne- 1b\n\t"
         "isync"
         : "=&r" (r), "=r" (lockword)
         : "r" (tmp), "1" (lockword)
         : "memory");
#endif
#elif COMPILER(MSVC)
    __asm {
        mov eax, lockword     ; assign lockword into eax
        mov ebx, 1            ; assign 1 into ebx
        xchg [eax], ebx       ; exchange *lockword and 1
        mov r, ebx            ; store old value of *lockword in r
    }
#elif COMPILER(RVCT)
    __asm
    {
        mov      r0, #1
        ldrex    r1, [lockword]
        teq      r1, #0
        strexeq  r2, r0, [lockword]
        mov      r, r1
    }
#endif
    if (!r) {
      return;
    }

    // This code was adapted from the ptmalloc2 implementation of
    // spinlocks which would sched_yield() upto 50 times before
    // sleeping once for a few milliseconds.  Mike Burrows suggested
    // just doing one sched_yield() outside the loop and always
    // sleeping after that.  This change helped a great deal on the
    // performance of spinlocks under high contention.  A test program
    // with 10 threads on a dual Xeon (four virtual processors) went
    // from taking 30 seconds to 16 seconds.

    // Sleep for a few milliseconds
    struct timespec tm;
    tm.tv_sec = 0;
    tm.tv_nsec = 2000001;
    wkc_nanosleep(&tm, NULL);
  }
}
#endif

void Mutex::lock()
{
#ifndef USE_LOCK
    // Ugh!: temporarily disabled!
    // 100423 ACCESS Co.,Ltd.
#else
    int r;
#if COMPILER(GCC)
#if PLATFORM(X86)
    __asm__ __volatile__
      ("xchgl %0, %1"
       : "=r"(r), "=m"(m_mutex)
       : "0"(1), "m"(m_mutex)
       : "memory");
#else
    volatile unsigned int *lockword_ptr = &lockword_;
    __asm__ __volatile__
        ("1: lwarx %0, 0, %1\n\t"
         "stwcx. %2, 0, %1\n\t"
         "bne- 1b\n\t"
         "isync"
         : "=&r" (r), "=r" (lockword_ptr)
         : "r" (1), "1" (lockword_ptr)
         : "memory");
#endif
#elif COMPILER(MSVC)
    __asm {
        mov eax, this    ; store &lockword_ (which is this+0) in eax
        mov ebx, 1       ; store 1 in ebx
        xchg [eax], ebx  ; exchange lockword_ and 1
        mov r, ebx       ; store old value of lockword_ in r
    }
#elif COMPILER(RVCT)
    __asm
    {
        mov      r0, #1
        ldrex    r1, [this]
        teq      r1, #0
        strexeq  r2, r0, [this]
        mov      r, r1
    }
#endif
    if (r) SlowLock(&m_mutex);
#endif
}

bool Mutex::tryLock()
{
#ifndef USE_LOCK
    // Ugh!: temporarily disabled!
    // 100423 ACCESS Co.,Ltd.
#else
    int r;
#if COMPILER(GCC)
#if PLATFORM(X86)
    __asm__ __volatile__
      ("xchgl %0, %1"
       : "=r"(r), "=m"(m_mutex)
       : "0"(1), "m"(m_mutex)
       : "memory");
#else
    volatile unsigned int *lockword_ptr = &lockword_;
    __asm__ __volatile__
        ("1: lwarx %0, 0, %1\n\t"
         "stwcx. %2, 0, %1\n\t"
         "bne- 1b\n\t"
         "isync"
         : "=&r" (r), "=r" (lockword_ptr)
         : "r" (1), "1" (lockword_ptr)
         : "memory");
#endif
#elif COMPILER(MSVC)
    __asm {
        mov eax, this    ; store &lockword_ (which is this+0) in eax
        mov ebx, 1       ; store 1 in ebx
        xchg [eax], ebx  ; exchange lockword_ and 1
        mov r, ebx       ; store old value of lockword_ in r
    }
#elif COMPILER(RVCT)
    __asm
    {
        mov      r0, #1
        ldrex    r1, [this]
        teq      r1, #0
        strexeq  r2, r0, [this]
        mov      r, r1
    }
#endif
    if (r) return false;
#endif
    return true;
}

void Mutex::unlock()
{
#ifndef USE_LOCK
    // Ugh!: temporarily disabled!
    // 100423 ACCESS Co.,Ltd.
#else
#if COMPILER(GCC)
#if PLATFORM(X86)
    __asm__ __volatile__
      ("movl $0, %0"
       : "=m"(m_mutex)
       : "m" (m_mutex)
       : "memory");
#else
    __asm__ __volatile__
      ("isync\n\t"
       "eieio\n\t"
       "stw %1, %0"
#if PLATFORM(DARWIN) || PLATFORM(PPC)
       : "=o" (lockword_)
#else
       : "=m" (lockword_) 
#endif
       : "r" (0)
       : "memory");
#endif
#elif COMPILER(MSVC)
      __asm {
          mov eax, this  ; store &lockword_ (which is this+0) in eax
          mov [eax], 0   ; set lockword_ to 0
      }
#elif COMPILER(RVCT)
    __asm
    {
        mov  r0, #0
        str  r0, [this]
    }
#endif
#endif
}

ThreadCondition::ThreadCondition()
{
    m_condition = wkcCondNewPeer();
}

ThreadCondition::~ThreadCondition()
{
    if (m_condition) {
        wkcCondDeletePeer((void *)m_condition);
    }
}

void ThreadCondition::wait(Mutex& mutex)
{
    wkcCondWaitPeer((void *)m_condition, (void *)mutex.impl());
}

bool ThreadCondition::timedWait(Mutex& mutex, double absoluteTime)
{
    // Time is in the past - return right away.
    if (absoluteTime < currentTime())
        return false;
    
    // Time is too far in the future for g_cond_timed_wait - wait forever.
    if (absoluteTime > WKC_INT_MAX) {
        wait(mutex);
        return true;
    }

    double offsettime = absoluteTime - currentTime();
    int timeSeconds = static_cast<int>(offsettime);
    int timeMilliseconds = static_cast<int>((offsettime - timeSeconds) * 1000.0);
    int timeout = timeSeconds * 1000 + timeMilliseconds;

    return wkcCondTimedWaitPeer((void *)m_condition, (void *)mutex.impl(), timeout);
}

void ThreadCondition::signal()
{
    wkcCondSignalPeer((void *)m_condition);
}

void ThreadCondition::broadcast()
{
    wkcCondBroadcastPeer((void *)m_condition);
}

void ThreadingWKC_deleteSharedInstance()
{
    delete atomicallyInitializedStaticMutex;
    delete gMutex;
}

void ThreadingWKC_resetVariables()
{
    atomicallyInitializedStaticMutex = NULL;
    mainThreadIdentifier = NULL;
    gMutex = 0;
}


}
