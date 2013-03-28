// Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.
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


#include "config.h"
#include <string.h>

#if COMPILER(MSVC)
#include <fstream>
using namespace std;
#endif

#if PLATFORM(WKC)
#include <wkc/wkcmpeer.h>

#include "FastMallocDebugWKC.h"

#if COMPILER(MSVC)
#include "FastMallocSystemWKC.h"
#endif

#if ENABLE(WKC_FASTMALLOC_STACK_TRACE) && COMPILER(MSVC) && defined(_DEBUG)
#define ENABLE_WKC_FASTMALLOC_WIN_STACK_TRACE 1
#else
#define ENABLE_WKC_FASTMALLOC_WIN_STACK_TRACE 0
#endif
 
#if COMPILER(MSVC) && defined(_DEBUG)
#define ENABLE_WKC_VALIDATE_SORT_ALGORITHM 1
#else
#define ENABLE_WKC_VALIDATE_SORT_ALGORITHM 0
#endif

#define FASTMALLOCDEBUG_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define FASTMALLOCDEBUG_MIN(a, b) (((a) < (b)) ? (a) : (b))

#ifdef ENABLE_REPLACEMENT_SYSTEMMEMORY

namespace WTF {

void fastMallocDebugGetMemoryMap(MemoryInfo& memInfo, bool needUsedMemory)
{
    memset(&memInfo, 0, sizeof(MemoryInfo));
}

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
bool fastMallocDebugAllocMemoryMap(MemoryInfo& memInfo)
{
    memset(&memInfo, 0, sizeof(MemoryInfo));
    return true;
}

void fastMallocDebugFreeMemoryMap(MemoryInfo& memInfo)
{
    memset(&memInfo, 0, sizeof(MemoryInfo));
}
#endif  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

}   // namespace

#else  /* ENABLE_REPLACEMENT_SYSTEMMEMORY */

#if FORCE_SYSTEM_MALLOC

namespace WTF {

void fastMallocDebugGetMemoryMap(MemoryInfo& memInfo, bool needUsedMemory)
{
    memset(&memInfo, 0, sizeof(MemoryInfo));
}

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
bool fastMallocDebugAllocMemoryMap(MemoryInfo& memInfo)
{
    memset(&memInfo, 0, sizeof(MemoryInfo));
    return true;
}

void fastMallocDebugFreeMemoryMap(MemoryInfo& memInfo)
{
    memset(&memInfo, 0, sizeof(MemoryInfo));
}
#endif  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

} // namespace WTF

#else // FORCE_SYSTEM_MALLOC

#include "AlwaysInline.h"
#include "Assertions.h"
#include "TCSpinLock.h"
#include <wkc/wkcmpeer.h>
#if COMPILER(MSVC)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#if ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE)
#include <ImageHlp.h>
#endif

namespace WTF {

#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
// Constants
static const unsigned int kMinMemoryLeakInfo = 1024 * 1024;
static const unsigned int kMinMemoryLeakNode = 1024;
static const unsigned int kMinMemoryLeakHashOpenNum = 1024;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
static const unsigned int kMinAddressArrayNum = 128 * 1024;
static const unsigned int kMinSpanArrayNum = 1024;
static const unsigned int kMinHashOpenNum = 128;
static const unsigned int kMaxHashChainNum = 128;
#else   /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */
static const unsigned int kSpanRootLength = 512;
static const unsigned int kMaxNumAddressArray = 512 * 1024;
static const unsigned int kMemoryInfoRootLength = 2048;
#endif  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
static const unsigned int kMinStackTraceNum = 128 * 1024;
static const unsigned int kMinStackObjectNodeHashOpenNum = 128 * 1024;
static const unsigned int kMinStackTraceInfo = 1024;
static const unsigned int kMinStackObjectNode = 128 * 1024;
static const unsigned int kMinStackObjectStatisticsNode = 1024;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
static const unsigned int kMinStackObject = 128 * 1024;
#else   /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */
static const unsigned int kMaxNumStackObjectInfoArray = 256 * 1024;
static const unsigned int kMaxNumStackTraceNodeArray = kMaxNumAddressArray * 20;
static const unsigned int kMaxNumMemoryLeakInfoArray = 128 * 1024;
static const unsigned int kMaxNumMemoryLeakNodeArray = 128 * 1024;
static const unsigned int kMaxNumStackTraceInfoArray = kMaxNumMemoryLeakInfoArray * 32;
static const unsigned int kMaxNumStackObjectArray = kMaxNumMemoryLeakInfoArray * 32;
static const unsigned int kMaxNumStackObjectNodeArray = kMaxNumMemoryLeakInfoArray * 32;
#endif /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

#if ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE)
static const char* const cPrefixToRemove[] = {
    (const char*)"WTF::do_malloc",
    (const char*)"WTF::fastMalloc",
    (const char*)"WTF::fastCalloc",
    (const char*)"WTF::fastRealloc",
    (const char*)"WTF::fastZeroedMalloc",
    (const char*)"WTF::tryFastMalloc",
    (const char*)"WTF::tryFastCalloc",
    (const char*)"WTF::tryFastRealloc",
    (const char*)"WTF::tryFastZeroedMalloc",
    (const char*)"operator new"
};

static const char* const cTerminatorStr[] = {
    (const char*)"timerProc",
    (const char*)"Ordinal0",
    (const char*)"Ordinal1",
    (const char*)"Ordinal2",
    (const char*)"Ordinal3",
    (const char*)"Ordinal4",
    (const char*)"Ordinal5",
    (const char*)"Ordinal6",
    (const char*)"Ordinal7",
    (const char*)"Ordinal8",
    (const char*)"Ordinal9"
};

static const int cPrefixSize = (int)(sizeof(cPrefixToRemove) / sizeof(cPrefixToRemove[0]));
static const int cTerminatorSize = (int)(sizeof(cTerminatorStr) / sizeof(cTerminatorStr[0]));
#endif /* ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE) */

#endif  /* ENABLE(WKC_FASTMALLOC_STACK_TRACE) */

static const unsigned int kInitHashValue = 0x9e3779b9U;
static const unsigned int kQuickSortStackSize = 50;
static const unsigned int kSwitchSortMethodSize = 10;
static const int kFastMallocIndexNotFound = -1;

// Debug Print Configurations
static const bool cWriteMemoryLeak = false;
#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
static const bool cWriteStackTraceWithSymbol = false;
static const bool cWriteStackTraceWithCallstack = false;
#endif  /* ENABLE(WKC_FASTMALLOC_STACK_TRACE) */


// Structure's Definitions
struct TCListRoot_ {
    void* head;
    void* tail;
    unsigned int len;
};

typedef struct TCListRoot_ TCListRoot;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)

struct TCHashRoot_ {
    TCListRoot *listRoot;
    unsigned int len;
    unsigned int maxLen;
    unsigned int hashSize;
    unsigned int maxNum;
};

typedef struct TCHashRoot_ TCHashRoot;
#endif  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
typedef struct TCStackObject_ TCStackObject;

struct TCStackObject_ {
    StackObjectInfo obj;
    int refCount;
    int leakCount;
    int sumLeakSize;
    TCStackObject* next;
    TCStackObject* prev;
};

typedef ObjectNode TCStackTraceNode;
typedef ObjectNode TCStackObjectNode;
typedef ObjectNode TCStackObjectStatisticsNode;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)

typedef TCHashRoot TCStackObjectNodeHashRoot;

#else   /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

struct TCStackObjectNodeHashRoot_ {
    TCListRoot listRoot[kMinStackObjectNodeHashOpenNum];
    unsigned int len;
    unsigned int maxLen;
    unsigned int hashSize;
};

typedef struct TCStackObjectNodeHashRoot_ TCStackObjectNodeHashRoot;

#endif  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

#endif  /* ENABLE(WKC_FASTMALLOC_STACK_TRACE) */

typedef ObjectNode TCSpanNode;

struct TCFreeList_ {
    TCListRoot root;
    unsigned int num;
    void* ptr;
};

typedef struct TCFreeList_ TCFreeList;

typedef struct TCMemoryNode_ TCMemoryNode;

struct TCMemoryNode_ {
    UsedMemoryInfo info;
    TCMemoryNode* next;
    TCMemoryNode* prev;
    time_t curTime;
#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
    unsigned int traceLen;
    TCStackTraceNode* head;
    TCStackTraceNode* tail;
#endif
};

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)

typedef TCHashRoot TCMemoryInfoHashRoot;
typedef TCHashRoot TCSpanHashRoot;
typedef TCHashRoot TCMemoryLeakHashRoot;

#else   /* !ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

struct TCSpanHashRoot_ {
    TCListRoot listRoot[kSpanRootLength];
    unsigned int len;
    unsigned int maxLen;
    unsigned int hashSize;
};

typedef TCSpanHashRoot_ TCSpanHashRoot;

struct TCMemoryInfoHashRoot_ {
    TCListRoot listRoot[kMemoryInfoRootLength];
    unsigned int len;
    unsigned int maxLen;
    unsigned int hashSize;
};

typedef struct TCMemoryInfoHashRoot_ TCMemoryInfoHashRoot;

struct TCMemoryLeakHashRoot_ {
    TCListRoot listRoot[kMinMemoryLeakHashOpenNum];
    unsigned int len;
    unsigned int maxLen;
    unsigned int hashSize;
};

typedef struct TCMemoryLeakHashRoot_ TCMemoryLeakHashRoot;

#endif /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */


// Global Variables
static SpinLock gMemoryInfoLock = SPINLOCK_INITIALIZER;

static TCFreeList gFreeSpanNodeList;
static TCFreeList gFreeMemoryNodeList;
static TCFreeList gFreeMemoryLeakInfoList;
static TCFreeList gFreeMemoryLeakNodeList;

static TCSpanHashRoot gUsedSpanHash;
static TCMemoryInfoHashRoot gUsedMemoryHash;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
static bool gMemoryNodeInit = false;
static bool gSpanNodeInit = false;
static bool gAllocatedMemoryMap = false;
static bool gAllocatedMemoryMapByAllocator = false;
#else   /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */
static TCSpanNode gSpanNode[kMaxNumSpanInfoArray];
static TCMemoryNode gMemoryNode[kMaxNumAddressArray];
#endif  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)

static TCFreeList gFreeStackTraceNodeList;
static TCFreeList gFreeStackTraceInfoList;
static TCFreeList gFreeStackObjectList;
static TCFreeList gFreeStackObjectNodeList;
static TCFreeList gFreeStackObjectStatisticsNodeList;
static TCStackObjectNodeHashRoot gStackObjectNodeHash;
static TCListRoot gMemoryLeakStatistics;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)

static bool gStackTraceNodeInit = false;

#else   /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

static StackObjectInfo gStackObjectInfo[kMaxNumStackObjectInfoArray];
static TCStackTraceNode gStackTraceNode[kMaxNumStackTraceNodeArray];
static MemoryLeakInfo gMemoryLeakInfo[kMaxNumMemoryLeakInfoArray];
static MemoryLeakNode gMemoryLeakNode[kMaxNumMemoryLeakNodeArray];
static StackTraceInfo gStackTraceInfo[kMaxNumStackTraceInfoArray];
static TCStackObject gStackObject[kMaxNumStackObjectArray];
static TCStackObjectNode gStackObjectNode[kMaxNumStackObjectNodeArray];
static TCStackObjectNode gStaciObjectStatisticsNode[kMaxNumStackObjectNodeArray];

#endif  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

#if ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE)
static HANDLE gCurProcess = 0;
static BYTE gBuffer1[sizeof(IMAGEHLP_SYMBOL) + kMaxNameLength + 1];
static BYTE gBuffer2[sizeof(IMAGEHLP_SYMBOL) + kMaxNameLength + 1];
#endif /* ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE) */

#endif  /*  ENABLE(WKC_FASTMALLOC_STACK_TRACE) */

static MemoryLeakDumpProc gMemoryLeakDumpProc = NULL;
static void* gMemoryLeakDumpCtx = NULL;

#if COMPILER(MSVC) && defined(_DEBUG)
static unsigned int gStackCounter = 0;
#endif


// Macros & prototypes
#define initSpanNode(node) initObject(node)
#define appendFreeSpanNode(node) appendFreeNode(&gFreeSpanNodeList, node)

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
#define growFreeSpanNodeList(requestSize) growFreeList(&gFreeSpanNodeList, FASTMALLOCDEBUG_MIN(requestSize, kMinAddressArrayNum))
#define findUsedSpanNode(span) ((TCSpanNode*)findNode(&gUsedSpanHash, (void*)span))
#define findFreeSpanNode() findFreeNode(&gFreeSpanNodeList, kMinAddressArrayNum)
#define setUsedSpanNode(node) setNode(&gUsedSpanHash, node)

#else   /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

#define initStackTraceInfo(info) initObjectNode(info)
#define initStackTraceNode(node) initObjectNode(node)
#define initStackObjectNode(node) initObjectNode(node)
#define findFreeSpanNode() findFreeNode(&gFreeSpanNodeList, 0/* no meaning this arg */)

#endif  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
#define growFreeStackTraceNodeList() growFreeList(&gFreeStackTraceNodeList, kMinStackTraceNum)
#define findFreeStackTraceNode() findFreeNode(&gFreeStackTraceNodeList, kMinStackTraceNum)
#define appendFreeStackTraceNode(node) appendFreeNode(&gFreeStackTraceNodeList, node)

