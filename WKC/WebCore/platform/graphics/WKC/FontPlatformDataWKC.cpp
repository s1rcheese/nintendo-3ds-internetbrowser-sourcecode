/*
 * Copyright (C) 2007 Kevin Ollivier <kevino@theolliviers.com>
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
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

#include "config.h"
#include "FontPlatformData.h"

#include "FontDescription.h"
#include "PlatformString.h"

#include <wkc/wkcpeer.h>

namespace WebCore {

static bool gEnableScalingMonosizeFont = true;

void
FontPlatformData::enableScalingMonosizeFont(bool flag)
{
    gEnableScalingMonosizeFont = flag;
}

void
FontPlatformData::resetVariables()
{
    gEnableScalingMonosizeFont = true;
}

FontPlatformData::FontPlatformData(const FontDescription& in_desc, const AtomicString& in_family)
{
    int size = 0;
    int weight = WKC_FONT_WEIGHT_NORMAL;
    int family = WKC_FONT_FAMILY_NONE;
    bool italic = in_desc.italic();

    switch (in_desc.genericFamily()) {
    case FontDescription::NoFamily:
        family = WKC_FONT_FAMILY_NONE;
        break;
    case FontDescription::StandardFamily:
        family = WKC_FONT_FAMILY_STANDARD;
        break;
    case FontDescription::SerifFamily:
        family = WKC_FONT_FAMILY_SERIF;
        break;
    case FontDescription::SansSerifFamily:
        family = WKC_FONT_FAMILY_SANSSERIF;
        break;
    case FontDescription::MonospaceFamily:
        family = WKC_FONT_FAMILY_MONOSPACE;
        break;
    case FontDescription::CursiveFamily:
        family = WKC_FONT_FAMILY_CURSIVE;
        break;
    case FontDescription::FantasyFamily:
        family = WKC_FONT_FAMILY_FANTASY;
        break;
    }
    switch (in_desc.weight()) {
    case FontWeight100:
        weight = WKC_FONT_WEIGHT_100;
        break;
    case FontWeight200:
        weight = WKC_FONT_WEIGHT_200;
        break;
    case FontWeight300:
        weight = WKC_FONT_WEIGHT_300;
        break;
    case FontWeight400:
        weight = WKC_FONT_WEIGHT_400;
        break;
    case FontWeight500:
        weight = WKC_FONT_WEIGHT_500;
        break;
    case FontWeight600:
        weight = WKC_FONT_WEIGHT_600;
        break;
    case FontWeight700:
        weight = WKC_FONT_WEIGHT_700;
        break;
    case FontWeight800:
        weight = WKC_FONT_WEIGHT_800;
        break;
    case FontWeight900:
        weight = WKC_FONT_WEIGHT_900;
        break;
    }
    m_requestSize = in_desc.computedPixelSize();
    size = m_requestSize;
    m_weight = weight;
    m_italic = italic;

    m_font = wkcFontNewPeer(size, weight, italic, family, in_family.string().utf8().data());
    m_fontState = VALID;

    m_createdSize = wkcFontGetSizePeer(m_font);
    m_ascent = wkcFontGetAscentPeer(m_font);
    m_descent = wkcFontGetDescentPeer(m_font);
    m_lineSpacing = wkcFontGetLineSpacingPeer(m_font);
    m_canScale = wkcFontCanScalePeer(m_font);

    m_scale = 1.f;
    if (gEnableScalingMonosizeFont && m_createdSize) {
        m_scale = (float)m_requestSize / (float)m_createdSize;
    }
    m_iscale = 1.f / m_scale;

    m_hashValue = calculateHash();
}

FontPlatformData::FontPlatformData(const FontPlatformData& other)
{
	m_font = other.m_font ? wkcFontNewCopyPeer(other.m_font) : 0;
	m_fontState = other.m_fontState;

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
}

unsigned FontPlatformData::calculateHash() const
{
	if (m_font == 0) {
		return ~0;
	}

    if (m_scale!=1.f) {
        unsigned char buf[64];
        unsigned char* p = buf;
        memset(buf, 0, sizeof(buf));

        *((int *)p) = m_requestSize;
        p+=sizeof(int);
        *((int *)p) = m_weight;
        p+=sizeof(int);
        *((int *)p) = m_italic;
        p+=sizeof(int);

        int len = wkcFontSizeOfFontPeer();
        if (len > sizeof(buf) - sizeof(int)*3) {
            len = sizeof(buf) - sizeof(int)*3;
        }
        memcpy(p, m_font, len);

        return StringImpl::computeHash(reinterpret_cast<UChar*>(buf), sizeof(buf)/sizeof(UChar));
    } else {
        return StringImpl::computeHash(reinterpret_cast<UChar*>(m_font), wkcFontSizeOfFontPeer() / sizeof(UChar));
    }
}

FontPlatformData::~FontPlatformData()
{
    m_fontState = UNINITIALIZED;
    if (m_font) {
        wkcFontDeletePeer(m_font);
    }
    m_font = 0;
}

#ifndef NDEBUG
String FontPlatformData::description() const
{
    return String();
}
#endif

}
