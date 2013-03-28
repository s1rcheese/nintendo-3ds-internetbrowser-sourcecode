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

#include "helpers/WKCHistoryItem.h"
#include "helpers/privates/WKCHistoryItemPrivate.h"

#include "HistoryItem.h"
#include "helpers/privates/WKCImagePrivate.h"

namespace WKC {
HistoryItemPrivate::HistoryItemPrivate(WebCore::HistoryItem* parent)
    : m_webcore(parent)
    , m_wkc(*this)
    , m_image(0)
{
}

HistoryItemPrivate::~HistoryItemPrivate()
{
    delete m_image;
}


const String&
HistoryItemPrivate::title()
{
    m_title = m_webcore->title();
    return m_title;
}

const String&
HistoryItemPrivate::urlString()
{
    m_urlString = m_webcore->urlString();
    return m_urlString;
}

const String&
HistoryItemPrivate::originalURLString()
{
    m_originalURLString = m_webcore->originalURLString();
    return m_originalURLString;
}

Image*
HistoryItemPrivate::icon()
{
    WebCore::Image* img = m_webcore->icon();
    if (!img)
        return 0;

    if (!m_image || m_image->webcore()!=img) {
        delete m_image;
        m_image = new ImagePrivate(img);
    }
    return &m_image->wkc();
}

void
HistoryItemPrivate::setURLString(const String& str)
{
    m_webcore->setURLString(str);
}

int
HistoryItemPrivate::refCount() const
{
    return m_webcore->refCount();
}

HistoryItem::HistoryItem(HistoryItemPrivate& parent)
    : m_private(parent)
{
}

HistoryItem::~HistoryItem()
{
}


const String&
HistoryItem::urlString() const
{
    return m_private.urlString();
}

const String&
HistoryItem::originalURLString() const
{
    return m_private.originalURLString();
}

const String&
HistoryItem::title() const
{
    return m_private.title();
}

Image*
HistoryItem::icon() const
{
    return m_private.icon();
}

void
HistoryItem::setURLString(const String& str)
{
    m_private.setURLString(str);
}

int
HistoryItem::refCount() const
{
    return m_private.refCount();
}

} // namespace