#define findFreeStackTraceInfo() ((StackTraceInfo*)findFreeNode(&gFreeStackTraceInfoList, kMinStackTraceInfo))
#define appendFreeStackTraceInfo(info) appendFreeNode(&gFreeStackTraceInfoList, (ObjectNode*)info)

#define findFreeStackObjectNode() ((TCStackObjectNode*)findFreeNode(&gFreeStackObjectNodeList, kMinStackObjectNode))
#define findFreeStackObjectStatisticsNode() ((TCStackObjectStatisticsNode*)findFreeNode(&gFreeStackObjectStatisticsNodeList, kMinStackObjectStatisticsNode))

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
COMPILE_ASSERT(sizeof(StackTraceInfo) == sizeof(ObjectNode), sizeof__StackTraceInfo);

#define growFreeStackTraceInfoList() growFreeList(&gFreeStackTraceInfoList, kMinStackTraceInfo)

#define growFreeStackObjectNodeList() growFreeList(&gFreeStackObjectNodeList, kMinStackObjectNode)
#define growFreeStackObjectStatisticsNodeList() growFreeList(&gFreeStackObjectStatisticsNodeList, kMinStackObjectStatisticsNode)
#endif /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

#endif  /* ENABLE(WKC_FASTMALLOC_STACK_TRACE) */


