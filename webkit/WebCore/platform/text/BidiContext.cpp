/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003, 2004, 2006, 2007, 2009 Apple Inc. All right reserved.
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
#include "BidiContext.h"

#include <wtf/StdLibExtras.h>

namespace WebCore {

using namespace WTF::Unicode;

#if PLATFORM(WKC)
static BidiContext* gLtrContext = 0;
static BidiContext* gLtrOverrideContext = 0;
static BidiContext* gRtlContext = 0;
static BidiContext* gRtlOverrideContext = 0;
#endif

PassRefPtr<BidiContext> BidiContext::create(unsigned char level, Direction direction, bool override, BidiContext* parent)
{
    ASSERT(direction == (level % 2 ? RightToLeft : LeftToRight));

    if (parent)
        return adoptRef(new BidiContext(level, direction, override, parent));

    ASSERT(level <= 1);
    if (!level) {
#if PLATFORM(WKC)
        if (!gLtrContext) {
            gLtrContext = new BidiContext(0, LeftToRight, false, 0);
        }
        BidiContext& ltrContext = *gLtrContext;
#else
        DEFINE_STATIC_LOCAL(BidiContext, ltrContext, (0, LeftToRight, false, 0));
#endif
        if (!override)
            return &ltrContext;

#if PLATFORM(WKC)
        if (!gLtrOverrideContext) {
            gLtrOverrideContext = new BidiContext(0, LeftToRight, true, 0);
        }
        BidiContext& ltrOverrideContext = *gLtrOverrideContext;
#else
        DEFINE_STATIC_LOCAL(BidiContext, ltrOverrideContext, (0, LeftToRight, true, 0));
#endif
        return &ltrOverrideContext;
    }

#if PLATFORM(WKC)
    if (!gRtlContext) {
        gRtlContext = new BidiContext(1, RightToLeft, false, 0);
    }
    BidiContext& rtlContext = *gRtlContext;
#else
    DEFINE_STATIC_LOCAL(BidiContext, rtlContext, (1, RightToLeft, false, 0));
#endif
    if (!override)
        return &rtlContext;

#if PLATFORM(WKC)
    if (!gRtlOverrideContext) {
        gRtlOverrideContext = new BidiContext(1, RightToLeft, true, 0);
    }
    BidiContext& rtlOverrideContext = *gRtlOverrideContext;
#else
    DEFINE_STATIC_LOCAL(BidiContext, rtlOverrideContext, (1, RightToLeft, true, 0));
#endif
    return &rtlOverrideContext;
}

bool operator==(const BidiContext& c1, const BidiContext& c2)
{
    if (&c1 == &c2)
        return true;
    if (c1.level() != c2.level() || c1.override() != c2.override() || c1.dir() != c2.dir())
        return false;
    if (!c1.parent())
        return !c2.parent();
    return c2.parent() && *c1.parent() == *c2.parent();
}

#if PLATFORM(WKC)
void BidiContext::deleteSharedInstance()
{
    delete gLtrContext;
    delete gLtrOverrideContext;
    delete gRtlContext;
    delete gRtlOverrideContext;
}

void BidiContext::resetVariables()
{
    gLtrContext = 0;
    gLtrOverrideContext = 0;
    gRtlContext = 0;
    gRtlOverrideContext = 0;
}
#endif

} // namespace WebCore
