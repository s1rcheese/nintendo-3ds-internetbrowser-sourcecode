/*
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef _WKC_HELPERS_PRIVATE_RESOURCEHANDLE_H_
#define _WKC_HELPERS_PRIVATE_RESOURCEHANDLE_H_

#include "helpers/WKCResourceHandle.h"

namespace WebCore {
class ResourceHandle;
} // namespace

namespace WKC {

class CredentialPrivate;
class AuthenticationChallengePrivate;
class ResourceLoaderPrivate;

class ResourceHandlePrivate {
public:
    ResourceHandlePrivate(WebCore::ResourceHandle*);
    ~ResourceHandlePrivate();

    WebCore::ResourceHandle* webcore() const { return m_webcore; }
    ResourceHandle& wkc() { return m_wkc; }

    void receivedProxyCredential(const CredentialPrivate&);
    void receivedCredential(const AuthenticationChallengePrivate&, const CredentialPrivate&);
    void receivedRequestToContinueWithoutCredential(const AuthenticationChallengePrivate&);

    void* certChain() const;
    long SSLVerifyPeerResult() const;
    long SSLVerifyHostResult() const;
    const char* url() const;
    const char* urlhost() const;
    bool isInclusion() const;

    ResourceLoaderPrivate* internal_client() const;

private:
    WebCore::ResourceHandle* m_webcore;
    ResourceHandle m_wkc;
};

} // namespace

#endif // _WKC_HELPERS_PRIVATE_RESOURCEHANDLE_H_
