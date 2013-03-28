/*
 *  Copyright (C) 2007, 2008 Alp Toker <alp@atoker.com>
 *  Copyright (C) 2007, 2008, 2009 Holger Hans Peter Freyther
 *  Copyright (C) 2007 Christian Dywan <christian@twotoasts.de>
 *  Copyright (C) 2008, 2009 Collabora Ltd.  All rights reserved.
 *  Copyright (C) 2009 Gustavo Noronha Silva <gns@gnome.org>
 *  Copyright (C) Research In Motion Limited 2009. All rights reserved.
 *  Copyright (c) 2010, 2011 ACCESS CO., LTD. All rights reserved.
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

#include "CachedFrame.h"
#include "FrameLoaderClientWKC.h"
#include "FrameView.h"
#include "DocumentLoader.h"
#include "MIMETypeRegistry.h"
#include "PluginDatabase.h"
#include "FormState.h"
#include "HTMLFormElement.h"
#include "HTMLFrameOwnerElement.h"
#include "PlatformString.h"
#include "ResourceRequest.h"
#include "ResourceResponse.h"
#include "ResourceHandle.h"
#include "ScriptString.h"
#include "Certificate.h"

#include "WKCWebViewPrivate.h"
#include "WKCWebFrame.h"
#include "WKCWebFramePrivate.h"

#include "helpers/FrameLoaderClientIf.h"

#include "helpers/WKCFrame.h"
#include "helpers/WKCKURL.h"
#include "helpers/WKCString.h"

#include "helpers/privates/WKCAuthenticationChallengePrivate.h"
#include "helpers/privates/WKCCachedFramePrivate.h"
#include "helpers/privates/WKCCertificatePrivate.h"
#include "helpers/privates/WKCDOMWrapperWorldPrivate.h"
#include "helpers/privates/WKCDocumentLoaderPrivate.h"
#include "helpers/privates/WKCFormStatePrivate.h"
#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/privates/WKCHTMLFrameOwnerElementPrivate.h"
#include "helpers/privates/WKCHistoryItemPrivate.h"
#include "helpers/privates/WKCResourceErrorPrivate.h"
#include "helpers/privates/WKCResourceLoaderPrivate.h"
#include "helpers/privates/WKCNavigationActionPrivate.h"
#include "helpers/privates/WKCResourceRequestPrivate.h"
#include "helpers/privates/WKCResourceResponsePrivate.h"
#include "helpers/privates/WKCSecurityOriginPrivate.h"


#if ENABLE(WKC_META_VIEWPORT)
#include "Settings.h"
#endif

#include "NotImplemented.h"

// implementations

namespace WKC {

FrameLoaderClientWKC::FrameLoaderClientWKC(WKCWebFramePrivate* frame)
     : m_frame(frame),
       m_appClient(0)
{
    m_hasSentResponseToPlugin = false;
    m_pluginView = 0;
    m_policyDecision = 0;
}
FrameLoaderClientWKC::~FrameLoaderClientWKC()
{
    if (m_appClient) {
        if (m_frame) {
            m_frame->clientBuilders().deleteFrameLoaderClient(m_appClient);
        }
        m_appClient = 0;
    }
}

FrameLoaderClientWKC*
FrameLoaderClientWKC::create(WKCWebFramePrivate* frame)
{
    FrameLoaderClientWKC* self = 0;
    self = new FrameLoaderClientWKC(frame);
    if (!self) return 0;
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
FrameLoaderClientWKC::construct()
{
    m_appClient = m_frame->clientBuilders().createFrameLoaderClient(m_frame->parent());
    if (!m_appClient) return false;
    return true;
}

void
FrameLoaderClientWKC::notifyStatus(WKC::LoadStatus loadStatus)
{
    m_frame->m_loadStatus = loadStatus;
    WKCWebViewPrivate* webView = m_frame->m_view;
    if (m_frame->m_parent == webView->m_mainFrame) {
        webView->m_loadStatus = loadStatus;
    }
}

WKCWebFrame*
FrameLoaderClientWKC::webFrame() const
{
    return m_frame->parent();
}

void
FrameLoaderClientWKC::frameLoaderDestroyed()
{
    m_frame->coreFrameDestroyed();
    if (m_appClient) {
        m_frame->clientBuilders().deleteFrameLoaderClient(m_appClient);
    }
    WKCWebFrame::deleteWKCWebFrame(m_frame->parent());
    m_frame = 0;
    delete this;
}

bool
FrameLoaderClientWKC::hasWebView() const
{
    return true;
}

// callbacks

void
FrameLoaderClientWKC::makeRepresentation(WebCore::DocumentLoader* loader)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->makeRepresentation(&ldr.wkc());
}

void
FrameLoaderClientWKC::forceLayout()
{
    WebCore::FrameView* view = m_frame->core()->view();
    if (view) {
        view->forceLayout(true);
    }
}

void
FrameLoaderClientWKC::forceLayoutForNonHTML()
{
    m_appClient->forceLayoutForNonHTML();
}


void
FrameLoaderClientWKC::setCopiesOnScroll()
{
    m_appClient->setCopiesOnScroll();
}


void
FrameLoaderClientWKC::detachedFromParent2()
{
    m_appClient->detachedFromParent2();

    WebCore::FrameLoader* frameLoader = m_frame->core()->loader();
    if (!frameLoader)
        return;

    WebCore::DocumentLoader* documentLoader = frameLoader->activeDocumentLoader();
    if (!documentLoader)
        return;

    if (frameLoader->state() == WebCore::FrameStateCommittedPage)
        documentLoader->mainReceivedError(frameLoader->cancelledError(documentLoader->request()), true);
}

void
FrameLoaderClientWKC::detachedFromParent3()
{
    m_appClient->detachedFromParent3();
    // If we are pan-scrolling when frame is detached, stop the pan scrolling. 
    m_frame->core()->eventHandler()->stopAutoscrollTimer();
}


void
FrameLoaderClientWKC::assignIdentifierToInitialRequest(unsigned long identifier, WebCore::DocumentLoader* loader, const WebCore::ResourceRequest& resource)
{
    ResourceRequestPrivate req(resource);
    DocumentLoaderPrivate ldr(loader);
    m_appClient->assignIdentifierToInitialRequest(identifier, &ldr.wkc(), req.wkc());
}


void
FrameLoaderClientWKC::dispatchWillSendRequest(WebCore::DocumentLoader* loader, unsigned long identifier, WebCore::ResourceRequest& request, const WebCore::ResourceResponse& redirect_response)
{
    DocumentLoaderPrivate ldr(loader);
    ResourceRequestPrivate req(request);
    ResourceResponsePrivate res(redirect_response);
    m_appClient->dispatchWillSendRequest(&ldr.wkc(), identifier, req.wkc(), res.wkc());
}

bool
FrameLoaderClientWKC::shouldUseCredentialStorage(WebCore::DocumentLoader* loader, unsigned long identifier)
{
    bool ret = false;
    DocumentLoaderPrivate ldr(loader);
    ret = m_appClient->shouldUseCredentialStorage(&ldr.wkc(), identifier);
    return ret;
}

void
FrameLoaderClientWKC::dispatchDidReceiveAuthenticationChallenge(WebCore::DocumentLoader* loader, unsigned long identifier, const WebCore::AuthenticationChallenge& challenge)
{
    DocumentLoaderPrivate ldr(loader);
    AuthenticationChallengePrivate wc(challenge);
    m_appClient->dispatchDidReceiveAuthenticationChallenge(&ldr.wkc(), identifier, wc.wkc());
}

void
FrameLoaderClientWKC::dispatchDidCancelAuthenticationChallenge(WebCore::DocumentLoader* loader, unsigned long  identifier, const WebCore::AuthenticationChallenge& challenge)
{
    DocumentLoaderPrivate ldr(loader);
    AuthenticationChallengePrivate wc(challenge);
    m_appClient->dispatchDidCancelAuthenticationChallenge(&ldr.wkc(), identifier, wc.wkc());
}

void
FrameLoaderClientWKC::dispatchDidReceiveResponse(WebCore::DocumentLoader* loader, unsigned long  identifier, const WebCore::ResourceResponse& response)
{
    DocumentLoaderPrivate ldr(loader);
    ResourceResponsePrivate res(response);
    m_appClient->dispatchDidReceiveResponse(&ldr.wkc(), identifier, res.wkc());
}

void
FrameLoaderClientWKC::dispatchDidReceiveContentLength(WebCore::DocumentLoader* loader, unsigned long identifier, int lengthReceived)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->dispatchDidReceiveContentLength(&ldr.wkc(), identifier, lengthReceived);
}

void
FrameLoaderClientWKC::dispatchDidFinishLoading(WebCore::DocumentLoader* loader, unsigned long  identifier)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->dispatchDidFinishLoading(&ldr.wkc(), identifier);
}

void
FrameLoaderClientWKC::dispatchDidFailLoading(WebCore::DocumentLoader* loader, unsigned long  identifier, const WebCore::ResourceError& error)
{
    DocumentLoaderPrivate ldr(loader);
    ResourceErrorPrivate wobj(error);
    m_appClient->dispatchDidFailLoading(&ldr.wkc(), identifier, wobj.wkc());
}

bool
FrameLoaderClientWKC::dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader* loader, const WebCore::ResourceRequest& request, const WebCore::ResourceResponse& response, int length)
{
    bool ret = false;
    DocumentLoaderPrivate ldr(loader);
    ResourceRequestPrivate req(request);
    ResourceResponsePrivate res(response);
    ret = m_appClient->dispatchDidLoadResourceFromMemoryCache(&ldr.wkc(), req.wkc(), res.wkc(), length);
    return ret;
}

void
FrameLoaderClientWKC::dispatchDidLoadResourceByXMLHttpRequest(unsigned long identifier, const WebCore::ScriptString& string)
{
    const JSC::UString js = string;
    const WebCore::String s = js;
    m_appClient->dispatchDidLoadResourceByXMLHttpRequest(identifier, s);
}


void
FrameLoaderClientWKC::dispatchDidHandleOnloadEvents()
{
    m_appClient->dispatchDidHandleOnloadEvents();
}

void
FrameLoaderClientWKC::dispatchDidReceiveServerRedirectForProvisionalLoad()
{
    m_appClient->dispatchDidReceiveServerRedirectForProvisionalLoad();
}

void
FrameLoaderClientWKC::dispatchDidCancelClientRedirect()
{
    m_appClient->dispatchDidCancelClientRedirect();
}

void
FrameLoaderClientWKC::dispatchWillPerformClientRedirect(const WebCore::KURL& uri, double a, double b)
{
    m_appClient->dispatchWillPerformClientRedirect(uri, a, b);
}

void
FrameLoaderClientWKC::dispatchDidChangeLocationWithinPage()
{
    m_appClient->dispatchDidChangeLocationWithinPage();

    if (m_frame->m_uri) {
        fastFree(m_frame->m_uri);
        m_frame->m_uri = 0;
    }
    m_frame->m_uri = wkc_strdup(m_frame->core()->loader()->url().prettyURL().utf8().data());
    // just ignore the error
}

void
FrameLoaderClientWKC::dispatchDidPushStateWithinPage()
{
    m_appClient->dispatchDidPushStateWithinPage();
}

void
FrameLoaderClientWKC::dispatchDidReplaceStateWithinPage()
{
    m_appClient->dispatchDidReplaceStateWithinPage();
}

void
FrameLoaderClientWKC::dispatchDidPopStateWithinPage()
{
    m_appClient->dispatchDidPopStateWithinPage();
}

void
FrameLoaderClientWKC::dispatchWillClose()
{
    m_appClient->dispatchWillClose();
}

void
FrameLoaderClientWKC::dispatchDidReceiveIcon()
{
    m_appClient->dispatchDidReceiveIcon();
}

void
FrameLoaderClientWKC::dispatchDidStartProvisionalLoad()
{
    m_appClient->dispatchDidStartProvisionalLoad();
    notifyStatus(WKC::ELoadStatusProvisional);
}

void
FrameLoaderClientWKC::dispatchDidReceiveTitle(const WebCore::String& title)
{
    m_appClient->dispatchDidReceiveTitle(title);

    if (m_frame->m_title) {
        fastFree(m_frame->m_title);
        m_frame->m_title = 0;
    }
    m_frame->m_title = wkc_wstrndup(title.characters(), title.length());
    // just ignore the error
}

void
FrameLoaderClientWKC::dispatchDidCommitLoad()
{
    m_appClient->dispatchDidCommitLoad();

    if (m_frame->m_uri) {
        fastFree(m_frame->m_uri);
        m_frame->m_uri = 0;
    }
    m_frame->m_uri = wkc_strdup(m_frame->core()->loader()->activeDocumentLoader()->url().prettyURL().utf8().data());
    // just ignore the error
    if (m_frame->m_title) {
        fastFree(m_frame->m_title);
        m_frame->m_title = 0;
    }

    notifyStatus(WKC::ELoadStatusCommitted);
}

void
FrameLoaderClientWKC::dispatchDidFailProvisionalLoad(const WebCore::ResourceError& error)
{
    ResourceErrorPrivate wobj(error);
    m_appClient->dispatchDidFailProvisionalLoad(wobj.wkc());
    notifyStatus(WKC::ELoadStatusNone);
}

void
FrameLoaderClientWKC::dispatchDidFailLoad(const WebCore::ResourceError& error)
{
    ResourceErrorPrivate wobj(error);
    m_appClient->dispatchDidFailLoad(wobj.wkc());
    notifyStatus(WKC::ELoadStatusFailed);
}

void
FrameLoaderClientWKC::dispatchDidFinishDocumentLoad()
{
    m_appClient->dispatchDidFinishDocumentLoad();
}

void
FrameLoaderClientWKC::dispatchDidFinishLoad()
{
    m_appClient->dispatchDidFinishLoad();
    notifyStatus(WKC::ELoadStatusFinished);
}

void
FrameLoaderClientWKC::dispatchDidFirstLayout()
{
    m_appClient->dispatchDidFirstLayout();
}

void
FrameLoaderClientWKC::dispatchDidFirstVisuallyNonEmptyLayout()
{
    m_appClient->dispatchDidFirstVisuallyNonEmptyLayout();
    notifyStatus(WKC::ELoadStatusFirstVisual);
}

bool
FrameLoaderClientWKC::dispatchWillAcceptCookie(const WebCore::String& firstparty_host, const WebCore::String& cookie_domain)
{
    return m_appClient->dispatchWillAcceptCookie(firstparty_host, cookie_domain);
}

void
FrameLoaderClientWKC::dispatchWillReceiveData(WebCore::ResourceLoader* loader, int length)
{
    ResourceLoaderPrivate ldr(loader);
    m_appClient->dispatchWillReceiveData(&ldr.wkc(), length);
}

void
FrameLoaderClientWKC::notifySSLStateChange(WebCore::ResourceHandle* handle, int state)
{
    ResourceHandlePrivate hdl(handle);
    m_appClient->notifySSLStateChange(&hdl.wkc(), state);
}

int
FrameLoaderClientWKC::requestSSLClientCertSelect(WebCore::ResourceHandle* handle, const char* requester, void* certs, int num)
{
    ResourceHandlePrivate hdl(handle);

    WebCore::ClientCertificate** clientCerts;
    clientCerts = (WebCore::ClientCertificate**)certs;
    ClientCertificatePrivate crts(clientCerts);

    return m_appClient->requestSSLClientCertSelect(&hdl.wkc(), requester, &crts.wkc(), num);
}

WebCore::Frame*
FrameLoaderClientWKC::dispatchCreatePage()
{
    WKC::Frame* ret = m_appClient->dispatchCreatePage();
    if (!ret)
        return 0;
    return (WebCore::Frame *)ret->priv().webcore();
}

void
FrameLoaderClientWKC::dispatchShow()
{
    m_appClient->dispatchShow();
}


void
FrameLoaderClientWKC::dispatchDecidePolicyForMIMEType(WebCore::FramePolicyFunction function, const WebCore::String& mime_type, const WebCore::ResourceRequest& request)
{
    WKC::FramePolicyFunction* f = new WKC::FramePolicyFunction(m_frame->core(), (void *)&function);
    ResourceRequestPrivate req(request);
    m_appClient->dispatchDecidePolicyForMIMEType(*f, mime_type, req.wkc());
}

void
FrameLoaderClientWKC::dispatchDecidePolicyForNewWindowAction(WebCore::FramePolicyFunction function, const WebCore::NavigationAction& action, const WebCore::ResourceRequest& request, WTF::PassRefPtr<WebCore::FormState> state, const WebCore::String& frame_name)
{
    WKC::FramePolicyFunction* f = new WKC::FramePolicyFunction(m_frame->core(), (void *)&function);
    ResourceRequestPrivate req(request);
    NavigationActionPrivate nav(action);
    FormStatePrivate st(state.get());
    m_appClient->dispatchDecidePolicyForNewWindowAction(*f, nav.wkc(), req.wkc(), &st.wkc(), frame_name);
}

void
FrameLoaderClientWKC::dispatchDecidePolicyForNavigationAction(WebCore::FramePolicyFunction function, const WebCore::NavigationAction& action, const WebCore::ResourceRequest& request, WTF::PassRefPtr<WebCore::FormState> state)
{
    WKC::FramePolicyFunction* f = new WKC::FramePolicyFunction(m_frame->core(), (void *)&function);
    ResourceRequestPrivate req(request);
    NavigationActionPrivate nav(action);
    FormStatePrivate st(state.get());
    m_appClient->dispatchDecidePolicyForNavigationAction(*f, nav.wkc(), req.wkc(), &st.wkc());
}

void
FrameLoaderClientWKC::cancelPolicyCheck()
{
    m_appClient->cancelPolicyCheck();
}


void
FrameLoaderClientWKC::dispatchUnableToImplementPolicy(const WebCore::ResourceError& error)
{
    ResourceErrorPrivate wobj(error);
    m_appClient->dispatchUnableToImplementPolicy(wobj.wkc());
}


void
FrameLoaderClientWKC::dispatchWillSubmitForm(WebCore::FramePolicyFunction function, WTF::PassRefPtr<WebCore::FormState> state)
{
    WKC::FramePolicyFunction* f = new WKC::FramePolicyFunction(m_frame->core(), (void *)&function);
    FormStatePrivate st(state.get());
    m_appClient->dispatchWillSubmitForm(*f, &st.wkc());
}


void
FrameLoaderClientWKC::dispatchDidLoadMainResource(WebCore::DocumentLoader* loader)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->dispatchDidLoadMainResource(&ldr.wkc());
}

void
FrameLoaderClientWKC::revertToProvisionalState(WebCore::DocumentLoader* loader)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->revertToProvisionalState(&ldr.wkc());
}

void
FrameLoaderClientWKC::setMainDocumentError(WebCore::DocumentLoader* loader, const WebCore::ResourceError& error)
{
    DocumentLoaderPrivate ldr(loader);
    ResourceErrorPrivate wobj(error);
    m_appClient->setMainDocumentError(&ldr.wkc(), wobj.wkc());
}


void
FrameLoaderClientWKC::postProgressStartedNotification()
{
    m_appClient->postProgressStartedNotification();
}

void
FrameLoaderClientWKC::postProgressEstimateChangedNotification()
{
    m_appClient->postProgressEstimateChangedNotification();
}

void
FrameLoaderClientWKC::postProgressFinishedNotification()
{
    m_appClient->postProgressFinishedNotification();
}


PassRefPtr<WebCore::Frame>
FrameLoaderClientWKC::createFrame(const WebCore::KURL& url, const WebCore::String& name, WebCore::HTMLFrameOwnerElement* ownerElement,
                                  const WebCore::String& referrer, bool allowsScrolling, int marginWidth, int marginHeight)
{
    WebCore::Frame* frame = m_frame->core();
    if (!frame || !frame->loader() || !frame->loader()->activeDocumentLoader())
        return 0;

    WKC::WKCWebFrame* child = 0;
    if (ownerElement) {
        HTMLFrameOwnerElementPrivate o(ownerElement);
        child = WKC::WKCWebFrame::create(m_frame->m_view, m_frame->clientBuilders(), reinterpret_cast<HTMLFrameOwnerElement*>(&o.wkc()));
    } else {
        child = WKC::WKCWebFrame::create(m_frame->m_view, m_frame->clientBuilders(), 0);
    }
    // The parent frame page may be removed by JavaScript.
    if (!frame->page()) {
        return 0;
    }

    if (!child) return 0;

    RefPtr<WebCore::Frame> childframe = adoptRef(child->privateFrame()->core());
    frame->tree()->appendChild(childframe);
    childframe->tree()->setName(name);
    childframe->init();

    if (!childframe->page()) {
        return 0;
    }
    frame->loader()->loadURLIntoChildFrame(url, referrer, childframe.get());
    if (!childframe->tree()->parent()) {
        return 0;
    }

#if ENABLE(WKC_ANDROID_LAYOUT)
    if (frame->view() && childframe->view()) {
        childframe->view()->setScreenWidth(frame->view()->screenWidth());
    }
#endif

    return childframe.release();
}

PassRefPtr<WebCore::Widget>
FrameLoaderClientWKC::createPlugin(const WebCore::IntSize&, WebCore::HTMLPlugInElement*, const WebCore::KURL&, const WTF::Vector<WebCore::String>&, const WTF::Vector<WebCore::String>&, const WebCore::String&, bool)
{
    // Ugh!: implement something!
    // 100106 ACCESS Co.,Ltd.
    notImplemented();
    return 0;
}

void
FrameLoaderClientWKC::redirectDataToPlugin(WebCore::Widget* pluginWidget)
{
    // Ugh!: implement something!
    // 100106 ACCESS Co.,Ltd.
    notImplemented();
}

PassRefPtr<WebCore::Widget>
FrameLoaderClientWKC::createJavaAppletWidget(const WebCore::IntSize&, WebCore::HTMLAppletElement*, const WebCore::KURL& baseURL, const WTF::Vector<WebCore::String>& paramNames, const WTF::Vector<WebCore::String>& paramValues)
{
    notImplemented();
    return 0;
}

WebCore::String
FrameLoaderClientWKC::overrideMediaType() const
{
    return WebCore::String();
}

void
FrameLoaderClientWKC::dispatchDidClearWindowObjectInWorld(WebCore::DOMWrapperWorld* world)
{
    if (world != WebCore::mainThreadNormalWorld()) {
        return;
    }
    DOMWrapperWorldPrivate w(world);
    m_appClient->dispatchDidClearWindowObjectInWorld(&w.wkc());
}

void
FrameLoaderClientWKC::documentElementAvailable()
{
    m_appClient->documentElementAvailable();
}

void
FrameLoaderClientWKC::didPerformFirstNavigation() const
{
    m_appClient->didPerformFirstNavigation();
}


void
FrameLoaderClientWKC::registerForIconNotification(bool flag)
{
    m_appClient->registerForIconNotification(flag);
}


WebCore::ObjectContentType
FrameLoaderClientWKC::objectContentType(const WebCore::KURL& url, const WebCore::String& mimeType)
{
    return (WebCore::ObjectContentType)m_appClient->objectContentType(url, mimeType);
}


void
FrameLoaderClientWKC::setMainFrameDocumentReady(bool flag)
{
    m_appClient->setMainFrameDocumentReady(flag);
}


void
FrameLoaderClientWKC::startDownload(const WebCore::ResourceRequest& request)
{
    ResourceRequestPrivate req(request);
    m_appClient->startDownload(req.wkc());
}


void
FrameLoaderClientWKC::willChangeTitle(WebCore::DocumentLoader* loader)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->willChangeTitle(&ldr.wkc());
}

void
FrameLoaderClientWKC::didChangeTitle(WebCore::DocumentLoader* loader)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->didChangeTitle(&ldr.wkc());
    setTitle(loader->title(), loader->url());
}


void
FrameLoaderClientWKC::committedLoad(WebCore::DocumentLoader* loader, const char* data, int len)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->committedLoad(&ldr.wkc(), data, len);

    WebCore::String encoding = loader->overrideEncoding();
    bool userChosen = !encoding.isNull();
    if (!userChosen) {
        encoding = loader->response().textEncodingName();
    }

    WebCore::FrameLoader* frameLoader = m_frame->core()->loader();
    frameLoader->setEncoding(encoding, userChosen);
    if (data) {
        frameLoader->addData(data, len);
    }

    WebCore::Frame* coreFrame = loader->frame();
    if (coreFrame && coreFrame->document() && coreFrame->document()->isMediaDocument()) {
        loader->cancelMainResourceLoad(frameLoader->client()->pluginWillHandleLoadError(loader->response()));
    }
}

void
FrameLoaderClientWKC::finishedLoading(WebCore::DocumentLoader* loader)
{
    DocumentLoaderPrivate ldr(loader);
    m_appClient->finishedLoading(&ldr.wkc());
    committedLoad(loader, 0, 0);
}


void
FrameLoaderClientWKC::updateGlobalHistory()
{
    m_appClient->updateGlobalHistory();
}

void
FrameLoaderClientWKC::updateGlobalHistoryRedirectLinks()
{
    m_appClient->updateGlobalHistoryRedirectLinks();
}

bool
FrameLoaderClientWKC::shouldGoToHistoryItem(WebCore::HistoryItem* item) const
{
    bool ret = false;
    HistoryItemPrivate his(item);
    ret = m_appClient->shouldGoToHistoryItem(&his.wkc());
    return ret;
}

void
FrameLoaderClientWKC::dispatchDidAddBackForwardItem(WebCore::HistoryItem* item) const
{
    HistoryItemPrivate his(item);
    m_appClient->dispatchDidAddBackForwardItem(&his.wkc());
}

void
FrameLoaderClientWKC::dispatchDidRemoveBackForwardItem(WebCore::HistoryItem* item) const
{
    HistoryItemPrivate his(item);
    m_appClient->dispatchDidRemoveBackForwardItem(&his.wkc());
}

void
FrameLoaderClientWKC::dispatchDidChangeBackForwardIndex() const
{
    m_appClient->dispatchDidChangeBackForwardIndex();
}


void
FrameLoaderClientWKC::didDisplayInsecureContent()
{
    m_appClient->didDisplayInsecureContent();
}

void
FrameLoaderClientWKC::didRunInsecureContent(WebCore::SecurityOrigin* origin)
{
    SecurityOriginPrivate o(origin);
    m_appClient->didRunInsecureContent(&o.wkc());
}


WebCore::ResourceError
FrameLoaderClientWKC::cancelledError(const WebCore::ResourceRequest& request)
{
    ResourceRequestPrivate req(request);
    return m_appClient->cancelledError(req.wkc()).priv().webcore();
}

WebCore::ResourceError
FrameLoaderClientWKC::blockedError(const WebCore::ResourceRequest& request)
{
    ResourceRequestPrivate req(request);
    return m_appClient->blockedError(req.wkc()).priv().webcore();
}

WebCore::ResourceError
FrameLoaderClientWKC::cannotShowURLError(const WebCore::ResourceRequest& request)
{
    ResourceRequestPrivate req(request);
    return m_appClient->cannotShowURLError(req.wkc()).priv().webcore();
}

WebCore::ResourceError
FrameLoaderClientWKC::interruptForPolicyChangeError(const WebCore::ResourceRequest& request)
{
    ResourceRequestPrivate req(request);
    return m_appClient->interruptForPolicyChangeError(req.wkc()).priv().webcore();
}


WebCore::ResourceError
FrameLoaderClientWKC::cannotShowMIMETypeError(const WebCore::ResourceResponse& response)
{
    ResourceResponsePrivate res(response);
    return m_appClient->cannotShowMIMETypeError(res.wkc()).priv().webcore();
}

WebCore::ResourceError
FrameLoaderClientWKC::fileDoesNotExistError(const WebCore::ResourceResponse& response)
{
    ResourceResponsePrivate res(response);
    return m_appClient->fileDoesNotExistError(res.wkc()).priv().webcore();
}

WebCore::ResourceError
FrameLoaderClientWKC::pluginWillHandleLoadError(const WebCore::ResourceResponse& response)
{
    ResourceResponsePrivate res(response);
    return m_appClient->pluginWillHandleLoadError(res.wkc()).priv().webcore();
}


bool
FrameLoaderClientWKC::shouldFallBack(const WebCore::ResourceError& error)
{
    ResourceErrorPrivate wobj(error);
    return m_appClient->shouldFallBack(wobj.wkc());
}


bool
FrameLoaderClientWKC::canHandleRequest(const WebCore::ResourceRequest& request) const
{
    ResourceRequestPrivate req(request);
    return m_appClient->canHandleRequest(req.wkc());
}

bool
FrameLoaderClientWKC::canShowMIMEType(const WebCore::String& type) const
{
    return m_appClient->canShowMIMEType(type);
}

bool
FrameLoaderClientWKC::representationExistsForURLScheme(const WebCore::String& string) const
{
    return m_appClient->representationExistsForURLScheme(string);
}

WebCore::String
FrameLoaderClientWKC::generatedMIMETypeForURLScheme(const WebCore::String& string) const
{
    return m_appClient->generatedMIMETypeForURLScheme(string);
}


void
FrameLoaderClientWKC::frameLoadCompleted()
{
    m_appClient->frameLoadCompleted();
}

void
FrameLoaderClientWKC::saveViewStateToItem(WebCore::HistoryItem* item)
{
    HistoryItemPrivate his(item);
    m_appClient->saveViewStateToItem(&his.wkc());
}

void
FrameLoaderClientWKC::restoreViewState()
{
    m_appClient->restoreViewState();
}

void
FrameLoaderClientWKC::provisionalLoadStarted()
{
    m_appClient->provisionalLoadStarted();
}

void
FrameLoaderClientWKC::didFinishLoad()
{
    m_appClient->didFinishLoad();
}

void
FrameLoaderClientWKC::prepareForDataSourceReplacement()
{
    m_appClient->prepareForDataSourceReplacement();
}


WTF::PassRefPtr<WebCore::DocumentLoader>
FrameLoaderClientWKC::createDocumentLoader(const WebCore::ResourceRequest& request, const WebCore::SubstituteData& substituteData)
{
    return WebCore::DocumentLoader::create(request, substituteData);
}

void
FrameLoaderClientWKC::setTitle(const WebCore::String& title, const WebCore::KURL& uri)
{
    m_appClient->setTitle(title, uri);
    if (m_frame->m_title) {
        fastFree(m_frame->m_title);
        m_frame->m_title = 0;
    }
    m_frame->m_title = wkc_wstrndup(title.characters(), title.length());
}


WebCore::String
FrameLoaderClientWKC::userAgent(const WebCore::KURL& uri)
{
    return m_appClient->userAgent(uri);
}


void
FrameLoaderClientWKC::savePlatformDataToCachedFrame(WebCore::CachedFrame* frame)
{
    CachedFramePrivate c(frame);
    m_appClient->savePlatformDataToCachedFrame(&c.wkc());
}

void
FrameLoaderClientWKC::transitionToCommittedFromCachedFrame(WebCore::CachedFrame* frame)
{
    CachedFramePrivate c(frame);
    m_appClient->transitionToCommittedFromCachedFrame(&c.wkc());
}

void
FrameLoaderClientWKC::transitionToCommittedForNewPage()
{

#if ENABLE(WKC_META_VIEWPORT)
    // reset metadata settings for the main frame as they are not preserved cross page
    if (m_frame->core() == m_frame->core()->page()->mainFrame() && m_frame->core()->settings())
        m_frame->core()->settings()->resetMetadataSettings();
#endif

#if ENABLE(WKC_ANDROID_LAYOUT)
    int screenWidth = 0;
    if (m_frame->core()->view()) {
        screenWidth = m_frame->core()->view()->screenWidth();
    }
#endif

    WKCWebViewPrivate* containingWindow = m_frame->m_view;
    WebCore::IntSize desktopsize = containingWindow->defaultDesktopSize();
    WebCore::IntSize viewsize = containingWindow->defaultViewSize();
    bool transparent = containingWindow->transparent();
    WebCore::Color backgroundColor = transparent ? WebCore::Color::transparent : WebCore::Color::white;
    WebCore::Frame* frame = m_frame->core();
    bool usefixed = false;
    if (frame->view()) {
        usefixed = frame->view()->useFixedLayout();
    }

    frame->createView(desktopsize, backgroundColor, transparent, viewsize, usefixed);

    if (frame->view()) {
        if (!frame->ownerElement()) {
            frame->view()->setCanHaveScrollbars(false);
        }
#if ENABLE(WKC_FRAME_FLATTENING)
        else {
            if (!frame->ownerElement()->hasTagName(WebCore::HTMLNames::iframeTag)
                && !frame->ownerElement()->hasTagName(WebCore::HTMLNames::embedTag)
                && !frame->ownerElement()->hasTagName(WebCore::HTMLNames::objectTag)
                && frame->settings()->flatFrameSetLayoutEnabled()) {
                frame->view()->setCanHaveScrollbars(false);
            }
        }
#endif
    }

#if ENABLE(WKC_ANDROID_LAYOUT)
    if (frame->view()) {
        frame->view()->setScreenWidth(screenWidth);
    }
#endif

    m_appClient->transitionToCommittedForNewPage();
}


bool
FrameLoaderClientWKC::canCachePage() const
{
    return m_appClient->canCachePage();
}

void
FrameLoaderClientWKC::download(WebCore::ResourceHandle* handle, const WebCore::ResourceRequest& request, const WebCore::ResourceRequest& request2, const WebCore::ResourceResponse& response)
{
    ResourceHandlePrivate h(handle);
    ResourceRequestPrivate req1(request);
    ResourceRequestPrivate req2(request2);
    ResourceResponsePrivate res(response);
    m_appClient->download(&h.wkc(), req1.wkc(), req2.wkc(), res.wkc());
}


#if ENABLE(WKC_META_VIEWPORT)
void
FrameLoaderClientWKC::updateViewport()
{
    m_appClient->updateViewport();
}
#endif

FramePolicyFunction::FramePolicyFunction(void* parent, void* func)
    : m_parent(parent)
    , m_func(func)
{
}

FramePolicyFunction::~FramePolicyFunction()
{
}

void
FramePolicyFunction::reply(WKC::PolicyAction action)
{
    WebCore::Frame* frame = (WebCore::Frame *)m_parent;
    WebCore::FramePolicyFunction* func = (WebCore::FramePolicyFunction *)m_func;

    (frame->loader()->policyChecker()->**func)((WebCore::PolicyAction)action);
    delete this;
}

} // namespace
