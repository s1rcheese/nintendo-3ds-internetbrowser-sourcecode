/*
    ResourceHandleManager.oneshot.h
 */

/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
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

#ifndef ResourceHandleManager_h
#define ResourceHandleManager_h

#include "CString.h"
#include "Frame.h"
#include "PlatformString.h"
#include "Timer.h"
#include "ResourceHandleClient.h"


#if PLATFORM(WIN)
#include <winsock2.h>
#include <windows.h>
#endif

#include <curl/curl.h>
#include <wtf/Vector.h>

namespace WebCore {

class ResourceHandleManager {
public:
    enum ProxyType {
        HTTP    = CURLPROXY_HTTP,
        HTTP10  = CURLPROXY_HTTP_1_0,
        Socks4  = CURLPROXY_SOCKS4,   // not support
        Socks4A = CURLPROXY_SOCKS4A, // not support
        Socks5  = CURLPROXY_SOCKS5,   // not support
        Socks5Hostname = CURLPROXY_SOCKS5_HOSTNAME   // not support
    };

    enum ProxyAuth {
        Basic  = CURLAUTH_BASIC,
        Digest = CURLAUTH_DIGEST,
        NTLM   = CURLAUTH_NTLM,
        NONE   = CURLAUTH_NONE   // not use proxy auth
    };

    // For Singleton
    static bool createSharedInstance();
    static ResourceHandleManager* sharedInstance();
    static void deleteSharedInstance();
    static bool isExistSharedInstance();
    static void forceTerminateInstance();

    void add(ResourceHandle*);
    void cancel(ResourceHandle*);
    void setCookieJarFileName(const char* cookieJarFileName);
    void setMaxTCPConnections(long number);
    void setMaxCookieEntries(long number);

    void doAuthChallenge(ResourceHandle*);
    void cancelAuthChallenge(ResourceHandle*);

    void dispatchSynchronousJob(ResourceHandle*);

    void setProxyInfo(const String& host = "",
                      unsigned long port = 0,
                      ProxyType type = HTTP,
                      const String& username = "",
                      const String& password = "",
                      ProxyAuth auth = Basic);
    void setProxyUserPass(const String& username = "", const String& password = "");
    void didReceiveProxyAuthChallenge(ResourceHandle* job, ResourceHandleInternal* d);
    static bool hasProxyUserPass();

    void cookieSerializeStart();
    int cookieSerializeProgress(char*, unsigned int, unsigned int*);
    void cookieSerializeEnd();
    void cookieDeserializeStart(bool);
    int cookieDeserializeProgress(const char*, unsigned int);
    void cookieDeserializeEnd();
    void clearCookies();
    void setCookie(const String &domain, const String &path, const String &cookie);
    String getCookie(const String &domain, const String &path, bool secure);

    // Shared Cookie Manipulation
    static void sharedCookieSerializeStart();
    static int sharedCookieSerializeProgress(char*, unsigned int, unsigned int*);
    static void sharedCookieSerializeEnd();
    static void sharedCookieDeserializeStart(bool);
    static int sharedCookieDeserializeProgress(const char*, unsigned int);
    static void sharedCookieDeserializeEnd();
    static void sharedCookieSet(const String &domain, const String &path, const String &cookie);
    static String sharedCookieGet(const String &domain, const String &path, bool secure);

    // Add Cookie
    static bool addCookie(const String &domain, const String &path, const String &cookie);

    static void resetVariables();

    CURLSH* sharehandle() {return m_curlShareHandle;}

private:
    ResourceHandleManager();
    ~ResourceHandleManager();
    static ResourceHandleManager* create();
    bool construct();
    void downloadTimerCallback(Timer<ResourceHandleManager>*);
    void removeFromCurl(ResourceHandle*);
    void startJob(ResourceHandle*);
    bool startScheduledJobs();
    void cancelScheduledJobs();

    void initializeHandle(ResourceHandle*);

    void setupPOST(ResourceHandle*, struct curl_slist**);
    void setupPUT(ResourceHandle*, struct curl_slist**);

    Timer<ResourceHandleManager> m_downloadTimer;
    CURLM* m_curlMultiHandle;
    CURLSH* m_curlShareHandle;
    char* m_cookieJarFileName;
    char m_curlErrorBuffer[CURL_ERROR_SIZE];

    // from add() to startScheduledJobs() or cancel()
    Vector<ResourceHandle*> m_scheduledJobList;
    void appendScheduledJob(ResourceHandle*);
    bool removeScheduledJob(ResourceHandle*);  // from cancel()
    ResourceHandle* shiftScheduledJob();
    void removeAllScheduledJobs();

    // from curl_multi_add_handle() to curl_multi_remove_handle()/curl_easy_cleanup()
    // max is m_tcpConnections
    Vector<ResourceHandle*> m_runningJobList;
    void appendRunningJob(ResourceHandle* job);
    bool removeRunningJob(ResourceHandle* job);
    ResourceHandle* shiftRunningJob();  // for deleteing all jobs
    void removeAllRunningJobs();

    String m_proxy;
    String m_proxyHost;
    int    m_proxyPort;
    ProxyType m_proxyType;
    String m_proxyUser;
    String m_proxyPass;
    ProxyAuth m_proxyAuth;

    String m_proxyUserCache;
    String m_proxyPassCache;

    bool m_willChallengeProxyAuth;

    struct curl_slist* m_cookieInfo;
    struct curl_slist* m_nextCookie;
    bool m_serializeEnd;
    bool m_cookiesDeleting;
    bool m_CookieRestartDeserialize;
    int m_cookieMaxEntries;

    // For Singleton
    void forceTerminate();
    static ResourceHandleManager *m_sharedInstance;
    static bool m_forceTerminating;

    // TCP connections
    int m_tcpConnections;

    // JavaScript Sequential
    bool m_jsSequential;
};

}

#endif
