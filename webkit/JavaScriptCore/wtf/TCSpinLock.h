// Copyright (c) 2005, 2006, Google Inc.
// All rights reserved.
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

// ---
// Author: Sanjay Ghemawat <opensource@google.com>

#ifndef TCMALLOC_INTERNAL_SPINLOCK_H__
#define TCMALLOC_INTERNAL_SPINLOCK_H__

#if (PLATFORM(X86) || PLATFORM(PPC)) && (COMPILER(GCC) || COMPILER(MSVC))

#include <time.h>       /* For nanosleep() */

#include <sched.h>      /* For sched_yield() */

#if HAVE(STDINT_H)
#include <stdint.h>
#elif HAVE(INTTYPES_H)
#include <inttypes.h>
#else
#include <sys/types.h>
#endif

#if PLATFORM(WIN_OS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#if COMPILER(MSVC) && defined(_DEBUG)
#define WKC_TCSPINLOCK_WIN_STACK_TRACE
#endif

#ifdef WKC_TCSPINLOCK_WIN_STACK_TRACE
#include <windows.h>
#include <ImageHlp.h>
#include <fstream>

using namespace std;

enum {
    kSpinLockObjectNum = 10,
    kMaxSpinLockStackTraceNodeNum = 1000,
    kSpinLockStackTraceNodeNum = kSpinLockObjectNum * kMaxSpinLockStackTraceNodeNum,
    kMaxNanoSleepLoopCount = 10
};

typedef struct SpinLockStackTraceNode_ SpinLockStackTraceNode;

struct SpinLockStackTraceNode_ {
    void* obj;
    SpinLockStackTraceNode* next;
    SpinLockStackTraceNode* prev;
};

typedef struct SpinLockObject_ SpinLockObject;

struct SpinLockObject_ {
    SpinLockObject* next;
    SpinLockObject* prev;
    unsigned int num;
    SpinLockStackTraceNode* head;
    SpinLockStackTraceNode* tail;
};

struct SpinLockListRoot_ {
    void* head;
    void* tail;
    unsigned int len;
};

typedef struct SpinLockListRoot_ SpinLockListRoot;

struct SpinLockFreeList_ {
    SpinLockListRoot root;
};

typedef struct SpinLockFreeList_ SpinLockFreeList;

static bool gSpinLockInit = false;
static bool gSpinLockPrint = false;
static SpinLockObject gSpinLockObject[kSpinLockObjectNum];
static SpinLockStackTraceNode gSpinLockStackTraceNode[kSpinLockStackTraceNodeNum];
static SpinLockFreeList gFreeSpinLockObject;
static SpinLockFreeList gFreeSpinLockStackTraceNode;
static SpinLockListRoot gSpinLockObjectList;

static void initSpinLockStackTraceNode(SpinLockStackTraceNode& node)
{
    node.obj = NULL;
    node.next = NULL;
    node.prev = NULL;
}

static void initSpinLockObject(SpinLockObject& obj)
{
    obj.next = NULL;
    obj.prev = NULL;
    obj.num = 0;
    obj.head = NULL;
    obj.tail = NULL;
}

static void initSpinLockListRoot(SpinLockListRoot& listRoot)
{
    listRoot.head = NULL;
    listRoot.tail = NULL;
    listRoot.len = 0;
}

static SpinLockStackTraceNode* findFreeSpinLockStackTraceNode()
{
    SpinLockListRoot* listRoot = &gFreeSpinLockStackTraceNode.root;
    SpinLockStackTraceNode* node = (SpinLockStackTraceNode*)listRoot->head;

    ASSERT(node);

    listRoot->head = node->next;

    if (node->next == NULL)
        listRoot->tail = NULL;

    initSpinLockStackTraceNode(*node);

    listRoot->len--;

    return node;
}

