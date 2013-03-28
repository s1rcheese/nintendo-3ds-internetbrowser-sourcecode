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

#include "helpers/WKCNode.h"
#include "helpers/privates/WKCNodePrivate.h"

#include "Node.h"
#include "HTMLNames.h"
#include "Element.h"

#include "helpers/WKCString.h"

#include "helpers/privates/WKCDocumentPrivate.h"
#include "helpers/privates/WKCElementPrivate.h"
#include "helpers/privates/WKCHTMLInputElementPrivate.h"
#include "helpers/privates/WKCHTMLTextAreaElementPrivate.h"
#include "helpers/privates/WKCHTMLAreaElementPrivate.h"
#include "helpers/privates/WKCHTMLIFrameElementPrivate.h"
#include "helpers/privates/WKCHTMLFrameElementPrivate.h"
#include "helpers/privates/WKCRenderObjectPrivate.h"

namespace WKC {

NodePrivate*
NodePrivate::create(WebCore::Node* parent)
{
    NodePrivate* node = 0;
    if (!parent)
        return 0;

    if (parent->hasTagName(WebCore::HTMLNames::inputTag)) {
        node = new HTMLInputElementPrivate(reinterpret_cast<WebCore::HTMLInputElement*>(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::textareaTag)) {
        node = new HTMLTextAreaElementPrivate(reinterpret_cast<WebCore::HTMLTextAreaElement*>(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::areaTag)) {
        node = new HTMLAreaElementPrivate(reinterpret_cast<WebCore::HTMLAreaElement*>(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::iframeTag)) {
        node = new HTMLIFrameElementPrivate(reinterpret_cast<WebCore::HTMLIFrameElement*>(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::frameTag)) {
        node = new HTMLFrameElementPrivate(reinterpret_cast<WebCore::HTMLFrameElement*>(parent));
    } else {
        node = new NodePrivate(parent);
    }
    return node;
}

NodePrivate*
NodePrivate::create(const WebCore::Node* parent)
{
    NodePrivate* node = 0;
    if (!parent)
        return 0;

    if (parent->hasTagName(WebCore::HTMLNames::inputTag)) {
        node = new HTMLInputElementPrivate((WebCore::HTMLInputElement*)parent);
    } else if (parent->hasTagName(WebCore::HTMLNames::textareaTag)) {
        node = new HTMLTextAreaElementPrivate((WebCore::HTMLTextAreaElement*)parent);
    } else if (parent->hasTagName(WebCore::HTMLNames::areaTag)) {
        node = new HTMLAreaElementPrivate((WebCore::HTMLAreaElement*)(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::iframeTag)) {
        node = new HTMLIFrameElementPrivate((WebCore::HTMLIFrameElement*)(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::frameTag)) {
        node = new HTMLFrameElementPrivate((WebCore::HTMLFrameElement*)(parent));
    } else {
        node = new NodePrivate(parent);
    }
    return node;
}

NodePrivate*
NodePrivate::create(const NodePrivate& wparent)
{
    NodePrivate* node = 0;
    WebCore::Node* parent = wparent.webcore();

    if (parent->hasTagName(WebCore::HTMLNames::inputTag)) {
        node = new HTMLInputElementPrivate((WebCore::HTMLInputElement*)parent);
    } else if (parent->hasTagName(WebCore::HTMLNames::textareaTag)) {
        node = new HTMLTextAreaElementPrivate((WebCore::HTMLTextAreaElement*)parent);
    } else if (parent->hasTagName(WebCore::HTMLNames::areaTag)) {
        node = new HTMLAreaElementPrivate((WebCore::HTMLAreaElement*)parent);
    } else if (parent->hasTagName(WebCore::HTMLNames::iframeTag)) {
        node = new HTMLIFrameElementPrivate((WebCore::HTMLIFrameElement*)(parent));
    } else if (parent->hasTagName(WebCore::HTMLNames::frameTag)) {
        node = new HTMLFrameElementPrivate((WebCore::HTMLFrameElement*)(parent));
    } else {
        node = new NodePrivate(parent);
    }
    return node;
}

NodePrivate::NodePrivate(WebCore::Node* parent)
    : m_webcore(parent)
    , m_wkc(*this)
    , m_document(0)
    , m_parentElement(0)
    , m_renderer(0)
    , m_parent(0)
    , m_parentNode(0)
    , m_firstChild(0)
    , m_traverseNextNode(0)
    , m_traverseNextSibling(0)
    , m_shadowAncestorNode(0)
{
}

NodePrivate::NodePrivate(const WebCore::Node* parent)
    : m_webcore(const_cast<WebCore::Node*>(parent))
    , m_wkc(*this)
    , m_document(0)
    , m_parentElement(0)
    , m_renderer(0)
    , m_parent(0)
    , m_parentNode(0)
    , m_firstChild(0)
    , m_traverseNextNode(0)
    , m_traverseNextSibling(0)
    , m_shadowAncestorNode(0)
{
}

NodePrivate::~NodePrivate()
{
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);
    delete m_renderer;

    if (m_document)
        delete m_document;
    if (m_parentElement)
        delete m_parentElement;
    if (m_parent)
        delete m_parent;
    if (m_parentNode)
        delete m_parentNode;
    if (m_firstChild)
        delete m_firstChild;
    if (m_traverseNextNode)
        delete m_traverseNextNode;
    if (m_traverseNextSibling)
        delete m_traverseNextSibling;
    if (m_shadowAncestorNode)
        delete m_shadowAncestorNode;
}

String
NodePrivate::nodeName() const
{
    return m_webcore->nodeName();
}

bool
NodePrivate::hasTagName(int id) const
{
    switch (id) {
    case HTMLNames_inputTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::inputTag);
    case HTMLNames_textareaTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::textareaTag);
    case HTMLNames_selectTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::selectTag);
    case HTMLNames_frameTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::frameTag);
    case HTMLNames_iframeTag:
        return m_webcore->hasTagName(WebCore::HTMLNames::iframeTag);
    default:
        return false;
    }
}

bool
NodePrivate::isFocusable() const
{
    return m_webcore->isFocusable();
}

bool
NodePrivate::isHTMLElement() const
{
    return m_webcore->isHTMLElement();
}

bool
NodePrivate::isElementNode() const
{
    return m_webcore->isElementNode();
}

bool
NodePrivate::isFrameOwnerElement() const
{
    return m_webcore->isFrameOwnerElement();
}

bool
NodePrivate::inDocument() const
{
    return m_webcore->inDocument();
}

bool
NodePrivate::hasEventListeners(int id)
{
    switch (id ){
    case eventNames_clickEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().clickEvent);
    case eventNames_mousedownEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().mousedownEvent);
    case eventNames_mousemoveEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().mousemoveEvent);
    case eventNames_dragEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().dragEvent);
    case eventNames_dragstartEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().dragstartEvent);
    case eventNames_dragendEvent:
        return m_webcore->hasEventListeners(WebCore::eventNames().dragendEvent);
    }
    return false;
}


