/*
 * Copyright (C) 2007 Holger Hans Peter Freyther
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
 */

#ifndef ChromeClientWKC_h
#define ChromeClientWKC_h

#include "ChromeClient.h"
#include "KURL.h"

namespace WKC {
class ChromeClientIf;
class WKCWebViewPrivate;
class WKCWebView;

class ChromeClientWKC : public WebCore::ChromeClient
{
public:
    static ChromeClientWKC* create(WKCWebViewPrivate* view);
    ~ChromeClientWKC();

    WKCWebView* webView() const;

    // callbacks
    virtual void chromeDestroyed();

    virtual void setWindowRect(const WebCore::FloatRect&);
    virtual WebCore::FloatRect windowRect();

    virtual WebCore::FloatRect pageRect();

    virtual float scaleFactor();

    virtual void focus();
    virtual void unfocus();

    virtual bool canTakeFocus(WebCore::FocusDirection);
    virtual void takeFocus(WebCore::FocusDirection);

    virtual void focusedNodeChanged(WebCore::Node*);

    virtual WebCore::Page* createWindow(WebCore::Frame*, const WebCore::FrameLoadRequest&, const WebCore::WindowFeatures&);
    virtual void show();

    virtual bool canRunModal();
    virtual void runModal();

    virtual void setToolbarsVisible(bool);
    virtual bool toolbarsVisible();

    virtual void setStatusbarVisible(bool);
    virtual bool statusbarVisible();

    virtual void setScrollbarsVisible(bool);
    virtual bool scrollbarsVisible();

    virtual void setMenubarVisible(bool);
    virtual bool menubarVisible();

    virtual void setResizable(bool);

    virtual void addMessageToConsole(WebCore::MessageSource source, WebCore::MessageType type,
                                     WebCore::MessageLevel level, const WebCore::String& message,
                                     unsigned int lineNumber, const WebCore::String& sourceID);

    virtual bool canRunBeforeUnloadConfirmPanel();
    virtual bool runBeforeUnloadConfirmPanel(const WebCore::String& message, WebCore::Frame* frame);

    virtual void closeWindowSoon();

    virtual void runJavaScriptAlert(WebCore::Frame*, const WebCore::String&);
    virtual bool runJavaScriptConfirm(WebCore::Frame*, const WebCore::String&);
    virtual bool runJavaScriptPrompt(WebCore::Frame*, const WebCore::String& message, const WebCore::String& defaultValue, WebCore::String& result);
    virtual void setStatusbarText(const WebCore::String&);
    virtual bool shouldInterruptJavaScript();
    virtual bool tabsToLinks() const;

    virtual WebCore::IntRect windowResizerRect() const;
#if 1
    // updated with webkit.org r55675
    virtual void invalidateContents(const WebCore::IntRect&, bool); 
    virtual void invalidateWindow(const WebCore::IntRect&, bool); 
    virtual void invalidateContentsAndWindow(const WebCore::IntRect&, bool); 
    virtual void invalidateContentsForSlowScroll(const WebCore::IntRect&, bool);
#else
    virtual void repaint(const WebCore::IntRect&, bool contentChanged, bool immediate = false, bool repaintContentOnly = false);
#endif
    virtual void scroll(const WebCore::IntSize& scrollDelta, const WebCore::IntRect& rectToScroll, const WebCore::IntRect& clipRect);
    virtual WebCore::IntPoint screenToWindow(const WebCore::IntPoint&) const;
    virtual WebCore::IntRect windowToScreen(const WebCore::IntRect&) const;
    virtual PlatformPageClient platformPageClient() const;
    virtual void contentsSizeChanged(WebCore::Frame*, const WebCore::IntSize&) const;

    virtual void scrollbarsModeDidChange() const;
    virtual void mouseDidMoveOverElement(const WebCore::HitTestResult&, unsigned modifierFlags);

    virtual void setToolTip(const WebCore::String&, WebCore::TextDirection);

    virtual void print(WebCore::Frame*);
#if ENABLE(DATABASE)
    virtual void exceededDatabaseQuota(WebCore::Frame*, const WebCore::String&);
#endif
#if ENABLE(OFFLINE_WEB_APPLICATIONS)
    virtual void reachedMaxAppCacheSize(int64_t spaceNeeded);
#endif
    virtual void runOpenPanel(WebCore::Frame*, PassRefPtr<WebCore::FileChooser>);

    virtual void formStateDidChange(const WebCore::Node*);

    virtual PassOwnPtr<WebCore::HTMLParserQuirks> createHTMLParserQuirks();

    virtual bool setCursor(WebCore::PlatformCursorHandle);

    virtual void scrollRectIntoView(const WebCore::IntRect&, const WebCore::ScrollView*) const;
    virtual void requestGeolocationPermissionForFrame(WebCore::Frame*, WebCore::Geolocation*);

private:
    ChromeClientWKC(WKCWebViewPrivate* view);
    bool construct();

private:
    WKCWebViewPrivate* m_view;
    WKC::ChromeClientIf* m_appClient;
    WebCore::KURL m_hHoveredLinkURL;
};

} // namespace

#endif // ChromeClientWKC_h
