//
// FastMallocSystemWKC.h
// Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
//


#ifndef WTF_FastMallocSystemWKC_h

#define WTF_FastMallocSystemWKC_h

namespace WTF {
#if COMPILER(MSVC) && defined(_DEBUG)
void* fastSystemMallocX(size_t size, const char* fname, int line);
#define fastSystemMalloc(size) fastSystemMallocX(size, __FILE__, __LINE__)
void* fastSystemReallocX(void* ptr, size_t size, const char* fname, int line);
#define fastSystemRealloc(ptr, size) fastSystemReallocX(ptr, size, __FILE__, __LINE__)
#else
void* fastSystemMalloc(size_t size);
void* fastSystemRealloc(void* ptr, size_t size);
#endif
void fastSystemFree(void* ptr);
}

#endif  /* WTF_FastMallocSystemWKC_h */
