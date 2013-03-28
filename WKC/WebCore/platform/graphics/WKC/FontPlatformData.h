/*
 * Copyright (C) 2006 Kevin Ollivier  All rights reserved.
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#ifndef FontPlatformData_h
#define FontPlatformData_h

#include "FontDescription.h"
#include "AtomicString.h"
#include "CString.h"
#include "StringImpl.h"
#include <wtf/RefPtr.h>

namespace WebCore {

class String;

class FontPlatformData {
public:
    enum FontState { UNINITIALIZED, DELETED, VALID };

    FontPlatformData(WTF::HashTableDeletedValueType)
    : m_fontState(DELETED),
      m_font(0),
      m_requestSize(0),
      m_createdSize(0),
      m_weight(0),
      m_italic(false),
      m_scale(1),
      m_iscale(1),
      m_canScale(false),
      m_ascent(0),
      m_descent(0),
      m_lineSpacing(0),
      m_hashValue((unsigned)-1)
    { }

    ~FontPlatformData();

    FontPlatformData(const FontDescription&, const AtomicString&);
    FontPlatformData(float size, bool bold, bool italic)
    : m_fontState(UNINITIALIZED)
    , m_font(0),
      m_requestSize(0),
      m_createdSize(0),
      m_weight(0),
      m_italic(false),
      m_scale(1),
      m_iscale(1),
      m_canScale(false),
      m_ascent(0),
      m_descent(0),
      m_lineSpacing(0),
      m_hashValue((unsigned)-1)
    { }
    FontPlatformData(const FontPlatformData&);
    
    FontPlatformData() 
    : m_fontState(UNINITIALIZED)
    , m_font(0),
      m_requestSize(0),
      m_createdSize(0),
      m_weight(0),
      m_italic(false),
      m_scale(1),
      m_iscale(1),
      m_canScale(false),
      m_ascent(0),
      m_descent(0),
      m_lineSpacing(0),
      m_hashValue((unsigned)-1)
    { }
    
    unsigned hash() const {
        switch (m_fontState) {
        case DELETED:
            return (unsigned)-1;
        case UNINITIALIZED:
            return 0;
        case VALID:
            return m_hashValue;
        default:
            return (unsigned)-1;
        }
    }

    bool operator==(const FontPlatformData& other) const
    { 
        if (m_font && m_fontState == VALID && other.m_fontState == VALID && other.m_font) {
            return wkcFontIsEqualPeer(m_font, other.m_font) && (m_hashValue == other.m_hashValue);
        }
        else
            return m_fontState == other.m_fontState;
    }

    FontPlatformData& operator=(const FontPlatformData& other)
    {
        this->~FontPlatformData();
        m_fontState = other.m_fontState;
        if (other.m_font) {
            m_font = other.m_font ? wkcFontNewCopyPeer(other.m_font) : 0;
        }
        m_requestSize = other.m_requestSize;
        m_createdSize = other.m_createdSize;
        m_weight = other.m_weight;
        m_italic = other.m_italic;
        m_scale = other.m_scale;
        m_iscale = other.m_iscale;
        m_canScale = other.m_canScale;
        m_ascent = other.m_ascent;
        m_descent = other.m_descent;
        m_lineSpacing = other.m_lineSpacing;
        m_hashValue = calculateHash();
        return *this;
    }

    bool isHashTableDeletedValue() const { return m_fontState == DELETED; }

    static void enableScalingMonosizeFont(bool flag);
    static void resetVariables();
    
#ifndef NDEBUG
    String description() const;
#endif

    inline void* Font() const { return m_font; };

    inline int requestSize() const { return m_requestSize; };
    inline int createdSize() const { return m_createdSize; };
    inline int weight() const { return m_weight; };
    inline bool isItalic() const { return m_italic; };
    inline float scale() const { return m_scale; };
    inline float iscale() const { return m_iscale; };
    inline bool canScale() const { return m_canScale; };

    inline int ascent() const { return m_ascent; };
    inline int descent() const { return m_descent; };
    inline int lineSpacing() const { return m_lineSpacing; };

private:
    unsigned calculateHash() const;

private:
    FontState m_fontState;

    void* m_font;
    int m_requestSize;
    int m_createdSize;
    int m_weight;
    bool m_italic;
    float m_scale;
    float m_iscale;
    bool m_canScale;

    int m_ascent;
    int m_descent;
    int m_lineSpacing;

    unsigned m_hashValue;
};

}

#endif
