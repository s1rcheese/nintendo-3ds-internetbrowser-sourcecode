/*
 * Copyright (C) 2006, 2007 Apple Inc.  All rights reserved.
 * Copyright (C) 2008 Collabora Ltd. All rights reserved.
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
#include "PluginView.h"

#include "Document.h"
#include "DocumentLoader.h"
#include "Element.h"
#include "FrameLoader.h"
#include "FrameLoadRequest.h"
#include "FrameTree.h"
#include "Frame.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HTMLNames.h"
#include "HTMLPlugInElement.h"
#include "HostWindow.h"
#include "Image.h"
#include "KeyboardEvent.h"
#include "MouseEvent.h"
#include "Page.h"
#include "PlatformMouseEvent.h"
#include "PluginDebug.h"
#include "PluginMainThreadScheduler.h"
#include "PluginPackage.h"
#include "RenderLayer.h"
#include "Settings.h"
#include "JSDOMBinding.h"
#include "ScriptController.h"
#include "npruntime_impl.h"
#include "runtime.h"
#include "runtime_root.h"
#include <runtime/JSLock.h>
#include <runtime/JSValue.h>

#include "NotImplemented.h"

using JSC::ExecState;
using JSC::Interpreter;
using JSC::JSLock;
using JSC::JSObject;
using JSC::UString;

using std::min;

using namespace WTF;

namespace WebCore {

using namespace HTMLNames;

bool PluginView::dispatchNPEvent(NPEvent& event)
{
    // sanity check
    if (!m_plugin->pluginFuncs()->event)
        return false;

    notImplemented();
    return false;
}

void PluginView::updatePluginWidget()
{
    notImplemented();
}

void PluginView::setFocus()
{
    notImplemented();
}

void PluginView::show()
{
    notImplemented();
}

void PluginView::hide()
{
    notImplemented();
}

void PluginView::paint(GraphicsContext* context, const IntRect& rect)
{
    notImplemented();
}

void PluginView::handleKeyboardEvent(KeyboardEvent* event)
{
    notImplemented();
}

void PluginView::handleMouseEvent(MouseEvent* event)
{
    notImplemented();
}

void PluginView::setParent(ScrollView* parent)
{
    notImplemented();
}

void PluginView::setNPWindowRect(const IntRect&)
{
    notImplemented();
}

void PluginView::setParentVisible(bool visible)
{
    notImplemented();
}

NPError PluginView::handlePostReadFile(Vector<char>& buffer, uint32 len, const char* buf)
{
    notImplemented();
    return NPERR_FILE_NOT_FOUND;
}

NPError PluginView::getValue(NPNVariable variable, void* value)
{
    notImplemented();
    return NPERR_GENERIC_ERROR;
}

void PluginView::invalidateRect(const IntRect& rect)
{
    notImplemented();
}

void PluginView::invalidateRect(NPRect* rect)
{
    notImplemented();
}

void PluginView::invalidateRegion(NPRegion)
{
    notImplemented();
}

void PluginView::forceRedraw()
{
    notImplemented();
}

bool PluginView::platformStart()
{
    notImplemented();
    return false;
}

void PluginView::platformDestroy()
{
    notImplemented();
}

void PluginView::halt()
{
    notImplemented();
}

void PluginView::restart()
{
    notImplemented();
}

} // namespace WebCore

