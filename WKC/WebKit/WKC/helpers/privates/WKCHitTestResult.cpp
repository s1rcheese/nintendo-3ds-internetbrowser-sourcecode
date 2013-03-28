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

#include "helpers/WKCHitTestResult.h"
#include "helpers/privates/WKCHitTestResultPrivate.h"

#include "HitTestResult.h"

#include "helpers/privates/WKCNodePrivate.h"
#include "helpers/privates/WKCScrollbarPrivate.h"

namespace WKC {

HitTestResultPrivateBase::HitTestResultPrivateBase()
    : m_innerNode(0)
    , m_scrollbar(0)
{
}

HitTestResultPrivateBase::~HitTestResultPrivateBase()
{
    delete m_innerNode;
    delete m_scrollbar;
}

HitTestResultPrivate::HitTestResultPrivate(const WebCore::HitTestResult& parent)
    : HitTestResultPrivateBase()
    , m_webcore(parent)
    , m_wkc(this)
{
}

HitTestResultPrivate::~HitTestResultPrivate()
{
}

HitTestResultPrivateToCore::HitTestResultPrivateToCore(const HitTestResult& parent, const WKCPoint& pos)
    : HitTestResultPrivateBase()
    , m_wkc(parent)
    , m_webcore(new WebCore::HitTestResult(pos))
{
}

HitTestResultPrivateToCore::~HitTestResultPrivateToCore()
{
    delete m_webcore;
}

Node*
HitTestResultPrivateBase::innerNode()
{
    WebCore::Node* inode = webcore().innerNode();
    if (!inode)
        return 0;

    delete m_innerNode;
    m_innerNode = NodePrivate::create(inode);
    return &m_innerNode->wkc();
}

Scrollbar*
HitTestResultPrivateBase::scrollbar()
{
    WebCore::Scrollbar* bar = webcore().scrollbar();
    if (!bar)
        return 0;
    if (!m_scrollbar || m_scrollbar->webcore()!=bar) {
        delete m_scrollbar;
        m_scrollbar = new ScrollbarPrivate(bar);
    }
    return &m_scrollbar->wkc();
}

HitTestResult::HitTestResult(HitTestResultPrivate* parent)
    : m_private(parent)
    , m_owned(false)
{
}

HitTestResult::HitTestResult(const WKCPoint& pos)
    : m_private(reinterpret_cast<HitTestResultPrivate*>(new HitTestResultPrivateToCore(*this, pos)))
    , m_owned(true)
{
}

HitTestResult::~HitTestResult()
{
    if (m_owned)
        delete reinterpret_cast<HitTestResultPrivateToCore*>(m_private);
}

Node*
HitTestResult::innerNode() const
{
    return m_private->innerNode();
}

Scrollbar*
HitTestResult::scrollbar()
{
    return m_private->scrollbar();
}

} // namespace
