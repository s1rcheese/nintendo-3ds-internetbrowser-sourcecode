/*
 * This file is part of the popup menu implementation for <select> elements in WebCore.
 *
 * Copyright (C) 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2008 Collabora Ltd.
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
#include "PopupMenu.h"

#include "CString.h"
#include "FrameView.h"
#include "HostWindow.h"
#include "PlatformString.h"
#include "RenderMenuList.h"

#include "NotImplemented.h"

#include "WKCWebViewPrivate.h"
#include "DropDownListClientWKC.h"


namespace WebCore {

static PopupMenuClient *gPopupMenuClient = 0;

PopupMenu::PopupMenu(PopupMenuClient* client)
    : m_popupClient(client)
{
}

PopupMenu::~PopupMenu()
{
    hide();
    gPopupMenuClient = 0;
}

void PopupMenu::show(const IntRect& rect, FrameView* view, int index)
{
    if (gPopupMenuClient) {  // already other dropdownlist showing...
        return;
    }

    PlatformPageClient pageclient;
    pageclient = view->hostWindow()->platformPageClient();
    if (pageclient) {
        WKC::WKCWebViewPrivate *webview = (WKC::WKCWebViewPrivate*)pageclient;
        gPopupMenuClient = client();
        webview->dropdownlistclient()->show(rect, view, index, client());
        gPopupMenuClient = 0;
    }
}

void PopupMenu::hide()
{
    PopupMenuClient *menuclient;

    if (client())
        menuclient = client();
    else
        menuclient = gPopupMenuClient;

    if (menuclient) {
        PlatformPageClient pageclient;
        pageclient = menuclient->hostWindow()->platformPageClient();
        if (pageclient) {
            WKC::WKCWebViewPrivate *webview = (WKC::WKCWebViewPrivate*)pageclient;
            webview->dropdownlistclient()->hide(menuclient);
        }
        menuclient->popupDidHide();
    }
}

void PopupMenu::updateFromElement()
{
    if (client()) {
        PlatformPageClient pageclient;
        pageclient = client()->hostWindow()->platformPageClient();
        if (pageclient) {
            WKC::WKCWebViewPrivate *webview = (WKC::WKCWebViewPrivate*)pageclient;
            webview->dropdownlistclient()->updateFromElement(client());
        }
        client()->setTextFromItem(client()->selectedIndex());
    }
}

bool PopupMenu::itemWritingDirectionIsNatural()
{
    notImplemented();
    return true;
}

}

