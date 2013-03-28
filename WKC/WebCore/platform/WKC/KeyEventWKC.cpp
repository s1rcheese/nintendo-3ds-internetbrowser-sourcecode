/*
 * Copyright (C) 2006, 2007 Apple Inc.  All rights reserved.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (C) 2008 Collabora, Ltd.  All rights reserved.
 * All rights reserved.
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
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
#include "PlatformKeyboardEvent.h"

#include "KeyboardCodes.h"
#include "NotImplemented.h"
#include "TextEncoding.h"

#include "WKCPlatformEvents.h"

namespace WebCore {

static String keyIdentifierForWKCKeyCode(int keyCode)
{
    switch (keyCode) {
    case WKC::EKeyBack:
        return "U+0008";
    case WKC::EKeyTab:
        return "U+0009";
    case WKC::EKeyClear:
        return "Clear";
    case WKC::EKeyReturn:
        return "Enter";
    case WKC::EKeyMenu:
        return "Alt";
    case WKC::EKeyPause:
        return "Pause";
    case WKC::EKeyPrior:
        return "PageUp";
    case WKC::EKeyNext:
        return "PageDown";
    case WKC::EKeyEnd:
        return "End";
    case WKC::EKeyHome:
        return "Home";
    case WKC::EKeyLeft:
        return "Left";
    case WKC::EKeyUp:
        return "Up";
    case WKC::EKeyRight:
        return "Right";
    case WKC::EKeyDown:
        return "Down";
    case WKC::EKeySelect:
        return "Select";
    case WKC::EKeyExecute:
        return "Execute";
    case WKC::EKeySnapShot:
        return "PrintScreen";
    case WKC::EKeyInsert:
        return "Insert";
    case WKC::EKeyDelete:
        return "U+007F";
    case WKC::EKeyHelp:
        return "Help";

    case WKC::EKeyF1:
        return "F1";
    case WKC::EKeyF2:
        return "F2";
    case WKC::EKeyF3:
        return "F3";
    case WKC::EKeyF4:
        return "F4";
    case WKC::EKeyF5:
        return "F5";
    case WKC::EKeyF6:
        return "F6";
    case WKC::EKeyF7:
        return "F7";
    case WKC::EKeyF8:
        return "F8";
    case WKC::EKeyF9:
        return "F9";
    case WKC::EKeyF10:
        return "F10";
    case WKC::EKeyF11:
        return "F11";
    case WKC::EKeyF12:
        return "F12";
    case WKC::EKeyF13:
        return "F13";
    case WKC::EKeyF14:
        return "F14";
    case WKC::EKeyF15:
        return "F15";
    case WKC::EKeyF16:
        return "F16";
    case WKC::EKeyF17:
        return "F17";
    case WKC::EKeyF18:
        return "F18";
    case WKC::EKeyF19:
        return "F19";
    case WKC::EKeyF20:
        return "F20";
    case WKC::EKeyF21:
        return "F21";
    case WKC::EKeyF22:
        return "F22";
    case WKC::EKeyF23:
        return "F23";
    case WKC::EKeyF24:
        return "F24";

    default:
        break;
    }

    return String::format("U+%04X", keyCode);
}

static int windowsKeyCodeForKeyEvent(int keyCode)
{
    switch (keyCode) {
    case WKC::EKeyBack:
    case WKC::EKeyTab:
    case WKC::EKeyClear:
    case WKC::EKeyReturn:
    case WKC::EKeyShift:
    case WKC::EKeyControl:
    case WKC::EKeyMenu:
    case WKC::EKeyPause:
    case WKC::EKeyCapital:
    case WKC::EKeyKana:
//    case WKC::EKeyHangul:
    case WKC::EKeyJunja:
    case WKC::EKeyFinal:
    case WKC::EKeyHanja:
//    case WKC::EKeyKanji:
    case WKC::EKeyEscape:
    case WKC::EKeyConvert:
    case WKC::EKeyNonConvert:
    case WKC::EKeyAccept:
    case WKC::EKeyModeChange:
    case WKC::EKeySpace:
    case WKC::EKeyPrior:
    case WKC::EKeyNext:
    case WKC::EKeyEnd:
    case WKC::EKeyHome:
    case WKC::EKeyLeft:
    case WKC::EKeyUp:
    case WKC::EKeyRight:
    case WKC::EKeyDown:
    case WKC::EKeySelect:
    case WKC::EKeyPrint:
    case WKC::EKeyExecute:
    case WKC::EKeySnapShot:
    case WKC::EKeyInsert:
    case WKC::EKeyDelete:
    case WKC::EKeyHelp:

    case WKC::EKey0:
    case WKC::EKey1:
    case WKC::EKey2:
    case WKC::EKey3:
    case WKC::EKey4:
    case WKC::EKey5:
    case WKC::EKey6:
    case WKC::EKey7:
    case WKC::EKey8:
    case WKC::EKey9:
    case WKC::EKeyA:
    case WKC::EKeyB:
    case WKC::EKeyC:
    case WKC::EKeyD:
    case WKC::EKeyE:
    case WKC::EKeyF:
    case WKC::EKeyG:
    case WKC::EKeyH:
    case WKC::EKeyI:
    case WKC::EKeyJ:
    case WKC::EKeyK:
    case WKC::EKeyL:
    case WKC::EKeyM:
    case WKC::EKeyN:
    case WKC::EKeyO:
    case WKC::EKeyP:
    case WKC::EKeyQ:
    case WKC::EKeyR:
    case WKC::EKeyS:
    case WKC::EKeyT:
    case WKC::EKeyU:
    case WKC::EKeyV:
    case WKC::EKeyW:
    case WKC::EKeyX:
    case WKC::EKeyY:
    case WKC::EKeyZ:

    case WKC::EKeyLWin:
    case WKC::EKeyRWin:
    case WKC::EKeyApps:
    case WKC::EKeySleep:
    case WKC::EKeyNumPad0:
    case WKC::EKeyNumPad1:
    case WKC::EKeyNumPad2:
    case WKC::EKeyNumPad3:
    case WKC::EKeyNumPad4:
    case WKC::EKeyNumPad5:
    case WKC::EKeyNumPad6:
    case WKC::EKeyNumPad7:
    case WKC::EKeyNumPad8:
    case WKC::EKeyNumPad9:
    case WKC::EKeyMultiply:
    case WKC::EKeyAdd:
    case WKC::EKeySeparator:
    case WKC::EKeySubtract:
    case WKC::EKeyDecimal:
    case WKC::EKeyDivide:
    case WKC::EKeyF1:
    case WKC::EKeyF2:
    case WKC::EKeyF3:
    case WKC::EKeyF4:
    case WKC::EKeyF5:
    case WKC::EKeyF6:
    case WKC::EKeyF7:
    case WKC::EKeyF8:
    case WKC::EKeyF9:
    case WKC::EKeyF10:
    case WKC::EKeyF11:
    case WKC::EKeyF12:
    case WKC::EKeyF13:
    case WKC::EKeyF14:
    case WKC::EKeyF15:
    case WKC::EKeyF16:
    case WKC::EKeyF17:
    case WKC::EKeyF18:
    case WKC::EKeyF19:
    case WKC::EKeyF20:
    case WKC::EKeyF21:
    case WKC::EKeyF22:
    case WKC::EKeyF23:
    case WKC::EKeyF24:
    case WKC::EKeyNumLock:
    case WKC::EKeyScroll:
    case WKC::EKeyLShift:
    case WKC::EKeyRShift:
    case WKC::EKeyLControl:
    case WKC::EKeyRControl:
    case WKC::EKeyLMenu:
    case WKC::EKeyRMenu:
    case WKC::EKeyBrowserBack:
    case WKC::EKeyBrowserForward:
    case WKC::EKeyBrowserRefresh:
    case WKC::EKeyBrowserStop:
    case WKC::EKeyBrowserSearch:
    case WKC::EKeyBrowserFavorites:
    case WKC::EKeyBrowserHome:
    case WKC::EKeyVolumeMute:
    case WKC::EKeyVolumeDown:
    case WKC::EKeyVolumeUp:
    case WKC::EKeyMediaNextTrack:
    case WKC::EKeyMediaPrevTrack:
    case WKC::EKeyMediaStop:
    case WKC::EKeyMediaPlayPause:
    case WKC::EKeyMediaLaunchMail:
    case WKC::EKeyMediaLaunchMediaselect:
    case WKC::EKeyMediaLaunchApp1:
    case WKC::EKeyMediaLaunchApp2:

    case WKC::EKeyOem1:
    case WKC::EKeyOemPlus:
    case WKC::EKeyOemComma:
    case WKC::EKeyOemMinus:
    case WKC::EKeyOemPeriod:
    case WKC::EKeyOem2:
    case WKC::EKeyOem3:
    case WKC::EKeyOem4:
    case WKC::EKeyOem5:
    case WKC::EKeyOem6:
    case WKC::EKeyOem7:
    case WKC::EKeyOem8:

    case WKC::EKeyOem102:
    case WKC::EKeyProcessKey:
    case WKC::EKeyPacket:
    case WKC::EKeyAttn:
    case WKC::EKeyCrSel:
    case WKC::EKeyExSel:
    case WKC::EKeyErEOF:
    case WKC::EKeyPlay:
    case WKC::EKeyZoom:
    case WKC::EKeyNoName:
    case WKC::EKeyPA1:
    case WKC::EKeyOemClear:
        return keyCode;

    default:
        break;
    }
    return VK_UNKNOWN;
}

static String
singleCharacterString(int keyCode)
{
    UChar buf[2];
    switch (keyCode) {
    case WKC::EKeyReturn:
        return String("\r");
    case WKC::EKeyBack:
        return String("\x8");
    case WKC::EKeyTab:
        return String("\t");
    default:
        buf[0] = (UChar)keyCode;
        buf[1] = 0;
        return String(buf, 1);
    }
}



// Keep this in sync with the other platform event constructors
// TODO: m_gdkEventKey should be refcounted
PlatformKeyboardEvent::PlatformKeyboardEvent(void* event)
{
    WKC::WKCKeyEvent* ev = (WKC::WKCKeyEvent *)event;

    m_windowsVirtualKeyCode = 0;

    switch (ev->m_type) {
    case WKC::EKeyEventPressed:
        m_type = KeyDown;
        break;
    case WKC::EKeyEventReleased:
        m_type = KeyUp;
        break;
    default:
        m_type = KeyUp;
    }
    m_keyIdentifier = keyIdentifierForWKCKeyCode(ev->m_key);
    m_windowsVirtualKeyCode = windowsKeyCodeForKeyEvent(ev->m_key);
    m_text = singleCharacterString(ev->m_key);
    m_unmodifiedText = singleCharacterString(ev->m_key);
    m_autoRepeat = false;
    m_nativeVirtualKeyCode = ev->m_key;
    switch (ev->m_key) {
    case WKC::EKeyNumPad0:
    case WKC::EKeyNumPad1:
    case WKC::EKeyNumPad2:
    case WKC::EKeyNumPad3:
    case WKC::EKeyNumPad4:
    case WKC::EKeyNumPad5:
    case WKC::EKeyNumPad6:
    case WKC::EKeyNumPad7:
    case WKC::EKeyNumPad8:
    case WKC::EKeyNumPad9:
    case WKC::EKeyMultiply:
    case WKC::EKeyAdd:
    case WKC::EKeySeparator:
    case WKC::EKeySubtract:
    case WKC::EKeyDecimal:
    case WKC::EKeyDivide:
        m_isKeypad = true;
        break;
    default:
        m_isKeypad = false;
    }

    if (ev->m_modifiers & WKC::EModifierShift) {
        m_shiftKey = true;
    } else {
        m_shiftKey = false;
    }
    if (ev->m_modifiers & WKC::EModifierCtrl) {
        m_ctrlKey = true;
    } else {
        m_ctrlKey = false;
    }
    if (ev->m_modifiers & WKC::EModifierAlt) {
        m_altKey = true;
    } else {
        m_altKey = false;
    }
    if (ev->m_modifiers & WKC::EModifierMod1) {
        m_metaKey = true;
    } else {
        m_metaKey = false;
    }
}

void PlatformKeyboardEvent::disambiguateKeyDownEvent(Type type, bool backwardCompatibilityMode)
{
    // Can only change type from KeyDown to RawKeyDown or Char, as we lack information for other conversions.
    ASSERT(m_type == KeyDown);
    m_type = type;
    if (type == RawKeyDown) {
        m_text = String();
        m_unmodifiedText = String();
    } else {
        m_keyIdentifier = String();
        m_windowsVirtualKeyCode = 0;
    }
}

bool PlatformKeyboardEvent::currentCapsLockState()
{
    notImplemented();
    return false;
}

}