static void appendFreeSpinLockStackTraceNode(SpinLockStackTraceNode* node)
{
    SpinLockListRoot* listRoot = &gFreeSpinLockStackTraceNode.root;

    initSpinLockStackTraceNode(*node);

    if (listRoot->head != NULL && listRoot->tail != NULL) {
        SpinLockStackTraceNode* prevNode = (SpinLockStackTraceNode*)listRoot->tail;

        if (prevNode != NULL)
            prevNode->next = node;

        node->prev = prevNode;
        listRoot->tail = node;
    } else {
        listRoot->head = listRoot->tail = node;
    }

    listRoot->len++;
}

static SpinLockObject* removeFirstSpinLockObject()
{
    SpinLockListRoot* listRoot = &gSpinLockObjectList;
    SpinLockObject* obj = (SpinLockObject*)listRoot->head;

    listRoot->head = obj->next;
    if (obj->next == NULL)
        listRoot->tail = NULL;

    for (SpinLockStackTraceNode* node = obj->head; node != NULL; ) {
        SpinLockStackTraceNode* next_node = node->next;
        appendFreeSpinLockStackTraceNode(node);
        node = next_node;
    }

    initSpinLockObject(*obj);

    listRoot->len--;

    return obj;
}

static SpinLockObject* findFreeSpinLockObject()
{
    SpinLockListRoot* listRoot = &gFreeSpinLockObject.root;
    SpinLockObject* obj = (SpinLockObject*)listRoot->head;

    if (obj == NULL) {
        obj = removeFirstSpinLockObject();
    } else {
        listRoot->head = obj->next;

        if (obj->next == NULL)
            listRoot->tail = NULL;

        initSpinLockObject(*obj);

        listRoot->len--;
    }

    return obj;
}

static void appendSpinLockObject(SpinLockObject* obj)
{
    SpinLockListRoot* listRoot = &gSpinLockObjectList;

    if (listRoot->head != NULL && listRoot->tail != NULL) {
        SpinLockObject* prevObj = (SpinLockObject*)listRoot->tail;

        if (prevObj != NULL)
            prevObj->next = obj;

        obj->prev = prevObj;
        listRoot->tail = obj;
    } else {
        listRoot->head = listRoot->tail = obj;
    }

    listRoot->len++;
}

static bool appendSpinLockStackTraceNode(SpinLockObject* obj, SpinLockStackTraceNode* node)
{
    bool ret = false;

    if (obj->head != NULL && obj->tail != NULL) {
        SpinLockStackTraceNode* prevNode = obj->tail;

        if (prevNode != NULL)
            prevNode->next = node;

        node->prev = prevNode;
        obj->tail = node;
    } else {
        obj->head = obj->tail = node;
    }

    if (++obj->num < kMaxSpinLockStackTraceNodeNum)
        ret = true;

    return ret;
}

static inline unsigned long getSpinLockFramePointer()
{
    __asm mov eax, [ebp]
}

static void setSpinLockStackTrace(SpinLockObject* obj, int skipStacks = 0)
{
    SpinLockStackTraceNode* node;
    int skip = skipStacks;

    ASSERT(obj != NULL);

    unsigned long dwStart = getSpinLockFramePointer();
    for (int i = 0; i < skip; i++) {
        dwStart = ((unsigned long*)dwStart)[0];
    }

    unsigned long dwFP = dwStart;
    while (dwFP != NULL) {
        unsigned long dwNewFP = ((unsigned long*)dwFP)[0];
        if (dwFP >= dwNewFP) // Sanity check
            break;

        node = findFreeSpinLockStackTraceNode();

        node->obj = (void*)(((unsigned long*)dwFP)[1]);

        if (!appendSpinLockStackTraceNode(obj, node))
            break;

        dwFP = dwNewFP;
    }
}

static void setSpinLockInfo()
{
    SpinLockObject* obj = findFreeSpinLockObject();
    setSpinLockStackTrace(obj);
    appendSpinLockObject(obj);
}

