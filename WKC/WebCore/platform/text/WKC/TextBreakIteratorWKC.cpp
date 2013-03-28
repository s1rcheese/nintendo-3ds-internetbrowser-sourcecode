/*
 * Copyright (C) 2006 Lars Knoll <lars@trolltech.com>
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Jurg Billeter <j@bitron.ch>
 * Copyright (C) 2008 Dominik Rottsches <dominik.roettsches@access-company.com>
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
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
#include "TextBreakIterator.h"

#include <wkc/wkcpeer.h>

namespace WebCore {

static void* gIteratorCharacter = NULL;
static void* gIteratorWord = NULL;
static void* gIteratorLine = NULL;
static void* gIteratorSentence = NULL;
static void* gIteratorCursorMovement = NULL;

void
finalizeTextBreakIterators()
{
   if (gIteratorCharacter) wkcTextBreakIteratorDeletePeer(gIteratorCharacter);
   if (gIteratorWord) wkcTextBreakIteratorDeletePeer(gIteratorWord);
   if (gIteratorLine) wkcTextBreakIteratorDeletePeer(gIteratorLine);
   if (gIteratorSentence) wkcTextBreakIteratorDeletePeer(gIteratorSentence);
   if (gIteratorCursorMovement) wkcTextBreakIteratorDeletePeer(gIteratorCursorMovement);

   gIteratorCharacter = gIteratorLine = gIteratorSentence = gIteratorCursorMovement = NULL;
}

bool
initializeTextBreakIterators()
{
   gIteratorCharacter = wkcTextBreakIteratorNewPeer(WKC_TEXTBREAKITERATOR_TYPE_CHARACTER);
   if (!gIteratorCharacter) goto error_end;
   gIteratorWord = wkcTextBreakIteratorNewPeer(WKC_TEXTBREAKITERATOR_TYPE_WORD);
   if (!gIteratorWord) goto error_end;
   gIteratorLine = wkcTextBreakIteratorNewPeer(WKC_TEXTBREAKITERATOR_TYPE_LINE);
   if (!gIteratorLine) goto error_end;
   gIteratorSentence = wkcTextBreakIteratorNewPeer(WKC_TEXTBREAKITERATOR_TYPE_SENTENCE);
   if (!gIteratorSentence) goto error_end;
   gIteratorCursorMovement = wkcTextBreakIteratorNewPeer(WKC_TEXTBREAKITERATOR_TYPE_CURSORMOVEMENT);
   if (!gIteratorCursorMovement) goto error_end;

   return true;

error_end:
   finalizeTextBreakIterators();
   return false;
}


TextBreakIterator*
characterBreakIterator(const UChar* str, int length)
{
    if (!str || !length) return 0;

    if (!wkcTextBreakIteratorSetStringPeer(gIteratorCharacter, str, length)) {
        return 0;
    }
    return (TextBreakIterator *)gIteratorCharacter;
}

TextBreakIterator*
cursorMovementIterator(const UChar* str, int length)
{
    if (!str || !length) return 0;

    if (!wkcTextBreakIteratorSetStringPeer(gIteratorCursorMovement, str, length)) {
        return 0;
    }
    return (TextBreakIterator *)gIteratorCursorMovement;
}

TextBreakIterator*
wordBreakIterator(const UChar* str, int length)
{
    if (!str || !length) return 0;

    if (!wkcTextBreakIteratorSetStringPeer(gIteratorWord, str, length)) {
        return 0;
    }
    return (TextBreakIterator *)gIteratorWord;
}

TextBreakIterator*
lineBreakIterator(const UChar* str, int length)
{
    if (!str || !length) return 0;

    if (!wkcTextBreakIteratorSetStringPeer(gIteratorLine, str, length)) {
        return 0;
    }
    return (TextBreakIterator *)gIteratorLine;
}

TextBreakIterator*
sentenceBreakIterator(const UChar*str, int length)
{
    if (!str || !length) return 0;

    if (!wkcTextBreakIteratorSetStringPeer(gIteratorSentence, str, length)) {
        return 0;
    }
    return (TextBreakIterator *)gIteratorSentence;
}

int
textBreakFirst(TextBreakIterator* self)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorFirstPeer((void *)self);
    if (ret>=0) return ret;
    return TextBreakDone;
}

int
textBreakLast(TextBreakIterator* self)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorLastPeer((void *)self);
    if (ret>=0) return ret;
    return TextBreakDone;
}

int
textBreakNext(TextBreakIterator* self)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorNextPeer((void *)self);
    if (ret>=0) return ret;
    return TextBreakDone;
}

int
textBreakPrevious(TextBreakIterator* self)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorPreviousPeer((void *)self);
    if (ret>=0) return ret;
    return TextBreakDone;
}

int
textBreakCurrent(TextBreakIterator* self)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorCurrentPeer((void *)self);
    if (ret>=0) return ret;
    return TextBreakDone;
}

int
textBreakPreceding(TextBreakIterator* self, int pos)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorPrecedingPeer((void *)self, pos);
    if (ret>=0) return ret;
    return TextBreakDone;
}

int
textBreakFollowing(TextBreakIterator* self, int pos)
{
    if (!self) return TextBreakDone;

    int ret = wkcTextBreakIteratorFollowingPeer((void *)self, pos);
    if (ret>=0) return ret;
    return TextBreakDone;
}

bool
isTextBreak(TextBreakIterator* self, int pos)
{
    if (!self) return true;

    return wkcTextBreakIteratorIsTextBreakPeer((void *)self, pos);
}

#if PLATFORM(WKC)
void TextBreakIterator_resetVariables()
{
    gIteratorCharacter = NULL;
    gIteratorWord = NULL;
    gIteratorLine = NULL;
    gIteratorSentence = NULL;
    gIteratorCursorMovement = NULL;
}
#endif


}
