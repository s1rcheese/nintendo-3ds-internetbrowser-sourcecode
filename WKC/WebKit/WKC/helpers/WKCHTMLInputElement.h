/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (c) 2010, 2011 ACCESS CO., LTD. All rights reserved.
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
 *
 */

#ifndef _WKC_HELPERS_WKC_HTMLINPUTELEMENT_H_
#define _WKC_HELPERS_WKC_HTMLINPUTELEMENT_H_

#include "WKCHTMLElement.h"

namespace WKC {
class RenderObject;
class String;

class HTMLInputElement : public WKC::HTMLElement {
public:
    enum InputType {
        TEXT = 0, // TEXT must be 0.
        PASSWORD,
        ISINDEX,
        CHECKBOX,
        RADIO,
        SUBMIT,
        RESET,
        FILE,
        HIDDEN,
        IMAGE,
        BUTTON,
        SEARCH,
        RANGE,
        EMAIL,
        NUMBER,
        TELEPHONE,
        URL,
        COLOR,
        DATE,
        DATETIME,
        DATETIMELOCAL,
        MONTH,
        TIME,
        WEEK,
        // If you add new types or change the order of enum values, update numberOfTypes below.
    };

    InputType inputType() const;

    const String value() const;
    bool readOnly() const;
    bool disabled() const;
    int maxLength() const;
    void setValue(const String&, bool sendChangeEvent = false);
};
}

#endif // _WKC_HELPERS_WKC_HTMLINPUTELEMENT_H_

