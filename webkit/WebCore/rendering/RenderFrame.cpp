/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
 *           (C) 2000 Stefan Schimanski (1Stein@gmx.de)
 * Copyright (C) 2004, 2005, 2006, 2009 Apple Inc. All rights reserved.
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

#include "config.h"
#include "RenderFrame.h"

#include "FrameView.h"
#include "HTMLFrameElement.h"
#if ENABLE(WKC_FRAME_FLATTENING)
#include "RenderView.h"
#endif
namespace WebCore {

RenderFrame::RenderFrame(HTMLFrameElement* frame)
    : RenderPart(frame)
{
    setInline(false);
}

FrameEdgeInfo RenderFrame::edgeInfo() const
{
    HTMLFrameElement* element = static_cast<HTMLFrameElement*>(node());
    return FrameEdgeInfo(element->noResize(), element->hasFrameBorder());
}

void RenderFrame::viewCleared()
{
    HTMLFrameElement* element = static_cast<HTMLFrameElement*>(node());
    if (!element || !widget() || !widget()->isFrameView())
        return;

    FrameView* view = static_cast<FrameView*>(widget());

    int marginw = element->getMarginWidth();
    int marginh = element->getMarginHeight();

    if (marginw != -1)
        view->setMarginWidth(marginw);
    if (marginh != -1)
        view->setMarginHeight(marginh);
}

#if ENABLE(WKC_FRAME_FLATTENING)
void RenderFrame::layoutWithFlattening(bool flexibleWidth, bool flexibleHeight)
{
    //FrameView* childFrameView = static_cast<FrameView*>(m_widget);
    FrameView* childFrameView = static_cast<FrameView*>(widget());
    //RenderView* childRoot = childFrameView ? static_cast<RenderView*>(childFrameView->frame()->renderer()) : 0;
    RenderView* childRoot = childFrameView ? static_cast<RenderView*>(childFrameView->frame()->contentRenderer()) : 0;

    // don't expand frames set to have zero width or height
    //if (!m_width || !m_height || !childRoot) {
    if (!width() || !height()|| !childRoot) {
        updateWidgetPosition();
        if (childFrameView)
            while (childFrameView->layoutPending())
                childFrameView->layout();
        setNeedsLayout(false);
        return;
    }

    // expand the frame by setting frame height = content height
    
    // need to update to calculate min/max correctly
    updateWidgetPosition();
    if (childRoot->prefWidthsDirty())
        childRoot->calcPrefWidths();
    
    HTMLFrameElement* element = static_cast<HTMLFrameElement*>(node());
    bool scrolling = element->scrollingMode() != ScrollbarAlwaysOff;
    //bool scrolling = element()->scrollingMode() != ScrollbarAlwaysOff;
    
    // if scrollbars are off assume it is ok for this frame to become really narrow
    if (scrolling || flexibleWidth || childFrameView->frame()->isFrameSet())
         setWidth(max(width(), childRoot->minPrefWidth()));
         //m_width = max(m_width, childRoot->minPrefWidth());
 
    // update again to pass the width to the child frame
    updateWidgetPosition();
     
    do
        childFrameView->layout();
    while (childFrameView->layoutPending() || childRoot->needsLayout());
        
    if (scrolling || flexibleHeight || childFrameView->frame()->isFrameSet())
        setHeight(max(height(), childFrameView->contentsHeight()));
        //m_height = max(m_height, childFrameView->contentsHeight());
    if (scrolling || flexibleWidth || childFrameView->frame()->isFrameSet())
        setWidth(max(width(), childFrameView->contentsWidth()));
        //m_width = max(m_width, childFrameView->contentsWidth());
    
    updateWidgetPosition();

    setNeedsLayout(false);
}
#endif

} // namespace WebCore
