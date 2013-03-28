/*
    WKCWebView.cpp
    Copyright (C) 2007, 2008 Holger Hans Peter Freyther
    Copyright (C) 2007, 2008, 2009 Christian Dywan <christian@imendio.com>
    Copyright (C) 2007 Xan Lopez <xan@gnome.org>
    Copyright (C) 2007, 2008 Alp Toker <alp@atoker.com>
    Copyright (C) 2008 Jan Alonzo <jmalonzo@unpluggable.com>
    Copyright (C) 2008 Gustavo Noronha Silva <gns@gnome.org>
    Copyright (C) 2008 Nuanti Ltd.
    Copyright (C) 2008, 2009 Collabora Ltd.
    Copyright (C) 2009 Igalia S.L.
    Copyright (C) 2009 Movial Creative Technologies Inc.
    Copyright (C) 2009 Bobby Powers
    Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "config.h"

#include "WKCWebView.h"
#include "WKCWebViewPrivate.h"
#include "WKCWebFrame.h"
#include "WKCWebFramePrivate.h"
#include "WKCPlatformEvents.h"
#include "WKCClientBuilders.h"
#include "WKCSettings.h"

#include "ChromeClientWKC.h"
#include "ContextMenuClientWKC.h"
#include "EditorClientWKC.h"
#include "DragClientWKC.h"
#include "FrameLoaderClientWKC.h"
#include "InspectorClientWKC.h"
#include "DropDownListClientWKC.h"

#include <wkc/wkcpeer.h>
#include <wkc/wkcmpeer.h>
#include <wkc/wkcsocket.h>

#include "helpers/WKCHistoryItem.h"
#include "helpers/WKCNode.h"
#include "helpers/privates/WKCHistoryItemPrivate.h"
#include "helpers/privates/WKCResourceHandlePrivate.h"
#include "helpers/privates/WKCNodePrivate.h"
#include "helpers/privates/WKCPagePrivate.h"
#include "helpers/privates/WKCEventHandlerPrivate.h"
#include "helpers/privates/WKCFocusControllerPrivate.h"

#include "Page.h"
#include "PageGroup.h"
#include "Frame.h"
#include "FrameView.h"
#include "NodeList.h"
#include "HTMLLinkElement.h"
#include "HitTestRequest.h"
#include "HitTestResult.h"
#include "ImageBufferData.h"
#include "RenderView.h"
#include "RenderText.h"
#include "FocusController.h"
#include "BackForwardList.h"
#include "PlatformKeyboardEvent.h"
#include "PlatformMouseEvent.h"
#include "PlatformWheelEvent.h"
#include "StringImpl.h"
#include "DocumentLoader.h"
#include "ProgressTracker.h"
#include "IconDatabase.h"
#include "Cache.h"
#include "PageCache.h"
#include "TextEncodingRegistry.h"
#include "Settings.h"
#include "FloatRect.h"
#include "ImageDecoderWKC.h"

#include "InitializeThreading.h"
#include "PageGroup.h"
#include "Pasteboard.h"

#include "ImageSource.h"

#include "NotImplemented.h"
#include "CookieJar.h"

// for global variables
#include "AXObjectCache.h"
#include "BytecodeGenerator.h"
#include "RefCountedLeakCounter.h"
#include "JSImageData.h"
#include "runtime_root.h"
#include "IdentifierRep.h"
#include "Executable.h"
#include "SamplingTool.h"
#include "MediaQueryEvaluator.h"
#include "MediaFeatureNames.h"
#include "CSSStyleSelector.h"
#include "CSSSelector.h"
#include "CSSPropertyLonghand.h"
#include "CSSInitialValue.h"
#include "CSSImportRule.h"
#include "CSSComputedStyleDeclaration.h"
#include "FontPlatformData.h"

#include "ResourceHandleManager.h"
#include "ResourceHandleManagerSSL.h"
#include "CookieJar.h"

#include "bindings/js/GCController.h"
#include "bindings/js/JSLazyEventListener.h"
#include "bindings/js/ScriptEventListener.h"

#include "dom/ContainerNode.h"
#include "dom/Document.h"
#include "dom/DOMImplementation.h"
#include "dom/EventTarget.h"
#include "dom/Node.h"
#include "dom/NodeRareData.h"
#include "dom/QualifiedName.h"
#include "dom/Range.h"
#include "dom/ScriptElement.h"
#include "dom/DocumentFragment.h"
#include "dom/StyledElement.h"
#include "dom/XMLTokenizer.h"
#include "dom/XMLTokenizerScope.h"

#include "editing/ApplyStyleCommand.h"
#include "editing/Editor.h"
#include "editing/htmlediting.h"
#include "editing/HTMLInterchange.h"
#include "editing/IndentOutdentCommand.h"
#include "editing/markup.h"
#include "editing/ReplaceSelectionCommand.h"

#include "history/CachedFrame.h"
#include "history/CachedPage.h"
#include "history/PageCache.h"

#include "html/HTMLButtonElement.h"
#include "html/HTMLElement.h"
#include "html/HTMLFieldSetElement.h"
#include "html/HTMLFormElement.h"
#include "html/HTMLInputElement.h"
#include "html/HTMLKeygenElement.h"
#include "html/HTMLLabelElement.h"
#include "html/HTMLLegendElement.h"
#include "html/HTMLLinkElement.h"
#include "html/HTMLOptGroupElement.h"
#include "html/HTMLOptionElement.h"
#include "html/HTMLParser.h"
#include "html/HTMLSelectElement.h"
#include "html/HTMLTableElement.h"
#include "html/HTMLTextAreaElement.h"
#include "html/ValidityState.h"

#if ENABLE(JAVASCRIPT_DEBUGGER)
#include "inspector/JavaScriptDebugServer.h"
#include "inspector/JavaScriptProfile.h"
#include "inspector/JavaScriptProfileNode.h"
#endif

#include "loader/archive/ArchiveFactory.h"
#include "loader/Cache.h"
#include "loader/CachedImage.h"
#include "loader/CachedResource.h"
#include "loader/CrossOriginAccessControl.h"
#include "loader/CrossOriginPreflightResultCache.h"
#include "loader/icon/IconDatabase.h"
#include "loader/ImageLoader.h"
#include "loader/loader.h"
#include "loader/SubresourceLoader.h"

#include "page/animation/AnimationBase.h"
#include "page/DOMTimer.h"
#include "page/Geolocation.h"
#include "page/PageGroup.h"
#include "page/SecurityOrigin.h"
#include "page/UserContentURLPattern.h"
#include "page/XSSAuditor.h"

#include "platform/Arena.h"
#include "platform/text/AtomicString.h"
#include "platform/text/BidiContext.h"
#include "platform/graphics/Font.h"
#include "platform/graphics/FontCache.h"
#include "platform/GeolocationService.h"
#include "platform/mock/GeolocationServiceMock.h"
#include "platform/graphics/GlyphPageTreeNode.h"
#include "platform/graphics/Image.h"
#include "platform/MIMETypeRegistry.h"
#include "platform/network/NetworkStateNotifier.h"
#include "platform/network/ResourceHandle.h"
#include "platform/Pasteboard.h"
#include "platform/SharedTimer.h"
#include "platform/ScrollView.h"
#include "platform/text/TextEncoding.h"
#include "platform/text/TextEncodingRegistry.h"
#include "platform/text/TextBreakIterator.h"
#include "platform/ThreadGlobalData.h"
#include "platform/ThreadTimers.h"
#include "platform/Timer.h"
#include "platform/FileChooser.h"

#include "plugins/PluginView.h"
#include "plugins/PluginStream.h"
#include "plugins/PluginMainThreadScheduler.h"
#include "plugins/PluginDatabase.h"

#include "profiler/Profiler.h"

#include "rendering/RenderWidget.h"
#include "rendering/RootInlineBox.h"
#include "rendering/RenderTheme.h"
#include "rendering/RenderScrollbarTheme.h"
#include "rendering/RenderScrollbar.h"
#include "rendering/RenderPartObject.h"
#include "rendering/RenderObject.h"
#include "rendering/RenderListItem.h"
#include "rendering/RenderListBox.h"
#include "rendering/RenderLayer.h"
#include "rendering/RenderImage.h"
#include "rendering/RenderFlexibleBox.h"
#include "rendering/RenderCounter.h"
#include "rendering/RenderBoxModelObject.h"
#include "rendering/RenderBox.h"
#include "rendering/RenderBlock.h"
#include "rendering/LayoutState.h"
#include "rendering/InlineTextBox.h"
#include "rendering/InlineBox.h"

#include "runtime/UString.h"
#include "runtime/Structure.h"

#include "wtf/DateMath.h"
#include "wtf/dtoa.h"
#include "xml/XMLHttpRequest.h"

#include "HTMLElementFactory.h"
#include "JSHTMLElementWrapperFactory.h"

#include "FastMallocWKC.h"


#ifdef __MINGW32__
# ifdef LoadString
#  undef LoadString
# endif
#endif // __MINGW32__

#if COMPILER(MSVC) && defined(_DEBUG)
bool gForceTerminate = false;
#endif

#if USE(MUTEX_DEBUG_LOG)
CRITICAL_SECTION gCriticalSection;
static bool gCriticalSectionFlag = false;
#endif

namespace WebCore {
extern bool initializeTextBreakIterators();
extern void finalizeTextBreakIterators();
extern void setScreenSizeWKC(const WebCore::IntSize& size);
extern void setAvailableScreenSize(const WebCore::IntSize& size);
extern void setScreenDepth(int depth, int depth_per_component);
extern void setIsMonochrome(bool monochrome);
extern bool initializeSharedTimer();
extern void finalizeSharedTimer();

extern void RenderThemeWKC_resetVariables();
extern void CookieJar_resetVariables();
extern void JSImageDataCustom_deleteSharedInstance();
extern void JSImageDataCustom_resetVariables();
extern void JSHTMLInputElement_deleteSharedInstance();
extern void JSHTMLInputElement_resetVariables();

typedef void (*ResolveFilenameForDisplayProc)(const unsigned short* path, const int path_len, unsigned short* out_path, int* out_path_len, const int path_maxlen);
extern void FileChooser_SetResolveFilenameForDisplayProc(WebCore::ResolveFilenameForDisplayProc proc);

extern WebCore::IntRect scrollbarTrackRect(Scrollbar* scrollbar);
extern void scrollbarSplitTrack(Scrollbar* scrollbar, const WebCore::IntRect& track, WebCore::IntRect& startTrack, WebCore::IntRect& thumb, WebCore::IntRect& endTrack);
extern WebCore::IntRect scrollbarBackButtonRect(Scrollbar* scrollbar, ScrollbarPart part);
extern WebCore::IntRect scrollbarForwardButtonRect(Scrollbar* scrollbar, ScrollbarPart part);
}

namespace WTF {
extern void finalizeMainThreadPlatform();
extern void initializeTCMallocWKC();
extern void finalizeTCMallocWKC();
extern void ThreadingWKC_deleteSharedInstance();
extern void ThreadingWKC_resetVariables();
}

namespace JSC { class JSGlobalData; }

extern "C" {
void libXML2_xmlmemory_resetVariables();
void libXML2_xmlIO_resetVariables();
void libXML2_xlink_resetVariables();
void libXML2_tree_resetVariables();
void libXML2_threads_resetVariables();
void libXML2_relaxng_resetVariables();
void libXML2_parser_resetVariables();
void libXML2_HTMLparser_resetVariables();
void libXML2_threads_resetVariables();
void libXML2_globals_resetVariables();
void libXML2_encoding_resetVariables();
void libXML2_error_resetVariables();
void libXML2_dict_resetVariables();
void libXML2_catalog_resetVariables();
}

using namespace WTF;

#if 0
extern "C" void GUI_Printf(const char*, ...);
# define NF4_DP(a) GUI_Printf a
#else
# define NF4_DP(a) (void(0))
#endif

#if COMPILER(MSVC) && defined(_DEBUG)
#if 0
extern "C" void GUI_Printf(const char*, ...);
# define NF4_MEM_DP LOG_ERROR
#else
# define NF4_MEM_DP(...) ((void)0)
#endif
#else
# define NF4_MEM_DP(...) ((void)0)
#endif /* COMPILER(MSVC) && defined(_DEBUG) */

