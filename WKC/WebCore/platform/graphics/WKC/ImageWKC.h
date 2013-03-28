/*
 * Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef ImageWKC_h
#define ImageWKC_h

#include "IntSize.h"

namespace WebCore {

class RGBA32Buffer;

class ImageWKC {
public:
    ImageWKC(RGBA32Buffer* buffer);
    ImageWKC(void* bitmap, const IntSize& size, int type, bool ownbitmap=true);
    ~ImageWKC();

    void* bitmap() const;
    int rowbytes() const;
    void* mask() const;
    int maskrowbytes() const;
    const IntSize& size();
    enum {
        EColorARGB8888, // 
        EColorRGAB5515, // bit6: on->opaque off->colored
        EColorRGAB5515MASK, // bit6: on->opaque off->colored, with mask
        EColorARGB1232,
        EColors
    };
    int type() const;
    bool hasAlpha() const;
    bool hasTrueAlpha() const;
    double scalex() const;
    double scaley() const;

private:
    void* m_bitmap;
    RGBA32Buffer* m_buffer;
    int m_rowbytes;
    IntSize m_size;
    int m_type;
    bool m_ownbitmap;
};

} // namespace

#endif // ImageWKC_h
