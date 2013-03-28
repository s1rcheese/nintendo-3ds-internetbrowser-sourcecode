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

#include "helpers/WKCElement.h"
#include "helpers/privates/WKCElementPrivate.h"

#include "Element.h"

#include "helpers/WKCNode.h"
#include "helpers/privates/WKCNodePrivate.h"

namespace WKC {

ElementPrivate::ElementPrivate(WebCore::Element* parent)
    : NodePrivate(parent)
    , m_webcore(parent)
    , m_wkc(*this)
{
}

ElementPrivate::~ElementPrivate()
{
}


Element::Element(ElementPrivate& parent)
    : Node(parent)
{
}

bool
ElementPrivate::isFormControlElement() const
{
    return ((WebCore::Element*)(((NodePrivate*)this)->webcore()))->isFormControlElement();
}

bool
Element::isFormControlElement() const
{
    return ((ElementPrivate&)priv()).isFormControlElement();
}

} // namespace
