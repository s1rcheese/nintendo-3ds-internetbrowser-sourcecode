/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2007 Holger Hans Peter Freyther <zecke@selfish.org>
 * Copyright (C) 2008, 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.
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
#include "ImageBuffer.h"

#include "Base64.h"
#include "BitmapImage.h"
#include "Color.h"
#include "GraphicsContext.h"
#include "ImageData.h"
#include "ImageWKC.h"
#include "MIMETypeRegistry.h"
#include "Pattern.h"
#include "PlatformString.h"
#include "FastMalloc.h"

#include "NotImplemented.h"

#if PLATFORM(WKC)
#include <wkc/wkcmpeer.h>
#endif

using namespace std;

namespace WebCore {

static bool gUseBilinear = false;
static bool gUseAA = true;

void
ImageBufferData::setUseBilinear(bool flag)
{
    gUseBilinear = flag;
}

void
ImageBufferData::setUseAA(bool flag)
{
    gUseAA = flag;
}

void
ImageBufferData::resetVariables()
{
    gUseBilinear = false;
    gUseAA = true;
}

ImageBufferData::ImageBufferData(const IntSize& size)
    : m_image(0)
{
}

ImageBuffer::ImageBuffer(const IntSize& size, ImageColorSpace imageColorSpace, bool& success)
    : m_data(size)
    , m_size(size)
{
    success = false;
    void* bitmap = 0;
    int rowbytes = 0;
    GraphicsContext* ctx = 0;
    void* offscreen = 0;
    void* dc = 0;
    WKCSize osize;

    osize.fWidth = size.width();
    osize.fHeight = size.height();
    offscreen = wkcOffscreenNewPeer(WKC_OFFSCREEN_TYPE_IMAGEBUF, 0, 0, &osize);
    if (!offscreen) {
        return;
    }
    bitmap = wkcOffscreenBitmapPeer(offscreen, &rowbytes);
    if (!bitmap) {
        wkcOffscreenDeletePeer(offscreen);
        return;
    }

    dc = wkcDrawContextNewPeer(offscreen);
    if (!dc) {
        wkcMemoryNotifyMemoryAllocationErrorPeer(osize.fWidth*osize.fHeight * 4, WKC_MEMORYALLOC_TYPE_IMAGE);
        wkcOffscreenDeletePeer(offscreen);
        return;
    }

    ctx = new GraphicsContext(dc);
    if (!ctx) {
        wkcMemoryNotifyMemoryAllocationErrorPeer(osize.fWidth*osize.fHeight * 4, WKC_MEMORYALLOC_TYPE_IMAGE);
        wkcDrawContextDeletePeer(dc);
        wkcOffscreenDeletePeer(offscreen);
        return;
    }

    m_data.m_image = new ImageWKC(bitmap, size, ImageWKC::EColorARGB8888, false);
    if (!m_data.m_image) {
        wkcMemoryNotifyMemoryAllocationErrorPeer(osize.fWidth*osize.fHeight * 4, WKC_MEMORYALLOC_TYPE_IMAGE);
        wkcDrawContextDeletePeer(dc);
        wkcOffscreenDeletePeer(offscreen);
        return;
    }

    m_context.set(ctx);

    wkcOffscreenSetUseInterpolationForImagePeer(offscreen, gUseBilinear);
    wkcOffscreenSetUseAntiAliasForPolygonPeer(offscreen, gUseAA);

    FloatRect fr(0, 0, m_size.width(), m_size.height());
    ctx->clip(fr);

    success = true;
}

ImageBuffer::~ImageBuffer()
{
    GraphicsContext* ctx = 0;
    void* dc;
    void* offscreen;

    if (m_data.m_image) {
        delete m_data.m_image;
    }

    ctx = m_context.release();

    if (ctx) {
        dc = ctx->platformContext();
        offscreen = wkcDrawContextGetOffscreenPeer(dc);

        delete ctx;
        wkcDrawContextDeletePeer(dc);
        wkcOffscreenDeletePeer(offscreen);
    }
}

GraphicsContext* ImageBuffer::context() const
{
    return m_context.get();
}

Image* ImageBuffer::image() const
{
    if (!m_data.m_image) return 0;

    if (!m_image) {
        void* newsurface = 0;
        ImageWKC* img = 0;

        wkcOffscreenFlushPeer(wkcDrawContextGetOffscreenPeer(context()->platformContext()), WKC_OFFSCREEN_FLUSH_FOR_DRAW);

        // consider divide memory area for allocating image / others
        wkcMemorySetAllocatingForImagesPeer(true);
        WTF::TryMallocReturnValue rv = tryFastZeroedMalloc(m_data.m_image->rowbytes() * m_data.m_image->size().height());
        wkcMemorySetAllocatingForImagesPeer(false);
        if (!rv.getValue(newsurface)) {
            wkcMemoryNotifyMemoryAllocationErrorPeer(m_data.m_image->rowbytes() * m_data.m_image->size().height(), WKC_MEMORYALLOC_TYPE_IMAGE);
            newsurface = 0;
        } else {
            memcpy(newsurface, m_data.m_image->bitmap(), m_data.m_image->rowbytes() * m_data.m_image->size().height());
        }
        img = new ImageWKC(newsurface, m_data.m_image->size(), ImageWKC::EColorARGB8888);
        if (!img) {
            wkcMemoryNotifyMemoryAllocationErrorPeer(m_data.m_image->size().width() * m_data.m_image->size().height() * 4, WKC_MEMORYALLOC_TYPE_IMAGE);
            if (newsurface) {
                fastFree(newsurface);
            }
            return 0;
        }
        m_image = BitmapImage::create((void *)img);
        if (!m_image) {
            wkcMemoryNotifyMemoryAllocationErrorPeer(m_data.m_image->size().width() * m_data.m_image->size().height() * 4, WKC_MEMORYALLOC_TYPE_IMAGE);
            delete img; // newsurface was already owned by img
            return 0;
        }
    }
    return m_image.get();
}

void ImageBuffer::platformTransformColorSpace(const Vector<int>& lookUpTable)
{
    if (!m_data.m_image) return;

    wkcOffscreenFlushPeer(wkcDrawContextGetOffscreenPeer(context()->platformContext()), WKC_OFFSCREEN_FLUSH_FOR_READPIXELS);

    unsigned char* dataSrc = (unsigned char *)m_data.m_image->bitmap();
    if (!dataSrc) return;
    int stride = m_data.m_image->rowbytes();
    for (int y = 0; y < m_size.height(); ++y) {
        unsigned* row = reinterpret_cast<unsigned*>(dataSrc + stride * y);
        for (int x = 0; x < m_size.width(); x++) {
            unsigned* pixel = row + x;
            Color pixelColor = colorFromPremultipliedARGB(*pixel);
            pixelColor = Color(lookUpTable[pixelColor.red()],
                               lookUpTable[pixelColor.green()],
                               lookUpTable[pixelColor.blue()],
                               pixelColor.alpha());
            *pixel = premultipliedARGBFromColor(pixelColor);
        }
    }
}

template <Multiply multiplied>
PassRefPtr<ImageData> getImageData(const IntRect& rect, const ImageBufferData& data, const IntSize& size)
{
    PassRefPtr<ImageData> result = ImageData::create(rect.width(), rect.height());
    if (!data.m_image) return result;
    unsigned char* dataSrc = (unsigned char *)data.m_image->bitmap();
    unsigned char* dataDst = result->data()->data()->data();
    if (!dataSrc) return result;
    if (!dataDst) return result;
    if (rect.x() < 0 || rect.y() < 0 || (rect.x() + rect.width()) > size.width() || (rect.y() + rect.height()) > size.height())
        memset(dataDst, 0, result->data()->length());

    int originx = rect.x();
    int destx = 0;
    if (originx < 0) {
        destx = -originx;
        originx = 0;
    }
    int endx = rect.x() + rect.width();
    if (endx > size.width())
        endx = size.width();
    int numColumns = endx - originx;

    int originy = rect.y();
    int desty = 0;
    if (originy < 0) {
        desty = -originy;
        originy = 0;
    }
    int endy = rect.y() + rect.height();
    if (endy > size.height())
        endy = size.height();
    int numRows = endy - originy;

    int stride = data.m_image->rowbytes();
    unsigned destBytesPerRow = 4 * rect.width();

    unsigned char* destRows = dataDst + desty * destBytesPerRow + destx * 4;
    for (int y = 0; y < numRows; ++y) {
        unsigned* row = reinterpret_cast<unsigned*>(dataSrc + stride * (y + originy));
        for (int x = 0; x < numColumns; x++) {
            int basex = x * 4;
            unsigned* pixel = row + x + originx;
            Color pixelColor;
            if (multiplied == Unmultiplied)
                pixelColor = colorFromPremultipliedARGB(*pixel);
            else
                pixelColor = Color(*pixel);
            destRows[basex]     = pixelColor.red();
            destRows[basex + 1] = pixelColor.green();
            destRows[basex + 2] = pixelColor.blue();
            destRows[basex + 3] = pixelColor.alpha();
        }
        destRows += destBytesPerRow;
    }

    return result;
}

PassRefPtr<ImageData> ImageBuffer::getUnmultipliedImageData(const IntRect& rect) const
{
    wkcOffscreenFlushPeer(wkcDrawContextGetOffscreenPeer(context()->platformContext()), WKC_OFFSCREEN_FLUSH_FOR_READPIXELS);

    return getImageData<Unmultiplied>(rect, m_data, m_size);
}

PassRefPtr<ImageData> ImageBuffer::getPremultipliedImageData(const IntRect& rect) const
{
    wkcOffscreenFlushPeer(wkcDrawContextGetOffscreenPeer(context()->platformContext()), WKC_OFFSCREEN_FLUSH_FOR_READPIXELS);

    return getImageData<Premultiplied>(rect, m_data, m_size);
}

template <Multiply multiplied>
void putImageData(ImageData*& source, const IntRect& sourceRect, const IntPoint& destPoint, ImageBufferData& data, const IntSize& size)
{
    if (!data.m_image) return;
    unsigned char* dataDst = (unsigned char *)data.m_image->bitmap();
    if (!dataDst) return;
    ASSERT(sourceRect.width() > 0);
    ASSERT(sourceRect.height() > 0);

    int originx = sourceRect.x();
    int destx = destPoint.x() + sourceRect.x();
    ASSERT(destx >= 0);
    ASSERT(destx < size.width());
    ASSERT(originx >= 0);
    ASSERT(originx <= sourceRect.right());

    int endx = destPoint.x() + sourceRect.right();
    ASSERT(endx <= size.width());

    int numColumns = endx - destx;

    int originy = sourceRect.y();
    int desty = destPoint.y() + sourceRect.y();
    ASSERT(desty >= 0);
    ASSERT(desty < size.height());
    ASSERT(originy >= 0);
    ASSERT(originy <= sourceRect.bottom());

    int endy = destPoint.y() + sourceRect.bottom();
    ASSERT(endy <= size.height());
    int numRows = endy - desty;

    unsigned srcBytesPerRow = 4 * source->width();
    int stride = data.m_image->rowbytes();

    unsigned char* srcRows = source->data()->data()->data() + originy * srcBytesPerRow + originx * 4;
    for (int y = 0; y < numRows; ++y) {
        unsigned* row = reinterpret_cast<unsigned*>(dataDst + stride * (y + desty));
        for (int x = 0; x < numColumns; x++) {
            int basex = x * 4;
            unsigned* pixel = row + x + destx;
            Color pixelColor(srcRows[basex],
                    srcRows[basex + 1],
                    srcRows[basex + 2],
                    srcRows[basex + 3]);
            if (multiplied == Unmultiplied)
                *pixel = premultipliedARGBFromColor(pixelColor);
            else
                *pixel = pixelColor.rgb();
        }
        srcRows += srcBytesPerRow;
    }
}

void ImageBuffer::putUnmultipliedImageData(ImageData* source, const IntRect& sourceRect, const IntPoint& destPoint)
{
    wkcOffscreenFlushPeer(wkcDrawContextGetOffscreenPeer(context()->platformContext()), WKC_OFFSCREEN_FLUSH_FOR_READPIXELS);
    putImageData<Unmultiplied>(source, sourceRect, destPoint, m_data, m_size);
}

void ImageBuffer::putPremultipliedImageData(ImageData* source, const IntRect& sourceRect, const IntPoint& destPoint)
{
    wkcOffscreenFlushPeer(wkcDrawContextGetOffscreenPeer(context()->platformContext()), WKC_OFFSCREEN_FLUSH_FOR_READPIXELS);
    putImageData<Premultiplied>(source, sourceRect, destPoint, m_data, m_size);
}

String ImageBuffer::toDataURL(const String& mimeType) const
{
    notImplemented();
    return "data:,";
}

} // namespace WebCore