// APIs
static inline unsigned int computeHash(unsigned char* in_str, unsigned int in_len, unsigned int init_value = kInitHashValue)
{
    // This hash is designed to work on 16-bit chunks at a time. But since the normal case
    // (above) is to hash UTF-16 characters, we just treat the 8-bit chars as if they
    // were 16-bit chunks, which should give matching results

    unsigned hash = init_value;
    unsigned char* data = in_str;
    unsigned char* end = in_str + in_len;
 
    // Main loop
    for (;;) {
        if (data == end)
            break;

        unsigned char b0 = *data++;

        if (data == end) {
            hash += b0;
            hash ^= hash << 11;
            hash += hash >> 17;
            break;
        }

        unsigned char b1 = *data++;

        hash += b0;
        unsigned tmp = (b1 << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        hash += hash >> 11;
    }
    
    // Force "avalanching" of final 127 bits.
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 2;
    hash += hash >> 15;
    hash ^= hash << 10;

    // This avoids ever returning a hash code of 0, since that is used to
    // signal "hash not computed yet", using a value that is likely to be
    // effectively the same as 0 when the low bits are masked.
    hash |= !hash << 31;

    return hash;
}

static void initObject(ObjectNode& node)
{
    node.obj = NULL;
    node.next = NULL;
    node.prev = NULL;
}

static void initListRoot(TCListRoot& root)
{
    root.head = NULL;
    root.tail = NULL;
    root.len = 0;
}

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
static bool growFreeList(TCFreeList* freeList, unsigned int growNum)
{
    ASSERT(growNum > 0);
    TCListRoot* listRoot = &freeList->root;
    ObjectNode* node = (ObjectNode*)fastSystemMalloc(sizeof(ObjectNode) * growNum);
    ASSERT(node != NULL);

    for (unsigned int i = 0; i < growNum; i++)
        initObject(node[i]);

    for (unsigned int i = 0; i < growNum; i++) {
        if (i > 0) {
            node[i].prev = &node[i - 1];
        } else {
            node[i].prev = NULL;
        }

        if (i < growNum - 1) {
            node[i].next = &node[i + 1];
        } else {
            node[i].next = NULL;
        }
    }

    listRoot->head = &node[0];
    listRoot->tail = &node[growNum - 1];
    listRoot->len = growNum;

    unsigned int arrayNum = freeList->num;
    ObjectNode** node_array = (ObjectNode**)freeList->ptr;
    ObjectNode** node_array_new = (ObjectNode**)fastSystemRealloc(node_array, sizeof(ObjectNode*) * (arrayNum + 1));
    ASSERT(node_array_new != NULL);
    node_array_new[arrayNum++] = node;
    freeList->ptr = node_array_new;
    freeList->num = arrayNum;

    return true;
}

static void setNode(TCHashRoot* hashRoot, ObjectNode* node)
{
    const unsigned int hashSize = hashRoot->hashSize;
    unsigned int hash = computeHash((unsigned char*)&node->obj, sizeof(void*));
    hash &= hashSize - 1;

    TCListRoot* listRoot = &hashRoot->listRoot[hash];
    if (listRoot->head) {
        // prepend
        ObjectNode* prevHead = (ObjectNode*)listRoot->head;
        prevHead->prev = node;
        node->next = prevHead;
        node->prev = NULL;

        listRoot->head = node;
    } else {
        listRoot->head = listRoot->tail = node;
    }

    listRoot->len++;

    hashRoot->len++;

    hashRoot->maxLen = FASTMALLOCDEBUG_MAX(hashRoot->len, hashRoot->maxLen);
}

static ObjectNode* findNode(TCHashRoot* hashRoot, void* ptr, bool release = true)
{
    ObjectNode* node = NULL;
    const unsigned int hashSize = hashRoot->hashSize;
    unsigned int hash = computeHash((unsigned char*)&ptr, sizeof(void*));
    hash &= hashSize - 1;

    TCListRoot* listRoot = &hashRoot->listRoot[hash];

    for (ObjectNode* temp = (ObjectNode*)listRoot->head; temp != NULL; temp = temp->next) {
        if (temp->obj == ptr) {
            if (release) {
                if (temp->next) {
                    temp->next->prev = temp->prev;
                } else {
                    listRoot->tail = temp->prev;
                }

                if (temp->prev) {
                    temp->prev->next = temp->next;
                } else {
                    listRoot->head = temp->next;
                }

                temp->next = temp->prev = NULL;

                listRoot->len--;

                hashRoot->len--;
            }

            node = temp;
            break;
        }
    }

    return node;
}
#endif /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

static ObjectNode* findFreeNode(TCFreeList* freeList, unsigned int num)
{
    TCListRoot* listRoot = &freeList->root;
    ObjectNode* node = (ObjectNode*)listRoot->head;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    if (node == NULL) {
        growFreeList(freeList, num);
        node = (ObjectNode*)listRoot->head;
    }
#endif

    ASSERT(node != NULL);

    if (node->next != NULL)
        node->next->prev = NULL;
    else
        listRoot->tail = NULL;

    listRoot->head = node->next;

    listRoot->len--;

    initObject(*node);

    return node;
}

static void appendFreeNode(TCFreeList* freeList, ObjectNode* node)
{
    if (node == NULL)
        return;

    initObject(*node);

    TCListRoot* listRoot = &freeList->root;

    ObjectNode* prevTail = (ObjectNode*)listRoot->tail;

    if (prevTail != NULL)
        prevTail->next = node;

    node->prev = prevTail;

    listRoot->tail = node;

    listRoot->len++;
}

static void initUsedMemoryInfo(UsedMemoryInfo& info)
{
    info.adr = NULL;
    info.requestSize = 0;
    info.usedSize = 0;
    info.classID = 0;
}

static void initMemoryNode(TCMemoryNode& node)
{
    initUsedMemoryInfo(node.info);
    node.next = NULL;
    node.prev = NULL;
#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
    memset(&node.curTime, 0, sizeof(node.curTime));
    node.traceLen = 0;
    node.head = NULL;
    node.tail = NULL;
#endif
}

static void initSpanInfo(SpanInfo& spanInfo)
{
    spanInfo.span = NULL;
    spanInfo.head = NULL;
    spanInfo.tail = NULL;
    spanInfo.nextAddress = NULL;
    spanInfo.used = false;
    spanInfo.usedBlocks = 0;
    spanInfo.maxBlocks = 0;
    spanInfo.pages = 0;
    spanInfo.classID = 0;
    spanInfo.blockSize = 0;
    spanInfo.size = 0;
    spanInfo.requestedSize = 0;
    spanInfo.numUsedMemArray = 0;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    if (gAllocatedMemoryMapByAllocator) {
        const unsigned int array_num = FASTMALLOCDEBUG_HOWMANY(kPageSize, kAlignment);

        for (unsigned int i = 0; i < array_num; i++) {
            initUsedMemoryInfo(spanInfo.usedMemArray[i]);
            spanInfo.usedMemPtrArray[i] = NULL;
        }
    } else {
        spanInfo.usedMemArray = NULL;
        spanInfo.usedMemPtrArray = NULL;
    }
#else
    const unsigned int size = sizeof(spanInfo.usedMemArray) / sizeof(spanInfo.usedMemArray[0]);
    for (unsigned int i = 0; i < size; i++) {
        initUsedMemoryInfo(spanInfo.usedMemArray[i]);
        spanInfo.usedMemPtrArray[i] = NULL;
    }
#endif
}

static void initMemoryInfo(MemoryInfo& memInfo)
{
#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    unsigned int size = 0;

    if (!gAllocatedMemoryMapByAllocator) {
        size = gUsedSpanHash.len;
    } else {
        size = gUsedSpanHash.maxNum;
    }

    if (!gAllocatedMemoryMap) {
        memInfo.spanArray = (SpanInfo*)fastSystemMalloc(sizeof(SpanInfo) * size);
        ASSERT(memInfo.spanArray != NULL);
        memInfo.spanPtrArray = (SpanInfo**)fastSystemMalloc(sizeof(SpanInfo*) * size);
        ASSERT(memInfo.spanPtrArray != NULL);

        if (gAllocatedMemoryMapByAllocator) {
            const unsigned int array_num = FASTMALLOCDEBUG_HOWMANY(kPageSize, kAlignment);

            for (unsigned int i = 0; i < size; i++) {
                SpanInfo* spanInfo = &memInfo.spanArray[i];
                spanInfo->usedMemArray = (UsedMemoryInfo*)fastSystemMalloc(sizeof(UsedMemoryInfo) * array_num);
                ASSERT(spanInfo->usedMemArray != NULL);
                spanInfo->usedMemPtrArray = (UsedMemoryInfo**)fastSystemMalloc(sizeof(UsedMemoryInfo*) * array_num);
                ASSERT(spanInfo->usedMemPtrArray != NULL);
            }
        }

        gAllocatedMemoryMap = true;
    }
#else
    const unsigned int size = sizeof(memInfo.spanArray) / sizeof(memInfo.spanArray[0]);
#endif

    memInfo.pageSize = 0;
    memInfo.numSpanArray = 0;

    for (unsigned int i = 0; i < size; i++) {
        initSpanInfo(memInfo.spanArray[i]);
        memInfo.spanPtrArray[i] = NULL;
    }
}

static void swapSpanInfo(void* inoutSpanInfo1, unsigned int index1, void* inoutSpanInfo2, unsigned int index2)
{
    SpanInfo** spanInfo1 = (SpanInfo**)inoutSpanInfo1;
    SpanInfo** spanInfo2 = (SpanInfo**)inoutSpanInfo2;
    SpanInfo** info1 = &spanInfo1[index1];
    SpanInfo** info2 = &spanInfo2[index2];
    SpanInfo* temp;

    temp = *info1;
    *info1 = *info2;
    *info2 = temp;
}

static bool compSpanInfo(void* inoutSpanInfo1, unsigned int index1, void* inoutSpanInfo2, unsigned int index2)
{
    SpanInfo** spanInfo1 = (SpanInfo**)inoutSpanInfo1;
    SpanInfo** spanInfo2 = (SpanInfo**)inoutSpanInfo2;
    SpanInfo* info1 = spanInfo1[index1];
    SpanInfo* info2 = spanInfo2[index2];

    return (info1->head < info2->head) ? true : false;
}

static void swapUsedMemoryInfo(void* inoutUsedMemoryInfo1, unsigned int index1, void* inoutUsedMemoryInfo2, unsigned int index2)
{
    UsedMemoryInfo** usedMemoryInfo1 = (UsedMemoryInfo**)inoutUsedMemoryInfo1;
    UsedMemoryInfo** usedMemoryInfo2 = (UsedMemoryInfo**)inoutUsedMemoryInfo2;
    UsedMemoryInfo** info1 = &usedMemoryInfo1[index1];
    UsedMemoryInfo** info2 = &usedMemoryInfo2[index2];
    UsedMemoryInfo* temp;

    temp = *info1;
    *info1 = *info2;
    *info2 = temp;
}

static bool compUsedMemoryInfo(void* inoutUsedMemoryInfo1, unsigned int index1, void* inoutUsedMemoryInfo2, unsigned index2)
{
    UsedMemoryInfo** usedMemoryInfo1 = (UsedMemoryInfo**)inoutUsedMemoryInfo1;
    UsedMemoryInfo** usedMemoryInfo2 = (UsedMemoryInfo**)inoutUsedMemoryInfo2;
    UsedMemoryInfo* info1 = usedMemoryInfo1[index1];
    UsedMemoryInfo* info2 = usedMemoryInfo2[index2];

    return (info1->adr < info2->adr) ? true : false;
}

static void InsertionSort(void* inout_item, unsigned int in_left, unsigned int in_right, 
                            bool (*compProc)(void*, unsigned int, void*, unsigned int),
                            void (*swapProc)(void*, unsigned int, void*, unsigned int))
{
    int items = in_right - in_left + 1;
    if (items <= 1)
        return;

    for (unsigned int i = in_left + 1; i <= in_right; i++) {
        for (unsigned int j = i; j > in_left; j--) {
            if (!(*compProc)(inout_item, j, inout_item, j - 1))
                break;

            (*swapProc)(inout_item, j - 1, inout_item, j);
        }
    }
}

static void QuickSort(void* inout_item, unsigned int in_left, unsigned int in_right,
                        bool (*compProc)(void*, unsigned int, void*, unsigned int),
                        void (*swapProc)(void*, unsigned int, void*, unsigned int))
{
    int left;
    int right;
    int pivot_pos;
    int *lindex = NULL;
    int *rindex = NULL;
#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    int num = 0;
#else
    int lsstack[kQuickSortStackSize];
    int rsstack[kQuickSortStackSize];
#endif
    int stack_pos = 0;
    int pl;
    int pr;
    int items = in_right - in_left + 1;

    if (items <= 1 || !compProc || !swapProc) {
        return;
    } else if (items <= kSwitchSortMethodSize) {
        InsertionSort(inout_item, in_left, in_right, compProc, swapProc);
        return;
    }

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    lindex = (int*)fastSystemMalloc(sizeof(int) * kQuickSortStackSize);
    ASSERT(lindex != NULL);
    rindex = (int*)fastSystemMalloc(sizeof(int) * kQuickSortStackSize);
    ASSERT(rindex != NULL);
    num++;
#else
    lindex = &lsstack[0];
    rindex = &rsstack[0];
#endif

    lindex[stack_pos] = in_left;
    rindex[stack_pos] = in_right;

    stack_pos++;

    while (stack_pos-- > 0) {
        pl = left = lindex[stack_pos];
        pr = right = rindex[stack_pos];

        /* re-use "items" */
        items = right - left + 1;

        pivot_pos = left + items / 2;

        if ((*compProc)(inout_item, pivot_pos, inout_item, pl))
            (*swapProc)(inout_item, pl, inout_item, pivot_pos);

        if ((*compProc)(inout_item, pr, inout_item, pivot_pos))
            (*swapProc)(inout_item, pr, inout_item, pivot_pos);

        if ((*compProc)(inout_item, pivot_pos, inout_item, pl))
            (*swapProc)(inout_item, pl, inout_item, pivot_pos);

        while (pl <= pr) {
            while (pl <= right) {
                if (!(*compProc)(inout_item, pl, inout_item, pivot_pos))
                    break;

                pl++;
            }

            while (pr >= left) {
                if (!(*compProc)(inout_item, pivot_pos, inout_item, pr))
                    break;

                pr--;
            }

            if (pl <= pr) {
                if (pl != pr)
                    (*swapProc)(inout_item, pl, inout_item, pr);

                if (pivot_pos == pl)
                    pivot_pos = pr;
                else if (pivot_pos == pr)
                    pivot_pos = pl;

                pl++;
                pr--;
            }
        }

        if (left < pr) {
            items = pr - left + 1;
            if (items <= kSwitchSortMethodSize) {
                InsertionSort(inout_item, left, pr, compProc, swapProc);
            } else {
#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
                if (stack_pos >= (kQuickSortStackSize * num)) {
                    int* temp = (int*)fastSystemRealloc(lindex, (sizeof(int) * (num + 1)));
                    ASSERT(temp != NULL);
                    lindex = temp;

                    temp = (int*)fastSystemRealloc(rindex, (sizeof(int) * (num + 1)));
                    ASSERT(temp != NULL);
                    rindex = temp;

                    num++;
                }
#endif /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

                lindex[stack_pos] = left;
                rindex[stack_pos] = pr;
                stack_pos++;
#if COMPILER(MSVC) && defined(_DEBUG)
                gStackCounter = FASTMALLOCDEBUG_MAX(gStackCounter, stack_pos);
#endif
#if !ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
                ASSERT(stack_pos < kQuickSortStackSize);
#endif
            }
        }

        if (pl < right) {
            items = right - pl + 1;
            if (items <= kSwitchSortMethodSize) {
                InsertionSort(inout_item, pl, right, compProc, swapProc);
            } else {
#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
                if (stack_pos >= (kQuickSortStackSize * num)) {
                    int* temp = (int*)fastSystemRealloc(lindex, (sizeof(int) * (num + 1)));
                    ASSERT(temp != NULL);
                    lindex = temp;

                    temp = (int*)fastSystemRealloc(rindex, (sizeof(int) * (num + 1)));
                    ASSERT(temp != NULL);
                    rindex = temp;

                    num++;
                }
#endif /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

                lindex[stack_pos] = pl;
                rindex[stack_pos] = right;
                stack_pos++;
#if COMPILER(MSVC) && defined(_DEBUG)
                gStackCounter = FASTMALLOCDEBUG_MAX(gStackCounter, stack_pos);
#endif
#if !ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
                ASSERT(stack_pos < kQuickSortStackSize);
#endif
            }
        }
    }

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    if (lindex != NULL)
        fastSystemFree(lindex);
    if (rindex != NULL)
        fastSystemFree(rindex);
#endif
}

static void sortUsedMemoryInfo(MemoryInfo& memInfo)
{
    SpanInfo* spanInfo;
    const unsigned int span_num = memInfo.numSpanArray;

    for (unsigned int i = 0; i < span_num; i++) {
        spanInfo = &memInfo.spanArray[i];
        const unsigned int mem_num = spanInfo->numUsedMemArray;

        if (mem_num > 0) {
#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
            if (!gAllocatedMemoryMapByAllocator) {
                spanInfo->usedMemPtrArray = (UsedMemoryInfo**)fastSystemMalloc(sizeof(UsedMemoryInfo*) * mem_num);
                ASSERT(spanInfo->usedMemPtrArray != NULL);
            }
#endif

            for (unsigned int j = 0; j < mem_num; j++)
                spanInfo->usedMemPtrArray[j] = &spanInfo->usedMemArray[j];

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
            QuickSort(spanInfo->usedMemPtrArray, 0, mem_num - 1, compUsedMemoryInfo, swapUsedMemoryInfo);
#else
            QuickSort(&spanInfo->usedMemPtrArray, 0, mem_num - 1, compUsedMemoryInfo, swapUsedMemoryInfo);
#endif
        } else {
#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
            if (!gAllocatedMemoryMapByAllocator) {
                spanInfo->usedMemPtrArray = NULL;
            }
#endif
        }

#if ENABLE(WKC_VALIDATE_SORT_ALGORITHM)
        void* prevptr = NULL;

        for (unsigned int j = 0; j < mem_num; j++) {
            void* ptr = spanInfo->usedMemPtrArray[j]->adr;
            ASSERT(prevptr <= ptr);
            prevptr = ptr;
        }
#endif  /* ENABLE(WKC_VALIDATE_SORT_ALGORITHM) */
    }
}

static void sortSpanInfo(MemoryInfo& memInfo)
{
    unsigned int num = memInfo.numSpanArray;

    for (unsigned int i = 0; i < num; i++)
        memInfo.spanPtrArray[i] = &memInfo.spanArray[i];

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    QuickSort(memInfo.spanPtrArray, 0, num - 1, compSpanInfo, swapSpanInfo);
#else
    QuickSort(&memInfo.spanPtrArray, 0, num - 1, compSpanInfo, swapSpanInfo);
#endif

#if ENABLE(WKC_VALIDATE_SORT_ALGORITHM)
    void* prevptr = NULL;

    for (unsigned int i = 0; i < num; i++) {
        void* ptr = memInfo.spanPtrArray[i]->head;
        ASSERT(prevptr <= ptr);
        prevptr = ptr;
    }
#endif /* ENABLE(WKC_VALIDATE_SORT_ALGORITHM) */
}

static void GetUsedSpanInfo(MemoryInfo& memInfo)
{
    unsigned int num = 0;

    memInfo.pageSize = kPageSize;

    const unsigned int hashSize = gUsedSpanHash.hashSize;

    for (unsigned int i = 0; i < hashSize; i++) {
        TCListRoot* root = &gUsedSpanHash.listRoot[i];

        for (TCSpanNode* node = (TCSpanNode*)root->head; node != NULL; node = node->next) {
            void* span = (void*)node->obj;
            ASSERT(span != NULL);

            SpanInfo* spanInfo = &memInfo.spanArray[num++];
            ASSERT(spanInfo != NULL);

            spanInfo->span = span;
#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
            ASSERT(num <= gUsedSpanHash.len);
#endif
            fastMallocGetSpanInfo(spanInfo->span, *spanInfo);
        }
    }

    memInfo.numSpanArray = num;

    sortSpanInfo(memInfo);
}

static int findSpanInfo(MemoryInfo& inMemInfo, UsedMemoryInfo& inUsedMemInfo, unsigned int cl)
{
    unsigned int num = inMemInfo.numSpanArray;
    int index = kFastMallocIndexNotFound;
    SpanInfo* spanInfo;
    unsigned int left = 0;
    unsigned int right = num - 1;
    unsigned int nextIndex = 0;
    void* ptr = inUsedMemInfo.adr;
    void* head;
    void* tail;

    for (;;) {
        nextIndex = (left + right) / 2;
        spanInfo = inMemInfo.spanPtrArray[nextIndex];

        head = spanInfo->head;
        tail = spanInfo->tail;

        if (head <= ptr && ptr <= tail) {
            if (spanInfo->classID == cl)
                index = nextIndex;  // Found
            break;
        } else {
            if ((nextIndex == left) && (nextIndex == right))
                break;  // Not Found

            if (ptr < head) {
                if (right == nextIndex) {
                    right = left;
                } else {
                    right = nextIndex;
                }
            } else if (ptr > tail) {
                if (left == nextIndex) {
                    left = right;
                } else {
                    left = nextIndex;
                }
            }
        }
    }

    ASSERT(index >= 0);

    return index;
}

static void attachAddressInfo(MemoryInfo& inMemInfo, UsedMemoryInfo& inUsedMemInfo, unsigned int cl, bool needUsedMemory)
{
    const int index = findSpanInfo(inMemInfo, inUsedMemInfo, cl);
    ASSERT(index >= 0 && index < (int)inMemInfo.numSpanArray);
    SpanInfo* spanInfo = inMemInfo.spanPtrArray[index];
    spanInfo->requestedSize += inUsedMemInfo.requestSize;
    if (needUsedMemory) {
        unsigned int num = spanInfo->numUsedMemArray;
#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
        if (!gAllocatedMemoryMapByAllocator) {
            UsedMemoryInfo* temp;
            if (spanInfo->usedMemArray != NULL) {
                temp = (UsedMemoryInfo*)fastSystemRealloc(spanInfo->usedMemArray, sizeof(UsedMemoryInfo) * (num + 1));
            } else {
                temp = (UsedMemoryInfo*)fastSystemMalloc(sizeof(UsedMemoryInfo));
            }
            ASSERT(temp != NULL);
            spanInfo->usedMemArray = temp;
            initUsedMemoryInfo(spanInfo->usedMemArray[num]);
        }
#endif
        spanInfo->usedMemArray[num++] = inUsedMemInfo;
        spanInfo->numUsedMemArray = num;
        ASSERT(spanInfo->numUsedMemArray <= kMaxNumUsedMemArray);
    }
}

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
static bool growFreeMemoryNodeList(unsigned int requestNum = 0)
{
    TCListRoot* listRoot = &gFreeMemoryNodeList.root;
    const unsigned int node_num = (requestNum > 0) ? FASTMALLOCDEBUG_MIN(requestNum, kMinAddressArrayNum) : kMinAddressArrayNum;

    // For Used Memory Hash
    TCMemoryNode* mem_node = (TCMemoryNode*)fastSystemMalloc(sizeof(TCMemoryNode) * node_num);
    ASSERT(mem_node != NULL);

    for (unsigned int i = 0; i < node_num; i++)
        initMemoryNode(mem_node[i]);

    for (unsigned int i = 0; i < node_num; i++) {
        if (i > 0) {
            mem_node[i].prev = &mem_node[i - 1];
        } else {
            mem_node[i].prev = NULL;
        }

        if (i < node_num - 1) {
            mem_node[i].next = &mem_node[i + 1];
        } else {
            mem_node[i].next = NULL;
        }
    }

    listRoot->head = &mem_node[0];
    listRoot->tail = &mem_node[node_num - 1];
    listRoot->len = node_num;

    unsigned int num = gFreeMemoryNodeList.num;
    TCMemoryNode** node_array = (TCMemoryNode**)gFreeMemoryNodeList.ptr;
    TCMemoryNode** node_array_new = (TCMemoryNode**)fastSystemRealloc(node_array, sizeof(TCMemoryNode*) * (num + 1));
    ASSERT(node_array_new != NULL);
    node_array_new[num++] = mem_node;
    gFreeMemoryNodeList.ptr = node_array_new;
    gFreeMemoryNodeList.num = num;

    return true;
}
#endif /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

static TCMemoryNode* findFreeMemoryNode()
{
    TCListRoot* listRoot = &gFreeMemoryNodeList.root;
    TCMemoryNode* node = (TCMemoryNode*)listRoot->head;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    if (node == NULL) {
        growFreeMemoryNodeList(kMinAddressArrayNum);
        node = (TCMemoryNode*)listRoot->head;
    }
#endif

    ASSERT(node != NULL);

    if (node->next != NULL)
        node->next->prev = NULL;
    else
        listRoot->tail = NULL;

    listRoot->head = node->next;

    listRoot->len--;

    initMemoryNode(*node);

    return node;
}

static void appendFreeMemoryNode(TCMemoryNode* node)
{
    if (node == NULL)
        return;

    initMemoryNode(*node);

    TCListRoot* listRoot = &gFreeMemoryNodeList.root;

    TCMemoryNode* prevTail = (TCMemoryNode*)listRoot->tail;

    if (prevTail != NULL)
        prevTail->next = node;

    node->prev = prevTail;

    listRoot->tail = node;

    listRoot->len++;
}


static void setUsedMemoryNode(TCMemoryNode* node)
{
    const unsigned int hashSize = gUsedMemoryHash.hashSize;
    unsigned int hash = computeHash((unsigned char*)&node->info.adr, sizeof(void*));
    hash &= hashSize - 1;

    TCListRoot* uMemMap = &gUsedMemoryHash.listRoot[hash];
    if (uMemMap->head) {
        // prepend
        TCMemoryNode* prevHead = (TCMemoryNode*)uMemMap->head;
        prevHead->prev = node;
        node->next = prevHead;
        node->prev = NULL;

        uMemMap->head = node;
    } else {
        uMemMap->head = uMemMap->tail = node;
    }

    uMemMap->len++;

    gUsedMemoryHash.len++;

    gUsedMemoryHash.maxLen = FASTMALLOCDEBUG_MAX(gUsedMemoryHash.len, gUsedMemoryHash.maxLen);
}

static TCMemoryNode* findUsedMemoryNode(void* ptr)
{
    TCMemoryNode* node = NULL;
    const unsigned int hashSize = gUsedMemoryHash.hashSize;
    unsigned int hash = computeHash((unsigned char*)&ptr, sizeof(void*));
    hash &= hashSize - 1;

    TCListRoot* uMemMap = &gUsedMemoryHash.listRoot[hash];

    for (TCMemoryNode* temp = (TCMemoryNode*)uMemMap->head; temp != NULL; temp = temp->next) {
        if (temp->info.adr == ptr) {
            if (temp->next) {
                temp->next->prev = temp->prev;
            } else {
                uMemMap->tail = temp->prev;
            }

            if (temp->prev) {
                temp->prev->next = temp->next;
            } else {
                uMemMap->head = temp->next;
            }

            temp->next = temp->prev = NULL;

            node = temp;

            uMemMap->len--;

            gUsedMemoryHash.len--;
            break;
        }
    }

    return node;
}

static void initMemoryLeakInfo(MemoryLeakInfo& info)
{
    info.adr = NULL;
    info.size = 0;
    info.reqSize = 0;
    memset(&info.curTime, 0, sizeof(info.curTime));
    info.next = NULL;
    info.prev = NULL;
}

static void initMemoryLeakNode(MemoryLeakNode& node)
{
    node.numInfo = 0;
    node.memHead = NULL;
    node.memTail = NULL;

    node.numTrace = 0;
    node.stHead = NULL;
    node.stTail = NULL;

    node.next = NULL;
    node.prev = NULL;
}

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
static bool growFreeMemoryLeakInfoList()
{
    TCListRoot* listRoot = &gFreeMemoryLeakInfoList.root;
    const unsigned int info_num = kMinMemoryLeakInfo;

    // For Used Memory Hash
    MemoryLeakInfo* info = (MemoryLeakInfo*)fastSystemMalloc(sizeof(MemoryLeakInfo) * info_num);
    ASSERT(info != NULL);

    for (unsigned int i = 0; i < info_num; i++)
        initMemoryLeakInfo(info[i]);

    for (unsigned int i = 0; i < info_num; i++) {
        if (i > 0) {
            info[i].prev = &info[i - 1];
        } else {
            info[i].prev = NULL;
        }

        if (i < info_num - 1) {
            info[i].next = &info[i + 1];
        } else {
            info[i].next = NULL;
        }
    }

    listRoot->head = &info[0];
    listRoot->tail = &info[info_num - 1];
    listRoot->len = info_num;

    unsigned int num = gFreeMemoryLeakInfoList.num;
    MemoryLeakInfo** info_array = (MemoryLeakInfo**)gFreeMemoryLeakInfoList.ptr;
    MemoryLeakInfo** info_array_new = (MemoryLeakInfo**)fastSystemRealloc(info_array, sizeof(MemoryLeakInfo*) * (num + 1));
    ASSERT(info_array_new != NULL);
    info_array_new[num++] = info;
    gFreeMemoryLeakInfoList.ptr = info_array_new;
    gFreeMemoryLeakInfoList.num = num;

    return true;
}

static bool growFreeMemoryLeakNodeList()
{
    TCListRoot* listRoot = &gFreeMemoryLeakNodeList.root;
    const unsigned int node_num = kMinMemoryLeakNode;

    // For Used Memory Hash
    MemoryLeakNode* node = (MemoryLeakNode*)fastSystemMalloc(sizeof(MemoryLeakNode) * node_num);
    ASSERT(node != NULL);

    for (unsigned int i = 0; i < node_num; i++)
        initMemoryLeakNode(node[i]);

    for (unsigned int i = 0; i < node_num; i++) {
        if (i > 0) {
            node[i].prev = &node[i - 1];
        } else {
            node[i].prev = NULL;
        }

        if (i < node_num - 1) {
            node[i].next = &node[i + 1];
        } else {
            node[i].next = NULL;
        }
    }

    listRoot->head = &node[0];
    listRoot->tail = &node[node_num - 1];
    listRoot->len = node_num;

    unsigned int num = gFreeMemoryLeakNodeList.num;
    MemoryLeakNode** node_array = (MemoryLeakNode**)gFreeMemoryLeakNodeList.ptr;
    MemoryLeakNode** node_array_new = (MemoryLeakNode**)fastSystemRealloc(node_array, sizeof(MemoryLeakNode*) * (num + 1));
    ASSERT(node_array_new != NULL);
    node_array_new[num++] = node;
    gFreeMemoryLeakNodeList.ptr = node_array_new;
    gFreeMemoryLeakNodeList.num = num;

    return true;
}

#else   /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

static void initMemoryLeakHashRoot(TCMemoryLeakHashRoot& hashRoot)
{
    int size = sizeof(hashRoot.listRoot) / sizeof(hashRoot.listRoot[0]);

    for (int i = 0; i < size; i++)
        initListRoot(hashRoot.listRoot[i]);

    hashRoot.len = 0;
    hashRoot.maxLen = 0;
}
#endif /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

static MemoryLeakInfo* findFreeMemoryLeakInfo()
{
    TCListRoot* listRoot = &gFreeMemoryLeakInfoList.root;
    MemoryLeakInfo* info = (MemoryLeakInfo*)listRoot->head;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    if (info == NULL) {
        growFreeMemoryLeakInfoList();
        info = (MemoryLeakInfo*)listRoot->head;
    }
#endif

    ASSERT(info != NULL);

    if (info->next != NULL)
        info->next->prev = NULL;
    else
        listRoot->tail = NULL;

    listRoot->head = info->next;

    listRoot->len--;

    initMemoryLeakInfo(*info);

    return info;
}

static void appendFreeMemoryLeakInfo(MemoryLeakInfo* info)
{
    if (info == NULL)
        return;

    initMemoryLeakInfo(*info);

    TCListRoot* listRoot = &gFreeMemoryLeakInfoList.root;

    MemoryLeakInfo* prevTail = (MemoryLeakInfo*)listRoot->tail;

    if (prevTail != NULL)
        prevTail->next = info;

    info->prev = prevTail;

    listRoot->tail = info;

    listRoot->len++;
}

static MemoryLeakNode* findFreeMemoryLeakNode()
{
    TCListRoot* listRoot = &gFreeMemoryLeakNodeList.root;
    MemoryLeakNode* node = (MemoryLeakNode*)listRoot->head;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    if (node == NULL) {
        growFreeMemoryLeakNodeList();
        node = (MemoryLeakNode*)listRoot->head;
    }
#endif

    ASSERT(node != NULL);

    if (node->next != NULL)
        node->next->prev = NULL;
    else
        listRoot->tail = NULL;

    listRoot->head = node->next;

    listRoot->len--;

    initMemoryLeakNode(*node);

    return node;
}

static void appendFreeMemoryLeakNode(MemoryLeakNode* node)
{
    if (node == NULL)
        return;

    initMemoryLeakNode(*node);

    TCListRoot* listRoot = &gFreeMemoryLeakNodeList.root;

    MemoryLeakNode* prevTail = (MemoryLeakNode*)listRoot->tail;

    if (prevTail != NULL)
        prevTail->next = node;

    node->prev = prevTail;

    listRoot->tail = node;

    listRoot->len++;
}

static void insertMemoryLeakInfo(MemoryLeakNode* node, MemoryLeakInfo* info)
{
    ASSERT(node != NULL && info != NULL);

    bool inserted = false;

    for (MemoryLeakInfo* leakInfo = node->memHead; leakInfo != NULL; leakInfo = leakInfo->next) {
        if ((leakInfo->curTime > info->curTime)
                || ((leakInfo->curTime == info->curTime)
                && (leakInfo->reqSize < info->reqSize))) {
            if (leakInfo->prev != NULL) {
                // Insert
                leakInfo->prev->next = info;
                info->prev = leakInfo->prev;
                leakInfo->prev = info;
                info->next = leakInfo;                
            } else {
                // Prepend
                leakInfo->prev = info;
                info->next = leakInfo;
                node->memHead = info;
            }

            inserted = true;
            break;
        }
    }

    if (!inserted) {
        if (node->memHead != NULL && node->memTail != NULL) {
            // Append
            MemoryLeakInfo* prevTail = node->memTail;

            prevTail->next = info;
            info->prev = prevTail;

            node->memTail = info;
        } else {
            node->memHead = node->memTail = info;
        }
    }

    node->numInfo++;
} 

static void appendStackTraceInfo(MemoryLeakNode* memNode, StackTraceInfo* traceInfo)
{
    ASSERT(memNode != NULL && traceInfo != NULL);

    if (memNode->stHead != NULL && memNode->stTail != NULL) {
        StackTraceInfo* prevTail = memNode->stTail;

        if (prevTail != NULL)
            prevTail->next = traceInfo;

        traceInfo->prev = prevTail;

        memNode->stTail = traceInfo;
    } else {
        memNode->stHead = memNode->stTail = traceInfo;
    }

    memNode->numTrace++;
}

static void insertMemoryLeakHash(MemoryLeakRoot* memRoot,
                                    TCMemoryLeakHashRoot* hashRoot,
                                    unsigned int leakReqSum, unsigned int leakSum)
{
    const unsigned int hashSize = hashRoot->hashSize;

    for (unsigned int i = 0; i < hashSize; i++) {
        TCListRoot* listRoot = &hashRoot->listRoot[i];

        if (listRoot->head != NULL) {
            if (memRoot->head != NULL) {
                MemoryLeakNode* prevTail = (MemoryLeakNode*)memRoot->tail;
                MemoryLeakNode* newHead = (MemoryLeakNode*)listRoot->head;

                prevTail->next = newHead;
                newHead->prev = prevTail;
            } else {
                memRoot->head = (MemoryLeakNode*)listRoot->head;
            }

            memRoot->tail = (MemoryLeakNode*)listRoot->tail;
            memRoot->num += listRoot->len;
        }
    }

    memRoot->leakSum = leakSum;
    memRoot->leakReqSum = leakReqSum;
}

static void printMemoryLeakInfo(MemoryLeakRoot& leakRoot, bool printCallstack)
{
#if COMPILER(MSVC)
    time_t timer;
    struct tm *lt;

    time(&timer);

    lt = localtime(&timer);

    char name[kMaxNameLength + 1];

    int len = snprintf(name, kMaxNameLength,
                        "mem_leak_%04d%02d%02d_%02d%02d%02d.csv",
                        lt->tm_year + 1900,
                        lt->tm_mon + 1,
                        lt->tm_mday,
                        lt->tm_hour,
                        lt->tm_min,
                        lt->tm_sec);
    if (len == kMaxNameLength) {
        name[len] = 0;
    }

    try {
        ofstream ofs(name);

        ofs << "Memory Leaks," << leakRoot.num << endl;
        ofs << "Leak sum," << leakRoot.leakSum << "bytes,(" << leakRoot.leakSum / 1024 << "KB)" << endl;    
        ofs << "Time," << lt->tm_year + 1900 << "/" << lt->tm_mon + 1 << "/" << lt->tm_mday;
        ofs << " " << lt->tm_hour << ":" << lt->tm_min << ":" << lt->tm_sec << endl;

        ofs << endl << endl;

        for (MemoryLeakNode* leakNode = leakRoot.head; leakNode != NULL; leakNode = leakNode->next) {
            if (printCallstack) {
                ofs << "Same," << leakNode->numInfo << endl;
            }

            ofs << "AllocTime,Address,Size" << endl;

            for (MemoryLeakInfo* leakInfo = leakNode->memHead; leakInfo != NULL; leakInfo = leakInfo->next) {
                struct tm* lt = localtime(&leakInfo->curTime);

                ofs << lt->tm_year + 1900 << "/" << lt->tm_mon + 1 << "/" << lt->tm_mday;
                ofs << " " << lt->tm_hour << ":" << lt->tm_min << ":" << lt->tm_sec;
                ofs << ",0x" << leakInfo->adr;
                ofs << "," << leakInfo->reqSize;
                ofs << endl;
            }

            if (printCallstack) {
                ofs << endl;

                for (StackTraceInfo* traceInfo = leakNode->stHead; traceInfo != NULL; traceInfo = traceInfo->next) {
                    StackObjectInfo* obj = (StackObjectInfo*)traceInfo->obj;
                    ofs << "\"" << obj->name << "\"," << obj->line << endl;
                }
            }

            ofs << endl << endl << endl;
        }
    } catch (std::ios_base::failure) {
    }
#endif /* COMPILER(MSVC) */
}

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
static void initFreeList(TCFreeList& list)
{
    initListRoot(list.root);
    list.num = 0;
    list.ptr = NULL;
}

static void initHashRoot(TCHashRoot& root)
{
    root.listRoot = NULL;
    root.len = 0;
    root.maxLen = 0;
    root.hashSize = 0;
    root.maxNum = 0;
}

#else  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */
static void setUsedSpanNode(TCSpanNode* node)
{
    const unsigned int hashSize = gUsedSpanHash.hashSize;
    unsigned int hash = computeHash((unsigned char*)&node->obj, sizeof(void*));
    hash &= hashSize - 1;

    TCListRoot* spanMap = &gUsedSpanHash.listRoot[hash];
    if (spanMap->head) {
        // prepend
        TCSpanNode* prevHead = (TCSpanNode*)spanMap->head;
        prevHead->prev = node;
        node->next = prevHead;
        node->prev = NULL;

        spanMap->head = node;
    } else {
        spanMap->head = spanMap->tail = node;
    }

    spanMap->len++;

    gUsedSpanHash.len++;

    gUsedSpanHash.maxLen = FASTMALLOCDEBUG_MAX(gUsedSpanHash.len, gUsedSpanHash.maxLen);
}

static TCSpanNode* findUsedSpanNode(void* span)
{
    TCSpanNode* node = NULL;
    const unsigned int hashSize = gUsedSpanHash.hashSize;
    unsigned int hash = computeHash((unsigned char*)&span, sizeof(void*));
    hash &= hashSize - 1;

    TCListRoot* spanMap = &gUsedSpanHash.listRoot[hash];

    for (TCSpanNode* temp = (TCSpanNode*)spanMap->head; temp != NULL; temp = temp->next) {
        if (temp->obj == span) {
            if (temp->next) {
                temp->next->prev = temp->prev;
            } else {
                spanMap->tail = temp->prev;
            }

            if (temp->prev) {
                temp->prev->next = temp->next;
            } else {
                spanMap->head = temp->next;
            }

            temp->next = temp->prev = NULL;

            node = temp;

            spanMap->len--;

            gUsedSpanHash.len--;
            break;
        }
    }

    return node;
}
#endif  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

static void removeUsedSpanNode(void* span)
{
    ASSERT(span);
    TCSpanNode* node = findUsedSpanNode(span);
    ASSERT(node);
    appendFreeSpanNode(node);    
}

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
static void resetSpanNodeVariables()
{
    if (gSpanNodeInit) {
        // For Span Map
        fastSystemFree(gUsedSpanHash.listRoot);

        // For Span Hash
        TCSpanNode** ptr = (TCSpanNode**)gFreeSpanNodeList.ptr;
        unsigned int num = gFreeSpanNodeList.num;

        for (unsigned int i = 0; i < num; i++) {
            fastSystemFree(ptr[i]);
        }

        fastSystemFree(ptr);
    }

    initHashRoot(gUsedSpanHash);
    initFreeList(gFreeSpanNodeList);

    gSpanNodeInit = false;
}

static void resetMemoryNodeVariables()
{
    if (gMemoryNodeInit) {
        // For Used Memory Map
        fastSystemFree(gUsedMemoryHash.listRoot);

        // For Used Memory Hash
        {
            TCMemoryNode** ptr = (TCMemoryNode**)gFreeMemoryNodeList.ptr;
            unsigned int num = gFreeMemoryNodeList.num;

            for (unsigned int i = 0; i < num; i++) {
                fastSystemFree(ptr[i]);
            }

            fastSystemFree(ptr);
        }

        {
            MemoryLeakInfo** ptr = (MemoryLeakInfo**)gFreeMemoryLeakInfoList.ptr;
            unsigned int num = gFreeMemoryLeakInfoList.num;

            for (unsigned int i = 0; i < num; i++) {
                fastSystemFree(ptr[i]);
            }

            fastSystemFree(ptr);
        }

        {
            MemoryLeakNode** ptr = (MemoryLeakNode**)gFreeMemoryLeakNodeList.ptr;
            unsigned int num = gFreeMemoryLeakNodeList.num;

            for (unsigned int i = 0; i < num; i++) {
                fastSystemFree(ptr[i]);
            }

            fastSystemFree(ptr);
        }
    }

    initHashRoot(gUsedMemoryHash);
    initFreeList(gFreeMemoryNodeList);
    initFreeList(gFreeMemoryLeakInfoList);
    initFreeList(gFreeMemoryLeakNodeList);

    gMemoryNodeInit = false;
}

#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
static void resetStackTraceVariables()
{
    if (gStackTraceNodeInit) {
        {
            TCStackTraceNode** ptr = (TCStackTraceNode**)gFreeStackTraceNodeList.ptr;
            unsigned int num = gFreeStackTraceNodeList.num;

            for (unsigned int i = 0; i < num; i++) {
                fastSystemFree(ptr[i]);
            }

            fastSystemFree(ptr);
        }

        {
            StackTraceInfo** ptr = (StackTraceInfo**)gFreeStackTraceInfoList.ptr;
            unsigned int num = gFreeStackTraceInfoList.num;

            for (unsigned int i = 0; i < num; i++) {
                fastSystemFree(ptr[i]);
            }

            fastSystemFree(ptr);
        }

        {
            MemoryLeakNode** ptr = (MemoryLeakNode**)gFreeMemoryLeakNodeList.ptr;
            unsigned int num = gFreeMemoryLeakNodeList.num;

            for (unsigned int i = 0; i < num; i++) {
                fastSystemFree(ptr[i]);
            }

            fastSystemFree(ptr);
        }

        {
            TCStackObject** ptr = (TCStackObject**)gFreeStackObjectList.ptr;
            unsigned int num = gFreeStackObjectList.num;

            for (unsigned int i = 0; i < num; i++) {
                fastSystemFree(ptr[i]);
            }

            fastSystemFree(ptr);
        }

        {
            TCStackObjectNode** ptr = (TCStackObjectNode**)gFreeStackObjectNodeList.ptr;
            unsigned int num = gFreeStackObjectNodeList.num;

            for (unsigned int i = 0; i < num; i++) {
                fastSystemFree(ptr[i]);
            }

            fastSystemFree(ptr);
        }

        {
            TCStackObjectNode** ptr = (TCStackObjectNode**)gFreeStackObjectStatisticsNodeList.ptr;
            unsigned int num = gFreeStackObjectStatisticsNodeList.num;

            for (unsigned int i = 0; i < num; i++) {
                fastSystemFree(ptr[i]);
            }

            fastSystemFree(ptr);
        }

        fastSystemFree(gStackObjectNodeHash.listRoot);
    }

    initFreeList(gFreeMemoryLeakNodeList);
    initFreeList(gFreeStackTraceNodeList);
    initFreeList(gFreeStackTraceInfoList);
    initFreeList(gFreeStackObjectList);
    initFreeList(gFreeStackObjectNodeList);
    initFreeList(gFreeStackObjectStatisticsNodeList);
    initHashRoot(gStackObjectNodeHash);
    initListRoot(gMemoryLeakStatistics);

    gStackTraceNodeInit = false;
}
#endif /* ENABLE(WKC_FASTMALLOC_STACK_TRACE) */
#endif  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
#if ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE)
static inline unsigned long getFramePointer()
{
    __asm mov eax, [ebp]
}
#endif /* ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE) */

static void appendStackTraceNode(TCMemoryNode* memNode, TCStackTraceNode* traceNode)
{
    ASSERT(memNode != NULL && traceNode != NULL);

    if (memNode->head == NULL && memNode->tail == NULL) {
        memNode->head = memNode->tail = traceNode;
    } else {
        TCStackTraceNode* prevTail = memNode->tail;

        if (prevTail != NULL)
            prevTail->next = traceNode;

        traceNode->prev = prevTail;

        memNode->tail = traceNode;
    }

    memNode->traceLen++;
}

static void setStackTrace(TCMemoryNode* memNode, int skipStacks = 3)
{
#if ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE)
    TCStackTraceNode* traceNode;
    int skip = skipStacks;

    ASSERT(memNode != NULL);

    DWORD dwStart = getFramePointer();
    for (int i = 0; i < skip; i++) {
        dwStart = ((DWORD*)dwStart)[0];
    }

    DWORD dwFP = dwStart;
    while (dwFP != NULL) {
        DWORD dwNewFP = ((DWORD*)dwFP)[0];
        if (dwFP >= dwNewFP) // Sanity check
            break;

        traceNode = findFreeStackTraceNode();

        traceNode->obj = (void*)(((DWORD*)dwFP)[1]);

        appendStackTraceNode(memNode, traceNode);

        dwFP = dwNewFP;
    }
#endif /* ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE) */
}

static void releaseStackTrace(TCMemoryNode* memNode)
{
    for (TCStackTraceNode* node = memNode->head; node != NULL;) {
        TCStackTraceNode* next = node->next;
        appendFreeStackTraceNode(node);
        node = next;
    }

    memNode->traceLen = 0;
    memNode->head = memNode->tail = NULL;
}

static void initStackObjectInfo(StackObjectInfo& info)
{
    info.adr = NULL;
    info.line = 0;
    memset(&info.name, 0, sizeof(info.name));
}

#if !ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
static void initObjectNode(ObjectNode& node)
{
    node.obj = NULL;
    node.next = NULL;
    node.prev = NULL;
}
#endif /* !ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

static void initStackObject(TCStackObject& obj)
{
    initStackObjectInfo(obj.obj);
    obj.refCount = 0;
    obj.leakCount = 0;
    obj.sumLeakSize = 0;
    obj.next = NULL;
    obj.prev = NULL;
}

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
static bool growFreeStackObjectList()
{
    TCListRoot* listRoot = &gFreeStackObjectList.root;
    const unsigned int obj_num = kMinStackObject;

    // For Used Memory Hash
    TCStackObject* obj = (TCStackObject*)fastSystemMalloc(sizeof(TCStackObject) * obj_num);
    ASSERT(obj != NULL);

    for (unsigned int i = 0; i < obj_num; i++)
        initStackObject(obj[i]);

    for (unsigned int i = 0; i < obj_num; i++) {
        if (i > 0) {
            obj[i].prev = &obj[i - 1];
        } else {
            obj[i].prev = NULL;
        }

        if (i < obj_num - 1) {
            obj[i].next = &obj[i + 1];
        } else {
            obj[i].next = NULL;
        }
    }

    listRoot->head = &obj[0];
    listRoot->tail = &obj[obj_num - 1];
    listRoot->len = obj_num;

    unsigned int num = gFreeStackObjectList.num;
    TCStackObject** obj_array = (TCStackObject**)gFreeStackObjectList.ptr;
    TCStackObject** obj_array_new = (TCStackObject**)fastSystemRealloc(obj_array, sizeof(TCStackObject*) * (num + 1));
    ASSERT(obj_array_new != NULL);
    obj_array_new[num++] = obj;
    gFreeStackObjectList.ptr = obj_array_new;
    gFreeStackObjectList.num = num;

    return true;
}
#endif /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

void setStackObjectNode(TCStackObjectNode* node)
{
    const unsigned int hashSize = gStackObjectNodeHash.hashSize;
    TCStackObject* obj = (TCStackObject*)node->obj;
    StackObjectInfo* obj_info = &obj->obj;
    unsigned int hash = computeHash((unsigned char*)&obj_info->adr, sizeof(void*));
    hash &= hashSize - 1;

    TCListRoot* listRoot = &gStackObjectNodeHash.listRoot[hash];
    if (listRoot->head) {
        // prepend
        TCStackObjectNode* prevHead = (TCStackObjectNode*)listRoot->head;
        prevHead->prev = node;
        node->next = prevHead;
        node->prev = NULL;

        listRoot->head = node;
    } else {
        listRoot->head = listRoot->tail = node;
    }

    listRoot->len++;

    gStackObjectNodeHash.len++;

    gStackObjectNodeHash.maxLen = FASTMALLOCDEBUG_MAX(gStackObjectNodeHash.len, gStackObjectNodeHash.maxLen);
}

static TCStackObjectNode* findStackObjectNode(void* ptr, bool release = false)
{
    TCStackObjectNodeHashRoot* hashRoot = &gStackObjectNodeHash;
    TCStackObjectNode* node = NULL;
    const unsigned int hashSize = hashRoot->hashSize;
    unsigned int hash = computeHash((unsigned char*)&ptr, sizeof(void*));
    hash &= hashSize - 1;

    TCListRoot* listRoot = &hashRoot->listRoot[hash];

    for (TCStackObjectNode* temp = (TCStackObjectNode*)listRoot->head; temp != NULL; temp = temp->next) {
        TCStackObject* obj = (TCStackObject*)temp->obj;
        if (obj->obj.adr == ptr) {
            if (release) {
                if (temp->next) {
                    temp->next->prev = temp->prev;
                } else {
                    listRoot->tail = temp->prev;
                }

                if (temp->prev) {
                    temp->prev->next = temp->next;
                } else {
                    listRoot->head = temp->next;
                }

                temp->next = temp->prev = NULL;

                listRoot->len--;

                hashRoot->len--;
            }

            node = temp;
            break;
        }
    }

    return node;
}

TCStackObject* findFreeStackObject()
{
    TCListRoot* listRoot = &gFreeStackObjectList.root;
    TCStackObject* obj = (TCStackObject*)listRoot->head;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    if (obj == NULL) {
        growFreeStackObjectList();
        obj = (TCStackObject*)listRoot->head;
    }
#endif

    ASSERT(obj != NULL);

    if (obj->next != NULL)
        obj->next->prev = NULL;
    else
        listRoot->tail = NULL;

    listRoot->head = obj->next;

    listRoot->len--;

    initStackObject(*obj);

    return obj;
}

static void appendFreeStackObject(TCStackObject* obj)
{
    if (obj == NULL)
        return;

    initStackObject(*obj);

    TCListRoot* listRoot = &gFreeStackObjectList.root;

    TCStackObject* prevTail = (TCStackObject*)listRoot->tail;

    if (prevTail != NULL)
        prevTail->next = obj;

    obj->prev = prevTail;

    listRoot->tail = obj;

    listRoot->len++;
}

static void appendStackObjectToStatistics(TCStackObject* newObj, TCMemoryNode* memNode)
{
    if (newObj == NULL)
        return;

    bool found = false;
    TCListRoot* listRoot = &gMemoryLeakStatistics;

    // Duplicate Check
    for (TCStackObjectStatisticsNode* node = (TCStackObjectStatisticsNode*)listRoot->head; node != NULL; node = node->next) {
        if (node->obj == newObj) {
            found = true;
            break;
        }
    }

    newObj->leakCount++;
    newObj->sumLeakSize += memNode->info.requestSize;

    if (found)
        return;

    TCStackObjectStatisticsNode* node_new = findFreeStackObjectStatisticsNode();

    node_new->obj = newObj;

    if (listRoot->head != NULL && listRoot->tail != NULL) {
        TCStackObjectStatisticsNode* prevNode = (TCStackObjectStatisticsNode*)listRoot->tail;

        prevNode->next = node_new;
        node_new->prev = prevNode;

        listRoot->tail = node_new;
    } else {
        listRoot->head = listRoot->tail = node_new;
    }

    listRoot->len++;
}

static void sortStackObjectStatistics()
{
    TCListRoot* listRoot = &gMemoryLeakStatistics;
    TCStackObjectStatisticsNode* node_old = (TCStackObjectStatisticsNode*)listRoot->head;

    if (node_old == NULL)
        return;

    TCListRoot listRootNew;

    initListRoot(listRootNew);

    for (node_old = (TCStackObjectStatisticsNode*)listRoot->head; node_old != NULL; node_old = (TCStackObjectStatisticsNode*)listRoot->head) {
        TCStackObject* obj_old = (TCStackObject*)node_old->obj;

        // Remove Head
        listRoot->head = node_old->next;
        if (node_old->next) {
            node_old->next->prev = NULL;

            if (node_old->next->next == NULL) {
                listRoot->tail = node_old->next;
            }
        } else {
            listRoot->tail = NULL;
        }
        listRoot->len--;

        node_old->next = node_old->prev = NULL;

        bool inserted = false;
        TCStackObjectStatisticsNode* node = NULL;

        for (node = (TCStackObjectStatisticsNode*)listRootNew.head; node != NULL; node = node->next) {
            TCStackObject* obj = (TCStackObject*)node->obj;

            if ((obj_old->leakCount > obj->leakCount)
                    || ((obj_old->leakCount == obj->leakCount)
                    && (obj_old->sumLeakSize > obj->sumLeakSize))) {
                if (node->prev != NULL) {
                    // Insert
                    node->prev->next = node_old;
                    node_old->prev = node->prev;
                    node->prev = node_old;
                    node_old->next = node;
                } else {
                    // Prepend
                    node->prev = node_old;
                    node_old->next = node;
                    listRootNew.head = node_old;
                }

                listRootNew.len++;
                inserted = true;
                break;
            }
        }

        if (!inserted) {
            // Append
            node = (TCStackObjectStatisticsNode*)listRootNew.tail;

            if (node != NULL) {
                ASSERT(node->next == NULL);
                node->next = node_old;
            }

            node_old->prev = node;

            listRootNew.tail = node_old;

            if (listRootNew.head == NULL)
                listRootNew.head = node_old;

            listRootNew.len++;
        }
    }

    *listRoot = listRootNew;
}

static bool initializeStackTrace()
{
    bool ret = false;

#if ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE)
    gCurProcess = GetCurrentProcess();

    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_OMAP_FIND_NEAREST);

    if (!SymInitialize(gCurProcess, NULL, TRUE)) {
        goto exit_func;
    }
#endif /* ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE) */

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    resetStackTraceVariables();

    growFreeStackTraceNodeList();

    growFreeStackTraceInfoList();

    growFreeStackObjectList();

    growFreeStackObjectNodeList();

    growFreeStackObjectStatisticsNodeList();

    {
        const unsigned int hashSize = kMinStackObjectNodeHashOpenNum;

        gStackObjectNodeHash.listRoot = (TCListRoot*)fastSystemMalloc(sizeof(TCListRoot) * hashSize);
        gStackObjectNodeHash.hashSize = hashSize;

        for (unsigned int i = 0; i < hashSize; i++)
            initListRoot(gStackObjectNodeHash.listRoot[i]);

        gStackObjectNodeHash.len = 0;
        gStackObjectNodeHash.maxLen = 0;
        gStackObjectNodeHash.maxNum = 0;
    }

    gStackTraceNodeInit = true;

#else  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */
    {
        {
            const unsigned int memSize = sizeof(gStackTraceInfo) / sizeof(gStackTraceInfo[0]);

            for (unsigned int i = 0; i < memSize; i++)
                initStackTraceInfo(gStackTraceInfo[i]);

            for (unsigned int i = 0; i < memSize; i++) {
                if (i > 0) {
                    gStackTraceInfo[i].prev = &gStackTraceInfo[i - 1];
                } else {
                    gStackTraceInfo[i].prev = NULL;
                }

                if (i < memSize - 1) {
                    gStackTraceInfo[i].next = &gStackTraceInfo[i + 1];
                } else {
                    gStackTraceInfo[i].next = NULL;
                }
            }

            TCListRoot* listRoot = &gFreeStackTraceInfoList.root;
            initListRoot(*listRoot);
            listRoot->head = &gStackTraceInfo[0];
            listRoot->tail = &gStackTraceInfo[memSize - 1];
            listRoot->len = memSize;
            gFreeStackTraceInfoList.num = 0;
            gFreeStackTraceInfoList.ptr = NULL;
        }

        {
            const unsigned int memSize = sizeof(gStackObject) / sizeof(gStackObject[0]);

            for (unsigned int i = 0; i < memSize; i++)
                initStackObject(gStackObject[i]);

            for (unsigned int i = 0; i < memSize; i++) {
                if (i > 0) {
                    gStackObject[i].prev = &gStackObject[i - 1];
                } else {
                    gStackObject[i].prev = NULL;
                }

                if (i < memSize - 1) {
                    gStackObject[i].next = &gStackObject[i + 1];
                } else {
                    gStackObject[i].next = NULL;
                }
            }

            TCListRoot* listRoot = &gFreeStackObjectList.root;
            initListRoot(*listRoot);
            listRoot->head = &gStackObject[0];
            listRoot->tail = &gStackObject[memSize - 1];
            listRoot->len = memSize;
            gFreeStackObjectList.num = 0;
            gFreeStackObjectList.ptr = NULL;
        }

        {
            const unsigned int memSize = sizeof(gStackObjectNode) / sizeof(gStackObjectNode[0]);

            for (unsigned int i = 0; i < memSize; i++)
                initStackObjectNode(gStackObjectNode[i]);

            for (unsigned int i = 0; i < memSize; i++) {
                if (i > 0) {
                    gStackObjectNode[i].prev = &gStackObjectNode[i - 1];
                } else {
                    gStackObjectNode[i].prev = NULL;
                }

                if (i < memSize - 1) {
                    gStackObjectNode[i].next = &gStackObjectNode[i + 1];
                } else {
                    gStackObjectNode[i].next = NULL;
                }
            }

            TCListRoot* listRoot = &gFreeStackObjectNodeList.root;
            initListRoot(*listRoot);
            listRoot->head = &gStackObjectNode[0];
            listRoot->tail = &gStackObjectNode[memSize - 1];
            listRoot->len = memSize;
            gFreeStackObjectNodeList.num = 0;
            gFreeStackObjectNodeList.ptr = NULL;
        }

        {
            const unsigned int memSize = sizeof(gStackTraceNode) / sizeof(gStackTraceNode[0]);

            for (unsigned int i = 0; i < memSize; i++)
                initStackTraceNode(gStackTraceNode[i]);

            for (unsigned int i = 0; i < memSize; i++) {
                if (i > 0) {
                    gStackTraceNode[i].prev = &gStackTraceNode[i - 1];
                } else {
                    gStackTraceNode[i].prev = NULL;
                }

                if (i < memSize - 1) {
                    gStackTraceNode[i].next = &gStackTraceNode[i + 1];
                } else {
                    gStackTraceNode[i].next = NULL;
                }
            }

            TCListRoot* listRoot = &gFreeStackTraceNodeList.root;
            initListRoot(*listRoot);
            listRoot->head = &gStackTraceNode[0];
            listRoot->tail = &gStackTraceNode[memSize - 1];
            listRoot->len = memSize;
            gFreeStackTraceNodeList.num = 0;
            gFreeStackTraceNodeList.ptr = NULL;
        }

        {
            const unsigned int memSize = sizeof(gStaciObjectStatisticsNode) / sizeof(gStaciObjectStatisticsNode[0]);

            for (unsigned int i = 0; i < memSize; i++)
                initStackTraceNode(gStaciObjectStatisticsNode[i]);

            for (unsigned int i = 0; i < memSize; i++) {
                if (i > 0) {
                    gStaciObjectStatisticsNode[i].prev = &gStaciObjectStatisticsNode[i - 1];
                } else {
                    gStaciObjectStatisticsNode[i].prev = NULL;
                }

                if (i < memSize - 1) {
                    gStaciObjectStatisticsNode[i].next = &gStaciObjectStatisticsNode[i + 1];
                } else {
                    gStaciObjectStatisticsNode[i].next = NULL;
                }
            }

            TCListRoot* listRoot = &gFreeStackObjectStatisticsNodeList.root;
            initListRoot(*listRoot);
            listRoot->head = &gStaciObjectStatisticsNode[0];
            listRoot->tail = &gStaciObjectStatisticsNode[memSize - 1];
            listRoot->len = memSize;
            gFreeStackObjectStatisticsNodeList.num = 0;
            gFreeStackObjectStatisticsNodeList.ptr = NULL;
        }

        {
            const unsigned int hashSize = sizeof(gStackObjectNodeHash.listRoot) / sizeof(gStackObjectNodeHash.listRoot[0]);

            for (unsigned int i = 0; i < hashSize; i++)
                initListRoot(gStackObjectNodeHash.listRoot[i]);

            gStackObjectNodeHash.len = 0;
            gStackObjectNodeHash.maxLen = 0;
            gStackObjectNodeHash.hashSize = hashSize;
        }
    }
#endif /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

    ret = true;

#if ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE)
exit_func:
#endif
    return ret;
}

