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

#include "helpers/WKCFrameLoader.h"
#include "helpers/privates/WKCFrameLoaderPrivate.h"

#include "FrameLoader.h"
#include "DocumentLoader.h"
#include "ResourceRequest.h"
#include "helpers/privates/WKCDocumentLoaderPrivate.h"
#include "helpers/privates/WKCResourceRequestPrivate.h"

namespace WKC {
FrameLoaderPrivate::FrameLoaderPrivate(WebCore::FrameLoader* parent)
    : m_webcore(parent)
    , m_wkc(*this)
    , m_documentLoader(0)
    , m_activeDocumentLoader(0)
    , m_provisionalDocumentLoader(0)
    , m_originalRequest(0)
{
}

FrameLoaderPrivate::~FrameLoaderPrivate()
{
    delete m_documentLoader;
    delete m_activeDocumentLoader;
    delete m_provisionalDocumentLoader;
    delete m_originalRequest;
}

DocumentLoader*
FrameLoaderPrivate::documentLoader()
{
    WebCore::DocumentLoader* loader = m_webcore->documentLoader();
    if (!loader) return 0;

    if (!m_documentLoader || m_documentLoader->webcore() != loader) {
        delete m_documentLoader;
        m_documentLoader = new DocumentLoaderPrivate(loader);
    }

    return &m_documentLoader->wkc();
}

DocumentLoader*
FrameLoaderPrivate::activeDocumentLoader()
{
    WebCore::DocumentLoader* loader = m_webcore->activeDocumentLoader();
    if (!loader) return 0;

    if (!m_activeDocumentLoader || m_activeDocumentLoader->webcore() != loader) {
        delete m_activeDocumentLoader;
        m_activeDocumentLoader = new DocumentLoaderPrivate(loader);
    }

    return &m_activeDocumentLoader->wkc();
}

DocumentLoader*
FrameLoaderPrivate::provisionalDocumentLoader()
{
    WebCore::DocumentLoader* loader = m_webcore->provisionalDocumentLoader();
    if (!loader)
        return 0;

    if (!m_provisionalDocumentLoader || m_provisionalDocumentLoader->webcore() != loader) {
        delete m_provisionalDocumentLoader;
        m_provisionalDocumentLoader = new DocumentLoaderPrivate(loader);
    }

    return &m_provisionalDocumentLoader->wkc();
}

ObjectContentType
FrameLoaderPrivate::defaultObjectContentType(const KURL& url, const String& mimeType)
{
    return (ObjectContentType)WebCore::FrameLoader::defaultObjectContentType(url, mimeType);
}

const ResourceRequest&
FrameLoaderPrivate::originalRequest()
{
    const WebCore::ResourceRequest& req = m_webcore->originalRequest();

    delete m_originalRequest;
    m_originalRequest = new ResourceRequestPrivate(req);

    return m_originalRequest->wkc();
}

FrameLoadType
FrameLoaderPrivate::loadType() const
{
    return (FrameLoadType)m_webcore->loadType();
}

FrameLoader::FrameLoader(FrameLoaderPrivate& parent)
    : m_private(parent)
{
}

FrameLoader::~FrameLoader()
{
}

DocumentLoader*
FrameLoader::documentLoader()
{
    return m_private.documentLoader();
}

DocumentLoader*
FrameLoader::activeDocumentLoader()
{
    return m_private.activeDocumentLoader();
}

DocumentLoader*
FrameLoader::provisionalDocumentLoader()
{
    return m_private.provisionalDocumentLoader();
}

ObjectContentType
FrameLoader::defaultObjectContentType(const KURL& url, const String& mimeType)
{
    return FrameLoaderPrivate::defaultObjectContentType(url, mimeType);
}

const ResourceRequest&
FrameLoader::originalRequest() const
{
    return m_private.originalRequest();
}

FrameLoadType
FrameLoader::loadType() const
{
    return m_private.loadType();
}

} // namespace
