/*
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 * 
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 * 
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#include "config.h"
#include "ClipboardWKC.h"

#include "CachedImage.h"
#include "CString.h"
#include "Editor.h"
#include "Element.h"
#include "FileList.h"
#include "Frame.h"
#include "markup.h"
#include "NotImplemented.h"
#include "RenderImage.h"
#include "StringHash.h"

namespace WebCore {

PassRefPtr<Clipboard> Editor::newGeneralClipboard(ClipboardAccessPolicy policy)
{
    return ClipboardWKC::create(policy, false);
}

ClipboardWKC::ClipboardWKC(ClipboardAccessPolicy policy, bool forDragging)
    : Clipboard(policy, forDragging)
{
}

ClipboardWKC::~ClipboardWKC()
{
}

void ClipboardWKC::clearData(const String&)
{
    notImplemented();
}

void ClipboardWKC::clearAllData()
{
    notImplemented();
}

String ClipboardWKC::getData(const String&, bool &success) const
{
    notImplemented();
    success = false;
    return String();
}

bool ClipboardWKC::setData(const String&, const String&)
{
    notImplemented();
    return false;
}

HashSet<String> ClipboardWKC::types() const
{
    notImplemented();
    return HashSet<String>();
}

PassRefPtr<FileList> ClipboardWKC::files() const
{
    notImplemented();
    return 0;
}

IntPoint ClipboardWKC::dragLocation() const
{
    notImplemented();
    return IntPoint(0, 0);
}

CachedImage* ClipboardWKC::dragImage() const
{
    notImplemented();
    return 0;
}

void ClipboardWKC::setDragImage(CachedImage*, const IntPoint&)
{
    notImplemented();
}

Node* ClipboardWKC::dragImageElement()
{
    notImplemented();
    return 0;
}

void ClipboardWKC::setDragImageElement(Node*, const IntPoint&)
{
    notImplemented();
}

DragImageRef ClipboardWKC::createDragImage(IntPoint&) const
{
    notImplemented();
    return 0;
}

void ClipboardWKC::declareAndWriteDragImage(Element* element, const KURL& url, const String& label, Frame*)
{
    // Ugh!: implement something!
    // 091228 ACCESS Co.,Ltd.
}

void ClipboardWKC::writeURL(const KURL& url, const String& label, Frame*)
{
    // Ugh!: implement something!
    // 091228 ACCESS Co.,Ltd.
}

void ClipboardWKC::writeRange(Range* range, Frame* frame)
{
    // Ugh!: implement something!
    // 091228 ACCESS Co.,Ltd.
}

bool ClipboardWKC::hasData()
{
    notImplemented();
    return false;
}

}
