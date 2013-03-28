/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Apple Computer, Inc.
 * All rights reserved.
 * Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.
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

#ifndef WKCEditorClient_h
#define WKCEditorClient_h

#include <wkc/wkcbase.h>
#include "WKCHelpersEnums.h"

namespace WKC {

class Range;
class HTMLElement;
class Node;
class Page;
class String;
class CSSStyleDeclaration;
class EditCommand;
class Element;
struct GrammarDetail;
typedef struct WKCKeyEvent_ WKCKeyEvent;

class EditorClientIf {
public:
   // from EditorClient
   virtual void pageDestroyed() = 0;

   virtual bool shouldDeleteRange(WKC::Range*) = 0;
   virtual bool shouldShowDeleteInterface(WKC::HTMLElement*) = 0;
   virtual bool smartInsertDeleteEnabled() = 0;
   virtual bool isSelectTrailingWhitespaceEnabled() = 0;
   virtual bool isContinuousSpellCheckingEnabled() = 0;
   virtual void toggleContinuousSpellChecking() = 0;
   virtual bool isGrammarCheckingEnabled() = 0;
   virtual void toggleGrammarChecking() = 0;
   virtual int spellCheckerDocumentTag() = 0;

   virtual bool isEditable() = 0;

   virtual bool shouldBeginEditing(WKC::Range*) = 0;
   virtual bool shouldEndEditing(WKC::Range*) = 0;
   virtual bool shouldInsertNode(WKC::Node*, WKC::Range*, WKC::EditorInsertAction) = 0;
   virtual bool shouldInsertText(const WKC::String&, WKC::Range*, WKC::EditorInsertAction) = 0;
   virtual bool shouldChangeSelectedRange(WKC::Range* fromRange, WKC::Range* toRange, WKC::EAffinity, bool stillSelecting) = 0;

   virtual bool shouldApplyStyle(WKC::CSSStyleDeclaration*, WKC::Range*) = 0;

   virtual bool shouldMoveRangeAfterDelete(WKC::Range*, WKC::Range*) = 0;

   virtual void didBeginEditing() = 0;
   virtual void respondToChangedContents() = 0;
   virtual void respondToChangedSelection() = 0;
   virtual void didEndEditing() = 0;
   virtual void didWriteSelectionToPasteboard() = 0;
   virtual void didSetSelectionTypesForPasteboard() = 0;

   virtual void registerCommandForUndo(WKC::EditCommand*) = 0;
   virtual void registerCommandForRedo(WKC::EditCommand*) = 0;
   virtual void clearUndoRedoOperations() = 0;

   virtual bool canUndo() const = 0;
   virtual bool canRedo() const = 0;

   virtual void undo() = 0;
   virtual void redo() = 0;

   virtual void handleKeyboardEvent(WKC::WKCKeyEvent*) = 0;
   virtual void handleInputMethodKeydown(WKC::WKCKeyEvent*) = 0;

   virtual void textFieldDidBeginEditing(WKC::Element*) = 0;
   virtual void textFieldDidEndEditing(WKC::Element*) = 0;
   virtual void textDidChangeInTextField(WKC::Element*) = 0;
   virtual bool doTextFieldCommandFromEvent(WKC::Element*, WKC::WKCKeyEvent*) = 0;
   virtual void textWillBeDeletedInTextField(WKC::Element*) = 0;
   virtual void textDidChangeInTextArea(WKC::Element*) = 0;

   virtual void ignoreWordInSpellDocument(const WKC::String&) = 0;
   virtual void learnWord(const WKC::String&) = 0;
   virtual void checkSpellingOfString(const unsigned short*, int length, int* misspellingLocation, int* misspellingLength) = 0;
   virtual WKC::String getAutoCorrectSuggestionForMisspelledWord(const WKC::String&) = 0;
   virtual void checkGrammarOfString(const unsigned short*, int length, WKC::GrammarDetail**, int* badGrammarLocation, int* badGrammarLength) = 0;
   virtual void updateSpellingUIWithGrammarString(const WKC::String&, const WKC::GrammarDetail&) = 0;
   virtual void updateSpellingUIWithMisspelledWord(const WKC::String&) = 0;
   virtual void showSpellingUI(bool show) = 0;
   virtual bool spellingUIIsShowing() = 0;
   virtual void getGuessesForWord(const WKC::String&, WKC::String&) = 0;
   virtual void setInputMethodState(bool enabled) = 0;
};

} // namespace

#endif // WKCEditorClient_h
