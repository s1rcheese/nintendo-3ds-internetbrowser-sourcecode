/*
 *  Copyright (C) 2008 Jurg Billeter <j@bitron.ch>
 *  Copyright (C) 2008 Dominik Rottsches <dominik.roettsches@access-company.com>
 *  Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
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

#include "config.h"
#include <stdint.h>
#include "UnicodeWKC.h"

namespace WTF {
namespace Unicode {

UChar32 foldCase(UChar32 ch)
{
    return wkcUnicodeFoldCasePeer(ch);
}

int foldCase(UChar* result, int resultLength, const UChar* src, int srcLength, bool* error)
{
    int i, j;
    UChar32 chars[4];
    int ret;
    int len;

    len = 0;
    *error = false;
    for (i=0; i<srcLength; i++) {
        ret = wkcUnicodeFoldCaseFullPeer(src[i], chars);
        for (j=0; j<ret; j++) {
            if (result && resultLength>=0) {
                result[len] = chars[j];
                resultLength--;
            } else {
                *error = true;
            }
            len++;
        }
    }
    return len;
}

int toLower(UChar* result, int resultLength, const UChar* src, int srcLength, bool* error)
{
    int i=0;
    *error = false;
    for (i=0; i<srcLength; i++) {
        if (result && resultLength>=0) {
            result[i] = wkcUnicodeToLowerPeer(src[i]);
            resultLength--;
        } else {
            *error = true;
        }
    }
    return srcLength;
}

int toUpper(UChar* result, int resultLength, const UChar* src, int srcLength, bool* error)
{
    int i=0;
    *error = false;
    for (i=0; i<srcLength; i++) {
        if (result && resultLength>=0) {
            result[i] = wkcUnicodeToUpperPeer(src[i]);
            resultLength--;
        } else {
            *error = true;
        }
    }
    return srcLength;
}

Direction direction(UChar32 c)
{
    int type = wkcUnicodeDirectionTypePeer(c);
    switch (type) {
    case WKC_UNICODE_DIRECTION_LEFTTORIGHT:
        return LeftToRight;
    case WKC_UNICODE_DIRECTION_RIGHTTOLEFT:
        return RightToLeft;
    case WKC_UNICODE_DIRECTION_RIGHTTOLEFTARABIC:
        return RightToLeftArabic;
    case WKC_UNICODE_DIRECTION_LEFTTORIGHTEMBEDDING:
        return LeftToRightEmbedding;
    case WKC_UNICODE_DIRECTION_RIGHTTOLEFTEMBEDDING:
        return RightToLeftEmbedding;
    case WKC_UNICODE_DIRECTION_LEFTTORIGHTOVERRIDE:
        return LeftToRightOverride;
    case WKC_UNICODE_DIRECTION_RIGHTTOLEFTOVERRIDE:
        return RightToLeftOverride;
    case WKC_UNICODE_DIRECTION_POPDIRECTIONALFORMAT:
        return PopDirectionalFormat;
    case WKC_UNICODE_DIRECTION_EUROPEANNUMBER:
        return EuropeanNumber;
    case WKC_UNICODE_DIRECTION_ARABICNUMBER:
        return ArabicNumber;
    case WKC_UNICODE_DIRECTION_EUROPEANNUMBERSEPARATOR:
        return EuropeanNumberSeparator;
    case WKC_UNICODE_DIRECTION_EUROPEANNUMBERTERMINATOR:
        return EuropeanNumberTerminator;
    case WKC_UNICODE_DIRECTION_COMMONNUMBERSEPARATOR:
        return CommonNumberSeparator;
    case WKC_UNICODE_DIRECTION_NONSPACINGMARK:
        return NonSpacingMark;
    case WKC_UNICODE_DIRECTION_BOUNDARYNEUTRAL:
        return BoundaryNeutral;
    case WKC_UNICODE_DIRECTION_BLOCKSEPARATOR:
        return BlockSeparator;
    case WKC_UNICODE_DIRECTION_SEGMENTSEPARATOR:
        return SegmentSeparator;
    case WKC_UNICODE_DIRECTION_WHITESPACENEUTRAL:
        return WhiteSpaceNeutral;
    default:
        return OtherNeutral;
    }
}

int umemcasecmp(const UChar* a, const UChar* b, int len)
{
    return wkcUnicodeUCharMemCaseCmpPeer(a, b, len);
}

}
}
