/*
 *  WKCPlatformEvents.h
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

#ifndef WKCPlatformEvents_h
#define WKCPlatformEvents_h

#include "WKCEnums.h"

namespace WKC {

typedef struct WKCKeyEvent_ {
    WKC::KeyEventType m_type;
    WKC::Key m_key;
    WKC::Modifier m_modifiers;
} WKCKeyEvent;

typedef struct WKCMouseEvent_ {
    WKC::MouseEventType m_type;
    WKC::MouseButton m_button;
    int m_x;
    int m_y;
    WKC::Modifier m_modifiers;
    unsigned int m_timestampinsec;
} WKCMouseEvent;

typedef struct WKCWheelEvent_ {
    int m_dx;
    int m_dy;
    int m_x;
    int m_y;
    WKC::Modifier m_modifiers;
} WKCWheelEvent;

} // namespace

#endif // WKCPlatformEvents_h
