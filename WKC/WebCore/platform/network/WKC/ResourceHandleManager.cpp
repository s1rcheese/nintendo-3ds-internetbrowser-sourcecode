/*
    ResourceHandleManager.oneshot.cpp
 */

/*
 * Copyright (C) 2004, 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (C) 2008 Collabora Ltd.
 * Copyright (C) 2008 Nuanti Ltd.
 * Copyright (C) 2009 Appcelerator Inc.
 * Copyright (C) 2009 Brent Fulgham <bfulgham@webkit.org>
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

#include "config.h"
#include "ResourceHandleManager.h"
#include "ResourceHandleManagerSSL.h"

#include "Base64.h"
#include "CookieJar.h"
#include "CString.h"
#include "HTTPParsers.h"
#include "MIMETypeRegistry.h"
#include "NotImplemented.h"
#include "ResourceError.h"
#include "ResourceHandle.h"
#include "ResourceHandleInternal.h"
#include "TextEncoding.h"

#include <errno.h>
#include <stdio.h>
#include <wtf/Threading.h>
#include <wtf/Vector.h>

#if !PLATFORM(WIN_OS)
#include <sys/param.h>
#undef MAX_PATH
#define MAX_PATH MAXPATHLEN
#endif

#if COMPILER(MSVC)
#define strncasecmp _strnicmp
#endif

#ifndef TRUE
# define TRUE (1)
#endif
#ifndef FALSE
# define FALSE (0)
#endif

namespace WebCore {

const double pollTimeSeconds = 0.05;

/* dispatchSynchronousJob() reserves one connection  */
#define DEFAULT_TCP_CONNECTIONS   7L
#define MAX_TCP_CONNECTIONS       8L

// for debug
#undef DEBUG_LOADING
#undef DEBUG_LOADING_MORE
#undef DEBUG_ERROR
#undef DEBUG_FUNCTION
#undef DEBUG_FUNCTION_MORE
#undef CURL_DEBUG_CALLBACK
#undef DEBUG_HTTP_HEADER

extern "C" void peerDebugPrintf(const char* in_format, ...);

#ifdef DEBUG_LOADING
  #define LOADINGPRINTF(a)  peerDebugPrintf a
#else
  #define LOADINGPRINTF(a)
#endif

#ifdef DEBUG_LOADING_MORE
  #define LOADINGMOREPRINTF(a)  peerDebugPrintf a
#else
  #define LOADINGMOREPRINTF(a)
#endif

#ifdef DEBUG_ERROR
  #define ERRORPRINTF(a)  peerDebugPrintf a
#else
  #define ERRORPRINTF(a)
#endif

#ifdef DEBUG_FUNCTION
  #define FUNCTIONPRINTF(a)  peerDebugPrintf a
#else
  #define FUNCTIONPRINTF(a)
#endif

#if defined(DEBUG_FUNCTION) && defined(DEBUG_FUNCTION_MORE)
  #define FUNCTIONMOREPRINTF(a)  peerDebugPrintf a
#else
  #define FUNCTIONMOREPRINTF(a)
#endif

#ifdef DEBUG_HTTP_HEADER
  #define HTTPHEADERPRINTF(a)  peerDebugPrintf a
#else
  #define HTTPHEADERPRINTF(a)
#endif
//
// Mutex
//
static void* gCookieMutex = 0;
static void* gDnsMutex    = 0;
static void* gShareMutex  = 0;

static void sharedResourceMutexFinalize(void)
{
    if (gCookieMutex) {
        wkcMutexDeletePeer(gCookieMutex);
        gCookieMutex = 0;
    }
    if (gDnsMutex) {
        wkcMutexDeletePeer(gDnsMutex);
        gDnsMutex = 0;
    }
    if (gShareMutex) {
        wkcMutexDeletePeer(gShareMutex);
        gShareMutex = 0;
    }
}

static void sharedResourceMutexInitialize(void)
{
    sharedResourceMutexFinalize();
    gCookieMutex = wkcMutexNewPeer();
    gDnsMutex    = wkcMutexNewPeer();
    gShareMutex  = wkcMutexNewPeer();
}

static void* sharedResourceMutex(curl_lock_data data)
{
    switch (data) {
    case CURL_LOCK_DATA_COOKIE:
        return gCookieMutex;
    case CURL_LOCK_DATA_DNS:
        return gDnsMutex;
    case CURL_LOCK_DATA_SHARE:
        return gShareMutex;
    default:
        ASSERT_NOT_REACHED();
        return NULL;
    }
}

// libcurl does not implement its own thread synchronization primitives.
// these two functions provide mutexes for cookies, and for the global DNS
// cache.
static void curl_lock_callback(CURL* handle, curl_lock_data data, curl_lock_access access, void* userPtr)
{
    if (void* mutex = sharedResourceMutex(data))
        wkcMutexLockPeer(mutex);
}

static void curl_unlock_callback(CURL* handle, curl_lock_data data, void* userPtr)
{
    if (void* mutex = sharedResourceMutex(data))
        wkcMutexUnlockPeer(mutex);
}

//
// libcurl memory callbacks
//
static void *ResourceHandleManagerMalloc(size_t size)
{
    void *ptr;

    ptr = fastMalloc(size);
    if (!ptr) {
        return NULL;
    }
    memset(ptr, 0x00, size);

    return ptr;
}

static void ResourceHandleManagerFree(void *ptr)
{
    fastFree(ptr);
}


static void *ResourceHandleManagerRealloc(void *ptr, size_t size)
{
    void *new_ptr;

    new_ptr = fastRealloc(ptr, size);

    return new_ptr;
}

static char *ResourceHandleManagerStrdup(const char *str)
{
    char *new_str;
    size_t size;

    if (!str) {
        return NULL;
    }
    size = strlen(str);

    new_str = (char *)fastMalloc(size + 1);
    if (!new_str) {
        return NULL;
    }
    memcpy(new_str, str, size);
    new_str[size] = 0;

    return new_str;
}

static void *ResourceHandleManagerCalloc(size_t nmemb, size_t size)
{
    void *ptr;

    ptr = fastCalloc(nmemb, size);

    return ptr;
}

static char *fastStrdup(const char *str)
{
    char *new_str;
    size_t size;

    if (!str) {
        return NULL;
    }
    size = strlen(str) + 1;

    new_str = (char *)fastMalloc(size);
    if (!new_str) {
        return NULL;
    }
    memcpy(new_str, str, size);

    return new_str;
}

static String hostAndPort(const KURL& kurl)
{
    String url;

    if (kurl.hasPort()) {
        char port_buf[8];
        snprintf(port_buf, 7, "%d", kurl.port());
        url = kurl.host() + ":" + port_buf;
    }
    else
        url = kurl.host();

    return url;
}

