/*
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (c) 2010, 2011 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef FocusController_h
#define FocusController_h

#include "FocusDirection.h"
#if ENABLE(WKC_SPATIAL_NAVI)
#include "SpatialNavigation.h"
#endif
#include <wtf/Forward.h>
#include <wtf/Noncopyable.h>
#include <wtf/RefPtr.h>

namespace WebCore {

#if ENABLE(WKC_SPATIAL_NAVI)
    class Document;
#endif
    class Frame;
    class KeyboardEvent;
    class Node;
    class Page;

    class FocusController : public Noncopyable {
    public:
        FocusController(Page*);

        void setFocusedFrame(PassRefPtr<Frame>);
        Frame* focusedFrame() const { return m_focusedFrame.get(); }
        Frame* focusedOrMainFrame();

        bool setInitialFocus(FocusDirection, KeyboardEvent*);
        bool advanceFocus(FocusDirection, KeyboardEvent*, bool initialFocus = false);
        
        bool setFocusedNode(Node*, PassRefPtr<Frame>);

        void setActive(bool);
        bool isActive() const { return m_isActive; }

        void setFocused(bool);
        bool isFocused() const { return m_isFocused; }

#if ENABLE(WKC_SPATIAL_NAVI)
#if PLATFORM(WKC)
        Node* findNextFocusableNode(const FocusDirection direction, const IntRect* specificRect);
        Node* findNextFocusableNodeInRect(const FocusDirection direction, Frame* frame, const IntRect* rect);
#endif
#endif

    private:
#if ENABLE(WKC_SPATIAL_NAVI)
        bool advanceFocusDirectionally(FocusDirection, KeyboardEvent*);
        bool advanceFocusInDocumentOrder(FocusDirection, KeyboardEvent*, bool initialFocus);
#if PLATFORM(WKC)
        void findFocusableNodeInDirection(Node* outter, Node*, FocusDirection, KeyboardEvent*,
                                          FocusCandidate& closestFocusCandidate,
                                          const FocusCandidate& parentCandidate = FocusCandidate(),
                                          const IntRect* specificRect = 0);
        void deepFindFocusableNodeInDirection(Node* container, Node* focused, FocusDirection, KeyboardEvent*, FocusCandidate&, const IntRect* specificRect);
#else
        void findFocusableNodeInDirection(Node* outter, Node*, FocusDirection, KeyboardEvent*,
                                          FocusCandidate& closestFocusCandidate,
                                          const FocusCandidate& parentCandidate = FocusCandidate());
        void deepFindFocusableNodeInDirection(Node* container, Node* focused, FocusDirection, KeyboardEvent*, FocusCandidate&);
#endif

#if PLATFORM(WKC)
        Node* findVerticallyFocusableNodeInRect(FocusDirection direction, Node* start, KeyboardEvent* event, const IntRect* rect);
        void findHorizontallyFocusableNodeInRect(FocusDirection direction, Node* start, KeyboardEvent* event, const IntRect* rect, Node** candidateNode, IntRect& candidateNodeRect);
#endif
#endif
        Page* m_page;
        RefPtr<Frame> m_focusedFrame;
        bool m_isActive;
        bool m_isFocused;
    };

} // namespace WebCore
    
#endif // FocusController_h
