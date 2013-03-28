/*
 *  WebKitWebFramePrivate.h
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

#ifndef WKCWebFramePrivate_h
#define WKCWebFramePrivate_h

// class definition

#include "WKCClientBuilders.h"

namespace WebCore {
    class Frame;
    class HTMLFrameOwnerElement;
#if ENABLE(WKC_PAGESAVE_MHTML)
    class MHTMLWebArchive;
#endif
}

namespace WKC {

class WKCWebViewPrivate;
class WKCWebFrame;
class FrameLoaderClientWKC;
class FramePrivate;

class WKCWebFramePrivate
{
    friend class WKCWebFrame;
    friend class FrameLoaderClientWKC;

public:
    static WKCWebFramePrivate* create(WKCWebFrame* parent, WKCWebViewPrivate* view, WKCClientBuilders& buliders, WebCore::HTMLFrameOwnerElement* ownerElement = 0);
    ~WKCWebFramePrivate();

    void notifyForceTerminate();

    inline WKCWebFrame* parent() const { return m_parent; };
    WebCore::Frame* core() const { return m_coreFrame; };
    FramePrivate* wkcCore() const { return m_wkcCoreFrame; }
    inline WKCClientBuilders& clientBuilders() const { return m_builders; };


private:
    WKCWebFramePrivate(WKCWebFrame* parent, WKCWebViewPrivate* view, WKCClientBuilders& builders, WebCore::HTMLFrameOwnerElement* ownerElement);
    bool construct();

    void coreFrameDestroyed();

private:
    WKCWebFrame* m_parent;
    WKCWebViewPrivate* m_view;
    WKCClientBuilders& m_builders;
    WebCore::HTMLFrameOwnerElement* m_ownerElement;

    WebCore::Frame* m_coreFrame;
    FramePrivate* m_wkcCoreFrame;

    // status
    WKC::LoadStatus m_loadStatus;

    // strings
    char* m_uri;
    unsigned short* m_title;
    unsigned short* m_name;

    bool m_forceTerminated;

    char* m_faviconURL;
#if ENABLE(WKC_PAGESAVE_MHTML)
    WebCore::MHTMLWebArchive* m_archive;
#endif
};

} // namespace

#endif // WKCWebFramePrivate_h