static void printMemoryLeakStatistics(TCListRoot& listRoot)
{
#if COMPILER(MSVC) && ENABLE(WKC_FASTMALLOC_STACK_TRACE)
    if (listRoot.head != NULL) {
        time_t timer;
        struct tm *lt;

        time(&timer);

        lt = localtime(&timer);

        char name[kMaxNameLength + 1];

        int len = snprintf(name, kMaxNameLength,
                            "mem_leak_stat_%04d%02d%02d_%02d%02d%02d.csv",
                            lt->tm_year + 1900,
                            lt->tm_mon + 1,
                            lt->tm_mday,
                            lt->tm_hour,
                            lt->tm_min,
                            lt->tm_sec);
        if (len == kMaxNameLength) {
            name[len] = 0;
        }

        try {
            ofstream ofs(name);

            ofs << "Memory Leak Statistics" << endl;
            ofs << "Name,Line,Count,Size" << endl;

            for (TCStackObjectStatisticsNode* node = (TCStackObjectStatisticsNode*)listRoot.head; node != NULL; node = node->next) {
                TCStackObject* obj = (TCStackObject*)node->obj;
                StackObjectInfo* obj_info = &obj->obj;

                ofs << "\"" << obj_info->name << "\",";

                if (obj_info->line > 0) {
                    ofs << obj_info->line;
                }

                ofs << "," << obj->leakCount << "," << obj->sumLeakSize << endl;
            }
        } catch (std::ios_base::failure) {
        }
    }
#endif /* COMPILER(MSVC) && ENABLE(WKC_FASTMALLOC_STACK_TRACE) */
}

