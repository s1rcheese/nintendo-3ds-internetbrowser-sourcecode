/*
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "config.h"

#include "helpers/WKCSelectionController.h"
#include "helpers/privates/WKCSelectionControllerPrivate.h"

#include "SelectionController.h"

namespace WKC {

SelectionControllerPrivate::SelectionControllerPrivate(WebCore::SelectionController* parent)
    : m_webcore(parent)
    , m_wkc(*this)
{
}

SelectionControllerPrivate::~SelectionControllerPrivate()
{
}

bool
SelectionControllerPrivate::isCaret() const
{
    return m_webcore->isCaret();
}

bool
SelectionControllerPrivate::isRange() const
{
    return m_webcore->isRange();
}

void
SelectionControllerPrivate::clear()
{
    return m_webcore->clear();
}

SelectionController::SelectionController(SelectionControllerPrivate& parent)
    : m_private(parent)
{
}

SelectionController::~SelectionController()
{
}


bool
SelectionController::isCaret() const
{
    return m_private.isCaret();
}

bool
SelectionController::isRange() const
{
    return m_private.isRange();
}

void
SelectionController::clear()
{
    m_private.clear();
}

} // namespace
