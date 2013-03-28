/*
 * Copyright (C) 2007, 2008 Holger Hans Peter Freyther
 * Copyright (C) 2007, 2008 Christian Dywan <christian@imendio.com>
 * Copyright (C) 2008 Nuanti Ltd.
 * Copyright (C) 2008 Alp Toker <alp@atoker.com>
 * Copyright (C) 2008 Gustavo Noronha Silva <gns@gnome.org>
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
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
#include "Page.h"
#include "FrameView.h"
#include "WKCWebView.h"
#include "WKCWebFrame.h"
#include "WKCWebFramePrivate.h"
#include "ChromeClientWKC.h"
#include "FileChooser.h"
#include "Geolocation.h"
#include "FrameLoadRequest.h"
#include "WindowFeatures.h"
#include "PlatformString.h"

#include "WKCWebViewPrivate.h"

#include "NotImplemented.h"

#include "helpers/ChromeClientIf.h"
#include "helpers/WKCPage.h"
#include "helpers/WKCString.h"
#include "helpers/WKCHitTestResult.h"
#include "helpers/WKCFrameLoadRequest.h"

#include "helpers/privates/WKCHitTestResultPrivate.h"
#include "helpers/privates/WKCFrameLoadRequestPrivate.h"
#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/privates/WKCFileChooserPrivate.h"
#include "helpers/privates/WKCGeolocationPrivate.h"
#include "helpers/privates/WKCNodePrivate.h"
#include "helpers/privates/WKCPagePrivate.h"

namespace WKC {

ChromeClientWKC::ChromeClientWKC(WKCWebViewPrivate* view)
     : m_view(view)
{
    m_appClient = 0;
}

ChromeClientWKC::~ChromeClientWKC()
{
    if (m_appClient) {
        m_view->clientBuilders().deleteChromeClient(m_appClient);
        m_appClient = 0;
    }
}

ChromeClientWKC*
ChromeClientWKC::create(WKCWebViewPrivate* view)
{
    ChromeClientWKC* self = 0;
    self = new ChromeClientWKC(view);
    if (!self) return 0;
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
ChromeClientWKC::construct()
{
    m_appClient = m_view->clientBuilders().createChromeClient(m_view->parent());
    if (!m_appClient) return false;
    return true;
}

WKCWebView*
ChromeClientWKC::webView() const
{
    return m_view->parent();
}

void
ChromeClientWKC::chromeDestroyed()
{
    delete this;
}

void
ChromeClientWKC::setWindowRect(const WebCore::FloatRect& rect)
{
    WKCFloatRect r = { rect.x(), rect.y(), rect.width(), rect.height() };
    m_appClient->setWindowRect(r);
}
WebCore::FloatRect
ChromeClientWKC::windowRect()
{
    WKCFloatRect rr = m_appClient->windowRect();
    return WebCore::FloatRect(rr.fX, rr.fY, rr.fWidth, rr.fHeight);
}

WebCore::FloatRect
ChromeClientWKC::pageRect()
{
    WKCFloatRect rr = m_appClient->pageRect();
    return WebCore::FloatRect(rr.fX, rr.fY, rr.fWidth, rr.fHeight);
}

float
ChromeClientWKC::scaleFactor()
{
    return m_appClient->scaleFactor();
}

void
ChromeClientWKC::focus()
{
    m_appClient->focus();
}
void
ChromeClientWKC::unfocus()
{
    m_appClient->unfocus();
}

bool
ChromeClientWKC::canTakeFocus(WebCore::FocusDirection dir)
{
    return m_appClient->canTakeFocus((WKC::WKCFocusDirection)dir);
}
void
ChromeClientWKC::takeFocus(WebCore::FocusDirection dir)
{
    m_appClient->takeFocus((WKC::WKCFocusDirection)dir);
}

void
ChromeClientWKC::focusedNodeChanged(WebCore::Node* node)
{
    if (!node) {
        m_appClient->focusedNodeChanged(0);
    } else {
        NodePrivate* n = NodePrivate::create(node);
        m_appClient->focusedNodeChanged(&n->wkc());
        delete n;
    }
}

WebCore::Page*
ChromeClientWKC::createWindow(WebCore::Frame* frame, const WebCore::FrameLoadRequest& request, const WebCore::WindowFeatures& features)
{
    FramePrivate fp(frame);
    FrameLoadRequestPrivate req(request);
    WKC::Page* ret = m_appClient->createWindow(&fp.wkc(), req.wkc(), (const WKC::WindowFeatures &)features);
    if (!ret)
        return 0;
    return (WebCore::Page *)ret->priv().webcore();
}
void
ChromeClientWKC::show()
{
    m_appClient->show();
}

bool
ChromeClientWKC::canRunModal()
{
    return m_appClient->canRunModal();
}
void
ChromeClientWKC::runModal()
{
    m_appClient->runModal();
}

void
ChromeClientWKC::setToolbarsVisible(bool visible)
{
    m_appClient->setToolbarsVisible(visible);
}
bool
ChromeClientWKC::toolbarsVisible()
{
    return m_appClient->toolbarsVisible();
}

void
ChromeClientWKC::setStatusbarVisible(bool visible)
{
    m_appClient->setStatusbarVisible(visible);
}
bool
ChromeClientWKC::statusbarVisible()
{
    return m_appClient->statusbarVisible();
}

void
ChromeClientWKC::setScrollbarsVisible(bool visible)
{
    m_appClient->setScrollbarsVisible(visible);
}
bool
ChromeClientWKC::scrollbarsVisible()
{
    return m_appClient->scrollbarsVisible();
}

void
ChromeClientWKC::setMenubarVisible(bool visible)
{
    m_appClient->setMenubarVisible(visible);
}
bool
ChromeClientWKC::menubarVisible()
{
    return m_appClient->menubarVisible();
}

void
ChromeClientWKC::setResizable(bool flag)
{
    m_appClient->setResizable(flag);
}

void
ChromeClientWKC::addMessageToConsole(WebCore::MessageSource source, WebCore::MessageType type,
                                  WebCore::MessageLevel level, const WebCore::String& message,
                                  unsigned int lineNumber, const WebCore::String& sourceID)
{
    m_appClient->addMessageToConsole((WKC::MessageSource)source, (WKC::MessageType)type, (WKC::MessageLevel)level, message, lineNumber, sourceID);
}

bool
ChromeClientWKC::canRunBeforeUnloadConfirmPanel()
{
    return m_appClient->canRunBeforeUnloadConfirmPanel();
}
bool
ChromeClientWKC::runBeforeUnloadConfirmPanel(const WebCore::String& message, WebCore::Frame* frame)
{
    FramePrivate fp(frame);
    return m_appClient->runBeforeUnloadConfirmPanel(message, &fp.wkc());
}

void
ChromeClientWKC::closeWindowSoon()
{
	webView()->stopLoading();
	webView()->mainFrame()->privateFrame()->core()->page()->setGroupName(WebCore::String());
	m_appClient->closeWindowSoon();
}

void
ChromeClientWKC::runJavaScriptAlert(WebCore::Frame* frame, const WebCore::String& string)
{
    FramePrivate fp(frame);
    m_appClient->runJavaScriptAlert(&fp.wkc(), string);
}
bool
ChromeClientWKC::runJavaScriptConfirm(WebCore::Frame* frame, const WebCore::String& string)
{
    FramePrivate fp(frame);
    return m_appClient->runJavaScriptConfirm(&fp.wkc(), string);
}
bool
ChromeClientWKC::runJavaScriptPrompt(WebCore::Frame* frame, const WebCore::String& message, const WebCore::String& defaultvalue, WebCore::String& out_result)
{
    WKC::String result("");
    FramePrivate fp(frame);
    bool ret = m_appClient->runJavaScriptPrompt(&fp.wkc(), message, defaultvalue, result);
    out_result = result;
    return ret;
}
void
ChromeClientWKC::setStatusbarText(const WebCore::String& string)
{
    m_appClient->setStatusbarText(string);
}
bool
ChromeClientWKC::shouldInterruptJavaScript()
{
    return m_appClient->shouldInterruptJavaScript();
}
bool
ChromeClientWKC::tabsToLinks() const
{
    return m_appClient->tabsToLinks();
}

WebCore::IntRect
ChromeClientWKC::windowResizerRect() const
{
    return m_appClient->windowResizerRect();
}
#if 1
// updated with webkit.org r55675
void
ChromeClientWKC::invalidateContents(const WebCore::IntRect& rect, bool)
{
    m_appClient->repaint(rect, true /*contentChanged*/, false /*immediate*/, true /*repaintContentOnly*/);
}

