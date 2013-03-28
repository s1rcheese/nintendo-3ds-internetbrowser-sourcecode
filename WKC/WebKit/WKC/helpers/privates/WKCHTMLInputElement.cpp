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

#include "helpers/WKCHTMLInputElement.h"
#include "helpers/WKCString.h"

#include "HTMLInputElement.h"

#include "helpers/WKCString.h"
#include "helpers/privates/WKCHTMLInputElementPrivate.h"

namespace WKC {

HTMLInputElementPrivate::HTMLInputElementPrivate(WebCore::HTMLInputElement* parent)
    : HTMLElementPrivate(parent)
{
}

HTMLInputElementPrivate::~HTMLInputElementPrivate()
{
}

HTMLInputElement::InputType
HTMLInputElementPrivate::inputType() const
{
    return (HTMLInputElement::InputType)reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->inputType();
}


String
HTMLInputElementPrivate::value() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->value();
}

bool
HTMLInputElementPrivate::readOnly() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->readOnly();
}

bool
HTMLInputElementPrivate::disabled() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->disabled();
}

int
HTMLInputElementPrivate::maxLength() const
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->maxLength();
}

void
HTMLInputElementPrivate::setValue(const String& str, bool sendChangeEvent)
{
    return reinterpret_cast<WebCore::HTMLInputElement*>(webcore())->setValue(str, sendChangeEvent);
}

HTMLInputElement::InputType
HTMLInputElement::inputType() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).inputType();
}

const String
HTMLInputElement::value() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).value();
}

bool
HTMLInputElement::readOnly() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).readOnly();
}

bool
HTMLInputElement::disabled() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).disabled();
}

int
HTMLInputElement::maxLength() const
{
    return reinterpret_cast<HTMLInputElementPrivate&>(priv()).maxLength();
}

void
HTMLInputElement::setValue(const String& str, bool sendChangeEvent)
{
    reinterpret_cast<HTMLInputElementPrivate&>(priv()).setValue(str, sendChangeEvent);
}

} // namespace
