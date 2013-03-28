/*
 * Copyright (C) 2004, 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2005, 2006 Michael Emmel mike.emmel@gmail.com
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
#include "ResourceHandle.h"

#include "DocLoader.h"
#include "NotImplemented.h"
#include "ResourceHandleInternal.h"
#include "ResourceHandleManager.h"

#if PLATFORM(WIN) && PLATFORM(CF)
#include <wtf/RetainPtr.h>
#endif

#include <openssl/ssl.h>
#include <openssl/x509v3.h>

namespace WebCore {

class WebCoreSynchronousLoader : public ResourceHandleClient {
public:
    WebCoreSynchronousLoader();

    virtual void didReceiveResponse(ResourceHandle*, const ResourceResponse&);
    virtual void didReceiveData(ResourceHandle*, const char*, int, int lengthReceived);
    virtual void didFinishLoading(ResourceHandle*);
    virtual void didFail(ResourceHandle*, const ResourceError&);

    ResourceResponse resourceResponse() const { return m_response; }
    ResourceError resourceError() const { return m_error; }
    Vector<char> data() const { return m_data; }

private:
    ResourceResponse m_response;
    ResourceError m_error;
    Vector<char> m_data;
};

WebCoreSynchronousLoader::WebCoreSynchronousLoader()
{
}

void WebCoreSynchronousLoader::didReceiveResponse(ResourceHandle*, const ResourceResponse& response)
{
    m_response = response;
}

void WebCoreSynchronousLoader::didReceiveData(ResourceHandle*, const char* data, int length, int)
{
    m_data.append(data, length);
}

void WebCoreSynchronousLoader::didFinishLoading(ResourceHandle*)
{
}

void WebCoreSynchronousLoader::didFail(ResourceHandle*, const ResourceError& error)
{
    m_error = error;
}


/*
static HashSet<String>& allowsAnyHTTPSCertificateHosts()
{
    static HashSet<String> hosts;

    return hosts;
}
*/

ResourceHandleInternal::~ResourceHandleInternal()
{
    if (m_url) {
        fastFree(m_url);
        m_url = 0;
    }
    if (m_urlhost) {
        fastFree(m_urlhost);
        m_urlhost = 0;
    }

    if (m_customHeaders) {
        curl_slist_free_all(m_customHeaders);
        m_customHeaders = 0;
    }
}

ResourceHandle::~ResourceHandle()
{
    cancel();
}

bool ResourceHandle::start(Frame* frame)
{
    // The frame could be null if the ResourceHandle is not associated to any
    // Frame, e.g. if we are downloading a file.
    // If the frame is not null but the page is null this must be an attempted
    // load from an onUnload handler, so let's just block it.
    if (frame && !frame->page())
        return false;

    ResourceHandleManager::sharedInstance()->add(this);
    return true;
}

void ResourceHandle::cancel()
{
    if (ResourceHandleManager::sharedInstance()) {
        ResourceHandleManager::sharedInstance()->cancel(this);
    }
}

PassRefPtr<SharedBuffer> ResourceHandle::bufferedData()
{
    return 0;
}

bool ResourceHandle::supportsBufferedData()
{
    return false;
}

#if PLATFORM(WIN) && PLATFORM(CF)
void ResourceHandle::setHostAllowsAnyHTTPSCertificate(const String& host)
{
    allowsAnyHTTPSCertificateHosts().add(host.lower());
}
#endif

#if PLATFORM(WIN) && PLATFORM(CF)
// FIXME:  The CFDataRef will need to be something else when
// building without 
static HashMap<String, RetainPtr<CFDataRef> >& clientCerts()
{
    static HashMap<String, RetainPtr<CFDataRef> > certs;
    return certs;
}

void ResourceHandle::setClientCertificate(const String& host, CFDataRef cert)
{
    clientCerts().set(host.lower(), cert);
}
#endif

