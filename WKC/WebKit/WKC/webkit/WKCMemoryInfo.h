/*
 *  WKCMemoryInfo.h
 *
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
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#ifndef _WKCMEMORYINFO_H_
#define _WKCMEMORYINFO_H_

// Note: This file must be synchronized with FastMallocWKC.h

#include <wkc/wkcclib.h>


#ifdef MAX_PATH
#undef MAX_PATH
#endif
#define MAX_PATH  260   // must be synchronized with wkcplatform.h


#define WKC_FASTMALLOC_HOWMANY(a, b) (((unsigned int)(a) + ((unsigned int)(b - 1))) / (unsigned int)(b))

namespace WKC {

namespace FastMalloc {

enum {
    kPageShift  = 12,
    kPageSize   = 1 << kPageShift,
    kAlignShift = 3,
    kAlignment  = 1 << kAlignShift,
#if WKC_ENABLE_FASTMALLOC_SMALL_CLASS_BY_TABLE
    kNumClasses = 28,
#else
    kNumClasses = 68,
#endif // WKC_ENABLE_FASTMALLOC_SMALL_CLASS_BY_TABLE
    kMinSystemAlloc = 1 << (20 - kPageShift),
    kMaxPages = kMinSystemAlloc,
    kMaxHeapSizes = 20,

    kMaxNumUsedMemArray = WKC_FASTMALLOC_HOWMANY(kPageSize, kAlignment),
    kMaxNumSpanInfoArray = 5376,

    kMaxFileNameLength = MAX_PATH,

    END_OF_ENUM
};


enum {
    FASTMALLCSTATISTICS_SPANLIST_TYPE_NORMAL   = 0x0001,
    FASTMALLCSTATISTICS_SPANLIST_TYPE_RETURNED = 0x0002,
    FASTMALLCSTATISTICS_SPANLIST_TYPE_ALL      = (FASTMALLCSTATISTICS_SPANLIST_TYPE_NORMAL | FASTMALLCSTATISTICS_SPANLIST_TYPE_RETURNED)
};

struct UsedMemoryInfo_ {
    void* adr;
    unsigned int requestSize;
    unsigned int usedSize;
    unsigned short classID;
    bool outOfRange;
};
typedef struct UsedMemoryInfo_ UsedMemoryInfo;

struct SpanInfo_ {
    void* span;             // Never use this member.
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
#ifdef WKC_ENABLE_FASTMALLOC_DYNAMIC_ALLOCATION
    UsedMemoryInfo* usedMemArray;
    UsedMemoryInfo** usedMemPtrArray;
#else
    UsedMemoryInfo usedMemArray[kMaxNumUsedMemArray];
    UsedMemoryInfo* usedMemPtrArray[kMaxNumUsedMemArray];
#endif // WKC_ENABLE_FASTMALLOC_DYNAMIC_ALLOCATION
};
typedef struct SpanInfo_ SpanInfo;

struct MemoryInfo_ {
    unsigned int pageSize;
    unsigned int numSpanArray;
#ifdef WKC_ENABLE_FASTMALLOC_DYNAMIC_ALLOCATION
    SpanInfo* spanArray;
    SpanInfo** spanPtrArray;
#else
    SpanInfo spanArray[kMaxNumSpanInfoArray];
    SpanInfo* spanPtrArray[kMaxNumSpanInfoArray];
#endif // WKC_ENABLE_FASTMALLOC_DYNAMIC_ALLOCATION
};
typedef struct MemoryInfo_ MemoryInfo;

struct StackObjectInfo_ {
    void* adr;
    unsigned int line;
    unsigned int displacement;
    unsigned char name[kMaxFileNameLength + 1];
};

typedef struct StackObjectInfo_ StackObjectInfo;

typedef struct StackTraceInfo_ StackTraceInfo;

struct StackTraceInfo_ {
    StackObjectInfo* obj;
    StackTraceInfo* next;
    StackTraceInfo* prev;
};

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


struct Statistics_ {
    size_t heapSize;
    size_t freeSizeInHeap;
    size_t freeSizeInCaches;
    size_t returnedSize;
    size_t pageSize;
    size_t maxFreeBlockSizeInHeap;
    size_t classFreeSizeInCaches;
    size_t classBlockSizeInCaches;
    unsigned int pageLength;
    unsigned int classInCaches;
    size_t eachClassAssignedPagesInCaches[kNumClasses];
    size_t eachClassThreadFreeSizeInCaches[kNumClasses];
    size_t eachClassCentralFreeSizeInCaches[kNumClasses];
    size_t eachClassBlockSizeInCaches[kNumClasses];
    size_t eachPageFreeSizeInHeap[kMaxPages];
    unsigned int numLargeFreeSize;
    size_t largeFreeSizeInHeap[kMaxHeapSizes];
};
typedef struct Statistics_ Statistics;

typedef void (*MemoryLeakDumpProc)(void *in_ctx);

void GetStatistics(Statistics& stat, size_t requestSize = 0);
size_t GetStatisticsFreeSizeInCache();
size_t GetStatisticsFreeSizeInHeap();
size_t GetStatisticsMaxFreeBlockSizeInHeap(size_t requestSize = 0);
size_t GetStatisticsEachClassThreadFreeSizeInCaches(unsigned int classID, bool bSize = false);
size_t GetStatisticsEachClassCentralFreeSizeInCaches(unsigned int classID, bool bSize = false);
size_t GetStatisticsEachClassBlockSizeInCaches(unsigned int classID);
size_t GetStatisticsEachPageFreeSizeInHeap(unsigned int iPage, unsigned int type = FASTMALLCSTATISTICS_SPANLIST_TYPE_ALL, bool bSize = false);
void GetStatisticsLargeFreeSizeInHeap(size_t* sizeArray, unsigned int maxArray, unsigned int* numArray, unsigned int type = FASTMALLCSTATISTICS_SPANLIST_TYPE_ALL, bool bSize = false);
void GetMemoryMap(MemoryInfo& memInfo, bool needUsedMemory = true);
bool AllocMemoryMap(MemoryInfo& memInfo);
void ReleaseMemoryMap(MemoryInfo& memInfo);

void SetMemoryLeakDumpProc(MemoryLeakDumpProc in_proc, void* in_ctx);
bool GetMemoryLeakInfo(MemoryLeakRoot& leakRoot, bool resolveSymbol = false);
void ClearStackTrace();
void ReleaseMemoryLeakInfo(MemoryLeakRoot& leakRoot);

bool CanAllocMemory(size_t inRequestSize, size_t* outAvailSize = NULL, bool* outCheckPeer = NULL, size_t* outRealRequestSize = NULL, size_t* outAvailPeerSize = NULL);

} // namespace FastMalloc

} // namespace WKC

#endif // _WKCMEMORYINFO_H_
