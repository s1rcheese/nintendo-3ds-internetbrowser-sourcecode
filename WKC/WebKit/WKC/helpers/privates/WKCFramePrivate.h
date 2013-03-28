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

#ifndef _WKC_HELPERS_PRIVATE_FRAME_H_
#define _WKC_HELPERS_PRIVATE_FRAME_H_

#include "helpers/WKCFrame.h"

namespace WebCore { 
class Frame;
} // namespace

namespace WKC {

class HTMLFrameOwnerElement;
class Document;
class Page;
class FrameView;
class SelectionController;
class FrameLoader;
class FrameTree;

class HTMLFrameOwnerElementPrivate;
class DocumentPrivate;
class FrameViewPrivate;
class FrameLoaderPrivate;
class PagePrivate;
class SelectionControllerPrivate;
class FrameTreePrivate;

class FramePrivate {
public:
    FramePrivate(WebCore::Frame*);
    FramePrivate(const WebCore::Frame*);
    ~FramePrivate();

    WebCore::Frame* webcore() const { return m_webcore; }
    Frame& wkc() { return m_wkc; }

    WKC::Document* document();
    WKC::Page* page();
    WKC::FrameView* view();
    WKC::SelectionController* selection();
    WKC::FrameLoader* loader();

    FrameTree* tree();
    HTMLFrameOwnerElement* ownerElement();

private:
    WebCore::Frame* m_webcore;
    Frame m_wkc;

    DocumentPrivate* m_document;
    FrameLoaderPrivate* m_frameLoader;
    FrameViewPrivate* m_frameView;
    FrameTreePrivate* m_tree;
    PagePrivate* m_page;
    SelectionControllerPrivate* m_selection;
    HTMLFrameOwnerElementPrivate* m_ownerElement;
};

} // namespace

#endif // _WKC_HELPERS_PRIVATE_FRAME_H_