//
//  Constructor and Destructor
//
ResourceHandleManager::ResourceHandleManager()
    : m_downloadTimer(this, &ResourceHandleManager::downloadTimerCallback)
    , m_tcpConnections(DEFAULT_TCP_CONNECTIONS)
    , m_cookieInfo(0)
    , m_nextCookie(0)
    , m_serializeEnd(false)
    , m_cookiesDeleting(false)
    , m_jsSequential(false)
    , m_cookieMaxEntries(20) /* default */
    , m_willChallengeProxyAuth(false)
{
    FUNCTIONPRINTF(("<rhm>ResourceHandleManager()"));

    curl_global_init_mem(CURL_GLOBAL_ALL, ResourceHandleManagerMalloc,
                                          ResourceHandleManagerFree,
                                          ResourceHandleManagerRealloc,
                                          ResourceHandleManagerStrdup,
                                          ResourceHandleManagerCalloc);
    m_curlMultiHandle = curl_multi_init();
    curl_multi_setopt(m_curlMultiHandle, CURLMOPT_MAXCONNECTS, m_tcpConnections);

    sharedResourceMutexInitialize();

    m_curlShareHandle = curl_share_init();
    curl_share_setopt(m_curlShareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
    curl_share_setopt(m_curlShareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    curl_share_setopt(m_curlShareHandle, CURLSHOPT_LOCKFUNC, curl_lock_callback);
    curl_share_setopt(m_curlShareHandle, CURLSHOPT_UNLOCKFUNC, curl_unlock_callback);

    /* costomized option */
    curl_share_setopt(m_curlShareHandle, CURLSHOPT_COOKIE_MAX_ENTRIES, m_cookieMaxEntries);

    ResourceHandleManagerSSL::createSharedInstance(m_curlMultiHandle);
}

ResourceHandleManager::~ResourceHandleManager()
{
    FUNCTIONPRINTF(("<rhm>~ResourceHandleManager()"));

    removeAllRunningJobs();
    ResourceHandleManagerSSL::deleteSharedInstance();

    curl_multi_cleanup(m_curlMultiHandle);
    curl_share_cleanup(m_curlShareHandle);
    sharedResourceMutexFinalize();
    curl_global_cleanup();
}

void ResourceHandleManager::setMaxTCPConnections(long number)
{
    if (0 < number && number < MAX_TCP_CONNECTIONS) {
        m_tcpConnections = number;
        curl_multi_setopt(m_curlMultiHandle, CURLMOPT_MAXCONNECTS, number);
    }
}

void ResourceHandleManager::setMaxCookieEntries(long number)
{
    if (0 < number) {
        m_cookieMaxEntries = number;
        curl_share_setopt(m_curlShareHandle, CURLSHOPT_COOKIE_MAX_ENTRIES, number);
    }
}

ResourceHandleManager* ResourceHandleManager::create()
{
    ResourceHandleManager* self;

    self = new ResourceHandleManager();
    if (!self)
        goto exit_func;

    if (!self->construct()) {
        delete self;
        self = 0;
        goto exit_func;
    }

exit_func:
    return self;
}

bool ResourceHandleManager::construct()
{
    return true;
}

// Singleton
ResourceHandleManager* ResourceHandleManager::m_sharedInstance = 0;

bool ResourceHandleManager::createSharedInstance()
{
    if (m_sharedInstance)
        return true;

    m_sharedInstance = create();
    if (!m_sharedInstance)
        return false;

    return true;
}

ResourceHandleManager* ResourceHandleManager::sharedInstance()
{
    return m_sharedInstance;
}

void ResourceHandleManager::deleteSharedInstance()
{
    if (m_sharedInstance)
        delete m_sharedInstance;

    m_sharedInstance = 0;
}

bool ResourceHandleManager::isExistSharedInstance()
{
    return m_sharedInstance ? true : false;
}

//
// Force Terminate
//
void ResourceHandleManager::forceTerminateInstance()
{
    ResourceHandleManager* mgr = ResourceHandleManager::sharedInstance();
    if (mgr)
        mgr->forceTerminate();
}

void ResourceHandleManager::forceTerminate()
{
    ResourceHandleManagerSSL::resetVariables();

    m_curlMultiHandle = 0;
    m_curlShareHandle = 0;
    sharedResourceMutexFinalize();
    curl_global_force_reset();

    m_sharedInstance = 0;
}

//
// handling HTTP
//
static String guessMIMETypeByURL(const KURL& kurl, long httpCode)
{
    if (httpCode == 200) {
        if (!kurl.query().isEmpty())
            return "application/octet-stream";
        if (!kurl.hasPath())
            return "text/html";
        if (kurl.lastPathComponent().isEmpty())
            return "text/html";
        return MIMETypeRegistry::getMIMETypeForPath(kurl.lastPathComponent());
    }
    else if (httpCode < 200 || httpCode == 204 || httpCode == 205 ||httpCode == 304) {
        // These response code MUST NOT has response body.
        return "";
    }
    else {
        // Other status code like 301,401,407 may return with text html body.
        return "text/html";
    }
}


static void handleLocalReceiveResponse(CURL* handle, ResourceHandle* job, ResourceHandleInternal* d)
{
    FUNCTIONPRINTF(("<rhm>handleLocalReceiveResponse()"));
    // since the code in headerCallback will not have run for local files
    // the code to set the URL and fire didReceiveResponse is never run,
    // which means the ResourceLoader's response does not contain the URL.
    // Run the code here for local files to resolve the issue.
    // TODO: See if there is a better approach for handling this.
    const char* hdr;
    CURLcode err = curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &hdr);
    ASSERT(CURLE_OK == err);
    d->m_response.setURL(KURL(ParsedURLString, hdr));

    long httpCode = 0;
    err = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &httpCode);
    ASSERT(CURLE_OK == err);

    if (d->m_response.mimeType().isEmpty()) {
        d->m_response.setMimeType(extractMIMETypeFromMediaType(d->m_response.httpHeaderField("Content-Type")).lower());
        if (d->m_response.mimeType().isEmpty()) {
            d->m_response.setMimeType(guessMIMETypeByURL(d->m_response.url(), httpCode));
        }
    }

    if (d->client())
        d->client()->didReceiveResponse(job, d->m_response);
    if (d->m_cancelled)
        return;

    d->m_response.setResponseFired(true);
}

/*
 * This is being called for HTTP body in the response. 
 *
 * called with data after all headers have been processed via headerCallback
 */
static size_t writeCallback(void* ptr, size_t size, size_t nmemb, void* data)
{
    FUNCTIONMOREPRINTF(("<rhm>writeCallback(%p)", data));

    ResourceHandle* job = static_cast<ResourceHandle*>(data);
    ResourceHandleInternal* d = job->getInternal();

    size_t totalSize = size*nmemb;

    if (d->m_doAuthChallenge)
        return totalSize;  // ignore received body coz continuing for authentication
    if (d->m_cancelled)
        return 0;

#if LIBCURL_VERSION_NUM > 0x071200
    // We should never be called when deferred loading is activated.
    ASSERT(!d->m_defersLoading);
#endif


    // this shouldn't be necessary but apparently is. CURL writes the data
    // of html page even if it is a redirect that was handled internally
    // can be observed e.g. on gmail.com
    CURL* h = d->m_handle;
    long httpCode = 0;
    CURLcode err = curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &httpCode);
    ASSERT(CURLE_OK == err);
    if (100 <= httpCode && httpCode < 200) {
        return 0;
    }
    else if (300 <= httpCode && httpCode < 400) {
        String location = d->m_response.httpHeaderField("location");
        if (!location.isEmpty()) {
            return totalSize;
        }
    }

    if (!d->m_response.responseFired()) {
        handleLocalReceiveResponse(h, job, d);
        if (d->m_doAuthChallenge)
            return totalSize;
        if (d->m_cancelled)
            return 0;
    }

    if (d->client()) {
        d->client()->willReceiveData(job, totalSize);
        if (d->m_cancelled)
            return 0;
    }

    if (d->client())
        d->client()->didReceiveData(job, static_cast<char*>(ptr), totalSize, 0);
    if (d->m_cancelled)
        return 0;

    LOADINGMOREPRINTF(("Received Data  [%p] len=%d", job, totalSize));
    return totalSize;
}

/*
 * This is being called for each HTTP header in the response. This includes '\r\n'
 * for the last line of the header.
 *
 * We will add each HTTP Header to the ResourceResponse and on the termination
 * of the header (\r\n) we will parse Content-Type and Content-Disposition and
 * update the ResourceResponse and then send it away.
 *
 */
