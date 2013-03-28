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

#ifndef _WKC_HELPERS_PRIVATE_DOCUMENT_H_
#define _WKC_HELPERS_PRIVATE_DOCUMENT_H_

#include "helpers/WKCDocument.h"

namespace WebCore {
class Document;
} // namespace

namespace WKC {

class DocLoader;
class Frame;
class Node;
class RenderView;
class DocLoaderPrivate;
class FramePrivate;
class NodePrivate;
class RenderViewPrivate;

class DocumentPrivate {
public:
    DocumentPrivate(WebCore::Document*);
    ~DocumentPrivate();

    WebCore::Document* webcore() const { return m_webcore; }
    Document& wkc() { return m_wkc; }

    bool isImageDocument() const;
    void updateLayoutIgnorePendingStylesheets();
    KURL completeURL(const String&) const;

    Node* focusedNode();
    RenderView* renderView();
    DocLoader* docLoader();

    Frame* frame();
    Node* firstChild();

private:
    WebCore::Document* m_webcore;
    Document m_wkc;

    DocLoaderPrivate* m_docLoader;
    NodePrivate* m_focusedNode;
    RenderViewPrivate* m_renderView;
    FramePrivate* m_frame;
    NodePrivate* m_firstChild;
};
} // namespace

#endif // _WKC_HELPERS_PRIVATE_DOCUMENT_H_