static const unsigned int kSpinLockMaxNameLength = MAX_PATH;
static BYTE gSpinLockBuffer1[sizeof(IMAGEHLP_SYMBOL) + kSpinLockMaxNameLength + 1];
static BYTE gSpinLockBuffer2[sizeof(IMAGEHLP_SYMBOL) + kSpinLockMaxNameLength + 1];

static void printSpinLockInfo()
{
    if (!gSpinLockPrint) {
        SpinLockListRoot* listRoot = &gSpinLockObjectList;

        HANDLE curProcess = GetCurrentProcess();

        IMAGEHLP_SYMBOL* pSymbol = (IMAGEHLP_SYMBOL*)gSpinLockBuffer1;
        IMAGEHLP_LINE* pLine = (IMAGEHLP_LINE*)gSpinLockBuffer2;

        time_t timer;
        struct tm *lt;

        time(&timer);

        lt = localtime(&timer);

        char name[kSpinLockMaxNameLength + 1];

        int len = snprintf(name, kSpinLockMaxNameLength,
                            "spin_lock_%04d%02d%02d_%02d%02d%02d.csv",
                            lt->tm_year + 1900,
                            lt->tm_mon + 1,
                            lt->tm_mday,
                            lt->tm_hour,
                            lt->tm_min,
                            lt->tm_sec);
        if (len == kSpinLockMaxNameLength) {
            name[len] = 0;
        }

        try {
            ofstream ofs(name);

            int index = 0;

            for (SpinLockObject* obj = (SpinLockObject*)listRoot->head; obj != NULL; obj = obj->next) {
                ofs << ++index << endl;

                ofs << "Function,Line" << endl;

                for (SpinLockStackTraceNode* node = (SpinLockStackTraceNode*)obj->head; node != NULL; node = node->next) {
                    DWORD dwAddr = (DWORD)node->obj;

                    pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
                    pSymbol->MaxNameLength = kSpinLockMaxNameLength;
                    BOOL ret_sym = SymGetSymFromAddr(curProcess, dwAddr, 0, pSymbol);

                    if (ret_sym) {
                        ofs << "\"" << pSymbol->Name << "\"";

                        pLine->SizeOfStruct = sizeof(IMAGEHLP_LINE);
                        DWORD dwLineDisp = 0;
                        BOOL ret_line = SymGetLineFromAddr(curProcess, dwAddr, &dwLineDisp, pLine);

                        if (ret_line) {
                            ofs << "," << pLine->LineNumber;
                        }

                        ofs << endl;
                    }
                }

                ofs << endl << endl << endl;
            }
        } catch (std::ios_base::failure) {
        }

        gSpinLockPrint = true;
    }
}

static void initSpinLockStackTrace()
{
    if (!gSpinLockInit) {
        {
            unsigned int num = sizeof(gSpinLockObject) / sizeof(gSpinLockObject[0]);

            for (unsigned int i = 0; i < num; i++)
                initSpinLockObject(gSpinLockObject[i]);

            for (unsigned int i = 0; i < num; i++) {
                if (i > 0) {
                    gSpinLockObject[i].prev = &gSpinLockObject[i - 1];
                } else {
                    gSpinLockObject[i].prev = NULL;
                }

                if (i < num - 1) {
                    gSpinLockObject[i].next = &gSpinLockObject[i + 1];
                } else {
                    gSpinLockObject[i].next = NULL;
                }
            }

            SpinLockListRoot* listRoot = &gFreeSpinLockObject.root;
            initSpinLockListRoot(*listRoot);
            listRoot->head = &gSpinLockObject[0];
            listRoot->tail = &gSpinLockObject[num - 1];
            listRoot->len = num;
        }

        {
            unsigned int num = sizeof(gSpinLockStackTraceNode) / sizeof(gSpinLockStackTraceNode[0]);

            for (unsigned int i = 0; i < num; i++)
                initSpinLockStackTraceNode(gSpinLockStackTraceNode[i]);

            for (unsigned int i = 0; i < num; i++) {
                if (i > 0) {
                    gSpinLockStackTraceNode[i].prev = &gSpinLockStackTraceNode[i - 1];
                } else {
                    gSpinLockStackTraceNode[i].prev = NULL;
                }

                if (i < num - 1) {
                    gSpinLockStackTraceNode[i].next = &gSpinLockStackTraceNode[i + 1];
                } else {
                    gSpinLockStackTraceNode[i].next = NULL;
                }
            }

            SpinLockListRoot* listRoot = &gFreeSpinLockStackTraceNode.root;
            initSpinLockListRoot(*listRoot);
            listRoot->head = &gSpinLockStackTraceNode[0];
            listRoot->tail = &gSpinLockStackTraceNode[num - 1];
            listRoot->len = num;
        }

        gSpinLockInit = true;
        gSpinLockPrint = false;
    }
}
#endif  /* WKC_TCSPINLOCK_WIN_STACK_TRACE */

