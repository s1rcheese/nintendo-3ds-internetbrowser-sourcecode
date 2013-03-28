/*
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.
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
 */

#ifndef WKCChromeClient_h
#define WKCChromeClient_h

#include <wkc/wkcbase.h>
#include "WKCHelpersEnums.h"

namespace WKC {

class Node;
class Frame;
class FrameLoadRequest;
class WindowFeatures;
class String;
class HitTestResult;
class FileChooser;
class HTMLParserQuirks;
class ScrollView;
class Geolocation;
class Page;

typedef void* PlatformPageClient;
typedef void* PlatformCursorHandle;

class ChromeClientIf
{
public:
    virtual void chromeDestroyed() = 0;

    virtual void setWindowRect(const WKCFloatRect&) = 0;
    virtual WKCFloatRect windowRect() = 0;

    virtual WKCFloatRect pageRect() = 0;

    virtual float scaleFactor() = 0;

    virtual void focus() = 0;
    virtual void unfocus() = 0;

    virtual bool canTakeFocus(WKC::WKCFocusDirection) = 0;
    virtual void takeFocus(WKC::WKCFocusDirection) = 0;

    virtual void focusedNodeChanged(WKC::Node*) = 0;

    virtual WKC::Page* createWindow(WKC::Frame*, const WKC::FrameLoadRequest&, const WKC::WindowFeatures&) = 0;
    virtual void show() = 0;

    virtual bool canRunModal() = 0;
    virtual void runModal() = 0;

    virtual void setToolbarsVisible(bool) = 0;
    virtual bool toolbarsVisible() = 0;

    virtual void setStatusbarVisible(bool) = 0;
    virtual bool statusbarVisible() = 0;

    virtual void setScrollbarsVisible(bool) = 0;
    virtual bool scrollbarsVisible() = 0;

    virtual void setMenubarVisible(bool) = 0;
    virtual bool menubarVisible() = 0;

    virtual void setResizable(bool) = 0;

    virtual void addMessageToConsole(WKC::MessageSource source, WKC::MessageType type,
                                     WKC::MessageLevel level, const WKC::String& message,
                                     unsigned int lineNumber, const WKC::String& sourceID) = 0;

    virtual bool canRunBeforeUnloadConfirmPanel() = 0;
    virtual bool runBeforeUnloadConfirmPanel(const WKC::String& message, WKC::Frame* frame) = 0;

    virtual void closeWindowSoon() = 0;

    virtual void runJavaScriptAlert(WKC::Frame*, const WKC::String&) = 0;
    virtual bool runJavaScriptConfirm(WKC::Frame*, const WKC::String&) = 0;
    virtual bool runJavaScriptPrompt(WKC::Frame*, const WKC::String& message, const WKC::String& defaultValue, WKC::String& result) = 0;
    virtual void setStatusbarText(const WKC::String&) = 0;
    virtual bool shouldInterruptJavaScript() = 0;
    virtual bool tabsToLinks() const = 0;

    virtual WKCRect windowResizerRect() const = 0;

    virtual void repaint(const WKCRect&, bool contentChanged, bool immediate = false, bool repaintContentOnly = false) = 0;
    virtual void scroll(const WKCSize& scrollDelta, const WKCRect& rectToScroll, const WKCRect& clipRect) = 0;
    virtual WKCPoint screenToWindow(const WKCPoint&) const = 0;
    virtual WKCRect windowToScreen(const WKCRect&) const = 0;
    virtual PlatformPageClient platformPageClient() const = 0;
    virtual void contentsSizeChanged(WKC::Frame*, const WKCSize&) const = 0;

    virtual void scrollbarsModeDidChange() const = 0;
    virtual void mouseDidMoveOverElement(const WKC::HitTestResult&, unsigned modifierFlags) = 0;

    virtual void setToolTip(const WKC::String&, WKC::TextDirection) = 0;

    virtual void print(WKC::Frame*) = 0;
//#if ENABLE(DATABASE)
//    virtual void exceededDatabaseQuota(WKC::Frame*, const WKC::String&) = 0;
//#endif
//#if ENABLE(OFFLINE_WEB_APPLICATIONS)
//    virtual void reachedMaxAppCacheSize(int64_t spaceNeeded) = 0;
//#endif
    virtual void runOpenPanel(WKC::Frame*, WKC::FileChooser*) = 0;

    virtual void formStateDidChange(const WKC::Node*) = 0;

    virtual WKC::HTMLParserQuirks* createHTMLParserQuirks() = 0;

    virtual bool setCursor(WKC::PlatformCursorHandle) = 0;

    virtual void scrollRectIntoView(const WKCRect&, const WKC::ScrollView*) const = 0;
    virtual void requestGeolocationPermissionForFrame(WKC::Frame*, WKC::Geolocation*) = 0;
};

} // namespace

#endif // WKCChromeClient_h
