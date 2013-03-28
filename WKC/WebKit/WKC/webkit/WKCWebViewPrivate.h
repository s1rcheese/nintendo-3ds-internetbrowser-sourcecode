/*
 * WKCWebViewPrivate.h
 *
 * Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.
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
 */

#ifndef WKCWebViewPrivate_h
#define WKCWebViewPrivate_h

#include "WKCClientBuilders.h"
#include "WKCEnums.h"

#include "IntRect.h"
#include "FloatPoint.h"

namespace WebCore {
    class Page;
}

namespace WKC {

class WKCWebView;
class InspectorClientWKC;
class DropDownListClientWKC;
class FrameLoaderClientWKC;
class WKCSettings;

class Node;
class Page;
class NodePrivate;
class PagePrivate;

// class definitions

class WKCWebViewPrivate
{
    friend class WKCWebView;
    friend class FrameLoaderClientWKC;

public:
    static WKCWebViewPrivate* create(WKCWebView* parent, WKCClientBuilders& builders);
    ~WKCWebViewPrivate();
    void notifyForceTerminate();

    inline WebCore::Page* core() const { return m_corePage; };
    WKC::Page* wkcCore() const;
    inline WKCWebView* parent() const { return m_parent; };
    inline WKCClientBuilders& clientBuilders() const { return m_clientBuilders; };

    inline DropDownListClientWKC* dropdownlistclient() { return m_dropdownlist; };
    // settings
    WKCSettings* settings();

    // drawings
    bool setOffscreen(WKC::OffscreenFormat format, void* bitmap, int rowbytes, const WebCore::IntSize& desktopsize, const WebCore::IntSize& viewsize, bool fixedlayout);
    void notifyResizeDesktopSize(const WebCore::IntSize& size);
    void notifyResizeViewSize(const WebCore::IntSize& size);
    void notifyRelayout();
    void notifyPaintOffscreen(const WebCore::IntRect& rect);
    void notifyScrollOffscreen(const WebCore::IntRect& rect, const WebCore::IntSize& diff);
    const WebCore::IntSize& desktopSize() const;
    const WebCore::IntSize& viewSize() const;
    const WebCore::IntSize& defaultDesktopSize() const;
    const WebCore::IntSize& defaultViewSize() const;
    void setUseAntiAliasForDrawings(bool flag);
    void setUseBilinearForScaledImages(bool flag);
    static void setUseBilinearForCanvasImages(bool flag);
    static void setUseAntiAliasForCanvas(bool flag);
    void setScrollPositionForOffscreen(const WebCore::IntPoint& scrollPosition);
    void notifyScrollPositionChanged();

    //
    bool transparent() { return m_isTransparent; };
    void setTransparent(bool flag);

    inline float opticalZoomLevel() const { return m_opticalZoomLevel; };
    inline const WKCFloatPoint& opticalZoomOffset() const { return m_opticalZoomOffset; };
    void setOpticalZoom(float, const WKCFloatPoint&);

    WKC::Node* getFocusedNode();
    WKC::Node* getNodeFromPoint(int x, int y);

private:
    WKCWebViewPrivate(WKCWebView* parent, WKCClientBuilders& builders);
    bool construct();

    bool prepareDrawings();

private:
    WKCWebView* m_parent;
    WKCClientBuilders& m_clientBuilders;

    // core
    WebCore::Page* m_corePage;
    WKC::PagePrivate* m_wkcCorePage;

    // instances
    WKCWebFrame* m_mainFrame;
    InspectorClientWKC* m_inspector;
    DropDownListClientWKC* m_dropdownlist;
    WKCSettings* m_settings;

    // offscreen
    void* m_drawContext;
    void* m_offscreen;

    WebCore::IntSize m_desktopSize;
    WebCore::IntSize m_viewSize;
    WebCore::IntSize m_defaultDesktopSize;
    WebCore::IntSize m_defaultViewSize;

    float m_opticalZoomLevel;
    WKCFloatPoint m_opticalZoomOffset;

    bool m_isZoomFullContent;
    bool m_isTransparent;
    WKC::LoadStatus m_loadStatus;

    // temporary string resources
    unsigned short* m_encoding;
    unsigned short* m_customEncoding;

    bool m_forceTerminated;

    WKC::NodePrivate* m_focusedNode;
    WKC::NodePrivate* m_nodeFromPoint;
};

} // namespace

#endif // WKCWebViewPrivate_h
