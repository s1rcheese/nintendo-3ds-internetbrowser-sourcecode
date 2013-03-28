/*
 *  Copyright (C) 2007 Holger Hans Peter Freyther
 *  Copyright (C) 2007 Alp Toker <alp@atoker.com>
 *  Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "Pasteboard.h"

#include "CString.h"
#include "DocumentFragment.h"
#include "Frame.h"
#include "NotImplemented.h"
#include "PlatformString.h"
#include "TextResourceDecoder.h"
#include "Image.h"
#include "RenderImage.h"
#include "KURL.h"
#include "markup.h"

#include "NotImplemented.h"

namespace WebCore {

static Pasteboard* gPasteboard = 0;

Pasteboard* Pasteboard::generalPasteboard()
{
    if (!gPasteboard) {
        gPasteboard = new Pasteboard();
    }
    return gPasteboard;
}

Pasteboard::Pasteboard()
{
    notImplemented();
}

Pasteboard::~Pasteboard()
{
}

void Pasteboard::writeSelection(Range* selectedRange, bool canSmartCopyOrDelete, Frame* frame)
{
    notImplemented();
}

void Pasteboard::writePlainText(const String& text)
{
    notImplemented();
}

void Pasteboard::writeURL(const KURL& url, const String&, Frame* frame)
{
    notImplemented();
}

void Pasteboard::writeImage(Node* node, const KURL&, const String&)
{
    notImplemented();
}

void Pasteboard::clear()
{
    notImplemented();
}

bool Pasteboard::canSmartReplace()
{
    notImplemented();
    return false;
}

PassRefPtr<DocumentFragment> Pasteboard::documentFragment(Frame* frame, PassRefPtr<Range> context,
                                                          bool allowPlainText, bool& chosePlainText)
{
    notImplemented();
    return 0;
}

String Pasteboard::plainText(Frame* frame)
{
    notImplemented();
    return String();
}

void Pasteboard::deleteSharedInstance()
{
    delete gPasteboard;
}

void Pasteboard::resetVariables()
{
    gPasteboard = 0;
}

}