static size_t headerCallback(char* ptr, size_t size, size_t nmemb, void* data)
{
    FUNCTIONMOREPRINTF(("<rhm>headerCallback(%p)", data));

    ResourceHandle* job = static_cast<ResourceHandle*>(data);
    ResourceHandleInternal* d = job->getInternal();
    if (d->m_cancelled)
        return 0;

#if LIBCURL_VERSION_NUM > 0x071200
    // We should never be called when deferred loading is activated.
    ASSERT(!d->m_defersLoading);
#endif

    size_t totalSize = size * nmemb;

    if (d->client()) {
        d->client()->willReceiveData(job, totalSize);
        if (d->m_cancelled)
            return 0;
    }

    String header(static_cast<const char*>(ptr), totalSize);

    /*
     * a) We can finish and send the ResourceResponse
     * b) We will add the current header to the HTTPHeaderMap of the ResourceResponse
     *
     * The HTTP standard requires to use \r\n but for compatibility it recommends to
     * accept also \n.
     */
    if (header == String("\r\n") || header == String("\n")) {
        LOADINGMOREPRINTF(("Header Received[%p]", job));

        CURL* h = d->m_handle;

        const char* hdr;
        (void)curl_easy_getinfo(h, CURLINFO_EFFECTIVE_URL, &hdr);
        d->m_response.setURL(KURL(ParsedURLString, hdr));

        long httpCode = 0;
        (void)curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &httpCode);
        d->m_response.setHTTPStatusCode(httpCode);

        // SSL
        if (0 == httpCode && d->m_isSSL) {
            long connectCode = 0;
            (void)curl_easy_getinfo(h, CURLINFO_HTTP_CONNECTCODE, &connectCode);
            if (connectCode == 200) {
                return totalSize;
            }
            else if (connectCode == 407) {
                /* should callback like didReceiveAuthenticationChallenge() for Proxy Authentication */;
                /* If httpCode is 407 then it will calls didReceiveAuthenticationChallenge(). */;
                httpCode = 407;
            }
            else if (connectCode >= 300 && connectCode < 400) {
                /* Proxy's redirecting.... Is it avaiable??? */;
                /* should do something else */;
            }
            else {
                /* should do something else */;
                httpCode = connectCode;
            }
        }

        // 100 continue
        if (100 == httpCode) {
            return totalSize;
        }

#ifdef DEBUG_HTTP_HEADER
        {
#ifndef DEBUG_LOADING_MORE
            HTTPHEADERPRINTF(("Header Received[%p]", job));
#endif
            HTTPHEADERPRINTF(("> URL: %s", hdr));
            HTTPHEADERPRINTF(("> HTTP Status: %d", httpCode));

            HTTPHeaderMap responseHeaders = d->m_response.httpHeaderFields();
            HTTPHeaderMap::const_iterator end = responseHeaders.end();
            for (HTTPHeaderMap::const_iterator it = responseHeaders.begin(); it != end; ++it) {
                String key = it->first;
                String value = it->second;
                HTTPHEADERPRINTF(("> %s: %s", key.utf8().data(), value.utf8().data()));
            }
        }
#endif

        double contentLength = 0;
        (void)curl_easy_getinfo(h, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);
        d->m_response.setExpectedContentLength(static_cast<long long int>(contentLength));

        d->m_response.setMimeType(extractMIMETypeFromMediaType(d->m_response.httpHeaderField("Content-Type")).lower());
        d->m_response.setTextEncodingName(extractCharsetFromMediaType(d->m_response.httpHeaderField("Content-Type")));
        d->m_response.setSuggestedFilename(filenameFromHTTPContentDisposition(d->m_response.httpHeaderField("Content-Disposition")));

        if (d->m_response.mimeType().isEmpty()) {
            d->m_response.setMimeType(guessMIMETypeByURL(d->m_response.url(), httpCode));
        }

        // HTTP authentication
        if (401 == httpCode && d->m_response.isHTTP()) {
            job->didReceiveAuthenticationChallenge(d->m_webChallenge);
        }

        // Proxy authentication
        if (407 == httpCode && d->m_response.isHTTP()) {
            ResourceHandleManager::sharedInstance()->didReceiveProxyAuthChallenge(job, d);
        }

        if (d->m_cancelled)
            return 0;
        if (d->m_doAuthChallenge)
            return totalSize;

        // HTTP redirection
        if (httpCode >= 300 && httpCode < 400) {
            String location = d->m_response.httpHeaderField("location");
            if (!location.isEmpty()) {
                KURL newURL = KURL(job->request().url(), location);

                if (!d->m_isInclusion) {
                    d->m_request.setFirstPartyForCookies(newURL);
                }
                ResourceRequest redirectedRequest = job->request();
                redirectedRequest.setURL(newURL);

                // To match a http method when the curl performs redirect
                // (referenced lib/transfer.c,Curl_follow())
                switch(httpCode){
                    case 301:
                    case 302:
                        if(equalIgnoringCase(redirectedRequest.httpMethod(), "POST") ||
                           equalIgnoringCase(redirectedRequest.httpMethod(), "PUT")) {
                            redirectedRequest.setHTTPMethod("GET");
                        }
                        break;
                    case 303:
                        if(!equalIgnoringCase(redirectedRequest.httpMethod(), "GET")) {
                            redirectedRequest.setHTTPMethod("GET");
                        }
                        break;
                }

                if (d->client())
                    d->client()->willSendRequest(job, redirectedRequest, d->m_response);
                if (d->m_cancelled)
                    return 0;

                d->m_response.setHTTPHeaderField("location", String());

                String x_frame_options = d->m_response.httpHeaderField("x-frame-options");
                if (!x_frame_options.isEmpty()) {
                    d->m_response.setHTTPHeaderField("x-frame-options", String());
                }

                d->m_request.setURL(newURL);

                if (d->m_url) {
                    fastFree(d->m_url);
                }
                d->m_url  = fastStrdup(newURL.string().latin1().data());
                if (d->m_urlhost) {
                    fastFree(d->m_urlhost);
                }
                d->m_urlhost = fastStrdup(hostAndPort(newURL).latin1().data());

                if (equalIgnoringCase(newURL.protocol(), "https")) {
                    d->m_isSSL = true;
                    ResourceHandleManagerSSL::sharedInstance()->initializeHandleSSL(job);
                }

                return totalSize;
            }
        }

        if (d->client())
            d->client()->didReceiveResponse(job, d->m_response);
        if (d->m_cancelled)
            return 0;

        d->m_response.setResponseFired(true);
    }
    else {
        int splitPos = header.find(":");
        if (splitPos != -1) {
            long avail = CURLAUTH_NONE;
            bool needToSet = true;
            String name = header.left(splitPos);
            String value = header.substring(splitPos+1).stripWhiteSpace();

            if (equalIgnoringCase(name, "WWW-Authenticate")) {
                (void)curl_easy_getinfo(d->m_handle, CURLINFO_HTTPAUTH_AVAIL, &avail);
            } else if (equalIgnoringCase(name, "Proxy-Authenticate")) {
                (void)curl_easy_getinfo(d->m_handle, CURLINFO_PROXYAUTH_AVAIL, &avail);
            }

            if (avail != CURLAUTH_NONE) {
                if (CURLAUTH_DIGEST & avail) {
                    needToSet = value.lower().startsWith("digest");
                } else if (CURLAUTH_NTLM & avail) {
                    needToSet = value.lower().startsWith("ntlm"); 
                } else if (CURLAUTH_BASIC & avail) {
                    needToSet = value.lower().startsWith("basic"); 
                }
            }

            if (needToSet) {
                d->m_response.setHTTPHeaderField(name, value);
            }
        }
        /* find status text and set it */
        else if (header.find("HTTP/", 0, false) != -1) {
            CURL* h = d->m_handle;
            long httpcode = 0;
            if (curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &httpcode) == CURLE_OK) {
                char buf[4];
                int pos, len;
                len = snprintf(buf, 3, "%d", httpcode);
                if (len > 0) {
                    buf[len] = '\0';
                    pos = header.find(buf);
                    if (pos != -1) {
                        pos += len;
                        d->m_response.setHTTPStatusText(header.substring(pos).stripWhiteSpace());
                    }
                }
            }
        }
    }

    return totalSize;
}

/* This is called to obtain HTTP POST or PUT data.
   Iterate through FormData elements and upload files.
   Carefully respect the given buffer size and fill the rest of the data at the next calls.
*/
static size_t readCallback(void* ptr, size_t size, size_t nmemb, void* data)
{
    FUNCTIONMOREPRINTF(("<rhm>readCallback(%p)", data));

    ResourceHandle* job = static_cast<ResourceHandle*>(data);
    ResourceHandleInternal* d = job->getInternal();

    if (d->m_cancelled)
        return 0;

#if LIBCURL_VERSION_NUM > 0x071200
    // We should never be called when deferred loading is activated.
    ASSERT(!d->m_defersLoading);
#endif

    if (!size || !nmemb)
        return 0;

    if (!d->m_formDataStream.hasMoreElements())
        return 0;

    size_t sent = d->m_formDataStream.read(ptr, size, nmemb);
    if (sent == 0)
        sent = CURL_READFUNC_NODATA;

    return sent;
}

/* This is called to obtain HTTP POST or PUT data and to receive redirect. */
static curlioerr ioctlCallback(CURL *handle, int cmd, void *clientp)
{
    FUNCTIONMOREPRINTF(("<rhm>ioctlCallback(%p)", clientp));

    ResourceHandle* job = static_cast<ResourceHandle*>(clientp);
    ResourceHandleInternal* d = job->getInternal();
    if (!d || !d->m_handle)
        return CURLIOE_OK;

    // erase "Content-Type" header
    struct curl_slist *cur_headers = d->m_customHeaders;
    struct curl_slist *new_headers = NULL;
    while (cur_headers) {
        if (cur_headers->data) {
            if (strncasecmp(cur_headers->data, "Content-Type:", 13)) {
                new_headers = curl_slist_append(new_headers, cur_headers->data);
            }
        }
        cur_headers = cur_headers->next;
    }
    if (new_headers) {
        curl_slist_free_all(d->m_customHeaders);
        curl_easy_setopt(d->m_handle, CURLOPT_HTTPHEADER, new_headers);
        d->m_customHeaders = new_headers;
    }

    /* no operation */
    return CURLIOE_OK;
}

#ifdef CURL_DEBUG_CALLBACK
int debugCallback(CURL *handle, curl_infotype type,  char *data, size_t size, void *userptr)
{
    switch (type) {
    case CURLINFO_TEXT:
        {
            int len;
            len = strlen(data);
            if (data[len-2] == '\r' || data[len-2] == '\n') {
              data[len-2] = 0x00;
            }
            else {
              data[len-1] = 0x00;
            }
        }
        peerDebugPrintf("<rhm>curl log: %s", data);
        break;
#if 0
    case CURLINFO_HEADER_IN:
        peerDebugPrintf("<rhm>curl log: CURLINFO_HEADER_IN");
        break;
    case CURLINFO_HEADER_OUT:
        peerDebugPrintf("<rhm>curl log: CURLINFO_HEADER_OUT");
        break;
    case CURLINFO_DATA_IN:
        peerDebugPrintf("<rhm>curl log: CURLINFO_DATA_IN");
        break;
    case CURLINFO_DATA_OUT:
        peerDebugPrintf("<rhm>curl log: CURLINFO_DATA_OUT");
        break;
#endif
    }
    return 0;
}
#endif