namespace WKC {

// private container
WKCWebViewPrivate::WKCWebViewPrivate(WKCWebView* parent, WKCClientBuilders& builders)
     : m_parent(parent),
       m_clientBuilders(builders)
{
    m_corePage = 0;
    m_wkcCorePage = 0;
    m_mainFrame = 0;
    m_inspector = 0;
    m_dropdownlist = 0;
    m_settings = 0;

    m_offscreen = 0;
    m_drawContext = 0;

    m_isZoomFullContent = true;
    m_isTransparent = false;
    m_loadStatus = ELoadStatusNone;

    m_opticalZoomLevel = 1.f;
    WKCFloatPoint_Set(&m_opticalZoomOffset, 0, 0);

    m_encoding = 0;
    m_customEncoding = 0;

    m_focusedNode = 0;
    m_nodeFromPoint = 0;

    m_forceTerminated = false;
}

WKCWebViewPrivate::~WKCWebViewPrivate()
{
    if (m_forceTerminated) {
        return;
    }

    delete m_focusedNode;
    delete m_nodeFromPoint;

    if (m_offscreen) {
        wkcOffscreenDeletePeer(m_offscreen);
    }
    m_offscreen = 0;
    if (m_drawContext) {
        wkcDrawContextDeletePeer(m_drawContext);
    }
    m_drawContext = 0;

    if (m_encoding) {
        fastFree(m_encoding);
        m_encoding = 0;
    }
    if (m_customEncoding) {
        fastFree(m_customEncoding);
        m_customEncoding = 0;
    }

    if (m_wkcCorePage) {
        delete m_wkcCorePage;
    }
    if (m_corePage) {
        m_mainFrame->privateFrame()->core()->loader()->detachFromParent();
        delete m_corePage;
        m_corePage = 0;
    }
    if (m_dropdownlist) {
        delete m_dropdownlist;
        m_dropdownlist = 0;
    }

    if (m_inspector) {
#if !ENABLE(INSPECTOR)
        delete m_inspector;
#endif
        m_inspector = 0;
    }
    // m_mainFrame will be deleted automatically...

    if (m_settings) {
        delete m_settings;
        m_settings = 0;
    }
}

WKCWebViewPrivate*
WKCWebViewPrivate::create(WKCWebView* parent, WKCClientBuilders& builders)
{
    WKCWebViewPrivate* self = 0;
    self = new WKCWebViewPrivate(parent, builders);
    if (!self) return 0;
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
WKCWebViewPrivate::construct()
{
    WKC::ChromeClientWKC* chrome = 0;
    WKC::ContextMenuClientWKC* contextmenu = 0;
    WKC::EditorClientWKC* editor = 0;
    WKC::DragClientWKC* drag = 0;
    WebCore::Settings* settings = 0;

    chrome = WKC::ChromeClientWKC::create(this);
    if (!chrome) goto error_end;
    contextmenu = WKC::ContextMenuClientWKC::create(this);
    if (!contextmenu) goto error_end;
    editor = WKC::EditorClientWKC::create(this);
    if (!editor) goto error_end;
    drag = WKC::DragClientWKC::create(this);
    if (!drag) goto error_end;

    m_inspector = WKC::InspectorClientWKC::create(this);
    if (!m_inspector) goto error_end;
    m_corePage = new WebCore::Page(chrome, contextmenu, editor, drag, m_inspector, 0, 0);
    if (!m_corePage) goto error_end;
    m_wkcCorePage = new PagePrivate(m_corePage);

    m_mainFrame = WKC::WKCWebFrame::create(this, m_clientBuilders);
    if (!m_mainFrame) goto error_end;

    m_dropdownlist = WKC::DropDownListClientWKC::create(this);
    if (!m_dropdownlist) goto error_end;

    settings = m_corePage->settings();

    m_settings = new WKC::WKCSettings(this);
    if (!m_settings) goto error_end;

    settings->setDefaultTextEncodingName("UTF-8");
    settings->setSerifFontFamily("Times New Roman");
    settings->setFixedFontFamily("Courier New");
    settings->setSansSerifFontFamily("Arial");
    settings->setStandardFontFamily("Times New Roman");
    settings->setLoadsImagesAutomatically(true);
    settings->setShrinksStandaloneImagesToFit(true);
    settings->setShouldPrintBackgrounds(true);
    settings->setJavaScriptEnabled(true);
#if ENABLE(WKC_SPATIAL_NAVI)
    settings->setSpatialNavigationEnabled(true);
#endif
    settings->setPluginsEnabled(false);
    settings->setTextAreasAreResizable(true);
    settings->setUserStyleSheetLocation(WebCore::KURL());
    settings->setDeveloperExtrasEnabled(false);
    settings->setPrivateBrowsingEnabled(false);
    settings->setCaretBrowsingEnabled(false);
    settings->setDatabasesEnabled(false);
    settings->setLocalStorageEnabled(false);
    settings->setXSSAuditorEnabled(false);
    settings->setJavaScriptCanOpenWindowsAutomatically(false);
#if 1
    // added at webkit.org trunk r58703 (modification is changed fo this platform)
    settings->setJavaScriptCanAccessClipboard(false);
#endif
    settings->setOfflineWebApplicationCacheEnabled(false);
    settings->setEditingBehavior(WebCore::EditingWindowsBehavior);
    settings->setAllowUniversalAccessFromFileURLs(false);
    settings->setDOMPasteAllowed(false);
    settings->setNeedsSiteSpecificQuirks(false);
    settings->setUsesPageCache(false);
    settings->setDefaultFixedFontSize(14);
    settings->setDefaultFontSize(14);
    settings->setDownloadableBinaryFontsEnabled(false);
    settings->setShowDebugBorders(false);
    settings->setGeolocationEnabled(false);
    settings->setAuthorAndUserStylesEnabled(true);
#if ENABLE(WKC_PAGESAVE_MHTML)
    settings->setWebArchiveDebugModeEnabled(true);
#endif
//  settings->setAllowScriptsToCloseWindows(true);

#if 1
    WebCore::RGBA32Buffer::setInternalColorFormatRGAB5515();
#else
    WebCore::RGBA32Buffer::setInternalColorFormatRGBA8888();
#endif

//    WebCore::ImageSource::setMaxNumberOfPixels(IMAGE_DECODER_DOWN_SAMPLING_MAX_NUMBER_OF_PIXELS);

    WebCore::ImageSource::setMaxSize(WebCore::IntSize(-1,-1));

    m_corePage->setJavaScriptURLsAreAllowed(true);

    return true;

error_end:
    if (m_settings) {
        delete m_settings;
        m_settings = 0;
    }
    if (m_dropdownlist) {
        delete m_dropdownlist;
        m_dropdownlist = 0;
    }
    if (m_corePage) {
        delete m_corePage;
        m_corePage = 0;
        // m_mainFrame will be deleted automatically...
    } else {
        delete drag;
        delete editor;
        delete contextmenu;
        delete chrome;
    }
    if (m_inspector) {
        delete m_inspector;
        m_inspector = 0;
    }
    return false;
}

void
WKCWebViewPrivate::notifyForceTerminate()
{
    NF4_DP(("WKCWebViewPrivate::notifyForceTerminate\n"));
    m_forceTerminated = true;
    if (m_mainFrame) {
        m_mainFrame->notifyForceTerminate();
    }
}

WKC::WKCSettings*
WKCWebViewPrivate::settings()
{
    return m_settings;
}

WKC::Page*
WKCWebViewPrivate::wkcCore() const
{
    return &m_wkcCorePage->wkc();
}

// drawings
bool
WKCWebViewPrivate::setOffscreen(OffscreenFormat format, void* bitmap, int rowbytes, const WebCore::IntSize& desktopsize, const WebCore::IntSize& viewsize, bool fixedlayout)
{
    int pformat = 0;
    WKCSize size;

    if (m_offscreen) {
        wkcOffscreenDeletePeer(m_offscreen);
    }

    switch (format) {
    case EOffscreenFormatRGBA4444:
        pformat = WKC_OFFSCREEN_TYPE_RGBA4444;
        break;
    case EOffscreenFormatRGBA5551:
        pformat = WKC_OFFSCREEN_TYPE_RGBA5551;
        break;
    case EOffscreenFormatRGBA5650:
        pformat = WKC_OFFSCREEN_TYPE_RGBA5650;
        break;
    case EOffscreenFormatRGBA5650Tiny:
        pformat = WKC_OFFSCREEN_TYPE_RGBA5650TINY;
        break;
    case EOffscreenFormatRGBA8888:
        pformat = WKC_OFFSCREEN_TYPE_RGBA8888;
        break;
    case EOffscreenFormatRGB888:
        pformat = WKC_OFFSCREEN_TYPE_RGB888;
        break;
    case EOffscreenFormatNative:
        pformat = WKC_OFFSCREEN_TYPE_NATIVE;
        break;
    case EOffscreenFormatPolygon:
        pformat = WKC_OFFSCREEN_TYPE_POLYGON;
        break;
    default:
        return false;
    }
    size.fWidth = desktopsize.width();
    size.fHeight = desktopsize.height();
    m_offscreen = wkcOffscreenNewPeer(pformat, bitmap, rowbytes, &size);
    if (!m_offscreen) return false;

    if (m_drawContext) {
        wkcDrawContextDeletePeer(m_drawContext);
        m_drawContext = 0;
    }
    m_drawContext = wkcDrawContextNewPeer(m_offscreen);
    if (!m_drawContext) return false;

    m_desktopSize = desktopsize;
    m_defaultDesktopSize = desktopsize;
    m_defaultViewSize = viewsize;
    m_viewSize = viewsize;

    WebCore::Frame* frame = core()->mainFrame();
    if (!frame || !frame->view()) {
        return false;
    }

    if (fixedlayout) {
        frame->view()->setUseFixedLayout(true);
        frame->view()->setFixedLayoutSize(viewsize);
    } else {
        frame->view()->setUseFixedLayout(false);
    }

    frame->view()->resize(desktopsize.width(), desktopsize.height());
    frame->view()->forceLayout();
    frame->view()->adjustViewSize();
    return true;
}

void
WKCWebViewPrivate::notifyResizeDesktopSize(const WebCore::IntSize& size)
{
    m_desktopSize = size;

    WebCore::Frame* frame = core()->mainFrame();
    if (!frame || !frame->view()) {
        return;
    }

    frame->view()->resize(size.width(), size.height());
    frame->view()->forceLayout();
    frame->view()->adjustViewSize();
}

void
WKCWebViewPrivate::notifyResizeViewSize(const WebCore::IntSize& size)
{
    WebCore::Frame* frame = core()->mainFrame();
    if (!frame) return;
    WebCore::FrameView* view = frame->view();
    if (!view) return;

    m_viewSize = size;
    view->setFixedLayoutSize(size);
}

const WebCore::IntSize&
WKCWebViewPrivate::desktopSize() const
{
    return m_desktopSize;
}

const WebCore::IntSize&
WKCWebViewPrivate::viewSize() const
{
    return m_viewSize;
}

const WebCore::IntSize&
WKCWebViewPrivate::defaultDesktopSize() const
{
    return m_defaultDesktopSize;
}

const WebCore::IntSize&
WKCWebViewPrivate::defaultViewSize() const
{
    return m_defaultViewSize;
}

void
WKCWebViewPrivate::notifyRelayout()
{
    WebCore::Frame* frame = core()->mainFrame();

    if (!frame || !frame->contentRenderer() || !frame->view()) {
        return;
    }

    frame->view()->layoutIfNeededRecursive();
}

void
WKCWebViewPrivate::notifyPaintOffscreen(const WebCore::IntRect& rect)
{
    if (!m_offscreen) return;

    WebCore::Frame* frame = core()->mainFrame();

    if (!frame || !frame->contentRenderer() || !frame->view()) {
        return;
    }

    WebCore::GraphicsContext ctx((void *)m_drawContext);

    wkcOffscreenBeginPaintPeer(m_offscreen);
    WebCore::FloatRect cr(rect.x(), rect.y(), rect.width(), rect.height());
    ctx.save();
    ctx.clip(cr);
    frame->view()->paint(&ctx, rect);
    ctx.restore();
    wkcOffscreenEndPaintPeer(m_offscreen);
}

void
WKCWebViewPrivate::notifyScrollOffscreen(const WebCore::IntRect& rect, const WebCore::IntSize& diff)
{
    if (!m_offscreen) return;

    WKCRect r;
    WKCSize d;
    r.fX = rect.x();
    r.fY = rect.y();
    r.fWidth = rect.width();
    r.fHeight = rect.height();
    d.fWidth = diff.width();
    d.fHeight = diff.height();
    wkcOffscreenScrollPeer(m_offscreen, &r, &d);
}

void
WKCWebViewPrivate::setTransparent(bool flag)
{
    m_isTransparent = flag;
    WebCore::Frame* frame = core()->mainFrame();
    if (!frame) return;
    frame->view()->setTransparent(flag);
}

void
WKCWebViewPrivate::setOpticalZoom(float zoom_level, const WKCFloatPoint& offset)
{
    m_opticalZoomLevel = zoom_level;
    m_opticalZoomOffset = offset;
    wkcOffscreenSetOpticalZoomPeer(m_offscreen, zoom_level, &offset);
}

void
WKCWebViewPrivate::setUseAntiAliasForDrawings(bool flag)
{
    if (!m_offscreen) return;
    wkcOffscreenSetUseAntiAliasForPolygonPeer(m_offscreen, flag);
}

void
WKCWebViewPrivate::setUseAntiAliasForCanvas(bool flag)
{
    WebCore::ImageBufferData::setUseAA(flag);
}

void
WKCWebViewPrivate::setUseBilinearForScaledImages(bool flag)
{
    if (!m_offscreen) return;
    wkcOffscreenSetUseInterpolationForImagePeer(m_offscreen, flag);
}

void
WKCWebViewPrivate::setUseBilinearForCanvasImages(bool flag)
{
    WebCore::ImageBufferData::setUseBilinear(flag);
}


void
WKCWebViewPrivate::setScrollPositionForOffscreen(const WebCore::IntPoint& scrollPosition)
{
    if (!m_offscreen) return;
    const WKCPoint pos = { scrollPosition.x(), scrollPosition.y() };
    wkcOffscreenSetScrollPositionPeer(m_offscreen, &pos);
}

void
WKCWebViewPrivate::notifyScrollPositionChanged()
{
    WebCore::FrameView* view = m_mainFrame->privateFrame()->core()->view();
    view->scrollPositionChanged();
}

WKC::Node*
WKCWebViewPrivate::getFocusedNode()
{
    WebCore::Document* doc = core()->focusController()->focusedOrMainFrame()->document();
    WebCore::Node* node = doc->focusedNode();

    if (!node)
        return 0;

    if (!m_focusedNode || m_focusedNode->webcore()!=node) {
        delete m_focusedNode;
        m_focusedNode = NodePrivate::create(node);
    }
    return &m_focusedNode->wkc();
}

WKC::Node*
WKCWebViewPrivate::getNodeFromPoint(int x, int y)
{
    WebCore::Node* node;
    WebCore::Frame* frame = core()->mainFrame();

    while (frame) {
        WebCore::Document* doc = frame->document();
        WebCore::FrameView* view = frame->view();
        WebCore::IntPoint documentPoint = view ? view->windowToContents(WebCore::IntPoint(x, y)) : WebCore::IntPoint(x, y);
        WebCore::RenderView* renderView = doc->renderView();
        WebCore::HitTestRequest request(WebCore::HitTestRequest::ReadOnly | WebCore::HitTestRequest::Active);
        WebCore::HitTestResult result(documentPoint);

        renderView->layer()->hitTest(request, result);
        node = result.innerNode();
        while (node && !node->isElementNode())
            node = node->parentNode();
        if (node)
            node = node->shadowAncestorNode();

        frame = WebCore::EventHandler::subframeForTargetNode(node);
    }

    if (!node)
        return 0;

    if (!m_nodeFromPoint || m_nodeFromPoint->webcore()!=node) {
        delete m_nodeFromPoint;
        m_nodeFromPoint = NodePrivate::create(node);
    }
    return &m_nodeFromPoint->wkc();
}


// implementations

WKCWebView::WKCWebView()
     : m_private(0)
{
}

WKCWebView::~WKCWebView()
{
    if (m_private) {
        if (!m_private->m_forceTerminated) {
            stopLoading();
        }
        delete m_private;
        m_private = 0;
    }
}

WKCWebView*
WKCWebView::create(WKCClientBuilders& builders)
{
    WKCWebView* self = 0;

    self = new WKCWebView();
    if (!self) return 0;
    if (!self->construct(builders)) {
        delete self;
        return 0;
    }

    return self;
}

bool
WKCWebView::construct(WKCClientBuilders& builders)
{
    m_private = WKCWebViewPrivate::create(this, builders);
    if (!m_private) return false;
    return true;
}

void
WKCWebView::deleteWKCWebView(WKCWebView *self)
{
    delete self;
}

void
WKCWebView::notifyForceTerminate()
{
    if (m_private) {
        m_private->notifyForceTerminate();
    }
}

// off-screen draw

bool
WKCWebView::setOffscreen(OffscreenFormat format, void* bitmap, int rowbytes, const WKCSize& desktopsize, const WKCSize& viewsize, bool fixedlayout)
{
    WebCore::IntSize ds(desktopsize.fWidth, desktopsize.fHeight);
    WebCore::IntSize vs(viewsize.fWidth, viewsize.fHeight);
    return m_private->setOffscreen(format, bitmap, rowbytes, ds, vs, fixedlayout);
}

void
WKCWebView::notifyResizeViewSize(const WKCSize& size)
{
    WebCore::IntSize s(size.fWidth, size.fHeight);
    m_private->notifyResizeViewSize(s);
}

void
WKCWebView::notifyResizeDesktopSize(const WKCSize& size)
{
    WebCore::IntSize s(size.fWidth, size.fHeight);
    m_private->notifyResizeDesktopSize(s);
}

void
WKCWebView::notifyRelayout(bool force)
{
    if (force) {
        WebCore::Frame* frame = m_private->core()->mainFrame();
        if (!frame) return;
        WebCore::Document* document = frame->document();
        if (!document) return;
        WebCore::RenderView* renderView = document->renderView();
        if (!renderView) return;
        renderView->setNeedsLayout(true);
    }
    m_private->notifyRelayout();
}

void
WKCWebView::notifyPaintOffscreen(const WKCRect& rect)
{
    WebCore::IntRect r(rect.fX, rect.fY, rect.fWidth, rect.fHeight);
    m_private->notifyPaintOffscreen(r);
}

void
WKCWebView::notifyScrollOffscreen(const WKCRect& rect, const WKCSize& diff)
{
    WebCore::IntRect r(rect.fX, rect.fY, rect.fWidth, rect.fHeight);
    WebCore::IntSize d(diff.fWidth, diff.fHeight);
    m_private->notifyScrollOffscreen(r, d);
}

// events

bool
WKCWebView::notifyKeyPress(WKC::Key key, WKC::Modifier modifiers)
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();
    if (!frame->view()) return false;

    WKC::WKCKeyEvent ev;
    ev.m_type = WKC::EKeyEventPressed;
    ev.m_key = key;
    ev.m_modifiers = modifiers;
    WebCore::PlatformKeyboardEvent keyboardEvent((void *)&ev);

    if (frame->eventHandler()->keyEvent(keyboardEvent)) {
        return true;
    }
    return false;
}
bool
WKCWebView::notifyKeyRelease(WKC::Key key, WKC::Modifier modifiers)
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();

    if (!frame->view()) return false;

    WKC::WKCKeyEvent ev;
    ev.m_type = WKC::EKeyEventReleased;
    ev.m_key = key;
    ev.m_modifiers = modifiers;
    WebCore::PlatformKeyboardEvent keyboardEvent((void *)&ev);

