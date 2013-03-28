/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2008 Apple Inc. All rights reserved.
 * Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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

#ifndef _WKC_HELPERS_WKC_HTMLIMAGEELEMENT_H_
#define _WKC_HELPERS_WKC_HTMLIMAGEELEMENT_H_

#include "WKCElement.h"

namespace WKC {
class HTMLImageElement : public WKC::Element {
public:
    int width(bool flag=false) const;
    int height(bool flag=false) const;
};
}

#endif // _WKC_HELPERS_WKC_HTMLIMAGEELEMENT_H_