static void TCMalloc_SlowLock(volatile unsigned int* lockword);

// The following is a struct so that it can be initialized at compile time
struct TCMalloc_SpinLock {

  inline void Lock() {
    int r;
#if COMPILER(GCC)
#if PLATFORM(X86)
    __asm__ __volatile__
      ("xchgl %0, %1"
       : "=r"(r), "=m"(lockword_)
       : "0"(1), "m"(lockword_)
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
#endif
#ifdef WKC_TCSPINLOCK_WIN_STACK_TRACE
    if (!r) setSpinLockInfo();
#endif
    if (r) TCMalloc_SlowLock(&lockword_);
  }

  inline void Unlock() {
#if COMPILER(GCC)
#if PLATFORM(X86)
    __asm__ __volatile__
      ("movl $0, %0"
       : "=m"(lockword_)
       : "m" (lockword_)
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
#endif
#ifdef WKC_TCSPINLOCK_WIN_STACK_TRACE
    setSpinLockInfo();
#endif
  }
    // Report if we think the lock can be held by this thread.
    // When the lock is truly held by the invoking thread
    // we will always return true.
    // Indended to be used as CHECK(lock.IsHeld());
    inline bool IsHeld() const {
        return lockword_ != 0;
    }

    inline void Init() { lockword_ = 0; }

    volatile unsigned int lockword_;
};

#define SPINLOCK_INITIALIZER { 0 }

static void TCMalloc_SlowLock(volatile unsigned int* lockword) {
#ifdef WKC_TCSPINLOCK_WIN_STACK_TRACE
  int loopCounter = 0;
#endif
  sched_yield();        // Yield immediately since fast path failed
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
#endif
    if (!r) {
#ifdef WKC_TCSPINLOCK_WIN_STACK_TRACE
      setSpinLockInfo();
#endif
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
#if PLATFORM(WIN_OS)
    Sleep(2);
#else
    struct timespec tm;
    tm.tv_sec = 0;
    tm.tv_nsec = 2000001;
    nanosleep(&tm, NULL);
#endif

#ifdef WKC_TCSPINLOCK_WIN_STACK_TRACE
    if (++loopCounter == kMaxNanoSleepLoopCount) {
        printSpinLockInfo();
        throw(300);
    }
#endif
  }
}

#elif PLATFORM(WKC)
#include <wkc/wkcpeer.h>

#if COMPILER(RVCT)

static void TCMalloc_SlowLock(volatile unsigned int* lockword);

struct TCMalloc_SpinLock {

    inline void Lock() {
        int r;
        __asm
        {
            mov      r0, #1
            ldrex    r1, [this]
            teq      r1, #0
            strexeq  r2, r0, [this]
            mov      r, r1
        }
        if (r) TCMalloc_SlowLock(&lockword_);
    }

    inline void Unlock() {
        __asm
        {
            mov  r0, #0
            str  r0, [this]
        }
    }

    // Report if we think the lock can be held by this thread.
    // When the lock is truly held by the invoking thread
    // we will always return true.
    // Indended to be used as CHECK(lock.IsHeld());
    inline bool IsHeld() const {
        return lockword_ != 0;
    }

    inline void Init() {lockword_ = 0;}

    volatile unsigned int lockword_;
};

#define SPINLOCK_INITIALIZER { 0 }

static void TCMalloc_SlowLock(volatile unsigned int* lockword)
{
    wkcThreadYieldPeer();        // Yield immediately since fast path failed
    while (true) {
        int r;

        __asm
        {
            mov      r0, #1
            ldrex    r1, [lockword]
            teq      r1, #0
            strexeq  r2, r0, [lockword]
            mov      r, r1
        }

        if (!r) {
            return;
        }

        // Sleep for a few milliseconds
        struct timespec tm;
        tm.tv_sec = 0;
        tm.tv_nsec = 2000001;
        wkc_nanosleep(&tm, NULL);
    }
}

#else  // COMPILER(RVCT)

#define SPINLOCK_INIT_VALUE ((void*)(0x7f7f7f7f))

struct TCMalloc_SpinLock {
  void* private_lock_;

  inline void Init() {
//      private_lock_ = wkcMutexNewPeer();
//      if (!private_lock_) CRASH();
  }
  inline void Finalize() {
//      if (private_lock_ && private_lock_!=SPINLOCK_INIT_VALUE) {
//          wkcMutexDeletePeer(private_lock_);
//      }
  }
  inline void Lock() {
//      if (private_lock_==SPINLOCK_INIT_VALUE) {
//          Init();
//      }
//      wkcMutexLockPeer(private_lock_);
  }
  inline void Unlock() {
//      wkcMutexUnlockPeer(private_lock_);
  }
  bool IsHeld() {
//      if (private_lock_==SPINLOCK_INIT_VALUE) {
//          Init();
//      }
//      if (wkcMutexTryLockPeer(private_lock_))
      return true;
//      Unlock();
//      return false;
  }
};

#define SPINLOCK_INITIALIZER { SPINLOCK_INIT_VALUE }

#endif  // COMPILER(RVCT)

#else

#include <pthread.h>

// Portable version
struct TCMalloc_SpinLock {
  pthread_mutex_t private_lock_;

  inline void Init() {
    if (pthread_mutex_init(&private_lock_, NULL) != 0) CRASH();
  }
  inline void Finalize() {
    if (pthread_mutex_destroy(&private_lock_) != 0) CRASH();
  }
  inline void Lock() {
    if (pthread_mutex_lock(&private_lock_) != 0) CRASH();
  }
  inline void Unlock() {
    if (pthread_mutex_unlock(&private_lock_) != 0) CRASH();
  }
  bool IsHeld() {
    if (pthread_mutex_trylock(&private_lock_))
      return true;

    Unlock();
    return false;
  }
};

#define SPINLOCK_INITIALIZER { PTHREAD_MUTEX_INITIALIZER }

#endif

// Corresponding locker object that arranges to acquire a spinlock for
// the duration of a C++ scope.
class TCMalloc_SpinLockHolder {
 private:
  TCMalloc_SpinLock* lock_;
 public:
  inline explicit TCMalloc_SpinLockHolder(TCMalloc_SpinLock* l)
    : lock_(l) {
#ifdef WKC_TCSPINLOCK_WIN_STACK_TRACE
      initSpinLockStackTrace();
#endif
      l->Lock();
  }
  inline ~TCMalloc_SpinLockHolder() { lock_->Unlock(); }
};

// Short-hands for convenient use by tcmalloc.cc
typedef TCMalloc_SpinLock SpinLock;
typedef TCMalloc_SpinLockHolder SpinLockHolder;

#endif  // TCMALLOC_INTERNAL_SPINLOCK_H__
