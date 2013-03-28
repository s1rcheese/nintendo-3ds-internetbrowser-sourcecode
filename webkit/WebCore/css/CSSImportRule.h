/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002, 2006, 2008 Apple Inc. All rights reserved.
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

#ifndef CSSImportRule_h
#define CSSImportRule_h

#include "CSSRule.h"
#include "CachedResourceClient.h"
#include "CachedResourceHandle.h"
#include "MediaList.h"
#include "PlatformString.h"

namespace WebCore {

class CachedCSSStyleSheet;
class MediaList;

class CSSImportRule : public CSSRule, private CachedResourceClient {
public:
    static PassRefPtr<CSSImportRule> create(CSSStyleSheet* parent, const String& href, PassRefPtr<MediaList> media)
    {
        return adoptRef(new CSSImportRule(parent, href, media));
    }

    virtual ~CSSImportRule();

    String href() const { return m_strHref; }
    MediaList* media() const { return m_lstMedia.get(); }
    CSSStyleSheet* styleSheet() const { return m_styleSheet.get(); }

    virtual String cssText() const;

    // Not part of the CSSOM
    bool isLoading() const;

    virtual void addSubresourceStyleURLs(ListHashSet<KURL>& urls);

    static void resetVariables();
    static void deleteSharedInstance();

private:
    CSSImportRule(CSSStyleSheet* parent, const String& href, PassRefPtr<MediaList>);

    virtual bool isImportRule() { return true; }
    virtual void insertedIntoParent();

    // from CSSRule
    virtual unsigned short type() const { return IMPORT_RULE; }

    // from CachedResourceClient
#if 1
    // modified at webkit.org trunk r53607
    virtual void setCSSStyleSheet(const String& href, const KURL& baseURL, const String& charset, const CachedCSSStyleSheet*);
#else
    virtual void setCSSStyleSheet(const String& url, const String& charset, const CachedCSSStyleSheet*);
#endif

    String m_strHref;
    RefPtr<MediaList> m_lstMedia;
    RefPtr<CSSStyleSheet> m_styleSheet;
    CachedResourceHandle<CachedCSSStyleSheet> m_cachedSheet;
    bool m_loading;

    static const String* m_SlashKHTMLFixesDotCss;
    static const String* m_MediaWikiKHTMLFixesStyleSheet;
};

} // namespace WebCore

#endif // CSSImportRule_h
