/*
 *  WKCWebFrame.cpp
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

#include "config.h"
#include "WKCWebFrame.h"
#include "WKCWebFramePrivate.h"
#include "FrameLoaderClientWKC.h"
#include "WKCWebView.h"
#include "WKCWebViewPrivate.h"

#include "Frame.h"
#include "FrameView.h"
#include "KURL.h"
#include "SharedBuffer.h"
#include "PlatformString.h"
#include "JavaScriptCore/APICast.h"
#include "JSDOMBinding.h"
#include "SubstituteData.h"
#include "SubresourceLoader.h"
#include "IconLoader.h"
#include <image-decoders/ImageDecoder.h>
#if ENABLE(WKC_PAGESAVE_MHTML)
#include "ArchiveFactory.h"
#include "DocumentLoader.h"
#include "mhtml/MHTMLWebArchive.h"
#endif

#include "helpers/privates/WKCFramePrivate.h"
#include "helpers/privates/WKCHTMLFrameOwnerElementPrivate.h"

static const unsigned short cNullWStr[] = {0};

namespace WKC {

// private container
WKCWebFramePrivate::WKCWebFramePrivate(WKCWebFrame* parent, WKCWebViewPrivate* view, WKCClientBuilders& builders, WebCore::HTMLFrameOwnerElement* ownerelement)
     : m_parent(parent),
       m_view(view),
       m_builders(builders),
       m_ownerElement(ownerelement),
       m_coreFrame(0),
       m_wkcCoreFrame(0),
       m_loadStatus(ELoadStatusNone),
       m_uri(0),
       m_title(0),
       m_name(0),
       m_faviconURL(0)
#if ENABLE(WKC_PAGESAVE_MHTML)
     , m_archive(0)
#endif
{
    m_forceTerminated = false;
}

WKCWebFramePrivate::~WKCWebFramePrivate()
{
    if (m_forceTerminated) {
        return;

    }

    if (m_wkcCoreFrame) {
        delete m_wkcCoreFrame;
        m_wkcCoreFrame = 0;
    }

    if (m_coreFrame) {
        m_coreFrame->loader()->cancelAndClear();
        m_coreFrame = 0;
        // m_coreFrame would be deleted automatically
    }

    if (m_uri) {
        fastFree(m_uri);
        m_uri = 0;
    }
    if (m_title) {
        fastFree(m_title);
        m_title = 0;
    }
    if (m_name) {
        fastFree(m_name);
        m_name = 0;
    }
    if (m_faviconURL) {
        fastFree(m_faviconURL);
        m_faviconURL = 0;
    }
#if ENABLE(WKC_PAGESAVE_MHTML)
    if (m_archive) {
        m_archive->freeTmpHeap();
        m_archive->deref();
        m_archive = 0;
    }
#endif
}

WKCWebFramePrivate*
WKCWebFramePrivate::create(WKCWebFrame* parent, WKCWebViewPrivate* view, WKCClientBuilders& builders, WebCore::HTMLFrameOwnerElement* ownerelement)
{
    WKCWebFramePrivate* self = 0;
    self = new WKCWebFramePrivate(parent, view, builders, ownerelement);
    if (!self) return self;
    if (!self->construct()) {
        delete self;
        return 0;
    }
    return self;
}

bool
WKCWebFramePrivate::construct()
{
    WKC::FrameLoaderClientWKC* client = 0;
    WebCore::HTMLFrameOwnerElement* parent = 0;

    ASSERT(m_view);

    client = FrameLoaderClientWKC::create(this);
    if (!client) return false;
    if (m_ownerElement) {
        parent = m_ownerElement;
    }
    RefPtr<WebCore::Frame> newframe = WebCore::Frame::create(m_view->core(), parent, client);
    if (!newframe) {
        delete client;
        return false;
    }
    m_coreFrame = newframe.get();
    if (m_ownerElement) {
        m_coreFrame->ref();
    }

    m_wkcCoreFrame = new FramePrivate(m_coreFrame);

    return true;
}

void WKCWebFrame::deleteWKCWebFrame(WKCWebFrame *self)
{
    delete self;
}

void
WKCWebFramePrivate::notifyForceTerminate()
{
    m_forceTerminated = true;
}

void
WKCWebFramePrivate::coreFrameDestroyed()
{
    m_coreFrame = 0;
}


// implementations

WKCWebFrame::WKCWebFrame()
     : m_private(0)
{
}

WKCWebFrame::~WKCWebFrame()
{
    if (m_private) {
        delete m_private;
    }
}

WKCWebFrame*
WKCWebFrame::create(WKCWebView* view, WKCClientBuilders& builders)
{
    return WKCWebFrame::create(view->m_private, builders);
}


WKCWebFrame*
WKCWebFrame::create(WKCWebViewPrivate* view, WKCClientBuilders& builders, WKC::HTMLFrameOwnerElement* ownerelement)
{
    WKCWebFrame* self = 0;

    self = new WKCWebFrame();
    if (!self) return 0;
    if (!self->construct(view, builders, ownerelement)) {
        delete self;
        return 0;
    }
    return self;
}

bool
WKCWebFrame::construct(WKCWebViewPrivate* view, WKCClientBuilders& builders, WKC::HTMLFrameOwnerElement* ownerelement)
{
    WebCore::HTMLFrameOwnerElement* owner = 0;
    if (ownerelement) {
        owner = reinterpret_cast<WebCore::HTMLFrameOwnerElement*>(ownerelement->priv().webcore());
    }
    m_private = WKCWebFramePrivate::create(this, view, builders, owner);
    if (!m_private) return false;
    m_private->core()->init();
    return true;
}

void
WKCWebFrame::notifyForceTerminate()
{
    if (m_private) {
        m_private->notifyForceTerminate();
    }
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

// APIs
WKC::Frame*
WKCWebFrame::core() const
{
    return &m_private->wkcCore()->wkc();
}

bool
WKCWebFrame::compare(const WKC::Frame* frame) const
{
    if (!frame)
        return false;
    return (m_private->wkcCore()->webcore() == const_cast<WKC::Frame *>(frame)->priv().webcore());
}

WKCWebView*
WKCWebFrame::webView()
{
    return m_private->m_view->parent();
}

const unsigned short*
WKCWebFrame::name()
{
    if (m_private->m_name) {
        return m_private->m_name;
    }

    WebCore::Frame* coreFrame = m_private->core();
    if (!coreFrame) {
        return cNullWStr;
    }

    WebCore::String string = coreFrame->tree()->name();
    m_private->m_name = wkc_wstrdup(string.charactersWithNullTermination());
    return m_private->m_name;
}
const unsigned short*
WKCWebFrame::title()
{
    return m_private->m_title;
}
const char*
WKCWebFrame::uri()
{
    return m_private->m_uri;
}
WKCWebFrame*
WKCWebFrame::parent()
{
    WebCore::Frame* coreFrame = m_private->core();
    if (!coreFrame) {
        return 0;
    }

    return kit(coreFrame->tree()->parent());
}

void
WKCWebFrame::loadURI(const char* uri)
{
    WebCore::Frame* coreFrame = m_private->core();
    if (!coreFrame) {
        return;
    }

    coreFrame->loader()->load(WebCore::ResourceRequest(WebCore::KURL(WebCore::KURL(), WebCore::String::fromUTF8(uri))), false);
}

#ifdef __MINGW32__
# ifdef LoadString
#  undef LoadString
# endif
#endif
void
WKCWebFrame::loadString(const char* content, const unsigned short* mime_type, const unsigned short* encoding, const char *base_uri, const char *unreachable_uri)
{
    WebCore::Frame* coreFrame = m_private->core();
    WebCore::FrameLoader* loader = coreFrame->loader();

    WebCore::KURL baseKURL = (base_uri && base_uri[0]) ? WebCore::KURL(WebCore::KURL(), WebCore::String::fromUTF8(base_uri)) : WebCore::blankURL();

    WebCore::ResourceRequest request(baseKURL);

    WTF::RefPtr<WebCore::SharedBuffer> sharedBuffer = WebCore::SharedBuffer::create(content, strlen(content));
    WebCore::SubstituteData substituteData(sharedBuffer.release(),
                                           mime_type ? WebCore::String(mime_type) : WebCore::String::fromUTF8("text/html"),
                                           encoding ? WebCore::String(encoding) : WebCore::String::fromUTF8("UTF-8"),
                                           WebCore::KURL(WebCore::KURL(), WebCore::String::fromUTF8(unreachable_uri)),
                                           baseKURL);
    
    loader->load(request, substituteData, false);
}


void
WKCWebFrame::stopLoading()
{
    WebCore::Frame* coreFrame = m_private->core();
    if (!coreFrame) {
        return;
    }

    coreFrame->loader()->stopAllLoaders();
}
void
WKCWebFrame::reload()
{
    WebCore::Frame* coreFrame = m_private->core();
    if (!coreFrame) {
        return;
    }

    coreFrame->loader()->reload();
}

WKCWebFrame*
WKCWebFrame::findFrame(const unsigned short* name)
{
    WebCore::Frame* coreFrame = m_private->core();
    if (!coreFrame) {
        return 0;
    }

    WebCore::String nameString = WebCore::String(name);
    return kit(coreFrame->tree()->find(WebCore::AtomicString(nameString)));
}

WKC::LoadStatus
WKCWebFrame::loadStatus()
{
    return m_private->m_loadStatus;
}
WKC::ScrollbarMode
WKCWebFrame::horizontalScrollbarMode()
{
    WebCore::Frame* coreFrame = m_private->core();
    WebCore::FrameView* view = coreFrame->view();
    if (!view) {
        return WKC::EScrollbarAuto;
    }

    WebCore::ScrollbarMode hMode = view->horizontalScrollbarMode();

    if (hMode == WebCore::ScrollbarAlwaysOn) {
        return WKC::EScrollbarAlwaysOn;
    }

    if (hMode == WebCore::ScrollbarAlwaysOff) {
        return WKC::EScrollbarAlwaysOff;
    }

    return WKC::EScrollbarAuto;
}
WKC::ScrollbarMode
WKCWebFrame::verticalScrollbarMode()
{
    WebCore::Frame* coreFrame = m_private->core();
    WebCore::FrameView* view = coreFrame->view();
    if (!view) {
        return WKC::EScrollbarAuto;
    }

    WebCore::ScrollbarMode hMode = view->verticalScrollbarMode();

    if (hMode == WebCore::ScrollbarAlwaysOn) {
        return WKC::EScrollbarAlwaysOn;
    }

    if (hMode == WebCore::ScrollbarAlwaysOff) {
        return WKC::EScrollbarAlwaysOff;
    }

    return WKC::EScrollbarAuto;
}

WKCSecurityOrigin*
WKCWebFrame::securityOrigin()
{
    // Ugh!: implement it!
    // 100106 ACCESS Co.,Ltd.
    return 0;
}

const char*
WKCWebFrame::faviconURL()
{
    WebCore::FrameLoader* frameLoader = m_private->core()->loader();

    if (frameLoader->state() == WebCore::FrameStateComplete) {
        const WebCore::KURL& url = frameLoader->iconURL();
        if (!url.isEmpty()) {
            if (m_private->m_faviconURL)
                fastFree(m_private->m_faviconURL);
            m_private->m_faviconURL = wkc_strdup(url.string().utf8().data());
            return m_private->m_faviconURL;
        }
    }

    return 0;
}

bool
WKCWebFrame::getFaviconInfo(WKCFaviconInfo* info)
{
    WebCore::FrameLoader* frameLoader = m_private->core()->loader();

    WebCore::IconLoader* iconLoader = frameLoader->iconLoader();
    if (iconLoader) {
        WebCore::SubresourceLoader* resourceLoader = iconLoader->resourceLoader();
        if (resourceLoader) {
            RefPtr<WebCore::SharedBuffer> data = resourceLoader->resourceData();
            if (data && !data->isEmpty()) {
                /* decode the image */
                WebCore::NativeImageSourcePtr decoder = static_cast<WebCore::NativeImageSourcePtr>(WebCore::ImageDecoder::create(*(data.get())));
                if (!decoder) return false;
                decoder->setData(data.get(), true);
                size_t index, num;
                WebCore::RGBA32Buffer* rgbBuf;

                num = decoder->frameCount();
                if (num < 1) {
                    delete decoder;
                    return false;
                }
                /* search for a 16x16 icon */
                for (index = 0; index < num; index++) {
                    rgbBuf = decoder->frameBufferAtIndex(index);
                    if (!rgbBuf) {
                        delete decoder;
                        return false;
                    }
                    if (rgbBuf->width() == 16 && rgbBuf->height() == 16) {
                        break;
                    }
                }
                /* use the first icon if no 16x16 icon was found */
                if (index == num) {
                    rgbBuf = decoder->frameBufferAtIndex(0);
                }

                info->fIconSize = data->size();
                info->fIconBmpBpp = rgbBuf->bpp();
                info->fIconBmpHeight = rgbBuf->height();
                info->fIconBmpWidth = rgbBuf->width();
                if (info->fIconData) {
                    memcpy(info->fIconData, data->data(), data->size());
                }
                if (info->fIconBmpData) {
                    memcpy(info->fIconBmpData, rgbBuf->bytes(), info->fIconBmpBpp * info->fIconBmpHeight * info->fIconBmpWidth);
                }
                if (rgbBuf->mask()) {
                    info->fHasMask = true;
                    if (info->fIconBmpMask) {
                        memcpy(info->fIconBmpMask, rgbBuf->mask(), info->fIconBmpHeight * info->fIconBmpWidth);
                    }
                }
                delete decoder;
                return true;
            }
        }
    }
    return false;
}

