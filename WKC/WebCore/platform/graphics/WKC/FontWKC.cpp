/*
 * Copyright (C) 2007 Kevin Ollivier.  All rights reserved.
 * Copyright (c) 2010, 2012 ACCESS CO., LTD. All rights reserved.
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
#include "Font.h"

#include "FontFallbackList.h"
#include "GlyphBuffer.h"
#include "GraphicsContext.h"
#include "IntRect.h"
#include "NotImplemented.h"
#include "SimpleFontData.h"

#include <wkc/wkcpeer.h>

namespace WebCore {

static UChar fixedChar(const UChar c)
{
    if (Font::treatAsSpace(c)) {
        return 0x20;
    } else if (c==0x200b || (Font::treatAsZeroWidthSpace(c) || c==0xfeff)) {
        return 0;
    }
    return c;
}

bool Font::canReturnFallbackFontsForComplexText()
{
    return false;
}

void Font::drawGlyphs(GraphicsContext* graphicsContext, const SimpleFontData* sfont, const GlyphBuffer& glyphBuffer, 
                      int from, int numGlyphs, const FloatPoint& point) const
{
    int i = 0;
    FloatPoint pos(point);
    float w=0, h=0;
    UChar c=0;
    void* dc = (void *)graphicsContext->platformContext();
    WKCPeerFont pf;
    WKCFloatRect clip;
    WKCFloatPoint pp;

    if (!dc) return;

    pf.fFont = sfont->platformData().Font();
    if (!pf.fFont) return;
    const FontPlatformData& pd(sfont->platformData());

    pf.fRequestedSize = pd.requestSize();
    pf.fCreatedSize = pd.createdSize();
    pf.fWeight = pd.weight();
    pf.fItalic = pd.isItalic();
    pf.fScale = pd.scale();
    pf.fiScale = pd.iscale();
    pf.fCanScale = pd.canScale();
    pf.fFontId = (void *)pd.hash();

    pos.setY(pos.y() - (float)pd.ascent()*pd.scale());

    h = (pd.lineSpacing()) * pd.scale();
    for (i=0; i<numGlyphs; i++) {
        float advance = 0.f;
        c = fixedChar(glyphBuffer.glyphAt(from + i));
        if (!c) continue;
        advance = glyphBuffer.advanceAt(from + i);
        w = wkcFontGetClipWidthPeer(pf.fFont, c) * pd.scale();
        clip.fX = pos.x();
        clip.fY = pos.y();
        clip.fWidth = w;
        clip.fHeight = h;
        pp.fX = pos.x();
        pp.fY = pos.y();
        wkcDrawContextDrawCharPeer(dc, (unsigned int)c, &pp, &clip, &pf);

        pos.move(advance, 0);
    }
}

// Ugh!: following "complex" drawings are tiny version!
// 1080831 ACCESS Co.,Ltd.

FloatRect Font::selectionRectForComplexText(const TextRun& run, const IntPoint& point, int h, int from, int to) const
{
    void* font = 0;
    const UChar* str = 0;
    const SimpleFontData* sfont = 0;
    int i, len;
    float x0,x1,x2;
    float scale = 0;
    UChar c = 0;

    len = run.length();
    if (!len) return FloatRect();
    if (from>=len || to>len) return FloatRect();
    if (from>=to) return FloatRect();
    if (from<0) from = 0;
    if (to>len) to = len;

    str = run.characters();
    sfont = primaryFont();
    if (!sfont) return FloatRect();
    font = sfont->platformData().Font();
    if (!font) return FloatRect();
    scale = sfont->platformData().scale();

    x0=x1=x2=0;
    for (i=0; i<from; i++) {
        c = fixedChar(str[i]);
        if (!c) continue;
        x0 += ((float)wkcFontGetCharWidthPeer(font, c) * scale);
    }
    for (i=from; i<to; i++) {
        c = fixedChar(str[i]);
        if (!c) continue;
        x1 += ((float)wkcFontGetCharWidthPeer(font, c) * scale);
    }
    for (i=to; i<len; i++) {
        c = fixedChar(str[i]);
        if (!c) continue;
        x2 += ((float)wkcFontGetCharWidthPeer(font, c) * scale);
    }

    if (run.rtl()) {
        return FloatRect(point.x() + x2, point.y(), x1, h);
    } else {
        return FloatRect(point.x() + x0, point.y(), x1, h);
    }
}

void Font::drawComplexText(GraphicsContext* graphicsContext, const TextRun& run, const FloatPoint& point, int from, int to) const
{
    const UChar* str = 0;
    void* dc = (void *)graphicsContext->platformContext();
    const SimpleFontData* sfont = primaryFont();
    float scale = 0;
    int i, len;
    UChar c;
    WKCPeerFont pf;
    WKCFloatRect clip;
    WKCFloatPoint pos;
    int a, d;

    if (!dc) return;
    if (!sfont) return;

    len = run.length();
    if (!len) return;
    if (from>=len || to>len) return;
    if (from>=to) return;
    if (from<0) from = 0;
    if (to>len) to = len;

    pf.fFont = sfont->platformData().Font();
    if (!pf.fFont) return;
    const FontPlatformData& pd(sfont->platformData());
    pf.fRequestedSize = pd.requestSize();
    pf.fCreatedSize = pd.createdSize();
    pf.fWeight = pd.weight();
    pf.fItalic = pd.isItalic();
    pf.fScale = pd.scale();
    pf.fiScale = pd.iscale();
    pf.fCanScale = pd.canScale();
    pf.fFontId = (void *)pd.hash();
    scale = pd.scale();

    str = run.characters();

    a = (float)pd.ascent() * scale;
    d = (float)pd.descent() * scale;
    pos.fY = point.y() - a;

    clip.fHeight = a+d;
    clip.fY = pos.fY;

    if (run.rtl()) {
        float x0=0,x1=0,x2=0;
        for (i=0; i<from; i++) {
            c = fixedChar(str[i]);
            if (!c) continue;
            x0 += (float)wkcFontGetCharWidthPeer(pf.fFont, c) * scale;
        }
        for (i=from; i<to; i++) {
            c = fixedChar(str[i]);
            if (!c) continue;
            x1 += (float)wkcFontGetCharWidthPeer(pf.fFont, c) * scale;
        }
        for (i=to; i<len; i++) {
            c = fixedChar(str[i]);
            if (!c) continue;
            x2 += (float)wkcFontGetCharWidthPeer(pf.fFont, c) * scale;
        }
        pos.fX = point.x() + x2 + x1;
        for (i=from; i<to; i++) {
            c = fixedChar(str[i]);
            if (!c) continue;
            clip.fWidth = (float)wkcFontGetCharWidthPeer(pf.fFont, c) * scale;
            pos.fX -= clip.fWidth;
            clip.fX = pos.fX;
            wkcDrawContextDrawCharPeer(dc, (unsigned int)c, &pos, &clip, &pf);
        }
    } else {
        pos.fX = point.x();
        for (i=0; i<from; i++) {
            c = fixedChar(str[i]);
            if (!c) continue;
            pos.fX += (float)wkcFontGetCharWidthPeer(pf.fFont, c) * scale;
        }
        for (i=from; i<to; i++) {
            c = fixedChar(str[i]);
            if (!c) continue;
            clip.fX = pos.fX;
            clip.fWidth = (float)wkcFontGetCharWidthPeer(pf.fFont, c) * scale;
            wkcDrawContextDrawCharPeer(dc, (unsigned int)c, &pos, &clip, &pf);
            pos.fX += clip.fWidth;
        }
    }
}

float Font::floatWidthForComplexText(const TextRun& run, HashSet<const SimpleFontData*>* /* fallbackFonts */) const
{
    void* font = 0;
    float w = 0.f;
    const UChar* str = 0;
    UChar c = 0;
    const SimpleFontData* sfont = primaryFont();
    float scale = 0;
    int i, len;

    if (!sfont) return 0.f;

    len = run.length();
    if (!len) return 0.f;

    str = run.characters();

    font = sfont->platformData().Font();
    if (!font) return 0.f;
    scale = sfont->platformData().scale();

    for (i=0; i<len; i++) {
        c = fixedChar(str[i]);
        if (!c) continue;
        w += (float)wkcFontGetCharWidthPeer(font, c) * scale;
    }

    return w;
}

