/*
 * Copyright (C) 2006, 2007, 2008 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
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
#include "Gradient.h"
#include "FloatQuad.h"

#include "CSSParser.h"
#include "GraphicsContext.h"

namespace WebCore {

static WKCPeerPattern gPattern = {0};

static void
_getGradientColor(void* self, float val, float* red, float* green, float* blue, float* alpha)
{
    Gradient* iself = (Gradient* )self;
    if (iself) {
        iself->getColor(val, red, green, blue, alpha);
    } else {
        *red = *green = *blue = *alpha = 0.f;
    }
}

PlatformGradient
Gradient::platformGradient()
{
    WKCPeerPattern* pattern = &gPattern;

    const float p0x = m_p0.x();
    const float p0y = m_p0.y();
    const float p1x = m_p1.x();
    const float p1y = m_p1.y();

    pattern->fType = WKC_PATTERN_GRADIENT;
    pattern->u.fGradient.fSelf = this;
    pattern->u.fGradient.fRadial = m_radial;
    WKCFloatPoint_Set(&pattern->u.fGradient.fPoint0, p0x, p0y);
    WKCFloatPoint_Set(&pattern->u.fGradient.fPoint1, p1x, p1y);
    const int stops = WKC_MIN(m_stops.size(), WKCPEERGRADIENT_MAXSTOPS);
    pattern->u.fGradient.fStops = stops;
    int i=0;
    for (i=0; i<stops; i++)
        pattern->u.fGradient.fStop[i] = m_stops[i].stop;
    for (;i<WKCPEERGRADIENT_MAXSTOPS; i++)
        pattern->u.fGradient.fStop[i] = 0;
    pattern->u.fGradient.fGetColorProc = _getGradientColor;

    if (m_radial) {
        const float xd = p1x-p0x;
        const float yd = p1y-p0y;
        const float rd = m_r1-m_r0;
        pattern->u.fGradient.fMatrix[0] = (xd*xd+yd*yd-rd*rd);
        if (pattern->u.fGradient.fMatrix[0]==0.f)
            return 0;
        pattern->u.fGradient.fMatrix[1] = xd*p0x+yd*p0y-rd*m_r0;
        pattern->u.fGradient.fMatrix[2] = p0x*p0x+p0y*p0y-m_r0*m_r0;
        pattern->u.fGradient.fMatrix[3] = 1.f / pattern->u.fGradient.fMatrix[0];
    } else {
        if (p0x!=p1x && p0y!=p1y) {
            const float k = 1.f/(p1x-p0x);
            const float div = -k * (p0y-p1y);
            const float im = 1.f/(div*div+1);
            const float nn = div*div*p0x - p0y*div;
            pattern->u.fGradient.fMatrix[0] = k*div*im;
            pattern->u.fGradient.fMatrix[1] = k*im;
            pattern->u.fGradient.fMatrix[2] = k*(nn*im-p0x);
        } else if (p0y!=p1y) {
            pattern->u.fGradient.fMatrix[0] = 1/(p1y-p0y);
            pattern->u.fGradient.fMatrix[1] = 0;
            pattern->u.fGradient.fMatrix[2] = -p0y / (p1y-p0y);
        } else if (p0x!=p1x) {
            pattern->u.fGradient.fMatrix[0] = 0;
            pattern->u.fGradient.fMatrix[1] = 1/(p1x-p0x);
            pattern->u.fGradient.fMatrix[2] = -p0x / (p1x-p0x);
        } else {
            return 0;
        }
    }

    return pattern;
}

void Gradient::platformDestroy()
{
    // nothing to do
}

void Gradient::fill(GraphicsContext* context, const FloatRect& rect)
{
    PlatformGraphicsContext* ctx = context->platformContext();
    if (!ctx)
        return;
    const TransformationMatrix m(context->getCTM());
    const FloatQuad quad = m.mapQuad(rect);
    if (quad.isEmpty())
        return;
    WKCFloatPoint p[6];
    WKCFloatPoint_Set(&p[0], quad.p1().x(), quad.p1().y());
    WKCFloatPoint_Set(&p[1], quad.p2().x(), quad.p2().y());
    WKCFloatPoint_Set(&p[2], quad.p3().x(), quad.p3().y());
    WKCFloatPoint_Set(&p[3], quad.p4().x(), quad.p4().y());
    WKCFloatPoint_SetPoint(&p[4], &p[0]);
    WKCFloatPoint_Set(&p[5], FLT_MIN, FLT_MIN);

    WKCPeerPattern* pt = (WKCPeerPattern *)platformGradient();
    if (!pt)
        return;

    context->save();
    wkcDrawContextSetPatternPeer(ctx, pt);
    wkcDrawContextDrawPolygonPeer(ctx, 6, p);
    wkcDrawContextSetPatternPeer(ctx, 0);
    context->restore();
}

} //namespace