    if (frame->eventHandler()->keyEvent(keyboardEvent)) {
        return true;
    }
    return false;
}
bool
WKCWebView::notifyMouseDown(const WKCPoint& pos, WKC::MouseButton button, WKC::Modifier modifiers)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WKC::WKCMouseEvent ev;

    ev.m_type = WKC::EMouseEventDown;
    ev.m_button = button;
    ev.m_x = pos.fX;
    ev.m_y = pos.fY;
    ev.m_modifiers = modifiers;
    ev.m_timestampinsec = wkcGetTickCountPeer() / 1000;
    WebCore::PlatformMouseEvent mouseEvent((void *)&ev);

    return frame->eventHandler()->handleMousePressEvent(mouseEvent);
}
bool
WKCWebView::notifyMouseUp(const WKCPoint& pos, WKC::MouseButton button, Modifier modifiers)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WKC::WKCMouseEvent ev;

    ev.m_type = WKC::EMouseEventUp;
    ev.m_button = button;
    ev.m_x = pos.fX;
    ev.m_y = pos.fY;
    ev.m_modifiers = modifiers;
    ev.m_timestampinsec = wkcGetTickCountPeer() / 1000;
    WebCore::PlatformMouseEvent mouseEvent((void *)&ev);

    return frame->eventHandler()->handleMouseReleaseEvent(mouseEvent);
}
bool
WKCWebView::notifyMouseMove(const WKCPoint& pos, WKC::MouseButton button, Modifier modifiers)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    if (!frame->view()) return false;

    WKC::WKCMouseEvent ev;

    ev.m_type = WKC::EMouseEventMove;
    ev.m_button = button;
    ev.m_x = pos.fX;
    ev.m_y = pos.fY;
    ev.m_modifiers = modifiers;
    ev.m_timestampinsec = wkcGetTickCountPeer() / 1000;
    WebCore::PlatformMouseEvent mouseEvent((void *)&ev);

    return frame->eventHandler()->mouseMoved(mouseEvent);
}
bool
WKCWebView::notifyMouseDoubleClick(const WKCPoint& pos, WKC::MouseButton button, WKC::Modifier modifiers)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WKC::WKCMouseEvent ev;

    ev.m_type = WKC::EMouseEventDoubleClick;
    ev.m_button = button;
    ev.m_x = pos.fX;
    ev.m_y = pos.fY;
    ev.m_modifiers = modifiers;
    ev.m_timestampinsec = wkcGetTickCountPeer() / 1000;
    WebCore::PlatformMouseEvent mouseEvent((void *)&ev);

    return frame->eventHandler()->handleMousePressEvent(mouseEvent);
}
bool
WKCWebView::notifyScroll(WKC::ScrollType type)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WebCore::ScrollDirection dir = WebCore::ScrollUp;
    WebCore::ScrollGranularity gra = WebCore::ScrollByLine;

    switch (type) {
    case EScrollUp:
        gra = WebCore::ScrollByLine;
        dir = WebCore::ScrollUp;
        break;
    case EScrollDown:
        gra = WebCore::ScrollByLine;
        dir = WebCore::ScrollDown;
        break;
    case EScrollLeft:
        gra = WebCore::ScrollByLine;
        dir = WebCore::ScrollLeft;
        break;
    case EScrollRight:
        gra = WebCore::ScrollByLine;
        dir = WebCore::ScrollRight;
        break;
    case EScrollPageUp:
        gra = WebCore::ScrollByPage;
        dir = WebCore::ScrollUp;
        break;
    case EScrollPageDown:
        gra = WebCore::ScrollByPage;
        dir = WebCore::ScrollDown;
        break;
    case EScrollPageLeft:
        gra = WebCore::ScrollByPage;
        dir = WebCore::ScrollLeft;
        break;
    case EScrollPageRight:
        gra = WebCore::ScrollByPage;
        dir = WebCore::ScrollRight;
        break;
    case EScrollTop:
        gra = WebCore::ScrollByDocument;
        dir = WebCore::ScrollUp;
        break;
    case EScrollBottom:
        gra = WebCore::ScrollByDocument;
        dir = WebCore::ScrollDown;
        break;
    default:
        return false;
    }

    if (!frame->eventHandler()->scrollOverflow(dir, gra)) {
        frame->view()->scroll(dir, gra);
        return true;
    }
    return false;
}

bool
WKCWebView::notifyScroll(int dx, int dy)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();

    frame->view()->scrollBy(WebCore::IntSize(dx, dy));
    return true;
}

bool
WKCWebView::notifyScrollTo(int x, int y)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();

    frame->view()->setScrollPosition(WebCore::IntPoint(x, y));
    return true;
}

void
WKCWebView::scrollPosition(WKCPoint& pos)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();

    WebCore::IntPoint p = frame->view()->scrollPosition();
    pos.fX = p.x();
    pos.fY = p.y();
}

void
WKCWebView::contentsSize(WKCSize& size)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();

    WebCore::IntSize s = frame->view()->contentsSize();
    size.fWidth = s.width();
    size.fHeight = s.height();
}

void
WKCWebView::notifyFocusIn()
{
    WebCore::FocusController* focusController = m_private->core()->focusController();

    focusController->setActive(true);

    if (focusController->focusedFrame()) {
        focusController->setFocused(true);
    } else {
        focusController->setFocused(true);
        focusController->setFocusedFrame(m_private->core()->mainFrame());
    }
}

void
WKCWebView::notifyFocusOut()
{
    m_private->core()->focusController()->setActive(false);
    m_private->core()->focusController()->setFocused(false);
}

void
WKCWebView::notifyScrollPositionChanged()
{
    m_private->notifyScrollPositionChanged();
}

WKC::Node*
WKCWebView::findFocusableNode(const WKC::FocusDirection direction, const WKCRect* specificRect)
{
#if ENABLE(WKC_SPATIAL_NAVI)
    return m_private->wkcCore()->focusController()->findNextFocusableNode(direction, specificRect);
#else
    return 0;
#endif
}

WKC::Node*
WKCWebView::findFocusableNodeInRect(const WKC::FocusDirection direction, const WKCRect* rect)
{
#if ENABLE(WKC_SPATIAL_NAVI)
    return m_private->wkcCore()->focusController()->findNextFocusableNodeInRect(direction, m_private->m_mainFrame->core(), rect);
#else
    return 0;
#endif
}

bool
WKCWebView::setFocusedNode(WKC::Node* inode)
{
#if ENABLE(WKC_SPATIAL_NAVI)
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();
    WebCore::Document* newDocument = 0;
    WebCore::Node* node = inode ? inode->priv().webcore() : 0;
    if (node && frame) {
        WebCore::Document* focusedDocument = frame->document();
        newDocument = node->document();
        if (newDocument != focusedDocument) {
            focusedDocument->setFocusedNode(0);
        }
        if (newDocument)
            m_private->core()->focusController()->setFocusedFrame(newDocument->frame());
    }
    if (newDocument) {
        return newDocument->setFocusedNode(node);
    }
    return m_private->core()->focusController()->focusedOrMainFrame()->document()->setFocusedNode(node);
#else
    return false;
#endif
}

void
WKCWebView::notifySuspend()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return;
}

void
WKCWebView::notifyResume()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return;
}

// APIs
const unsigned short*
WKCWebView::title()
{
    return m_private->m_mainFrame->title();
}
const char*
WKCWebView::uri()
{
    return m_private->m_mainFrame->uri();
}

void
WKCWebView::setMaintainsBackForwardList(bool flag)
{
    m_private->core()->backForwardList()->setEnabled(flag);
}

void
WKCWebView::addHistoryItem(const char* uri, const unsigned short* title)
{
    WebCore::BackForwardList* list;

    list = m_private->core()->backForwardList();
    if (!list || !list->enabled())
        return;

    RefPtr<WebCore::HistoryItem> item = WebCore::HistoryItem::create(WebCore::KURL(WebCore::KURL(), WebCore::String::fromUTF8(uri)), WebCore::String(title), 0);
    list->addItem(item.release());
}

unsigned int
WKCWebView::getHistoryLength()
{
    WebCore::BackForwardList* list;

    list = m_private->core()->backForwardList();
    if (!list || !list->enabled())
        return 0;

    return list->entries().size();
}

bool
WKCWebView::getHistoryCurrentIndex(unsigned int& index)
{
    WebCore::BackForwardList* list;

    list = m_private->core()->backForwardList();
    if (!list || !list->enabled())
        return false;

    if (getHistoryLength() == 0)
        return false;

    index = list->backListCount();
    return true;
}

bool
WKCWebView::getHistoryIndexByItem(WKC::HistoryItem* item, unsigned int& index)
{
    unsigned int i;
    WebCore::BackForwardList* list;

    list = m_private->core()->backForwardList();
    if (!list || !list->enabled())
        return false;

    WebCore::HistoryItemVector& historyItems = list->entries();
    if (!historyItems.size() || !item)
        return false;
        

    for (i=0; i < historyItems.size(); ++i) {
        if (historyItems[i] == item->priv().webcore()) {
            index = i;
            return true;
        }
    }

    return false;
}

void
WKCWebView::removeHistoryItemByIndex(unsigned int index)
{
    WebCore::BackForwardList* list;

    list = m_private->core()->backForwardList();
    if (!list || !list->enabled())
        return;

    unsigned int current;
    if (!getHistoryCurrentIndex(current))
        return;

    WebCore::HistoryItem* item = list->itemAtIndex(index - current);
    if (!item)
        return;
    list->removeItem(item);
}
 
bool
WKCWebView::getHistoryItemByIndex(unsigned int index, char* const uri, unsigned int& uriLen, unsigned short* const title, unsigned int& titleLen)
{
    WebCore::BackForwardList* list;

    list = m_private->core()->backForwardList();
    if (!list || !list->enabled())
        return false;

    unsigned int current;
    if (!getHistoryCurrentIndex(current))
        return false;

    WebCore::HistoryItem* item = list->itemAtIndex(index - current);
    if (!item)
        return false;

    if (uri) {
        strncpy(uri, item->urlString().utf8().data(), item->urlString().utf8().length());
        *(uri + item->urlString().utf8().length()) = 0;
    } else
        uriLen = item->urlString().utf8().length();

    if (title)
        wkc_wstrncpy(title, 0, item->title().characters(), item->title().length());
    else
        titleLen = item->title().length();

    return true;
}

void
WKCWebView::gotoHistoryItemByIndex(unsigned int index)
{
    WebCore::BackForwardList* list;

    list = m_private->core()->backForwardList();
    if (!list || !list->enabled())
        return;

    unsigned int current;
    if (!getHistoryCurrentIndex(current))
        return;

    // Following is a part of implementation of Page::goBackOrForward(int distance)
    // except 0 check of distance.
    // Currently we need to do goToItem even if distance is 0.
    int distance = index - current;
    WebCore::HistoryItem* item = list->itemAtIndex(distance);
    if (!item) {
        if (distance > 0) {
            int forwardListCount = list->forwardListCount();
            if (forwardListCount > 0) 
                item = list->itemAtIndex(forwardListCount);
        } else {
            int backListCount = list->backListCount();
            if (backListCount > 0)
                item = list->itemAtIndex(-backListCount);
        }
    }
    if (!item)
        return;

    m_private->core()->goToItem(item, WebCore::FrameLoadTypeIndexedBackForward);
}

bool
WKCWebView::canGoBack()
{
    NF4_DP(("WKCWebView::canGoBack Enter\n"));

    if (!m_private->core()) {
        NF4_DP(("WKCWebView::canGoBack Exit 1\n"));
        return false;
    }
    if (!m_private->core()->backForwardList()->backItem()) {
        NF4_DP(("WKCWebView::canGoBack Exit 2\n"));
        return false;
    }

    NF4_DP(("WKCWebView::canGoBack Exit 3\n"));
    return true;
}
bool
WKCWebView::canGoBackOrForward(int steps)
{
    return m_private->core()->canGoBackOrForward(steps);
}
bool
WKCWebView::canGoForward()
{
    if (!m_private->core()) {
        return false;
    }
    if (!m_private->core()->backForwardList()->forwardItem()) {
        return false;
    }
    return true;
}
bool
WKCWebView::goBack()
{
    return m_private->core()->goBack();
}
void
WKCWebView::goBackOrForward(int steps)
{
    m_private->core()->goBackOrForward(steps);
}
bool
WKCWebView::goForward()
{
    return m_private->core()->goForward();
}

void
WKCWebView::stopLoading()
{
    WebCore::Frame* frame = 0;
    WebCore::FrameLoader* loader = 0;
    frame = m_private->core()->mainFrame();
    if (!frame) return;
    loader = frame->loader();
    if (!loader) return;
    loader->stopForUserCancel();
}
void
WKCWebView::reload()
{
    m_private->core()->mainFrame()->loader()->reload();
}
void
WKCWebView::reloadBypassCache()
{
    m_private->core()->mainFrame()->loader()->reload(true);
}
void
WKCWebView::loadURI(const char* uri)
{
    if (!uri) return;
    if (!uri[0]) return;
    if (!m_private) return;

    WKCWebFrame* frame = m_private->m_mainFrame;
    frame->loadURI(uri);
}
void
WKCWebView::loadString(const char* content, const unsigned short* mimetype, const unsigned short* encoding, const char* base_uri)
{
    if (!content) return;
    if (!content[0]) return;
    if (!m_private) return;

    WKCWebFrame* frame = m_private->m_mainFrame;
    frame->loadString(content, mimetype, encoding, base_uri);
}
void
WKCWebView::loadHTMLString(const char* content, const char* base_uri)
{
    static const unsigned short cTextHtml[] = {'t','e','x','t','/','h','t','m','l',0};
    loadString(content, cTextHtml, 0, base_uri);
}

bool
WKCWebView::searchText(const unsigned short* text, bool case_sensitive, bool forward, bool wrap)
{
    WebCore::TextCaseSensitivity ts = WebCore::TextCaseSensitive;
    WebCore::FindDirection dir = WebCore::FindDirectionForward;

    if (case_sensitive) {
        ts = WebCore::TextCaseSensitive;
    } else {
        ts = WebCore::TextCaseInsensitive;
    }
    if (forward) {
        dir = WebCore::FindDirectionForward;
    } else {
        dir = WebCore::FindDirectionBackward;
    }

    return m_private->core()->findString(WebCore::String(text), ts, dir, wrap);
}
unsigned int
WKCWebView::markTextMatches(const unsigned short* string, bool case_sensitive, unsigned int limit)
{
    WebCore::TextCaseSensitivity ts = WebCore::TextCaseSensitive;

    if (case_sensitive) {
        ts = WebCore::TextCaseSensitive;
    } else {
        ts = WebCore::TextCaseInsensitive;
    }

    return m_private->core()->markAllMatchesForText(WebCore::String(string), ts, false, limit);
}
void
WKCWebView::setHighlightTextMatches(bool highlight)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();

    do {
        frame->setMarkedTextMatchesAreHighlighted(highlight);
        frame = frame->tree()->traverseNextWithWrap(false);
    } while (frame);
}
void
WKCWebView::unmarkTextMatches()
{
    m_private->core()->unmarkAllTextMatches();
}

static WKCWebFrame*
kit(WebCore::Frame* coreFrame)
{
    if (!coreFrame)
      return 0;

    ASSERT(coreFrame->loader());
    FrameLoaderClientWKC* client = static_cast<FrameLoaderClientWKC*>(coreFrame->loader()->client());
    return client ? client->webFrame() : 0;
}

WKCWebFrame*
WKCWebView::mainFrame()
{
    return m_private->m_mainFrame;
}
WKCWebFrame*
WKCWebView::focusedFrame()
{
    WebCore::Frame* focusedFrame = m_private->core()->focusController()->focusedFrame();
    return kit(focusedFrame);
}

void
WKCWebView::executeScript(const char* script)
{
    m_private->core()->mainFrame()->script()->executeScript(WebCore::String::fromUTF8(script), true);
}

void
WKCWebView::setJavaScriptURLsAreAllowed(bool flag)
{
    m_private->core()->setJavaScriptURLsAreAllowed(flag);
}

bool
WKCWebView::canCutClipboard()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return false;
}
bool
WKCWebView::canCopyClipboard()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return false;
}
bool
WKCWebView::canPasteClipboard()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return false;
}
void
WKCWebView::cutClipboard()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
}
void
WKCWebView::copyClipboard()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
}
void
WKCWebView::pasteClipboard()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
}
void
WKCWebView::deleteSelection()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
}

bool
WKCWebView::hasSelection()
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();
    return (frame->selection()->start() != frame->selection()->end());
}

void
WKCWebView::clearSelection()
{
    for (WebCore::Frame* frame = m_private->core()->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
        frame->selection()->clear();
    }
}

static bool
_selectionRects(WebCore::Page* page, WTF::Vector<WebCore::IntRect>& rects, bool textonly, bool useSelectionHeight)
{
//    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();
    WebCore::Frame* frame = page->focusController()->focusedOrMainFrame();
    RefPtr<WebCore::Range> range = frame->selection()->toNormalizedRange();
    if (!range) {
        return false; 
    }
    WebCore::Node* startContainer = range->startContainer();
    WebCore::Node* endContainer = range->endContainer();

    if (!startContainer || !endContainer)
        return false;

    WebCore::Node* stopNode = range->pastLastNode();
    for (WebCore::Node* node = range->firstNode(); node != stopNode; node = node->traverseNextNode()) {
        WebCore::RenderObject* r = node->renderer();
        if (!r)
            continue;
        bool istext = r->isText();
        if (textonly && !istext)
            continue;
        if (istext) {
            WebCore::RenderText* renderText = WebCore::toRenderText(r);
            int startOffset = node == startContainer ? range->startOffset() : 0;
            int endOffset = node == endContainer ? range->endOffset() : std::numeric_limits<int>::max();
            renderText->absoluteRectsForRange(rects, startOffset, endOffset, useSelectionHeight);
        } else {
            WebCore::FloatPoint absPos = r->localToAbsolute();
            r->absoluteRects(rects, absPos.x(), absPos.y());
        }
    }

    /* adjust the rect with visible content rect */
    WebCore::IntRect visibleContentRect = frame->view()->visibleContentRect();
    for (size_t i = 0; i < rects.size(); ++i) {
        rects[i] = WebCore::intersection(rects[i], visibleContentRect);
        /* remove empty rect */
        if (rects[i].isEmpty()) {
            rects.remove(i);
            i--;
        }
    }

    return true;
}

