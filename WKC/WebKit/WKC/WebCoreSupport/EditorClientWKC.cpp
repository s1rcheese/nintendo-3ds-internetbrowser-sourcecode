/*
 *  Copyright (C) 2007 Alp Toker <alp@atoker.com>
 *  Copyright (C) 2008 Nuanti Ltd.
 *  Copyright (C) 2009 Diego Escalante Urrelo <diegoe@gnome.org>
 *  Copyright (C) 2006, 2007 Apple Inc.  All rights reserved.
 *  Copyright (C) 2009, Igalia S.L.
 *  Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include "CSSStyleDeclaration.h"
#include "EditorClientWKC.h"
#include "EditCommand.h"
#include "Element.h"
#include "HTMLElement.h"
#include "KeyboardEvent.h"
#include "Node.h"
#include "PlatformKeyboardEvent.h"
#include "Range.h"
#include "PlatformString.h"

#include "WKCWebViewPrivate.h"
#include "WKCPlatformEvents.h"

#include "helpers/EditorClientIf.h"
#include "helpers/WKCString.h"

#include "helpers/privates/WKCCSSStyleDeclarationPrivate.h"
#include "helpers/privates/WKCEditCommandPrivate.h"
#include "helpers/privates/WKCElementPrivate.h"
#include "helpers/privates/WKCHTMLElementPrivate.h"
#include "helpers/privates/WKCNodePrivate.h"
#include "helpers/privates/WKCRangePrivate.h"

// implementations

#define WKCPOBJ(a) ((a) ? ((a)->platformObj()) : 0)

namespace WKC {

EditorClientWKC::EditorClientWKC(WKCWebViewPrivate* view)
     : m_view(view),
       m_appClient(0)
{
}

EditorClientWKC::~EditorClientWKC()
{
    if (m_appClient) {
        m_view->clientBuilders().deleteEditorClient(m_appClient);
        m_appClient = 0;
    }
}

EditorClientWKC*
EditorClientWKC::create(WKCWebViewPrivate* view)
{
    EditorClientWKC* self = 0;
    self = new EditorClientWKC(view);
    if (!self) return 0;
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
EditorClientWKC::construct()
{
    m_appClient = m_view->clientBuilders().createEditorClient(m_view->parent());
    if (!m_appClient) return false;
    return true;
}

void
EditorClientWKC::pageDestroyed()
{
    delete this;
}


bool
EditorClientWKC::shouldDeleteRange(WebCore::Range* range)
{
    RangePrivate r(range);
    return m_appClient->shouldDeleteRange(&r.wkc());
}

bool
EditorClientWKC::shouldShowDeleteInterface(WebCore::HTMLElement* element)
{
    HTMLElementPrivate e(element);
    return m_appClient->shouldShowDeleteInterface(&e.wkc());
}

bool
EditorClientWKC::smartInsertDeleteEnabled()
{
    return m_appClient->smartInsertDeleteEnabled();
}

bool
EditorClientWKC::isSelectTrailingWhitespaceEnabled()
{
    return m_appClient->isSelectTrailingWhitespaceEnabled();
}

bool
EditorClientWKC::isContinuousSpellCheckingEnabled()
{
    return m_appClient->isContinuousSpellCheckingEnabled();
}

void
EditorClientWKC::toggleContinuousSpellChecking()
{
    m_appClient->toggleContinuousSpellChecking();
}

bool
EditorClientWKC::isGrammarCheckingEnabled()
{
    return m_appClient->isGrammarCheckingEnabled();
}

void
EditorClientWKC::toggleGrammarChecking()
{
    m_appClient->toggleGrammarChecking();
}

int
EditorClientWKC::spellCheckerDocumentTag()
{
    return m_appClient->spellCheckerDocumentTag();
}


bool
EditorClientWKC::isEditable()
{
    return m_appClient->isEditable();
}


bool
EditorClientWKC::shouldBeginEditing(WebCore::Range* range)
{
    RangePrivate r(range);
    return m_appClient->shouldBeginEditing(&r.wkc());
}

bool
EditorClientWKC::shouldEndEditing(WebCore::Range* range)
{
    RangePrivate r(range);
    return m_appClient->shouldEndEditing(&r.wkc());
}

bool
EditorClientWKC::shouldInsertNode(WebCore::Node* node, WebCore::Range* range, WebCore::EditorInsertAction action)
{
    RangePrivate r(range);

    NodePrivate* n = 0;
    if (node) {
        n = NodePrivate::create(node);
    }
    bool ret = m_appClient->shouldInsertNode(&n->wkc(), &r.wkc(), (WKC::EditorInsertAction)action);
    delete n;
    return ret;
}

bool
EditorClientWKC::shouldInsertText(const WebCore::String& string, WebCore::Range* range, WebCore::EditorInsertAction action)
{
    RangePrivate r(range);
    return m_appClient->shouldInsertText(string, &r.wkc(), (WKC::EditorInsertAction)action);
}

bool
EditorClientWKC::shouldChangeSelectedRange(WebCore::Range* fromRange, WebCore::Range* toRange, WebCore::EAffinity affinity, bool stillSelecting)
{
    RangePrivate fr(fromRange);
    RangePrivate tr(toRange);
    return m_appClient->shouldChangeSelectedRange(&fr.wkc(), &tr.wkc(), (WKC::EAffinity)affinity, stillSelecting);
}


bool
EditorClientWKC::shouldApplyStyle(WebCore::CSSStyleDeclaration* decl, WebCore::Range* range)
{
    RangePrivate r(range);
    CSSStyleDeclarationPrivate c(decl);
    return m_appClient->shouldApplyStyle(&c.wkc(), &r.wkc());
}


bool
EditorClientWKC::shouldMoveRangeAfterDelete(WebCore::Range* range1, WebCore::Range* range2)
{
    RangePrivate r1(range1);
    RangePrivate r2(range2);
    return m_appClient->shouldMoveRangeAfterDelete(&r1.wkc(), &r2.wkc());
}


void
EditorClientWKC::didBeginEditing()
{
    m_appClient->didBeginEditing();
}

void
EditorClientWKC::respondToChangedContents()
{
    m_appClient->respondToChangedContents();
}

void
EditorClientWKC::respondToChangedSelection()
{
    m_appClient->respondToChangedSelection();
}

void
EditorClientWKC::didEndEditing()
{
    m_appClient->didEndEditing();
}

void
EditorClientWKC::didWriteSelectionToPasteboard()
{
    m_appClient->didWriteSelectionToPasteboard();
}

void
EditorClientWKC::didSetSelectionTypesForPasteboard()
{
    m_appClient->didSetSelectionTypesForPasteboard();
}


void
EditorClientWKC::registerCommandForUndo(WTF::PassRefPtr<WebCore::EditCommand> command)
{
    EditCommandPrivate e(command.get());
    m_appClient->registerCommandForUndo(&e.wkc());
}

void
EditorClientWKC::registerCommandForRedo(WTF::PassRefPtr<WebCore::EditCommand> command)
{
    EditCommandPrivate e(command.get());
    m_appClient->registerCommandForRedo(&e.wkc());
}

void
EditorClientWKC::clearUndoRedoOperations()
{
    m_appClient->clearUndoRedoOperations();
}


bool
EditorClientWKC::canUndo() const
{
    return m_appClient->canUndo();
}

bool
EditorClientWKC::canRedo() const
{
    return m_appClient->canRedo();
}


void
EditorClientWKC::undo()
{
    m_appClient->undo();
}

void
EditorClientWKC::redo()
{
    m_appClient->redo();
}

static bool
_platformKeyEvent(WebCore::KeyboardEvent* event, WKC::WKCKeyEvent& ev)
{
    const WebCore::PlatformKeyboardEvent* pe = 0;
    unsigned int mod = 0;
    unsigned int pmod = 0;

    if (!event)
        return false;
    pe = event->keyEvent();
    if (!pe)
        return false;

    switch (pe->type()) {
    case WebCore::PlatformKeyboardEvent::KeyDown:
        ev.m_type = EKeyEventPressed;
        break;
    case WebCore::PlatformKeyboardEvent::KeyUp:
        ev.m_type = EKeyEventReleased;
        break;
    default:
        return false;
    }
    mod = pe->modifiers();
    pmod = WKC::EModifierNone;
    if (mod&WebCore::PlatformKeyboardEvent::AltKey)
        pmod |= WKC::EModifierAlt;
    if (mod&WebCore::PlatformKeyboardEvent::CtrlKey)
        pmod |= WKC::EModifierCtrl;
    if (mod&WebCore::PlatformKeyboardEvent::ShiftKey)
        pmod |= WKC::EModifierShift;
    if (mod&WebCore::PlatformKeyboardEvent::MetaKey)
        pmod |= WKC::EModifierMod1;
    ev.m_modifiers = (WKC::Modifier)pmod;
    return true;
}

void
EditorClientWKC::handleKeyboardEvent(WebCore::KeyboardEvent* event)
{
    WKC::WKCKeyEvent ev;
    if (_platformKeyEvent(event, ev)) {
        m_appClient->handleKeyboardEvent(&ev);
    } else {
        m_appClient->handleKeyboardEvent(0);
    }
}

void
EditorClientWKC::handleInputMethodKeydown(WebCore::KeyboardEvent* event)
{
    WKC::WKCKeyEvent ev;
    if (_platformKeyEvent(event, ev)) {
        m_appClient->handleInputMethodKeydown(&ev);
    } else {
        m_appClient->handleInputMethodKeydown(0);
    }
}


void
EditorClientWKC::textFieldDidBeginEditing(WebCore::Element* element)
{
    ElementPrivate e(element);
    m_appClient->textFieldDidBeginEditing(&e.wkc());
}

void
EditorClientWKC::textFieldDidEndEditing(WebCore::Element* element)
{
    ElementPrivate e(element);
    m_appClient->textFieldDidEndEditing(&e.wkc());
}

void
EditorClientWKC::textDidChangeInTextField(WebCore::Element* element)
{
    ElementPrivate e(element);
    m_appClient->textDidChangeInTextField(&e.wkc());
}

bool
EditorClientWKC::doTextFieldCommandFromEvent(WebCore::Element* element, WebCore::KeyboardEvent* event)
{
    ElementPrivate e(element);
    WKC::WKCKeyEvent ev;
    if (_platformKeyEvent(event, ev)) {
        return m_appClient->doTextFieldCommandFromEvent(&e.wkc(), &ev);
    } else {
        return m_appClient->doTextFieldCommandFromEvent(&e.wkc(), 0);
    }
}

void
EditorClientWKC::textWillBeDeletedInTextField(WebCore::Element* element)
{
    ElementPrivate e(element);
    m_appClient->textWillBeDeletedInTextField(&e.wkc());
}

void
EditorClientWKC::textDidChangeInTextArea(WebCore::Element* element)
{
    ElementPrivate e(element);
    m_appClient->textDidChangeInTextArea(&e.wkc());
}


void
EditorClientWKC::ignoreWordInSpellDocument(const WebCore::String& string)
{
    m_appClient->ignoreWordInSpellDocument(string);
}

void
EditorClientWKC::learnWord(const WebCore::String& string)
{
    m_appClient->learnWord(string);
}

void
EditorClientWKC::checkSpellingOfString(const UChar* chars, int length, int* misspellingLocation, int* misspellingLength)
{
    m_appClient->checkSpellingOfString(chars, length, misspellingLocation, misspellingLength);
}

WebCore::String
EditorClientWKC::getAutoCorrectSuggestionForMisspelledWord(const WebCore::String& string)
{
    return m_appClient->getAutoCorrectSuggestionForMisspelledWord(string);
}

void
EditorClientWKC::checkGrammarOfString(const UChar* str, int length, WTF::Vector<WebCore::GrammarDetail>& detail, int* badGrammarLocation, int* badGrammarLength)
{
    // Ugh!: support this feature!
    // 110128 ACCESS Co.,Ltd.
//    m_appClient->checkGrammarOfString(str, length, detail, badGrammarLocation, badGrammarLength);
}

void
EditorClientWKC::updateSpellingUIWithGrammarString(const WebCore::String& string, const WebCore::GrammarDetail& detail)
{
    // Ugh!: support this feature!
    // 110128 ACCESS Co.,Ltd.
//    m_appClient->updateSpellingUIWithGrammarString(string, detail);
}

void
EditorClientWKC::updateSpellingUIWithMisspelledWord(const WebCore::String& string)
{
    m_appClient->updateSpellingUIWithMisspelledWord(string);
}

void
EditorClientWKC::showSpellingUI(bool show)
{
    m_appClient->showSpellingUI(show);
}

bool
EditorClientWKC::spellingUIIsShowing()
{
    return m_appClient->spellingUIIsShowing();
}

void
EditorClientWKC::getGuessesForWord(const WebCore::String& string, WTF::Vector<WebCore::String>& guesses)
{
    // Ugh!: support this feature!
    // 110128 ACCESS Co.,Ltd.
//    m_appClient->getGuessesForWord(string, guesses);
}

void
EditorClientWKC::setInputMethodState(bool enabled)
{
    m_appClient->setInputMethodState(enabled);
}

} // namespace