static void finalizeStackTrace()
{
    if (cWriteMemoryLeak) {
        MemoryLeakRoot leakRoot;
        bool ret;

        ret = fastMallocDebugGetMemoryLeakInfo(leakRoot, cWriteStackTraceWithSymbol && cWriteStackTraceWithCallstack);
        if (ret) {
            if (cWriteStackTraceWithCallstack) {
                printMemoryLeakStatistics(gMemoryLeakStatistics);
            }
            printMemoryLeakInfo(leakRoot, cWriteStackTraceWithCallstack);
            fastMallocDebugClearMemoryLeakInfo(leakRoot);
        }
    }

    if (gMemoryLeakDumpProc) {
        (*gMemoryLeakDumpProc)(gMemoryLeakDumpCtx);
    }
#if ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE)
    SymCleanup(gCurProcess);
#endif
#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    resetStackTraceVariables();
#endif
}
#endif  /* ENABLE(WKC_FASTMALLOC_STACK_TRACE) */

static bool setMemoryLeakInfo(TCMemoryLeakHashRoot* hashRoot,
                                MemoryLeakNode* node, MemoryLeakInfo* info,
                                unsigned int& leakReqSum, unsigned int& leakSum)
{
    const unsigned int hashSize = hashRoot->hashSize;
    unsigned int hash = 0;
    bool found = false;
    MemoryLeakNode* leakNode = NULL;

#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
    for (StackTraceInfo* traceInfo = (StackTraceInfo*)node->stHead; traceInfo != NULL; traceInfo = traceInfo->next) {
        TCStackObject* obj = (TCStackObject*)traceInfo->obj;
        StackObjectInfo* obj_info = &obj->obj;

        hash = computeHash((unsigned char*)&obj_info->adr, sizeof(void*), hash);
    }
#else  /* ENABLE(WKC_FASTMALLOC_STACK_TRACE) */
    hash = computeHash((unsigned char*)&info->adr, sizeof(void*));
#endif  /* ENABLE(WKC_FASTMALLOC_STACK_TRACE) */

    hash &= hashSize - 1;

    TCListRoot* listRoot = &hashRoot->listRoot[hash];

#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
    for (leakNode = (MemoryLeakNode*)listRoot->head; leakNode != NULL; leakNode = leakNode->next) {
        if ((leakNode->numTrace > 0) && (node->numTrace > 0) && (leakNode->numTrace == node->numTrace)) {
            StackTraceInfo* traceInfoHash = leakNode->stHead;
            StackTraceInfo* traceInfoNew = node->stHead;
            bool notMatch = false;

            ASSERT(traceInfoHash != NULL && traceInfoNew != NULL);  // Fail safe

            for (;;) {
                if (traceInfoHash == NULL || traceInfoNew == NULL) {
                    ASSERT(traceInfoHash == NULL && traceInfoNew == NULL);  // Fail safe
                    break;
                }

                TCStackObject* obj_hash = (TCStackObject*)traceInfoHash->obj;
                TCStackObject* obj_new = (TCStackObject*)traceInfoNew->obj;

                StackObjectInfo* obj_info_hash = &obj_hash->obj;
                StackObjectInfo* obj_info_new = &obj_new->obj;

                if (obj_info_hash->adr != obj_info_new->adr) {
                    notMatch = true;
                    break;
                }

                traceInfoHash = traceInfoHash->next;
                traceInfoNew = traceInfoNew->next;
            }

            if (!notMatch) {
                found = true;
                ASSERT(leakNode != NULL);
                break;
            }
        }
    }
#endif /* ENABLE(WKC_FASTMALLOC_STACK_TRACE) */

    if (!found) {
        insertMemoryLeakInfo(node, info);

        // Prepend
        if (listRoot->head != NULL && listRoot->tail != NULL) {
            MemoryLeakNode* prevNode = (MemoryLeakNode*)listRoot->head;

            prevNode->prev = node;
            node->next = prevNode;

            listRoot->head = node;
        } else {
            listRoot->head = listRoot->tail = node;
        }

        listRoot->len++;

        hashRoot->len++;
    } else {
        insertMemoryLeakInfo(leakNode, info);
    }

    leakReqSum += info->reqSize;
    leakSum += info->size;

    return found;
}