void
ChromeClientWKC::invalidateWindow(const WebCore::IntRect& rect, bool immediate)
{
    m_appClient->repaint(rect, false /*contentChanged*/, immediate, false /*repaintContentOnly*/);
}

void
ChromeClientWKC::invalidateContentsAndWindow(const WebCore::IntRect& rect, bool immediate)
{
    m_appClient->repaint(rect, true /*contentChanged*/, immediate, false /*repaintContentOnly*/);
}

void
ChromeClientWKC::invalidateContentsForSlowScroll(const WebCore::IntRect& rect, bool immediate)
{
    m_appClient->repaint(rect, true /*contentChanged*/, immediate, true /*repaintContentOnly*/);
}
#else
void
ChromeClientWKC::repaint(const WebCore::IntRect& rect, bool contentChanged, bool immediate, bool repaintContentOnly)
{
    m_appClient->repaint(rect, contentChanged, immediate, repaintContentOnly);
}
#endif
void
ChromeClientWKC::scroll(const WebCore::IntSize& scrollDelta, const WebCore::IntRect& rectToScroll, const WebCore::IntRect& clipRect)
{
    m_appClient->scroll(scrollDelta, rectToScroll, clipRect);
}
WebCore::IntPoint
ChromeClientWKC::screenToWindow(const WebCore::IntPoint& pos) const
{
    return pos;
}
WebCore::IntRect
ChromeClientWKC::windowToScreen(const WebCore::IntRect& rect) const
{
    return rect;
}
PlatformPageClient
ChromeClientWKC::platformPageClient() const
{
    // This code relate to the PopupMenuWKC.
    // If you modified this code, check to the PopupMenuWKC also.
    return (PlatformPageClient)m_view;
}
void
ChromeClientWKC::contentsSizeChanged(WebCore::Frame* frame, const WebCore::IntSize& size) const
{
    WKCSize s = { size.width(), size.height() };
    FramePrivate fp(frame);
    m_appClient->contentsSizeChanged(&fp.wkc(), s);
}

