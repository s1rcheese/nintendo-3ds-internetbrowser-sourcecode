/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2008 Christian Dywan <christian@imendio.com>
 * Copyright (C) 2008 Collabora Ltd.
 * Copyright (C) 2009 Holger Hans Peter Freyther
 * All rights reserved.
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
#include "PlatformScreen.h"

#include "HostWindow.h"
#include "ScrollView.h"
#include "Widget.h"

namespace {

int gScreenWidth = 0;
int gScreenHeight = 0;
int gAvailableScreenWidth = 0;
int gAvailableScreenHeight = 0;
int gScreenDepth = 0;
int gScreenDepthPerComponent = 0;
bool gIsMonochrome = false;

}

namespace WebCore {

int screenDepth(Widget* widget)
{
    if (!widget) {
        return 0;
    }
    return gScreenDepth;
}

int screenDepthPerComponent(Widget* widget)
{
    if (!widget) {
        return 0;
    }
    return gScreenDepthPerComponent;
}

bool screenIsMonochrome(Widget* widget)
{
    if (!widget) {
        return false;
    }
    return gIsMonochrome;
}

FloatRect screenRect(Widget* widget)
{
    if (!widget) {
        return FloatRect();
    }

    return FloatRect(IntPoint(0,0), IntSize(gScreenWidth, gScreenHeight));
}

void setScreenSizeWKC(const IntSize& size)
{
    gScreenWidth = size.width();
    gScreenHeight = size.height();
}

void setAvailableScreenSize(const IntSize& size)
{
    gAvailableScreenWidth = size.width();
    gAvailableScreenHeight = size.height();
}

void setScreenDepth(int depth, int depth_per_component)
{
    gScreenDepth = depth;
    gScreenDepthPerComponent = depth_per_component;
}

void setIsMonochrome(bool monochrome)
{
    gIsMonochrome = monochrome;
}

FloatRect screenAvailableRect(Widget* widget)
{
    if (!widget) {
        return FloatRect();
    }

    return FloatRect(IntPoint(0,0), IntSize(gAvailableScreenWidth, gAvailableScreenHeight));
}

} // namespace WebCore
