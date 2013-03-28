/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2006, 2007, 2008 Apple Inc. All rights reserved.
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

#ifndef CSSStyleSheet_h
#define CSSStyleSheet_h

#include "CSSRuleList.h"
#include "StyleSheet.h"

namespace WebCore {

class CSSNamespace;
class CSSParser;
class CSSRule;
class DocLoader;
class Document;

typedef int ExceptionCode;

class CSSStyleSheet : public StyleSheet {
public:
    static PassRefPtr<CSSStyleSheet> create()
    {
#if 1
        // modified at webkit.org trunk r53607
        return adoptRef(new CSSStyleSheet(static_cast<CSSStyleSheet*>(0), String(), KURL(), String()));
#else
        return adoptRef(new CSSStyleSheet(static_cast<CSSStyleSheet*>(0), String(), String()));
#endif
    }
    static PassRefPtr<CSSStyleSheet> create(Node* ownerNode)
    {
#if 1
        // modified at webkit.org trunk r53607
        return adoptRef(new CSSStyleSheet(ownerNode, String(), KURL(), String()));
#else
        return adoptRef(new CSSStyleSheet(ownerNode, String(), String()));
#endif
    }
#if 1
    // modified at webkit.org trunk r53607 and r54645
    static PassRefPtr<CSSStyleSheet> create(Node* ownerNode, const String& originalURL, const KURL& finalURL)
#else
    static PassRefPtr<CSSStyleSheet> create(Node* ownerNode, const String& href)
#endif
    {
#if 1
        // modified at webkit.org trunk r53607 and r54645
        return adoptRef(new CSSStyleSheet(ownerNode, originalURL, finalURL, String()));
#else
        return adoptRef(new CSSStyleSheet(ownerNode, href, String()));
#endif
    }
#if 1
    // modified at webkit.org trunk r53607 and r54645
    static PassRefPtr<CSSStyleSheet> create(Node* ownerNode, const String& originalURL, const KURL& finalURL, const String& charset)
#else
    static PassRefPtr<CSSStyleSheet> create(Node* ownerNode, const String& href, const String& charset)
#endif
    {
#if 1
        // modified at webkit.org trunk r53607 and r54645
        return adoptRef(new CSSStyleSheet(ownerNode, originalURL, finalURL, charset));
#else
        return adoptRef(new CSSStyleSheet(ownerNode, href, charset));
#endif
    }
#if 1
    // modified at webkit.org trunk r53607 and r54645
    static PassRefPtr<CSSStyleSheet> create(CSSRule* ownerRule, const String& originalURL, const KURL& finalURL, const String& charset)
#else
    static PassRefPtr<CSSStyleSheet> create(CSSRule* ownerRule, const String& href, const String& charset)
#endif
    {
#if 1
        // modified at webkit.org trunk r53607 and r54645
        return adoptRef(new CSSStyleSheet(ownerRule, originalURL, finalURL, charset));
#else
        return adoptRef(new CSSStyleSheet(ownerRule, href, charset));
#endif
    }
#if 1
    // added at webkit.org trunk r53607 and modified at r54645
    static PassRefPtr<CSSStyleSheet> createInline(Node* ownerNode, const KURL& finalURL)
    {
        return adoptRef(new CSSStyleSheet(ownerNode, finalURL.string(), finalURL, String()));
    }
#endif

    virtual ~CSSStyleSheet();

    CSSRule* ownerRule() const;
    PassRefPtr<CSSRuleList> cssRules(bool omitCharsetRules = false);
    unsigned insertRule(const String& rule, unsigned index, ExceptionCode&);
    void deleteRule(unsigned index, ExceptionCode&);

    // IE Extensions
    PassRefPtr<CSSRuleList> rules() { return cssRules(true); }
    int addRule(const String& selector, const String& style, int index, ExceptionCode&);
    int addRule(const String& selector, const String& style, ExceptionCode&);
    void removeRule(unsigned index, ExceptionCode& ec) { deleteRule(index, ec); }

    void addNamespace(CSSParser*, const AtomicString& prefix, const AtomicString& uri);
    const AtomicString& determineNamespace(const AtomicString& prefix);

    virtual void styleSheetChanged();

    virtual bool parseString(const String&, bool strict = true);

    virtual bool isLoading();

    virtual void checkLoaded();

    Document* doc() { return m_doc; }

    const String& charset() const { return m_charset; }

    bool loadCompleted() const { return m_loadCompleted; }

    virtual KURL completeURL(const String& url) const;
    virtual void addSubresourceStyleURLs(ListHashSet<KURL>&);

    void setStrictParsing(bool b) { m_strictParsing = b; }
    bool useStrictParsing() const { return m_strictParsing; }

    void setIsUserStyleSheet(bool b) { m_isUserStyleSheet = b; }
    bool isUserStyleSheet() const { return m_isUserStyleSheet; }

private:
#if 1
    // modified at webkit.org trunk r53607 and r54645
    CSSStyleSheet(Node* ownerNode, const String& originalURL, const KURL& finalURL, const String& charset);
    CSSStyleSheet(CSSStyleSheet* parentSheet, const String& originalURL, const KURL& finalURL, const String& charset);
    CSSStyleSheet(CSSRule* ownerRule, const String& originalURL, const KURL& finalURL, const String& charset);
#else
    CSSStyleSheet(Node* ownerNode, const String& href, const String& charset);
    CSSStyleSheet(CSSStyleSheet* parentSheet, const String& href, const String& charset);
    CSSStyleSheet(CSSRule* ownerRule, const String& href, const String& charset);
#endif

    virtual bool isCSSStyleSheet() const { return true; }
    virtual String type() const { return "text/css"; }

    Document* m_doc;
    CSSNamespace* m_namespaces;
    String m_charset;
    bool m_loadCompleted : 1;
    bool m_strictParsing : 1;
    bool m_isUserStyleSheet : 1;
};

} // namespace

#endif