WKCRect
WKCWebView::selectionBoundingBox(bool textonly, bool useSelectionHeight)
{
    WebCore::IntRect result;
    Vector<WebCore::IntRect> rects;

    _selectionRects(m_private->core(), rects, textonly, useSelectionHeight);
    const size_t n = rects.size();
    for (size_t i = 0; i < n; ++i)
        result.unite(rects[i]);

    WKCRect r = { result.x(), result.y(), result.width(), result.height() };
    return r;
}

const unsigned short*
WKCWebView::selectionText()
{
    WebCore::Frame* frame = m_private->core()->focusController()->focusedOrMainFrame();
    RefPtr<WebCore::Range> range = frame->selection()->toNormalizedRange();
    if (!range) {
        return 0; 
    }

    return range->text().charactersWithNullTermination();
}

void
WKCWebView::selectAll()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
}

bool
WKCWebView::editable()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return false;
}
void
WKCWebView::setEditable(bool flag)
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
}

WKC::Page*
WKCWebView::core()
{
    return m_private->wkcCore();
}

WKC::WKCSettings*
WKCWebView::settings()
{
    return m_private->settings();
}

WKCWebInspector*
WKCWebView::inspector()
{
#if 1
    // Ugh!: do something!
    // 100107 ACCESS Co.,Ltd.
    notImplemented();
    return 0;
#else
    return m_private->m_inspector;
#endif
}

bool
WKCWebView::canShowMimeType(const unsigned short* mime_type)
{
    WebCore::Frame* frame = 0;
    WebCore::FrameLoader* loader = 0;
    frame = m_private->core()->mainFrame();
    loader = frame->loader();
    if (loader) {
        return loader->canShowMIMEType(WebCore::String(mime_type));
    }
    return false;
}

bool
WKCWebView::transparent()
{
    return m_private->transparent();
}
void
WKCWebView::setTransparent(bool flag)
{
    m_private->setTransparent(flag);
}

float
WKCWebView::zoomLevel()
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    if (!frame) return 1.0f;
    return frame->zoomFactor();
}
float
WKCWebView::setZoomLevel(float zoom_level)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    if (frame) {
        wkcOffscreenClearGlyphCachePeer();
        frame->setZoomFactor(zoom_level, !m_private->m_isZoomFullContent);
    }
    return zoom_level;
}
void
WKCWebView::zoomIn(float ratio)
{
    float cur = 0;
    cur = zoomLevel();
    setZoomLevel(cur + ratio);
}
void
WKCWebView::zoomOut(float ratio)
{
    float cur = 0;
    cur = zoomLevel();
    setZoomLevel(cur - ratio);
}
bool
WKCWebView::fullContentZoom()
{
    return m_private->m_isZoomFullContent;
}
void
WKCWebView::setFullContentZoom(bool full_content_zoom)
{
    float cur = 0;
    cur = zoomLevel();
    if (m_private->m_isZoomFullContent == full_content_zoom) {
        return;
    }
    m_private->m_isZoomFullContent = full_content_zoom;
    setZoomLevel(cur);
}

float
WKCWebView::opticalZoomLevel() const
{
    return m_private->opticalZoomLevel();
}
const WKCFloatPoint&
WKCWebView::opticalZoomOffset() const
{
    return m_private->opticalZoomOffset();
}

float
WKCWebView::setOpticalZoom(float zoom_level, const WKCFloatPoint& offset)
{
    m_private->setOpticalZoom(zoom_level, offset);
    return zoom_level;
}

const unsigned short*
WKCWebView::encoding()
{
    WebCore::String encoding = m_private->core()->mainFrame()->loader()->encoding();

    if (encoding.isEmpty()) {
        return 0;
    }
    if (m_private->m_encoding) {
        fastFree(m_private->m_encoding);
        m_private->m_encoding = 0;
    }
    m_private->m_encoding = wkc_wstrdup(encoding.charactersWithNullTermination());
    return m_private->m_encoding;
}
void
WKCWebView::setCustomEncoding(const unsigned short* encoding)
{
    m_private->core()->mainFrame()->loader()->reloadWithOverrideEncoding(WebCore::String(encoding));
}
const unsigned short*
WKCWebView::customEncoding()
{
    WebCore::String overrideEncoding = m_private->core()->mainFrame()->loader()->documentLoader()->overrideEncoding();

    if (overrideEncoding.isEmpty()) {
        return 0;
    }
    if (m_private->m_customEncoding) {
        fastFree(m_private->m_customEncoding);
        m_private->m_customEncoding = 0;
    }
    m_private->m_customEncoding = wkc_wstrdup(overrideEncoding.charactersWithNullTermination());
    return m_private->m_customEncoding;
}

WKC::LoadStatus
WKCWebView::loadStatus()
{
    return m_private->m_loadStatus;
}
double
WKCWebView::progress()
{
    return m_private->core()->progress()->estimatedProgress();
}

void
WKCWebView::undo()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
}
bool
WKCWebView::canUndo()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return false;
}
void
WKCWebView::redo()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
}
bool
WKCWebView::canRedo()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return false;
}

void
WKCWebView::setViewSourceMode(bool mode)
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
}
bool
WKCWebView::viewSourceMode()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return false;
}

WKCHitTestResult*
WKCWebView::hitTestResult()
{

    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return 0;
}

void
WKCWebView::setCacheCapacities(unsigned int min_dead_resource, unsigned int max_dead_resource, unsigned int total)
{
    unsigned int min_dead = min_dead_resource;
    unsigned int max_dead = max_dead_resource;

    if (min_dead > total) {
        min_dead = total;
    }
    if (max_dead > total) {
        max_dead = total;
    }

    WebCore::cache()->setCapacities(min_dead, max_dead, total);
}

void
WKCWebView::setDeadDecodedDataDeletionInterval(double interval)
{
    WebCore::cache()->setDeadDecodedDataDeletionInterval(interval);
}

void
WKCWebView::cachedSize(unsigned int& dead_resource, unsigned int& live_resource)
{
    live_resource = WebCore::cache()->liveSize();
    dead_resource = WebCore::cache()->deadSize();
}

void
WKCWebView::setCacheModel(WKC::CacheModel model, unsigned int itotal)
{
    unsigned int total = 0;
    unsigned int mdead_cap = 0;
    unsigned int max_dead_cap = 0;
    double deadDecodedDataDeletionInterval = 0;
    unsigned int pagecache_cap = 0;

    switch (model) {
    case WKC::ECacheModelDocumentViewer:
        total = itotal;
        mdead_cap = 0;
        max_dead_cap = 0;
        deadDecodedDataDeletionInterval = 0;
        pagecache_cap = 0;
        break;
    case WKC::ECacheModelWebBrowser:
        pagecache_cap = 3;
        total = itotal;
        mdead_cap = total / 4;
        max_dead_cap = total / 2;
        deadDecodedDataDeletionInterval = 60;
        break;
    default:
        return;
    }

    WebCore::cache()->setCapacities(mdead_cap, max_dead_cap, total);
    WebCore::cache()->setDeadDecodedDataDeletionInterval(deadDecodedDataDeletionInterval);
    WebCore::pageCache()->setCapacity(pagecache_cap);
}

void
WKCWebView::clearCaches()
{
    if (wkcMemoryGetAllocationStatePeer() == WKC_MEMORYALLOC_TYPE_ANIMEGIF) {
        /* The GIF-decoder is deleted in use. It cause the access violation. */
        return;
    }

    wkcMemorySetBusyAreaPeer(WKC_MEMORY_BUSYAREA_CLEARCACHE);
    /* clear conetnts cahces */
    WebCore::cache()->setCapacities(0, 0, 0);

    /* other caches */
    WebCore::fontCache()->purgeInactiveFontData();
    WebCore::CrossOriginPreflightResultCache::shared().empty();
    wkcMemorySetBusyAreaPeer(WKC_MEMORY_BUSYAREA_NONE);
}

void 
WKCWebView::setMinDelayBeforeLiveDecodedPruneCaches(double delay)
{
    WebCore::Cache::setMinDelayBeforeLiveDecodedPrune(delay);
}

void
WKCWebView::setProxy(bool enable, const char* host, int port, bool isHTTP10, const char* proxyuser, const char* proxypass, ProxyAuth auth)
{
   WebCore::ResourceHandleManager::ProxyType  proxy_type = WebCore::ResourceHandleManager::HTTP;

    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        if (enable) {
            const char* user = "";
            const char* pass = "";
            WebCore::ResourceHandleManager::ProxyAuth proxyAuth;
            if (proxyuser) user = proxyuser;
            if (proxypass) pass = proxypass;
            switch (auth) {
            case Digest:
                proxyAuth = WebCore::ResourceHandleManager::Digest;
                break;
            case NTLM:
                proxyAuth = WebCore::ResourceHandleManager::NTLM;
                break;
            case Basic:
                proxyAuth = WebCore::ResourceHandleManager::Basic;
                break;
            default:
                proxyAuth = WebCore::ResourceHandleManager::NONE;
                break;
            }
            if (isHTTP10)
                proxy_type = WebCore::ResourceHandleManager::HTTP10;
            mgr->setProxyInfo(host, port, proxy_type,
                              user, pass, proxyAuth);
        } else {
            mgr->setProxyInfo("", 0, proxy_type,
                              "", "", WebCore::ResourceHandleManager::NONE);
        }
    }
}

void
WKCWebView::setMaxTCPConnections(long num)
{
    using WebCore::ResourceHandleManager;
    if (WebCore::ResourceHandleManager* mgr = WebCore::ResourceHandleManager::sharedInstance()) {
        mgr->setMaxTCPConnections(num);
    }
}

unsigned int
WKCWebView::getRSSLinkNum()
{
    WebCore::Document* doc = m_private->core()->mainFrame()->document();
    RefPtr<WebCore::NodeList> list = doc->getElementsByTagName("link");
    unsigned len = list.get()->length();
    unsigned int ret = 0;
    
    if (len > 0) {
        for (unsigned i = 0; i < len; i++) {
            WebCore::HTMLLinkElement* link = static_cast<WebCore::HTMLLinkElement*>(list.get()->item(i));
            if (!link->getAttribute("href").isEmpty() && !link->getAttribute("href").isNull()) {
                if (equalIgnoringCase(link->getAttribute("rel"), "alternate")) {
                    if (equalIgnoringCase(link->getAttribute("type"), "application/rss+xml")
                        || equalIgnoringCase(link->getAttribute("type"), "application/atom+xml")) {
                        ret ++;
                    }
                }
            }
        }
    }

    return ret;
}

unsigned int
WKCWebView::getRSSLinkInfo(WKCRSSLinkInfo* info, unsigned int info_len)
{
    WebCore::Document* doc = m_private->core()->mainFrame()->document();
    RefPtr<WebCore::NodeList> list = doc->getElementsByTagName("link");
    unsigned int len = list.get()->length();
    unsigned int count = 0;

    if (info_len == 0)
        return 0;

    for (unsigned i = 0; i < len; i++) {
        WebCore::HTMLLinkElement* link = static_cast<WebCore::HTMLLinkElement*>(list.get()->item(i));
        if (!link->getAttribute("href").isEmpty() && !link->getAttribute("href").isNull()) {
            if (equalIgnoringCase(link->getAttribute("rel"), "alternate")) {
                if (equalIgnoringCase(link->getAttribute("type"), "application/rss+xml")
                    || equalIgnoringCase(link->getAttribute("type"), "application/atom+xml")) {
                    /* get title and href attributes from DOM */   
                    (info + count)->m_flag = ERSSLinkFlagNone;
                    int title_len = link->getAttribute("title").length();
                    if (title_len > ERSSTitleLenMax) {
                        title_len = ERSSTitleLenMax;
                        (info + count)->m_flag |= ERSSLinkFlagTitleTruncate;
                    }
                    wkc_wstrncpy((info + count)->m_title, ERSSTitleLenMax, link->getAttribute("title").characters(), title_len);

                    WebCore::KURL url = doc->completeURL(link->getAttribute("href").string());
                    int url_len = url.string().utf8().length();
                    if (url_len > ERSSUrlLenMax) {
                        url_len = ERSSUrlLenMax;
                        (info + count)->m_flag |= ERSSLinkFlagUrlTruncate;
                    }
                    strncpy((info + count)->m_url, url.string().utf8().data(), url_len);
                    (info + count)->m_url[url_len] = 0;
                    
                    count ++;
                    if (count == info_len)
                        break;
                }
            }
        }
    }

    return count;
}

WKC::Node*
WKCWebView::getFocusedNode()
{
    return m_private->getFocusedNode();
}

WKC::Node*
WKCWebView::getNodeFromPoint(int x, int y)
{
    return m_private->getNodeFromPoint(x,y);
}

bool
WKCWebView::clickableFromPoint(int x, int y)
{
    WKC::Node* pnode = getNodeFromPoint(x, y);

    if (pnode) {
        WebCore::Node* node = (WebCore::Node *)pnode->priv().webcore();
        if (node->hasEventListeners(WebCore::eventNames().clickEvent)) {
            return true;
        }
    }
    return false;
}

bool
WKCWebView::draggableFromPoint(int x, int y)
{
    WKC::Node* pnode = getNodeFromPoint(x, y);
    WebCore::Node* node = (WebCore::Node *)pnode->priv().webcore();
    bool hasmousedown = false;
    while (node) {
        if (!hasmousedown)
            hasmousedown = node->hasEventListeners(WebCore::eventNames().mousedownEvent);
        if (node->hasEventListeners(WebCore::eventNames().dragEvent)
         || node->hasEventListeners(WebCore::eventNames().dragstartEvent)
         || node->hasEventListeners(WebCore::eventNames().dragendEvent)
         // In some contents there is mousemove event handler in parent node 
         // and mousedown event handler in child node. So we treat this node
         // as draggable.
         || (hasmousedown && node->hasEventListeners(WebCore::eventNames().mousemoveEvent))) {
            return true;
        }
        node = node->parentNode();
    }
    return false;
}

static bool
getScrollbarPartFromPoint(int x, int y, WebCore::Scrollbar* scrollbar, WebCore::FrameView* view, bool isFrameScrollbar, WKCWebView::ScrollbarPart& part, WebCore::IntRect& rect)
{
    WebCore::IntPoint mousePosition = scrollbar->convertFromContainingWindow(WebCore::IntPoint(x, y));
    mousePosition.move(scrollbar->x(), scrollbar->y());

    if (!scrollbar->frameRect().contains(mousePosition))
        return false;

    WebCore::IntRect track = WebCore::scrollbarTrackRect(scrollbar);
    if (track.contains(mousePosition)) {
        WebCore::IntRect beforeThumbRect;
        WebCore::IntRect thumbRect;
        WebCore::IntRect afterThumbRect;
        WebCore::scrollbarSplitTrack(scrollbar, track, beforeThumbRect, thumbRect, afterThumbRect);
        if (thumbRect.contains(mousePosition)) {
            part = WKCWebView::ThumbPart;
            rect = thumbRect;
        } else if (beforeThumbRect.contains(mousePosition)) {
            part = WKCWebView::BackTrackPart;
            rect = beforeThumbRect;
        } else if (afterThumbRect.contains(mousePosition)) {
            part = WKCWebView::ForwardTrackPart;
            rect = afterThumbRect;
        } else {
            part = WKCWebView::TrackBGPart;
            rect = track;
        }
    } else if (WebCore::scrollbarBackButtonRect(scrollbar, WebCore::BackButtonStartPart).contains(mousePosition)) {
        part = WKCWebView::BackButtonPart;
        rect = WebCore::scrollbarBackButtonRect(scrollbar, WebCore::BackButtonStartPart);
    } else if (WebCore::scrollbarBackButtonRect(scrollbar, WebCore::BackButtonEndPart).contains(mousePosition)) {
        part = WKCWebView::BackButtonPart;
        rect = WebCore::scrollbarBackButtonRect(scrollbar, WebCore::BackButtonEndPart);
    } else if (WebCore::scrollbarForwardButtonRect(scrollbar, WebCore::ForwardButtonStartPart).contains(mousePosition)) {
        part = WKCWebView::ForwardButtonPart;
        rect = WebCore::scrollbarForwardButtonRect(scrollbar, WebCore::ForwardButtonStartPart);
    } else if (WebCore::scrollbarForwardButtonRect(scrollbar, WebCore::ForwardButtonEndPart).contains(mousePosition)) {
        part = WKCWebView::ForwardButtonPart;
        rect = WebCore::scrollbarForwardButtonRect(scrollbar, WebCore::ForwardButtonEndPart);
    } else {
        part = WKCWebView::ScrollbarBGPart;
        rect = scrollbar->frameRect();
    }

    if (isFrameScrollbar)
        rect = view->convertToContainingWindow(rect);
    else
        rect = view->contentsToWindow(rect);

    return true;
}