int Font::offsetForPositionForComplexText(const TextRun& run, int x, bool includePartialGlyphs) const
{
    void* font = 0;
    float w = 0.f;
    const UChar* str = 0;
    UChar c = 0;
    const SimpleFontData* sfont = 0;
    int i, len;
    float delta = (float)x;
    float scale = 0;

    len = run.length();
    if (!len) return 0;

    str = run.characters();
    sfont = primaryFont();
    font = sfont->platformData().Font();
    if (!font) return 0;
    scale = sfont->platformData().scale();

    i = 0;
    if (run.rtl()) {
        delta -= floatWidthForComplexText(run, 0);
        while (i<len) {
            c = fixedChar(str[i]);
            if (c) {
                w = (float)wkcFontGetCharWidthPeer(font, c) * scale;
            } else {
                w = 0;
            }
            delta += w;
            if (includePartialGlyphs) {
                if (delta - w / 2 >= 0)
                    break;
            } else {
                if (delta >= 0)
                    break;
            }
            i++;
        }
    } else {
        while (i<len) {
            c = fixedChar(str[i]);
            if (c) {
                w = (float)wkcFontGetCharWidthPeer(font, c) * scale;
            } else {
                w = 0;
            }
            delta -= w;
            if (includePartialGlyphs) {
                if (delta + w / 2 <= 0)
                    break;
            } else {
                if (delta <= 0)
                    break;
            }
            i++;
        }
    }

    return i;
}

}