static void initMemoryLeakRoot(MemoryLeakRoot& root)
{
    root.head = NULL;
    root.tail = NULL;
    root.num = 0;
    root.leakReqSum = 0;
    root.leakSum = 0;
}
#endif  /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) */

// Public API
bool initializeTCMallocDebugWKC()
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
    if (!initializeStackTrace())
        return false;
#endif

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    {
        resetMemoryNodeVariables();
        resetSpanNodeVariables();

        const unsigned int sysmemsize = wkcMemoryGetAvailSystemMemorySizePeer();

        const unsigned int max_span_num = FASTMALLOCDEBUG_HOWMANY(sysmemsize, kPageSize);

        const unsigned int span_num = FASTMALLOCDEBUG_MIN(max_span_num, kMinSpanArrayNum);

        const unsigned int array_num = FASTMALLOCDEBUG_HOWMANY(kPageSize, kAlignment);

        const unsigned int max_node_num = span_num * array_num;

        // For Used Memory Hash
        growFreeMemoryNodeList(max_node_num);

        const unsigned int hash_size1 = FASTMALLOCDEBUG_MAX(FASTMALLOCDEBUG_HOWMANY(max_node_num, kMaxHashChainNum), kMinHashOpenNum);

        gUsedMemoryHash.listRoot = (TCListRoot*)fastSystemMalloc(sizeof(TCListRoot) * hash_size1);
        gUsedMemoryHash.hashSize = hash_size1;

        for (unsigned int i = 0; i < hash_size1; i++)
            initListRoot(gUsedMemoryHash.listRoot[i]);

        gUsedMemoryHash.len = 0;
        gUsedMemoryHash.maxLen = 0;
        gUsedMemoryHash.maxNum = max_node_num;

        gMemoryNodeInit = true;


        // For Memory Leak
        growFreeMemoryLeakInfoList();
        growFreeMemoryLeakNodeList();


        // For Span Hash
        growFreeSpanNodeList(max_span_num);

        const unsigned int hash_size2 = FASTMALLOCDEBUG_MAX(FASTMALLOCDEBUG_HOWMANY(span_num, kMaxHashChainNum), kMinHashOpenNum);

        gUsedSpanHash.listRoot = (TCListRoot*)fastSystemMalloc(sizeof(TCListRoot) * hash_size2);
        gUsedSpanHash.hashSize = hash_size2;

        for (unsigned int i = 0; i < hash_size2; i++)
            initListRoot(gUsedSpanHash.listRoot[i]);

        gUsedSpanHash.len = 0;
        gUsedSpanHash.maxLen = 0;
        gUsedSpanHash.maxNum = max_span_num;

        gSpanNodeInit = true;
    }

    if (!gAllocatedMemoryMapByAllocator) {
        gAllocatedMemoryMap = false;
    }
