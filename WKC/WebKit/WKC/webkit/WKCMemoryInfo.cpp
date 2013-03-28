/*
 *  WKCMemoryInfo.cpp
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

#include "config.h"
#include "WKCMemoryInfo.h"
#include "FastMallocWKC.h"
#include "FastMallocDebugWKC.h"

namespace WKC {

namespace FastMalloc {

void
GetStatistics(Statistics& stat, size_t requestSize)
{
    COMPILE_ASSERT(kPageShift == WTF::kPageShift, kPageShift);
    COMPILE_ASSERT(kPageSize == WTF::kPageSize, kPageSize);
    COMPILE_ASSERT(kAlignShift == WTF::kAlignShift, kAlignShift);
    COMPILE_ASSERT(kAlignment == WTF::kAlignment, kAlignment);
    COMPILE_ASSERT(kNumClasses == WTF::kNumClasses, kNumClasses);
    COMPILE_ASSERT(kMinSystemAlloc == WTF::kMinSystemAlloc, kMinSystemAlloc);
    COMPILE_ASSERT(kMaxPages == WTF::kMaxPages, kMaxPages);
    COMPILE_ASSERT(kMaxHeapSizes == WTF::kMaxHeapSizes, kMaxHeapSizes);
    COMPILE_ASSERT(sizeof(Statistics) == sizeof(WTF::FastMallocStatistics), sizeof__Statistics);

    WTF::fastMallocStatistics(*(WTF::FastMallocStatistics*)&stat, requestSize);
}

size_t
GetStatisticsFreeSizeInCache()
{
    return WTF::fastMallocStatisticsFreeSizeInCache();
}

size_t
GetStatisticsFreeSizeInHeap()
{
    return WTF::fastMallocStatisticsFreeSizeInHeap();
}

size_t
GetStatisticsMaxFreeBlockSizeInHeap(size_t requestSize)
{
    return WTF::fastMallocStatisticsMaxFreeBlockSizeInHeap(requestSize);
}

size_t
GetStatisticsEachClassThreadFreeSizeInCaches(unsigned int classID, bool bSize)
{
    return WTF::fastMallocStatisticsEachClassThreadFreeSizeInCaches(classID, bSize);
}

size_t
GetStatisticsEachClassCentralFreeSizeInCaches(unsigned int classID, bool bSize)
{
    return WTF::fastMallocStatisticsEachClassCentralFreeSizeInCaches(classID, bSize);
}

size_t
GetStatisticsEachClassBlockSizeInCaches(unsigned int classID)
{
    return WTF::fastMallocStatisticsEachClassBlockSizeInCaches(classID);
}

size_t
GetStatisticsEachPageFreeSizeInHeap(unsigned int iPage, unsigned int type, bool bSize)
{
    int wtf_type = 0;

    if (type & FASTMALLCSTATISTICS_SPANLIST_TYPE_NORMAL) {
        wtf_type |= WTF::FASTMALLCSTATISTICS_SPANLIST_TYPE_NORMAL;
    }
    if (type & FASTMALLCSTATISTICS_SPANLIST_TYPE_RETURNED) {
        wtf_type |= WTF::FASTMALLCSTATISTICS_SPANLIST_TYPE_RETURNED;
    }
    return WTF::fastMallocStatisticsEachPageFreeSizeInHeap(iPage, wtf_type,  bSize);
}

void
GetStatisticsLargeFreeSizeInHeap(size_t* sizeArray, unsigned int maxArray, unsigned int* numArray, unsigned int type, bool bSize)
{
    WTF::fastMallocStatisticsLargeFreeSizeInHeap(sizeArray, maxArray, numArray, type, bSize);
}

void
GetMemoryMap(MemoryInfo& memInfo, bool needUsedMemory)
{
    COMPILE_ASSERT(kMaxNumUsedMemArray == WTF::kMaxNumUsedMemArray, kMaxNumUsedMemArray);
    COMPILE_ASSERT(kMaxNumSpanInfoArray == WTF::kMaxNumSpanInfoArray, kMaxNumSpanInfoArray);
    COMPILE_ASSERT(sizeof(UsedMemoryInfo) == sizeof(WTF::UsedMemoryInfo), sizeof__UsedMemoryInfo);
    COMPILE_ASSERT(sizeof(SpanInfo) == sizeof(WTF::SpanInfo), sizeof__SpanInfo);
    COMPILE_ASSERT(sizeof(MemoryInfo) == sizeof(WTF::MemoryInfo), sizeof__MemoryInfo);

    WTF::fastMallocDebugGetMemoryMap(*(WTF::MemoryInfo*)&memInfo, needUsedMemory);
}

bool
AllocMemoryMap(MemoryInfo& memInfo)
{
    bool ret = false;

    COMPILE_ASSERT(kMaxNumUsedMemArray == WTF::kMaxNumUsedMemArray, kMaxNumUsedMemArray);
    COMPILE_ASSERT(kMaxNumSpanInfoArray == WTF::kMaxNumSpanInfoArray, kMaxNumSpanInfoArray);
    COMPILE_ASSERT(sizeof(UsedMemoryInfo) == sizeof(WTF::UsedMemoryInfo), sizeof__UsedMemoryInfo);
    COMPILE_ASSERT(sizeof(SpanInfo) == sizeof(WTF::SpanInfo), sizeof__SpanInfo);
    COMPILE_ASSERT(sizeof(MemoryInfo) == sizeof(WTF::MemoryInfo), sizeof__MemoryInfo);
    ret = WTF::fastMallocDebugAllocMemoryMap(*(WTF::MemoryInfo*)&memInfo);
    return ret;
}

void
ReleaseMemoryMap(MemoryInfo& memInfo)
{
    COMPILE_ASSERT(kMaxNumUsedMemArray == WTF::kMaxNumUsedMemArray, kMaxNumUsedMemArray);
    COMPILE_ASSERT(kMaxNumSpanInfoArray == WTF::kMaxNumSpanInfoArray, kMaxNumSpanInfoArray);
    COMPILE_ASSERT(sizeof(UsedMemoryInfo) == sizeof(WTF::UsedMemoryInfo), sizeof__UsedMemoryInfo);
    COMPILE_ASSERT(sizeof(SpanInfo) == sizeof(WTF::SpanInfo), sizeof__SpanInfo);
    COMPILE_ASSERT(sizeof(MemoryInfo) == sizeof(WTF::MemoryInfo), sizeof__MemoryInfo);

    WTF::fastMallocDebugFreeMemoryMap(*(WTF::MemoryInfo*)&memInfo);
}

void
SetMemoryLeakDumpProc(MemoryLeakDumpProc in_proc, void* in_ctx)
{
    WTF::fastMallocDebugSetMemoryLeakDumpProc((WTF::MemoryLeakDumpProc)in_proc, in_ctx);
}

bool
GetMemoryLeakInfo(MemoryLeakRoot& leakRoot, bool resolveSymbol)
{
    COMPILE_ASSERT(sizeof(StackObjectInfo) == sizeof(WTF::StackObjectInfo), sizeof__StackObjectInfo);
    COMPILE_ASSERT(sizeof(StackTraceInfo) == sizeof(WTF::StackTraceInfo), sizeof__StackTraceInfo);
    COMPILE_ASSERT(sizeof(MemoryLeakInfo) == sizeof(WTF::MemoryLeakInfo), sizeof__MemoryLeakInfo);
    COMPILE_ASSERT(sizeof(MemoryLeakNode) == sizeof(WTF::MemoryLeakNode), sizeof__MemoryLeakNode);
    COMPILE_ASSERT(sizeof(MemoryLeakRoot) == sizeof(WTF::MemoryLeakRoot), sizeof__MemoryLeakRoot);
    COMPILE_ASSERT(kMaxFileNameLength <= WTF::kMaxNameLength, kMaxFileNameLength);

    return WTF::fastMallocDebugGetMemoryLeakInfo(*(WTF::MemoryLeakRoot*)&leakRoot, resolveSymbol);
}

void
ClearStackTrace()
{
    WTF::fastMallocDebugClearStackTrace();
}

void
ReleaseMemoryLeakInfo(MemoryLeakRoot& leakRoot)
{
    WTF::fastMallocDebugClearMemoryLeakInfo(*(WTF::MemoryLeakRoot*)&leakRoot);
}

bool
CanAllocMemory(size_t inRequestSize, size_t* outAvailSize, bool* outCheckPeer, size_t* outRealRequestSize, size_t* outAvailPeerSize)
{
    return WTF::fastMallocCanAllocMemory(inRequestSize, outAvailSize, outCheckPeer, outRealRequestSize, outAvailPeerSize);
}

} // namespace FastMalloc

} // namespace WKC
