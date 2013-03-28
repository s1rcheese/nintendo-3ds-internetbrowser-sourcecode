/*
 * Copyright (C) 2008 Alp Toker <alp@atoker.com>
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "FontCache.h"

#include "Font.h"
#include "SimpleFontData.h"
#include <wtf/Assertions.h>

#include "NotImplemented.h"

namespace WebCore {

void FontCache::platformInit()
{
}

const SimpleFontData* FontCache::getFontDataForCharacters(const Font& font, const UChar* characters, int length)
{
    SimpleFontData* newfont = NULL;
    newfont = new SimpleFontData(FontPlatformData(font.fontDescription(), font.family().family()));
    return newfont;
}

FontPlatformData* FontCache::getSimilarFontPlatformData(const Font& font)
{
    return new FontPlatformData(font.fontDescription(), font.family().family());
}

FontPlatformData* FontCache::getLastResortFallbackFont(const FontDescription& fontDescription)
{
    static AtomicString timesStr("systemfont");
    return getCachedFontPlatformData(fontDescription, timesStr);
}

void FontCache::getTraitsInFamily(const AtomicString& familyName, Vector<unsigned>& traitsMasks)
{
    notImplemented();
}

FontPlatformData* FontCache::createFontPlatformData(const FontDescription& fontDescription, const AtomicString& family)
{
    return new FontPlatformData(fontDescription, family);
}

}
