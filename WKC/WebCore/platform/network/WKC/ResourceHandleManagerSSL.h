/*
 * Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.
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

#ifndef ResourceHandleManagerSSL_h
#define ResourceHandleManagerSSL_h

#include "CString.h"
#include "PlatformString.h"

#if PLATFORM(WIN)
#include <winsock2.h>
#include <windows.h>
#endif

#include <curl/curl.h>

namespace WebCore {

class ClientCertificate;

class ResourceHandleManagerSSL {
public:
    // For Singleton
    static bool createSharedInstance(CURLM* curlMultiHandle);
    static void deleteSharedInstance();
    static ResourceHandleManagerSSL* sharedInstance();
    static void resetVariables();

    void* SSLRegisterClientCert(const unsigned char* pkcs12, int pkcs12_len, const unsigned char* pass, int pass_len);
    int   SSLUnregisterClientCert(void* certid);

    void initializeHandleSSL(ResourceHandle*);
    void removeRunningJobSSL(ResourceHandle*);

    bool allowsServerHost(const String& host);
    void setAllowServerCA(const char *host_w_port, const char *ca);
    void removeAllowServerCA(const char *host_w_port);
    void setAllowServerHost(const char *host_w_port);

    void StateChangeCallback(CURL* handle, curl_sslstate status, void* data);
    void ClientCertSelectCallback(void* ssl);

private:
    ResourceHandleManagerSSL(CURLM* curlMultiHandle);
    ~ResourceHandleManagerSSL();
    static ResourceHandleManagerSSL* create(CURLM* curlMultiHandle);

    HashMap<String, int> m_SSLVersion;
    void setSSLVersion(const KURL&, long);
    long sslVersion(const KURL&);

    // For Singleton
    static ResourceHandleManagerSSL *m_sharedInstance;
    CURLM* m_curlMultiHandle;

    void curlRefreshTimer();

    // Allows Server Certificate CA
    HashMap<String, const char*> m_AllowServerCA;
    const char* allowsServerCA(const String& host);

    // Allows Server Host
    HashSet<String> m_AllowServerHost;

    // Client Certificates
    HashSet<ClientCertificate*> m_clientCerts;

    HashMap<String, ClientCertificate*> m_clientCertCache;
    void setClientCertCache(const char*, ClientCertificate*);
    ClientCertificate* getClientCertCache(const char*);
    void deleteClientCertCache(ClientCertificate*);
};

}

#endif
