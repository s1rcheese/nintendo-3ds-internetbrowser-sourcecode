/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2008 Collabora Ltd. All rights reserved.
 * All rights reserved.
 * Copyright (c) 2010, 2011 ACCESS CO., LTD. All rights reserved.
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

#ifndef WKCFrameLoaderClient_h
#define WKCFrameLoaderClient_h

#include <wkc/wkcbase.h>
#include "WKCHelpersEnums.h"

namespace WKC {

class DocumentLoader;
class ResourceRequest;
class ResourceResponse;
class AuthenticationChallenge;
class ResourceError;
class KURL;
class ResourceLoader;
class String;
class FormState;
class HTMLFrameOwnerElement;
class HTMLPlugInElement;
class Widget;
class HTMLPluginElement;
class HTMLAppletElement;
class DOMWrapperWorld;
class HistoryItem;
class ResourceHandle;
class NavigationAction;
class SecurityOrigin;
class SubstituteData;
class CachedFrame;
class Frame;
class ClientCertificate;

class FrameLoaderClientWKC;

class FramePolicyFunction {
public:
    void reply(WKC::PolicyAction);

private:
    friend class FrameLoaderClientWKC;
    FramePolicyFunction(void* parent, void* func);
    ~FramePolicyFunction();
private:
    void* m_parent;
    void* m_func;
};

class FrameLoaderClientIf {
public:
    virtual void frameLoaderDestroyed() = 0;

    virtual bool hasWebView() const = 0;

    virtual void makeRepresentation(WKC::DocumentLoader*) = 0;
    virtual void forceLayout() = 0;
    virtual void forceLayoutForNonHTML() = 0;

    virtual void setCopiesOnScroll() = 0;

    virtual void detachedFromParent2() = 0;
    virtual void detachedFromParent3() = 0;

    virtual void assignIdentifierToInitialRequest(unsigned long identifier, WKC::DocumentLoader*, const WKC::ResourceRequest&) = 0;

    virtual void dispatchWillSendRequest(WKC::DocumentLoader*, unsigned long  identifier, WKC::ResourceRequest&, const WKC::ResourceResponse& redirectResponse) = 0;
    virtual bool shouldUseCredentialStorage(WKC::DocumentLoader*, unsigned long identifier) = 0;
    virtual void dispatchDidReceiveAuthenticationChallenge(WKC::DocumentLoader*, unsigned long identifier, const WKC::AuthenticationChallenge&) = 0;
    virtual void dispatchDidCancelAuthenticationChallenge(WKC::DocumentLoader*, unsigned long  identifier, const WKC::AuthenticationChallenge&) = 0;
    virtual void dispatchDidReceiveResponse(WKC::DocumentLoader*, unsigned long  identifier, const WKC::ResourceResponse&) = 0;
    virtual void dispatchDidReceiveContentLength(WKC::DocumentLoader*, unsigned long identifier, int lengthReceived) = 0;
    virtual void dispatchDidFinishLoading(WKC::DocumentLoader*, unsigned long  identifier) = 0;
    virtual void dispatchDidFailLoading(WKC::DocumentLoader*, unsigned long  identifier, const WKC::ResourceError&) = 0;
    virtual bool dispatchDidLoadResourceFromMemoryCache(WKC::DocumentLoader*, const WKC::ResourceRequest&, const WKC::ResourceResponse&, int length) = 0;
    virtual void dispatchDidLoadResourceByXMLHttpRequest(unsigned long, const WKC::String&) = 0;

    virtual void dispatchDidHandleOnloadEvents() = 0;
    virtual void dispatchDidReceiveServerRedirectForProvisionalLoad() = 0;
    virtual void dispatchDidCancelClientRedirect() = 0;
    virtual void dispatchWillPerformClientRedirect(const WKC::KURL&, double, double) = 0;
    virtual void dispatchDidChangeLocationWithinPage() = 0;
    virtual void dispatchDidPushStateWithinPage() = 0;
    virtual void dispatchDidReplaceStateWithinPage() = 0;
    virtual void dispatchDidPopStateWithinPage() = 0;
    virtual void dispatchWillClose() = 0;
    virtual void dispatchDidReceiveIcon() = 0;
    virtual void dispatchDidStartProvisionalLoad() = 0;
    virtual void dispatchDidReceiveTitle(const WKC::String&) = 0;
    virtual void dispatchDidCommitLoad() = 0;
    virtual void dispatchDidFailProvisionalLoad(const WKC::ResourceError&) = 0;
    virtual void dispatchDidFailLoad(const WKC::ResourceError&) = 0;
    virtual void dispatchDidFinishDocumentLoad() = 0;
    virtual void dispatchDidFinishLoad() = 0;
    virtual void dispatchDidFirstLayout() = 0;
    virtual void dispatchDidFirstVisuallyNonEmptyLayout() = 0;
    virtual bool dispatchWillAcceptCookie(const WKC::String& firstparty_host, const WKC::String& cookie_domain) = 0;
    virtual void dispatchWillReceiveData(WKC::ResourceLoader*, int length) = 0;

    virtual void notifySSLStateChange(WKC::ResourceHandle*, int state) = 0;
    virtual int  requestSSLClientCertSelect(WKC::ResourceHandle*, const char* in_requester, WKC::ClientCertificate* in_certs, int in_num) = 0;

    virtual WKC::Frame* dispatchCreatePage() = 0;
    virtual void dispatchShow() = 0;

