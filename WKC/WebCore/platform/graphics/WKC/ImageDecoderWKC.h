/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2008-2009 Torch Mobile, Inc.
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
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

#ifndef ImageDecoderWKC_h
#define ImageDecoderWKC_h

#include "IntRect.h"
#include "ImageSource.h"
#include "PlatformString.h"
#include "SharedBuffer.h"
#include <wtf/Assertions.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

namespace WebCore {

    // The RGBA32Buffer object represents the decoded image data in RGBA32 format.  This buffer is what all
    // decoders write a single frame into.  Frames are then instantiated for drawing by being handed this buffer.
    class RGBA32Buffer {
    public:
        enum FrameStatus { FrameEmpty, FramePartial, FrameComplete };
        enum FrameDisposalMethod {
            // If you change the numeric values of these, make sure you audit all
            // users, as some users may cast raw values to/from these constants.
            DisposeNotSpecified,       // Leave frame in framebuffer
            DisposeKeep,               // Leave frame in framebuffer
            DisposeOverwriteBgcolor,   // Clear frame to transparent
            DisposeOverwritePrevious,  // Clear frame to previous framebuffer contents
        };
        typedef unsigned PixelData;

        RGBA32Buffer();
        ~RGBA32Buffer();

        // For backends which refcount their data, this constructor doesn't need
        // to create a new copy of the image data, only increase the ref count.
        //
        // This exists because ImageDecoder keeps a Vector<RGBA32Buffer>, and
        // Vector requires this constructor.
        RGBA32Buffer(const RGBA32Buffer& other);

        // Deletes the pixel data entirely; used by ImageDecoder to save memory
        // when we no longer need to display a frame and only need its metadata.
        void clear();

        // Zeroes the pixel data in the buffer, setting it to fully-transparent.
        void zeroFill();

        // Creates a new copy of the image data in |other|, so the two images
        // can be modified independently.
        void copyBitmapData(const RGBA32Buffer& other);

        // Copies the pixel data at [(startX, startY), (endX, startY)) to the
        // same X-coordinates on each subsequent row up to but not including
        // endY.
        void copyRowNTimes(int startX, int endX, int startY, int endY);

        // Allocates space for the pixel data.  Must be called before any pixels
        // are written. Will return true on success, false if the memory
        // allocation fails.  Calling this multiple times is undefined and may
        // leak memory.
#if !ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
        bool setSize(int newWidth, int newHeight, double scalex=1.0, double scaley=1.0, bool reducedepth=false);
#else
        bool setSize(int newWidth, int newHeight);
#endif

        // To be used by ImageSource::createFrameAtIndex().  Returns a pointer
        // to the underlying native image data.  This pointer will be owned by
        // the BitmapImage and freed in FrameData::clear().
        NativeImagePtr asNewNativeImage();

        bool hasAlpha() const { return m_hasAlpha; };
        bool hasTrueAlpha() const { return m_hasTrueAlpha; } ;
        const IntRect& rect() const { return m_rect; }
        FrameStatus status() const { return m_status; }
        unsigned duration() const { return m_duration; }
        FrameDisposalMethod disposalMethod() const { return m_disposalMethod; }

        void setHasAlpha(bool alpha);
        void setRect(const IntRect& r) { m_rect = r; }
        void setStatus(FrameStatus status);
        void setDuration(unsigned duration) { m_duration = duration; }
        void setDisposalMethod(FrameDisposalMethod method) { m_disposalMethod = method; }

