/*
 *  Copyright (C) 2007 Holger Hans Peter Freyther
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
#include "ContextMenu.h"
#include "NotImplemented.h"
#include "ContextMenuController.h"

namespace WebCore {

// TODO: ref-counting correctness checking.
// See http://bugs.webkit.org/show_bug.cgi?id=16115

ContextMenu::ContextMenu(const HitTestResult& result)
    : m_hitTestResult(result)
{
    m_platformDescription = 0;
}

ContextMenu::~ContextMenu()
{
}

void ContextMenu::appendItem(ContextMenuItem& item)
{
    notImplemented();
}

void ContextMenu::setPlatformDescription(PlatformMenuDescription menu)
{
    notImplemented();
}

PlatformMenuDescription ContextMenu::platformDescription() const
{
    notImplemented();
    return 0;
}

PlatformMenuDescription ContextMenu::releasePlatformDescription()
{
    notImplemented();
    return 0;
}

}
