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

#ifndef _WKC_HELPERS_PRIVATE_RESOURCERESPONSE_H_
#define _WKC_HELPERS_PRIVATE_RESOURCERESPONSE_H_

#include "helpers/WKCResourceResponse.h"
#include "helpers/WKCString.h"

namespace WebCore {
class ResourceResponse;
} // namespace

namespace WKC {
class ResourceHandlePrivate;
class KURL;
class String;

class ResourceResponsePrivateBase {
public:
    ResourceResponsePrivateBase();
    virtual ~ResourceResponsePrivateBase();

    virtual const WebCore::ResourceResponse& webcore() const = 0;
    virtual const ResourceResponse& wkc() const = 0;

    const KURL url() const;
    const String mimeType() const;

    bool isAttachment() const;
    bool isNull() const;
    int httpStatusCode() const;
    long long expectedContentLength() const;
    const String& httpStatusText();

    ResourceHandle* resourceHandle();

private:
    ResourceHandlePrivate* m_resourceHandle;
    String m_httpStatusText;
};

class ResourceResponsePrivate : public ResourceResponsePrivateBase {
public:
    ResourceResponsePrivate(const WebCore::ResourceResponse&);
    ~ResourceResponsePrivate();

    virtual const WebCore::ResourceResponse& webcore() const { return m_webcore; }
    virtual const ResourceResponse& wkc() const { return m_wkc; }

private:
    const WebCore::ResourceResponse& m_webcore;
    ResourceResponse m_wkc;
};

class ResourceResponsePrivateToCore : public ResourceResponsePrivateBase {
public:
    ResourceResponsePrivateToCore(const ResourceResponse&);
    ~ResourceResponsePrivateToCore();

    virtual const WebCore::ResourceResponse& webcore() const { return m_webcore; }
    virtual const ResourceResponse& wkc() const { return m_wkc; }

private:
    WebCore::ResourceResponse* m_instance;
    const WebCore::ResourceResponse& m_webcore;
    const ResourceResponse& m_wkc;
};


} // namespace

#endif // _WKC_HELPERS_PRIVATE_RESOURCERESPONSE_H_