static bool
isScrollbarOfFrameView(int x, int y, WebCore::FrameView* view, WKCWebView::ScrollbarPart& part, WKCRect& ir)
{
    WebCore::Scrollbar* scrollbar = view->verticalScrollbar();
    WebCore::IntRect rect(ir.fX, ir.fY, ir.fWidth, ir.fHeight);

    if (scrollbar) {
        if (getScrollbarPartFromPoint(x, y, scrollbar, view, true, part, rect)) {
            ir = rect;
            return true;
        }
    }
    scrollbar = view->horizontalScrollbar();
    if (scrollbar) {
        if (getScrollbarPartFromPoint(x, y, scrollbar, view, true, part, rect)) {
            ir = rect;
            return true;
        }
    }

    return false;
}

bool
WKCWebView::isScrollbarFromPoint(int x, int y, ScrollbarPart& part, WKCRect& ir)
{
    WebCore::Scrollbar* scrollbar = 0;
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WebCore::FrameView* view = 0;
    WebCore::IntRect rect(ir.fX, ir.fY, ir.fWidth, ir.fHeight);

    part = NoPart;
    rect = WebCore::IntRect();

    while (frame) {
        WebCore::Document* doc = frame->document();
        view = frame->view();
        if (!view)
            return false;
        WebCore::IntPoint documentPoint = view->windowToContents(WebCore::IntPoint(x, y));
        WebCore::RenderView* renderView = doc->renderView();
        WebCore::HitTestRequest request(WebCore::HitTestRequest::ReadOnly | WebCore::HitTestRequest::Active);
        WebCore::HitTestResult result(documentPoint);

        renderView->layer()->hitTest(request, result);
        scrollbar = result.scrollbar();

        // Following is a part of MouseEventWithHitTestResults::targetNode() in order to get targetNode for subframe
        WebCore::Node* node = result.innerNode();
        if (!node)
            return false;
        if (!node->inDocument()) {
            WebCore::Element* element = node->parentElement();
            if (element && element->inDocument())
                node = element;
        }

        // Check if there is subframe
        frame = WebCore::EventHandler::subframeForTargetNode(node);
    }

    if (scrollbar)
        return getScrollbarPartFromPoint(x, y, scrollbar, view, false, part, rect);
    else
        return isScrollbarOfFrameView(x, y , view, part, ir);
}

WKCRect
WKCWebView::zoomRegionForPoint(const WKCPoint& ihit, int minwidth)
{
    WebCore::Frame* frame = m_private->core()->mainFrame();
    WebCore::FrameView* view = 0;
    WebCore::Node* foundContainer = 0;
    WKCRect r = {0,0};
    WebCore::IntPoint hit(ihit.fX,ihit.fY);

    while (frame) {
        view = frame->view();
        if (!view)
            return r;
        WebCore::IntPoint documentPoint = view->windowToContents(hit);
        WebCore::Document* doc = frame->document();
        WebCore::RenderView* renderView = doc->renderView();
        WebCore::HitTestRequest request(WebCore::HitTestRequest::ReadOnly | WebCore::HitTestRequest::Active);
        WebCore::HitTestResult result(documentPoint);

        renderView->layer()->hitTest(request, result);

        foundContainer = result.innerNode();
        if (!foundContainer)
            return WebCore::IntRect();

        // Following is a part of MouseEventWithHitTestResults::targetNode() in order to get targetNode for subframe
        WebCore::Node* node = foundContainer;
        if (!node->inDocument()) {
            WebCore::Element* element = node->parentElement();
            if (element && element->inDocument())
                node = element;
        }

        // Check if there is subframe
        frame = WebCore::EventHandler::subframeForTargetNode(node);
    }

    while (foundContainer) {
        if(foundContainer->isContainerNode() 
            && foundContainer->isElementNode()
            && foundContainer->getRect().width() > minwidth) {
           break;
        }
        foundContainer  = foundContainer->parentElement();
    }

    WebCore::IntRect ret = (foundContainer) ? view->contentsToWindow(foundContainer->getRect()) : WebCore::IntRect();
    WKCRect_SetXYWH(&r, ret.x(), ret.y(), ret.width(), ret.height());
    return r;
}

// History
bool WKCWebView::addVisitedLink(const char* uri, const unsigned short* title, const struct tm* date)
{
    WebCore::Page* page = m_private->core();

    if (!page)
        return false;

    WebCore::PageGroup* pagegrp = page->groupPtr();

    if (!pagegrp)
        return false;

    WebCore::KURL kurl(WebCore::KURL(), WebCore::String::fromUTF8(uri));

    pagegrp->addVisitedLink(kurl);

    return true;
}

void WKCWebView::setInternalColorFormat(int fmt)
{
    switch (fmt) {
    case EInternalColorFormat8888:
        WebCore::RGBA32Buffer::setInternalColorFormatRGBA8888();
        break;
    case EInternalColorFormat5515withMask:
    default:
        WebCore::RGBA32Buffer::setInternalColorFormatRGAB5515();
        break;
    }
}

void WKCWebView::setMaxSizeOfImageForDownsampling(int width, int height)
{
    WebCore::IntSize size(width, height);
    WebCore::ImageSource::setMaxSize(size);
}

void WKCWebView::setUseDitherForImageDecode(int bpp, bool flag)
{
   WebCore::RGBA32Buffer::useDither(bpp, flag);
}

void WKCWebView::setUseAntiAliasForDrawings(bool flag)
{
    m_private->setUseAntiAliasForDrawings(flag);
}

void WKCWebView::setUseAntiAliasForCanvas(bool flag)
{
    WKCWebViewPrivate::setUseAntiAliasForCanvas(flag);
}

void WKCWebView::setUseBilinearForScaledImages(bool flag)
{
    m_private->setUseBilinearForScaledImages(flag);
}

void WKCWebView::setUseBilinearForCanvasImages(bool flag)
{
    WKCWebViewPrivate::setUseBilinearForCanvasImages(flag);
}

void WKCWebView::clearCookies()
{
    WebCore::ResourceHandleManager::sharedInstance()->clearCookies();
}

void WKCWebView::setMaxCookieEntries(long number)
{
    WebCore::ResourceHandleManager::sharedInstance()->setMaxCookieEntries(number);
}

void WKCWebView::CookieSerializeStart()
{
    WebCore::ResourceHandleManager::sharedInstance()->sharedCookieSerializeStart();
}

int WKCWebView::CookieSerializeProgress(char* data, unsigned int len, unsigned int* writtenlen)
{
    return WebCore::ResourceHandleManager::sharedInstance()->sharedCookieSerializeProgress(data, len, writtenlen);
}

void WKCWebView::CookieSerializeEnd()
{
    WebCore::ResourceHandleManager::sharedInstance()->sharedCookieSerializeEnd();
}

void WKCWebView::CookieDeserializeStart(bool restart)
{
    WebCore::ResourceHandleManager::sharedInstance()->sharedCookieDeserializeStart(restart);
}

int  WKCWebView::CookieDeserializeProgress(const char *data, unsigned int len)
{
    if (!data || len < 1)
        return 1;

    return WebCore::ResourceHandleManager::sharedInstance()->sharedCookieDeserializeProgress(data, len);
}

void WKCWebView::CookieDeserializeEnd()
{
    WebCore::ResourceHandleManager::sharedInstance()->sharedCookieDeserializeEnd();
}

void WKCWebView::setScreenWidth(int width)
{
#if ENABLE(WKC_ANDROID_LAYOUT)
    WebCore::Frame* frame;
    for (frame = m_private->core()->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
        if (frame && frame->view()) {
            frame->view()->setScreenWidth(width);
            frame->view()->setNeedsLayout();
        }
    }
#endif
}

// global initialize / finalize

class WKCWebKitMemoryEventHandler {
public:
    WKCWebKitMemoryEventHandler(WKCMemoryEventHandler& handler)
         : m_memoryEventHandler(handler) {};
    ~WKCWebKitMemoryEventHandler() {};

    bool checkMemoryAvailability(unsigned int request_size, bool forimage) {
        return m_memoryEventHandler.checkMemoryAvailability(request_size, forimage);
    }
    int checkImageDecodeMode(int w, int h, float& scale_x, float& scale_y) {
        return m_memoryEventHandler.checkImageDecodeMode(w,h,scale_x,scale_y);
    }
    bool checkMemoryAllocatable(unsigned int request_size, WKCMemoryEventHandler::AllocationReason reason) {
        return m_memoryEventHandler.checkMemoryAllocatable(request_size, reason);
    }
    void notifyNeedMemoryReduction(unsigned int request_size, WKCMemoryEventHandler::BusyArea busy_area) {
        m_memoryEventHandler.notifyNeedMemoryReduction(request_size, busy_area);
    }
    void* notifyMemoryExhaust(unsigned int request_size, unsigned int& allocated_size) {
        return m_memoryEventHandler.notifyMemoryExhaust(request_size, allocated_size);
    }
    void notifyMemoryAllocationError(unsigned int request_size, WKCMemoryEventHandler::AllocationReason reason) {
        m_memoryEventHandler.notifyMemoryAllocationError(request_size, reason);
    }
    void notifyCrash(const char* file, int line, const char* function, const char* assertion) {
        m_memoryEventHandler.notifyCrash(file, line, function, assertion);
    }
    void notifyStackOverflow(bool need_restart, unsigned int stack_size, unsigned int consumption, unsigned int margin, void* stack_top, void* stack_base, void* current_stack_top, const char* file, int line, const char* function) {
        m_memoryEventHandler.notifyStackOverflow(need_restart, stack_size, consumption, margin, stack_top, stack_base, current_stack_top, file, line, function);
    }

private:
    WKCMemoryEventHandler& m_memoryEventHandler;
};

static unsigned char gMemoryEventHandlerInstance[sizeof(WKCWebKitMemoryEventHandler)];
static WKCWebKitMemoryEventHandler* gMemoryEventHandler = (WKCWebKitMemoryEventHandler *)&gMemoryEventHandlerInstance;
static void*
WKCWebKitNotifyNoMemory(unsigned int request_size, bool try_reduction)
{
    if (try_reduction) {
        // try memory reduction
        WKCMemoryEventHandler::BusyArea busy_area = WKCMemoryEventHandler::None;

        switch (wkcMemoryGetBusyAreaPeer()) {
        case WKC_MEMORY_BUSYAREA_NONE:          busy_area = WKCMemoryEventHandler::None;          break;
        case WKC_MEMORY_BUSYAREA_CLEARCACHE:    busy_area = WKCMemoryEventHandler::ClearCache;    break;
        default:    break;
        }

        gMemoryEventHandler->notifyNeedMemoryReduction(request_size, busy_area);
        return 0;
    }

    unsigned int dummy = 0;

    // Kill Timer
    WebCore::stopSharedTimer();

    // Kill Network Thread
    WebCore::ResourceHandleManager::forceTerminateInstance();

    return gMemoryEventHandler->notifyMemoryExhaust(request_size, dummy);
}

static WKCMemoryEventHandler::AllocationReason
WKCWebKitConvertAllocationReason(int in_reason)
{
    WKCMemoryEventHandler::AllocationReason result;

    switch (in_reason) {
    case WKC_MEMORYALLOC_TYPE_IMAGE:
        result = WKCMemoryEventHandler::Image;
        break;
    case WKC_MEMORYALLOC_TYPE_ANIMEGIF:
        result = WKCMemoryEventHandler::AnimeGif;
        break;
    case WKC_MEMORYALLOC_TYPE_JAVASCRIPT:
        result = WKCMemoryEventHandler::JavaScript;
        break;
    case WKC_MEMORYALLOC_TYPE_IMAGE_SHAREDBUFFER:
        result = WKCMemoryEventHandler::ImageSharedBuffer;
        break;
    case WKC_MEMORYALLOC_TYPE_JAVASCRIPT_HEAP:
        result = WKCMemoryEventHandler::JavaScriptHeap;
        break;
    default:
        result = WKCMemoryEventHandler::Normal;
        break;
    }
    return result;
}

static void
WKCWebKitNotifyMemoryAllocationError(unsigned int request_size, int in_reason)
{
    gMemoryEventHandler->notifyMemoryAllocationError(request_size, WKCWebKitConvertAllocationReason(in_reason));
}

static void
WKCWebKitNotifyCrash(const char* file, int line, const char* function, const char* assertion)
{
    gMemoryEventHandler->notifyCrash(file, line, function, assertion);
}

static void
WKCWebKitNotifyStackOverflow(bool need_restart, unsigned int stack_size, unsigned int consumption, unsigned int margin, void* stack_top, void* stack_base, void* current_stack_top, const char* file, int line, const char* function)
{
    gMemoryEventHandler->notifyStackOverflow(need_restart, stack_size, consumption, margin, stack_top, stack_base, current_stack_top, file, line, function);
}

static bool
WKCWebKitCheckMemoryAvailability(unsigned int size, bool forimage)
{
    return gMemoryEventHandler->checkMemoryAvailability(size, forimage);
}

static int
WKCWebKitQueryImageDecodeMode(int w, int h, float* out_scalex, float* out_scaley)
{
    float scalex=0, scaley=0;
    int ret = 0;
    switch (gMemoryEventHandler->checkImageDecodeMode(w, h, scalex, scaley)) {
    case WKCMemoryEventHandler::AllowDecode:
        ret = WKC_IMAGEDECODE_MODE_ALLOW;
        break;
    case WKCMemoryEventHandler::ScaleDecode:
        ret = WKC_IMAGEDECODE_MODE_SCALE;
        break;
    case WKCMemoryEventHandler::ReduceDecode:
        ret = WKC_IMAGEDECODE_MODE_REDUCE;
        break;
    case WKCMemoryEventHandler::ReduceAndScaleDecode:
        ret = WKC_IMAGEDECODE_MODE_REDUCEANDSCALE;
        break;
    case WKCMemoryEventHandler::DenyDecode:
    default:
        ret = WKC_IMAGEDECODE_MODE_DENY;
        break;
    }
    if (out_scalex) {
        *out_scalex = scalex;
    }
    if (out_scaley) {
        *out_scaley = scaley;
    }
    return ret;
}

static bool
WKCWebKitCheckMemoryAllocatable(unsigned int request_size, int in_reason)
{
    return gMemoryEventHandler->checkMemoryAllocatable(request_size, WKCWebKitConvertAllocationReason(in_reason));
}

static void*
peer_malloc_proc(unsigned int in_size, int in_crashonfailure)
{
    void* ptr = 0;

    if (in_crashonfailure) {
        ptr = fastMalloc(in_size);
    } else {
        TryMallocReturnValue rv = tryFastMalloc(in_size);
        if (!rv.getValue(ptr)) {
            return 0;
        }
    }
    return ptr;
}

static void
peer_free_proc(void* in_ptr)
{
    fastFree(in_ptr);
}

static void*
peer_realloc_proc(void* in_ptr, unsigned int in_size, int in_crashonfailure)
{
    void* ptr = 0;

    if (in_crashonfailure) {
        ptr = fastRealloc(in_ptr, in_size);
    } else {
        TryMallocReturnValue rv = tryFastRealloc(in_ptr, in_size);
        if (!rv.getValue(ptr)) {
            return 0;
        }
    }
    return ptr;
}

class WKCWebKitTimerEventHandler {
public:
    WKCWebKitTimerEventHandler(WKCTimerEventHandler& handler)
        : m_timerEventHandler(handler) {};
    ~WKCWebKitTimerEventHandler() {};

    void requestWakeUp(TimeoutProc in_proc, void* in_data) {
#if 0
// #if COMPILER(MSVC) && defined(_DEBUG)
        JSC::CollectorHeapStatistics stat;
        NF4_MEM_DP("WKCWebKitTimerEventHandler::requestWakeUp - JSC::CollectorHeapStatistics = 0x%08x", &stat);
        WKC::WKCWebView::jsHeapStatistics(stat);
#endif
        m_timerEventHandler.requestWakeUp(in_proc, in_data);
    };
private:
    WKCTimerEventHandler& m_timerEventHandler;
};

