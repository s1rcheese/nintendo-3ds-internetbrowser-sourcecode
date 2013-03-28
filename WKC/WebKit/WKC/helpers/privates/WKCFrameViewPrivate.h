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

#ifndef _WKC_HELPERS_PRIVATE_FRAMEVIEW_H_
#define _WKC_HELPERS_PRIVATE_FRAMEVIEW_H_

#include "helpers/WKCFrameView.h"

namespace WebCore {
class FrameView;
} // namespace

namespace WKC {

class Scrollbar;
class ScrollbarPrivate;

class FrameViewPrivate {
public:
    FrameViewPrivate(WebCore::FrameView*);
    ~FrameViewPrivate();

    WebCore::FrameView* webcore() const { return m_webcore; }
    FrameView& wkc() { return m_wkc; }

    void scrollPositionChanged();
    void setUseSlowRepaints();
    void layoutIfNeededRecursive();
    void forceLayout();

    WKCPoint contentsToWindow(const WKCPoint&);
    WKCRect contentsToWindow(const WKCRect&);
    WKCRect convertToContainingWindow(const WKCRect&) const;

    WKCPoint windowToContents(const WKCPoint&);

    Scrollbar* verticalScrollbar();
    Scrollbar* horizontalScrollbar();

private:
    WebCore::FrameView* m_webcore;
    FrameView m_wkc;

    ScrollbarPrivate* m_verticalScrollbar;
    ScrollbarPrivate* m_horizontalScrollbar;

};
} // namespace

#endif // _WKC_HELPERS_PRIVATE_FRAMEVIEW_H_