Document*
NodePrivate::document()
{
    WebCore::Document* doc = m_webcore->document();
    if (!doc)
        return 0;
    if (!m_document || m_document->webcore()!=doc) {
        delete m_document;
        m_document = new DocumentPrivate(doc);
    }
    return &m_document->wkc();
}

RenderObject*
NodePrivate::renderer()
{
    WebCore::RenderObject* render = m_webcore->renderer();
    if (!render)
        return 0;
    if (!m_renderer || m_renderer->webcore()!=render) {
        delete m_renderer;
        m_renderer = new RenderObjectPrivate(render);

    }
    return &m_renderer->wkc();
}

Element*
NodePrivate::parentElement()
{
    WebCore::Element* elem = m_webcore->parentElement();
    if (!elem)
        return 0;
    if (!m_parentElement || m_parentElement->webcore()!=elem) {
        delete m_parentElement;
        m_parentElement = new ElementPrivate(elem);
    }
    return &m_parentElement->wkc();
}


Node*
NodePrivate::parent()
{
    if (!m_webcore)
        return 0;

    WebCore::Node* n = m_webcore->parent();
    if (!n)
        return 0;
    if (n==this->webcore())
        return &wkc();

    if (!m_parent || m_parent->webcore()!=n) {
        delete m_parent;
        m_parent = create(n);
    }
    return &m_parent->wkc();
}