void
ChromeClientWKC::scrollbarsModeDidChange() const
{
    m_appClient->scrollbarsModeDidChange();
}
void
ChromeClientWKC::mouseDidMoveOverElement(const WebCore::HitTestResult& result, unsigned modifierFlags)
{
    HitTestResultPrivate r(result);
    m_appClient->mouseDidMoveOverElement(r.wkc(), modifierFlags);
}

void
ChromeClientWKC::setToolTip(const WebCore::String& string, WebCore::TextDirection dir)
{
    m_appClient->setToolTip(string, (WKC::TextDirection)dir);
}

void
ChromeClientWKC::print(WebCore::Frame* frame)
{
    // Ugh!: do something!
    // 100105 ACCESS Co.,Ltd.
}
#if ENABLE(DATABASE)
void
ChromeClientWKC::exceededDatabaseQuota(WebCore::Frame* frame, const WebCore::String& string)
{
    m_appClient->exceededDatabaseQuota(frame, string);
}
#endif
#if ENABLE(OFFLINE_WEB_APPLICATIONS)
void
ChromeClientWKC::reachedMaxAppCacheSize(int64_t spaceNeeded)
{
    m_appClient->reachedMaxAppCacheSize(spaceNeeded);
}
#endif
void
ChromeClientWKC::runOpenPanel(WebCore::Frame* frame, PassRefPtr<WebCore::FileChooser> chooser)
{
    FramePrivate fp(frame);
    FileChooserPrivate fc(chooser.get());
    m_appClient->runOpenPanel(&fp.wkc(), &fc.wkc());
}

void
ChromeClientWKC::formStateDidChange(const WebCore::Node* node)
{
    if (!node) {
        m_appClient->formStateDidChange(0);
    } else {
        NodePrivate* n = NodePrivate::create(node);
        m_appClient->formStateDidChange(&n->wkc());
        delete n;
    }
}

PassOwnPtr<WebCore::HTMLParserQuirks>
ChromeClientWKC::createHTMLParserQuirks()
{
//    WKC::HTMLParserQuirks* p = m_appClient->createHTMLParserQuirks();
//    return (WebCore::HTMLParserQuirks *)p->parent();
    return 0;
}

bool
ChromeClientWKC::setCursor(WebCore::PlatformCursorHandle handle)
{
    return m_appClient->setCursor((WKC::PlatformCursorHandle)handle);
}

void
ChromeClientWKC::scrollRectIntoView(const WebCore::IntRect& rect, const WebCore::ScrollView* view) const
{
    m_appClient->scrollRectIntoView(rect, 0/*WKCPOBJ(view)*/);
}

void
ChromeClientWKC::requestGeolocationPermissionForFrame(WebCore::Frame* frame, WebCore::Geolocation* geolocation)
{
    FramePrivate fp(frame);
    GeolocationPrivate wg(geolocation);
    m_appClient->requestGeolocationPermissionForFrame(&fp.wkc(), &wg.wkc());
}

} // namespace

