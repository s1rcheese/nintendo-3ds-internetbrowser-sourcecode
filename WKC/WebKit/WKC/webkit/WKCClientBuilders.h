/*
 *  WKCClientBuilders.h
 *
 *  Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
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

#ifndef WKCClientBuilders_h
#define WKCClientBuilders_h

// class definition

namespace WKC {

class ChromeClientIf;
class ContextMenuClientIf;
class DragClientIf;
class EditorClientIf;
class FrameLoaderClientIf;
class InspectorClientIf;
class DropDownListClientIf;

class WKCWebView;
class WKCWebFrame;

class WKCClientBuilders
{
public:
    virtual WKC::ChromeClientIf* createChromeClient(WKCWebView* view) = 0;
    virtual void deleteChromeClient(WKC::ChromeClientIf* client) = 0;

    virtual WKC::ContextMenuClientIf* createContextMenuClient(WKCWebView* view) = 0;
    virtual void deleteContextMenuClient(WKC::ContextMenuClientIf* client) = 0;

    virtual WKC::DragClientIf* createDragClient(WKCWebView* view) = 0;
    virtual void deleteDragClient(WKC::DragClientIf* client) = 0;

    virtual WKC::EditorClientIf* createEditorClient(WKCWebView* view) = 0;
    virtual void deleteEditorClient(WKC::EditorClientIf* client) = 0;

    virtual WKC::FrameLoaderClientIf* createFrameLoaderClient(WKCWebFrame* frame) = 0;
    virtual void deleteFrameLoaderClient(WKC::FrameLoaderClientIf* client) = 0;

    virtual WKC::InspectorClientIf* createInspectorClient(WKCWebView* view) = 0;
    virtual void deleteInspectorClient(WKC::InspectorClientIf* client) = 0;

    virtual WKC::DropDownListClientIf* createDropDownListClient(WKCWebView* view) = 0;
    virtual void deleteDropDownListClient(WKC::DropDownListClientIf* client) = 0;
};

} // namespace

#endif // WKCClientBuilders_h
