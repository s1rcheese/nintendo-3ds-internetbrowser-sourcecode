/*
 * Copyright (C) 2008 Gustavo Noronha Silva
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
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
#include "InspectorClientWKC.h"

#include "WKCWebViewPrivate.h"

#include "helpers/InspectorClientIf.h"

// implementations

namespace WKC {

InspectorClientWKC::InspectorClientWKC(WKCWebViewPrivate* view)
     : m_view(view),
       m_appClient(0),
       m_inspectedWebView(0),
       m_webInspector(0)
{
}
InspectorClientWKC::~InspectorClientWKC()
{
    if (m_appClient) {
        if (m_view) {
            m_view->clientBuilders().deleteInspectorClient(m_appClient);
        } else {
            // Ugh!: no way to destroy!
        }
    }
}

InspectorClientWKC*
InspectorClientWKC::create(WKCWebViewPrivate* view)
{
    InspectorClientWKC* self = 0;
    self = new InspectorClientWKC(view);
    if (!self) {
        return 0;
    }
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
InspectorClientWKC::construct()
{
    m_appClient = m_view->clientBuilders().createInspectorClient(m_view->parent());
    if (!m_appClient) return false;
    return true;
}

void
InspectorClientWKC::webViewDestroyed()
{
    m_view = 0;
}
void
InspectorClientWKC::inspectorDestroyed()
{
    delete this;
}

WebCore::Page*
InspectorClientWKC::createPage()
{
    return 0;
}

WebCore::String
InspectorClientWKC::localizedStringsURL()
{
    return WebCore::String();
}

WebCore::String
InspectorClientWKC::hiddenPanels()
{
    return WebCore::String();
}

void
InspectorClientWKC::showWindow()
{
}

void
InspectorClientWKC::closeWindow()
{
}

void
InspectorClientWKC::attachWindow()
{
}

void
InspectorClientWKC::detachWindow()
{
}

void
InspectorClientWKC::setAttachedWindowHeight(unsigned height)
{
}

void
InspectorClientWKC::highlight(WebCore::Node*)
{
}

void
InspectorClientWKC::hideHighlight()
{
}

void
InspectorClientWKC::inspectedURLChanged(const WebCore::String& newURL)
{
}

void
InspectorClientWKC::populateSetting(const WebCore::String& key, WebCore::String* value)
{
}

void
InspectorClientWKC::storeSetting(const WebCore::String& key, const WebCore::String& value)
{
}

void
InspectorClientWKC::inspectorWindowObjectCleared()
{
}

} // namespace
