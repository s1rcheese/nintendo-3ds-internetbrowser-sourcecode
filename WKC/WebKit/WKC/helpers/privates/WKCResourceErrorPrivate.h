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

#ifndef _WKC_HELPERS_PRIVATE_RESOURCEERROR_H_
#define _WKC_HELPERS_PRIVATE_RESOURCEERROR_H_

#include "helpers/WKCResourceError.h"
#include "helpers/WKCString.h"

namespace WebCore {
class ResourceError;
} // namespace

namespace WKC {
class ResourceHandle;

class ResourceErrorPrivateBase {
public:
    ResourceErrorPrivateBase();
    virtual ~ResourceErrorPrivateBase();

    virtual const WebCore::ResourceError& webcore() const = 0;
    virtual const ResourceError& wkc() const = 0;

    bool isNull() const;
    int errorCode() const;
    bool isCancellation() const;

    const String& failingURL();
    const String& domain();
    const String& localizedDescription();

    bool isInclusion() const;

private:
    String m_failingURL;
    String m_domain;
    String m_localizedDescription;
};

class ResourceErrorPrivate : public ResourceErrorPrivateBase {
public:
    ResourceErrorPrivate(const WebCore::ResourceError&);
    ~ResourceErrorPrivate();

    virtual const WebCore::ResourceError& webcore() const;
    virtual const ResourceError& wkc() const;

private:
    const WebCore::ResourceError& m_webcore;
    ResourceError m_wkc;
};

class ResourceErrorPrivateToCore : public ResourceErrorPrivateBase {
public:
    ResourceErrorPrivateToCore(const ResourceError&, const String& domain, int errorCode, const String& failingURL, const String& localizedDescription, ResourceHandle* resourceHandle);
    ~ResourceErrorPrivateToCore();

    virtual const WebCore::ResourceError& webcore() const;
    virtual const ResourceError& wkc() const;

private:
    WebCore::ResourceError* m_instance;
    const WebCore::ResourceError& m_webcore;
    const ResourceError& m_wkc;
};

} // namespace

#endif // _WKC_HELPERS_PRIVATE_RESOURCEERROR_H_