static unsigned char gTimerEventHandlerInstance[sizeof(WKCWebKitTimerEventHandler)];
static WKCWebKitTimerEventHandler* gTimerEventHandler = (WKCWebKitTimerEventHandler *)&gTimerEventHandlerInstance;

static void
WKCWebKitRequestWakeUp(TimeoutProc in_proc, void* in_data)
{
    gTimerEventHandler->requestWakeUp(in_proc, in_data);
}

void
WKCWebKitSetReservedMemorySizeAtCrashing(unsigned int size)
{
    wkcMemorySetReservedMemorySizeAtCrashingPeer(size);
}

void
WKCWebKitInitialize(void* memory, unsigned int memory_size, void* font_memory, unsigned int font_memory_size, WKCMemoryEventHandler& memory_event_handler, WKCTimerEventHandler& timer_event_handler)
{
    NF4_DP(("WKCWebKitInitialize Enter\n"));

    WKCWebKitResetVariables();

#if USE(MUTEX_DEBUG_LOG)
    InitializeCriticalSection(&gCriticalSection);
    gCriticalSectionFlag = true;
#endif

#if COMPILER(MSVC) && defined(_DEBUG)
    gForceTerminate = false;
#endif

    wkcDebugPrintInitializePeer();

    new (gMemoryEventHandler) WKCWebKitMemoryEventHandler(memory_event_handler);
    new (gTimerEventHandler) WKCWebKitTimerEventHandler(timer_event_handler);

    wkcMemorySetJSConfigurationProcPeer(JSC::Heap::setConfiguration);
    wkcMemorySetConfigurationProcPeer(WTF::fastMallocSetConfiguration);
    wkcMemoryInitializePeer((void *)((char *)memory), memory_size, peer_malloc_proc, peer_free_proc, peer_realloc_proc);
    wkcMemorySetNotifyNoMemoryProcPeer(WKCWebKitNotifyNoMemory);
    wkcMemorySetNotifyMemoryAllocationErrorProcPeer(WKCWebKitNotifyMemoryAllocationError);
    wkcMemorySetNotifyCrashProcPeer(WKCWebKitNotifyCrash);
    wkcMemorySetNotifyStackOverflowProcPeer(WKCWebKitNotifyStackOverflow);
    wkcMemorySetCheckAvailabilityProcPeer(WKCWebKitCheckMemoryAvailability);
    wkcDrawContextSetQueryImageDecodeModeProcPeer(WKCWebKitQueryImageDecodeMode);
    wkcMemorySetCheckMemoryAllocatableProcPeer(WKCWebKitCheckMemoryAllocatable);

    wkcHWOffscreenInitializePeer();

    WTF::initializeTCMallocWKC();

    if (!wkcTimerInitializePeer(WKCWebKitRequestWakeUp)) return;

    if (!WebCore::initializeTextBreakIterators()) return;
    if (!wkcFontEngineInitializePeer(font_memory, font_memory_size, (fontPeerMalloc)peer_malloc_proc, (fontPeerFree)peer_free_proc, true)) return;
    if (!WebCore::initializeSharedTimer()) return;

    wkcThreadInitializePeer();

    JSC::Structure::initializeStructures();

    // these 4 initializations should be called before initializeThreading().
    JSC::initializeThreading();
    WebCore::InitializeLoggingChannelsIfNecessary();
    wkcFileInitializePeer();
    wkcNetInitializePeer();
    wkcSystemInitializePeer();

    WebCore::atomicCanonicalTextEncodingName("UTF-8");

#if ENABLE(DATABASE)
    gchar* databaseDirectory = g_build_filename(g_get_user_data_dir(), "webkit", "databases", 0);
    webkit_set_web_database_directory_path(databaseDirectory);

    // FIXME: It should be possible for client applications to override the default appcache location
    WebCore::cacheStorage().setCacheDirectory(databaseDirectory);
    g_free(databaseDirectory);
#endif

    WebCore::PageGroup::setShouldTrackVisitedLinks(true);

#if 1
    // Ugh!: do something!
    // 100107 ACCESS Co.,Ltd.
#else
    WebCore::Pasteboard::generalPasteboard()->setHelper(WebCore::pasteboardHelperInstance());
#endif

#if 1
    // just disable icon database
    WebCore::iconDatabase()->setEnabled(false);
#else
    iconDatabase()->setEnabled(true);
    GOwnPtr<gchar> iconDatabasePath(g_build_filename(g_get_user_data_dir(), "webkit", "icondatabase", 0));
    iconDatabase()->open(iconDatabasePath.get());

    atexit(closeIconDatabaseOnExit);
#endif

    if (!WKCGlobalSettings::isExistSharedInstance())
        if (!WKCGlobalSettings::createSharedInstance(true)) return;

    if (!WebCore::ResourceHandleManager::isExistSharedInstance())
        if (!WebCore::ResourceHandleManager::createSharedInstance()) return;

    // SSL
    wkcSSLInitializePeer();

    JSC::TimeoutChecker::notifySetStartTime();

    // cache
    WebCore::cache()->setCapacities(0, 0, 1*1024*1024);
    WebCore::cache()->setDeadDecodedDataDeletionInterval(0);
    WKCWebView::setMinDelayBeforeLiveDecodedPruneCaches(1);
    WebCore::pageCache()->setCapacity(0); /* dont use page cache now*/

    NF4_DP(("WKCWebKitInitialize Exit\n"));
}

void
WKCWebKitFinalize()
{
    WebCore::CSSComputedStyleDeclaration_deleteSharedInstance();

    WebCore::CSSStyleSelector_deleteSharedInstance();

    WebCore::CSSFontSelector_deleteSharedInstance();

    WebCore::CSSImportRule::deleteSharedInstance();

    WebCore::CSSInitialValue::deleteSharedInstance();

    WebCore::CSSPrimitiveValue::deleteSharedInstance();

    WebCore::CSSSelector::deleteSharedInstance();

    WebCore::FunctionMap_deleteSharedInstance();

    WebCore::IdentifierRep_deleteSharedInstance();

    if (WebCore::ResourceHandleManager::isExistSharedInstance())
        WebCore::ResourceHandleManager::deleteSharedInstance();

    if (WKCGlobalSettings::isAutomatic())
        WKCGlobalSettings::deleteSharedInstance();

    WebCore::QualifiedName::deleteSharedInstance();

    WebCore::TextEncodingRegistry_deleteSharedInstance();

    WebCore::HTMLElementFactory::deleteSharedInstance();

    WebCore::JSHTMLWrapper_deleteSharedInstance();

    WebCore::GCController::deleteSharedInstance();

    WebCore::JSHTMLInputElement_deleteSharedInstance();

    WebCore::JSImageDataCustom_deleteSharedInstance();

    WebCore::JSLazyEventListener::deleteSharedInstance();

    WebCore::ScriptEventListener_deleteSharedInstance();

    WebCore::CSSPropertyLonghand_deleteSharedInstance();

    WebCore::ContainerNode::deleteSharedInstance();

    WebCore::Document::deleteSharedInstance();

    WebCore::DOMImplementation::deleteSharedInstance();

    WebCore::EventTarget::deleteSharedInstance();

    WebCore::NodeRareData::deleteSharedInstance();

    WebCore::Node::deleteSharedInstance();

    WebCore::Range::deleteSharedInstance();

    WebCore::ScriptElement_deleteSharedInstance();

    WebCore::StyledElement::deleteSharedInstance();

    WebCore::ApplyStyleCommand::deleteSharedInstance();

    WebCore::Editor::deleteSharedInstance();

    WebCore::HTMLEditing_deleteSharedInstance();

    WebCore::IndentOutdentCommand::deleteSharedInstance();

    WebCore::Markup_deleteSharedInstance();

    WebCore::ReplaceSelectionCommand::deleteSharedInstance();

    WebCore::CachedFrame::deleteSharedInstance();

    WebCore::CachedPage::deleteSharedInstance();

    WebCore::PageCache::deleteSharedInstance();

    WebCore::HTMLButtonElement::deleteSharedInstance();

    WebCore::HTMLElement::deleteSharedInstance();

    WebCore::HTMLFieldSetElement::deleteSharedInstance();

    WebCore::HTMLInputElement::deleteSharedInstance();

    WebCore::HTMLKeygenElement::deleteSharedInstance();

    WebCore::HTMLLegendElement::deleteSharedInstance();

    WebCore::HTMLLinkElement::deleteSharedInstance();

    WebCore::HTMLOptGroupElement::deleteSharedInstance();

    WebCore::HTMLOptionElement::deleteSharedInstance();

    WebCore::HTMLParser::deleteSharedInstance();

    WebCore::HTMLSelectElement::deleteSharedInstance();

    WebCore::HTMLTableElement::deleteSharedInstance();

    WebCore::ValidityState::deleteSharedInstance();

#if ENABLE(JAVASCRIPT_DEBUGGER)
    WebCore::JavaScriptDebugServer::deleteSharedInstance();

    WebCore::JavaScriptProfile_deleteSharedInstance();

    WebCore::JavaScriptProfileNode_deleteSharedInstance();
#endif

    WebCore::Cache::deleteSharedInstance();

    WebCore::CachedImage::deleteSharedInstance();

    WebCore::CachedResource::deleteSharedInstance();

    WebCore::CrossOriginAccessControl_deleteSharedInstance();

    WebCore::CrossOriginPreflightResultCache::deleteSharedInstance();

    WebCore::FrameLoader::deleteSharedInstance();

    WebCore::ImageLoader::deleteSharedInstance();

    WebCore::SubresourceLoader::deleteSharedInstance();

    WebCore::ArchiveFactory::deleteSharedInstance();

    WebCore::IconDatabase::deleteSharedInstance();

    WebCore::DOMWindow::deleteSharedInstance();

    WebCore::EventHandler::deleteSharedInstance();

    WebCore::Frame::deleteSharedInstance();

    WebCore::Page::deleteSharedInstance();

    WebCore::PageGroup::deleteSharedInstance();

    WebCore::SecurityOrigin::deleteSharedInstance();

    WebCore::UserContentURLPattern::deleteSharedInstance();

    WebCore::XSSAuditor::deleteSharedInstance();

    WebCore::AnimationBase::deleteSharedInstance();

    WebCore::KURL::deleteSharedInstance();

    WebCore::MIMETypeRegistry::deleteSharedInstance();

    WebCore::ScrollView::deleteSharedInstance();

    WebCore::ThreadTimers::deleteSharedInstance();

    WebCore::Pasteboard::deleteSharedInstance();

    WebCore::ScrollbarTheme::deleteSharedInstance();

    WebCore::FontCache::deleteSharedInstance();

    WebCore::GlyphPageTreeNode::deleteSharedInstance();

    WebCore::Image::deleteSharedInstance();

    WebCore::GeolocationServiceMock::deleteSharedInstance();

    WebCore::NetworkStateNotifier_deleteSharedInstance();

    WebCore::ResourceResponseBase::deleteSharedInstance();

    WebCore::BidiContext::deleteSharedInstance();

    WebCore::TextEncoding::deleteSharedInstance();

    WebCore::PluginDatabase::deleteSharedInstance();

    WebCore::PluginMainThreadScheduler::deleteSharedInstance();

    WebCore::PluginStream::deleteSharedInstance();

    WebCore::PluginView::deleteSharedInstance();

    WebCore::InlineTextBox::deleteSharedInstance();

    WebCore::RenderBlock::deleteSharedInstance();

    WebCore::RenderBox::deleteSharedInstance();

    WebCore::RenderCounter::deleteSharedInstance();

    WebCore::RenderFlexibleBox::deleteSharedInstance();

    WebCore::RenderImage::deleteSharedInstance();

    WebCore::RenderLayer::deleteSharedInstance();

    WebCore::RenderListItem::deleteSharedInstance();

    WebCore::RenderObject::deleteSharedInstance();

    WebCore::RenderPartObject::deleteSharedInstance();

    WebCore::RenderScrollbarTheme::deleteSharedInstance();

    WebCore::RenderTheme::deleteSharedInstance();

    WebCore::RenderWidget::deleteSharedInstance();

    WebCore::RootInlineBox::deleteSharedInstance();

    WebCore::RenderStyle::deleteSharedInstance();

    WebCore::XMLHttpRequest::deleteSharedInstance();

    WebCore::ThreadGlobalData_deleteSharedInstance();

    WebCore::JSDOMWindowBase::deleteSharedInstance();

    // The following function should be called before "WebCore::JSDOMWindowBase::deleteSharedInstance()"
    WebCore::cachedNormalWorld_deletesSharedInstance();

    JSC::SamplingTool_deleteSharedInstance();

    JSC::Profiler::deleteSharedInstance();

    JSC::deleteUStringSharedInstance();

    JSC::UString::deleteSharedInstance();

    JSC::Structure::deleteSharedInstance();

    JSC::Bindings::rootObjectSet_deleteSharedInstance();

    JSC::JSGlobalData::deleteSharedInstance();

    WTF::dtoa_deleteSharedInstance();

    WTF::MainThread_deleteSharedInstance();

    WTF::RefCountedLeakCounter::deleteSharedInstance();

    WTF::ThreadingWKC_deleteSharedInstance();

    WTF::finalizeMainThreadPlatform();
    wkcHWOffscreenFinalizePeer();
    WebCore::finalizeSharedTimer();
    wkcSSLFinalizePeer();
    wkcFontEngineFinalizePeer();
    WebCore::finalizeTextBreakIterators();
    wkcSystemFinalizePeer();
    wkcNetFinalizePeer();
    wkcFileFinalizePeer();
    wkcTimerFinalizePeer();

    WTF::finalizeTCMallocWKC();
    gTimerEventHandler->~WKCWebKitTimerEventHandler();
    memset(gTimerEventHandler, 0, sizeof(WKCWebKitTimerEventHandler));
    gMemoryEventHandler->~WKCWebKitMemoryEventHandler();
    memset(gMemoryEventHandler, 0, sizeof(WKCWebKitMemoryEventHandler));

    wkcThreadFinalizePeer();
    wkcMemoryFinalizePeer();

    wkcDebugPrintFinalizePeer();

#if USE(MUTEX_DEBUG_LOG)
    DeleteCriticalSection(&gCriticalSection);
    gCriticalSectionFlag = false;
#endif
}

void
WKCWebKitSuspendFont()
{
    wkcFontEngineFinalizePeer();
}

void
WKCWebKitResumeFont(void* font_memory, unsigned int font_memory_size)
{
    wkcFontEngineInitializePeer(font_memory, font_memory_size, (fontPeerMalloc)peer_malloc_proc, (fontPeerFree)peer_free_proc, true);
}

unsigned int WKCWebKitFontHeapSize()
{
    return wkcFontHeapSizePeer();
}

int WKCWebKitRegisterFontOnMemory(const unsigned char* memPtr)
{
    return wkcFontRegisterFontPeer(memPtr, NULL);
}

int WKCWebKitRegisterFontInFile(const char* filePath)
{
    return wkcFontRegisterFontPeer(NULL, filePath);
}

void WKCWebKitUnregisterFonts()
{
    wkcFontUnregisterFontsPeer();
}

bool WKCWebKitSetPrimaryFont(int fontID)
{
    return wkcFontSetPrimaryFontPeer(fontID);
}

void
WKCWebKitEnableScalingMonosizeFont(bool flag)
{
    WebCore::FontPlatformData::enableScalingMonosizeFont(flag);
}

bool
WKCWebKitRegisterI18NData(const void* data, unsigned int len)
{
    return wkcI18NRegisterDataPeer(data, len);
}

void
WKCWebKitUnregisterI18NData()
{
    wkcI18NRegisterDataPeer(0, 0);
}

const WKC::String
WKCWebKitGetNodeAttributeValue(const WKC::Node* node, const char* attributeName)
{
    if (node && node->priv().webcore()->attributes()) {
        WebCore::String nameString(attributeName);
        WebCore::QualifiedName name(WebCore::nullAtom, nameString.lower(), WebCore::nullAtom);
        if (WebCore::Attribute* attribute = node->priv().webcore()->attributes()->getAttributeItem(name)) {
            return WKC::String(attribute->value().string());
        }
    }

    return WKC::String();
}

void WKCWebKitSetResolveFilenameForDisplayProc(WKC::ResolveFilenameForDisplayProc proc)
{
    WebCore::ResolveFilenameForDisplayProc iproc = (WebCore::ResolveFilenameForDisplayProc)proc;
    WebCore::FileChooser_SetResolveFilenameForDisplayProc(iproc);
}

#if ENABLE(WKC_FORCE_NOTIFY_SCROLL)
static WKC::ForceNotifyScrollProc gForceNotifyScrollProc = 0;

