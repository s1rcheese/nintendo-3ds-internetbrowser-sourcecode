/*
 *  WebKitWebFrame.h
 *
 *  Copyright (c) 2010, 2011 ACCESS CO., LTD. All rights reserved.
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

#ifndef WKCWebFrame_h
#define WKCWebFrame_h

// class definition

#include "WKCEnums.h"
#include "WKCClientBuilders.h"
#include "WKCFaviconInfo.h"

namespace WKC {
    class Frame;
    class HTMLFrameOwnerElement;
}

namespace WKC {

class WKCWebViewPrivate;
class WKCWebFramePrivate;
class WKCWebDataSource;
class WKCSecurityOrigin;

class WKCWebFrame
{
public:
    static WKCWebFrame* create(WKCWebView* view, WKCClientBuilders& builders);
    static WKCWebFrame* create(WKCWebViewPrivate* view, WKCClientBuilders& builders, WKC::HTMLFrameOwnerElement* ownerelement=0);
    static void deleteWKCWebFrame(WKCWebFrame* self);

    void notifyForceTerminate();

    // APIs
    WKC::Frame* core() const;
    bool compare(const WKC::Frame*) const;

    WKCWebView* webView();
    const unsigned short* name();
    const unsigned short* title();
    const char* uri();
    WKCWebFrame* parent();

    void loadURI(const char* uri);
    void loadString(const char* content, const unsigned short* mime_type, const unsigned short* encoding, const char *base_uri, const char *unreachable_uri=0);
    void stopLoading();
    void reload();

    WKCWebFrame* findFrame(const unsigned short* name);

    WKC::LoadStatus loadStatus();
    WKC::ScrollbarMode horizontalScrollbarMode();
    WKC::ScrollbarMode verticalScrollbarMode();

    WKCSecurityOrigin* securityOrigin();
    const char* faviconURL();
    bool getFaviconInfo(WKCFaviconInfo* info);

    // pagesave
    bool contentSerializeStart();
    int contentSerializeProgress(void* buffer, unsigned int length);
    void contentSerializeEnd();
    bool isPageArchiveLoadFailed();

private:
    WKCWebFrame();
    ~WKCWebFrame();
    bool construct(WKCWebViewPrivate* view, WKCClientBuilders& builders, WKC::HTMLFrameOwnerElement* ownerelement);

    friend class WKCWebViewPrivate;
    friend class ChromeClientWKC;
    friend class FrameLoaderClientWKC;
    inline WKCWebFramePrivate* privateFrame() const { return m_private; }

private:
    WKCWebFramePrivate* m_private;
};

} // namespace

#endif // WKCWebFrame_h
