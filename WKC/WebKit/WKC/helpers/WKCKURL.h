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

#ifndef _WKC_HELPERS_WKC_KURL_H_
#define _WKC_HELPERS_WKC_KURL_H_

namespace WKC {
class String;

enum WKCURLParsedEnum { WKCURLParsed };

class KURLPrivate;

class KURL {
public:
    KURL();
    KURL(const KURL&, const char*);
    KURL(WKCURLParsedEnum, const char*);
    ~KURL();

    KURL& operator=(const KURL&);
    operator String() const;

    const String string() const;
    const String protocol() const;
    const String host() const;

    KURL(KURLPrivate*);
    KURL(const KURL&);

    KURLPrivate* parent() const { return m_parent; }

private:
    KURLPrivate* m_parent;
};

String decodeURLEscapeSequences(const String&);
bool protocolIs(const String& url, const char* protocol);

} // namespace

#endif // _WKC_HELPERS_WKC_KURL_H_
