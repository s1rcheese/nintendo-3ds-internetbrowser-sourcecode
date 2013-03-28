/*
 * Copyright (C) 2007 Kevin Ollivier <kevino@theolliviers.com>
 * Copyright (c) 2010, 2011 ACCESS CO., LTD. All rights reserved.
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
#include "GraphicsContext.h"

#include "TransformationMatrix.h"
#include "FloatRect.h"
#include "Font.h"
#include "Gradient.h"
#include "ImageObserver.h"
#include "ImageWKC.h"
#include "IntRect.h"
#include "FloatQuad.h"
#include "NotImplemented.h"
#include "Pattern.h"
#include "Pen.h"
#include "PlatformPathWKC.h"
#include <wtf/MathExtras.h>

#include <wkc/wkcpeer.h>

#include "NotImplemented.h"

namespace WebCore {

class GraphicsContextPlatformPrivateData {
public:
    GraphicsContextPlatformPrivateData()
        : m_transform()
        , m_itransform()
        , m_opacity(1.0) {}

    float m_opacity;
    Vector<Path> m_pathes;
    Path m_clip;
    TransformationMatrix m_transform;
    TransformationMatrix m_itransform;
};

class GraphicsContextPlatformPrivate : public GraphicsContextPlatformPrivateData {
public:
    GraphicsContextPlatformPrivate()
      : m_drawcontext(NULL) {}
    ~GraphicsContextPlatformPrivate()
    {
        while (!m_backupData.isEmpty())
            restore(false);
    };

    void save()
    {
        if (m_drawcontext) {
            wkcDrawContextSaveStatePeer(m_drawcontext);
        }
        m_backupData.append(*static_cast<GraphicsContextPlatformPrivateData*>(this));
    }

    void restore(bool restoreclip)
    {
        if (m_backupData.isEmpty())
            return;

        if (m_drawcontext) {
            wkcDrawContextRestoreStatePeer(m_drawcontext);
        }

        GraphicsContextPlatformPrivateData::operator=(m_backupData.last());
        m_backupData.removeLast();

        if (restoreclip && m_drawcontext) {
            PlatformPathWKC* pp = (PlatformPathWKC *)m_clip.platformPath();
            pp->clipPath(m_drawcontext, &m_transform);
        }
    }

    bool mapRect(const FloatRect& rect, WKCFloatPoint* p)
    {
        FloatQuad q = m_transform.mapQuad(rect);
        FloatRect br = q.boundingBox();
        int d = 0;

        if (q.isEmpty()) {
            return false;
        }
        if (br.width() < 1.f || br.height() < 1.f) {
            return false;
        }

        p[0].fX = q.p1().x();
        p[0].fY = q.p1().y();
        p[1].fX = q.p2().x();
        p[1].fY = q.p2().y();
        p[2].fX = q.p3().x();
        p[2].fY = q.p3().y();
        p[3].fX = q.p4().x();
        p[3].fY = q.p4().y();

        d = (p[1].fX-p[0].fX)*(p[2].fY-p[0].fY) - (p[2].fX-p[0].fX) * (p[1].fY-p[0].fY);
        if (d==0) {
            return false;
        }
        d = (p[3].fX-p[0].fX)*(p[2].fY-p[0].fY) - (p[2].fX-p[0].fX) * (p[3].fY-p[0].fY);
        if (d==0) {
            return false;
        }
        return true;
    }

public:
    void* m_drawcontext;
    Vector<GraphicsContextPlatformPrivateData> m_backupData;
};

GraphicsContext::GraphicsContext(PlatformGraphicsContext* context)
    : m_common(createGraphicsContextPrivate())
    , m_data(new GraphicsContextPlatformPrivate)
{ 
    setPaintingDisabled(!context);
    m_data->m_drawcontext = (void *)context;
    if (context) {
        // Make sure the context starts in sync with our state.
        setPlatformFillColor(fillColor(), DeviceColorSpace);
        setPlatformStrokeColor(strokeColor(), DeviceColorSpace);
    }
}

extern "C" void wkcDrawContextFlushPeer(void* dc);

GraphicsContext::~GraphicsContext()
{
    if (m_data && m_data->m_drawcontext) {
        wkcDrawContextFlushPeer(m_data->m_drawcontext);
    }

    destroyGraphicsContextPrivate(m_common);
    delete m_data;
}

PlatformGraphicsContext* GraphicsContext::platformContext() const
{
    return (PlatformGraphicsContext *)m_data->m_drawcontext;
}

void GraphicsContext::savePlatformState()
{
    m_data->save();
}

void GraphicsContext::restorePlatformState()
{
    m_data->restore(true);
}

static inline unsigned int platformColor(const Color& color)
{
    return (color.alpha()<<24) | (color.red()<<16) | (color.green()<<8) | (color.blue());
}

static inline int platformStyle(StrokeStyle style)
{
    switch (style) {
    case NoStroke:
        return WKC_STROKESTYLE_NO;
    case SolidStroke:
        return WKC_STROKESTYLE_SOLID;
    case DottedStroke:
        return WKC_STROKESTYLE_DOTTED;
    case DashedStroke:
        return WKC_STROKESTYLE_DASHED;
    }
    return 0;
}

static inline void platformPoint(const FloatPoint& in, WKCFloatPoint& out)
{
    out.fX = in.x();
    out.fY = in.y();
}

static inline void platformPoint(const IntPoint& in, WKCFloatPoint& out)
{
    out.fX = in.x();
    out.fY = in.y();
}

static inline void platformRect(const FloatRect& in, WKCFloatRect& out)
{
    out.fX = in.x();
    out.fY = in.y();
    out.fWidth = in.width();
    out.fHeight = in.height();
}

static inline void platformRect(const IntRect& in, WKCFloatRect& out)
{
    out.fX = in.x();
    out.fY = in.y();
    out.fWidth = in.width();
    out.fHeight = in.height();
}

static inline void platformSize(const IntSize& in, WKCFloatSize& out)
{
    out.fWidth = in.width();
    out.fHeight = in.height();
}

// Draws a filled rectangle with a stroked border.
void GraphicsContext::drawRect(const IntRect& rect)
{
    if (paintingDisabled())
        return;

    WKCFloatRect r;
    platformRect(rect, r);
    wkcDrawContextDrawRectPeer(m_data->m_drawcontext, &r);
}

// This is only used to draw borders.
void GraphicsContext::drawLine(const IntPoint& point1, const IntPoint& point2)
{
    if (paintingDisabled())
        return;

    WKCFloatPoint p1, p2;
    platformPoint(point1, p1);
    platformPoint(point2, p2);
    wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(strokeColor()), strokeThickness(), platformStyle(strokeStyle()));
    wkcDrawContextDrawLinePeer(m_data->m_drawcontext, &p1, &p2);
}

// This method is only used to draw the little circles used in lists.
void GraphicsContext::drawEllipse(const IntRect& rect)
{
    if (paintingDisabled())
        return;

    WKCFloatRect r;
    platformRect(rect, r);
    wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(strokeColor()), strokeThickness(), platformStyle(strokeStyle()));
    wkcDrawContextDrawEllipsePeer(m_data->m_drawcontext, &r);
}

void GraphicsContext::strokeArc(const IntRect& rect, int startAngle, int angleSpan)
{
    if (paintingDisabled())
        return;

    WKCFloatRect r;
    platformRect(rect, r);
    wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(strokeColor()), strokeThickness(), platformStyle(strokeStyle()));
    wkcDrawContextStrokeArcPeer(m_data->m_drawcontext, &r, startAngle, angleSpan);
}

void GraphicsContext::drawConvexPolygon(size_t npoints, const FloatPoint* points, bool shouldAntialias)
{
    WKCFloatPoint* p = 0;
    WKCFloatPoint quad[4];
    bool allocated = false;

    if (paintingDisabled())
        return;

    if (npoints <= 1)
        return;

    if (npoints<=4) {
        p = quad;
        allocated = false;
    } else {
        p = new WKCFloatPoint[npoints];
        if (!p) return;
        allocated = true;
    }
    for (size_t i=0; i<npoints; i++) {
        platformPoint(points[i], p[i]);
    }

    wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(strokeColor()), strokeThickness(), platformStyle(strokeStyle()));
    wkcDrawContextDrawConvexPolygonPeer(m_data->m_drawcontext, npoints, p, shouldAntialias ? 1:0);

    if (allocated) {
        delete [] p;
    }
}

void GraphicsContext::fillRect(const FloatRect& rect, const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    int type = 0;
    type = wkcDrawContextGetOffscreenTypePeer(m_data->m_drawcontext);

    m_data->save();
    if (type==WKC_OFFSCREEN_TYPE_IMAGEBUF) {
        WKCFloatPoint p[6];
        if (m_data->mapRect(rect, p)) {
            setPlatformFillColor(color, colorSpace);
            WKCFloatPoint_SetPoint(&p[4], &p[0]);
            WKCFloatPoint_Set(&p[5], FLT_MIN, FLT_MIN);
            wkcDrawContextDrawPolygonPeer(m_data->m_drawcontext, 6, p);
        }
    } else {
        WKCFloatRect r;
        platformRect(rect, r);
        // FIXME: Because drawRect already fills a rect and draws its border, we make sure border is not painted here.
        setPlatformStrokeStyle(NoStroke);
        wkcDrawContextFillRectPeer(m_data->m_drawcontext, &r, platformColor(color));
    }
    m_data->restore(false);
}

void GraphicsContext::fillRoundedRect(const IntRect& rect, const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight, const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    WKCFloatRect r;
    WKCFloatSize tl, tr, bl, br;
    platformRect(rect, r);
    platformSize(topLeft, tl);
    platformSize(topRight, tr);
    platformSize(bottomLeft, bl);
    platformSize(bottomRight, br);
    wkcDrawContextFillRoundRectPeer(m_data->m_drawcontext, &r, &tl, &tr, &bl, &br, platformColor(color));
}

void GraphicsContext::drawFocusRing(const Vector<Path>& paths, int width, int offset, const Color& color)
{
    // FIXME: implement
}

void GraphicsContext::drawFocusRing(const Color& color)
{
    if (paintingDisabled())
        return;

    int i=0, count=0;
    const Vector<IntRect>& rects = focusRingRects();
    WKCFloatRect r;

    count = rects.size();
    if (!count) return;

    m_data->save();
    wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(color), 1, WKC_STROKESTYLE_SOLID);
    wkcDrawContextSetFillColorPeer(m_data->m_drawcontext,0);
    for (i=0; i<count; i++) {
        platformRect(rects[i], r);
        wkcDrawContextDrawRectPeer(m_data->m_drawcontext, &r);
    }
    m_data->restore(false);
}

void GraphicsContext::clip(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    WKCFloatRect r;
    platformRect(rect, r);
    wkcDrawContextClipPeer(m_data->m_drawcontext, &r);
}

void GraphicsContext::clipOut(const Path&)
{
    if (paintingDisabled())
        return;
    notImplemented();

}

void GraphicsContext::clipOut(const IntRect&)
{
    notImplemented();
}

void GraphicsContext::clipOutEllipseInRect(const IntRect&)
{
    notImplemented();
}

void GraphicsContext::drawLineForText(const IntPoint& origin, int width, bool printing)
{
    if (paintingDisabled())
        return;

    WKCFloatPoint p;
    platformPoint(origin, p);
    wkcDrawContextDrawLineForTextPeer(m_data->m_drawcontext, &p, width, printing ? 1:0);
}


void GraphicsContext::drawLineForMisspellingOrBadGrammar(const IntPoint& origin, int width, bool grammar)
{
    if (paintingDisabled())
        return;

    WKCFloatPoint p;
    platformPoint(origin, p);
    wkcDrawContextDrawLineForMisspellingOrBadGrammarPeer(m_data->m_drawcontext, &p, width, grammar ? 1:0);
}

void GraphicsContext::clip(const Path& path) 
{ 
    m_data->m_clip = path;
    PlatformPathWKC* pp = (PlatformPathWKC *)path.platformPath();
    pp->clipPath(m_data->m_drawcontext, &m_data->m_transform);
}

void GraphicsContext::canvasClip(const Path& path)
{
    clip(path);
}

void GraphicsContext::clipToImageBuffer(const FloatRect&, const ImageBuffer*)
{
    notImplemented();
}

TransformationMatrix GraphicsContext::getCTM() const
{ 
    return m_data->m_transform;
}

void GraphicsContext::translate(float tx, float ty) 
{
    m_data->m_transform.translate(tx, ty);
    m_data->m_itransform = m_data->m_transform.inverse();
    wkcDrawContextSetMatrixPeer(m_data->m_drawcontext, m_data->m_transform.a(), m_data->m_transform.b(), m_data->m_transform.c(), m_data->m_transform.d(), m_data->m_transform.e(), m_data->m_transform.f());
    wkcDrawContextSetInvertMatrixPeer(m_data->m_drawcontext, m_data->m_itransform.a(), m_data->m_itransform.b(), m_data->m_itransform.c(), m_data->m_itransform.d(), m_data->m_itransform.e(), m_data->m_itransform.f());
}

void GraphicsContext::rotate(float angle) 
{ 
    m_data->m_transform.rotate(rad2deg(angle));
    m_data->m_itransform = m_data->m_transform.inverse();
    wkcDrawContextSetMatrixPeer(m_data->m_drawcontext, m_data->m_transform.a(), m_data->m_transform.b(), m_data->m_transform.c(), m_data->m_transform.d(), m_data->m_transform.e(), m_data->m_transform.f());
    wkcDrawContextSetInvertMatrixPeer(m_data->m_drawcontext, m_data->m_itransform.a(), m_data->m_itransform.b(), m_data->m_itransform.c(), m_data->m_itransform.d(), m_data->m_itransform.e(), m_data->m_itransform.f());
}

void GraphicsContext::scale(const FloatSize& scale) 
{
    m_data->m_transform.scaleNonUniform(scale.width(), scale.height());
    m_data->m_itransform = m_data->m_transform.inverse();
    wkcDrawContextSetMatrixPeer(m_data->m_drawcontext, m_data->m_transform.a(), m_data->m_transform.b(), m_data->m_transform.c(), m_data->m_transform.d(), m_data->m_transform.e(), m_data->m_transform.f());
    wkcDrawContextSetInvertMatrixPeer(m_data->m_drawcontext, m_data->m_itransform.a(), m_data->m_itransform.b(), m_data->m_itransform.c(), m_data->m_itransform.d(), m_data->m_itransform.e(), m_data->m_itransform.f());
}

void GraphicsContext::concatCTM(const TransformationMatrix& transform)
{
    m_data->m_transform = transform * m_data->m_transform;
    m_data->m_itransform = m_data->m_transform.inverse();
    wkcDrawContextSetMatrixPeer(m_data->m_drawcontext, m_data->m_transform.a(), m_data->m_transform.b(), m_data->m_transform.c(), m_data->m_transform.d(), m_data->m_transform.e(), m_data->m_transform.f());
    wkcDrawContextSetInvertMatrixPeer(m_data->m_drawcontext, m_data->m_itransform.a(), m_data->m_itransform.b(), m_data->m_itransform.c(), m_data->m_itransform.d(), m_data->m_itransform.e(), m_data->m_itransform.f());
}

FloatRect GraphicsContext::roundToDevicePixels(const FloatRect& frect)
{
    FloatRect result;
    double x = frect.x();
    double y = frect.y();
    x = round(x);
    y = round(y);
    result.setX(static_cast<float>(x));
    result.setY(static_cast<float>(y));
    x = frect.width();
    y = frect.height();
    x = round(x);
    y = round(y);
    result.setWidth(static_cast<float>(x));
    result.setHeight(static_cast<float>(y));
    return result;
}

void GraphicsContext::setURLForRect(const KURL&, const IntRect&)
{
    notImplemented();
}

void GraphicsContext::setCompositeOperation(CompositeOperator op)
{
    if (paintingDisabled())
        return;

    int ope;
    switch (op) {
    case CompositeClear:
        ope = WKC_COMPOSITEOPERATION_CLEAR;
        break;
    case CompositeCopy:
        ope = WKC_COMPOSITEOPERATION_COPY;
        break;
    case CompositeSourceOver:
        ope = WKC_COMPOSITEOPERATION_SOURCEOVER;
        break;
    case CompositeSourceIn:
        ope = WKC_COMPOSITEOPERATION_SOURCEIN;
        break;
    case CompositeSourceOut:
        ope = WKC_COMPOSITEOPERATION_SOURCEOUT;
        break;
    case CompositeSourceAtop:
        ope = WKC_COMPOSITEOPERATION_SOURCEATOP;
        break;
    case CompositeDestinationOver:
        ope = WKC_COMPOSITEOPERATION_DESTINATIONOVER;
        break;
    case CompositeDestinationIn:
        ope = WKC_COMPOSITEOPERATION_DESTINATIONIN;
        break;
    case CompositeDestinationOut:
        ope = WKC_COMPOSITEOPERATION_DESTINATIONOUT;
        break;
    case CompositeDestinationAtop:
        ope = WKC_COMPOSITEOPERATION_DESTINATIONATOP;
        break;
    case CompositeXOR:
        ope = WKC_COMPOSITEOPERATION_XOR;
        break;
    case CompositePlusDarker:
        ope = WKC_COMPOSITEOPERATION_PLUSDARKER;
        break;
    case CompositeHighlight:
        ope = WKC_COMPOSITEOPERATION_HIGHLIGHT;
        break;
    case CompositePlusLighter:
        ope = WKC_COMPOSITEOPERATION_PLUSLIGHTER;
        break;
    default:
        return;
    }
    wkcDrawContextSetCompositeOperationPeer(m_data->m_drawcontext, ope);
}

void GraphicsContext::beginPath()
{
    m_data->m_pathes.clear();
}

void GraphicsContext::addPath(const Path& path)
{
    m_data->m_pathes.append(path);
}

void GraphicsContext::setPlatformStrokeColor(const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    wkcDrawContextSetStrokeColorPeer(m_data->m_drawcontext, platformColor(color));
}

void GraphicsContext::setPlatformStrokeStyle(const StrokeStyle& strokeStyle)
{
    if (paintingDisabled())
        return;
    wkcDrawContextSetStrokeStylePeer(m_data->m_drawcontext, platformStyle(strokeStyle));
}

void GraphicsContext::setPlatformStrokeThickness(float thickness)
{
    if (paintingDisabled())
        return;

    wkcDrawContextSetStrokeThicknessPeer(m_data->m_drawcontext, thickness);
}

void GraphicsContext::setPlatformFillColor(const Color& color, ColorSpace colorSpace)
{
    if (paintingDisabled())
        return;

    wkcDrawContextSetFillColorPeer(m_data->m_drawcontext, platformColor(color));
}

void GraphicsContext::setPlatformShouldAntialias(bool enable)
{
    if (paintingDisabled())
        return;
    wkcDrawContextSetShouldAntialiasPeer(m_data->m_drawcontext, enable ? 1 : 0);
}

void GraphicsContext::setImageInterpolationQuality(InterpolationQuality)
{
}

InterpolationQuality GraphicsContext::imageInterpolationQuality() const
{
    return InterpolationDefault;
}

static WKCPeerPattern gPattern = {0};

PlatformPatternPtr
Pattern::createPlatformPattern(const WebCore::TransformationMatrix& userSpaceTransformation) const
{
    WKCPeerPattern* pattern = &gPattern;

    Image* tile = tileImage();
    if (!tile) return 0;
    ImageWKC* img = (ImageWKC *)tile->nativeImageForCurrentFrame();
    if (!img) return 0;

    pattern->fType = WKC_PATTERN_IMAGE;
    switch (img->type()) {
    case ImageWKC::EColorARGB8888:
        pattern->u.fImage.fType = WKC_IMAGETYPE_RGBA8888;
        break;
    case ImageWKC::EColorRGAB5515:
        pattern->u.fImage.fType = WKC_IMAGETYPE_RGAB5515;
        break;
    case ImageWKC::EColorRGAB5515MASK:
        pattern->u.fImage.fType = WKC_IMAGETYPE_RGAB5515MASK;
        break;
    case ImageWKC::EColorARGB1232:
        pattern->u.fImage.fType = WKC_IMAGETYPE_ARGB1232;
        break;
    default:
        goto error_end;
    }
    if (img->hasAlpha()) {
        pattern->u.fImage.fType |= WKC_IMAGETYPE_FLAG_HASALPHA;
        if (img->hasTrueAlpha()) {
            pattern->u.fImage.fType |= WKC_IMAGETYPE_FLAG_HASTRUEALPHA;
        }
    }
    
    pattern->u.fImage.fBitmap = img->bitmap();
    pattern->u.fImage.fRowBytes = img->rowbytes();
    pattern->u.fImage.fMask = img->mask();
    pattern->u.fImage.fMaskRowBytes = img->maskrowbytes();
    WKCFloatRect_SetXYWH(&pattern->u.fImage.fSrcRect, 0, 0, img->size().width(), img->size().height());
    WKCFloatSize_Set(&pattern->u.fImage.fScale,  (float)img->scalex(), (float)img->scaley());
    if (pattern->u.fImage.fScale.fWidth==0 || pattern->u.fImage.fScale.fHeight==0) {
        goto error_end;
    }
    WKCFloatSize_Set(&pattern->u.fImage.fiScale, 1.f/pattern->u.fImage.fScale.fWidth, 1.f/pattern->u.fImage.fScale.fHeight);
    WKCFloatPoint_Set(&pattern->u.fImage.fPhase, 0.f, 0.f);
    WKCFloatSize_Set(&pattern->u.fImage.fiTransform, 1.f, 1.f);

    pattern->u.fImage.fRepeatX = m_repeatX;
    pattern->u.fImage.fRepeatY = m_repeatY;

    return (PlatformPatternPtr)pattern;

error_end:
    if (ImageObserver* observer = tile->imageObserver())
        observer->didDraw(tile);
    return 0;
}

static void
tidyPattern(Pattern* pattern)
{
    if (!pattern)
        return;
    Image* tile = pattern->tileImage();
    if (!tile)
        return;
    if (ImageObserver* observer = tile->imageObserver())
        observer->didDraw(tile);
}

void GraphicsContext::fillPath()
{
    if (paintingDisabled())
        return;

    WKCPeerPattern* pt = 0;
    Pattern* pattern = fillPattern();
    Gradient* gradient = fillGradient();
    TransformationMatrix affine;
    if (pattern) {
        pt = (WKCPeerPattern *)pattern->createPlatformPattern(affine);
        if (!pt) return;
    } else if (gradient) {
        pt = (WKCPeerPattern *)gradient->platformGradient();
        if (!pt) return;
    } else {
        Color c = fillColor();
        if (!c.alpha())
            return;
    }
    wkcDrawContextSetPatternPeer(m_data->m_drawcontext, pt);

    m_data->save();

    for (Vector<Path>::const_iterator i = m_data->m_pathes.begin(); i != m_data->m_pathes.end(); ++i) {
        PlatformPathWKC* pp = (PlatformPathWKC *)i->platformPath();
        pp->fillPath(m_data->m_drawcontext, &m_data->m_transform);
    }

    m_data->restore(false);

    wkcDrawContextSetPatternPeer(m_data->m_drawcontext, 0);
    if (pattern)
        tidyPattern(pattern);
}

void GraphicsContext::strokePath()
{
    if (paintingDisabled())
        return;

    if (!m_data->m_opacity)
        return;

    WKCPeerPattern* pt = 0;
    Pattern* pattern = strokePattern();
    Gradient* gradient = strokeGradient();
    TransformationMatrix affine;
    if (pattern) {
        pt = (WKCPeerPattern *)pattern->createPlatformPattern(affine);
        if (!pt) return;
    } else if (gradient) {
        pt = (WKCPeerPattern *)gradient->platformGradient();
        if (!pt) return;
    } else {
        Color c = strokeColor();
        if (!c.alpha())
            return;
    }
    wkcDrawContextSetPatternPeer(m_data->m_drawcontext, pt);

    m_data->save();
    wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(strokeColor()), strokeThickness(), platformStyle(strokeStyle()));
    for (Vector<Path>::const_iterator i = m_data->m_pathes.begin(); i != m_data->m_pathes.end(); ++i) {
        PlatformPathWKC* pp = (PlatformPathWKC *)i->platformPath();
        pp->strokePath(m_data->m_drawcontext, &m_data->m_transform);
    }

    m_data->restore(false);

    wkcDrawContextSetPatternPeer(m_data->m_drawcontext, 0);
    if (pattern)
        tidyPattern(pattern);
}

void GraphicsContext::drawPath()
{
    fillPath();
    strokePath();
}

void GraphicsContext::fillRect(const FloatRect& rect)
{
    if (paintingDisabled())
        return;

    const int type = wkcDrawContextGetOffscreenTypePeer(m_data->m_drawcontext);

    m_data->save();

    if (type==WKC_OFFSCREEN_TYPE_IMAGEBUF) {
        WKCPeerPattern* pt = 0;
        Pattern* pattern = fillPattern();
        Gradient* gradient = fillGradient();
        TransformationMatrix affine;
        if (pattern) {
            pt = (WKCPeerPattern *)pattern->createPlatformPattern(affine);
            if (!pt) return;
        } else if (gradient) {
            pt = (WKCPeerPattern *)gradient->platformGradient();
            if (!pt) return;
        } else {
            Color c = fillColor();
            if (!c.alpha() || !m_data->m_opacity)
                return;
        }
        wkcDrawContextSetPatternPeer(m_data->m_drawcontext, pt);

        WKCFloatPoint p[6];
        if (m_data->mapRect(rect, p)) {
            WKCFloatPoint_SetPoint(&p[4], &p[0]);
            WKCFloatPoint_Set(&p[5], FLT_MIN, FLT_MIN);
            wkcDrawContextDrawPolygonPeer(m_data->m_drawcontext, 6, p);
        }
        wkcDrawContextSetPatternPeer(m_data->m_drawcontext, 0);
        if (pattern)
            tidyPattern(pattern);
    } else {
        WKCFloatRect r;
        m_data->save();
        platformRect(rect, r);
        // FIXME: Because drawRect already fills a rect and draws its border, we make sure border is not painted here.
        setPlatformStrokeStyle(NoStroke);
        wkcDrawContextFillRectPeer(m_data->m_drawcontext, &r, platformColor(fillColor()));
    }

    m_data->restore(false);
}

void GraphicsContext::setPlatformShadow(IntSize const& size, int blur, Color const& color, ColorSpace) 
{
    if (paintingDisabled())
        return;

    const WKCFloatRect ws = { 0, 0, size.width(), size.height() };
    wkcDrawContextSetShadowPeer(m_data->m_drawcontext, &ws, blur, platformColor(color));
}

void GraphicsContext::clearPlatformShadow() 
{
    if (paintingDisabled())
        return;
    wkcDrawContextClearShadowPeer(m_data->m_drawcontext);
}

void GraphicsContext::beginTransparencyLayer(float opacity) 
{ 
    if (paintingDisabled())
        return;
    m_data->save();
    m_data->m_opacity *= opacity;
    unsigned char alpha = (unsigned char)(m_data->m_opacity * 255);
    wkcDrawContextBeginTransparencyLayerPeer(m_data->m_drawcontext, alpha);
}

void GraphicsContext::endTransparencyLayer() 
{ 
    if (paintingDisabled())
        return;
    m_data->restore(false);
    wkcDrawContextEndTransparencyLayerPeer(m_data->m_drawcontext);
}

void GraphicsContext::clearRect(const FloatRect& rect) 
{
    if (paintingDisabled())
        return;

    int type = 0;
    type = wkcDrawContextGetOffscreenTypePeer(m_data->m_drawcontext);

    if (type==WKC_OFFSCREEN_TYPE_IMAGEBUF) {
        WKCFloatPoint p[5];
        if (m_data->mapRect(rect, p)) {
            WKCFloatPoint_SetPoint(&p[4], &p[0]);
            wkcDrawContextClearPolygonPeer(m_data->m_drawcontext, 5, p);
        }
    } else {
        WKCFloatRect r;
        platformRect(rect, r);
        wkcDrawContextClearRectPeer(m_data->m_drawcontext, &r);
    }
}

void GraphicsContext::strokeRect(const FloatRect& rect, float)
{ 
    if (paintingDisabled())
        return;

    int type = 0;
    type = wkcDrawContextGetOffscreenTypePeer(m_data->m_drawcontext);

    m_data->save();

    wkcDrawContextSetPenStylePeer(m_data->m_drawcontext, platformColor(strokeColor()), strokeThickness(), platformStyle(strokeStyle()));
    if (type==WKC_OFFSCREEN_TYPE_IMAGEBUF) {
        WKCPeerPattern* pt = 0;
        Pattern* pattern = strokePattern();
        Gradient* gradient = strokeGradient();
        TransformationMatrix affine;
        if (pattern && pattern->tileImage()) {
            pt = (WKCPeerPattern *)pattern->createPlatformPattern(affine);
            if (!pt) return;
        } else if (gradient) {
            pt = (WKCPeerPattern *)gradient->platformGradient();
            if (!pt) return;
        } else {
            Color c = strokeColor();
            if (!c.alpha() || !m_data->m_opacity)
                return;
        }
        wkcDrawContextSetPatternPeer(m_data->m_drawcontext, pt);

        WKCFloatPoint p[5];

        if (m_data->mapRect(rect, p)) {
            p[4].fX = p[0].fX;
            p[4].fY = p[0].fY;
            wkcDrawContextDrawPolylinePeer(m_data->m_drawcontext, 5, p);
        }

        wkcDrawContextSetPatternPeer(m_data->m_drawcontext, 0);
        if (pattern)
            tidyPattern(pattern);
    } else {
        WKCFloatRect r;
        platformRect(rect, r);
        wkcDrawContextStrokeRectPeer(m_data->m_drawcontext, &r);
    }

    m_data->restore(false);
}

void GraphicsContext::setLineCap(LineCap cap) 
{
    int type = 0;
    switch (cap) {
    case ButtCap:
        type = WKC_LINECAP_BUTTCAP;
        break;
    case RoundCap:
        type = WKC_LINECAP_ROUNDCAP;
        break;
    case SquareCap:
        type = WKC_LINECAP_SQUARECAP;
        break;
    default:
        return;
    }
    wkcDrawContextSetLineCapPeer(m_data->m_drawcontext, type);
}

void GraphicsContext::setLineJoin(LineJoin join)
{
    int type = 0;
    switch (join) {
    case MiterJoin:
        type = WKC_LINEJOIN_MITERJOIN;
        break;
    case RoundJoin:
        type = WKC_LINEJOIN_ROUNDJOIN;
        break;
    case BevelJoin:
        type = WKC_LINEJOIN_BEVELJOIN;
        break;
    default:
        return;
    }
    wkcDrawContextSetLineJoinPeer(m_data->m_drawcontext, type);
}

void GraphicsContext::setMiterLimit(float lim)
{
    wkcDrawContextSetMiterLimitPeer(m_data->m_drawcontext, lim);
}

void GraphicsContext::setAlpha(float alpha)
{
    wkcDrawContextSetAlphaPeer(m_data->m_drawcontext, (int)(255.0 * alpha));
}

void GraphicsContext::addInnerRoundedRectClip(const IntRect& rect, int thickness)
{
    if (paintingDisabled())
        return;

    WKCFloatRect r;
    platformRect(rect, r);
    wkcDrawContextAddInnerRoundedRectClipPeer(m_data->m_drawcontext, &r, thickness);
}

}