#else  /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */
    {
        unsigned int memsize = sizeof(gMemoryNode) / sizeof(gMemoryNode[0]);

        for (unsigned int i = 0; i < memsize; i++)
            initMemoryNode(gMemoryNode[i]);

        for (unsigned int i = 0; i < memsize; i++) {
            if (i > 0) {
                gMemoryNode[i].prev = &gMemoryNode[i - 1];
            } else {
                gMemoryNode[i].prev = NULL;
            }

            if (i < memsize - 1) {
                gMemoryNode[i].next = &gMemoryNode[i + 1];
            } else {
                gMemoryNode[i].next = NULL;
            }
        }

        TCListRoot* listRoot = &gFreeMemoryNodeList.root;
        initListRoot(*listRoot);
        listRoot->head = &gMemoryNode[0];
        listRoot->tail = &gMemoryNode[memsize - 1];
        listRoot->len = memsize;
        gFreeMemoryNodeList.num = 0;
        gFreeMemoryNodeList.ptr = NULL;

        const unsigned int hashSize = sizeof(gUsedMemoryHash.listRoot) / sizeof(gUsedMemoryHash.listRoot[0]);

        for (unsigned int i = 0; i < hashSize; i++)
            initListRoot(gUsedMemoryHash.listRoot[i]);

        gUsedMemoryHash.len = 0;
        gUsedMemoryHash.maxLen = 0;
        gUsedMemoryHash.hashSize = hashSize;
    }

    {
        unsigned int memSize = sizeof(gMemoryLeakInfo) / sizeof(gMemoryLeakInfo[0]);

        for (unsigned int i = 0; i < memSize; i++)
            initMemoryLeakInfo(gMemoryLeakInfo[i]);

        for (unsigned int i = 0; i < memSize; i++) {
            if (i > 0) {
                gMemoryLeakInfo[i].prev = &gMemoryLeakInfo[i - 1];
            } else {
                gMemoryLeakInfo[i].prev = NULL;
            }

            if (i < memSize - 1) {
                gMemoryLeakInfo[i].next = &gMemoryLeakInfo[i + 1];
            } else {
                gMemoryLeakInfo[i].next = NULL;
            }
        }

        TCListRoot* listRoot = &gFreeMemoryLeakInfoList.root;
        initListRoot(*listRoot);
        listRoot->head = &gMemoryLeakInfo[0];
        listRoot->tail = &gMemoryLeakInfo[memSize - 1];
        listRoot->len = memSize;
        gFreeMemoryLeakInfoList.num = 0;
        gFreeMemoryLeakInfoList.ptr = NULL;
    }

    {
        unsigned int memSize = sizeof(gMemoryLeakNode) / sizeof(gMemoryLeakNode[0]);

        for (unsigned int i = 0; i < memSize; i++)
            initMemoryLeakNode(gMemoryLeakNode[i]);

        for (unsigned int i = 0; i < memSize; i++) {
            if (i > 0) {
                gMemoryLeakNode[i].prev = &gMemoryLeakNode[i - 1];
            } else {
                gMemoryLeakNode[i].prev = NULL;
            }

            if (i < memSize - 1) {
                gMemoryLeakNode[i].next = &gMemoryLeakNode[i + 1];
            } else {
                gMemoryLeakNode[i].next = NULL;
            }
        }

        TCListRoot* listRoot = &gFreeMemoryLeakNodeList.root;
        initListRoot(*listRoot);
        listRoot->head = &gMemoryLeakNode[0];
        listRoot->tail = &gMemoryLeakNode[memSize - 1];
        listRoot->len = memSize;
        gFreeMemoryLeakNodeList.num = 0;
        gFreeMemoryLeakNodeList.ptr = NULL;
    }

    {
        unsigned int spanSize = sizeof(gSpanNode) / sizeof(gSpanNode[0]);

        for (unsigned int i = 0; i < spanSize; i++)
            initSpanNode(gSpanNode[i]);

        for (unsigned int i = 0; i < spanSize; i++) {
            if (i > 0) {
                gSpanNode[i].prev = &gSpanNode[i - 1];
            } else {
                gSpanNode[i].prev = NULL;
            }

            if (i < spanSize - 1) {
                gSpanNode[i].next = &gSpanNode[i + 1];
            } else {
                gSpanNode[i].next = NULL;
            }
        }

        TCListRoot* listRoot = &gFreeSpanNodeList.root;
        initListRoot(*listRoot);
        listRoot->head = &gSpanNode[0];
        listRoot->tail = &gSpanNode[spanSize - 1];
        listRoot->len = spanSize;
        gFreeSpanNodeList.num = 0;
        gFreeSpanNodeList.ptr = NULL;

        const unsigned int hashSize = sizeof(gUsedSpanHash.listRoot) / sizeof(gUsedSpanHash.listRoot[0]);

        for (unsigned int i = 0; i < hashSize; i++)
            initListRoot(gUsedSpanHash.listRoot[i]);

        gUsedSpanHash.len = 0;
        gUsedSpanHash.maxLen = 0;
        gUsedSpanHash.hashSize = hashSize;
    }
#endif /* ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

    return true;

#else   /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) */

    return false;

#endif /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) */
}

void finalizeTCMallocDebugWKC()
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
    finalizeStackTrace();
#endif
#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    resetSpanNodeVariables();
    resetMemoryNodeVariables();
#endif
#endif /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) */
}

void fastMallocDebugSetUsedSpan(void* span)
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
    TCSpanNode* node = findFreeSpanNode();
    node->obj = span;
    setUsedSpanNode(node);
#endif
}

void fastMallocDebugRemoveUsedSpan(void* span)
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
    removeUsedSpanNode(span);
#endif
}

void fastMallocDebugSetUsedMemory(void* ptr, unsigned int reqSize, unsigned int usedSize, unsigned short cl)
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
    SpinLockHolder h(&gMemoryInfoLock);

    TCMemoryNode* node = findFreeMemoryNode();

    UsedMemoryInfo* info = &node->info;

    info->adr = ptr;
    info->requestSize = reqSize;
    info->usedSize = usedSize;
    info->classID = cl;
    time(&node->curTime);

    setUsedMemoryNode(node);

#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
    setStackTrace(node);
#endif
#endif /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) */
}