void ResourceHandle::setDefersLoading(bool defers)
{
    if (d->m_defersLoading == defers)
        return;

#if LIBCURL_VERSION_NUM > 0x071200
    if (!d->m_handle)
        d->m_defersLoading = defers;
    else if (defers) {
        CURLcode error = curl_easy_pause(d->m_handle, CURLPAUSE_ALL);
        // If we could not defer the handle, so don't do it.
        if (error != CURLE_OK)
            return;

        d->m_defersLoading = defers;
    } else {
        // We need to set defersLoading before restarting a connection
        // or libcURL will call the callbacks in curl_easy_pause and
        // we would ASSERT.
        d->m_defersLoading = defers;

        CURLcode error = curl_easy_pause(d->m_handle, CURLPAUSE_CONT);
        if (error != CURLE_OK)
            // Restarting the handle has failed so just cancel it.
            cancel();
    }
#else
    d->m_defersLoading = defers;
    LOG_ERROR("Deferred loading is implemented if libcURL version is above 7.18.0");
#endif
}

bool ResourceHandle::willLoadFromCache(ResourceRequest&, Frame*)
{
    notImplemented();
    return false;
}

bool ResourceHandle::loadsBlocked()
{
    notImplemented();
    return false;
}

void ResourceHandle::loadResourceSynchronously(const ResourceRequest& request, StoredCredentials storedCredentials, ResourceError& error, ResourceResponse& response, Vector<char>& data, Frame*)
{
    WebCoreSynchronousLoader syncLoader;
    ResourceHandle handle(request, &syncLoader, true, false, true);

    ResourceHandleManager* manager = ResourceHandleManager::sharedInstance();

    manager->dispatchSynchronousJob(&handle);

    error = syncLoader.resourceError();
    data = syncLoader.data();
    response = syncLoader.resourceResponse();
}

//
// Authenticate server
//
void ResourceHandle::didReceiveAuthenticationChallenge(const AuthenticationChallenge& challenge) 
{
    ResourceHandleInternal* d = getInternal();
    if (!d) return;

    ResourceHandleManager* mgr = ResourceHandleManager::sharedInstance();
    if (!mgr) return;

    ProtectionSpaceServerType serverType = ProtectionSpaceServerHTTP;
    Vector<String> list;
    String realm = "";
    long avail = CURLAUTH_BASIC;
    String host;
    int port;
    long authproblem = 0;
    CURLcode err;

    String header = d->m_response.httpHeaderField("WWW-Authenticate");
    if (header.isNull()) {
        return; // do nothing
    }

    String protocol = d->m_request.url().protocol();
    serverType = (protocol == "http") ? ProtectionSpaceServerHTTP : ProtectionSpaceServerHTTPS;

    err = curl_easy_getinfo(d->m_handle, CURLINFO_HTTPAUTH_PROBLEM, &authproblem);
    if (CURLE_OK != err) return; // do nothing

    err = curl_easy_getinfo(d->m_handle, CURLINFO_HTTPAUTH_AVAIL, &avail);
    if (CURLE_OK != err) return; // do nothing

    host = d->m_response.httpHeaderField("Host");
    if (host.isNull()) {
        host = d->m_request.url().host();
    }
    port = d->m_request.url().port();
    port = (0 < port) ? port : 80;

    header = header.stripWhiteSpace();
    if (header.lower().startsWith("digest")) {
        header = header.right(header.length() - 6);
        header.split(",", list);
    }
    else if (header.lower().startsWith("basic")) {
        header = header.right(header.length() - 5);
        header = header.stripWhiteSpace();
        list.append(header);
    }

    for (int i = 0; i < list.size(); i++) {
        Vector<String> items;
        list[i].split("=", items);
        if (items.size() < 2) continue;
        if (items.size() > 2) {
            for (int j=2; j<items.size(); j++) {
                items[1] += "="+items[j];
            }
        }
        String type = items[0].stripWhiteSpace().lower();
        String value = items[1].stripWhiteSpace();
        if (value[0]=='"') {
            value = value.right(value.length()-1);
        }
        if (value[value.length()-1]=='"') {
            value = value.left(value.length()-1);
        }
        if (type == "realm") {
            realm = value;
        }
    }

    ProtectionSpaceAuthenticationScheme scheme = ProtectionSpaceAuthenticationSchemeDefault;
    if (avail&CURLAUTH_DIGEST) {
        scheme = ProtectionSpaceAuthenticationSchemeHTTPDigest;
    }
    else if (avail&CURLAUTH_NTLM) {
        scheme = ProtectionSpaceAuthenticationSchemeNTLM;
    }
    else if (avail&CURLAUTH_BASIC || avail&CURLAUTH_ANY) {
        scheme = ProtectionSpaceAuthenticationSchemeHTTPBasic;
    }

    d->m_webChallenge = AuthenticationChallenge(
        ProtectionSpace(host, port, serverType, realm, scheme),
        challenge.proposedCredential(),
        0,
        d->m_response,
        challenge.error(),
        this);

    if (authproblem && d->client())
            d->client()->didReceiveAuthenticationChallenge(this, d->m_webChallenge);
}

