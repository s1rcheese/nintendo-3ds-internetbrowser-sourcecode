/*
 *  Copyright (C) 2006 George Staikos <staikos@kde.org>
 *  Copyright (C) 2006 Alexey Proskuryakov <ap@nypop.com>
 *  Copyright (C) 2007 Apple Computer, Inc. All rights reserved.
 *  Copyright (C) 2008 Jurg Billeter <j@bitron.ch>
 *  Copyright (C) 2008 Dominik Rottsches <dominik.roettsches@access-company.com>
 *  Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef UnicodeWKC_h
#define UnicodeWKC_h

#include "UnicodeMacrosFromICU.h"
#include <stdint.h>
#include <wkc/wkcpeer.h>

typedef uint16_t UChar;
typedef int32_t UChar32;

namespace WTF {
namespace Unicode {

enum Direction {
    LeftToRight,
    RightToLeft,
    EuropeanNumber,
    EuropeanNumberSeparator,
    EuropeanNumberTerminator,
    ArabicNumber,
    CommonNumberSeparator,
    BlockSeparator,
    SegmentSeparator,
    WhiteSpaceNeutral,
    OtherNeutral,
    LeftToRightEmbedding,
    LeftToRightOverride,
    RightToLeftArabic,
    RightToLeftEmbedding,
    RightToLeftOverride,
    PopDirectionalFormat,
    NonSpacingMark,
    BoundaryNeutral
};

enum DecompositionType {
    DecompositionNone,
    DecompositionCanonical,
    DecompositionCompat,
    DecompositionCircle,
    DecompositionFinal,
    DecompositionFont,
    DecompositionFraction,
    DecompositionInitial,
    DecompositionIsolated,
    DecompositionMedial,
    DecompositionNarrow,
    DecompositionNoBreak,
    DecompositionSmall,
    DecompositionSquare,
    DecompositionSub,
    DecompositionSuper,
    DecompositionVertical,
    DecompositionWide,
};

enum CharCategory {
    NoCategory = WKC_UNICODE_CATEGORY_NOCATEGORY,
    Other_NotAssigned = WKC_UNICODE_CATEGORY_OTHERNOTASSIGNED,
    Letter_Uppercase = WKC_UNICODE_CATEGORY_LETTERUPPERCASE,
    Letter_Lowercase = WKC_UNICODE_CATEGORY_LETTERLOWERCASE,
    Letter_Titlecase = WKC_UNICODE_CATEGORY_LETTERTITLECASE,
    Letter_Modifier = WKC_UNICODE_CATEGORY_LETTERMODIFIER,
    Letter_Other = WKC_UNICODE_CATEGORY_LETTEROTHER,

    Mark_NonSpacing = WKC_UNICODE_CATEGORY_MARKNONSPACING,
    Mark_Enclosing = WKC_UNICODE_CATEGORY_MARKENCLOSING,
    Mark_SpacingCombining = WKC_UNICODE_CATEGORY_MARKSPACINGCOMBINING,

    Number_DecimalDigit = WKC_UNICODE_CATEGORY_NUMBERDECIMALDIGIT,
    Number_Letter = WKC_UNICODE_CATEGORY_NUMBERLETTER,
    Number_Other = WKC_UNICODE_CATEGORY_NUMBEROTHER,

    Separator_Space = WKC_UNICODE_CATEGORY_SEPARATORSPACE,
    Separator_Line = WKC_UNICODE_CATEGORY_SEPARATORLINE,
    Separator_Paragraph = WKC_UNICODE_CATEGORY_SEPARATORPARAGRAPH,

    Other_Control = WKC_UNICODE_CATEGORY_OTHERCONTROL,
    Other_Format = WKC_UNICODE_CATEGORY_OTHERFORMAT,
    Other_PrivateUse = WKC_UNICODE_CATEGORY_OTHERPRIVATEUSE,
    Other_Surrogate = WKC_UNICODE_CATEGORY_OTHERSURROGATE,

    Punctuation_Dash = WKC_UNICODE_CATEGORY_PUNCTUATIONDASH,
    Punctuation_Open = WKC_UNICODE_CATEGORY_PUNCTUATIONOPEN,
    Punctuation_Close = WKC_UNICODE_CATEGORY_PUNCTUATIONCLOSE,
    Punctuation_Connector = WKC_UNICODE_CATEGORY_PUNCTUATIONCONNECTOR,
    Punctuation_Other = WKC_UNICODE_CATEGORY_PUNCTUATIONOTHER,

    Symbol_Math = WKC_UNICODE_CATEGORY_SYMBOLMATH,
    Symbol_Currency = WKC_UNICODE_CATEGORY_SYMBOLCURRENCY,
    Symbol_Modifier = WKC_UNICODE_CATEGORY_SYMBOLMODIFIER,
    Symbol_Other = WKC_UNICODE_CATEGORY_SYMBOLOTHER,

    Punctuation_InitialQuote = WKC_UNICODE_CATEGORY_PUNCTUATIONINITIALQUOTE,
    Punctuation_FinalQuote = WKC_UNICODE_CATEGORY_PUNCTUATIONFINALQUOTE,
};

UChar32 foldCase(UChar32);

int foldCase(UChar* result, int resultLength, const UChar* src, int srcLength, bool* error);

int toLower(UChar* result, int resultLength, const UChar* src, int srcLength, bool* error);

inline UChar32 toLower(UChar32 c)
{
    return wkcUnicodeToLowerPeer(c);
}

inline UChar32 toUpper(UChar32 c)
{
    return wkcUnicodeToUpperPeer(c);
}

int toUpper(UChar* result, int resultLength, const UChar* src, int srcLength, bool* error);

inline UChar32 toTitleCase(UChar32 c)
{
    return wkcUnicodeToTitlePeer(c);
}

inline bool isArabicChar(UChar32 c)
{
    return c >= 0x0600 && c <= 0x06FF;
}

inline bool isAlphanumeric(UChar32 c)
{
    return (wkcUnicodeIsAlnumPeer(c)!=0);
}

inline bool isFormatChar(UChar32 c)
{
    return ((wkcUnicodeCategoryPeer(c) & WKC_UNICODE_CATEGORY_OTHERFORMAT) ? true : false);
}

inline bool isSeparatorSpace(UChar32 c)
{
    return ((wkcUnicodeCategoryPeer(c) & WKC_UNICODE_CATEGORY_SEPARATORSPACE) ? true : false);
}

inline bool isPrintableChar(UChar32 c)
{
    return (wkcUnicodeIsPrintPeer(c)!=0);
}

inline bool isDigit(UChar32 c)
{
    return (wkcUnicodeIsDigitPeer(c)!=0);
}

inline bool isPunct(UChar32 c)
{
    return (wkcUnicodeIsPunctPeer(c)!=0);
}

inline bool hasLineBreakingPropertyComplexContext(UChar32 c)
{
    // FIXME
    return false;
}

inline bool hasLineBreakingPropertyComplexContextOrIdeographic(UChar32 c)
{
    // FIXME
    return false;
}

inline UChar32 mirroredChar(UChar32 c)
{
    UChar32 mirror = 0;
    if (wkcUnicodeGetMirrorCharPeer(c, (int *)&mirror))
        return mirror;
    return c;
}

inline CharCategory category(UChar32 c)
{
    if (c > 0xffff)
        return NoCategory;

    return (CharCategory)wkcUnicodeCategoryPeer(c);
}

Direction direction(UChar32);

inline bool isLower(UChar32 c)
{
    return (wkcUnicodeIsLowerPeer(c)!=0);
}

inline int digitValue(UChar32 c)
{
    return wkcUnicodeDigitValuePeer(c);
}

inline uint8_t combiningClass(UChar32 c)
{
    // FIXME
    // return g_unichar_combining_class(c);
    return 0;
}

inline DecompositionType decompositionType(UChar32 c)
{
    // FIXME
    return DecompositionNone;
}

int umemcasecmp(const UChar*, const UChar*, int len);

}
}

#endif

