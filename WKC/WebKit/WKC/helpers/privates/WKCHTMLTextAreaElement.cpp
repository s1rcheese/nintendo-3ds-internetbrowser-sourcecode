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

#include "helpers/WKCHTMLTextAreaElement.h"
#include "helpers/privates/WKCHTMLTextAreaElementPrivate.h"

#include "HTMLTextAreaElement.h"

#include "helpers/WKCString.h"

namespace WKC {

HTMLTextAreaElementPrivate::HTMLTextAreaElementPrivate(WebCore::HTMLTextAreaElement* parent)
    : HTMLElementPrivate(parent)
{
}

HTMLTextAreaElementPrivate::~HTMLTextAreaElementPrivate()
{
}

String
HTMLTextAreaElementPrivate::value() const
{
    return reinterpret_cast<WebCore::HTMLTextAreaElement*>(webcore())->value();
}

bool
HTMLTextAreaElementPrivate::readOnly() const
{
    return reinterpret_cast<WebCore::HTMLTextAreaElement*>(webcore())->readOnly();
}

bool
HTMLTextAreaElementPrivate::disabled() const
{
    return reinterpret_cast<WebCore::HTMLTextAreaElement*>(webcore())->disabled();
}

int
HTMLTextAreaElementPrivate::maxLength() const
{
    return reinterpret_cast<WebCore::HTMLTextAreaElement*>(webcore())->maxLength();
}

void
HTMLTextAreaElementPrivate::setValue(const String& str)
{
    reinterpret_cast<WebCore::HTMLTextAreaElement*>(webcore())->setValue(str);
}

const String
HTMLTextAreaElement::value() const
{
    return reinterpret_cast<HTMLTextAreaElementPrivate&>(priv()).value();
}

bool
HTMLTextAreaElement::readOnly() const
{
    return reinterpret_cast<HTMLTextAreaElementPrivate&>(priv()).readOnly();
}

bool
HTMLTextAreaElement::disabled() const
{
    return reinterpret_cast<HTMLTextAreaElementPrivate&>(priv()).disabled();
}

int
HTMLTextAreaElement::maxLength() const
{
    return reinterpret_cast<HTMLTextAreaElementPrivate&>(priv()).maxLength();
}

void
HTMLTextAreaElement::setValue(const String& str)
{
    reinterpret_cast<HTMLTextAreaElementPrivate&>(priv()).setValue(str);
}
} // namespace
