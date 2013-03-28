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

#include "config.h"

#include "helpers/WKCResourceError.h"
#include "helpers/privates/WKCResourceErrorPrivate.h"

#include "ResourceError.h"
#include "ResourceHandleInternal.h"
#include "PlatformString.h"
#include "helpers/WKCString.h"
#include "helpers/privates/WKCResourceHandlePrivate.h"

namespace WKC {

ResourceErrorPrivateBase::ResourceErrorPrivateBase()
    : m_failingURL()
    , m_domain()
{
}

ResourceErrorPrivateBase::~ResourceErrorPrivateBase()
{
}

ResourceErrorPrivate::ResourceErrorPrivate(const WebCore::ResourceError& parent)
    : ResourceErrorPrivateBase()
    , m_webcore(parent)
    , m_wkc(this)
{
}

ResourceErrorPrivate::~ResourceErrorPrivate()
{
}

const WebCore::ResourceError&
ResourceErrorPrivate::webcore() const
{
    return m_webcore;
}

const ResourceError&
ResourceErrorPrivate::wkc() const
{
    return m_wkc;
}

ResourceErrorPrivateToCore::ResourceErrorPrivateToCore(const ResourceError& error, const String& domain, int errorCode, const String& failingURL, const String& localizedDescription, ResourceHandle* resourceHandle)
    : ResourceErrorPrivateBase()
    , m_instance(new WebCore::ResourceError(domain, errorCode, failingURL, localizedDescription, resourceHandle ? resourceHandle->priv().webcore() : 0))
    , m_webcore(*m_instance)
    , m_wkc(error)
{
}

ResourceErrorPrivateToCore::~ResourceErrorPrivateToCore()
{
    delete m_instance;
}

const WebCore::ResourceError&
ResourceErrorPrivateToCore::webcore() const
{
    return m_webcore;
}

const ResourceError&
ResourceErrorPrivateToCore::wkc() const
{
    return m_wkc;
}

bool
ResourceErrorPrivateBase::isNull() const
{
    return webcore().isNull();
}

int
ResourceErrorPrivateBase::errorCode() const
{
    return webcore().errorCode();
}

bool
ResourceErrorPrivateBase::isCancellation() const
{
    return webcore().isCancellation();
}

const String&
ResourceErrorPrivateBase::failingURL()
{
    m_failingURL = webcore().failingURL();
    return m_failingURL;
}

const String&
ResourceErrorPrivateBase::domain()
{
    m_domain = webcore().domain();
    return m_domain;
}

const String&
ResourceErrorPrivateBase::localizedDescription()
{
    m_localizedDescription = webcore().localizedDescription();
    return m_localizedDescription;
}

bool
ResourceErrorPrivateBase::isInclusion() const
{
    if (webcore().m_resourceHandle && webcore().m_resourceHandle->getInternal())
        return webcore().m_resourceHandle->getInternal()->m_isInclusion;
    else
        return false;
}

ResourceError::ResourceError(ResourceErrorPrivate* parent)
    : m_private(parent)
    , m_owned(false)
{
}

ResourceError::ResourceError(const String& domain, int errorCode, const String& failingURL, const String& localizedDescription, ResourceHandle* resourceHandle)
{
    ResourceErrorPrivateToCore* item = new ResourceErrorPrivateToCore(*this, domain, errorCode, failingURL, localizedDescription, resourceHandle);
    m_private = reinterpret_cast<ResourceErrorPrivate*>(item);
    m_owned = true;
}

ResourceError::~ResourceError()
{
    if (m_owned)
        delete m_private;
}


bool
ResourceError::isNull() const
{
    return m_private->isNull();
}

int
ResourceError::errorCode() const
{
    return m_private->errorCode();
}

bool
ResourceError::isCancellation() const
{
    return m_private->isCancellation();
}

const String&
ResourceError::failingURL() const
{
    return m_private->failingURL();
}

const String&
ResourceError::domain() const
{
    return m_private->domain();
}

const String&
ResourceError::localizedDescription() const
{
    return m_private->localizedDescription();
}

bool
ResourceError::isInclusion() const
{
    return m_private->isInclusion();
}

} // namespace
