/*
 *  WKCSkin.h
 *
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
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#ifndef WKCSkin_h
#define WKCSkin_h

#include <wkc/wkcbase.h>

namespace WKC {

enum SkinImage {
    ESkinImageCheckboxUnchecked = 0,
    ESkinImageCheckboxChecked,
    ESkinImageCheckboxUncheckedDisabled,
    ESkinImageCheckboxCheckedDisabled,
    ESkinImageCheckboxUncheckedHovered,
    ESkinImageCheckboxCheckedHovered,
    ESkinImageCheckboxUncheckedFocused,
    ESkinImageCheckboxCheckedFocused,
    ESkinImageRadioUnchecked,
    ESkinImageRadioChecked,
    ESkinImageRadioUncheckedDisabled,
    ESkinImageRadioCheckedDisabled,
    ESkinImageRadioUncheckedHovered,
    ESkinImageRadioCheckedHovered,
    ESkinImageRadioUncheckedFocused,
    ESkinImageRadioCheckedFocused,
    ESkinImageButton,
    ESkinImageButtonDisabled,
    ESkinImageButtonPressed,
    ESkinImageButtonHovered,
    ESkinImageMenuListButton,
    ESkinImageMenuListButtonDisabled,
    ESkinImageMenuListButtonHovered,
    ESkinImageMenuListButtonFocused,
    ESkinImageVScrollbarBackground,
    ESkinImageVScrollbarBackgroundDisabled,
    ESkinImageVScrollbarThumb,
    ESkinImageVScrollbarThumbHovered,
    ESkinImageVScrollbarUp,
    ESkinImageVScrollbarUpDisabled,
    ESkinImageVScrollbarUpHovered,
    ESkinImageVScrollbarDown,
    ESkinImageVScrollbarDownDisabled,
    ESkinImageVScrollbarDownHovered,
    ESkinImageHScrollbarBackground,
    ESkinImageHScrollbarBackgroundDisabled,
    ESkinImageHScrollbarThumb,
    ESkinImageHScrollbarThumbHovered,
    ESkinImageHScrollbarLeft,
    ESkinImageHScrollbarLeftDisabled,
    ESkinImageHScrollbarLeftHovered,
    ESkinImageHScrollbarRight,
    ESkinImageHScrollbarRightDisabled,
    ESkinImageHScrollbarRightHovered,
    ESkinImageScrollbarCrossCorner,
    ESkinImageScrollbarCrossCornerDisabled,

    ESkinImages
};

enum SkinColor {
    // system colors
    ESkinColorActiveBorder = 0,
    ESkinColorActiveCaption,
    ESkinColorAppWorkSpace,
    ESkinColorBackground,
    ESkinColorButtonFace,
    ESkinColorButtonHighlight,
    ESkinColorButtonShadow,
    ESkinColorButtonText,
    ESkinColorCaptionText,
    ESkinColorGrayYext,
    ESkinColorHighlight,
    ESkinColorHighlightText,
    ESkinColorInactiveBorder,
    ESkinColorInactiveCaption,
    ESkinColorInactiveCaptionText,
    ESkinColorInfoBackground,
    ESkinColorInfoText,
    ESkinColorMenu,
    ESkinColorMenuText,
    ESkinColorScrollbar,
    ESkinColorText,
    ESkinColorThreeDDarkShadow,
    ESkinColorThreeDFace,
    ESkinColorThreeDHighlight,
    ESkinColorThreeDLightShadow,
    ESkinColorThreeDShadow,
    ESkinColorWindow,
    ESkinColorWindowFrame,
    ESkinColorWindowText,

    // text selection colors
    ESkinColorActiveSelectionForeground,
    ESkinColorActiveSelectionBackground,
    ESkinColorInactiveSelectionForeground,
    ESkinColorInactiveSelectionBackground,

    // focus ring
    ESkinColorFocusRing,

    // text-field
    ESkinColorTextfieldBorder,
    ESkinColorTextfieldBackground,
    ESkinColorTextfieldBackgroundDisabled,

    ESkinColors
};
 
enum SystemFontType {
    ESystemFontTypeCaption = 0,
    ESystemFontTypeIcon,
    ESystemFontTypeMenu,
    ESystemFontTypeMessageBox,
    ESystemFontTypeSmallCaption,
    ESystemFontTypeWebkitMiniControl,
    ESystemFontTypeWebkitSmallControl,
    ESystemFontTypeWebkitControl,
    ESystemFontTypeStatusBar,
    
    ESystemFontTypes
};

typedef struct WKCSkinImage_ {
    WKCSize fSize;
    WKCPoint fPoints[4];
    // bitmap format: ARGB8888
    // (lsb) AAAAAAAA RRRRRRRR GGGGGGGG BBBBBBBB (msb)
    // rowbytes must be fSize.fWidth * 4
    void* fBitmap;
} WKCSkinImage;

typedef struct WKCSkin_ {
    WKCSkinImage fImages[ESkinImages];
    // color format: ARGB8888
    unsigned int fColors[ESkinColors];
    float fSystemFontSize[ESystemFontTypes];
} WKCSkin;

} // namespace

#endif // WKCSkin_h
