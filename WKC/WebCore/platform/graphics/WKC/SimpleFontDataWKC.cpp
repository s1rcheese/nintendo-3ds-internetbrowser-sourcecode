/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007, 2008 Alp Toker <alp@atoker.com>
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * All rights reserved.
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

#include "config.h"
#include "SimpleFontData.h"

#include "FloatRect.h"
#include "Font.h"
#include "FontCache.h"
#include "FontDescription.h"
#include "GlyphBuffer.h"

#include "NotImplemented.h"

#include <wkc/wkcpeer.h>

namespace WebCore {

void SimpleFontData::platformInit()
{
    void* font = m_platformData.Font();
    float scale = m_platformData.scale();
    if (!font) return;

    m_ascent = (float)m_platformData.ascent() * scale;
    m_descent = (float)m_platformData.descent() * scale;
    m_lineSpacing = (float)m_platformData.lineSpacing() * scale;
    m_xHeight = (float)wkcFontGetXHeightPeer(font) * scale;
    m_unitsPerEm = 1;
    m_lineGap = (float)wkcFontGetLineGapPeer(font) * scale;
}

void SimpleFontData::platformCharWidthInit()
{
    void* font = m_platformData.Font();
    float scale = m_platformData.scale();

    if (!font) return;
    //m_avgCharWidth = wkcFontGetAverageCharWidthPeer(font);
	/* use small x for the average char width */
    m_avgCharWidth = (float)wkcFontGetCharWidthPeer(font, 0x0078) * scale; 
	m_maxCharWidth = (float)wkcFontGetMaxCharWidthPeer(font) * scale;
}

void SimpleFontData::platformDestroy()
{
}

SimpleFontData* SimpleFontData::smallCapsFontData(const FontDescription& fontDescription) const
{
    if (!m_smallCapsFontData){
        FontDescription desc = FontDescription(fontDescription);
        desc.setSpecifiedSize(0.70f*fontDescription.computedSize());
        const FontPlatformData* pdata = new FontPlatformData(desc, desc.family().family());
        m_smallCapsFontData = new SimpleFontData(*pdata);
		delete pdata; // fixed Coverity ID 516
    }
    return m_smallCapsFontData;
}

bool SimpleFontData::containsCharacters(const UChar* characters, int length) const
{
    return false;
}

void SimpleFontData::determinePitch()
{
    void* font = m_platformData.Font();

    if (!font) return;

    m_treatAsFixedPitch = wkcFontIsFixedFontPeer(font);
}

float SimpleFontData::platformWidthForGlyph(Glyph glyph) const
{
    void* font = m_platformData.Font();
    const float scale = m_platformData.scale();

    if (!font) return 0;

    if (Font::treatAsSpace(glyph)) {
        glyph = 0x20;
    } else if ((Font::treatAsZeroWidthSpace(glyph) || glyph==0xfeff)) {
        return 0.f;
    }

    const int w = wkcFontGetCharWidthPeer(font, (unsigned int)glyph);

    if (scale==1.f) {
        return (float)w;
    } else {
        return (float)w * scale;
    }
}

void SimpleFontData::initCharWidths()
{
	platformCharWidthInit();
}

void SimpleFontData::platformGlyphInit()
{
	void* font = m_platformData.Font();
    const float scale = m_platformData.scale();

    if (!font) return;

    const Glyph spaceGlyph = 0x0020;
    m_spaceWidth = (float)wkcFontGetCharWidthPeer(font, spaceGlyph) * scale;
	m_glyphToWidthMap.setWidthForGlyph(spaceGlyph, m_spaceWidth);
    determinePitch();
    m_adjustedSpaceWidth = m_spaceWidth;
    const Glyph zeroWidthSpaceGlyph = 0x0000;
	m_glyphToWidthMap.setWidthForGlyph(zeroWidthSpaceGlyph, 0);
    m_missingGlyphData.fontData = this;
    m_missingGlyphData.glyph = 0;
}

}
