/*
 * Copyright (C) 2007 Apple Computer, Kevin Ollivier.  All rights reserved.
 * Copyright (c) 2008, Google Inc. All rights reserved.
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
#include "ImageWKC.h"

#include "BitmapImage.h"
#include "GraphicsContext.h"
#include "ImageObserver.h"
#include "FastMalloc.h"
#include "TransformationMatrix.h"
#include "FloatConversion.h"
#include "ImageDecoderWKC.h"
#include "ImageSource.h"

#include "NotImplemented.h"

namespace WebCore {

static inline void platformRect(const FloatRect& in, WKCFloatRect& out)
{
    out.fX = in.x();
    out.fY = in.y();
    out.fWidth = in.width();
    out.fHeight = in.height();
}

static inline int platformOperator(CompositeOperator op)
{
    switch (op) {
    case CompositeClear:
        return WKC_COMPOSITEOPERATION_CLEAR;
    case CompositeCopy:
        return WKC_COMPOSITEOPERATION_COPY;
    case CompositeSourceOver:
        return WKC_COMPOSITEOPERATION_SOURCEOVER;
    case CompositeSourceIn:
        return WKC_COMPOSITEOPERATION_SOURCEIN;
    case CompositeSourceOut:
        return WKC_COMPOSITEOPERATION_SOURCEOUT;
    case CompositeSourceAtop:
        return WKC_COMPOSITEOPERATION_SOURCEATOP;
    case CompositeDestinationOver:
        return WKC_COMPOSITEOPERATION_DESTINATIONOVER;
    case CompositeDestinationIn:
        return WKC_COMPOSITEOPERATION_DESTINATIONIN;
    case CompositeDestinationOut:
        return WKC_COMPOSITEOPERATION_DESTINATIONOUT;
    case CompositeDestinationAtop:
        return WKC_COMPOSITEOPERATION_DESTINATIONATOP;
    case CompositeXOR:
        return WKC_COMPOSITEOPERATION_XOR;
    case CompositePlusDarker:
        return WKC_COMPOSITEOPERATION_PLUSDARKER;
    case CompositeHighlight:
        return WKC_COMPOSITEOPERATION_HIGHLIGHT;
    case CompositePlusLighter:
        return WKC_COMPOSITEOPERATION_PLUSLIGHTER;
    default:
        return 0;
    }
}

ImageWKC::ImageWKC(void* bitmap, const IntSize& size, int type, bool ownbitmap)
    : m_bitmap(bitmap),
      m_buffer(0),
      m_size(size),
      m_type(type),
      m_ownbitmap(ownbitmap)
{
    int bpp = 4;

    switch (type) {
    case EColorRGAB5515:
    case EColorRGAB5515MASK:
        bpp = 2;
        break;
    case EColorARGB1232:
        bpp = 1;
        break;
    default:
        bpp = 4;
    }
    m_rowbytes = size.width() * bpp;
}

ImageWKC::ImageWKC(RGBA32Buffer* buffer)
    : m_bitmap(0),
      m_buffer(buffer),
      m_rowbytes(0),
      m_size(IntSize(0,0)),
      m_type(0),
      m_ownbitmap(false)
{
}

ImageWKC::~ImageWKC()
{
    if (m_bitmap && m_ownbitmap) {
        fastFree(m_bitmap);
        m_bitmap = 0;
    }
}

void*
ImageWKC::bitmap() const
{
    if (m_buffer) {
        return m_buffer->bytes();
    }
    return m_bitmap;
}

int 
ImageWKC::rowbytes() const
{
    if (m_buffer) {
        return m_buffer->width() * m_buffer->bpp();
    } else {
        return m_rowbytes;
    }
}

void*
ImageWKC::mask() const
{
    if (!m_buffer) return 0;
    return m_buffer->mask();

}

int
ImageWKC::maskrowbytes() const
{
    if (!m_buffer) return 0;
    if (!m_buffer->mask()) return 0;
    return m_buffer->width();
}

const IntSize& 
ImageWKC::size()
{
    if (m_buffer) {
        m_size.setWidth(m_buffer->width());
        m_size.setHeight(m_buffer->height());
        return m_size;
    } else {
        return m_size;
    }
}

int 
ImageWKC::type() const
{
    if (m_buffer) {
        if (m_buffer->bpp()==1) {
            return EColorARGB1232;
        } else if (m_buffer->bpp()==2) {
            if (m_buffer->mask()) {
                return EColorRGAB5515MASK;
            } else {
                return EColorRGAB5515;
            }
        } else {
            return EColorARGB8888;
        }
    } else {
        return m_type;
    }
}

bool
ImageWKC::hasAlpha() const
{
    if (m_buffer) {
        return m_buffer->hasAlpha();
    } else {
        return true;
    }
}

bool
ImageWKC::hasTrueAlpha() const
{
    if (m_buffer) {
        return m_buffer->hasTrueAlpha();
    } else {
        return true;
    }
}

double
ImageWKC::scalex() const
{
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING) || ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
    if (m_buffer) {
        return m_buffer->scalex();
    }
#endif
    return 1.0;
}

double
ImageWKC::scaley() const
{
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING) || ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
    if (m_buffer) {
        return m_buffer->scaley();
    }
#endif
    return 1.0;
}

BitmapImage::BitmapImage(void* in_image, ImageObserver* in_observer)
    : Image(in_observer)
    , m_currentFrame(0)
    , m_frames(0)
    , m_frameTimer(0)
    , m_repetitionCount(cAnimationNone)
    , m_repetitionCountStatus(Unknown)
    , m_repetitionsComplete(0)
    , m_isSolidColor(false)
    , m_checkedForSolidColor(false)
    , m_animationFinished(true)
    , m_allDataReceived(true)
    , m_haveSize(true)
    , m_sizeAvailable(true)
    , m_decodedSize(0)
    , m_haveFrameCount(true)
    , m_frameCount(1)
{
    ImageWKC* img = (ImageWKC *)in_image;
    int mul = 1;

    m_desiredFrameStartTime = 0;
    m_hasUniformFrameSize = false;

    initPlatformData();

    int width = img->size().width();
    int height = img->size().height();
    switch (img->type()) {
    case ImageWKC::EColorARGB8888:
        mul = 1; break;
    case ImageWKC::EColorRGAB5515:
    case ImageWKC::EColorRGAB5515MASK:
        mul = 2; break;
    case ImageWKC::EColorARGB1232:
        mul = 4; break;
    }
    m_decodedSize = img->rowbytes() * height * mul;

    m_size = IntSize(width, height);

    m_frames.grow(1);
    m_frames[0].m_frame = in_image;
    m_frames[0].m_hasAlpha = false;
    m_frames[0].m_haveMetadata = true;
    checkForSolidColor();
}

void BitmapImage::initPlatformData()
{
}

void BitmapImage::invalidatePlatformData()
{
}

bool FrameData::clear(bool clearMetadata)
{
    if (clearMetadata) {
        m_haveMetadata = false;
    }

    if (m_frame) {
        ImageWKC* img = (ImageWKC *)m_frame;
        if (img) {
            delete img;
        }
        m_frame = NULL;
        return true;
    }
    return false;
}

void BitmapImage::draw(GraphicsContext* context, const FloatRect& dst, const FloatRect& src, ColorSpace styleColorSpace, CompositeOperator op)
{
    // Spin the animation to the correct frame before we try to draw it, so we
    // don't draw an old frame and then immediately need to draw a newer one,
    // causing flicker and wasting CPU.
    startAnimation();

    void* dc = (void *)context->platformContext();
    ImageWKC* bitmap = (ImageWKC *)frameAtIndex(m_currentFrame);
    int type = WKC_IMAGETYPE_RGBA8888;

    if (!bitmap) // If it's too early we won't have an image yet.
        return;

    WKCFloatRect idst;
    WKCFloatRect isrc;

    double scalex = 1.f;
    double scaley = 1.f;

    switch (bitmap->type()) {
    case ImageWKC::EColorARGB8888:
        type = WKC_IMAGETYPE_RGBA8888;
        break;
    case ImageWKC::EColorRGAB5515:
        type = WKC_IMAGETYPE_RGAB5515;
        break;
    case ImageWKC::EColorRGAB5515MASK:
        type = WKC_IMAGETYPE_RGAB5515MASK;
        break;
    case ImageWKC::EColorARGB1232:
        type = WKC_IMAGETYPE_ARGB1232;
        break;
    default:
        goto end;
    }
    if (bitmap->hasAlpha()) {
        type |= WKC_IMAGETYPE_FLAG_HASALPHA;
        if (bitmap->hasTrueAlpha()) {
            type |= WKC_IMAGETYPE_FLAG_HASTRUEALPHA;
        }
    }

    platformRect(dst, idst);
    platformRect(src, isrc);

    scalex = bitmap->scalex();
    scaley = bitmap->scaley();
    isrc.fX = ceil((double)isrc.fX * scalex);
    isrc.fY = ceil((double)isrc.fY * scaley);
    isrc.fWidth = floor((double)isrc.fWidth * scalex);
    isrc.fHeight = floor((double)isrc.fHeight * scaley);

    if (isrc.fWidth && isrc.fHeight) {
        if (bitmap->bitmap()) {
            WKCPeerImage img = {0};
            img.fType = type;
            img.fBitmap = bitmap->bitmap();
            img.fRowBytes = bitmap->rowbytes();
            img.fMask = bitmap->mask();
            img.fMaskRowBytes = bitmap->maskrowbytes();
            WKCFloatRect_SetXYWH(&img.fSrcRect, isrc.fX, isrc.fY, isrc.fWidth, isrc.fHeight);
            WKCFloatSize_Set(&img.fScale, scalex, scaley);
            img.fiScale.fWidth = ((img.fScale.fWidth!= 0) ? 1.f / img.fScale.fWidth : 0);
            img.fiScale.fHeight = ((img.fScale.fHeight!= 0) ? 1.f / img.fScale.fHeight : 0);
            WKCFloatPoint_Set(&img.fPhase, 0, 0);
            WKCFloatSize_Set(&img.fiTransform, 1, 1);
            wkcDrawContextBitBltPeer(dc, &img, &idst, platformOperator(op));
        } else {
            wkcDrawContextFillRectPeer(dc, &idst, 0xffffffff);
        }
    }

end:
    if (ImageObserver* observer = imageObserver())
        observer->didDraw(this);
}

void Image::drawPattern(GraphicsContext* context, const FloatRect& srcRect, const TransformationMatrix& patternTransform,
                        const FloatPoint& phase, ColorSpace styleColorSpace, CompositeOperator op, const FloatRect& destRect)
{
    void* dc = (void *)context->platformContext();
    ImageWKC* bitmap = (ImageWKC *)nativeImageForCurrentFrame();
    int type = WKC_IMAGETYPE_RGBA8888;

    // Avoid NaN
    if (!isfinite(phase.x()) || !isfinite(phase.y()))
       return;
    if (!bitmap) // If it's too early we won't have an image yet.
        return;

    WKCFloatRect isrc;
    WKCFloatRect idst;
    WKCFloatPoint iphase;
    WKCFloatSize transform;
    WKCFloatSize srcscale;

    switch (bitmap->type()) {
    case ImageWKC::EColorARGB8888:
        type = WKC_IMAGETYPE_RGBA8888;
        break;
    case ImageWKC::EColorRGAB5515:
        type = WKC_IMAGETYPE_RGAB5515;
        break;
    case ImageWKC::EColorRGAB5515MASK:
        type = WKC_IMAGETYPE_RGAB5515MASK;
        break;
    case ImageWKC::EColorARGB1232:
        type = WKC_IMAGETYPE_ARGB1232;
        break;
    default:
        goto end;
    }
    if (bitmap->hasAlpha()) {
        type |= WKC_IMAGETYPE_FLAG_HASALPHA;
        if (bitmap->hasTrueAlpha()) {
            type |= WKC_IMAGETYPE_FLAG_HASTRUEALPHA;
        }
    }

    context->save();
    context->clip(IntRect(destRect.x(), destRect.y(), destRect.width(), destRect.height()));

    platformRect(srcRect, isrc);
    platformRect(destRect, idst);
    iphase.fX = phase.x();
    iphase.fY = phase.y();
    transform.fWidth = patternTransform.a();
    transform.fHeight = patternTransform.d();

    srcscale.fWidth = (float)bitmap->scalex();
    srcscale.fHeight = (float)bitmap->scaley();
    isrc.fX = isrc.fX * srcscale.fWidth;
    isrc.fY = isrc.fY * srcscale.fHeight;
    isrc.fWidth = isrc.fWidth * srcscale.fWidth;
    isrc.fHeight = isrc.fHeight * srcscale.fHeight;
    iphase.fX = iphase.fX * srcscale.fWidth;
    iphase.fY = iphase.fY * srcscale.fHeight;

    if (isrc.fWidth && isrc.fHeight && transform.fWidth && transform.fHeight) {
        if (bitmap->bitmap()) {
            WKCPeerImage img = {0};
            img.fType = type;
            img.fBitmap = bitmap->bitmap();
            img.fRowBytes = bitmap->rowbytes();
            img.fMask = bitmap->mask();
            img.fMaskRowBytes = bitmap->maskrowbytes();
            WKCFloatRect_SetXYWH(&img.fSrcRect, isrc.fX, isrc.fY, isrc.fWidth, isrc.fHeight);
            WKCFloatSize_SetSize(&img.fiTransform, &transform);
            WKCFloatPoint_SetPoint(&img.fPhase, &iphase);
            WKCFloatSize_SetSize(&img.fScale, &srcscale);
            img.fiScale.fWidth = ((img.fScale.fWidth!= 0) ? 1.f / img.fScale.fWidth : 0);
            img.fiScale.fHeight = ((img.fScale.fHeight!= 0) ? 1.f / img.fScale.fHeight : 0);
            
            wkcDrawContextBlitPatternPeer(dc, &img, &idst, platformOperator(op));
        } else {
            wkcDrawContextFillRectPeer(dc, &idst, 0xffffffff);
        }
    }

    context->restore();

    startAnimation();

end:
    if (ImageObserver* observer = imageObserver())
        observer->didDraw(this);
}

void BitmapImage::checkForSolidColor()
{
    m_checkedForSolidColor = true;
}

PassRefPtr<Image> Image::loadPlatformResource(const char* name)
{
    // FIXME: We need to have some 'placeholder' graphics for things like missing
    // plugins or broken images.
    Vector<char> arr;
    RefPtr<Image> img = BitmapImage::create();
    RefPtr<SharedBuffer> buffer = SharedBuffer::create(arr.data(), arr.size());
    img->setData(buffer, true);
    return img.release();
}

}