static void WKCWebKitForceNotifyScroll(const WebCore::IntPoint& scrollPoint, WebCore::ScrollView* view)
{
    if (gForceNotifyScrollProc) {
        WebCore::Frame* coreFrame= ((WebCore::FrameView*)view)->frame();
        FrameLoaderClientWKC* client = static_cast<FrameLoaderClientWKC*>(coreFrame->loader()->client());
        WKCWebFrame* frame = client ? client->webFrame() : 0;
        WKCPoint p = { scrollPoint.x(), scrollPoint.y() };

        (*gForceNotifyScrollProc)(frame, p);
    }
}

void WKCWebKitSetForceNotifyScrollProc(WKC::ForceNotifyScrollProc proc)
{
    gForceNotifyScrollProc = proc;
    WebCore::ScrollView::setForceNotifyScrollProc(WKCWebKitForceNotifyScroll);
}
#else
void WKCWebKitSetForceNotifyScrollProc(WKC::forceNotifyScrollProc /*proc*/)
{
}
#endif

static char gSkin_[sizeof(wkcSkin)];
static wkcSkin* gSkin = (wkcSkin *)&gSkin_;

void WKCWebKitRegisterSkin(const WKC::WKCSkin* skin)
{
    int i=0, j=0;

    if (!skin) {
        wkcStockImageRegisterSkinPeer((const wkcSkin *)0);
        return;
    }

    // for safety: WKC::WKCSkin and wkcSkin would be same structure,
    // but it should be safe transfer each members one by one...
    for (i=0; i<WKC::ESkinImages; i++) {
        gSkin->fImages[i].fBitmap = skin->fImages[i].fBitmap;
        gSkin->fImages[i].fSize.fWidth = skin->fImages[i].fSize.fWidth;
        gSkin->fImages[i].fSize.fHeight = skin->fImages[i].fSize.fHeight;
        for (j=0; j<4; j++) {
            gSkin->fImages[i].fPoints[j] = skin->fImages[i].fPoints[j];
        }
    }
    for (i=0; i<WKC::ESkinColors; i++) {
        gSkin->fColors[i] = skin->fColors[i];
    }
    for (i=0; i<WKC::ESystemFontTypes; i++) {
        gSkin->fSystemFontSize[i] = skin->fSystemFontSize[i];
    }
    wkcStockImageRegisterSkinPeer(gSkin);
}

void
WKCWebKitSetHWOffscreenDeviceParams(const HWOffscreenDeviceParams* params, void* opaque)
{
    WKCHWOffscreenParams procs = {
        params->fLockProc,
        params->fUnlockProc,
        params->fEnable,
        params->fEnableForImagebuffer
    };
    wkcHWOffscreenSetParamsPeer(&procs, opaque);
}

void
WKCWebKitSetScreenDeviceParams(const ScreenDeviceParams& params)
{
    WebCore::setScreenSizeWKC(WebCore::IntSize(params.fScreenWidth, params.fScreenHeight));
    WebCore::setAvailableScreenSize(WebCore::IntSize(params.fAvailableScreenWidth, params.fAvailableScreenHeight));
    WebCore::setScreenDepth(params.fScreenDepth, params.fScreenDepthPerComponent);
    WebCore::setIsMonochrome(params.fIsMonochrome);
}

// SSL
void* WKCWebKitSSLRegisterRootCA(const char* cert, int cert_len)
{
    return wkcSSLRegisterRootCAPeer(cert, cert_len);
}

int WKCWebKitSSLUnregisterRootCA(void* certid)
{
    return wkcSSLUnregisterRootCAPeer(certid);
}

void* WKCWebKitSSLRegisterCRL(const char* crl, int crl_len)
{
    return wkcSSLRegisterCRLPeer(crl, crl_len);
}

int WKCWebKitSSLUnregisterCRL(void* crlid)
{
    return wkcSSLUnregisterCRLPeer(crlid);
}

void* WKCWebKitSSLRegisterClientCert(const unsigned char* pkcs12, int pkcs12_len, const unsigned char* pass, int pass_len)
{
    using WebCore::ResourceHandleManagerSSL;
    if (WebCore::ResourceHandleManagerSSL* mgr = WebCore::ResourceHandleManagerSSL::sharedInstance()) {
        return mgr->SSLRegisterClientCert(pkcs12, pkcs12_len, pass, pass_len);
    }
    return NULL;
}

int WKCWebKitSSLUnregisterClientCert(void* certid)
{
    using WebCore::ResourceHandleManagerSSL;
    if (WebCore::ResourceHandleManagerSSL* mgr = WebCore::ResourceHandleManagerSSL::sharedInstance()) {
        return mgr->SSLUnregisterClientCert(certid);
    }
    return -1;
}

void WKCWebKitSSLSetServerCA(const char *host_w_port, const char *ca)
{
    using WebCore::ResourceHandleManagerSSL;
    if (WebCore::ResourceHandleManagerSSL* mgr = WebCore::ResourceHandleManagerSSL::sharedInstance()) {
        mgr->setAllowServerCA(host_w_port, ca);
    }
}

void WKCWebKitSSLSetAllowServerHost(const char *host_w_port)
{
    using WebCore::ResourceHandleManagerSSL;
    if (WebCore::ResourceHandleManagerSSL* mgr = WebCore::ResourceHandleManagerSSL::sharedInstance()) {
        mgr->setAllowServerHost(host_w_port);
    }
}

// File System
void WKCWebKitSetFileSystemProcs(const WKC::FileSystemProcs* procs)
{
    wkcFileCallbackSetPeer(procs->fOpenProc, procs->fCloseProc, procs->fReadProc, procs->fWriteProc,
                           procs->fEOFProc, procs->fStatProc, procs->fNoProc);
}

int WKCWebKitSetTimeZone(int offset, bool isSummerTime)
{
    return wkcSetTimeZone(offset, isSummerTime);
}

// glyph / image cache
bool WKCWebKitSetGlyphCache(int format, void* cache, const WKCSize* size)
{
    bool ret = false;
    if (cache) {
        ret = wkcOffscreenCreateGlyphCachePeer(format, cache, size);
        if (!ret) return false;
        ret = wkcHWOffscreenCreateGlyphCachePeer(format, cache, size);
        if (!ret) {
            wkcOffscreenDeleteGlyphCachePeer();
        }
        return ret;
    } else {
       wkcHWOffscreenDeleteGlyphCachePeer();
       wkcOffscreenDeleteGlyphCachePeer();
       return true;
    }
}
 
bool WKCWebKitSetImageCache(int format, void* cache, const WKCSize* size)
{
    bool ret = false;
    if (cache) {
        ret = wkcOffscreenCreateImageCachePeer(format, cache, size);
        if (!ret) return false;
        ret = wkcHWOffscreenCreateImageCachePeer(format, cache, size);
        if (!ret) {
            wkcOffscreenDeleteImageCachePeer();
        }
        return ret;
    } else {
       wkcHWOffscreenDeleteImageCachePeer();
       wkcOffscreenDeleteImageCachePeer();
       return true;
    }
}

void
WKCWebKitResetVariables()
{
    // This function must be the first
    WKCWebKitResetROMTables();

#if USE(MUTEX_DEBUG_LOG)
    if (gCriticalSectionFlag)
        DeleteCriticalSection(&gCriticalSection);
#endif

    WTF::ThreadingWKC_resetVariables();

    WebCore::TextBreakIterator_resetVariables();

    WebCore::SharedTimer_resetVariables();

    WebCore::ThreadGlobalData::resetVariables();

    WebCore::ThreadTimers::resetVariables();

    WebCore::TimerBase::resetVariables();

    WebCore::TextEncoding::resetVariables();

    WebCore::TextEncodingRegistry_resetVariables();

    JSC::JSGlobalData::resetVariables();

    JSC::Structure::resetVariables();

    JSC::resetUStringVariables();

    JSC::UString::resetVariables();

    WTF::DateMath_resetVariables();

    WTF::dtoa_resetVariables();

    JSC::resetThreadingVariables();

    WebCore::ResourceHandleManager::resetVariables();

    WebCore::AXObjectCache::resetVariables();

    WebCore::JSDOMWindowBase::resetVariables();

    WebCore::cachedNormalWorld_resetVariables();

    JSC::Bindings::rootObjectSet_resetVariables();

    JSC::Bindings::Instance::resetVariables();

    WebCore::IdentifierRep_resetVariables();

#if !COMPILER(RVCT)
    JSC::SamplingTool_resetVariables();
#endif

    JSC::BytecodeGenerator::resetVariables();

    WebCore::XMLHttpRequest::resetVariables();

    WebCore::FunctionMap_resetVariables();

    WebCore::MediaFeatureNames::resetVariables();

    WebCore::CSSStyleSelector_resetVariables();

    WebCore::CSSSelector::resetVariables();

    WebCore::CSSPropertyLonghand_resetVariables();

    WebCore::CSSPrimitiveValue::resetVariables();

    WebCore::CSSInitialValue::resetVariables();

    WebCore::CSSImportRule::resetVariables();

    WebCore::CSSFontSelector_resetVariables();

    WebCore::CSSComputedStyleDeclaration_resetVariables();

    WebCore::GCController::resetVariables();
    WebCore::JSHTMLInputElement_resetVariables();
    WebCore::JSImageDataCustom_resetVariables();
    WebCore::JSLazyEventListener::resetVariables();
    WebCore::ScriptEventListener_resetVariables();

    WebCore::RenderStyle::resetVariables();
    WebCore::RootInlineBox::resetVariables();
    WebCore::RenderWidget::resetVariables();
    WebCore::RenderTheme::resetVariables();
    WebCore::RenderScrollbarTheme::resetVariables();
    WebCore::RenderScrollbar::resetVariables();
    WebCore::RenderPartObject::resetVariables();
    WebCore::RenderObject::resetVariables();
    WebCore::RenderListItem::resetVariables();
    WebCore::RenderListBox::resetVariables();
    WebCore::RenderLayer::resetVariables();
    WebCore::RenderImage::resetVariables();
    WebCore::RenderFlexibleBox::resetVariables();
    WebCore::RenderCounter::resetVariables();
    WebCore::RenderBoxModelObject::resetVariables();
    WebCore::RenderBox::resetVariables();
    WebCore::RenderBlock::resetVariables();
    WebCore::LayoutState::resetVariables();
    WebCore::InlineTextBox::resetVariables();
    WebCore::InlineBox::resetVariables();

    JSC::Profiler::resetVariables();

    WebCore::PluginView::resetVariables();
    WebCore::PluginStream::resetVariables();
    WebCore::PluginMainThreadScheduler::resetVariables();
    WebCore::PluginDatabase::resetVariables();

    WebCore::ResetArenaVariables();
    WebCore::AtomicString::resetVariables();
    WebCore::BidiContext::resetVariables();
    WebCore::Font::resetVariables();
    WebCore::FontPlatformData::resetVariables();
    WebCore::FontCache::resetVariables();
    WebCore::GeolocationService::resetVariables();
    WebCore::GeolocationServiceMock::resetVariables();
    WebCore::GlyphPageTreeNode::resetVariables();
    WebCore::Image::resetVariables();
    WebCore::KURL::resetVariables();
    WebCore::MIMETypeRegistry::resetVariables();
    WebCore::NetworkStateNotifier_resetVariables();
    WebCore::ResourceHandle::resetVariables();
    WebCore::ResourceResponseBase::resetVariables();
    WebCore::ScrollView::resetVariables();

    WebCore::AnimationBase::resetVariables();
    WebCore::Console::resetVariables();
    WebCore::DOMTimer::resetVariables();
    WebCore::DOMWindow::resetVariables();
    WebCore::EventHandler::resetVariables();
    WebCore::Frame::resetVariables();
    WebCore::FrameView::resetVariables();
    WebCore::Geolocation::resetVariables();
    WebCore::Page::resetVariables();
    WebCore::PageGroup::resetVariables();
    WebCore::SecurityOrigin::resetVariables();
    WebCore::UserContentURLPattern::resetVariables();
    WebCore::XSSAuditor::resetVariables();

    WebCore::ArchiveFactory::resetVariables();
    WebCore::Cache::resetVariables();
    WebCore::CachedImage::resetVariables();
    WebCore::CachedResource::resetVariables();
    WebCore::CrossOriginAccessControl_resetVariables();
    WebCore::CrossOriginPreflightResultCache::resetVariables();
    WebCore::FrameLoader::resetVariables();
    WebCore::IconDatabase::resetVariables();
    WebCore::ImageLoader::resetVariables();
    WebCore::Loader::resetVariables();
    WebCore::SubresourceLoader::resetVariables();

#if ENABLE(JAVASCRIPT_DEBUGGER)
    WebCore::JavaScriptDebugServer::resetVariables();
    JavaScriptProfile_resetVariables();
    JavaScriptProfileNode_resetVariables();
#endif

    WebCore::ApplyStyleCommand::resetVariables();
    WebCore::Editor::resetVariables();
    WebCore::HTMLEditing_resetVariables();
    WebCore::HTMLInterchange_resetVariables();
    WebCore::IndentOutdentCommand::resetVariables();
    WebCore::Markup_resetVariables();
    WebCore::ReplaceSelectionCommand::resetVariables();

    WebCore::CachedFrame::resetVariables();
    WebCore::CachedPage::resetVariables();
    WebCore::PageCache::resetVariables();

    WebCore::HTMLButtonElement::resetVariables();
    WebCore::HTMLElement::resetVariables();
    WebCore::HTMLFieldSetElement::resetVariables();
    WebCore::HTMLFormElement::resetVariables();
    WebCore::HTMLInputElement::resetVariables();
    WebCore::HTMLKeygenElement::resetVariables();
    WebCore::HTMLLabelElement::resetVariables();
    WebCore::HTMLLegendElement::resetVariables();
    WebCore::HTMLLinkElement::resetVariables();
    WebCore::HTMLOptGroupElement::resetVariables();
    WebCore::HTMLOptionElement::resetVariables();
    WebCore::HTMLParser::resetVariables();
    WebCore::HTMLSelectElement::resetVariables();
    WebCore::HTMLTableElement::resetVariables();
    WebCore::HTMLTextAreaElement::resetVariables();
    WebCore::ValidityState::resetVariables();

    WebCore::ContainerNode::resetVariables();
    WebCore::Document::resetVariables();
    WebCore::DOMImplementation::resetVariables();
    WebCore::EventTarget::resetVariables();
    WebCore::Node::resetVariables();
    WebCore::NodeRareData::resetVariables();
    WebCore::QualifiedName::resetVariables();
    WebCore::Range::resetVariables();
    WebCore::ScriptElement_resetVariables();
    WebCore::StyledElement::resetVariables();
    WebCore::XMLTokenizer_resetVariables();
    WebCore::XMLTokenizerScope::resetVariables();

    WebCore::Pasteboard::resetVariables();
    WebCore::RenderThemeWKC_resetVariables();
    WebCore::ScrollbarTheme::resetVariables();

    WebCore::HTMLElementFactory::resetVariables();
    WebCore::JSHTMLWrapper_resetVariables();

    WebCore::ImageBufferData::resetVariables();

    WKC::EventHandlerPrivate::resetVariables();

    libXML2_xmlmemory_resetVariables();
    libXML2_xmlIO_resetVariables();
    libXML2_xlink_resetVariables();
    libXML2_tree_resetVariables();
    libXML2_threads_resetVariables();
    libXML2_relaxng_resetVariables();
    libXML2_parser_resetVariables();
    libXML2_HTMLparser_resetVariables();
    libXML2_threads_resetVariables();
    libXML2_globals_resetVariables();
    libXML2_encoding_resetVariables();
    libXML2_error_resetVariables();
    libXML2_dict_resetVariables();
    libXML2_catalog_resetVariables();

    WTF::RefCountedLeakCounter::resetVariables();
}

void
WKCWebKitForceTerminate()
{
    NF4_DP(("WKCWebKitForceTerminate Enter\n"));

#if COMPILER(MSVC) && defined(_DEBUG)
    gForceTerminate = true;
#endif

    if (WebCore::ResourceHandleManager::isExistSharedInstance())
        WebCore::ResourceHandleManager::forceTerminateInstance();

    if (WKCGlobalSettings::isAutomatic())
        WKCGlobalSettings::deleteSharedInstance();

//    wkcFontEngineForceTerminatePeer();
    wkcHWOffscreenForceTerminatePeer();
    wkcTextBreakIteratorForceTerminatePeer();
    wkcOffscreenForceTerminatePeer();
    wkcDrawContextForceTerminatePeer();
    wkcSSLForceTerminatePeer();
    wkcNetForceTerminatePeer();
    wkcThreadForceTerminatePeer();
    wkcFileForceTerminatePeer();
    wkcTimerForceTerminatePeer();
    wkcMemoryForceTerminatePeer();
    wkcDebugPrintForceTerminatePeer();
    memset(gTimerEventHandler, 0, sizeof(WKCWebKitTimerEventHandler));
    memset(gMemoryEventHandler, 0, sizeof(WKCWebKitMemoryEventHandler));

    NF4_DP(("WKCWebKitForceTerminate Exit\n"));
}