void fastMallocDebugRemoveUsedMemory(void* ptr)
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
    SpinLockHolder h(&gMemoryInfoLock);

    ASSERT(ptr);
    TCMemoryNode* node = findUsedMemoryNode(ptr);
    ASSERT(node);
#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
    releaseStackTrace(node);
#endif
    appendFreeMemoryNode(node);
#endif
}

void fastMallocDebugGetNumberInfo(unsigned int& maxNumSpan, unsigned int& maxNumMem)
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
    maxNumSpan = gUsedMemoryHash.maxLen;
    maxNumMem = gUsedSpanHash.maxLen;
#else
    maxNumSpan = 0;
    maxNumMem = 0;
#endif
}

// for Memory Map
void fastMallocDebugGetMemoryMap(MemoryInfo& memInfo, bool needUsedMemory)
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
    initMemoryInfo(memInfo);

    if (!gMemoryInfoLock.IsHeld()) {
        SpinLockHolder lockHolder(&gMemoryInfoLock);

        GetUsedSpanInfo(memInfo);

        const unsigned int size = gUsedMemoryHash.hashSize;

        for (unsigned int i = 0; i < size; i++) {
            TCListRoot* root = &gUsedMemoryHash.listRoot[i];
            for (TCMemoryNode* node = (TCMemoryNode*)root->head; node != NULL; node = node->next) {
                UsedMemoryInfo* info = &node->info;
                void* ptr = info->adr;
                if (ptr) {
                    unsigned int cl = fastMallocGetSizeClass(ptr);
                    if (cl == 0) {
                        cl = fastMallocGetSpanSizeClass(ptr);
                    }

                    attachAddressInfo(memInfo, *info, cl, needUsedMemory);
                }
            }
        }

        if (needUsedMemory)
            sortUsedMemoryInfo(memInfo);
    }
#endif /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) */
}

bool fastMallocDebugAllocMemoryMap(MemoryInfo& memInfo)
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) && ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    if (!gAllocatedMemoryMapByAllocator && !gAllocatedMemoryMap && gMemoryNodeInit && gSpanNodeInit) {
        unsigned int max_span_num = gUsedSpanHash.maxNum;
        ASSERT(max_span_num > 0);

        memInfo.spanArray = (SpanInfo*)fastSystemMalloc(sizeof(SpanInfo) * max_span_num);
        ASSERT(memInfo.spanArray != NULL);
        memInfo.spanPtrArray = (SpanInfo**)fastSystemMalloc(sizeof(SpanInfo*) * max_span_num);
        ASSERT(memInfo.spanPtrArray != NULL);

        const unsigned int array_num = FASTMALLOCDEBUG_HOWMANY(kPageSize, kAlignment);

        for (unsigned int i = 0; i < max_span_num; i++) {
            SpanInfo* spanInfo = &memInfo.spanArray[i];
            spanInfo->usedMemArray = (UsedMemoryInfo*)fastSystemMalloc(sizeof(UsedMemoryInfo) * array_num);
            ASSERT(spanInfo->usedMemArray != NULL);
            spanInfo->usedMemPtrArray = (UsedMemoryInfo**)fastSystemMalloc(sizeof(UsedMemoryInfo*) * array_num);
            ASSERT(spanInfo->usedMemPtrArray != NULL);
        }

        gAllocatedMemoryMap = true;
    }

    gAllocatedMemoryMapByAllocator = true;

    return true;

#else   /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) && ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */

    return false;

#endif  /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) && ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */
}

void fastMallocDebugFreeMemoryMap(MemoryInfo& memInfo)
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) && ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
    if (gAllocatedMemoryMap) {
        unsigned int span_num = memInfo.numSpanArray;
        if (gAllocatedMemoryMapByAllocator) {
            span_num = gUsedSpanHash.maxNum;
            ASSERT(span_num > 0);
        }

        for (unsigned int i = 0; i < span_num; i++) {
            SpanInfo* spanInfo = &memInfo.spanArray[i];
            if (spanInfo->usedMemArray != NULL)
                fastSystemFree(spanInfo->usedMemArray);
            if (spanInfo->usedMemPtrArray != NULL)
                fastSystemFree(spanInfo->usedMemPtrArray);
        }

        if (memInfo.spanArray != NULL)
            fastSystemFree(memInfo.spanArray);

        if (memInfo.spanPtrArray != NULL)
            fastSystemFree(memInfo.spanPtrArray);

        memInfo.numSpanArray = 0;
        memInfo.pageSize = 0;
        memInfo.spanArray = NULL;
        memInfo.spanPtrArray = NULL;
    }

    gAllocatedMemoryMap = false;
    gAllocatedMemoryMapByAllocator = false;
#endif  /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) && ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION) */
}

// for Memory Leaks
void fastMallocDebugSetMemoryLeakDumpProc(MemoryLeakDumpProc in_proc, void* in_ctx)
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
    gMemoryLeakDumpProc = in_proc;
    gMemoryLeakDumpCtx = in_ctx;
#endif
}

bool fastMallocDebugGetMemoryLeakInfo(MemoryLeakRoot& leakRoot, bool resolveSymbol)
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
    bool ret = false;

    initMemoryLeakRoot(leakRoot);

    if (!gMemoryInfoLock.IsHeld()) {
        SpinLockHolder lockHolder(&gMemoryInfoLock);

        TCMemoryLeakHashRoot memLeakHash;

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
        initHashRoot(memLeakHash);

        memLeakHash.listRoot = (TCListRoot*)fastSystemMalloc(sizeof(TCListRoot) * kMinMemoryLeakHashOpenNum);
        ASSERT(memLeakHash.listRoot);
        memLeakHash.hashSize = kMinMemoryLeakHashOpenNum;

        const int hashSize = memLeakHash.hashSize;
#else
        initMemoryLeakHashRoot(memLeakHash);

        const int hashSize = sizeof(memLeakHash.listRoot) / sizeof(memLeakHash.listRoot[0]);

        memLeakHash.hashSize = hashSize;
#endif

        for (int i = 0; i < hashSize; i++) {
            initListRoot(memLeakHash.listRoot[i]);
        }

        const unsigned int size = gUsedMemoryHash.hashSize;

        unsigned int leakReqSum = 0;
        unsigned int leakSum = 0;

        for (unsigned int i = 0; i < size; i++) {
            TCListRoot* root = &gUsedMemoryHash.listRoot[i];

            for (TCMemoryNode* memNode = (TCMemoryNode*)root->head; memNode != NULL; memNode = memNode->next) {
                bool setStat = false;
                MemoryLeakInfo* leakInfo = findFreeMemoryLeakInfo();

                leakInfo->adr = memNode->info.adr;
                leakInfo->size = memNode->info.usedSize;
                leakInfo->reqSize = memNode->info.requestSize;
                leakInfo->curTime = memNode->curTime;

                MemoryLeakNode* leakNode = findFreeMemoryLeakNode();

#if ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE)
                IMAGEHLP_SYMBOL* pSymbol = (IMAGEHLP_SYMBOL*)gBuffer1;
                IMAGEHLP_LINE* pLine = (IMAGEHLP_LINE*)gBuffer2;

                TCStackObject* obj_prev = NULL;

                for (TCStackTraceNode* traceNode = memNode->head; traceNode != NULL; traceNode = traceNode->next) {
                    DWORD dwAddr = (DWORD)traceNode->obj;
                    DWORD dwLineDisp = 0;
                    BOOL ret_sym = FALSE;
                    BOOL ret_line = FALSE;
                    TCStackObject* obj = NULL;
                    TCStackObjectNode* obj_node = NULL;
                    bool foundFromHash = false;

                    if (resolveSymbol) {
                        obj_node = findStackObjectNode(traceNode->obj);
                        foundFromHash = obj_node ? true : false;
                        if (obj_node == NULL) {
                            pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
                            pSymbol->MaxNameLength = kMaxNameLength;
                            ret_sym = SymGetSymFromAddr(gCurProcess, dwAddr, 0, pSymbol);

                            pLine->SizeOfStruct = sizeof(IMAGEHLP_LINE);
                            ret_line = SymGetLineFromAddr(gCurProcess, dwAddr, &dwLineDisp, pLine);
                        }
                    }

                    if (obj_node == NULL) {
                        obj = findFreeStackObject();
                        obj_node = findFreeStackObjectNode();
                        obj_node->obj = obj;
                    }

                    obj = (TCStackObject*)obj_node->obj;
                    StackObjectInfo* obj_info = &obj->obj;

                    if (!foundFromHash) {
                        obj_info->adr = (void*)dwAddr;
                        if (ret_sym) {
                            strncpy((char*)obj_info->name, pSymbol->Name, kMaxNameLength);
                        } else {
                            snprintf((char*)obj_info->name, kMaxNameLength, "0x%p", dwAddr);
                        }

                        if (ret_line) {
                            obj_info->line = pLine->LineNumber;
                            obj_info->displacement = dwLineDisp;
                        }
                    }

                    obj->refCount++;

                    if (!foundFromHash)
                        setStackObjectNode(obj_node);

                    for (int j = 0; j < cTerminatorSize; j++) {
                        if (strstr((const char*)obj_info->name, cTerminatorStr[j]) != NULL)
                            goto exit_most_inside_loop;
                    }

                    char* pdest = NULL;

                    for (int j = 0; j < cPrefixSize; j++) {
                        pdest = (char*)strstr((const char*)obj_info->name, cPrefixToRemove[j]);
                        if (pdest != NULL)
                            break;
                    }

                    if (pdest == NULL) {
                        if (obj_prev != NULL) {
                            StackTraceInfo* traceInfoPrev = findFreeStackTraceInfo();
                            traceInfoPrev->obj = obj_prev;
                            appendStackTraceInfo(leakNode, traceInfoPrev);
                            obj_prev = NULL;
                        }

                        StackTraceInfo* traceInfo = findFreeStackTraceInfo();
                        traceInfo->obj = obj;
                        appendStackTraceInfo(leakNode, traceInfo);

                        if (!setStat) {
                            appendStackObjectToStatistics(obj, memNode);
                            setStat = true;
                        }
                    } else {
                        obj_prev = obj;
                    }
                }

exit_most_inside_loop:
#endif  /* ENABLE(WKC_FASTMALLOC_WIN_STACK_TRACE) */
                if (setMemoryLeakInfo(&memLeakHash, leakNode, leakInfo, leakReqSum, leakSum)) {
#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
                    // Found the same stack trace
                    for (StackTraceInfo* traceInfo = (StackTraceInfo*)leakNode->stHead; traceInfo != NULL; ) {
                        StackTraceInfo* traceInfoNext = traceInfo->next;
                        appendFreeStackTraceInfo(traceInfo);
                        traceInfo = traceInfoNext;
                    }
#endif  /* ENABLE(WKC_FASTMALLOC_STACK_TRACE) */

                    appendFreeMemoryLeakNode(leakNode);
                }
            }
        }

        insertMemoryLeakHash(&leakRoot, &memLeakHash, leakReqSum, leakSum);

#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
        sortStackObjectStatistics();
#endif

#if ENABLE(WKC_FASTMALLOC_DYNAMIC_ALLOCATION)
        fastSystemFree(memLeakHash.listRoot);
#endif

        ret = true;
    }

    return ret;

#else  /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) */

    return false;

#endif /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) */
}

void fastMallocDebugClearMemoryLeakInfo(MemoryLeakRoot& leakRoot)
{
#if ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO)
    for (MemoryLeakNode* leakNode = leakRoot.head; leakNode != NULL; ) {
        MemoryLeakNode* nextLeakNode = leakNode->next;

#if ENABLE(WKC_FASTMALLOC_STACK_TRACE)
        for (StackTraceInfo* traceInfo = leakNode->stHead; traceInfo != NULL; ) {
            StackTraceInfo* nextTraceInfo = traceInfo->next;
            appendFreeStackTraceInfo(traceInfo);
            traceInfo = nextTraceInfo;
        }

        {
            const unsigned int hashSize = gStackObjectNodeHash.hashSize;

            for (unsigned int i = 0; i < hashSize; i++) {
                TCListRoot* listRoot = &gStackObjectNodeHash.listRoot[i];

                for (TCStackObjectNode* node = (TCStackObjectNode*)listRoot->head; node != NULL; node = node->next) {
                    TCStackObject* obj = (TCStackObject*)node->obj;
                    obj->leakCount = 0;
                    obj->refCount = 0;
                }
            }
        }
#endif /* ENABLE(WKC_FASTMALLOC_STACK_TRACE) */

        for (MemoryLeakInfo* leakInfo = leakNode->memHead; leakInfo != NULL; ) {
            MemoryLeakInfo* nextLeakInfo = leakInfo->next;
            appendFreeMemoryLeakInfo(leakInfo);
            leakInfo = nextLeakInfo;
        }

        appendFreeMemoryLeakNode(leakNode);
        leakNode = nextLeakNode;
    }

    initMemoryLeakRoot(leakRoot);

#else  /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) */

    memset(&leakRoot, 0, sizeof(leakRoot));

#endif /* ENABLE(WKC_FASTMALLOC_USED_MEMORY_INFO) */
}

void fastMallocDebugClearStackTrace()
{
    /* Nothing to do currently */
}

} // namespace WTF

#endif // FORCE_SYSTEM_MALLOC

#endif  /* ENABLE_REPLACEMENT_SYSTEMMEMORY */
#endif  /* PLATFORM(WKC) */
