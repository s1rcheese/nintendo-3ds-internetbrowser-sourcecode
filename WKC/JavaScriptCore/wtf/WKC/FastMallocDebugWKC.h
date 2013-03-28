/*
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef WTF_FastMallocDebugWKC_h
#define WTF_FastMallocDebugWKC_h

#include "FastMallocWKC.h"

#if PLATFORM(WKC)

#define FASTMALLOCDEBUG_HOWMANY(a, b) (((int)(a) + ((int)(b - 1))) / (int)(b))

namespace WTF {

    enum {
        kMaxNumUsedMemArray     = FASTMALLOCDEBUG_HOWMANY(kPageSize, kAlignment),
        kMaxNumSpanInfoArray    = 5376,     // Initial Memory Size / kPageSize
#ifdef MAX_PATH
        kMaxNameLength          = MAX_PATH
#else
        kMaxNameLength          = 256
#endif
    };

    typedef struct ObjectNode_ ObjectNode;

    struct ObjectNode_ {
        void* obj;
        ObjectNode* next;
        ObjectNode* prev;
    };

    struct UsedMemoryInfo_ {
        void* adr;
        unsigned int requestSize;
        unsigned int usedSize;
        unsigned short classID;
    };

    typedef struct UsedMemoryInfo_ UsedMemoryInfo;

    struct SpanInfo_ {
        void* span;
        void* head;
        void* tail;
        void* nextAddress;
        bool used;
        unsigned short pages;
        unsigned short classID;
        unsigned int blockSize; // valid when classID > 0
        unsigned int usedBlocks;
        unsigned int maxBlocks;
        unsigned int size;
        unsigned int requestedSize;
        unsigned int numUsedMemArray;
#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
        UsedMemoryInfo* usedMemArray;
        UsedMemoryInfo** usedMemPtrArray;
#else
        UsedMemoryInfo usedMemArray[kMaxNumUsedMemArray];
        UsedMemoryInfo* usedMemPtrArray[kMaxNumUsedMemArray];
#endif // ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    };

    typedef struct SpanInfo_ SpanInfo;

    struct MemoryInfo_ {
        unsigned int pageSize;
        unsigned int numSpanArray;
#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
        SpanInfo* spanArray;
        SpanInfo** spanPtrArray;
#else
        SpanInfo spanArray[kMaxNumSpanInfoArray];
        SpanInfo* spanPtrArray[kMaxNumSpanInfoArray];
#endif // ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    };

    typedef struct MemoryInfo_ MemoryInfo;

    struct StackObjectInfo_ {
        void* adr;
        unsigned int line;
        unsigned int displacement;
        unsigned char name[kMaxNameLength + 1];
    };

    typedef struct StackObjectInfo_ StackObjectInfo;

    typedef ObjectNode StackTraceInfo;

    typedef struct MemoryLeakInfo_ MemoryLeakInfo;

    struct MemoryLeakInfo_ {
        void* adr;
        unsigned int size;
        unsigned int reqSize;
        time_t curTime;
        MemoryLeakInfo* next;
        MemoryLeakInfo* prev;
    };

    typedef struct MemoryLeakNode_ MemoryLeakNode;

    struct MemoryLeakNode_ {
        unsigned int numInfo;
        MemoryLeakInfo* memHead;
        MemoryLeakInfo* memTail;

        unsigned int numTrace;
        StackTraceInfo* stHead;
        StackTraceInfo* stTail;

        MemoryLeakNode* next;
        MemoryLeakNode* prev;
    };

    struct MemoryLeakRoot_ {
        MemoryLeakNode* head;
        MemoryLeakNode* tail;
        unsigned int num;
        unsigned int leakReqSum;
        unsigned int leakSum;
    };

    typedef struct MemoryLeakRoot_ MemoryLeakRoot;

    typedef void (*MemoryLeakDumpProc)(void *in_ctx);

    void fastMallocGetSpanInfo(void* adr, SpanInfo& info);
    unsigned int fastMallocGetSizeClass(void* ptr);
    unsigned int fastMallocGetSpanSizeClass(void* ptr);

    bool initializeTCMallocDebugWKC();
    void finalizeTCMallocDebugWKC();
    void fastMallocDebugSetUsedSpan(void* span);
    void fastMallocDebugRemoveUsedSpan(void* span);
    void fastMallocDebugSetUsedMemory(void* ptr, unsigned int reqSize, unsigned int usedSize, unsigned short cl = 0);
    void fastMallocDebugRemoveUsedMemory(void* ptr);

    void fastMallocDebugGetNumberInfo(unsigned int& maxNumSpan, unsigned int& maxNumMem);

    // Memory Map
    void fastMallocDebugGetMemoryMap(MemoryInfo& memInfo, bool needUsedMemory = true);
    bool fastMallocDebugAllocMemoryMap(MemoryInfo& memInfo);
    void fastMallocDebugFreeMemoryMap(MemoryInfo& memInfo);

    // Memory Leaks
    void fastMallocDebugSetMemoryLeakDumpProc(MemoryLeakDumpProc in_proc, void* in_ctx);
    bool fastMallocDebugGetMemoryLeakInfo(MemoryLeakRoot& leakRoot, bool resolveSymbol = false);
    void fastMallocDebugClearMemoryLeakInfo(MemoryLeakRoot& leakRoot);

    // Stack Trace
    void fastMallocDebugClearStackTrace();

} // namespace WTF

#endif // PLATFORM(WKC)

#endif /* WTF_FastMallocDebugWKC_h */
