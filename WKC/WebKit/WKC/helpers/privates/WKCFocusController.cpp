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

#include "helpers/WKCFocusController.h"
#include "helpers/privates/WKCFocusControllerPrivate.h"

#include "FocusController.h"
#include "IntRect.h"

#include "helpers/WKCNode.h"
#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/privates/WKCNodePrivate.h"

namespace {

static bool
convertFocusDir(const WKC::FocusDirection wkc_dir, WebCore::FocusDirection& webcore_dir)
{
    switch (wkc_dir) {
        case WKC::EFocusDirectionUp:
            webcore_dir = WebCore::FocusDirectionUp;
            break;
        case WKC::EFocusDirectionDown:
            webcore_dir = WebCore::FocusDirectionDown;
            break;
        case WKC::EFocusDirectionLeft:
            webcore_dir = WebCore::FocusDirectionLeft;
            break;
        case WKC::EFocusDirectionRight:
            webcore_dir = WebCore::FocusDirectionRight;
            break;
        default:
            return false;
    }
    return true;
}

}

namespace WKC {

FocusControllerPrivate::FocusControllerPrivate(WebCore::FocusController* parent)
    : m_webcore(parent)
    , m_wkc(*this)
    , m_focusedFrame(0)
    , m_focusableNode(0)
    , m_focusableNodeInRect(0)
{
}

FocusControllerPrivate::~FocusControllerPrivate()
{
    delete m_focusableNodeInRect;
    delete m_focusableNode;
    delete m_focusedFrame;
}

Frame*
FocusControllerPrivate::focusedOrMainFrame()
{
    WebCore::Frame* frame = m_webcore->focusedOrMainFrame();
    if (!frame)
        return 0;
    if (!m_focusedFrame || m_focusedFrame->webcore()!=frame) {
        delete m_focusedFrame;
        m_focusedFrame = new FramePrivate(frame);
    }
    return &m_focusedFrame->wkc();
}

Node*
FocusControllerPrivate::findNextFocusableNode(FocusDirection direction, const WKCRect* specificRect)
{
    WebCore::FocusDirection webcore_dir;
    bool ok = convertFocusDir(direction, webcore_dir);
    if (!ok) {
        return 0;
    }

    WebCore::Node* node;
    if (specificRect) {
        WebCore::IntRect r(specificRect->fX, specificRect->fY, specificRect->fWidth, specificRect->fHeight);
        node = m_webcore->findNextFocusableNode(webcore_dir, &r);
    } else {
        node = m_webcore->findNextFocusableNode(webcore_dir, 0);
    }
    if (!node)
        return 0;

    if (m_focusableNode)
        delete m_focusableNode;
    m_focusableNode = NodePrivate::create(node);
    if (!m_focusableNode)
        return 0;

    return &m_focusableNode->wkc();
}

Node*
FocusControllerPrivate::findNextFocusableNodeInRect(FocusDirection direction, Frame* frame, const WKCRect* rect)
{
    WebCore::FocusDirection webcore_dir;
    bool ok = convertFocusDir(direction, webcore_dir);
    if (!ok) {
        return 0;
    }
    WebCore::IntRect r(rect->fX, rect->fY, rect->fWidth, rect->fHeight);
    WebCore::Node* node = m_webcore->findNextFocusableNodeInRect(webcore_dir, const_cast<WebCore::Frame*>(frame->priv().webcore()), const_cast<const WebCore::IntRect*>(&r));
    if (!node)
        return 0;

    if (m_focusableNodeInRect)
        delete m_focusableNodeInRect;
    m_focusableNodeInRect = NodePrivate::create(node);
    if (!m_focusableNodeInRect)
        return 0;

    return &m_focusableNodeInRect->wkc();
}

bool
isScrollableContainerNode(Node* node)
{
    return WebCore::isScrollableContainerNode(node ? node->priv().webcore() : 0);
}

bool
hasOffscreenRect(Node* node)
{
    return WebCore::hasOffscreenRect(node ? node->priv().webcore() : 0);
}


FocusController::FocusController(FocusControllerPrivate& parent)
    : m_private(parent)
{
}

FocusController::~FocusController()
{
}

Frame*
FocusController::focusedOrMainFrame()
{
    return m_private.focusedOrMainFrame();
}

Node*
FocusController::findNextFocusableNode(FocusDirection direction, const WKCRect* specificRect)
{
    return m_private.findNextFocusableNode(direction, specificRect);
}

Node*
FocusController::findNextFocusableNodeInRect(FocusDirection direction, Frame* frame, const WKCRect* rect)
{
    return m_private.findNextFocusableNodeInRect(direction, frame, rect);
}

} // namespace
