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

#include "helpers/WKCRenderObject.h"
#include "helpers/privates/WKCRenderObjectPrivate.h"

#include "RenderObject.h"

namespace WKC {
RenderObjectPrivate::RenderObjectPrivate(WebCore::RenderObject* parent)
    : m_webcore(parent)
    , m_wkc(*this)
{
}

RenderObjectPrivate::~RenderObjectPrivate()
{
}
 
bool
RenderObjectPrivate::isTextControl() const
{
    return m_webcore->isTextControl();
}

bool
RenderObjectPrivate::isTextArea() const
{
    return m_webcore->isTextArea();
}

WKCRect
RenderObjectPrivate::absoluteBoundingBoxRect()
{
    return m_webcore->absoluteBoundingBoxRect();
}

WKCRect
RenderObjectPrivate::absoluteClippedOverflowRect()
{
    return m_webcore->absoluteClippedOverflowRect();
}

RenderObject::RenderObject(RenderObjectPrivate& parent)
    : m_private(parent)
{
}

RenderObject::~RenderObject()
{
}


bool
RenderObject::isTextControl() const
{
    return m_private.isTextControl();
}

bool
RenderObject::isTextArea() const
{
    return m_private.isTextArea();
}

WKCRect
RenderObject::absoluteBoundingBoxRect() const
{
    return m_private.absoluteBoundingBoxRect();
}

WKCRect
RenderObject::absoluteClippedOverflowRect()
{
    return m_private.absoluteClippedOverflowRect();
}

} // namespace