    virtual void dispatchDecidePolicyForMIMEType(WKC::FramePolicyFunction&, const WKC::String& MIMEType, const WKC::ResourceRequest&) = 0;
    virtual void dispatchDecidePolicyForNewWindowAction(WKC::FramePolicyFunction&, const WKC::NavigationAction&, const WKC::ResourceRequest&, WKC::FormState*, const WKC::String& frameName) = 0;
    virtual void dispatchDecidePolicyForNavigationAction(WKC::FramePolicyFunction&, const WKC::NavigationAction&, const WKC::ResourceRequest&, WKC::FormState*) = 0;
    virtual void cancelPolicyCheck() = 0;

    virtual void dispatchUnableToImplementPolicy(const WKC::ResourceError&) = 0;

    virtual void dispatchWillSubmitForm(WKC::FramePolicyFunction&, WKC::FormState*) = 0;

    virtual void dispatchDidLoadMainResource(WKC::DocumentLoader*) = 0;
    virtual void revertToProvisionalState(WKC::DocumentLoader*) = 0;
    virtual void setMainDocumentError(WKC::DocumentLoader*, const WKC::ResourceError&) = 0;

    virtual void postProgressStartedNotification() = 0;
    virtual void postProgressEstimateChangedNotification() = 0;
    virtual void postProgressFinishedNotification() = 0;

    virtual WKC::Widget* createPlugin(const WKCSize&, WKC::HTMLPlugInElement*, const WKC::KURL&, const WKC::String**, const WKC::String**, const WKC::String&, bool) = 0;
    virtual void redirectDataToPlugin(WKC::Widget* pluginWidget) = 0;
    virtual WKC::Widget* createJavaAppletWidget(const WKCSize&, WKC::HTMLAppletElement*, const WKC::KURL& baseURL, const WKC::String** paramNames, const WKC::String** paramValues) = 0;
    virtual void dispatchDidClearWindowObjectInWorld(WKC::DOMWrapperWorld*) = 0;
    virtual void documentElementAvailable() = 0;
    virtual void didPerformFirstNavigation() const = 0;

    virtual void registerForIconNotification(bool) = 0;

    virtual WKC::ObjectContentType objectContentType(const WKC::KURL& url, const WKC::String& mimeType) = 0;

    virtual void setMainFrameDocumentReady(bool) = 0;

    virtual void startDownload(const WKC::ResourceRequest&) = 0;

    virtual void willChangeTitle(WKC::DocumentLoader*) = 0;
    virtual void didChangeTitle(WKC::DocumentLoader*) = 0;

    virtual void committedLoad(WKC::DocumentLoader*, const char*, int) = 0;
    virtual void finishedLoading(WKC::DocumentLoader*) = 0;

    virtual void updateGlobalHistory() = 0;
    virtual void updateGlobalHistoryRedirectLinks() = 0;
    virtual bool shouldGoToHistoryItem(WKC::HistoryItem*) const = 0;
    virtual void dispatchDidAddBackForwardItem(WKC::HistoryItem*) const = 0;
    virtual void dispatchDidRemoveBackForwardItem(WKC::HistoryItem*) const = 0;
    virtual void dispatchDidChangeBackForwardIndex() const = 0;

    virtual void didDisplayInsecureContent() = 0;
    virtual void didRunInsecureContent(WKC::SecurityOrigin*) = 0;

    virtual WKC::ResourceError cancelledError(const WKC::ResourceRequest&) = 0;
    virtual WKC::ResourceError blockedError(const WKC::ResourceRequest&) = 0;
    virtual WKC::ResourceError cannotShowURLError(const WKC::ResourceRequest&) = 0;
    virtual WKC::ResourceError interruptForPolicyChangeError(const WKC::ResourceRequest&) = 0;

    virtual WKC::ResourceError cannotShowMIMETypeError(const WKC::ResourceResponse&) = 0;
    virtual WKC::ResourceError fileDoesNotExistError(const WKC::ResourceResponse&) = 0;
    virtual WKC::ResourceError pluginWillHandleLoadError(const WKC::ResourceResponse&) = 0;

    virtual bool shouldFallBack(const WKC::ResourceError&) = 0;

    virtual bool canHandleRequest(const WKC::ResourceRequest&) const = 0;
    virtual bool canShowMIMEType(const WKC::String&) const = 0;
    virtual bool representationExistsForURLScheme(const WKC::String&) const = 0;
    virtual WKC::String generatedMIMETypeForURLScheme(const WKC::String&) const = 0;

    virtual void frameLoadCompleted() = 0;
    virtual void saveViewStateToItem(WKC::HistoryItem*) = 0;
    virtual void restoreViewState() = 0;
    virtual void provisionalLoadStarted() = 0;
    virtual void didFinishLoad() = 0;
    virtual void prepareForDataSourceReplacement() = 0;

    virtual void setTitle(const WKC::String& title, const WKC::KURL&) = 0;

    virtual WKC::String userAgent(const WKC::KURL&) = 0;

    virtual void savePlatformDataToCachedFrame(WKC::CachedFrame*) = 0;
    virtual void transitionToCommittedFromCachedFrame(WKC::CachedFrame*) = 0;
    virtual void transitionToCommittedForNewPage() = 0;

    virtual bool canCachePage() const = 0;
    virtual void download(WKC::ResourceHandle*, const WKC::ResourceRequest&, const WKC::ResourceRequest&, const WKC::ResourceResponse&) = 0;
    virtual void updateViewport() = 0;
};

} // namespace

#endif // WKCFrameLoaderClient_h
