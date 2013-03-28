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

#include "helpers/WKCHTMLAreaElement.h"
#include "helpers/privates/WKCHTMLAreaElementPrivate.h"

#include "IntRect.h"
#include "HTMLAreaElement.h"

#include "helpers/WKCRenderObject.h"
#include "helpers/privates/WKCRenderObjectPrivate.h"
#include "helpers/privates/WKCHTMLImageElementPrivate.h"


namespace WKC {

HTMLAreaElementPrivate::HTMLAreaElementPrivate(WebCore::HTMLAreaElement* parent)
    : HTMLElementPrivate(parent)
    , m_imageElement(0)
{
}

HTMLAreaElementPrivate::~HTMLAreaElementPrivate()
{
    delete m_imageElement;
}

HTMLImageElement*
HTMLAreaElementPrivate::imageElement()
{
    WebCore::HTMLImageElement* i = reinterpret_cast<WebCore::HTMLAreaElement*>(webcore())->imageElement();
    if (!i)
        return 0;
    if (!m_imageElement || m_imageElement->webcore()!=reinterpret_cast<WebCore::HTMLElement*>(i)) {
        delete m_imageElement;
        m_imageElement = new HTMLImageElementPrivate(i);
    }

    return reinterpret_cast<HTMLImageElement*>(&m_imageElement->wkc());
}

WKCRect
HTMLAreaElementPrivate::getRect(RenderObject* wobj)
{
    WebCore::RenderObject* obj = 0;
    if (wobj) {
        obj = const_cast<WebCore::RenderObject*>(wobj->priv().webcore());
    }
    return ((WebCore::HTMLAreaElement *)webcore())->getRect(obj);
}

HTMLImageElement*
HTMLAreaElement::imageElement() const
{
    return ((HTMLAreaElementPrivate&)priv()).imageElement();
}

WKCRect
HTMLAreaElement::getRect(RenderObject* object)
{
    return ((HTMLAreaElementPrivate&)priv()).getRect(object);
}

} // namespace
