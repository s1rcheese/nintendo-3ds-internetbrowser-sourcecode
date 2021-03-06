/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "JSImageData.h"

#include "ImageData.h"
#include "PlatformString.h"

#include <runtime/JSByteArray.h>
#include <wtf/StdLibExtras.h>

using namespace JSC;

namespace WebCore {

#if PLATFORM(WKC)
static RefPtr<Structure>* gCpaStructure = 0;
#endif

JSValue toJS(ExecState* exec, JSDOMGlobalObject* globalObject, ImageData* imageData)
{
    if (!imageData)
        return jsNull();
    
    DOMObject* wrapper = getCachedDOMObjectWrapper(exec, imageData);
    if (wrapper)
        return wrapper;
    
    wrapper = CREATE_DOM_OBJECT_WRAPPER(exec, globalObject, ImageData, imageData);
    Identifier dataName(exec, "data");
#if PLATFORM(WKC)
    if (!gCpaStructure) {
        gCpaStructure = new RefPtr<Structure>(JSByteArray::createStructure(jsNull()));
    }
    RefPtr<Structure>& cpaStructure = *gCpaStructure;
#else
    DEFINE_STATIC_LOCAL(RefPtr<Structure>, cpaStructure, (JSByteArray::createStructure(jsNull())));
#endif
    static const ClassInfo cpaClassInfo = { "CanvasPixelArray", 0, 0, 0 };
    wrapper->putDirect(dataName, new (exec) JSByteArray(exec, cpaStructure, imageData->data()->data(), &cpaClassInfo), DontDelete | ReadOnly);
    exec->heap()->reportExtraMemoryCost(imageData->data()->length());
    
    return wrapper;
}

#if PLATFORM(WKC)
void JSImageDataCustom_deleteSharedInstance()
{
    delete gCpaStructure;
}

void JSImageDataCustom_resetVariables()
{
    gCpaStructure = 0;
}
#endif

}
