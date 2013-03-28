// Copyright (c) 2005, 2007, Google Inc.
// All rights reserved.
// Copyright (c) 2010, 2011 ACCESS CO., LTD. All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// ---
// Author: Sanjay Ghemawat

#include "config.h"
#define HAVE_MADV_FREE_REUSE 1
#include <wkc/wkcmpeer.h>
#include "TCSystemAlloc.h"

#if PLATFORM(WKC)
void* TCMalloc_SystemAlloc(size_t size, size_t *actual_size, size_t alignment, unsigned int range_type) {
  // Discard requests that overflow
  if (size + alignment < size) return NULL;
  void* ret = NULL;

  ret = wkcMemoryAllocHeapXPeer(size, alignment, range_type);
  if (!ret) return NULL;
  if (actual_size) {
    *actual_size = size;
  }
  return ret;
}

void TCMalloc_SystemRelease(void* start, size_t length)
{
  wkcMemoryFreeHeapPeer(start, length);
}

void TCMalloc_SystemCommit(void* start, size_t length)
{
  wkcMemoryCommitHeapPeer(start, length);
}

void* TCMalloc_SystemAlloc(size_t size, unsigned int range_type)
{
  return wkcMemoryAllocXPeer(size, range_type);
}

void TCMalloc_SystemRelease(void* ptr)
{
  wkcMemoryFreePeer(ptr);
}

#else  /* PLATFORM(WKC) */

void* TCMalloc_SystemAlloc(size_t size, size_t *actual_size, size_t alignment) {
  // Discard requests that overflow
  if (size + alignment < size) return NULL;
  void* ret = NULL;

  ret = wkcMemoryAllocHeapPeer(size, alignment);
  if (!ret) return NULL;
  if (actual_size) {
    *actual_size = size;
  }
  return ret;
}

void TCMalloc_SystemRelease(void* start, size_t length)
{
   wkcMemoryFreeHeapPeer(start, length);
}

void TCMalloc_SystemCommit(void* start, size_t length)
{
    wkcMemoryCommitHeapPeer(start, length);
}
#endif /* PLATFORM(WKC) */

#ifndef ENABLE_REPLACEMENT_SYSTEMMEMORY
namespace WTF {

extern void* fastMalloc2(size_t);
extern TryMallocReturnValue tryFastMalloc2(size_t);
extern void* fastRealloc2(void*,size_t);
extern TryMallocReturnValue tryFastRealloc2(void*,size_t);
extern void* fastCalloc2(size_t,size_t);
extern TryMallocReturnValue tryFastCalloc2(size_t,size_t);
extern void fastFree2(void*);

extern "C" void* wkcMemoryReallocPreInitPeer(void* ptr, unsigned int len);

void*
fastMalloc(size_t size)
{
    if (wkcMemoryIsInitializedPeer() && !wkcMemoryIsCrashingPeer()) {
        wkcMemoryCheckAvailabilityPeer(size);
        return fastMalloc2(size);
    } else if (wkcMemoryIsInitializedPeer() && wkcMemoryIsCrashingPeer()) {
        void* result;
        TryMallocReturnValue rv = tryFastMalloc2(size);
        if (rv.getValue(result)) {
            return result;
        }
    }
    return wkcMemoryAllocPreInitPeer(size);
}

TryMallocReturnValue
tryFastMalloc(size_t size)
{
    if (wkcMemoryIsInitializedPeer() && !wkcMemoryIsCrashingPeer()) {
        wkcMemoryCheckAvailabilityPeer(size);
        return tryFastMalloc2(size);
    } else {
        return wkcMemoryAllocPreInitPeer(size);
    }
}

void* 
fastRealloc(void* ptr,size_t size)
{
    if (wkcMemoryIsInitializedPeer() && !wkcMemoryIsCrashingPeer()) {
        wkcMemoryCheckAvailabilityPeer(size);
        return fastRealloc2(ptr,size);
    } else if (wkcMemoryIsInitializedPeer() && wkcMemoryIsCrashingPeer()) {
        void* result;
        TryMallocReturnValue rv = tryFastRealloc2(ptr, size);
        if (rv.getValue(result)) {
            return result;
        }
    }
    return wkcMemoryReallocPreInitPeer(ptr, size);
}

TryMallocReturnValue
tryFastRealloc(void* ptr,size_t size)
{
    if (wkcMemoryIsInitializedPeer() && !wkcMemoryIsCrashingPeer()) {
        wkcMemoryCheckAvailabilityPeer(size);
        return tryFastRealloc2(ptr, size);
    } else {
        return wkcMemoryReallocPreInitPeer(ptr, size);
    }
}

void*
fastCalloc(size_t p,size_t n)
{
    wkcMemoryCheckAvailabilityPeer(p*n);
    return fastCalloc2(p, n);
}

TryMallocReturnValue
tryFastCalloc(size_t p,size_t n)
{
    wkcMemoryCheckAvailabilityPeer(p*n);
    return tryFastCalloc2(p, n);
}

void
fastFree(void* p)
{
    if (wkcMemoryFreePreInitPeer(p)) return;
    if (!wkcMemoryIsInitializedPeer()) return;
    fastFree2(p);
}

}

#endif /* ENABLE_REPLACEMENT_SYSTEMMEMORY */

