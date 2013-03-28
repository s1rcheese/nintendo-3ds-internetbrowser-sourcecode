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

#include "helpers/WKCDocument.h"
#include "helpers/privates/WKCDocumentPrivate.h"

#include "Document.h"
#include "helpers/privates/WKCDocLoaderPrivate.h"
#include "helpers/privates/WKCNodePrivate.h"
#include "helpers/privates/WKCRenderViewPrivate.h"
#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/WKCKURL.h"

namespace WKC {

DocumentPrivate::DocumentPrivate(WebCore::Document* parent)
    : m_webcore(parent)
    , m_wkc(*this)
    , m_docLoader(0)
    , m_focusedNode(0)
    , m_renderView(0)
    , m_frame(0)
    , m_firstChild(0)
{
}

DocumentPrivate::~DocumentPrivate()
{
    delete m_firstChild;
    delete m_frame;
    delete m_renderView;
    delete m_focusedNode;
    delete m_docLoader;
}

bool
DocumentPrivate::isImageDocument() const
{
    return m_webcore->isImageDocument();
}

void
DocumentPrivate::updateLayoutIgnorePendingStylesheets()
{
    m_webcore->updateLayoutIgnorePendingStylesheets();
}

KURL
DocumentPrivate::completeURL(const String& url) const
{
    return m_webcore->completeURL(url);    
}

Node*
DocumentPrivate::focusedNode()
{
    WebCore::Node* node = m_webcore->focusedNode();
    if (!node)
        return 0;
    if (!m_focusedNode || m_focusedNode->webcore()!=node) {
        delete m_focusedNode;
        m_focusedNode = NodePrivate::create(node);
    }
    return &m_focusedNode->wkc();
}



DocLoader*
DocumentPrivate::docLoader()
{
    WebCore::DocLoader* loader = m_webcore->docLoader();
    if (!loader)
        return 0;
    if (!m_docLoader || m_docLoader->webcore()!=loader) {
        delete m_docLoader;
        m_docLoader = new DocLoaderPrivate(loader);
    }
    return &m_docLoader->wkc();
}

RenderView*
DocumentPrivate::renderView() 
{
    WebCore::RenderView* view = m_webcore->renderView();
    if (!view)
        return 0;
    if (!m_renderView || m_renderView->webcore()!=view) {
        delete m_renderView;
        m_renderView = new RenderViewPrivate(view);
    }
    return &m_renderView->wkc();
}

Frame*
DocumentPrivate::frame()
{
    WebCore::Frame* frame = m_webcore->frame();
    if (!frame)
        return 0;
    if (!m_frame || m_frame->webcore()!=frame) {
        delete m_frame;
        m_frame = new FramePrivate(frame);
    }
    return &m_frame->wkc();
}

Node*
DocumentPrivate::firstChild()
{
    WebCore::Node* node = m_webcore->firstChild();
    if (!node)
        return 0;
    if (!m_firstChild || m_firstChild->webcore()!=node) {
        delete m_firstChild;
        m_firstChild = NodePrivate::create(node);
    }
    return &m_firstChild->wkc();
}

Document::Document(DocumentPrivate& parent)
    : m_private(parent)
{
}

Document::~Document()
{
}

Node*
Document::focusedNode() const
{
    return m_private.focusedNode();
}


Frame*
Document::frame() const
{
    return m_private.frame();
}

Node*
Document::firstChild() const
{
    return m_private.firstChild();
}

DocLoader*
Document::docLoader() const
{
    return m_private.docLoader();
}


bool
Document::isImageDocument() const
{
    return m_private.isImageDocument();
}

RenderView*
Document::renderView() const
{
    return m_private.renderView();
}

void
Document::updateLayoutIgnorePendingStylesheets()
{
    m_private.updateLayoutIgnorePendingStylesheets();
}

KURL
Document::completeURL(const String& url) const
{
    return m_private.completeURL(url);
}

} // namespace