void
WKCWebKitForceFinalize()
{
    WKCWebKitForceTerminate();
    WTF::finalizeTCMallocWKC();

    wkcTimerFinalizePeer();
}

unsigned int
WKCWebKitAvailableMemory()
{
    int total = wkcMemoryGetAvailSizePeer();
    int allocated = wkcMemoryGetAllocatedSizePeer();
    return total-allocated;
}

unsigned int
WKCWebKitMaxAvailableBlock()
{
    return wkcMemoryGetMaxAvailableBlockSizePeer();
}
void
WKCWebKitRequestGarbageCollect(void)
{
    WebCore::gcController().garbageCollectSoon();
}

void
WKCWebKitSetSystemStrings(const WKC::SystemStrings* strings)
{
    wkcSystemSetNavigatorPlatformPeer(strings->fNavigatorPlatform);
    wkcSystemSetNavigatorProductPeer(strings->fNavigatorProduct);
    wkcSystemSetNavigatorProductSubPeer(strings->fNavigatorProductSub);
    wkcSystemSetNavigatorVendorPeer(strings->fNavigatorVendor);
    wkcSystemSetNavigatorVendorSubPeer(strings->fNavigatorVendorSub);
    wkcSystemSetLanguagePeer(strings->fLanguage);
    wkcSystemSetButtonLabelSubmitPeer(strings->fButtonLabelSubmit);
    wkcSystemSetButtonLabelResetPeer(strings->fButtonLabelReset);
    wkcSystemSetButtonLabelFilePeer(strings->fButtonLabelFile);
}

void 
WKCWebKitSetThreadInfo(void* threadID, void* stackBase)
{
    wkcThreadSetMainThreadInfoPeer(threadID, stackBase);
}

void
WKCWebKitSetStackSize(unsigned int in_stack_size)
{
    void* thread;

    thread = wkcThreadCurrentThreadPeer();
    wkcThreadSetStackSizePeer(thread, in_stack_size);
}

void
WKCWebKitSetEncodingDetectorLanguageSet(int languageSetFlag)
{
    int flags = WKC_I18N_DETECT_ENCODING_NONE;

    if (languageSetFlag & WKC::EEncodingDetectorUniversal) {
        flags |= WKC_I18N_DETECT_ENCODING_UNIVERSAL;
    }
    if (languageSetFlag & WKC::EEncodingDetectorJapanese) {
        flags |= WKC_I18N_DETECT_ENCODING_JAPANESE;
    }
    wkcI18NSetDetectEncodingLanguageSetPeer(flags);
}

void
WKCWebView::setScrollPositionForOffscreen(const WKCPoint& scrollPosition)
{
    WebCore::IntPoint p(scrollPosition.fX, scrollPosition.fY);
    m_private->setScrollPositionForOffscreen(scrollPosition);
}

void
WKCWebView::jsHeapStatistics(WKC::WKCWebKitCollectorHeapStatistics& out_stat)
{
    JSC::JSGlobalData* jsGlobal;
    static JSC::CollectorHeapStatistics stat;   // To avoid Stack Overflow.

    ASSERT(kMaxCollectorHeapBlockInfo == JSC::kMaxBlockInfo);
    JSC::initCollectorHeapStatistics(stat);

    jsGlobal = JSC::JSGlobalData::sharedInstanceAddress();
    if (jsGlobal)
        jsGlobal->heap.statistics(stat, JSC::ECollectorHeapTypeJSGlobalData);

    jsGlobal = WebCore::JSDOMWindowBase::jsGlobalData();
    if (jsGlobal)
        jsGlobal->heap.statistics(stat, JSC::ECollectorHeapTypeJSDOMWindowBase);

    out_stat.numBlockInfo = stat.numBlockInfo;
    for (int i = 0; i < out_stat.numBlockInfo; i++) {
        out_stat.blockInfo[i].size = stat.blockInfo[i].size;
        switch (stat.blockInfo[i].type) {
        case JSC::ECollectorHeapTypeJSGlobalData:
            out_stat.blockInfo[i].type = ECollectorHeapTypeJSGlobalData;
            break;
        case JSC::ECollectorHeapTypeJSDOMWindowBase:
            out_stat.blockInfo[i].type = ECollectorHeapTypeJSDOMWindowBase;
            break;
        case JSC::ECollectorHeapTypeNone:
        default:
            out_stat.blockInfo[i].type = ECollectorHeapTypeNone;
            break;
        }
        out_stat.blockInfo[i].flag = ECollectorHeapBlockInfoFlagNone;
        if (stat.blockInfo[i].flag & JSC::ECollectorHeapBlockInfoFlagPeer) {
            out_stat.blockInfo[i].flag |= ECollectorHeapBlockInfoFlagPeer;
        }
        if (stat.blockInfo[i].flag & JSC::ECollectorHeapBlockInfoFlagTCMalloc) {
            out_stat.blockInfo[i].flag |= ECollectorHeapBlockInfoFlagTCMalloc;
        }
        if (stat.blockInfo[i].flag & JSC::ECollectorHeapBlockInfoFlagBackup) {
            out_stat.blockInfo[i].flag |= ECollectorHeapBlockInfoFlagBackup;
        }
        if (stat.blockInfo[i].flag & JSC::ECollectorHeapBlockInfoFlagNoCellUsed) {
            out_stat.blockInfo[i].flag |= ECollectorHeapBlockInfoFlagNoCellUsed;
        }
        out_stat.blockInfo[i].usedCellCount = stat.blockInfo[i].usedCellCount;
    }
}

bool
WKCWebView::getTMemoryInfo(WKC::TMemoryInfo& memInfo)
{
    bool ret = false;

#ifdef WKC_ENABLE_USED_MEMORY_INFO
#ifndef WKC_ENABLE_DYNAMIC_ALLOCATION
    COMPILE_ASSERT(KMemoryMaxNumDetailedMemoryArray == WKC_MEMORY_MAX_NUM_DETAILEDMEMORYARRAY, KMemoryMaxNumDetailedMemoryArray);
#endif // !WKC_ENABLE_DYNAMIC_ALLOCATION
    COMPILE_ASSERT(EMemoryRangeTypeNormal == WKC_MEMORYRANGE_TYPE_DEFAULT, EMemoryRangeTypeNormal);
    COMPILE_ASSERT(EMemoryRangeTypeManagement == WKC_MEMORYRANGE_TYPE_MANAGEMENT, EMemoryRangeTypeManagement);
    COMPILE_ASSERT(EMemoryRangeTypeSpan == WKC_MEMORYRANGE_TYPE_SPAN, EMemoryRangeTypeSpan);
    COMPILE_ASSERT(EMemoryRangeTypeJSHeap == WKC_MEMORYRANGE_TYPE_JSHEAP, EMemoryRangeTypeJSHeap);
    COMPILE_ASSERT(EMemoryRangeTypeJSStack == WKC_MEMORYRANGE_TYPE_JSSTACK, EMemoryRangeTypeJSStack);
    COMPILE_ASSERT(EMemoryRangeTypeHash == WKC_MEMORYRANGE_TYPE_HASH, EMemoryRangeTypeHash);
    COMPILE_ASSERT(EMemoryRangeTypeThreadHeap == WKC_MEMORYRANGE_TYPE_THREADHEAP, EMemoryRangeTypeThreadHeap);
    COMPILE_ASSERT(EMemoryRangeTypes == WKC_MEMORYRANGE_TYPES, EMemoryRangeTypes);

    COMPILE_ASSERT(EMemoryBlockTypeNone == WKC_MEMORYBLOCK_TYPE_NONE, EMemoryBlockTypeNone);
    COMPILE_ASSERT(EMemoryBlockTypeAvailable == WKC_MEMORYBLOCK_TYPE_AVAILABLE, EMemoryBlockTypeAvailable);
    COMPILE_ASSERT(EMemoryBlockTypeUsed == WKC_MEMORYBLOCK_TYPE_USED, EMemoryBlockTypeUsed);
    COMPILE_ASSERT(EMemoryBlockTypeFreed == WKC_MEMORYBLOCK_TYPE_FREED, EMemoryBlockTypeFreed);
    COMPILE_ASSERT(EMemoryBlockTypeHeap == WKC_MEMORYBLOCK_TYPE_HEAP, EMemoryBlockTypeHeap);
    COMPILE_ASSERT(EMemoryBlockTypeStack == WKC_MEMORYBLOCK_TYPE_STACK, EMemoryBlockTypeStack);
    COMPILE_ASSERT(EMemoryBlockTypeAligned == WKC_MEMORYBLOCK_TYPE_ALIGNED, EMemoryBlockTypeAligned);
    COMPILE_ASSERT(EMemoryBlockTypeMask == WKC_MEMORYBLOCK_TYPE_MASK, EMemoryBlockTypeMask);

    COMPILE_ASSERT(sizeof(TMemoryRangeInfo) == sizeof(WKCMemoryRangeInfo), sizeof__TMemoryRangeInfo);
    COMPILE_ASSERT(sizeof(TMemoryDetailedInfo) == sizeof(WKCMemoryDetailedInfo), sizeof__TMemoryDetailedInfo);
    COMPILE_ASSERT(sizeof(TMemoryInfo) == sizeof(WKCMemoryInfo), sizeof__TMemoryInfo);

    ret = (wkcMemoryGetMemoryInfoPeer((WKCMemoryInfo*)&memInfo) == 0) ? false : true;
#endif // WKC_ENABLE_USED_MEMORY_INFO
    return ret;
}

bool
WKCWebView::allocTMemoryInfo(WKC::TMemoryInfo& memInfo)
{
    bool ret = false;

#ifdef WKC_ENABLE_USED_MEMORY_INFO
#ifdef WKC_ENABLE_DYNAMIC_ALLOCATION
    COMPILE_ASSERT(EMemoryRangeTypeNormal == WKC_MEMORYRANGE_TYPE_DEFAULT, EMemoryRangeTypeNormal);
    COMPILE_ASSERT(EMemoryRangeTypeManagement == WKC_MEMORYRANGE_TYPE_MANAGEMENT, EMemoryRangeTypeManagement);
    COMPILE_ASSERT(EMemoryRangeTypeSpan == WKC_MEMORYRANGE_TYPE_SPAN, EMemoryRangeTypeSpan);
    COMPILE_ASSERT(EMemoryRangeTypeJSHeap == WKC_MEMORYRANGE_TYPE_JSHEAP, EMemoryRangeTypeJSHeap);
    COMPILE_ASSERT(EMemoryRangeTypeJSStack == WKC_MEMORYRANGE_TYPE_JSSTACK, EMemoryRangeTypeJSStack);
    COMPILE_ASSERT(EMemoryRangeTypeHash == WKC_MEMORYRANGE_TYPE_HASH, EMemoryRangeTypeHash);
    COMPILE_ASSERT(EMemoryRangeTypeThreadHeap == WKC_MEMORYRANGE_TYPE_THREADHEAP, EMemoryRangeTypeThreadHeap);
    COMPILE_ASSERT(EMemoryRangeTypes == WKC_MEMORYRANGE_TYPES, EMemoryRangeTypes);

    COMPILE_ASSERT(EMemoryBlockTypeNone == WKC_MEMORYBLOCK_TYPE_NONE, EMemoryBlockTypeNone);
    COMPILE_ASSERT(EMemoryBlockTypeAvailable == WKC_MEMORYBLOCK_TYPE_AVAILABLE, EMemoryBlockTypeAvailable);
    COMPILE_ASSERT(EMemoryBlockTypeUsed == WKC_MEMORYBLOCK_TYPE_USED, EMemoryBlockTypeUsed);
    COMPILE_ASSERT(EMemoryBlockTypeFreed == WKC_MEMORYBLOCK_TYPE_FREED, EMemoryBlockTypeFreed);
    COMPILE_ASSERT(EMemoryBlockTypeHeap == WKC_MEMORYBLOCK_TYPE_HEAP, EMemoryBlockTypeHeap);
    COMPILE_ASSERT(EMemoryBlockTypeStack == WKC_MEMORYBLOCK_TYPE_STACK, EMemoryBlockTypeStack);
    COMPILE_ASSERT(EMemoryBlockTypeAligned == WKC_MEMORYBLOCK_TYPE_ALIGNED, EMemoryBlockTypeAligned);
    COMPILE_ASSERT(EMemoryBlockTypeMask == WKC_MEMORYBLOCK_TYPE_MASK, EMemoryBlockTypeMask);

    COMPILE_ASSERT(sizeof(TMemoryRangeInfo) == sizeof(WKCMemoryRangeInfo), sizeof__TMemoryRangeInfo);
    COMPILE_ASSERT(sizeof(TMemoryDetailedInfo) == sizeof(WKCMemoryDetailedInfo), sizeof__TMemoryDetailedInfo);
    COMPILE_ASSERT(sizeof(TMemoryInfo) == sizeof(WKCMemoryInfo), sizeof__TMemoryInfo);

    ret = wkcMemoryAllocMemoryInfoPeer((WKCMemoryInfo*)&memInfo);
#endif // WKC_ENABLE_DYNAMIC_ALLOCATION
#endif // WKC_ENABLE_USED_MEMORY_INFO
    return ret;
}

void
WKCWebView::releaseTMemoryInfo(WKC::TMemoryInfo& memInfo)
{
#ifdef WKC_ENABLE_USED_MEMORY_INFO
#ifdef WKC_ENABLE_DYNAMIC_ALLOCATION
    wkcMemoryReleaseMemoryInfoPeer((WKCMemoryInfo*)&memInfo);
#endif // WKC_ENABLE_DYNAMIC_ALLOCATION
#endif // WKC_ENABLE_USED_MEMORY_INFO
}

extern "C" {
char* wkc_strdup(const char* in_str)
{
    int len = 0;
    char* ret = 0;

    if (!in_str) return 0;

    len = strlen(in_str);
    WTF::TryMallocReturnValue rv = tryFastMalloc(len + 1);
    if (!rv.getValue(ret)) return 0;
    strncpy(ret, in_str, len + 1);
    return ret;
}

int wkc_wstrlen(const unsigned short* in_str)
{
    int len = 0;

    if (!in_str) return 0;

    while (*in_str++) len++;

    return len;
}

unsigned short* wkc_wstrdup(const unsigned short* in_str)
{
    int len = 0;
    unsigned short* ret = 0;

    if (!in_str) return 0;
    len = wkc_wstrlen(in_str) + 1;
    WTF::TryMallocReturnValue rv = tryFastZeroedMalloc(sizeof(unsigned short)*len);
    if (!rv.getValue(ret)) return 0;
    memcpy((void *)ret, in_str, len*2);
    return ret;
}

unsigned short* wkc_wstrndup(const unsigned short* in_str, unsigned int in_length)
{
    unsigned short* ret = 0;

    if (!in_str || !in_length) return 0;
    WTF::TryMallocReturnValue rv = tryFastZeroedMalloc(sizeof(unsigned short)*(in_length+1));
    if (!rv.getValue(ret)) return 0;
    memcpy((void *)ret, in_str, in_length*2);
    return ret;
}

void wkc_wstrncpy(unsigned short* dest, unsigned int destmax, const unsigned short* src, unsigned int srclen)
{
    unsigned int i = 0;
    unsigned int len = srclen;

    if (!dest) return;

    if (destmax > 0) {
        if (len > destmax) {
            len = destmax;
        }
    }

    for (i=0; i<len; i++) {
        dest[i] = src[i];
    }
    dest[i] = 0;
}

}

// idn

#if 0
extern "C" {
WKC_API int wkcI18NIDNtoUnicodePeer(const unsigned char* in_host, int len, unsigned short* out_idn, int maxlen);
WKC_API int wkcI18NIDNfromUnicodePeer(const unsigned short* in_idn, int len, unsigned char* out_host, int maxlen);
}

namespace IDN {

int
toUnicode(const char* host, unsigned short* idn, int maxidn)
{
    return wkcI18NIDNtoUnicodePeer((const unsigned char *)host, -1, idn, maxidn);
}

int
fromUnicode(const unsigned short* idn, char* host, int maxhost)
{
    return wkcI18NIDNfromUnicodePeer(idn, -1, (unsigned char *)host, maxhost);
}

} // namespace
#endif

} // namespace