Node*
NodePrivate::parentNode()
{
    if (!m_webcore)
        return 0;

    WebCore::Node* n = m_webcore->parentNode();
    if (!n)
        return 0;
    if (n==this->webcore())
        return &wkc();

    if (!m_parentNode || m_parentNode->webcore()!=n) {
        delete m_parentNode;
        m_parentNode = create(n);
    }
    return &m_parentNode->wkc();
}

Node*
NodePrivate::firstChild()
{
    if (!m_webcore)
        return 0;

    WebCore::Node* n = m_webcore->firstChild();
    if (!n)
        return 0;
    if (n==this->webcore())
        return &wkc();

    if (!m_firstChild || m_firstChild->webcore()!=n) {
        delete m_firstChild;
        m_firstChild = create(n);
    }
    return &m_firstChild->wkc();
}

Node*
NodePrivate::traverseNextNode()
{
    if (!m_webcore)
        return 0;

    WebCore::Node* n = m_webcore->traverseNextNode();
    if (!n)
        return 0;
    if (n==this->webcore())
        return &wkc();

    if (!m_traverseNextNode || m_traverseNextNode->webcore()!=n) {
        delete m_traverseNextNode;
        m_traverseNextNode = create(n);
    }
    return &m_traverseNextNode->wkc();
}

Node*
NodePrivate::traverseNextSibling()
{
    if (!m_webcore)
        return 0;

    WebCore::Node* n = m_webcore->traverseNextSibling();
    if (!n)
        return 0;
    if (n==this->webcore())
        return &wkc();

    if (!m_traverseNextSibling || m_traverseNextSibling->webcore()!=n) {
        delete m_traverseNextSibling;
        m_traverseNextSibling = create(n);
    }
    return &m_traverseNextSibling->wkc();
}

Node*
NodePrivate::shadowAncestorNode()
{
    if (!m_webcore)
        return 0;

    WebCore::Node* n = m_webcore->shadowAncestorNode();
    if (!n)
        return 0;
    if (n==this->webcore())
        return &wkc();

    if (!m_shadowAncestorNode || m_shadowAncestorNode->webcore()!=n) {
        delete m_shadowAncestorNode;
        m_shadowAncestorNode = create(n);
    }
    return &m_shadowAncestorNode->wkc();
}


Node::Node(NodePrivate& parent)
    : m_ownedPrivate(0)
    , m_private(parent)
    , m_needsRef(false)
{
}

Node::Node(Node* parent, bool needsRef)
    : m_ownedPrivate(NodePrivate::create(parent->priv().webcore()))
    , m_private(*m_ownedPrivate)
    , m_needsRef(needsRef)
{
    if (needsRef)
        m_private.webcore()->ref();
}

Node::~Node()
{
    if (m_needsRef)
        m_private.webcore()->deref();
    if (m_ownedPrivate)
        delete m_ownedPrivate;
}

bool
Node::compare(const Node* other) const
{
    if (this==other)
        return true;
    if (!this || !other)
        return false;
    if (m_private.webcore() == other->m_private.webcore())
        return true;
    return false;
}

bool
Node::hasTagName(int id) const
{
    return m_private.hasTagName(id);
}

bool
Node::isFocusable() const
{
    return m_private.isFocusable();
}

bool
Node::isHTMLElement() const
{
    return m_private.isHTMLElement();
}

bool
Node::isElementNode() const
{
    return m_private.isElementNode();
}

bool
Node::isFrameOwnerElement() const
{
    return m_private.isFrameOwnerElement();
}

bool
Node::inDocument() const
{
    return m_private.inDocument();
}

String
Node::nodeName() const
{
    return m_private.nodeName();
}

bool
Node::hasEventListeners(int id)
{
    return m_private.hasEventListeners(id);
}


Document*
Node::document() const
{
    return m_private.document();
}

RenderObject*
Node::renderer() const
{
    return m_private.renderer();
}

Element*
Node::parentElement() const
{
    return m_private.parentElement();
}


Node*
Node::parent() const
{
    return m_private.parent();
}

Node*
Node::parentNode() const
{
    return m_private.parentNode();
}

Node*
Node::firstChild() const
{
    return m_private.firstChild();
}

Node*
Node::shadowAncestorNode() const
{
    return m_private.shadowAncestorNode();
}

Node*
Node::traverseNextNode() const
{
    return m_private.traverseNextNode();
}

Node*
Node::traverseNextSibling() const
{
    return m_private.traverseNextSibling();
}

} // namespace