void ResourceHandleManager::downloadTimerCallback(Timer<ResourceHandleManager>* timer)
{
    FUNCTIONMOREPRINTF(("<rhm>downloadTimerCallback()"));

    startScheduledJobs();

#if 0
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd = 0;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = selectTimeoutMS * 1000;       // select waits microseconds

    // Retry 'select' if it was interrupted by a process signal.
    int rc = 0;
    do {
        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);
        curl_multi_fdset(m_curlMultiHandle, &fdread, &fdwrite, &fdexcep, &maxfd);
        // When the 3 file descriptors are empty, winsock will return -1
        // and bail out, stopping the file download. So make sure we
        // have valid file descriptors before calling select.
        if (maxfd >= 0)
            rc = ::select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
    } while (rc == -1 && errno == EINTR);

    if (-1 == rc) {
#ifndef NDEBUG
        perror("bad: select() returned -1: ");
#endif
        return;
    }
#endif

    int runningHandles = 0;
    while (curl_multi_perform(m_curlMultiHandle, &runningHandles) == CURLM_CALL_MULTI_PERFORM) { }

    // check the curl messages indicating completed transfers
    // and free their resources
    while (true) {
        int messagesInQueue;
        CURLMsg* msg = curl_multi_info_read(m_curlMultiHandle, &messagesInQueue);
        if (!msg)
            break;

        // find the node which has same d->m_handle as completed transfer
        CURL* handle = msg->easy_handle;
        ASSERT(handle);
        ResourceHandle* job = 0;
        CURLcode err = curl_easy_getinfo(handle, CURLINFO_PRIVATE, &job);
        ASSERT(CURLE_OK == err);
        ASSERT(job);
        if (!job)
            continue;
        ResourceHandleInternal* d = job->getInternal();
        ASSERT(d->m_handle == handle);

        if (d->m_cancelled || d->m_doAuthChallenge) {
            removeFromCurl(job);
            continue;
        }

        if (CURLMSG_DONE != msg->msg)
            continue;

        if (CURLE_OK == msg->data.result) {
            if (!d->m_response.responseFired()) {
                handleLocalReceiveResponse(d->m_handle, job, d);
                if (d->m_cancelled || d->m_doAuthChallenge) {
                    removeFromCurl(job);
                    continue;
                }
            }

            if (d->client())
                d->client()->didFinishLoading(job);
        } else {
            char* url = 0;
            curl_easy_getinfo(d->m_handle, CURLINFO_EFFECTIVE_URL, &url);
            ERRORPRINTF(("Curl ERROR for url='%s', error: '[%d]%s'", url, msg->data.result, curl_easy_strerror(msg->data.result)));

            if (d->client())
                d->client()->didFail(job, ResourceError(String(), msg->data.result, String(url), String(curl_easy_strerror(msg->data.result)), job));

            if (CURLE_SSL_ISSUER_ERROR == msg->data.result)
                ResourceHandleManagerSSL::sharedInstance()->removeAllowServerCA((const char *)d->m_urlhost);
        }

        removeFromCurl(job);
    }

    cancelScheduledJobs();

    bool started = startScheduledJobs(); // new jobs might have been added in the meantime

    if (!m_downloadTimer.isActive() && (started || (runningHandles > 0))) {
        FUNCTIONMOREPRINTF(("<rhm>startOneShot(%f) by dtc", pollTimeSeconds));
        m_downloadTimer.startOneShot(pollTimeSeconds);
    }

    FUNCTIONMOREPRINTF(("<dtc> EXIT"));
}

static CURLcode cookie_accept_callback(CURL *curl, const char *domain, bool tailmatch, void *data)
{
    FUNCTIONPRINTF(("<rhm>cookie_accept_callback(%p, %s, %s, %p)", curl, domain, (tailmatch)?"True":"False", data));

    ResourceHandle* job = static_cast<ResourceHandle*>(data);
    ResourceHandleInternal* d = job->getInternal();

    if (d->m_cancelled) {
        return CURLE_COOKIE_DENY;
    }

    if (!d->m_request.allowCookies()) {
        return CURLE_COOKIE_DENY;
    }

    String firstparty_host = d->m_request.firstPartyForCookies().host();
    String cookie_domain = String((tailmatch)?".":"") + domain;

    if (d->client())
        if (!d->client()->willAcceptCookie(job, firstparty_host, cookie_domain))
            return CURLE_COOKIE_DENY;

    return CURLE_COOKIE_ACCEPT;
}

