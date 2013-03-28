/**
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2005, 2006 Apple Computer, Inc.
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
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
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "CSSInitialValue.h"

#include "PlatformString.h"

namespace WebCore {

CSSInitialValue* CSSInitialValue::m_ExplicitValue = 0;

/* static */ PassRefPtr<CSSInitialValue> CSSInitialValue::createExplicit()
{
    if (!m_ExplicitValue) {
        m_ExplicitValue = new CSSInitialValue(false);
    }

    return m_ExplicitValue;
}

/* static */ PassRefPtr<CSSInitialValue> CSSInitialValue::createImplicit()
{
    if (!m_ExplicitValue) {
        m_ExplicitValue = new CSSInitialValue(true);
    }

    return m_ExplicitValue;
}

unsigned short CSSInitialValue::cssValueType() const
{ 
    return CSS_INITIAL; 
}

String CSSInitialValue::cssText() const
{
    return "initial";
}

/* static */ void CSSInitialValue::resetVariables()
{
    m_ExplicitValue = 0;
}

/* static */ void CSSInitialValue::deleteSharedInstance()
{
    delete m_ExplicitValue;
    m_ExplicitValue = 0;
}

} // namespace WebCore