void ResourceHandle::receivedCredential(const AuthenticationChallenge& challenge, const Credential& credential) 
{
    ASSERT(!challenge.isNull());
    ResourceHandleInternal* d = getInternal();
    if (challenge != d->m_webChallenge)
        return;

    d->m_webChallenge = AuthenticationChallenge(challenge.protectionSpace(),
        credential,
        challenge.previousFailureCount(),
        challenge.failureResponse(),
        challenge.error(),
        this);

    ResourceHandleManager::sharedInstance()->doAuthChallenge(this);
}

void ResourceHandle::receivedRequestToContinueWithoutCredential(const AuthenticationChallenge& challenge) 
{
    ASSERT(!challenge.isNull());
    ResourceHandleInternal* d = getInternal();
    if (challenge != d->m_webChallenge)
        return;

    ResourceHandleManager::sharedInstance()->cancelAuthChallenge(this);
}

void ResourceHandle::receivedCancellation(const AuthenticationChallenge& challenge)
{
    notImplemented();
}

//
// Authenticate proxy
//
void ResourceHandle::didReceiveProxyAuthenticationChallenge(const AuthenticationChallenge& challenge,
                                                            String proxyHost, int port, long auth) 
{
    ResourceHandleInternal* d = getInternal();
    if (!d) return;

    ResourceHandleManager* mgr = ResourceHandleManager::sharedInstance();
    if (!mgr) return;

    ProtectionSpaceServerType serverType = ProtectionSpaceProxyHTTP;
    long avail = CURLAUTH_BASIC;
    long authproblem = 0;
    CURLcode err;

    String header = d->m_response.httpHeaderField("Proxy-Authenticate");
    if (header.isNull()) {
        return;
    }

    String protocol = d->m_request.url().protocol();
    serverType = (protocol == "http") ? ProtectionSpaceProxyHTTP : ProtectionSpaceProxyHTTPS;

    err = curl_easy_getinfo(d->m_handle, CURLINFO_HTTPAUTH_PROBLEM, &authproblem);
    if (CURLE_OK != err) return; // do nothing

    err = curl_easy_getinfo(d->m_handle, CURLINFO_PROXYAUTH_AVAIL, &avail);
    if (CURLE_OK != err) return; // do nothing
    if (!(avail & auth)) {
        return;
    }

    if (proxyHost.isNull()) {
        return;  // do nothing
    }
    String host = "Proxy: http://" + proxyHost + ":" + String::number(port);

    ProtectionSpaceAuthenticationScheme scheme = ProtectionSpaceAuthenticationSchemeDefault;
    if (auth == CURLAUTH_DIGEST) {
        scheme = ProtectionSpaceAuthenticationSchemeHTTPDigest;
    }
    else if (auth == CURLAUTH_NTLM) {
        scheme = ProtectionSpaceAuthenticationSchemeNTLM;
    }
    else if (auth == CURLAUTH_BASIC) {
        scheme = ProtectionSpaceAuthenticationSchemeHTTPBasic;
    }

    d->m_webChallenge = AuthenticationChallenge(
        ProtectionSpace(host, 0, serverType, "", scheme),
        challenge.proposedCredential(),
        0,
        d->m_response,
        challenge.error(),
        this);

    if (authproblem) {
        if (mgr->hasProxyUserPass())
            mgr->doAuthChallenge(this);
        else if (d->client())
            d->client()->didReceiveAuthenticationChallenge(this, d->m_webChallenge);
    }
}

void ResourceHandle::receivedProxyCredential(const Credential& credential) 
{
    ResourceHandleManager* mgr = ResourceHandleManager::sharedInstance();

    mgr->doAuthChallenge(this);
    mgr->setProxyUserPass(credential.user(), credential.password());
}

} // namespace WebCore
