//
// FastMallocSystemWKC.cpp
// Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
//

#include "config.h"

#include <cstdlib>

#if COMPILER(MSVC) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#endif

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "FastMallocSystemWKC.h"

namespace WTF {

#if COMPILER(MSVC)

#ifdef _CRTDBG_MAP_ALLOC
void* fastSystemMallocX(size_t size, const char* fname, int line)
{
    return _malloc_dbg(size, _NORMAL_BLOCK, fname, line);
}
#else
void* fastSystemMalloc(size_t size)
{
    return malloc(size);
}
#endif

#ifdef _CRTDBG_MAP_ALLOC
void* fastSystemReallocX(void* ptr, size_t size, const char* fname, int line)
{
    return _realloc_dbg(ptr, size, _NORMAL_BLOCK, fname, line);
}
#else
void* fastSystemRealloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}
#endif

void fastSystemFree(void* ptr)
{
    free(ptr);
}

#endif /* COMPILER(MSVC) */

}   // namespace

