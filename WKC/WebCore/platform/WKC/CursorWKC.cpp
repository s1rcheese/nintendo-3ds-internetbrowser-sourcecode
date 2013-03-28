/*
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007 Christian Dywan <christian@twotoasts.de>
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

#include "Image.h"
#include "IntPoint.h"

#include "NotImplemented.h"
#include <wtf/Assertions.h>

#include "Cursor.h"

namespace WebCore {

static Cursor cursorDefault;

Cursor::Cursor(const Cursor& other)
{
    m_impl = 0;
}

Cursor::Cursor(Image* image, const IntPoint& hotSpot)
{
    m_impl = 0;
}

Cursor::~Cursor()
{
}

Cursor& Cursor::operator=(const Cursor& other)
{
    m_impl = other.m_impl;
    return *this;
}

const Cursor& pointerCursor()
{
    return cursorDefault;
}

const Cursor& crossCursor()
{
    return cursorDefault;
}

const Cursor& handCursor()
{
    return cursorDefault;
}

const Cursor& moveCursor()
{
    return cursorDefault;
}

const Cursor& iBeamCursor()
{
    return cursorDefault;
}

const Cursor& waitCursor()
{
    return cursorDefault;
}

const Cursor& helpCursor()
{
    return cursorDefault;
}

const Cursor& eastResizeCursor()
{
    return cursorDefault;
}

const Cursor& northResizeCursor()
{
    return cursorDefault;
}

const Cursor& northEastResizeCursor()
{
    return cursorDefault;
}

const Cursor& northWestResizeCursor()
{
    return cursorDefault;
}

const Cursor& southResizeCursor()
{
    return cursorDefault;
}

const Cursor& southEastResizeCursor()
{
    return cursorDefault;
}

const Cursor& southWestResizeCursor()
{
    return cursorDefault;
}

const Cursor& westResizeCursor()
{
    return cursorDefault;
}

const Cursor& northSouthResizeCursor()
{
    return cursorDefault;
}

const Cursor& eastWestResizeCursor()
{
    return cursorDefault;
}

const Cursor& northEastSouthWestResizeCursor()
{
    return cursorDefault;
}

const Cursor& northWestSouthEastResizeCursor()
{
    return cursorDefault;
}

const Cursor& columnResizeCursor()
{
    return cursorDefault;
}

const Cursor& rowResizeCursor()
{
    return cursorDefault;
}
    
const Cursor& middlePanningCursor()
{
    return moveCursor();
}

const Cursor& eastPanningCursor()
{
    return eastResizeCursor();
}

const Cursor& northPanningCursor()
{
    return northResizeCursor();
}

const Cursor& northEastPanningCursor()
{
    return northEastResizeCursor();
}

const Cursor& northWestPanningCursor()
{
    return northWestResizeCursor();
}

const Cursor& southPanningCursor()
{
    return southResizeCursor();
}

const Cursor& southEastPanningCursor()
{
    return southEastResizeCursor();
}

const Cursor& southWestPanningCursor()
{
    return southWestResizeCursor();
}

const Cursor& westPanningCursor()
{
    return westResizeCursor();
}
    

const Cursor& verticalTextCursor()
{
    return cursorDefault;
}

const Cursor& cellCursor()
{
    return cursorDefault;
}

const Cursor& contextMenuCursor()
{
    return cursorDefault;
}

const Cursor& noDropCursor()
{
    return cursorDefault;
}

const Cursor& copyCursor()
{
    return cursorDefault;
}

const Cursor& progressCursor()
{
    return cursorDefault;
}

const Cursor& aliasCursor()
{
    return cursorDefault;
}

const Cursor& noneCursor()
{
    return cursorDefault;
}

const Cursor& notAllowedCursor()
{
    return noDropCursor();
}

const Cursor& zoomInCursor()
{
    return cursorDefault;
}

const Cursor& zoomOutCursor()
{
    return cursorDefault;
}

const Cursor& grabCursor()
{
    return cursorDefault;
}

const Cursor& grabbingCursor()
{
    return cursorDefault;
}

}
