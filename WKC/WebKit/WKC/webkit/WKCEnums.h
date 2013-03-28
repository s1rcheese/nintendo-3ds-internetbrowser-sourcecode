/*
 *  WKCEnums.h
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

#ifndef WKCEnums_h
#define WKCEnums_h

#include "WKCEnumKeys.h"

namespace WKC {

enum OffscreenFormat {
    EOffscreenFormatRGBA4444 = 0,
    EOffscreenFormatRGBA5551,
    EOffscreenFormatRGBA5650,
    EOffscreenFormatRGBA5650Tiny,
    EOffscreenFormatRGBA8888,
    EOffscreenFormatRGB888,
    EOffscreenFormatNative,
    EOffscreenFormatPolygon,
    EOffscreenFormats
};

enum KeyEventType {
    EKeyEventPressed = 0,
    EKeyEventReleased,
    TKeyEvents
};

enum MouseEventType {
    EMouseEventDown,
    EMouseEventUp,
    EMouseEventMove,
    EMouseEventDrag,
    EMouseEventDoubleClick,
    EMouseEventLongPressed,
    EMouseEvents
};

enum MouseButton {
    EMouseButtonNone = 0,
    EMouseButtonLeft,
    EMouseButtonMiddle,
    EMouseButtonRight,
    EMouseButtons
};

enum Modifier {
    EModifierNone  = 0x0000,
    EModifierCtrl  = 0x0001,
    EModifierShift = 0x0002,
    EModifierAlt   = 0x0004,
    EModifierMod1  = 0x0008,
    EModifierMod2  = 0x0010,
    EModifierMod3  = 0x0020,
    EModifierMod4  = 0x0040,
};

enum ScrollType {
    EScrollUp = 0,
    EScrollDown,
    EScrollLeft,
    EScrollRight,
    EScrollPageUp,
    EScrollPageDown,
    EScrollPageLeft,
    EScrollPageRight,
    EScrollTop,
    EScrollBottom,
    EScrolls
};

enum CacheModel {
    ECacheModelDocumentViewer,
    ECacheModelWebBrowser
};

enum LoadStatus {
    ELoadStatusNone,
    ELoadStatusProvisional,
    ELoadStatusCommitted,
    ELoadStatusFinished,
    ELoadStatusFirstVisual,
    ELoadStatusFailed,
};

enum ScrollbarMode {
    EScrollbarAlwaysOn,
    EScrollbarAlwaysOff,
    EScrollbarAuto
};

enum FocusDirection {
    EFocusDirectionUp,
    EFocusDirectionDown,
    EFocusDirectionLeft,
    EFocusDirectionRight,
    EFocusDirections
};

} // namespace

#endif // WKCEnums_h
