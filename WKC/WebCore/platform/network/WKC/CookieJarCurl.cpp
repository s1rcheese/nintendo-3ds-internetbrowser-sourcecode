/*
 * Copyright (c) 2010-2012 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 * 
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 * 
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#include "config.h"
#include "CookieJar.h"

#include "Cookie.h"
#include "Document.h"
#include "KURL.h"
#include "PlatformString.h"
#include "StringHash.h"
#include "ResourceHandleManager.h"
#include "FrameLoaderClient.h"

#include <wtf/HashMap.h>

namespace WebCore {

#if 0
static String optimize_url(const KURL& url)
{
    if (url.isNull())
        return String();

    KURL opt_url = KURL(ParsedURLString, url.baseAsString());

    return opt_url.protocol() + "://" + opt_url.host() + opt_url.path();
}
#endif

void setCookies(Document* document, const KURL& url, const String& value)
{
    String firstparty_host;
    String cookie_domain;

    if (document) {
        Document* top = document->topDocument();
        if (top) {
            firstparty_host = top->url().host();
        }
    }
    if (firstparty_host.isEmpty()) {
        firstparty_host = url.host();
    }

    if (!document || !document->frame())
        return;

    if (-1 != value.find("domain=", 0, false)) {
        cookie_domain = value.substring(value.find("domain=", 0, false) + strlen("domain="));
        int len1 = cookie_domain.find(",");
        int len2 = cookie_domain.find(";");
        int len  = -1;
        if (-1 != len1 || -1 != len2) {
            if (len1 > len2)
                len = (-1 < len2) ? len2 : len1;
            else
                len = (-1 < len1) ? len1 : len2;
            cookie_domain = cookie_domain.left(len);
        }
    }
    else
        cookie_domain = "." + url.host();

    if (!document->frame()->loader()->client()->dispatchWillAcceptCookie(firstparty_host, cookie_domain))
        return;
    
    KURL opt_url = KURL(ParsedURLString, url.baseAsString());
    ResourceHandleManager::sharedCookieSet(opt_url.host(), opt_url.path(), value);
}

String cookies(const Document* /*document*/, const KURL& url)
{
    KURL opt_url = KURL(ParsedURLString, url.baseAsString());
    bool secure = url.protocolIs("https");
    return ResourceHandleManager::sharedCookieGet(opt_url.host(), opt_url.path(), secure);
}

bool cookiesEnabled(const Document* /*document*/)
{
    return true;
}

// below functions are only for Inspectors

bool getRawCookies(const Document*, const KURL&, Vector<Cookie>& rawCookies)
{
    // FIXME: Not yet implemented
    rawCookies.clear();
    return false; // return true when implemented
}

void deleteCookie(const Document*, const KURL&, const String&)
{
    // FIXME: Not yet implemented
}

}
