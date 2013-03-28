/*
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nuanti Ltd.
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

#include "config.h"
#include "FocusController.h"

#include "AXObjectCache.h"
#include "Chrome.h"
#include "Document.h"
#include "Editor.h"
#include "EditorClient.h"
#include "Element.h"
#include "Event.h"
#include "EventHandler.h"
#include "EventNames.h"
#include "ExceptionCode.h"
#include "Frame.h"
#include "FrameView.h"
#include "FrameTree.h"
#include "HTMLFrameOwnerElement.h"
#include "HTMLNames.h"
#include "KeyboardEvent.h"
#include "Page.h"
#include "Range.h"
#include "RenderObject.h"
#include "RenderWidget.h"
#include "SelectionController.h"
#include "Settings.h"
#if ENABLE(WKC_SPATIAL_NAVI)
#include "SpatialNavigation.h"
#endif
#include "Widget.h"
#include <wtf/Platform.h>

namespace WebCore {

using namespace HTMLNames;
#if ENABLE(WKC_SPATIAL_NAVI)
using namespace std;
#endif

static inline void dispatchEventsOnWindowAndFocusedNode(Document* document, bool focused)
{
    // If we have a focused node we should dispatch blur on it before we blur the window.
    // If we have a focused node we should dispatch focus on it after we focus the window.
    // https://bugs.webkit.org/show_bug.cgi?id=27105
    if (!focused && document->focusedNode())
        document->focusedNode()->dispatchBlurEvent();
    document->dispatchWindowEvent(Event::create(focused ? eventNames().focusEvent : eventNames().blurEvent, false, false));
    if (focused && document->focusedNode())
        document->focusedNode()->dispatchFocusEvent();
}

FocusController::FocusController(Page* page)
    : m_page(page)
    , m_isActive(false)
    , m_isFocused(false)
{
}

void FocusController::setFocusedFrame(PassRefPtr<Frame> frame)
{
    if (m_focusedFrame == frame)
        return;

    RefPtr<Frame> oldFrame = m_focusedFrame;
    RefPtr<Frame> newFrame = frame;

    m_focusedFrame = newFrame;

    // Now that the frame is updated, fire events and update the selection focused states of both frames.
    if (oldFrame && oldFrame->view()) {
        oldFrame->selection()->setFocused(false);
        oldFrame->document()->dispatchWindowEvent(Event::create(eventNames().blurEvent, false, false));
    }

    if (newFrame && newFrame->view() && isFocused()) {
        newFrame->selection()->setFocused(true);
        newFrame->document()->dispatchWindowEvent(Event::create(eventNames().focusEvent, false, false));
    }
}

Frame* FocusController::focusedOrMainFrame()
{
    if (Frame* frame = focusedFrame())
        return frame;
    return m_page->mainFrame();
}

void FocusController::setFocused(bool focused)
{
    if (isFocused() == focused)
        return;
    
    m_isFocused = focused;
    
    if (m_focusedFrame && m_focusedFrame->view()) {
        m_focusedFrame->selection()->setFocused(focused);
        dispatchEventsOnWindowAndFocusedNode(m_focusedFrame->document(), focused);
    }
}

static Node* deepFocusableNode(FocusDirection direction, Node* node, KeyboardEvent* event)
{
    // The node we found might be a HTMLFrameOwnerElement, so descend down the frame tree until we find either:
    // 1) a focusable node, or
    // 2) the deepest-nested HTMLFrameOwnerElement
    while (node && node->isFrameOwnerElement()) {
        HTMLFrameOwnerElement* owner = static_cast<HTMLFrameOwnerElement*>(node);
        if (!owner->contentFrame())
            break;

        Document* document = owner->contentFrame()->document();

        node = (direction == FocusDirectionForward)
            ? document->nextFocusableNode(0, event)
            : document->previousFocusableNode(0, event);
        if (!node) {
            node = owner;
            break;
        }
    }

    return node;
}

bool FocusController::setInitialFocus(FocusDirection direction, KeyboardEvent* event)
{
    return advanceFocus(direction, event, true);
}

bool FocusController::advanceFocus(FocusDirection direction, KeyboardEvent* event, bool initialFocus)
{
#if ENABLE(WKC_SPATIAL_NAVI)
    switch (direction) {
    case FocusDirectionForward:
    case FocusDirectionBackward:
        return advanceFocusInDocumentOrder(direction, event, initialFocus);
    case FocusDirectionLeft:
    case FocusDirectionRight:
    case FocusDirectionUp:
    case FocusDirectionDown:
        return advanceFocusDirectionally(direction, event);
    default:
        ASSERT_NOT_REACHED();
    }

    return false;
}

bool FocusController::advanceFocusInDocumentOrder(FocusDirection direction, KeyboardEvent* event, bool initialFocus)
{
#endif
    Frame* frame = focusedOrMainFrame();
    ASSERT(frame);
    Document* document = frame->document();

    Node* currentNode = document->focusedNode();
    // FIXME: Not quite correct when it comes to focus transitions leaving/entering the WebView itself
    bool caretBrowsing = focusedOrMainFrame()->settings()->caretBrowsingEnabled();

    if (caretBrowsing && !currentNode)
        currentNode = frame->selection()->start().node();

    document->updateLayoutIgnorePendingStylesheets();

    Node* node = (direction == FocusDirectionForward)
        ? document->nextFocusableNode(currentNode, event)
        : document->previousFocusableNode(currentNode, event);
            
    // If there's no focusable node to advance to, move up the frame tree until we find one.
    while (!node && frame) {
        Frame* parentFrame = frame->tree()->parent();
        if (!parentFrame)
            break;

        Document* parentDocument = parentFrame->document();

        HTMLFrameOwnerElement* owner = frame->ownerElement();
        if (!owner)
            break;

        node = (direction == FocusDirectionForward)
            ? parentDocument->nextFocusableNode(owner, event)
            : parentDocument->previousFocusableNode(owner, event);

        frame = parentFrame;
    }

    node = deepFocusableNode(direction, node, event);

    if (!node) {
        // We didn't find a node to focus, so we should try to pass focus to Chrome.
        if (!initialFocus && m_page->chrome()->canTakeFocus(direction)) {
            document->setFocusedNode(0);
            setFocusedFrame(0);
            m_page->chrome()->takeFocus(direction);
            return true;
        }

        // Chrome doesn't want focus, so we should wrap focus.
        Document* d = m_page->mainFrame()->document();
        node = (direction == FocusDirectionForward)
            ? d->nextFocusableNode(0, event)
            : d->previousFocusableNode(0, event);

        node = deepFocusableNode(direction, node, event);

        if (!node)
            return false;
    }

    ASSERT(node);

    if (node == document->focusedNode())
        // Focus wrapped around to the same node.
        return true;

    if (!node->isElementNode())
        // FIXME: May need a way to focus a document here.
        return false;

    if (node->isFrameOwnerElement()) {
        // We focus frames rather than frame owners.
        // FIXME: We should not focus frames that have no scrollbars, as focusing them isn't useful to the user.
        HTMLFrameOwnerElement* owner = static_cast<HTMLFrameOwnerElement*>(node);
        if (!owner->contentFrame())
            return false;

        document->setFocusedNode(0);
        setFocusedFrame(owner->contentFrame());
        return true;
    }
    
    // FIXME: It would be nice to just be able to call setFocusedNode(node) here, but we can't do
    // that because some elements (e.g. HTMLInputElement and HTMLTextAreaElement) do extra work in
    // their focus() methods.

    Document* newDocument = node->document();

    if (newDocument != document)
        // Focus is going away from this document, so clear the focused node.
        document->setFocusedNode(0);

    if (newDocument)
        setFocusedFrame(newDocument->frame());

    if (caretBrowsing) {
        VisibleSelection newSelection(Position(node, 0), Position(node, 0), DOWNSTREAM);
        if (frame->shouldChangeSelection(newSelection))
            frame->selection()->setSelection(newSelection);
    }

    static_cast<Element*>(node)->focus(false);
    return true;
}

#if ENABLE(WKC_SPATIAL_NAVI)
bool FocusController::advanceFocusDirectionally(FocusDirection direction, KeyboardEvent* event)
{
    // for event invoking
    return true;
}

static void updateFocusCandidateInSameContainer(const FocusCandidate& candidate, FocusCandidate& closest)
{
    if (closest.isNull()) {
        closest = candidate;
        return;
    }

    if (candidate.alignment == closest.alignment) {
        if (candidate.distance < closest.distance)
            closest = candidate;
        return;
    }

    if (candidate.alignment > closest.alignment)
        closest = candidate;
}

static void updateFocusCandidateIfCloser(Node* focusedNode, const FocusCandidate& candidate, FocusCandidate& closest)
{
    // First, check the common case: neither candidate nor closest are
    // inside scrollable content, then no need to care about enclosingScrollableBox
    // heuristics or parent{Distance,Alignment}, but only distance and alignment.
    if (!candidate.inScrollableContainer() && !closest.inScrollableContainer()) {
        updateFocusCandidateInSameContainer(candidate, closest);
        return;
    }

    bool sameContainer = candidate.document() == closest.document() && candidate.enclosingScrollableBox == closest.enclosingScrollableBox;

    // Second, if candidate and closest are in the same "container" (i.e. {i}frame or any
    // scrollable block element), we can handle them as common case.
    if (sameContainer) {
        updateFocusCandidateInSameContainer(candidate, closest);
        return;
    }

    // Last, we are considering moving to a candidate located in a different enclosing
    // scrollable box than closest.
    bool isInInnerDocument = !isInRootDocument(focusedNode);

    bool sameContainerAsCandidate = isInInnerDocument ? focusedNode->document() == candidate.document() :
        focusedNode->isDescendantOf(candidate.enclosingScrollableBox);

    bool sameContainerAsClosest = isInInnerDocument ? focusedNode->document() == closest.document() :
        focusedNode->isDescendantOf(closest.enclosingScrollableBox);

    // sameContainerAsCandidate and sameContainerAsClosest are mutually exclusive.
    ASSERT(!(sameContainerAsCandidate && sameContainerAsClosest));

    if (sameContainerAsCandidate) {
        closest = candidate;
        return;
    }

    if (sameContainerAsClosest) {
        // Nothing to be done.
        return;
    }

    // NOTE: !sameContainerAsCandidate && !sameContainerAsClosest
    // If distance is shorter, and we are talking about scrollable container,
    // lets compare parent distance and alignment before anything.
    if (candidate.distance < closest.distance) {
        if (candidate.alignment >= closest.parentAlignment
         || candidate.parentAlignment == closest.parentAlignment) {
            closest = candidate;
            return;
        }

    } else if (candidate.parentDistance < closest.distance) {
        if (candidate.parentAlignment >= closest.alignment) {
            closest = candidate;
            return;
        }
    }
}

#if PLATFORM(WKC)
void FocusController::findFocusableNodeInDirection(Node* outer, Node* focusedNode,
                                                   FocusDirection direction, KeyboardEvent* event,
                                                   FocusCandidate& closest, const FocusCandidate& candidateParent,
                                                   const IntRect* specificRect)

#else
void FocusController::findFocusableNodeInDirection(Node* outer, Node* focusedNode,
                                                   FocusDirection direction, KeyboardEvent* event,
                                                   FocusCandidate& closest, const FocusCandidate& candidateParent)
#endif
{
#if PLATFORM(WKC)
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);
#endif
    ASSERT(outer);
    ASSERT(candidateParent.isNull()
        || candidateParent.node->isFrameOwnerElement()
        || isScrollableContainerNode(candidateParent.node));

    // Walk all the child nodes and update closest if we find a nearer node.
    Node* node = outer;
    while (node) {

        // Inner documents case.
        if (node->isFrameOwnerElement()) {
            deepFindFocusableNodeInDirection(node, focusedNode, direction, event, closest, specificRect);
        // Scrollable block elements (e.g. <div>, etc) case.
        } else if (isScrollableContainerNode(node) && !node->renderer()->isTextArea()) {
            deepFindFocusableNodeInDirection(node, focusedNode, direction, event, closest, specificRect);
            node = node->traverseNextSibling();
            continue;

#if PLATFORM(WKC)
        } else if (node != focusedNode && node->isFocusable() && isNodeInSpecificRect(node, specificRect)) {
#else
        } else if (node != focusedNode && node->isKeyboardFocusable(event)) {
#endif
            FocusCandidate candidate(node);

            // There are two ways to identify we are in a recursive call from deepFindFocusableNodeInDirection
            // (i.e. processing an element in an iframe, frame or a scrollable block element):

            // 1) If candidateParent is not null, and it holds the distance and alignment data of the
            // parent container element itself;
            // 2) Parent of outer is <frame> or <iframe>;
            // 3) Parent is any other scrollable block element.
            if (!candidateParent.isNull()) {
                candidate.parentAlignment = candidateParent.alignment;
                candidate.parentDistance = candidateParent.distance;
                candidate.enclosingScrollableBox = candidateParent.node;

            } else if (!isInRootDocument(outer)) {
                if (Document* document = static_cast<Document*>(outer->parent()))
                    candidate.enclosingScrollableBox = static_cast<Node*>(document->ownerElement());

            } else if (isScrollableContainerNode(outer->parent()))
                candidate.enclosingScrollableBox = outer->parent();

            // Get distance and alignment from current candidate.
            distanceDataForNode(direction, focusedNode, candidate);

            // Bail out if distance is maximum.
            if (candidate.distance == maxDistance()) {
                node = node->traverseNextNode(outer->parent());
                continue;
            }

            updateFocusCandidateIfCloser(focusedNode, candidate, closest);
        }

        node = node->traverseNextNode(outer->parent());
    }
}

void FocusController::deepFindFocusableNodeInDirection(Node* container, Node* focusedNode,
                                                       FocusDirection direction, KeyboardEvent* event,
                                                       FocusCandidate& closest,
                                                       const IntRect* specificRect)
{
    ASSERT(container->isFrameOwnerElement() || isScrollableContainerNode(container));

    // Track if focusedNode is a descendant of the current container node being processed.
    bool descendantOfContainer = false;
    Node* firstChild = 0;

    if (container->isFrameOwnerElement()) {

        HTMLFrameOwnerElement* owner = static_cast<HTMLFrameOwnerElement*>(container);
        if (!owner->contentFrame())
            return;

        Document* innerDocument = owner->contentFrame()->document();
        if (!innerDocument)
            return;

        descendantOfContainer = isNodeDeepDescendantOfDocument(focusedNode, innerDocument);
        firstChild = innerDocument->firstChild();

    // Scrollable block elements (e.g. <div>, etc)
    } else if (isScrollableContainerNode(container) && !container->renderer()->isTextArea()) {

        firstChild = container->firstChild();
        descendantOfContainer = focusedNode->isDescendantOf(container);
    }

    if (descendantOfContainer) {
        findFocusableNodeInDirection(firstChild, focusedNode, direction, event, closest, FocusCandidate(), specificRect);
        return;
    }

    // Check if the current container element itself is a good candidate
    // to move focus to. If it is, then we traverse its inner nodes.
    FocusCandidate candidateParent = FocusCandidate(container);
    distanceDataForNode(direction, focusedNode, candidateParent);

    // Bail out if distance is maximum.
    if (candidateParent.distance == maxDistance())
        return;

    // FIXME: Consider alignment?
    if (candidateParent.distance < closest.distance)
        findFocusableNodeInDirection(firstChild, focusedNode, direction, event, closest, candidateParent, specificRect);
}

#if PLATFORM(WKC)
static Node*
findFirstFocusableNode(Frame* frame, const IntRect* specificRect)
{
#if PLATFORM(WKC)
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);
#endif
    Node* node = frame->document()->firstChild();
    while (node) {
        if (!isNodeInSpecificRect(node, specificRect)) {
            node = node->traverseNextNode();
            continue;
        }
        if (node->isFrameOwnerElement()) {
            HTMLFrameOwnerElement* owner = static_cast<HTMLFrameOwnerElement*>(node);
            if (owner->contentFrame()) {
                node = findFirstFocusableNode(owner->contentFrame(), specificRect);
                if (node)
                    break;
                node = owner;
            }
        } else if (isScrollableContainerNode(node) && !node->renderer()->isTextArea()) {
            node = node->firstChild();
        }
        if (node->isFocusable() && !node->isFrameOwnerElement()) {
            break;
        }
        node = node->traverseNextNode();
    }

    if (node) {
        return node->isFocusable() ? node : 0;
    }
    return 0;
}

static Node*
findLastFocusableNode(Frame* frame, const IntRect* specificRect)
{
#if PLATFORM(WKC)
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);
#endif
    Node* node = frame->document()->lastChild();
    while (node) {
        if (!isNodeInSpecificRect(node, specificRect)) {
            node = node->traversePreviousNodePostOrder();
            continue;
        }
        if (node->isFrameOwnerElement()) {
            HTMLFrameOwnerElement* owner = static_cast<HTMLFrameOwnerElement*>(node);
            if (owner->contentFrame()) {
                node = findLastFocusableNode(owner->contentFrame(), specificRect);
                if (node)
                    break;
                node = owner;
            }
        } else if (isScrollableContainerNode(node) && !node->renderer()->isTextArea()) {
            node = node->lastChild();
        }
        if (node->isFocusable() && !node->isFrameOwnerElement()) {
            break;
        }
        node = node->traversePreviousNodePostOrder();
    }

    if (node) {
        return node->isFocusable() ? node : 0;
    }
    return 0;
}

Node* FocusController::findNextFocusableNode(const FocusDirection direction, const IntRect* specificRect)
{
#if PLATFORM(WKC)
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);
#endif
    Frame* frame = focusedOrMainFrame();
    ASSERT(frame);
    Document* focusedDocument = frame->document();
    if (!focusedDocument)
        return 0;

    focusedDocument->updateLayoutIgnorePendingStylesheets();

    Node* focusedNode = focusedDocument->focusedNode();
    if (!focusedNode) {
        if (direction == FocusDirectionUp) {
            return findLastFocusableNode(m_page->mainFrame(), specificRect);
        } else {
            return findFirstFocusableNode(m_page->mainFrame(), specificRect);
        }
    }

    frame = frame->tree()->top();

    FocusCandidate focusCandidate;
    findFocusableNodeInDirection(frame->document()->firstChild(), focusedNode, direction, 0, focusCandidate, FocusCandidate(), specificRect);

    Node* node = focusCandidate.node;
    if (!node || !node->isElementNode()) {
        return 0;
    }

    if (hasOffscreenRect(node)) {
        return 0;
    }

    return node;
}

static Node*
getClosestNode(Node* node, FocusDirection direction, bool sibling = false)
{
    ASSERT(node);
    ASSERT(direction == FocusDirectionUp    ||
           direction == FocusDirectionDown  ||
           direction == FocusDirectionLeft  ||
           direction == FocusDirectionRight);

    Node* closestNode;
    switch (direction) {
    case FocusDirectionUp:
        if (sibling)
            closestNode = node->traversePreviousSiblingPostOrder();
        else
            closestNode = node->traversePreviousNodePostOrder();
        break;
    case FocusDirectionDown:
    case FocusDirectionLeft:
    case FocusDirectionRight:
        if (sibling)
            closestNode = node->traverseNextSibling();
        else
            closestNode = node->traverseNextNode();
        break;
    default:
        ASSERT(false);
        closestNode = 0; // avoid warning
        break;
    }
    return closestNode;
}

Node* FocusController::findVerticallyFocusableNodeInRect(FocusDirection direction, Node* start, KeyboardEvent* event, const IntRect* rect)
{
#if PLATFORM(WKC)
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);
#endif
    ASSERT(direction == FocusDirectionUp || direction == FocusDirectionDown);
    if (!start)
        return 0;

    Node* node = start;
    HTMLFrameOwnerElement* owner;
    Document* document;

    while (node) {
        if (!isNodeInSpecificRect(node, rect)) {
            node = getClosestNode(node, direction);
            continue;
        }

        owner = 0;
        if (node->isFrameOwnerElement()) {
            owner = static_cast<HTMLFrameOwnerElement*>(node);
            if (!owner->contentFrame()) {
                node = 0;
                break;
            }
            document = owner->contentFrame()->document();
            if (direction == FocusDirectionUp) {
                node = findVerticallyFocusableNodeInRect(direction, document->lastChild(), event, rect);
            } else {
                node = findVerticallyFocusableNodeInRect(direction, document->firstChild(), event, rect);
            }
            if (node)
                break;
            node = getClosestNode(owner, direction);
            continue;
        } else if (isScrollableContainerNode(node) && !node->renderer()->isTextArea()) {
            Node* childNode = 0;
            if (direction == FocusDirectionUp) {
                childNode = findVerticallyFocusableNodeInRect(direction, node->lastChild(), event, rect);
            } else {
                ASSERT(direction == FocusDirectionDown);
                childNode = findVerticallyFocusableNodeInRect(direction, node->firstChild(), event, rect);
            }
            if (!childNode) {
                node = getClosestNode(node, direction, true);
                if (!node)
                    break;
            } else {
                node = childNode;
                break;
            }
        }

        if (node->isFocusable() && !node->isFrameOwnerElement()) {
            break;
        }
        if (owner) {
            node = owner;
        }
        node = getClosestNode(node, direction);
    }
    return node;
}

void FocusController::findHorizontallyFocusableNodeInRect(FocusDirection direction, Node* start, KeyboardEvent* event, const IntRect* rect, Node** candidateNode, IntRect& candidateNodeRect)
{
#if PLATFORM(WKC)
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);
#endif
//    ASSERT(direction == FocusDirectionLeft || direction == FocusDirectionRight);
    if (!start)
        return ;

    Node* node = start;
    HTMLFrameOwnerElement* owner;
    Document* document;
    IntRect nodeRect;

    while (node) {
        if (!isNodeInSpecificRect(node, rect)) {
            node = getClosestNode(node, direction);
            continue;
        }
        owner = 0;
        if (node->isFrameOwnerElement()) {
            owner = static_cast<HTMLFrameOwnerElement*>(node);
            if (!owner->contentFrame()) {
                *candidateNode = 0;
                return;
            }
            document = owner->contentFrame()->document();
            findHorizontallyFocusableNodeInRect(direction, document, event, rect, candidateNode, candidateNodeRect);
        } else if (isScrollableContainerNode(node) && !node->renderer()->isTextArea()) {
            findHorizontallyFocusableNodeInRect(direction, node->firstChild(), event, rect, candidateNode, candidateNodeRect);
        } else {
            if (node->isFocusable() && !node->isFrameOwnerElement()) {
                nodeRect = node->renderer()->absoluteBoundingBoxRect();
                FrameView* frameView = node->document()->view();
                if (!frameView) {
                    *candidateNode = 0;
                    return;
                }                
                nodeRect = frameView->contentsToWindow(nodeRect);
                nodeRect.intersect(*rect);
                if (!nodeRect.isEmpty()) {
                    if (candidateNodeRect.isEmpty()) {
                        *candidateNode = node;
                        candidateNodeRect = nodeRect;
                    }
                    if (direction == FocusDirectionRight && candidateNodeRect.x() > nodeRect.x()) {
                        *candidateNode = node;
                        candidateNodeRect = nodeRect;
                    } else if (direction == FocusDirectionLeft && candidateNodeRect.x() < nodeRect.x()) {
                        *candidateNode = node;
                        candidateNodeRect = nodeRect;
                    }
                }
            }
        }
        node = getClosestNode(node, direction);
    }
}

Node* FocusController::findNextFocusableNodeInRect(const FocusDirection direction, Frame* frame, const IntRect* rect)
{
#if PLATFORM(WKC)
    CRASH_IF_STACK_OVERFLOW(WKC_STACK_MARGIN_DEFAULT);
#endif
    ASSERT(frame);
    ASSERT(rect);

    Node* start;
    if (direction == FocusDirectionUp) {
        Node* last = 0;
        for (last = frame->document()->lastChild(); last && last->lastChild(); last = last->lastChild())
            ; // Empty loop.
        start = last;
    } else {
        ASSERT(direction == FocusDirectionDown || direction == FocusDirectionLeft || direction == FocusDirectionRight);
        start = frame->document()->firstChild();
    }

    frame->document()->updateLayoutIgnorePendingStylesheets();

    Node* node = 0;
    IntRect candidateNodeRect;
    switch (direction) {
    case FocusDirectionUp:
    case FocusDirectionDown:
        node = findVerticallyFocusableNodeInRect(direction, start, 0, rect);
        break;
    case FocusDirectionRight:
    case FocusDirectionLeft:
        findHorizontallyFocusableNodeInRect(direction, start, 0, rect, &node, candidateNodeRect);
        break;
    default:
        node = 0;
        break;
    }
    return node;
}
#endif // PLATFORM(WKC)
#endif // ENABLE(WKC_SPATIAL_NAVI)

static bool relinquishesEditingFocus(Node *node)
{
    ASSERT(node);
    ASSERT(node->isContentEditable());

    Node* root = node->rootEditableElement();
    Frame* frame = node->document()->frame();
    if (!frame || !root)
        return false;

    return frame->editor()->shouldEndEditing(rangeOfContents(root).get());
}

static void clearSelectionIfNeeded(Frame* oldFocusedFrame, Frame* newFocusedFrame, Node* newFocusedNode)
{
    if (!oldFocusedFrame || !newFocusedFrame)
        return;
        
    if (oldFocusedFrame->document() != newFocusedFrame->document())
        return;
    
    SelectionController* s = oldFocusedFrame->selection();
    if (s->isNone())
        return;

    bool caretBrowsing = oldFocusedFrame->settings()->caretBrowsingEnabled();
    if (caretBrowsing)
        return;

    Node* selectionStartNode = s->selection().start().node();
    if (selectionStartNode == newFocusedNode || selectionStartNode->isDescendantOf(newFocusedNode) || selectionStartNode->shadowAncestorNode() == newFocusedNode)
        return;
        
    if (Node* mousePressNode = newFocusedFrame->eventHandler()->mousePressNode())
        if (mousePressNode->renderer() && !mousePressNode->canStartSelection())
            if (Node* root = s->rootEditableElement())
                if (Node* shadowAncestorNode = root->shadowAncestorNode())
                    // Don't do this for textareas and text fields, when they lose focus their selections should be cleared
                    // and then restored when they regain focus, to match other browsers.
                    if (!shadowAncestorNode->hasTagName(inputTag) && !shadowAncestorNode->hasTagName(textareaTag))
                        return;
    
    s->clear();
}

bool FocusController::setFocusedNode(Node* node, PassRefPtr<Frame> newFocusedFrame)
{
    RefPtr<Frame> oldFocusedFrame = focusedFrame();
    RefPtr<Document> oldDocument = oldFocusedFrame ? oldFocusedFrame->document() : 0;
    
    Node* oldFocusedNode = oldDocument ? oldDocument->focusedNode() : 0;
    if (oldFocusedNode == node)
        return true;

    // FIXME: Might want to disable this check for caretBrowsing
    if (oldFocusedNode && oldFocusedNode->rootEditableElement() == oldFocusedNode && !relinquishesEditingFocus(oldFocusedNode))
        return false;

#if ENABLE(WKC_SPATIAL_NAVI)
    // Set input method state before changing the focused node, so that the
    // input method can still have a chance to finish the ongoing composition
    // session.
    m_page->editorClient()->setInputMethodState(node ? node->shouldUseInputMethod() : false);
#endif
    clearSelectionIfNeeded(oldFocusedFrame.get(), newFocusedFrame.get(), node);
    
    if (!node) {
        if (oldDocument)
            oldDocument->setFocusedNode(0);
#if ENABLE(WKC_SPATIAL_NAVI)

#else
        m_page->editorClient()->setInputMethodState(false);
#endif
        return true;
    }

    RefPtr<Document> newDocument = node->document();

    if (newDocument && newDocument->focusedNode() == node) {
#if ENABLE(WKC_SPATIAL_NAVI)

#else
        m_page->editorClient()->setInputMethodState(node->shouldUseInputMethod());
#endif
        return true;
    }
    
    if (oldDocument && oldDocument != newDocument)
        oldDocument->setFocusedNode(0);
    
    setFocusedFrame(newFocusedFrame);
#if 1
    // added at webkit.org trunk r65748
    // Setting the focused node can result in losing our last reft to node when JS event handlers fire.
    RefPtr<Node> protect = node;
#endif
    if (newDocument)
        newDocument->setFocusedNode(node);
#if ENABLE(WKC_SPATIAL_NAVI)
    // do nothing
#else
#if 1
    // modified at webkit.org trunk r65748
    if (newDocument->focusedNode() == node)
        m_page->editorClient()->setInputMethodState(node->shouldUseInputMethod());
#else
    m_page->editorClient()->setInputMethodState(node->shouldUseInputMethod());
#endif
#endif

    return true;
}

void FocusController::setActive(bool active)
{
    if (m_isActive == active)
        return;

    m_isActive = active;

    if (FrameView* view = m_page->mainFrame()->view()) {
        if (!view->platformWidget()) {
            view->layoutIfNeededRecursive();
            view->updateControlTints();
        }
    }

    focusedOrMainFrame()->selection()->pageActivationChanged();
    
    if (m_focusedFrame && isFocused())
        dispatchEventsOnWindowAndFocusedNode(m_focusedFrame->document(), active);
}

} // namespace WebCore
