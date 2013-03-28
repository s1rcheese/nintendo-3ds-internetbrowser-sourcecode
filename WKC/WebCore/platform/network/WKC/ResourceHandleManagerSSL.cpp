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

#include "config.h"

#include "CString.h"
#include "ResourceHandle.h"
#include "ResourceHandleInternal.h"
#include "ResourceHandleManager.h"
#include "ResourceHandleManagerSSL.h"
#include "Certificate.h"

#include <errno.h>
#include <stdio.h>

#if !PLATFORM(WIN_OS)
#include <sys/param.h>
#undef MAX_PATH
#define MAX_PATH MAXPATHLEN
#endif

#include <curl/curl.h>
#include <curl/multi.h>

#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/pkcs12.h>

#include <stdarg.h>
#include <peer_openssl.h>

#ifndef TRUE
# define TRUE (1)
#endif
#ifndef FALSE
# define FALSE (0)
#endif

namespace WebCore {

// for debug
//#define RHM_DEBUG
//#undef NDEBUG
extern "C" void peerDebugPrintf(const char* in_format, ...);

#if !defined(NDEBUG) && defined(RHM_DEBUG)
  #define RHMSSL_DP(a)  peerDebugPrintf a
#else
  #define RHMSSL_DP(a)
#endif

#define RHMSSL_AES_LEN  16
static unsigned char gMagic[RHMSSL_AES_LEN];
static unsigned char gIV[RHMSSL_AES_LEN];

#define RHMSSL_BUFF_LEN  1024
static unsigned char gBuff[RHMSSL_BUFF_LEN];

////////////////////////////////////////////
//
// static tool functions
//
static char* rhmssl_strdup(const char* str)
{
    char* new_str;
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

static void* rhmssl_malloc(size_t size)
{
    return fastMalloc(size);
}

static void* rhmssl_realloc(void* ptr, size_t size)
{
    void* new_ptr;

    new_ptr = fastRealloc(ptr, size);

    return new_ptr;
}

static void rhmssl_free(void* ptr)
{
    if (ptr) fastFree(ptr);
}

static String SSLhostAndPort(const KURL& kurl)
{
    String url;

    if (kurl.hasPort())
        url = kurl.host().lower() + ":" + String::number(kurl.port());
    else
        url = kurl.host() + ":443";

    return url;
}

//copy from curl
static int asn1_output(const ASN1_UTCTIME* tm, char* buf, size_t sizeofbuf)
{
    const char* asn1_string;
    int gmt = FALSE;
    int i;
    int year, month, day, hour, minute, second;

    year = month = day = hour = minute = second = 0;
    i = tm->length;
    asn1_string = (const char*)tm->data;

    if (i < 10)
        return 1;
    if (asn1_string[i-1] == 'Z')
        gmt=TRUE;
    for (i = 0; i < 10; i++)
        if((asn1_string[i] > '9') || (asn1_string[i] < '0'))
            return 2;

    year = (asn1_string[0] - '0')*10 + (asn1_string[1] - '0');
    if (year < 50)
        year += 100;

    month = (asn1_string[2] - '0')*10 + (asn1_string[3] - '0');
    if ((month > 12) || (month < 1))
        return 3;

    day    = (asn1_string[4] - '0')*10 + (asn1_string[5] - '0');
    hour   = (asn1_string[6] - '0')*10 + (asn1_string[7] - '0');
    minute = (asn1_string[8] - '0')*10 + (asn1_string[9] - '0');

    if((asn1_string[10] >= '0') && (asn1_string[10] <= '9') &&
       (asn1_string[11] >= '0') && (asn1_string[11] <= '9'))
        second = (asn1_string[10] - '0')*10 + (asn1_string[11] - '0');

    buf[0] = 0x00;
    snprintf(buf, sizeofbuf,
             "%04d-%02d-%02d %02d:%02d:%02d %s",
             (year + 1900), month, day, hour, minute, second, (gmt?"GMT":""));

    return 0;
}

static bool parse_pkcs12(const unsigned char* pkcs12, int pkcs12_len, const char* pass, EVP_PKEY **pri, X509 **x509, STACK_OF(X509) **ca)
{
    BIO* bio = NULL;
    PKCS12* p12 = NULL;
    bool ret = false;

    if (!pkcs12 || 0 == pkcs12_len)
        goto parse_error;

    bio = BIO_new_mem_buf((void*)pkcs12, pkcs12_len);
    if (!bio)
        goto parse_error;

    p12 = d2i_PKCS12_bio(bio, NULL);
    if (!p12)
        goto parse_error;

    PKCS12_PBE_add();

    if (!PKCS12_parse(p12, (const char *)pass, pri, x509, ca))
        goto parse_error;

    ret = true;

parse_error:
    BIO_free(bio);
    PKCS12_free(p12);
    return ret;
}

static unsigned char* encrypt_key(const unsigned char* in_key, int in_keylen, bool isEncrypt, int* out_len)
{
    int resultlen, tmplen;
    unsigned char* newkey;

    newkey = (unsigned char*)rhmssl_malloc(in_keylen + RHMSSL_AES_LEN);
    if (!newkey)
        return (unsigned char*)0;
    memset(newkey, 0x00, (in_keylen + RHMSSL_AES_LEN));

    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);

    if (isEncrypt) {
        EVP_EncryptInit(&ctx, EVP_aes_128_ecb(), gMagic, gIV);
        EVP_EncryptUpdate(&ctx, newkey, &resultlen, in_key, in_keylen);
        EVP_EncryptFinal(&ctx, newkey + resultlen, &tmplen);
    }
    else {
        EVP_DecryptInit(&ctx, EVP_aes_128_ecb(), gMagic, gIV);
        EVP_DecryptUpdate(&ctx, newkey, &resultlen, in_key, in_keylen);
        EVP_DecryptFinal(&ctx, newkey + resultlen, &tmplen);
    }

    newkey[resultlen + tmplen] = 0x0;
    EVP_CIPHER_CTX_cleanup(&ctx);

    if (out_len)
        *out_len = resultlen + tmplen;

    return newkey;
}


////////////////////////////////////////////
//
//  Constructor and Destructor (private)
//
ResourceHandleManagerSSL::ResourceHandleManagerSSL(CURLM* curlMultiHandle)
    : m_curlMultiHandle(curlMultiHandle)
{
    RHMSSL_DP(("<rhmssl>ResourceHandleManagerSSL()"));

    RAND_pseudo_bytes(gMagic, RHMSSL_AES_LEN);
    RAND_pseudo_bytes(gIV, RHMSSL_AES_LEN);

    memset(gBuff, 0x00, RHMSSL_BUFF_LEN);
}

ResourceHandleManagerSSL::~ResourceHandleManagerSSL()
{
    RHMSSL_DP(("<rhmssl>~ResourceHandleManagerSSL()"));
    ClientCertificate* cert;

    m_clientCertCache.clear();

    HashSet<ClientCertificate*>::const_iterator it  = m_clientCerts.begin();
    HashSet<ClientCertificate*>::const_iterator end = m_clientCerts.end();
    for (; it != end; ++it) {
        cert = (*it);
        delete cert;
    }
    m_clientCerts.clear();

    memset(gBuff, 0x00, RHMSSL_BUFF_LEN);
    memset(gMagic, 0x00, RHMSSL_AES_LEN);
    memset(gIV, 0x00, RHMSSL_AES_LEN);
}


////////////////////////////////////////////
//
// Singleton
//
ResourceHandleManagerSSL* ResourceHandleManagerSSL::m_sharedInstance = 0;

bool ResourceHandleManagerSSL::createSharedInstance(CURLM* curlMultiHandle)
{
    if (m_sharedInstance)
        return true;

    m_sharedInstance = create(curlMultiHandle);
    if (!m_sharedInstance)
        return false;

    return true;
}

void ResourceHandleManagerSSL::deleteSharedInstance()
{
    if (m_sharedInstance)
        delete m_sharedInstance;
    m_sharedInstance = 0;
}

ResourceHandleManagerSSL* ResourceHandleManagerSSL::create(CURLM* curlMultiHandle)
{
    ResourceHandleManagerSSL* self;

    self = new ResourceHandleManagerSSL(curlMultiHandle);
    if (!self)
        return self;

    return self;
}

ResourceHandleManagerSSL* ResourceHandleManagerSSL::sharedInstance()
{
    return m_sharedInstance;
}

// Reset Variables & Force Terminate
void ResourceHandleManagerSSL::resetVariables()
{
    m_sharedInstance = 0;
    memset(gBuff, 0x00, RHMSSL_BUFF_LEN);
    memset(gMagic, 0x00, RHMSSL_AES_LEN);
    memset(gIV, 0x00, RHMSSL_AES_LEN);
}


////////////////////////////////////////////
//
// callback functions
//
static int
ssl_verify_callback(int ok, X509_STORE_CTX *ctx)
{
    RHMSSL_DP(("<rhmssl>sl_verify_callback(%d,%p)", ok, ctx));

    char *s;

    gBuff[0] = 0x00;
    s = X509_NAME_oneline(X509_get_subject_name(ctx->current_cert), (char*)gBuff, RHMSSL_BUFF_LEN);
    if (!s) {
        RHMSSL_DP(("<rhmssl>ssl_verify_callback(): no Subject"));
        return 0;
    }

    if (ok) {
        if (8 < ctx->error_depth) {
            X509_STORE_CTX_set_error(ctx,X509_V_ERR_CERT_CHAIN_TOO_LONG);
            RHMSSL_DP(("<rhmssl>ssl_verify_callback(): depth=%d error=%d %s", ctx->error_depth, ctx->error, s));
            ok = 0;
        }
        else {
            RHMSSL_DP(("<rhmssl>ssl_verify_callback(): depth=%d %s",  ctx->error_depth, s));
        }
    }

    if (ok == 0) {
        RHMSSL_DP(("<rhmssl>ssl_verify_callback(): Error string: %s", X509_verify_cert_error_string(ctx->error)));

        switch (ctx->error) {
        case X509_V_ERR_CERT_NOT_YET_VALID:
        case X509_V_ERR_CERT_HAS_EXPIRED:
            ok = 1;
            RHMSSL_DP(("  ... ignored."));
            break;
        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            ok = 1;
            RHMSSL_DP(("  ... permit self signed Cert: %s", s));
            break;
        }
    }

    return ok;
}

#if 0
static int
ssl_app_verify_callback(X509_STORE_CTX *ctx, void *data)
{
    RHMSSL_DP(("<rhmssl>ssl_app_verify_callback(%p, %p)", ctx, data));

    ResourceHandle* job = static_cast<ResourceHandle*>(data);
    ResourceHandleInternal* d = job->getInternal();

    int ok = X509_verify_cert(ctx);  // it'll call ssl_verify_callback()
    RHMSSL_DP(("<rhmssl>ssl_app_verify_callback(): X509_verify_cert() is %s", (ok)?"OK":"NG"));

    long ssl_version = -1;
    CURLcode code = curl_easy_getinfo(d->m_handle, CURLINFO_SSL_CONNECTVERSION, &ssl_version);
    if (code == CURLE_OK && ssl_version != -1) {
        ResourceHandleManagerSSL::sharedInstance()->setSSLVersion(job->request().url(), ssl_version);
    }

    d->m_response.setResourceHandle(job);

    return ok;
}
#endif

static void
ssl_state_callback(CURL* handle, curl_sslstate status, void* data)
{
    RHMSSL_DP(("<rhmssl>ssl_state_callback(%p)", data));
    ResourceHandleManagerSSL::sharedInstance()->StateChangeCallback(handle, status, data);
}

static void
ssl_cert_request_callback(const SSL *ssl, int type, int val)
{
    if (type != SSL_CB_CONNECT_LOOP || val != 1)
        return;

    RHMSSL_DP(("<rhmssl>ssl_cert_request_callback(%p)", ssl));
    ResourceHandleManagerSSL::sharedInstance()->ClientCertSelectCallback((void*)ssl);
}

static CURLcode
sslctx_callback(CURL* handle, void* sslctx, void* data)
{
    RHMSSL_DP(("<rhmssl>sslctx_callback(%p)", data));

    SSL_CTX* sslCtx = (SSL_CTX *)sslctx;

    ResourceHandle* job = static_cast<ResourceHandle*>(data);
    ResourceHandleInternal* d = job->getInternal();
    if (d && d->m_cancelled) {
        return CURLE_OK;
    }
    KURL kurl = job->request().url();

    SSL_CTX_set_info_callback(sslCtx, ssl_cert_request_callback);
    SSL_CTX_set_app_data(sslCtx, job);
    if (ResourceHandleManagerSSL::sharedInstance()->allowsServerHost(SSLhostAndPort(kurl)))
        SSL_CTX_set_verify(sslCtx, SSL_VERIFY_NONE, ssl_verify_callback);
    else
        SSL_CTX_set_verify(sslCtx, SSL_VERIFY_PEER, ssl_verify_callback);

    //SSL_CTX_set_cert_verify_callback(sslCtx, ssl_app_verify_callback, data);

    return CURLE_OK;
}

////////////////////////////////////////////
//
// Public functions
//
void* ResourceHandleManagerSSL::SSLRegisterClientCert(const unsigned char* pkcs12, int pkcs12_len, const unsigned char* pass, int pass_len)
{
    ClientCertificate* clientCert;

    clientCert = ClientCertificate::create(pkcs12, pkcs12_len, pass, pass_len);
    if (clientCert)
        m_clientCerts.add(clientCert);

    return (void*)clientCert;
}

int ResourceHandleManagerSSL::SSLUnregisterClientCert(void* certid)
{
    ClientCertificate* cert = (ClientCertificate*)certid;
    if (!cert) return -1;

    HashSet<ClientCertificate*>::const_iterator it  = m_clientCerts.begin();
    HashSet<ClientCertificate*>::const_iterator end = m_clientCerts.end();
    for (; it != end; ++it) {
        if ((*it) == cert) {
            m_clientCerts.remove(cert);
            deleteClientCertCache(cert);
            delete cert;
            return 0;
        }
    }
    return -1;
}

void ResourceHandleManagerSSL::initializeHandleSSL(ResourceHandle* job)
{
    const char *ca;

    // FIXME: Enable SSL verification when we have a way of shipping certs
    // and/or reporting SSL errors to the user.
    ResourceHandleInternal* d = job->getInternal();
    KURL kurl = job->request().url();

    ca = allowsServerCA(SSLhostAndPort(kurl));
    if (ca) {
        curl_easy_setopt(d->m_handle, CURLOPT_CADATA, ca);
    }
    else if (wkcOsslCertfIsRegistPeer()) {
        curl_easy_setopt(d->m_handle, CURLOPT_CAINFO, WKCOSSL_CERT_FILE);
    }
    if (wkcOsslCRLIsRegistPeer())
        curl_easy_setopt(d->m_handle, CURLOPT_CRLFILE, WKCOSSL_CRL_FILE);

    curl_easy_setopt(d->m_handle, CURLOPT_RANDOM_FILE, WKCOSSL_RANDFILE);

    curl_easy_setopt(d->m_handle, CURLOPT_SSL_CTX_FUNCTION, sslctx_callback);
    curl_easy_setopt(d->m_handle, CURLOPT_SSL_CTX_DATA, job);
    curl_easy_setopt(d->m_handle, CURLOPT_SSL_STATE_FUNCTION, ssl_state_callback);
    curl_easy_setopt(d->m_handle, CURLOPT_SSL_STATE_DATA, job);

    curl_easy_setopt(d->m_handle, CURLOPT_SSLVERSION, sslVersion(kurl));
    curl_easy_setopt(d->m_handle, CURLOPT_CERTINFO, 1L);

    if (allowsServerHost(SSLhostAndPort(kurl))) {
        curl_easy_setopt(d->m_handle, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(d->m_handle, CURLOPT_SSL_VERIFYPEER, 0);
    }
    else {
        curl_easy_setopt(d->m_handle, CURLOPT_SSL_VERIFYHOST, 2);
        curl_easy_setopt(d->m_handle, CURLOPT_SSL_VERIFYPEER, 1);
    }

    d->m_SSLVerifyPeerResult = 0;
    d->m_SSLVerifyHostResult = 0;
    d->m_certChain = 0;
    d->m_response.setURL(KURL(ParsedURLString, ""));
}

void ResourceHandleManagerSSL::removeRunningJobSSL(ResourceHandle* job)
{
    ResourceHandleInternal* d = job->getInternal();

    // block callback
    curl_easy_setopt(d->m_handle, CURLOPT_SSL_CTX_FUNCTION, NULL);
    curl_easy_setopt(d->m_handle, CURLOPT_SSL_CTX_DATA, 0);
    curl_easy_setopt(d->m_handle, CURLOPT_SSL_STATE_FUNCTION, NULL);
    curl_easy_setopt(d->m_handle, CURLOPT_SSL_STATE_DATA, 0);
}

void ResourceHandleManagerSSL::setAllowServerCA(const char *host_w_port, const char *ca)
{
    if (!host_w_port || !ca)
        return;

    RHMSSL_DP(("<rhmssl>setAllowServerCA(%s, %p)", host_w_port, ca));

    String host = host_w_port;

    if (!strchr(host_w_port, ':'))
        host += ":443";

    m_AllowServerCA.add(host.lower(), rhmssl_strdup(ca));
}

void ResourceHandleManagerSSL::removeAllowServerCA(const char *host_w_port)
{
    if (!host_w_port)
        return;

    RHMSSL_DP(("<rhmssl>removeAllowServerCA(%s)", host_w_port));

    String host = host_w_port;

    if (!strchr(host_w_port, ':'))
        host += ":443";

    m_AllowServerCA.remove(host.lower());
}

void ResourceHandleManagerSSL::setAllowServerHost(const char *host_w_port)
{
    if (!host_w_port)
        return;

    RHMSSL_DP(("<rhmssl>setAllowServerHost(%s)", host_w_port));

    String host = host_w_port;

    if (!strchr(host_w_port, ':'))
        host += ":443";

    m_AllowServerHost.add(host.lower());
}

//
// callback functions
//
void ResourceHandleManagerSSL::StateChangeCallback(CURL* handle, curl_sslstate status, void* data)
{
    CURLcode err;
    long value;
    ResourceHandle* job = NULL;
    ResourceHandleInternal* d = NULL;

    job = static_cast<ResourceHandle*>(data);
    if (job)
        d = job->getInternal();
    if (!d || d->m_cancelled) {
        return;
    }

    d->m_response.setResourceHandle(job);

    switch (status) {
    case CURLSSL_STATE_NONE:
        break;
    case CURLSSL_STATE_INITIALIZE:
        const char* hdr;
        err = curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &hdr);
        d->m_response.setURL(KURL(ParsedURLString, hdr));
        break;
    case CURLSSL_STATE_HANDSHAKED:
    case CURLSSL_STATE_PEERFAIL:
    case CURLSSL_STATE_VERIFYFAIL:
    case CURLSSL_STATE_ERROR:
        if (!d->m_certChain) {
            struct curl_certinfo *ci = NULL;
            err = curl_easy_getinfo(d->m_handle, CURLINFO_CERTINFO, &ci);
            if(err == CURLE_OK && ci) {
                d->m_certChain = (void*)ci;
            }
        }
        if (0 == d->m_SSLVerifyPeerResult) {
            value = -1;
            err = curl_easy_getinfo(d->m_handle, CURLINFO_SSL_VERIFYPEERRESULT, &value);
            if(err == CURLE_OK && value != -1) {
                d->m_SSLVerifyPeerResult = value;
            }
        }
        if (0 == d->m_SSLVerifyHostResult) {
            value = -1;
            err = curl_easy_getinfo(d->m_handle, CURLINFO_SSL_VERIFYHOSTRESULT, &value);
            if(err == CURLE_OK && value != -1) {
                d->m_SSLVerifyHostResult = value;
            }
        }
        {
            value = -1;
            err = curl_easy_getinfo(d->m_handle, CURLINFO_SSL_CONNECTVERSION, &value);
            if (err == CURLE_OK && value != -1) {
                setSSLVersion(job->request().url(), value);
            }
        }
        break;
    case CURLSSL_STATE_HANDSHAKING:
    case CURLSSL_STATE_VERIFYING:
    case CURLSSL_STATE_FINISHED:
    default:
        break;
    }

    if (!d->m_cancelled && d->client()) {
        d->client()->notifySSLStateChange(job, (int)(status));
        if (!d->m_isSynchronous) {
            curlRefreshTimer();
        }
    }
}

void ResourceHandleManagerSSL::ClientCertSelectCallback(void *data)
{
    SSL* ssl = (SSL*)data;
    if (!ssl) return;

    ResourceHandle* job = NULL;
    ResourceHandleInternal* d = NULL;

    job = (ResourceHandle*)SSL_CTX_get_app_data(ssl->ctx);
    if (job)
        d = job->getInternal();
    if (!d || d->m_cancelled) {
        return;
    }

    ClientCertificate*  cert = NULL;
    ClientCertificate** certInfo = NULL;
    int certInfoIdx = -1;
    int certInfoNum = 0;
    EVP_PKEY *pri = NULL;
    STACK_OF(X509) *ca = NULL;
    X509 *x509 = NULL;
    unsigned char* orgpass = NULL;
    int orgpass_len = 0;
    X509 *server = NULL;
    char* requester = NULL;

    // should verify success!
    if (0 != SSL_get_verify_result(ssl))
        return;

    switch (ssl->version) {
    case SSL2_VERSION:
        if (!ssl->s2) return;
        break;
    case TLS1_VERSION:
    case SSL3_VERSION:
        if (!ssl->s3) return;
        if (ssl->s3->tmp.message_type != SSL3_MT_CERTIFICATE_REQUEST) return;
        break;
    case DTLS1_VERSION:
        if (!ssl->d1) return;
        break;
    default:
        break;
    }

    memset(gBuff, 0x00, RHMSSL_BUFF_LEN);
    requester = (char*)gBuff;

    server = SSL_get_peer_certificate(ssl);
    if (server) {
        int total_len = 0;
        strncat(requester, "Subject: ", (RHMSSL_BUFF_LEN - 1));  total_len += 9;
        total_len += X509_NAME_get_text_by_NID(X509_get_subject_name(server), NID_organizationName, requester + total_len, (RHMSSL_BUFF_LEN - 1 - total_len));
        strncat(requester, "\r\nIssuer: ", (RHMSSL_BUFF_LEN - 1 - total_len));  total_len += 10;
        total_len += X509_NAME_get_text_by_NID(X509_get_issuer_name(server), NID_organizationName, requester + total_len, (RHMSSL_BUFF_LEN - 1 - total_len));
    }

    cert = getClientCertCache(requester);
    if (!cert) {
        switch (ssl->version) {
        case SSL2_VERSION:
            RHMSSL_DP(("<rhmssl>ClientCertSelectCallback() for SSLv2"));
            RHMSSL_DP(("     Not Implimented Yet."));
            break;

        case TLS1_VERSION:
        case SSL3_VERSION:
            if (!ssl->s3) return;
            if (ssl->s3->tmp.message_type != SSL3_MT_CERTIFICATE_REQUEST) return;
            RHMSSL_DP(("<rhmssl>ClientCertSelectCallback() for SSLv3/TLS1"));

            while (sk_X509_NAME_num(ssl->s3->tmp.ca_names)) {
                X509_NAME* ca_name = sk_X509_NAME_pop(ssl->s3->tmp.ca_names);
                char *s;
                s = X509_NAME_oneline(ca_name, NULL, 0);

                HashSet<ClientCertificate*>::const_iterator it  = m_clientCerts.begin();
                HashSet<ClientCertificate*>::const_iterator end = m_clientCerts.end();
                for (; it != end; ++it) {
                    if ((*it)->sameIssuer(s)) {
                        certInfo = (ClientCertificate**)rhmssl_realloc(certInfo, sizeof(ClientCertificate*)*(certInfoNum+1));
                        certInfo[certInfoNum] = (*it);
                        certInfoNum++;
                    }
                }
            }

            if (!d->m_cancelled && d->client())
                certInfoIdx = d->client()->requestSSLClientCertSelect(job, (server)?(const char*)requester:"unknown", (void*)certInfo, certInfoNum);

            if (!d->m_isSynchronous) {
                curlRefreshTimer();
            }
            RHMSSL_DP(("<rhmssl> Selected Client Cert index=%d", certInfoIdx));
            if (certInfoIdx < 0) {
                rhmssl_free(certInfo);
                return;
            }
            cert = certInfo[certInfoIdx];
            rhmssl_free(certInfo);
            setClientCertCache(requester, cert);
            break;

        case DTLS1_VERSION:
            if (!ssl->d1) return;
            RHMSSL_DP(("<rhmssl>ClientCertSelectCallback() for DTLSv1"));
            RHMSSL_DP(("     Not Implimented Yet."));
            break;
        default:
            break;
        }
    }

    if (!cert) return;

    while (1) {
        orgpass = encrypt_key((const unsigned char *)cert->pass(), cert->passlen(), false, &orgpass_len);
        if (!orgpass)
            break;

        if (!parse_pkcs12((const unsigned char *)cert->pkcs12(), cert->pkcs12len(), (const char *)orgpass, &pri, &x509, &ca))
            break;

        if (SSL_use_certificate(ssl, x509) != 1)
            break;
        if (SSL_use_PrivateKey(ssl, pri) != 1)
            break;
        if (!SSL_check_private_key(ssl))
            break;
        if (ca && sk_X509_num(ca)) {
            for (int i = 0; i < sk_X509_num(ca); i++) {
                if (!SSL_CTX_add_extra_chain_cert(ssl->ctx, sk_X509_value(ca, i)))
                    break;
                if (!SSL_CTX_add_client_CA(ssl->ctx, sk_X509_value(ca, i)))
                    break;
            }
        }
        break;
    }
    if (orgpass) delete [] orgpass;
    if (pri) EVP_PKEY_free(pri);
    if (x509) X509_free(x509);

    return;
}

////////////////////////////////////////////
//
// private functions
//
void ResourceHandleManagerSSL::setSSLVersion(const KURL &kurl, long version)
{
    m_SSLVersion.set(SSLhostAndPort(kurl), version);
}

long ResourceHandleManagerSSL::sslVersion(const KURL& kurl)
{
    return m_SSLVersion.get(SSLhostAndPort(kurl));
}

void ResourceHandleManagerSSL::curlRefreshTimer()
{
    curl_multi_refresh_timer(m_curlMultiHandle);
}

const char* ResourceHandleManagerSSL::allowsServerCA(const String& host)
{
    return m_AllowServerCA.get(host.lower());
}

bool ResourceHandleManagerSSL::allowsServerHost(const String& host)
{
    return m_AllowServerHost.contains(host.lower());
}

void ResourceHandleManagerSSL::setClientCertCache(const char* requester, ClientCertificate* cert)
{
    m_clientCertCache.set(String(requester), cert);
}

ClientCertificate* ResourceHandleManagerSSL::getClientCertCache(const char *requester)
{
    return m_clientCertCache.get(String(requester));
}

static int
organizationName(const String& name, char *buf, int buflen)
{
    Vector<String> list;

    name.split("/", list);
    for (int i = 0; i < list.size(); i++) {
        Vector<String> items;
        list[i].split("=", items);
        if (items.size()==0) break;
        if (items.size()>2) {
            for (int j=2; j<items.size(); j++) {
                items[1] += "="+items[j];
            }
        }
        if (items[0] == "O") {
            strncat(buf, items[1].utf8().data(), buflen);
            return items[1].length();
        }
    }
    return 0;
}

void ResourceHandleManagerSSL::deleteClientCertCache(ClientCertificate* cert)
{
    char *requester;
    int buflen = RHMSSL_BUFF_LEN - 1;
    int len;

    memset(gBuff, 0x00, RHMSSL_BUFF_LEN);
    requester = (char*)gBuff;
    strncat(requester, "Subject: ", buflen);  requester += 9;  buflen -= 9;
    len = organizationName(cert->issuer(), requester, buflen);  requester += len;  buflen -= len;
    strncat(requester, "\r\nIssuer: ", buflen);  requester += 10;  buflen -= 10;
    len = organizationName(cert->subject(), requester, buflen); requester += len;  buflen -= len;

    m_clientCertCache.remove(String((const char*)gBuff));
}

////////////////////////////////////////////
//
// Client Certificate Class
//
ClientCertificate::ClientCertificate()
    : m_pkcs12(0)
    , m_pkcs12Len(0)
    , m_pass(0)
    , m_passLen(0)
{
}

ClientCertificate::~ClientCertificate()
{
    if(m_pkcs12) {
        memset(m_pkcs12, 0x00, m_pkcs12Len);
        rhmssl_free(m_pkcs12);
        m_pkcs12 = NULL;
        m_pkcs12Len = 0;
    }
    if (m_pass) {
        memset(m_pass, 0x00, m_passLen);
        rhmssl_free(m_pass);
        m_pass = NULL;
        m_passLen = 0;
    }
}

ClientCertificate* ClientCertificate::create(const unsigned char* pkcs12, int pkcs12_len, const unsigned char* pass, int pass_len)
{
    ClientCertificate* self = NULL;

    int newpass_len;
    String issuerSubject, issuer, subject;

    EVP_PKEY *pri = NULL;
    STACK_OF(X509) *ca = NULL;
    X509 *x509 = NULL;

    char *s=NULL;
    int buflen;
    ASN1_TIME *certdate = 0;
    ASN1_INTEGER *num = 0;


    self = new ClientCertificate();
    if (!self)
        return (ClientCertificate*)0;

    self->m_pkcs12 = rhmssl_malloc(pkcs12_len);
    if (!self->m_pkcs12)
        goto regist_error;
    memcpy(self->m_pkcs12, pkcs12, pkcs12_len);
    self->m_pkcs12Len = pkcs12_len;

    self->m_pass = encrypt_key(pass, pass_len, true, &newpass_len);
    if (!self->m_pass)
        goto regist_error;
    self->m_passLen = newpass_len;

    if (!parse_pkcs12(pkcs12, pkcs12_len, (const char *)pass, &pri, &x509, &ca))
        goto regist_error;

    gBuff[0] = 0x00;
    self->m_Issuer = X509_NAME_oneline(X509_get_issuer_name(x509), (char*)gBuff, RHMSSL_BUFF_LEN);
    gBuff[0] = 0x00;
    self->m_Subject = X509_NAME_oneline(X509_get_subject_name(x509), (char*)gBuff, RHMSSL_BUFF_LEN);

    num = X509_get_serialNumber(x509);
    if (num->length <= 4) {
        self->m_serialNumber = String::number(ASN1_INTEGER_get(num));
    }
    else {
        s = (char*)gBuff;
        buflen = RHMSSL_BUFF_LEN;
        s[0] = 0x00;
        if((num->length*3) < (buflen - 1)) {
            for (int j = 0; j < num->length; j++) {
                snprintf(s, buflen, "%02x%c", num->data[j], ((j+1 == num->length)?0x0:':'));
                s += 3;
                buflen -= 3;
            }
        }
        self->m_serialNumber = (char*)gBuff;
    }

    certdate = X509_get_notBefore(x509);
    asn1_output(certdate, (char*)gBuff, RHMSSL_BUFF_LEN);
    self->m_NotBefore = (char*)gBuff;

    certdate = X509_get_notAfter(x509);
    asn1_output(certdate, (char*)gBuff, RHMSSL_BUFF_LEN);
    self->m_NotAfter = (char*)gBuff;

    X509_free(x509);
    EVP_PKEY_free(pri);
    if (ca) sk_X509_free(ca);

    return self;

regist_error:
    if (x509) X509_free(x509);
    if (pri) EVP_PKEY_free(pri);
    if (ca) sk_X509_free(ca);
    if (self) delete self;

    return (ClientCertificate*)0;
}

} // namespace WebCore