        inline void setRGBA(int x, int y, unsigned r, unsigned g, unsigned b, unsigned a)
        {
            if (!m_bytes) return;

#if ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
            x = (x * m_scaleix)>>m_qshift;
            y = (y * m_scaleiy)>>m_qshift;
            if (x<0 || x>=m_width || y<0 || y>=m_height) {
                return;
            }
#endif

            if (a>0 && a<255) {
                m_hasTrueAlpha = true;
            }

            if (m_bpp==sizeof(PixelData)) {
                PixelData* dest = (PixelData *)m_bytes + (y * m_width) + x;
                if (a == 0)
                    *dest = 0;
                else {
                    if (a < 255) {
                        float alphaPercent = a / 255.0f;
                        r = static_cast<unsigned>(r * alphaPercent);
                        g = static_cast<unsigned>(g * alphaPercent);
                        b = static_cast<unsigned>(b * alphaPercent);
                    }
                    *dest = (a << 24 | r << 16 | g << 8 | b);
                }
            } else if (m_bpp==2) {
                if (m_usedither16) {
                    setARGB5515(x, y, r, g, b, a);
                } else {
                    if (m_mask) {
                        unsigned char* const mdest = (unsigned char *)m_mask + (y * m_width) + x;
                        *mdest = (unsigned char)a&0xff;
                    }
                    unsigned short* const dest = (unsigned short *)m_bytes + (y*m_width) + x;
                    if (a==0) {
                        *dest = 0x0020;
                    } else {
                        *dest = (((r>>3)&0x1f)<<11) | (((g>>3)&0x1f)<<6) | (b>>3)&0x1f;
                    }
                }
            } else if (m_bpp==1) {
                if (m_usedither8) {
                    setARGB1232(x, y, r, g, b, a);
                } else {
                    unsigned char* const dest = (unsigned char *)m_bytes + (y*m_width) + x;
                    if (a) {
                        *dest = 0x80 | ((r&0xc0)>>1) | ((g&0xe0)>>3) | ((b&0xc0)>>6);
                    } else {
                        *dest = 0;
                    }
                }
            }
        }

#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
        const IntRect& scaledRect() const { return m_scaledRect; }
        void setScaledRect(const IntRect& r) { m_scaledRect = r; }
        inline double scalex() { return m_scalex; };
        inline double scaley() { return m_scaley; };
#elif ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
        inline float scalex() { return m_scalex; };
        inline float scaley() { return m_scaley; };
#endif

        static void setInternalColorFormatRGAB5515();
        static void setInternalColorFormatRGBA8888();
        static void useDither(int bpp, bool);

        inline void* bytes() const { return m_bytes; };
        inline int bpp() const { return m_bpp; };
        inline void* mask() const { return m_mask; };

        inline int width() const { return m_size.width(); };
        inline int height() const { return m_size.height(); };

    private:
        RGBA32Buffer& operator=(const RGBA32Buffer& other);

        inline unsigned char* getMAddr(int x, int y) {
            if (!m_mask) return 0;
            return (unsigned char *)m_mask + (y * m_width) + x;
        }
        void setARGB1232(int x, int y, unsigned r, unsigned g, unsigned b, unsigned a);
        void setARGB5515(int x, int y, unsigned r, unsigned g, unsigned b, unsigned a);

#if ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
        bool checkDecode(int& width, int& height, float& scale_x, float& scale_y, bool& reducedepth);
#endif

        void* m_bytes;
        IntSize m_size;       // The size of the buffer.  This should be the
                              // same as ImageDecoder::m_size.
        bool m_hasAlpha;      // Whether or not any of the pixels in the buffer have transparency.
        bool m_hasTrueAlpha;  // Whether or not any of the pixels in the buffer have true (not monochrome) transparency.
        IntRect m_rect;       // The rect of the original specified frame within the overall buffer.
                              // This will always just be the entire buffer except for GIF frames
                              // whose original rect was smaller than the overall image size.
        FrameStatus m_status; // Whether or not this frame is completely finished decoding.
        unsigned m_duration;  // The animation delay.
        FrameDisposalMethod m_disposalMethod;
                              // What to do with this frame's data when initializing the next frame.

        int m_bpp;
        void* m_mask;

        bool m_usedither8;
        bool m_usedither16;
        unsigned int* m_ditherbuf;
        int m_ditherbuflen;
        int m_ditherline;

#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
        IntRect m_scaledRect;
        double m_scalex;
        double m_scaley;
#elif ENABLE(WKC_IMAGE_DECODER_DOWN_SAMPLING)
        IntRect m_scaledRect;
        float m_scalex;
        float m_scaley;
        int m_scaleix;
        int m_scaleiy;
        const int m_qshift;
#endif
        int m_width;
        int m_height;
    };

} // namespace WebCore

#endif