#if ENABLE(WKC_PAGESAVE_MHTML)
bool
WKCWebFrame::contentSerializeStart()
{
    if (m_private->m_archive) {
        m_private->m_archive->freeTmpHeap();
        m_private->m_archive->deref();
    }

    m_private->m_archive = WebCore::MHTMLWebArchive::create(m_private->core()).releaseRef();
    if (!m_private->m_archive)
        return false;

    if (!m_private->m_archive->contentSerializeStart()) {
        m_private->m_archive->freeTmpHeap();
        m_private->m_archive->deref();
        m_private->m_archive = 0;
        return false;
    }
    return true;
}

int
WKCWebFrame::contentSerializeProgress(void* buffer, unsigned int length)
{
    if (!m_private->m_archive)
        return -1;

    return m_private->m_archive->contentSerializeProgress(buffer, length);
}

void
WKCWebFrame::contentSerializeEnd()
{
    if (!m_private->m_archive)
        return;

    m_private->m_archive->contentSerializeEnd();
    m_private->m_archive->deref();
    m_private->m_archive = 0;
}

bool
WKCWebFrame::isPageArchiveLoadFailed()
{
    RefPtr<WebCore::DocumentLoader> dl = m_private->core()->loader()->activeDocumentLoader();
    if (WebCore::ArchiveFactory::isArchiveMimeType(dl->responseMIMEType())) {        
        if (!dl->parsedArchiveData())
            return true;
    }
    return false;
}
#endif
} // namespace
