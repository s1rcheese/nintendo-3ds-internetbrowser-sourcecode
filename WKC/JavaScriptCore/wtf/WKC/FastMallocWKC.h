/*
 *  Copyright (c) 2005, 2007, Google Inc. All rights reserved.
 *  Copyright (C) 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
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

#ifndef WTF_FastMallocWKC_h
#define WTF_FastMallocWKC_h

#include <wkc/wkcmpeer.h>

#include "FastMalloc.h"

#if PLATFORM(WKC)

#define FASTMALLOC_HOWMANY(a, b) (((int)(a) + ((int)(b - 1))) / (int)(b))

namespace WTF {

    enum {
        kPageShift              = 12,
        kPageSize               = 1 << kPageShift,
        kAlignShift             = 3,
        kAlignment              = 1 << kAlignShift,
#if ENABLE(WKC_FASTMALLOC_SMALL_CLASS_BY_TABLE)
        kNumClasses             = 28,
#else
        kNumClasses             = 68,
#endif
        kMinSystemAlloc         = 1 << (20 - kPageShift),
        kMaxPages               = kMinSystemAlloc,
        kMaxHeapSizes           = 20       // Initial Memory Size/ (kMaxPages * kPageSize)
    };

    enum {
        FASTMALLCSTATISTICS_SPANLIST_TYPE_NORMAL    = 0x0001,
        FASTMALLCSTATISTICS_SPANLIST_TYPE_RETURNED  = 0x0002,
        FASTMALLCSTATISTICS_SPANLIST_TYPE_ALL       = (FASTMALLCSTATISTICS_SPANLIST_TYPE_NORMAL | FASTMALLCSTATISTICS_SPANLIST_TYPE_RETURNED)
    };

    struct FastMallocStatistics {
        size_t heapSize;
        size_t freeSizeInHeap;
        size_t freeSizeInCaches;
        size_t returnedSize;
        size_t pageSize;                                        // Default Page Size in TCMalloc
        size_t maxFreeBlockSizeInHeap;                          // Maximum Available Block Size in Free Array and Large List for Large Memories
        size_t classFreeSizeInCaches;                           // Available Free Size in Thread Cache and Central Cache
        size_t classBlockSizeInCaches;                          // Block Size in each class
        unsigned int pageLength;                                // Page Length shared with Small and Large Memories
        unsigned int classInCaches;                             // Class ID for Small Memories
        size_t eachClassAssignedPagesInCaches[kNumClasses];     // Assigned Pages for Small Memories
        size_t eachClassThreadFreeSizeInCaches[kNumClasses];    // Free Array in Thread Cache for Small Memories
        size_t eachClassCentralFreeSizeInCaches[kNumClasses];   // Central Cache Array for Small Memories
        size_t eachClassBlockSizeInCaches[kNumClasses];         // Block Size for Small Memories
        size_t eachPageFreeSizeInHeap[kMaxPages];               // Free Array for Large Memories
        unsigned int numLargeFreeSize;                          // Number of used array
        size_t largeFreeSizeInHeap[kMaxHeapSizes];              // Large List for Large Memories (Translated into array)
    };
    void fastMallocStatistics(FastMallocStatistics& stat, size_t requestSize = 0);

    size_t fastMallocStatisticsFreeSizeInCache();
    size_t fastMallocStatisticsFreeSizeInHeap();
    size_t fastMallocStatisticsMaxFreeBlockSizeInHeap(size_t requestSize = 0);
    size_t fastMallocStatisticsEachClassThreadFreeSizeInCaches(unsigned int classID, bool bSize = false);
    size_t fastMallocStatisticsEachClassCentralFreeSizeInCaches(unsigned int classID, bool bSize = false);
    size_t fastMallocStatisticsEachClassBlockSizeInCaches(unsigned int classID);
    size_t fastMallocStatisticsEachPageFreeSizeInHeap(unsigned int iPage, unsigned int type = FASTMALLCSTATISTICS_SPANLIST_TYPE_ALL, bool bSize = false);
    void fastMallocStatisticsLargeFreeSizeInHeap(size_t* sizeArray, unsigned int maxArray, unsigned int* numArray, unsigned int type = FASTMALLCSTATISTICS_SPANLIST_TYPE_ALL, bool bSize = false);

    bool fastMallocCanAllocMemory(size_t inRequestSize, size_t* outAvailSize = NULL, bool* outCheckPeer = NULL, size_t* outRealRequestSize = NULL, size_t* outAvailPeerSize = NULL);

    void fastMallocSetConfiguration(WKCMemoryConfig* config);
} // namespace WTF

#endif // PLATFORM(WKC)

#endif /* WTF_FastMallocWKC_h */
