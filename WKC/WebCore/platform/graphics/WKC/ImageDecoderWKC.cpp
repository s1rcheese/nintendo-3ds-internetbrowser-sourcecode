/*
 * Copyright (C) 2008-2009 Torch Mobile, Inc.
 * Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.
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
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "ImageDecoder.h"
#include "FastMalloc.h"

#include "ImageWKC.h"

#include "NotImplemented.h"

#if PLATFORM(WKC)
#include <wkc/wkcmpeer.h>
#endif

#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
#include <algorithm>
#include <math.h>
#endif

#define WKC_QSIZE (12)
// for signed int
#define WKC_QINTMAX ((1<<(31 - (WKC_QSIZE)))-1)
#define WKC_QINT1 (1 << (WKC_QSIZE))
#define WKC_QINTMASK (~((1 << (WKC_QSIZE))-1))
#define WKC_QMASK (WKC_QINT1-1)
#define WKC_QINT(v) ((v) >> (WKC_QSIZE))
#define WKC_QINT_ISEQUAL(v1,v2) (((v1)>>WKC_QSIZE)==((v2)>>WKC_QSIZE))


namespace WebCore {

static int gDefaultBPP = 2; // RGAB5515
static bool gDither8 = true;
static bool gDither16 = false;

}

// copied from ImageDecoder.cpp

#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
#include <algorithm>
#endif

#include "BMPImageDecoder.h"
#include "GIFImageDecoder.h"
#include "ICOImageDecoder.h"
#include "JPEGImageDecoder.h"
#include "PNGImageDecoder.h"
#include "SharedBuffer.h"

namespace WebCore {

ImageDecoder* ImageDecoder::create(const SharedBuffer& data)
{
    // We need at least 4 bytes to figure out what kind of image we're dealing with.
    int length = data.size();
    if (length < 4)
        return 0;

#if PLATFORM(WKC)
    const char* contents;
    data.getSomeData(contents, 0);

    const unsigned char* uContents = (const unsigned char*)contents;
#else
    const unsigned char* uContents = (const unsigned char*)data.data();
    const char* contents = data.data();
#endif

    // GIFs begin with GIF8(7 or 9).
    if (strncmp(contents, "GIF8", 4) == 0)
        return new GIFImageDecoder();

    // Test for PNG.
    if (uContents[0]==0x89 &&
        uContents[1]==0x50 &&
        uContents[2]==0x4E &&
        uContents[3]==0x47)
        return new PNGImageDecoder();

    // JPEG
    if (uContents[0]==0xFF &&
        uContents[1]==0xD8 &&
        uContents[2]==0xFF)
        return new JPEGImageDecoder();

    // BMP
    if (strncmp(contents, "BM", 2) == 0)
        return new BMPImageDecoder();

    // ICOs always begin with a 2-byte 0 followed by a 2-byte 1.
    // CURs begin with 2-byte 0 followed by 2-byte 2.
    if (!memcmp(contents, "\000\000\001\000", 4) ||
        !memcmp(contents, "\000\000\002\000", 4))
        return new ICOImageDecoder();

    // Give up. We don't know what the heck this is.
    return 0;
}

RGBA32Buffer::RGBA32Buffer()
    : m_hasAlpha(false)
    , m_hasTrueAlpha(false)
    , m_status(FrameEmpty)
    , m_duration(0)
    , m_disposalMethod(DisposeNotSpecified)
    , m_qshift(WKC_QSIZE)
{
    m_bytes = 0;
    m_bpp = gDefaultBPP;
    m_mask = 0;
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
    m_scalex = 1.0;
    m_scaley = 1.0;
#elif ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
    m_scalex = 1.0;
    m_scaley = 1.0;
    m_scaleix = WKC_QINT1;
    m_scaleiy = WKC_QINT1;
#endif
    m_width = 0;
    m_height = 0;
    m_ditherbuf = 0;
    m_ditherbuflen = 0;
    m_ditherline = 0;
    m_usedither8 = gDither8;
    m_usedither16 = gDither16;

    m_size = IntSize();
} 

RGBA32Buffer::RGBA32Buffer(const RGBA32Buffer& other)
    : m_hasAlpha(false)
    , m_hasTrueAlpha(false)
    , m_status(FrameEmpty)
    , m_duration(0)
    , m_disposalMethod(DisposeNotSpecified)
    , m_qshift(WKC_QSIZE)
{
    m_bytes = 0;
    m_bpp = gDefaultBPP;
    m_mask = 0;
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
    m_scalex = 1.0;
    m_scaley = 1.0;
#elif ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
    m_scalex = 1.0;
    m_scaley = 1.0;
    m_scaleix = WKC_QINT1;
    m_scaleiy = WKC_QINT1;
#endif
    m_width = 0;
    m_height = 0;
    m_ditherbuf = 0;
    m_ditherbuflen = 0;
    m_ditherline = 0;
    m_usedither8 = gDither8;
    m_usedither16 = gDither16;

    m_size = IntSize();

    operator=(other);
}

RGBA32Buffer::~RGBA32Buffer()
{
    if (m_bytes) {
        fastFree(m_bytes);
        m_bytes = 0;
    }
    if (m_mask) {
        fastFree(m_mask);
        m_mask = 0;
    }
    if (m_ditherbuf) {
        fastFree(m_ditherbuf);
        m_ditherbuf = 0;
    }
}

void RGBA32Buffer::clear()
{
    if (m_bytes) {
        fastFree(m_bytes);
    }
    m_bytes = 0;
    if (m_mask) {
        fastFree(m_mask);
    }
    m_mask = 0;
    if (m_ditherbuf) {
        fastFree(m_ditherbuf);
    }
    m_ditherbuf = 0;
    m_ditherbuflen = 0;
    m_ditherline = 0;
    m_status = FrameEmpty;
    // NOTE: Do not reset other members here; clearFrameBufferCache()
    // calls this to free the bitmap data, but other functions like
    // initFrameBuffer() and frameComplete() may still need to read
    // other metadata out of this frame later.
}

void RGBA32Buffer::zeroFill()
{
    const int w=width(), h=height();
    if (m_bytes) {
        if (m_bpp==2) {
            for (int y=0; y<h; y++) {
                unsigned short* d = (unsigned short *)m_bytes + y*w;
                for (int x=0; x<w; x++,d++) {
                    *d = 0x0020;
                }
            }
        } else {
            memset(m_bytes, 0, w * h * m_bpp);
        }
    }
    if (m_mask) {
        memset(m_mask, 0, w*h);
    }
    if (m_ditherbuf) {
        memset(m_ditherbuf, 0x80, m_ditherbuflen*4);
        m_ditherline = 0;
    }
    m_hasAlpha = true;
    m_hasTrueAlpha = false;
}

void RGBA32Buffer::copyBitmapData(const RGBA32Buffer& other)
{
    int sizes = 0;

    if (this == &other)
        return;

    if (m_bytes) {
        fastFree(m_bytes);
        m_bytes = 0;
    }
    if (m_mask) {
        fastFree(m_mask);
        m_mask = 0;
    }
    if (m_ditherbuf) {
        fastFree(m_ditherbuf);
        m_ditherbuf = 0;
        m_ditherbuflen = 0;
        m_ditherline = 0;
    }

    sizes = other.m_size.width() * other.m_size.height();
    if (sizes==0 || !other.m_bytes) {
        goto error_end;
    }

    {
        if (!wkcMemoryCheckMemoryAllocatablePeer(sizes * other.m_bpp, wkcMemoryGetAllocationStatePeer())) {
            goto error_end;
        }

        wkcMemorySetAllocatingForImagesPeer(true);
        WTF::TryMallocReturnValue rv = tryFastMalloc(sizes * other.m_bpp);
        wkcMemorySetAllocatingForImagesPeer(false);
        if (rv.getValue(m_bytes)) {
            memcpy(m_bytes, other.m_bytes, sizes * other.m_bpp);
            m_size = other.m_size;
            m_width = other.m_width;
            m_height = other.m_height;
            m_bpp = other.m_bpp;
        } else {
            wkcMemoryNotifyMemoryAllocationErrorPeer(sizes*other.m_bpp, wkcMemoryGetAllocationStatePeer());
            goto error_end;
        }
    }

    if (other.m_mask) {
        if (!wkcMemoryCheckMemoryAllocatablePeer(sizes, wkcMemoryGetAllocationStatePeer())) {
            fastFree(m_bytes);
            goto error_end;
        }
        wkcMemorySetAllocatingForImagesPeer(true);
        WTF::TryMallocReturnValue mrv = tryFastMalloc(sizes * 1);
        wkcMemorySetAllocatingForImagesPeer(false);
        if (mrv.getValue(m_mask)) {
            memcpy(m_mask, other.m_mask, sizes * 1);
        } else {
            wkcMemoryNotifyMemoryAllocationErrorPeer(sizes*1, wkcMemoryGetAllocationStatePeer());
            fastFree(m_bytes);
            goto error_end;
        }
    }
    if ((other.bpp()==1 && m_usedither8) ||
        (other.bpp()==2 && m_usedither16)) {
        m_ditherbuflen = other.width() * 2;
        if (wkcMemoryCheckMemoryAllocatablePeer(m_ditherbuflen*4, wkcMemoryGetAllocationStatePeer())) {
            wkcMemorySetAllocatingForImagesPeer(true);
            WTF::TryMallocReturnValue drv = tryFastMalloc(m_ditherbuflen*4);
            wkcMemorySetAllocatingForImagesPeer(false);
            if (!drv.getValue(m_ditherbuf)) {
                // just disable dithering
                m_ditherbuflen = 0;
            } else {
                memset(m_ditherbuf, 0x80, m_ditherbuflen * 4);
            }
        }
    }

    setHasAlpha(other.m_hasAlpha);
    m_hasTrueAlpha = other.m_hasTrueAlpha;
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
    m_scalex = other.m_scalex;
    m_scaley = other.m_scaley;
#elif ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
    m_scalex = other.m_scalex;
    m_scaley = other.m_scaley;
    m_scaleix = other.m_scaleix;
    m_scaleiy = other.m_scaleiy;
#endif
    return;

error_end:
    m_bytes = 0;
    m_mask = 0;
    m_size = IntSize();
    m_width = m_height = 0;
    setHasAlpha(false);
    m_hasTrueAlpha = false;
    m_bpp = gDefaultBPP;
    m_ditherbuf = 0;
    m_ditherbuflen = 0;
    m_ditherline = 0;
    m_usedither8 = gDither8;
    m_usedither16 = gDither16;
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
    m_scalex = 1.0f;
    m_scaley = 1.0f;
#elif ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
    m_scalex = 1.0f;
    m_scaley = 1.0f;
    m_scaleix = WKC_QINT1;
    m_scaleiy = WKC_QINT1;
#endif
}

#if !ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
bool RGBA32Buffer::setSize(int newWidth, int newHeight, double scale_x, double scale_y, bool reducedepth)
#else
bool RGBA32Buffer::setSize(int newWidth, int newHeight)
#endif
{
    int sizes = 0;

    if (m_bytes) {
        fastFree(m_bytes);
        m_bytes = 0;
    }
    if (m_mask) {
        fastFree(m_mask);
        m_mask = 0;
    }
    if (m_ditherbuf) {
        fastFree(m_ditherbuf);
        m_ditherbuf = 0;
        m_ditherbuflen = 0;
        m_ditherline = 0;
    }

#if ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
    float scale_x=1.f, scale_y=1.f;
    bool reducedepth = false;
    if (!checkDecode(newWidth, newHeight, scale_x, scale_y, reducedepth)) {
        goto error_end;
    }
#endif

    sizes = newWidth * newHeight;
    if (sizes==0) goto error_end;
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING) || ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
    m_scalex = scale_x;
    m_scaley = scale_y;
    if (reducedepth) {
        m_bpp = 1;
    }
#endif
#if ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
    m_scaleix = (float)WKC_QINT1 * scale_x;
    m_scaleiy = (float)WKC_QINT1 * scale_y;
#endif

    {
        if (!wkcMemoryCheckMemoryAllocatablePeer(sizes * m_bpp, wkcMemoryGetAllocationStatePeer())) {
            goto error_end;
        }
        wkcMemorySetAllocatingForImagesPeer(true);
        WTF::TryMallocReturnValue rv = tryFastMalloc(sizes * m_bpp);
        wkcMemorySetAllocatingForImagesPeer(false);
        if (!rv.getValue(m_bytes)) {
            wkcMemoryNotifyMemoryAllocationErrorPeer(sizes*m_bpp, wkcMemoryGetAllocationStatePeer());
            goto error_end;
        }
    }
    if (m_bpp==2) {
        if (!wkcMemoryCheckMemoryAllocatablePeer(sizes, wkcMemoryGetAllocationStatePeer())) {
            fastFree(m_bytes);
            goto error_end;
        }
        wkcMemorySetAllocatingForImagesPeer(true);
        WTF::TryMallocReturnValue mrv = tryFastMalloc(sizes);
        wkcMemorySetAllocatingForImagesPeer(false);
        if (!mrv.getValue(m_mask)) {
            wkcMemoryNotifyMemoryAllocationErrorPeer(sizes, wkcMemoryGetAllocationStatePeer());
            fastFree(m_bytes);
            goto error_end;
        }
    }

    if ((m_bpp==1 && m_usedither8) ||
        (m_bpp==2 && m_usedither16)) {
        m_ditherbuflen = newWidth * 2;
        if (wkcMemoryCheckMemoryAllocatablePeer(m_ditherbuflen*4, wkcMemoryGetAllocationStatePeer())) {
            wkcMemorySetAllocatingForImagesPeer(true);
            WTF::TryMallocReturnValue drv = tryFastMalloc(m_ditherbuflen*4);
            wkcMemorySetAllocatingForImagesPeer(false);
            if (!drv.getValue(m_ditherbuf)) {
                // just disable dithering
                m_ditherbuflen = 0;
            } else {
                memset(m_ditherbuf, 0x80, m_ditherbuflen * 4);
            }
        }
    }

    m_size = IntSize(newWidth, newHeight);
    m_width = newWidth;
    m_height = newHeight;

    // Zero the image.
    zeroFill();

    return true;

error_end:
    m_bytes = 0;
    m_mask = 0;
    m_size = IntSize();
    m_width = m_height = 0;
    m_bpp = gDefaultBPP;
    m_ditherbuf = 0;
    m_ditherbuflen = 0;
    m_ditherline = 0;
    m_usedither8 = gDither8;
    m_usedither16 = gDither16;    
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
    m_scalex = 1.f;
    m_scaley = 1.f;
#elif ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
    m_scalex = 1.f;
    m_scaley = 1.f;
    m_scaleix = WKC_QINT1;
    m_scaleiy = WKC_QINT1;
#endif
    return false;
}

void RGBA32Buffer::setHasAlpha(bool alpha)
{
    m_hasAlpha = alpha;
}

void RGBA32Buffer::setStatus(FrameStatus status)
{
    m_status = status;
    switch (m_status) {
    case FrameComplete:
        if (!m_hasTrueAlpha && m_mask) {
            fastFree(m_mask);
            m_mask = 0;
        }
        break;
    default:
        break;
    }

}

RGBA32Buffer& RGBA32Buffer::operator=(const RGBA32Buffer& other)
{
    if (this == &other)
        return *this;

    copyBitmapData(other);
    setRect(other.rect());
    setStatus(other.status());
    setDuration(other.duration());
    setDisposalMethod(other.disposalMethod());
    return *this;
}

void RGBA32Buffer::copyRowNTimes(int _startX, int _endX, int _startY, int _endY)
{
    if (!m_bytes) return;

    const int startX = _startX * m_scalex;
    const int endX   = _endX   * m_scalex;
    const int startY = _startY * m_scaley;
    const int endY   = _endY   * m_scaley;

    if (startX>=width() || endX>width() || startY>=height() || endY>height()) {
        return;
    }

    ASSERT(startX < width());
    ASSERT(endX <= width());
    ASSERT(startY < height());
    ASSERT(endY <= height());

    if (m_bpp==sizeof(PixelData)) {
        const int rowBytes = (endX - startX) * m_bpp;
        const PixelData* const startAddr = (const PixelData *)m_bytes + startY*width() + startX;
        PixelData* dest = 0;
        for (int destY = startY + 1; destY < endY; ++destY) {
            dest = (PixelData *)m_bytes + destY*width() + startX;
            memcpy(dest, startAddr, rowBytes);
        }
    } else if (m_bpp==2) {
        const int rowBytes = (endX - startX) * m_bpp;
        const unsigned short* const startAddr = (const unsigned short *)m_bytes + startY*width() + startX;
        unsigned short* dest = 0;
        for (int destY = startY + 1; destY < endY; ++destY) {
            dest = (unsigned short *)m_bytes + destY*width() + startX;
            memcpy(dest, startAddr, rowBytes);
        }
    } else if (m_bpp==1) {
        const int rowBytes = (endX - startX) * m_bpp;
        const unsigned char* const startAddr = (const unsigned char *)m_bytes + startY*width() + startX;
        unsigned char* dest = 0;
        for (int destY = startY + 1; destY < endY; ++destY) {
            dest = (unsigned char *)m_bytes + destY*width() + startX;
            memcpy(dest, startAddr, rowBytes);
        }
    }

    if (m_ditherbuf) {
        memset(m_ditherbuf, 0x80, m_ditherbuflen*4);
    }

    if (!m_mask)  return;

    const int mrowBytes = (endX - startX);
    unsigned char* mstartAddr = getMAddr(startX, startY);
    for (int destY = startY + 1; destY < endY; ++destY)
        memcpy(getMAddr(startX, destY), mstartAddr, mrowBytes);
}

void RGBA32Buffer::setInternalColorFormatRGAB5515()
{
    gDefaultBPP = 2;
}

void RGBA32Buffer::setInternalColorFormatRGBA8888()
{
    gDefaultBPP = sizeof(PixelData);
}

void RGBA32Buffer::useDither(int bpp, bool flag)
{
    switch (bpp) {
    case 8:
        gDither8 = flag;
        break;
    case 16:
        gDither16 = flag;
        break;
    default:
        break;
    }
}

#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)

namespace {

enum MatchType {
    Exact,
    UpperBound,
    LowerBound
};

inline void fillScaledValues(Vector<int>& scaledValues, double scaleRate, int length)
{
    double inflateRate = 1. / scaleRate;
    scaledValues.reserveCapacity(static_cast<int>(length * scaleRate + 0.5));
    for (int scaledIndex = 0;;) {
        int index = static_cast<int>(scaledIndex * inflateRate + 0.5);
        if (index < length) {
            scaledValues.append(index);
            ++scaledIndex;
        } else
            break;
    }
}

template <MatchType type> int getScaledValue(const Vector<int>& scaledValues, int valueToMatch, int searchStart)
{
    const int* dataStart = scaledValues.data();
    const int* dataEnd = dataStart + scaledValues.size();
    const int* matched = std::lower_bound(dataStart + searchStart, dataEnd, valueToMatch);
    switch (type) {
    case Exact:
        return matched != dataEnd && *matched == valueToMatch ? matched - dataStart : -1;
    case LowerBound:
        return matched != dataEnd && *matched == valueToMatch ? matched - dataStart : matched - dataStart - 1;
    case UpperBound:
    default:
        return matched != dataEnd ? matched - dataStart : -1;
    }
}

}

void ImageDecoder::prepareScaleDataIfNecessary()
{
    int width = m_size.width();
    int height = m_size.height();
    int numPixels = height * width;
    int maxsize = m_maxNumPixels;

    if (m_reduceDepth) {
        maxsize *= 2; // twice as 16bpp
    }

    m_scaledColumns.clear();
    m_scaledRows.clear();

    if (maxsize <= 0 || numPixels <= maxsize) {
        m_scaled = false;
        return;
    }

    double scale = sqrt((double)maxsize / (double)numPixels);
    double scalex, scaley;
    double scaled_w, scaled_h;

    scaled_w = floor((double)width * scale);
    scaled_h = floor((double)height * scale);
    if (!width || !height || !scaled_w || !scaled_h) {
        return;
    }
    scalex = scaled_w / (double)width;
    scaley = scaled_h / (double)height;

    if (scalex==0 || scaley==0) {
        return;
    }

    m_scaled = true;

    fillScaledValues(m_scaledColumns, scalex, width);
    fillScaledValues(m_scaledRows, scaley, height);
}

int ImageDecoder::upperBoundScaledX(int origX, int searchStart)
{
    return getScaledValue<UpperBound>(m_scaledColumns, origX, searchStart);
}

int ImageDecoder::lowerBoundScaledX(int origX, int searchStart)
{
    return getScaledValue<LowerBound>(m_scaledColumns, origX, searchStart);
}

int ImageDecoder::upperBoundScaledY(int origY, int searchStart)
{
    return getScaledValue<UpperBound>(m_scaledRows, origY, searchStart);
}

int ImageDecoder::lowerBoundScaledY(int origY, int searchStart)
{
    return getScaledValue<LowerBound>(m_scaledRows, origY, searchStart);
}

int ImageDecoder::scaledY(int origY, int searchStart)
{
    return getScaledValue<Exact>(m_scaledRows, origY, searchStart);
}

bool ImageDecoder::setSize(unsigned width, unsigned height)
{
    float scale_x=1.f, scale_y=1.f;
    bool reduce = false;

    if (isOverSize(width, height)) {
        m_failed = true;
        return false;
    }

    switch (wkcDrawContextQueryImageDecodeModePeer(width, height, &scale_x, &scale_y)) {
    case WKC_IMAGEDECODE_MODE_ALLOW:
        scale_x = 1.f;
        scale_y = 1.f;
        break;
    case WKC_IMAGEDECODE_MODE_REDUCE:
        scale_x = 1.f;
        scale_y = 1.f;
        reduce = true;
        break;
    case WKC_IMAGEDECODE_MODE_REDUCEANDSCALE:
        reduce = true;
        // go through
    case WKC_IMAGEDECODE_MODE_SCALE:
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
        break;
#else
        // go through
#endif
    case WKC_IMAGEDECODE_MODE_DENY:
    default:
        m_failed = true;
        return false;
    }
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
    m_maxNumPixels = width*scale_x*height*scale_y;
    m_reduceDepth = reduce;
#endif

    m_size = IntSize(width, height);
    m_sizeAvailable = true;
    return true;
}
#endif // ENABLE(IMAGE_DECODER_DOWN_SAMPLING)

#if ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
bool RGBA32Buffer::checkDecode(int& width, int& height, float& scale_x, float& scale_y, bool& reducedepth)
{
    float sx = 1.f;
    float sy = 1.f;
    const int ow = width;
    const int oh = height;
    if (!ow || !oh) return false;

    scale_x = 1.f;
    scale_y = 1.f;
    reducedepth = false;

    switch (wkcDrawContextQueryImageDecodeModePeer(width, height, &sx, &sy)) {
    case WKC_IMAGEDECODE_MODE_ALLOW:
        sx = 1.f;
        sy = 1.f;
        break;

    case WKC_IMAGEDECODE_MODE_REDUCE:
        sx = 1.f;
        sy = 1.f;
        reducedepth = true;
        break;

    case WKC_IMAGEDECODE_MODE_REDUCEANDSCALE:
        reducedepth = true;
        // go through
    case WKC_IMAGEDECODE_MODE_SCALE:
        if (sx>1.f || sy>1.f || sx<0 || sy<0) {
            return false;
        }
        width *= sx;
        height *= sy;
        break;

    case WKC_IMAGEDECODE_MODE_DENY:
    default:
        return false;
    }

    if (!width || !height) return false;

    scale_x = (float)width / (float)ow;
    scale_y = (float)height / (float)oh;

    return true;
}
#endif

static const unsigned char c4[4] = {
    0, 71, 135, 255,
};

static const unsigned char c8[8] = {
    0, 39, 71, 103, 135, 167, 199, 255,
};

static const unsigned char c32[32] = {
    0, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87, 95, 103, 111, 119, 127, 135, 143, 151,
    159, 167, 175, 183, 191, 199, 207, 215, 223, 231, 239, 247, 255,
};

#define CLIPCHAR(a) (((a)>255) ? 255 : (((a)<0) ? 0 : (a)))

static WKC_FORCEINLINE unsigned int
_ditherparam(int r, int g, int b, unsigned int v, int p)
{
    int vr = ((v>>16)&0xff)-128;
    int vg = ((v>>8)&0xff)-128;
    int vb = (v&0xff)-128;
    int dr = vr + (r * p / 16);
    int dg = vg + (g * p / 16);
    int db = vb + (b * p / 16);
    dr = CLIPCHAR(dr+128);
    dg = CLIPCHAR(dg+128);
    db = CLIPCHAR(db+128);
    return (dr<<16) | (dg<<8) | db;
}

void RGBA32Buffer::setARGB1232(int x, int y, unsigned int r, unsigned int g, unsigned int b, unsigned int a)
{
    unsigned char* dest = 0;
    unsigned int* dcl = 0;
    unsigned int* dnl = 0;
    unsigned int* dp = 0;
    unsigned int v;
    int vr, vg, vb;
    int dr, dg, db;
    int er, eg, eb;
    int w,h;

    w = width();
    h = height();
    dest = (unsigned char *)m_bytes + (y*w) + x;

    if (y&0x01) {
        dcl = m_ditherbuf + w;
        dnl = m_ditherbuf;
    } else {
        dnl = m_ditherbuf + w;
        dcl = m_ditherbuf;
    }

    dp = dcl + x;
    v = *dp;
    vr = r + ((v>>16)&0xff) - 128;
    vg = g + ((v>>8)&0xff) - 128;
    vb = b + (v&0xff) - 128;
    dr = CLIPCHAR(vr) & 0xc0;
    dg = CLIPCHAR(vg) & 0xe0;
    db = CLIPCHAR(vb) & 0xc0;
    if (a) {
        *dest = 0x80 | (dr>>1) | (dg>>3) | (db>>6);
    } else {
        *dest = 0;
    }
    *dp = 0x80808080;

    er = vr - c4[(dr>>6)];
    eg = vg - c8[(dg>>5)];
    eb = vb - c4[(db>>6)];

    // (x+1, y)
    if (x<w-1) {
        dp = dcl + x + 1;
        *dp = _ditherparam(er, eg, eb, *dp, 7);
    }
    if (y<h-1) {
        dp = dnl + x - 1;
        if (x>1) {
            // (x-1,y+1)
            *dp = _ditherparam(er, eg, eb, *dp, 3);
        }
        dp++;
        // (x,y+1)
        *dp = _ditherparam(er, eg, eb, *dp, 5);
        dp++;
        if (x<w-1) {
            // (x+1,y+1)
            *dp = _ditherparam(er, eg, eb, *dp, 1);
        }
    }
}

void RGBA32Buffer::setARGB5515(int x, int y, unsigned int r, unsigned int g, unsigned int b, unsigned int a)
{
    unsigned short* dest = 0;
    unsigned char* mdest = getMAddr(x,y);
    unsigned int* dcl = 0;
    unsigned int* dnl = 0;
    unsigned int* dp = 0;
    unsigned int v;
    int vr, vg, vb;
    int dr, dg, db;
    int er, eg, eb;
    int w,h;

    w = width();
    h = height();
    dest = (unsigned short *)m_bytes + (y*w) + x;

    if (mdest) *mdest = (unsigned char)a&0xff;

    if (!m_ditherbuf) {
        if (a==0) {
            *dest = 0x0020;
        } else {
            *dest = (((r>>3)&0x1f)<<11) | (((g>>3)&0x1f)<<6) | (b>>3)&0x1f;
        }
        return;
    }

    if (y&0x01) {
        dcl = m_ditherbuf + w;
        dnl = m_ditherbuf;
    } else {
        dnl = m_ditherbuf + w;
        dcl = m_ditherbuf;
    }

    dp = dcl + x;
    v = *dp;
    vr = r + ((v>>16)&0xff) - 128;
    vg = g + ((v>>8)&0xff) - 128;
    vb = b + (v&0xff) - 128;
    dr = CLIPCHAR(vr) >> 3;
    dg = CLIPCHAR(vg) >> 3;
    db = CLIPCHAR(vb) >> 3;
    if (a) {
        *dest = (dr<<11) | (dg<<6) | db;;
    } else {
        *dest = 0x0020;
    }
    *dp = 0x80808080;

    er = vr - c32[dr];
    eg = vg - c32[dg];
    eb = vb - c32[db];

    // (x+1, y)
    if (x<w-1) {
        dp = dcl + x + 1;
        *dp = _ditherparam(er, eg, eb, *dp, 7);
    }
    if (y<h-1) {
        dp = dnl + x - 1;
        if (x>1) {
            // (x-1,y+1)
            *dp = _ditherparam(er, eg, eb, *dp, 3);
        }
        dp++;
        // (x,y+1)
        *dp = _ditherparam(er, eg, eb, *dp, 5);
        dp++;
        if (x<w-1) {
            // (x+1,y+1)
            *dp = _ditherparam(er, eg, eb, *dp, 1);
        }
    }
}


}
