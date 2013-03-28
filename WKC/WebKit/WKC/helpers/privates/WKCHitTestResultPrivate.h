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

#ifndef _WKC_HELPERS_PRIVATE_HITTESTRESULT_H_
#define _WKC_HELPERS_PRIVATE_HITTESTRESULT_H_

#include <wkc/wkcbase.h>
#include "helpers/WKCHitTestResult.h"

namespace WebCore {
class HitTestResult;
} // namespace

namespace WKC {

class NodePrivate;
class ScrollbarPrivate;

class HitTestResultPrivateBase {
public:
    HitTestResultPrivateBase();
    ~HitTestResultPrivateBase();

    virtual const WebCore::HitTestResult& webcore() const = 0;
    virtual const HitTestResult& wkc() const = 0;

    Node* innerNode();
    Scrollbar* scrollbar();

private:
    NodePrivate* m_innerNode;
    ScrollbarPrivate* m_scrollbar;
};

class HitTestResultPrivate : public HitTestResultPrivateBase {
public:
    HitTestResultPrivate(const WebCore::HitTestResult&);
    ~HitTestResultPrivate();

    virtual const WebCore::HitTestResult& webcore() const { return m_webcore; }
    virtual const HitTestResult& wkc() const { return m_wkc; }

private:
    const WebCore::HitTestResult& m_webcore;
    HitTestResult m_wkc;
};

class HitTestResultPrivateToCore : public HitTestResultPrivateBase {
public:
    HitTestResultPrivateToCore(const HitTestResult&, const WKCPoint&);
    ~HitTestResultPrivateToCore();

    virtual const WebCore::HitTestResult& webcore() const { return *m_webcore; }
    virtual const HitTestResult& wkc() const { return m_wkc; }

private:
    const HitTestResult& m_wkc;
    WebCore::HitTestResult* m_webcore;
};

} // namespace

#endif // _WKC_HELPERS_PRIVATE_HITTESTRESULT_H_