void ResourceHandleManager::clearCookies()
{
    FUNCTIONPRINTF(("<rhm>clearCookies()"));

    if (m_cookiesDeleting) return;

    if (0 == m_runningJobList.size()) {
        wkcMutexLockPeer(gCookieMutex);

        CURLSHcode shcode = curl_share_setopt(m_curlShareHandle, CURLSHOPT_UNSHARE, CURL_LOCK_DATA_COOKIE);
        if (CURLSHE_OK == shcode) {
            curl_share_setopt(m_curlShareHandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
            curl_share_setopt(m_curlShareHandle, CURLSHOPT_COOKIE_MAX_ENTRIES, m_cookieMaxEntries);
        }
        else {
            /* share is still in use */
            m_cookiesDeleting = true;
        }

        wkcMutexUnlockPeer(gCookieMutex);
    } else {
        m_cookiesDeleting = true;
    }
}

void ResourceHandleManager::setProxyInfo(const String& host, unsigned long port, ProxyType type,
                                         const String& username, const String& password, ProxyAuth auth)
{
    m_proxyHost = host;
    m_proxyPort = (port)?port:80;
    m_proxyType = type;
    m_proxyAuth = auth;

    if (Basic == m_proxyAuth && username.find(":") < 0)
        m_proxyUser = username;
    else
        m_proxyUser = "";
    m_proxyPass = password;

    /* at first, delete old one */
    curl_multi_del_authcache(m_curlMultiHandle, m_proxy.utf8().data(), false);
    removeAllScheduledJobs();
    removeAllRunningJobs();
    m_jsSequential = false;
    curl_multi_cleanup(m_curlMultiHandle);

    m_curlMultiHandle = curl_multi_init();
    curl_multi_setopt(m_curlMultiHandle, CURLMOPT_MAXCONNECTS, m_tcpConnections);

    if (0 == host.length()) {
        m_proxy = String("");
        m_proxyUser = String("");
        m_proxyPass = String("");
        m_willChallengeProxyAuth = false;
    }
    else {
        m_proxy = String("http://") + host + ":" + String::number(m_proxyPort);
        m_willChallengeProxyAuth = true;
    }
}

void ResourceHandleManager::setProxyUserPass(const String& username, const String& password)
{
    if (Basic == m_proxyAuth && username.find(":") < 0)
        m_proxyUser = username;
    else
        m_proxyUser = "";
    m_proxyPass = password;
    m_willChallengeProxyAuth = true;
}

void ResourceHandleManager::didReceiveProxyAuthChallenge(ResourceHandle* job, ResourceHandleInternal* d)
{
    job->didReceiveProxyAuthenticationChallenge(d->m_webChallenge, m_proxyHost, m_proxyPort, (long)m_proxyAuth);
}

bool ResourceHandleManager::hasProxyUserPass()
{
    ResourceHandleManager* self = ResourceHandleManager::sharedInstance();

    if (self->m_proxyUser.length() || self->m_proxyPass.length())
        return true;
    else
        return false;
}

void ResourceHandleManager::removeFromCurl(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>removeFromCurl(%p)", job));

    ResourceHandleInternal* d = job->getInternal();
    ASSERT(d->m_handle);
    if (!d->m_handle)
        return;

    while (!d->m_webChallenge.isNull() && d->m_doAuthChallenge) {
        // re-use d->m_handle

        long httpcode = 0;
        curl_easy_getinfo(d->m_handle, CURLINFO_RESPONSE_CODE, &httpcode);
        d->m_doAuthChallenge = false;

        if (0 == httpcode && d->m_isSSL) {
            // if Proxy-Auth joins SSL, httpcode returns zero.
            // So, use CONNECTCODE as RESPONSE_CODE.
            curl_easy_getinfo(d->m_handle, CURLINFO_HTTP_CONNECTCODE, &httpcode);
        }

        if (httpcode == 407 && !m_proxy.length()) {
            break;
        }

        if (httpcode == 401 || httpcode == 407) {
            curl_multi_remove_handle(m_curlMultiHandle, d->m_handle);

            Credential credential = d->m_webChallenge.proposedCredential();

            long parameter;

            switch(d->m_webChallenge.protectionSpace().authenticationScheme()) {
            case ProtectionSpaceAuthenticationSchemeHTTPDigest:
                parameter = CURLAUTH_DIGEST;
                break;
            case ProtectionSpaceAuthenticationSchemeNTLM:
                parameter = CURLAUTH_NTLM;
                break;
            case ProtectionSpaceAuthenticationSchemeHTTPBasic:
            case ProtectionSpaceAuthenticationSchemeDefault:
            default:
                parameter = CURLAUTH_BASIC;
                break;
            }

            if (401 == httpcode) {
                curl_easy_setopt(d->m_handle, CURLOPT_HTTPAUTH, parameter);
                if (parameter == CURLAUTH_BASIC)
                    curl_easy_setopt(d->m_handle, CURLOPT_USERNAME, (credential.user().find(":") < 0) ? credential.user().utf8().data() : "");
                else
                    curl_easy_setopt(d->m_handle, CURLOPT_USERNAME, credential.user().utf8().data());
                curl_easy_setopt(d->m_handle, CURLOPT_PASSWORD, credential.password().utf8().data());
                FUNCTIONMOREPRINTF(("<rhm> WWW Authentication: [%p]%s", job, d->m_url));
            }
            else {
                curl_easy_setopt(d->m_handle, CURLOPT_PROXYAUTH, parameter);
                curl_easy_setopt(d->m_handle, CURLOPT_PROXYUSERNAME, m_proxyUser.utf8().data());
                curl_easy_setopt(d->m_handle, CURLOPT_PROXYPASSWORD, m_proxyPass.utf8().data());
                m_proxyUserCache = m_proxyUser;
                m_proxyPassCache = m_proxyPass;
                m_proxyUser = "";
                m_proxyPass = "";
                FUNCTIONMOREPRINTF(("<rhm> Proxy Authentication: [%p]%s", job, d->m_url));
            }

            curl_multi_add_handle(m_curlMultiHandle, d->m_handle);

            if (!m_downloadTimer.isActive()) {
                FUNCTIONMOREPRINTF(("<rhm>startOneShot(%f) by removeFromCurl(%p)", pollTimeSeconds, job));
                m_downloadTimer.startOneShot(pollTimeSeconds);
            }
            return;
        }
        break;
    }

    removeRunningJob(job);
    curl_multi_remove_handle(m_curlMultiHandle, d->m_handle);
    curl_easy_cleanup(d->m_handle);
    d->m_handle = 0;
    if (m_cookiesDeleting) {
        m_cookiesDeleting = false;
        clearCookies();
    }
    job->deref();
}

void ResourceHandleManager::setupPUT(ResourceHandle* job, struct curl_slist** headers)
{
    FUNCTIONPRINTF(("<rhm>setupPUT()"));

    ResourceHandleInternal* d = job->getInternal();
    curl_easy_setopt(d->m_handle, CURLOPT_UPLOAD, TRUE);
    curl_easy_setopt(d->m_handle, CURLOPT_INFILESIZE, 0);

    if (!job->request().httpBody())
        return;

    Vector<FormDataElement> elements = job->request().httpBody()->elements();
    size_t numElements = elements.size();
    if (!numElements)
        return;

    // Obtain the total size of the PUT 
    // The size of a curl_off_t could be different in WebKit and in cURL depending on
    // compilation flags of both. For CURLOPT_INFILESIZE_LARGE we have to pass the
    // right size or random data will be used as the size.
    static int expectedSizeOfCurlOffT = 0;
    if (!expectedSizeOfCurlOffT) {
        curl_version_info_data *infoData = curl_version_info(CURLVERSION_NOW);
        if (infoData->features & CURL_VERSION_LARGEFILE)
            expectedSizeOfCurlOffT = sizeof(long long);
        else
            expectedSizeOfCurlOffT = sizeof(int);
    }

#if COMPILER(MSVC)
    // work around compiler error in Visual Studio 2005.  It can't properly
    // handle math with 64-bit constant declarations.
#pragma warning(disable: 4307)
#endif
    static const long long maxCurlOffT = (1LL << (expectedSizeOfCurlOffT * 8 - 1)) - 1;
    curl_off_t size = 0;
    bool chunkedTransfer = false;
    for (size_t i = 0; i < numElements; i++) {
        FormDataElement element = elements[i];
        if (element.m_type == FormDataElement::encodedFile) {
            long long fileSizeResult;
            if (getFileSize(element.m_filename, fileSizeResult)) {
                if (fileSizeResult > maxCurlOffT) {
                    // File size is too big for specifying it to cURL
                    chunkedTransfer = true;
                    break;
                } else if (fileSizeResult < 0) {
                    chunkedTransfer = true;
                    break;
                }
                size += fileSizeResult;
            }
        } else
            size += elements[i].m_data.size();
    }

    // cURL guesses that we want chunked encoding as long as we specify the header
    if (chunkedTransfer) {
        *headers = curl_slist_append(*headers, "Transfer-Encoding: chunked");
    }
    else {
        if (sizeof(long long) == expectedSizeOfCurlOffT)
          curl_easy_setopt(d->m_handle, CURLOPT_INFILESIZE_LARGE, (long long)size);
        else
          curl_easy_setopt(d->m_handle, CURLOPT_INFILESIZE_LARGE, (int)size);
    }

    curl_easy_setopt(d->m_handle, CURLOPT_READFUNCTION, readCallback);
    curl_easy_setopt(d->m_handle, CURLOPT_READDATA, job);
}

/* Calculate the length of the POST.
   Force chunked data transfer if size of files can't be obtained.
 */
void ResourceHandleManager::setupPOST(ResourceHandle* job, struct curl_slist** headers)
{
    FUNCTIONPRINTF(("<rhm>setupPOST()"));

    ResourceHandleInternal* d = job->getInternal();
    curl_easy_setopt(d->m_handle, CURLOPT_POST, TRUE);
    curl_easy_setopt(d->m_handle, CURLOPT_POSTFIELDSIZE, 0);

    if (!job->request().httpBody())
        return;

    Vector<FormDataElement> elements = job->request().httpBody()->elements();
    size_t numElements = elements.size();
    if (!numElements)
        return;

    // Do not stream for simple POST data
    if (numElements == 1) {
        job->request().httpBody()->flatten(d->m_postBytes);
        if (d->m_postBytes.size() != 0) {
            curl_easy_setopt(d->m_handle, CURLOPT_POSTFIELDSIZE, d->m_postBytes.size());
            curl_easy_setopt(d->m_handle, CURLOPT_POSTFIELDS, d->m_postBytes.data());
        }
        return;
    }

    // Obtain the total size of the POST
    // The size of a curl_off_t could be different in WebKit and in cURL depending on
    // compilation flags of both. For CURLOPT_POSTFIELDSIZE_LARGE we have to pass the
    // right size or random data will be used as the size.
    static int expectedSizeOfCurlOffT = 0;
    if (!expectedSizeOfCurlOffT) {
        curl_version_info_data *infoData = curl_version_info(CURLVERSION_NOW);
        if (infoData->features & CURL_VERSION_LARGEFILE)
            expectedSizeOfCurlOffT = sizeof(long long);
        else
            expectedSizeOfCurlOffT = sizeof(int);
    }

#if COMPILER(MSVC)
    // work around compiler error in Visual Studio 2005.  It can't properly
    // handle math with 64-bit constant declarations.
#pragma warning(disable: 4307)
#endif
    static const long long maxCurlOffT = (1LL << (expectedSizeOfCurlOffT * 8 - 1)) - 1;
    curl_off_t size = 0;
    bool chunkedTransfer = false;
    for (size_t i = 0; i < numElements; i++) {
        FormDataElement element = elements[i];
        if (element.m_type == FormDataElement::encodedFile) {
            long long fileSizeResult;
            if (getFileSize(element.m_filename, fileSizeResult)) {
                if (fileSizeResult > maxCurlOffT) {
                    // File size is too big for specifying it to cURL
                    chunkedTransfer = true;
                    break;
                } else if (fileSizeResult < 0) {
                    chunkedTransfer = true;
                    break;
                }
                size += fileSizeResult;
            }
        } else
            size += elements[i].m_data.size();
    }

    // cURL guesses that we want chunked encoding as long as we specify the header
    if (chunkedTransfer) {
        *headers = curl_slist_append(*headers, "Transfer-Encoding: chunked");
    }
    else {
        if (sizeof(long long) == expectedSizeOfCurlOffT)
          curl_easy_setopt(d->m_handle, CURLOPT_POSTFIELDSIZE_LARGE, (long long)size);
        else
          curl_easy_setopt(d->m_handle, CURLOPT_POSTFIELDSIZE_LARGE, (int)size);
    }

    curl_easy_setopt(d->m_handle, CURLOPT_READFUNCTION, readCallback);
    curl_easy_setopt(d->m_handle, CURLOPT_READDATA, job);

    curl_easy_setopt(d->m_handle, CURLOPT_IOCTLFUNCTION, ioctlCallback);
    curl_easy_setopt(d->m_handle, CURLOPT_IOCTLDATA, job);
}

void ResourceHandleManager::add(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>add(%p)", job));

    // we can be called from within curl, so to avoid re-entrancy issues
    // schedule this job to be added the next time we enter curl download loop
    job->ref();
    appendScheduledJob(job);

    if (!m_downloadTimer.isActive()) {
        FUNCTIONMOREPRINTF(("<rhm>startOneShot(%f) by add(%p)", pollTimeSeconds, job));
        m_downloadTimer.startOneShot(pollTimeSeconds);
    }
}

void ResourceHandleManager::doAuthChallenge(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>doAuthChallenge(%p)", job));

    ResourceHandleInternal* d = job->getInternal();
    if (!d) return;

    if (d->m_webChallenge.isNull()) {
        // not web authentication
        return;
    }

    d->m_doAuthChallenge = true;
}

void ResourceHandleManager::cancelAuthChallenge(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>cancelAuthChallenge(%p)", job));

    ResourceHandleInternal* d = job->getInternal();
    if (!d) return;

    d->m_doAuthChallenge = false;
}

bool ResourceHandleManager::startScheduledJobs()
{
    FUNCTIONPRINTF(("<rhm>startScheduledJobs()"));

    bool started = false;
    while (!m_scheduledJobList.isEmpty() && m_runningJobList.size() < m_tcpConnections) {
        if (m_willChallengeProxyAuth && m_runningJobList.size() > 0) {
            break;
        }
        ResourceHandle* job = shiftScheduledJob();
        if (!job)
            return true;
        startJob(job);

        ResourceHandleInternal* d = job->getInternal();
        if (d && d->m_cancelled){
            d->m_handle = 0;
            job->deref();
            continue;
        }
            
        started = true;
    }
    return started;
}

void ResourceHandleManager::cancelScheduledJobs()
{
    FUNCTIONPRINTF(("<rhm>cancelScheduledJobs()"));

    ResourceHandle* job;
    ResourceHandleInternal* d;
    bool loop = true;
    int size;

    while (loop) {
        loop = false;
        size = m_runningJobList.size();
        for (int i = 0; i < size; i++) {
            job = m_runningJobList[i];
            d = job->getInternal();
            if (d && d->m_cancelled) {
                FUNCTIONMOREPRINTF(("<rhm>cancelScheduledJobs(): %s", d->m_url));
                m_runningJobList.remove(i);

                if (d->m_isJSRequestatFirst)
                    m_jsSequential = false;

                curl_multi_remove_handle(m_curlMultiHandle, d->m_handle);
                curl_easy_cleanup(d->m_handle);
                d->m_handle = 0;
                job->deref();
                loop = true;
                break;
            }
        }
    }
    if (m_cookiesDeleting) {
        m_cookiesDeleting = false;
        clearCookies();
    }
}

static void parseDataUrl(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>parseDataUrl(%p)", job));

    ResourceHandleClient* client = job->client();
    ASSERT(client);
    if (!client)
        return;

    String url = job->request().url().string();
    ASSERT(url.startsWith("data:", false));

    int index = url.find(',');
    if (index == -1) {
        client->cannotShowURL(job);
        return;
    }

    job->setDataSchemeDownloading(true);

    String mediaType = url.substring(5, index - 5);
    String data = url.substring(index + 1);

    bool base64 = mediaType.endsWith(";base64", false);
    if (base64)
        mediaType = mediaType.left(mediaType.length() - 7);

    if (mediaType.isEmpty())
        mediaType = "text/plain;charset=US-ASCII";

    String mimeType = extractMIMETypeFromMediaType(mediaType);
    String charset = extractCharsetFromMediaType(mediaType);

    ResourceResponse response;
    response.setMimeType(mimeType);

    if (base64) {
        data = decodeURLEscapeSequences(data);
        response.setTextEncodingName(charset);
        client->didReceiveResponse(job, response);
        if (job->getInternal()->m_cancelled)
            return;
        client = job->client();

        // WebCore's decoder fails on Acid3 test 97 (whitespace).
        Vector<char> out;
        if (client && base64Decode(data.latin1().data(), data.latin1().length(), out) && out.size() > 0) {
            client->willReceiveData(job, out.size());
            if (job->getInternal()->m_cancelled)
                return;
            client->didReceiveData(job, out.data(), out.size(), 0);
            if (job->getInternal()->m_cancelled)
                return;
        }
    } else {
        // We have to convert to UTF-16 early due to limitations in KURL
        data = decodeURLEscapeSequences(data, TextEncoding(charset));
        response.setTextEncodingName("UTF-16");
        client->didReceiveResponse(job, response);
        if (job->getInternal()->m_cancelled) 
            return;
        client = job->client();

        if (client && data.length() > 0) {
            client->willReceiveData(job, data.length() * sizeof(UChar));
            if (job->getInternal()->m_cancelled)
                return;
            client->didReceiveData(job, reinterpret_cast<const char*>(data.characters()), data.length() * sizeof(UChar), 0);
            if (job->getInternal()->m_cancelled) 
                return;
        }
    }

    if (client)
        client->didFinishLoading(job);
}

void ResourceHandleManager::dispatchSynchronousJob(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>dispatchSynchronousJob(%p)", job));

    KURL kurl = job->request().url();

    if (kurl.protocolIs("data")) {
        parseDataUrl(job);
        return;
    }

    ResourceHandleInternal* d = job->getInternal();

#if LIBCURL_VERSION_NUM > 0x071200
    // If defersLoading is true and we call curl_easy_perform
    // on a paused handle, libcURL would do the transfert anyway
    // and we would assert so force defersLoading to be false.
    d->m_defersLoading = false;
#endif

    initializeHandle(job);
    if (!d->m_handle) {
        ERRORPRINTF(("<rhm>dispatchSynchronousJob(): No Memory"));
        if (d->client())
            d->client()->didFail(job, ResourceError(String(), CURLE_OUT_OF_MEMORY, String(d->m_url), String("no memory"), 0));
        return;
    }
    d->m_isSynchronous = true;

    bool changedProxyUser = false;
    bool changedProxyPass = false;
    if (m_proxyUser.isEmpty()) {
        m_proxyUser = m_proxyUserCache;
        changedProxyUser = true;
    }
    if (m_proxyPass.isEmpty()) {
        m_proxyPass = m_proxyPassCache;
        changedProxyPass = true;
    }

    bool retry;
    do {
        retry = false;
        
        // curl_easy_perform blocks until the transfert is finished.
        CURLcode ret =  curl_easy_perform(d->m_handle);
        
        while (!d->m_webChallenge.isNull() && d->m_doAuthChallenge) {
            // re-use d->m_handle

            long httpcode = 0;
            curl_easy_getinfo(d->m_handle, CURLINFO_RESPONSE_CODE, &httpcode);
            d->m_doAuthChallenge = false;

            if (0 == httpcode && d->m_isSSL) {
                // if Proxy-Auth joins SSL, httpcode returns zero.
                // So, use CONNECTCODE as RESPONSE_CODE.
                curl_easy_getinfo(d->m_handle, CURLINFO_HTTP_CONNECTCODE, &httpcode);
            }

            if (httpcode == 407 && !m_proxy.length()) {
                break;
            }

            if (httpcode == 401 || httpcode == 407) {
                Credential credential = d->m_webChallenge.proposedCredential();

                long parameter;

                switch(d->m_webChallenge.protectionSpace().authenticationScheme()) {
                    case ProtectionSpaceAuthenticationSchemeHTTPDigest:
                        parameter = CURLAUTH_DIGEST;
                        break;
                    case ProtectionSpaceAuthenticationSchemeNTLM:
                        parameter = CURLAUTH_NTLM;
                        break;
                    case ProtectionSpaceAuthenticationSchemeHTTPBasic:
                    case ProtectionSpaceAuthenticationSchemeDefault:
                    default:
                        parameter = CURLAUTH_BASIC;
                        break;
                }

                if (401 == httpcode) {
                    curl_easy_setopt(d->m_handle, CURLOPT_HTTPAUTH, parameter);
                    if (parameter == CURLAUTH_BASIC)
                        curl_easy_setopt(d->m_handle, CURLOPT_USERNAME, (credential.user().find(":") < 0) ? credential.user().utf8().data() : "");
                    else
                        curl_easy_setopt(d->m_handle, CURLOPT_USERNAME, credential.user().utf8().data());
                    curl_easy_setopt(d->m_handle, CURLOPT_PASSWORD, credential.password().utf8().data());
                    FUNCTIONMOREPRINTF(("<rhm> WWW Authentication: [%p]%s", job, d->m_url));
                }
                else {
                    curl_easy_setopt(d->m_handle, CURLOPT_PROXYAUTH, parameter);
                    curl_easy_setopt(d->m_handle, CURLOPT_PROXYUSERNAME, m_proxyUser.utf8().data());
                    curl_easy_setopt(d->m_handle, CURLOPT_PROXYPASSWORD, m_proxyPass.utf8().data());

                    if (changedProxyUser) {
                        m_proxyUser = "";
                    }
                    if (changedProxyPass) {
                        m_proxyPass = "";
                    }

                    FUNCTIONMOREPRINTF(("<rhm> Proxy Authentication: [%p]%s", job, d->m_url));
                }
                retry = true;
            }
            break;
        }
        if (!retry && ret != 0) {
            char* url = 0;
            curl_easy_getinfo(d->m_handle, CURLINFO_EFFECTIVE_URL, &url);
            ERRORPRINTF(("Curl sync ERROR for url='%s', error: '[%d]%s'", url, ret, curl_easy_strerror(ret)));
            if (d->client())
                d->client()->didFail(job, ResourceError(String(), ret, String(url), String(curl_easy_strerror(ret)), 0));
        }
    } while (retry);

    curl_easy_cleanup(d->m_handle);

    if (changedProxyUser) m_proxyUser = "";
    if (changedProxyPass) m_proxyPass = "";
}

void ResourceHandleManager::startJob(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>startJob(%p)", job));

    KURL kurl = job->request().url();

    if (kurl.protocolIs("data")) {
        parseDataUrl(job);
        job->cancel();
        return;
    }

    ResourceHandleInternal* d = job->getInternal();

    initializeHandle(job);
    if (!d->m_handle) {
        ERRORPRINTF(("<rhm>startJob(): No Memory"));
        job->cancel();
        return;
    }

    CURLMcode ret = curl_multi_add_handle(m_curlMultiHandle, d->m_handle);
    // don't call perform, because events must be async
    // timeout will occur and do curl_multi_perform
    if (ret && ret != CURLM_CALL_MULTI_PERFORM) {
        ERRORPRINTF(("Error %d starting job %s\n", ret, encodeWithURLEscapeSequences(job->request().url().string()).latin1().data()));
        job->cancel();
        return;
    }
    appendRunningJob(job);
}

void ResourceHandleManager::initializeHandle(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>initializeHandle(%p)", job));

    ResourceHandleInternal* d = job->getInternal();

    // at first, malloc memory
    d->m_handle = curl_easy_init();
    if (!d->m_handle) {
        return;
    }
    d->m_cancelled = false;

    KURL kurl = job->request().url();
    String url = kurl.string();

    // Remove any fragment part, otherwise curl will send it as part of the request.
    kurl.removeFragmentIdentifier();

    if (kurl.isLocalFile()) {
        String query = kurl.query();
        // Remove any query part sent to a local file.
        if (!query.isEmpty()) {
            int queryIndex = url.find(query);
            if (queryIndex != -1)
                url = url.left(queryIndex - 1);
        }
        // Determine the MIME type based on the path.
        d->m_response.setMimeType(MIMETypeRegistry::getMIMETypeForPath(url));
    }

#if LIBCURL_VERSION_NUM > 0x071200
    if (d->m_defersLoading) {
        CURLcode error = curl_easy_pause(d->m_handle, CURLPAUSE_ALL);
        // If we did not pause the handle, we would ASSERT in the
        // header callback. So just assert here.
        ASSERT(error == CURLE_OK);
    }
#endif

#ifdef CURL_DEBUG_CALLBACK
    curl_easy_setopt(d->m_handle, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(d->m_handle, CURLOPT_DEBUGFUNCTION, debugCallback);
#endif

    curl_easy_setopt(d->m_handle, CURLOPT_PRIVATE, job);
    curl_easy_setopt(d->m_handle, CURLOPT_ERRORBUFFER, m_curlErrorBuffer);
    curl_easy_setopt(d->m_handle, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(d->m_handle, CURLOPT_WRITEDATA, job);
    curl_easy_setopt(d->m_handle, CURLOPT_HEADERFUNCTION, headerCallback);
    curl_easy_setopt(d->m_handle, CURLOPT_WRITEHEADER, job);
    if (d->m_request.sendAutoHTTPReferfer())
        curl_easy_setopt(d->m_handle, CURLOPT_AUTOREFERER, 1);
    curl_easy_setopt(d->m_handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(d->m_handle, CURLOPT_MAXREDIRS, 10);
    curl_easy_setopt(d->m_handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
    curl_easy_setopt(d->m_handle, CURLOPT_SHARE, m_curlShareHandle);
    curl_easy_setopt(d->m_handle, CURLOPT_DNS_CACHE_TIMEOUT, 60 * 5); // 5 minutes
    // cookie
    curl_easy_setopt(d->m_handle, CURLOPT_COOKIEACCEPTFUNCTIONDATA, job);
    curl_easy_setopt(d->m_handle, CURLOPT_COOKIEACCEPTFUNCTION, cookie_accept_callback);

    // Set connect timeout.
    curl_easy_setopt(d->m_handle, CURLOPT_SERVER_RESPONSE_TIMEOUT, 20);
    curl_easy_setopt(d->m_handle, CURLOPT_CONNECTTIMEOUT, 20);

    if (equalIgnoringCase(kurl.protocol(), "https")) {
        d->m_isSSL = true;
        ResourceHandleManagerSSL::sharedInstance()->initializeHandleSSL(job);
    }

    // enable gzip and deflate through Accept-Encoding:
    curl_easy_setopt(d->m_handle, CURLOPT_ENCODING, "");

    // url must remain valid through the request
    //ASSERT(!d->m_url);

    // url is in ASCII so latin1() will only convert it to char* without character translation.
    // ~ResourceHandleInternal() will fastFree d->m_url.
    d->m_url = fastStrdup(url.latin1().data());
    d->m_urlhost = fastStrdup(hostAndPort(kurl).latin1().data());
    d->m_isInclusion = d->client()->isInclusionRequest();
    curl_easy_setopt(d->m_handle, CURLOPT_URL, d->m_url);

    struct curl_slist* headers = 0;

    // curl CURLOPT_USERPWD expects username:password
    if (d->m_user.length() || d->m_pass.length()) {
        // This case, http://user:pass@uri/, should be Basic authentication.
        String userpass = d->m_user + ":" + d->m_pass;
        curl_easy_setopt(d->m_handle, CURLOPT_USERPWD, userpass.utf8().data());
        curl_easy_setopt(d->m_handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    }

    if (job->request().httpHeaderFields().size() > 0) {
        HTTPHeaderMap customHeaders = job->request().httpHeaderFields();
        HTTPHeaderMap::const_iterator end = customHeaders.end();
        for (HTTPHeaderMap::const_iterator it = customHeaders.begin(); it != end; ++it) {
            String key = it->first;
            String value = it->second;
            String headerString(key);
            headerString.append(": ");
            headerString.append(value);
            CString headerLatin1 = headerString.latin1();
            headers = curl_slist_append(headers, headerLatin1.data());
        }
    }

    if ("GET" == job->request().httpMethod())
        curl_easy_setopt(d->m_handle, CURLOPT_HTTPGET, TRUE);
    else if ("POST" == job->request().httpMethod())
        setupPOST(job, &headers);
    else if ("PUT" == job->request().httpMethod())
        setupPUT(job, &headers);
    else if ("HEAD" == job->request().httpMethod())
        curl_easy_setopt(d->m_handle, CURLOPT_NOBODY, TRUE);

    if (headers) {
        curl_easy_setopt(d->m_handle, CURLOPT_HTTPHEADER, headers);
        d->m_customHeaders = headers;
    }

    // Set proxy options if we have them.
    if (m_proxy.length()) {
        curl_easy_setopt(d->m_handle, CURLOPT_PROXY, m_proxy.utf8().data());
        curl_easy_setopt(d->m_handle, CURLOPT_PROXYTYPE, m_proxyType);
        if (m_proxyAuth != NONE)
            curl_easy_setopt(d->m_handle, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        else
            curl_easy_setopt(d->m_handle, CURLOPT_PROXYAUTH, CURLAUTH_NONE);
    }

    d->m_response.setResourceHandle(job);
}

void ResourceHandleManager::cancel(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>cancel(%p)", job));

    if (removeScheduledJob(job))
        return;

    ResourceHandleInternal* d = job->getInternal();
    if (!d)
        return;

    d->m_cancelled = true;
    if (!d->m_handle)
        return;

    if (!m_downloadTimer.isActive()) {
        FUNCTIONMOREPRINTF(("<rhm>startOneShot(%f) by cancel(%p)", pollTimeSeconds, job));
        m_downloadTimer.startOneShot(pollTimeSeconds);
    }
}

/*
 *  Cookie
 */
void ResourceHandleManager::cookieSerializeStart()
{
    wkcMutexLockPeer(gCookieMutex);
    m_cookieInfo = curl_share_cookie_list(m_curlShareHandle);
    m_nextCookie = 0;
    m_serializeEnd = false;
}

int ResourceHandleManager::cookieSerializeProgress(char* cookie, unsigned int len, unsigned int* writtenlen)
{
    struct curl_slist* list;

    if (!m_cookieInfo)
        return 1;

    if (m_serializeEnd) {
        if (writtenlen)
            *writtenlen = 0;
        return 0;
    }

    if (m_nextCookie)
        list = m_nextCookie;
    else
        list = m_cookieInfo;

    unsigned int datalen = (unsigned int)strlen(list->data);
    if (writtenlen)
        *writtenlen = datalen;

    if (!cookie)
        return 0;

    if(datalen >= len) {
        return 1;
    }
    memcpy(cookie, list->data, datalen);
    cookie[datalen] = 0x00;

    m_nextCookie = list->next;
    if (!m_nextCookie)
        m_serializeEnd = true;

    return 0;
}

void ResourceHandleManager::cookieSerializeEnd()
{
    curl_slist_free_all(m_cookieInfo);
    m_cookieInfo = 0;
    m_nextCookie = 0;
    m_serializeEnd = false;
    wkcMutexUnlockPeer(gCookieMutex);
}

void ResourceHandleManager::cookieDeserializeStart(bool restart)
{
    wkcMutexLockPeer(gCookieMutex);
    m_CookieRestartDeserialize = restart;
}

int ResourceHandleManager::cookieDeserializeProgress(const char *cookie, unsigned int len)
{
    CURLSHcode code;
    char *lineptr = NULL;

    lineptr = (char *)ResourceHandleManagerMalloc(len + 1);
    if (!lineptr)
        return 1;
    memcpy(lineptr, cookie, len);
    lineptr[len] = 0x00;

    code = curl_share_set_cookie(m_curlShareHandle, NULL, NULL, lineptr, (m_CookieRestartDeserialize)?COOKIETYPE_COOKIELIST_ALL:COOKIETYPE_COOKIELIST);

    ResourceHandleManagerFree(lineptr);

    return (code)?1:0;
}

void ResourceHandleManager::cookieDeserializeEnd()
{
    m_CookieRestartDeserialize = false;
    wkcMutexUnlockPeer(gCookieMutex);
}

void ResourceHandleManager::setCookie(const String &domain, const String &path, const String &cookie)
{
    wkcMutexLockPeer(gCookieMutex);

    curl_share_set_cookie(m_curlShareHandle, domain.latin1().data(), path.latin1().data(), cookie.latin1().data(), COOKIETYPE_HEADERLINE);

    wkcMutexUnlockPeer(gCookieMutex);
}

String ResourceHandleManager::getCookie(const String &domain, const String &path, bool secure)
{
    wkcMutexLockPeer(gCookieMutex);
    int len = 0;
    char* buf = 0;

    len = curl_share_get_cookie(m_curlShareHandle, domain.latin1().data(), path.latin1().data(), secure ? TRUE : FALSE, NULL, 0);
    if (len>0) {
        buf = (char *)ResourceHandleManagerMalloc(len+1);
        if (buf) {
            len = curl_share_get_cookie(m_curlShareHandle, domain.latin1().data(), path.latin1().data(), secure ? TRUE : FALSE, buf, len+1);
        }
    }

    wkcMutexUnlockPeer(gCookieMutex);

    if (buf) {
        String result(buf);
        ResourceHandleManagerFree(buf);
        return result;
    } else {
        return String();
    }
}


// Shared Cookie Manipulation
void ResourceHandleManager::sharedCookieSerializeStart()
{
    m_sharedInstance->cookieSerializeStart();
}

int ResourceHandleManager::sharedCookieSerializeProgress(char* data, unsigned int len, unsigned int* writtenlen)
{
    return m_sharedInstance->cookieSerializeProgress(data, len, writtenlen);
}

void ResourceHandleManager::sharedCookieSerializeEnd()
{
    m_sharedInstance->cookieSerializeEnd();
}

void ResourceHandleManager::sharedCookieDeserializeStart(bool restart)
{
    m_sharedInstance->cookieDeserializeStart(restart);
}

int ResourceHandleManager::sharedCookieDeserializeProgress(const char *data, unsigned int len)
{
    return m_sharedInstance->cookieDeserializeProgress(data, len);
}

void ResourceHandleManager::sharedCookieDeserializeEnd()
{
    m_sharedInstance->cookieDeserializeEnd();
}

void ResourceHandleManager::sharedCookieSet(const String &domain, const String &path, const String &cookie)
{
    m_sharedInstance->setCookie(domain, path, cookie);
}

String ResourceHandleManager::sharedCookieGet(const String &domain, const String &path, bool secure)
{
    return m_sharedInstance->getCookie(domain, path, secure);
}

//
// Reset Variables
//
void ResourceHandleManager::resetVariables()
{
    ResourceHandleManagerSSL::resetVariables();

    m_sharedInstance = 0;
}

//
// Running job control
//
void ResourceHandleManager::appendScheduledJob(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>appendScheduledJob(%p)", job));

    if (job->request().url().lastPathComponent().endsWith(".js"))
        job->getInternal()->m_isJSRequestatFirst = true;

    m_scheduledJobList.append(job);
}

bool ResourceHandleManager::removeScheduledJob(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>removeScheduledJob(%p)", job));

    int size = m_scheduledJobList.size();
    for (int i = 0; i < size; i++) {
        if (job == m_scheduledJobList[i]) {
            m_scheduledJobList.remove(i);
            job->deref();
            return true;
        }
    }
    return false;
}

ResourceHandle* ResourceHandleManager::shiftScheduledJob()
{
    FUNCTIONPRINTF(("<rhm>shiftScheduledJob()"));

    int size = m_scheduledJobList.size();
    if (0 == size)
        return (ResourceHandle*)0;

    ResourceHandle* job = 0;
#if 0
    job = m_scheduledJobList[0];
    m_scheduledJobList.remove(0);
    return job;
#else
    if (!m_jsSequential) {
        job = m_scheduledJobList[0];
        m_scheduledJobList.remove(0);
        return job;
    }
    else {
        int i;
        for (i = 0; i < size; i++) {
            job = m_scheduledJobList[i];
            ResourceHandleInternal* d = job->getInternal();
            if (d && !d->m_isJSRequestatFirst) {
               m_scheduledJobList.remove(i);
               return job;
            }
        }
        return (ResourceHandle*)0;
    }
#endif
}

void ResourceHandleManager::removeAllScheduledJobs()
{
    FUNCTIONPRINTF(("<rhm>removeAllScheduledJobs()"));

    ResourceHandle* job;
    int num = m_scheduledJobList.size();

    while (num-- > 0) {
        job = m_scheduledJobList[num];
        m_scheduledJobList.remove(num);
        job->deref();
    }
}

//
// Running job control
//
void ResourceHandleManager::appendRunningJob(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>appendRunningJob(%p)", job));

    ResourceHandleInternal* d = job->getInternal();

    if (d && d->m_isJSRequestatFirst)
        m_jsSequential = true;

    m_runningJobList.append(job);
    LOADINGPRINTF(("Start  Loading [%p] %s", job, d->m_url));
}

bool ResourceHandleManager::removeRunningJob(ResourceHandle* job)
{
    FUNCTIONPRINTF(("<rhm>removeRunningJob(%p)", job));

    ResourceHandleInternal* d = job->getInternal();
    bool ret = false;
    int size = m_runningJobList.size();
    for (int i = 0; i < size; i++) {
        if (job == m_runningJobList[i]) {
            LOADINGPRINTF(("Finish Loading [%p]", job));
            m_runningJobList.remove(i);
            m_willChallengeProxyAuth = false;
            ret = true;
            break;
        }
    }

    if (ret && d) {
        if (d->m_isJSRequestatFirst)
            m_jsSequential = false;
        if (d->m_isSSL)
            ResourceHandleManagerSSL::sharedInstance()->removeRunningJobSSL(job);
    }

    return ret;
}

ResourceHandle* ResourceHandleManager::shiftRunningJob()
{
    /* only it is called removeAllRunningJobs() */
    FUNCTIONPRINTF(("<rhm>shiftRunningJob()"));

    int size = m_runningJobList.size();
    ResourceHandle* job = 0;
    if (0 < size) {
        job = m_runningJobList[0];
        m_runningJobList.remove(0);
    }
    return job;
}

void ResourceHandleManager::removeAllRunningJobs()
{
    FUNCTIONPRINTF(("<rhm>removeAllRunningJobs()"));

    ResourceHandle* job;

    while ((job = shiftRunningJob())) {
        ResourceHandleInternal* d = job->getInternal();
        if (d && d->m_handle) {
            curl_multi_remove_handle(m_curlMultiHandle, d->m_handle);
            curl_easy_cleanup(d->m_handle);
            d->m_handle = 0;
        }
        job->deref();
    }
}

} // namespace WebCore

