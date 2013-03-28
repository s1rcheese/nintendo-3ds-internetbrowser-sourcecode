/*
    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001 Dirk Mueller (mueller@kde.org)
    Copyright (C) 2002 Waldo Bastian (bastian@kde.org)
    Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
    Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
    Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.
*/

#include "config.h"
#include "CachedScript.h"

#include "CachedResourceClient.h"
#include "CachedResourceClientWalker.h"
#include "TextResourceDecoder.h"
#include <wtf/Vector.h>

#if PLATFORM(WKC)
#include "TextCodecWKC.h"
#include "TextEncodingDetector.h"
#include "TextEncodingRegistry.h"
#include <wkc/wkcmpeer.h>
#endif

namespace WebCore {

CachedScript::CachedScript(const String& url, const String& charset)
    : CachedResource(url, Script)
    , m_decoder(TextResourceDecoder::create("application/javascript", charset))
    , m_decodedDataDeletionTimer(this, &CachedScript::decodedDataDeletionTimerFired)
{
    // It's javascript we want.
    // But some websites think their scripts are <some wrong mimetype here>
    // and refuse to serve them if we only accept application/x-javascript.
    setAccept("*/*");
}

CachedScript::~CachedScript()
{
}

void CachedScript::didAddClient(CachedResourceClient* c)
{
    if (!m_loading)
        c->notifyFinished(this);
}

void CachedScript::allClientsRemoved()
{
    m_decodedDataDeletionTimer.startOneShot(0);
}

void CachedScript::setEncoding(const String& chs)
{
    m_decoder->setEncoding(chs, TextResourceDecoder::EncodingFromHTTPHeader);
}

String CachedScript::encoding() const
{
    return m_decoder->encoding().name();
}

const String& CachedScript::script()
{
    ASSERT(!isPurgeable());

    if (!m_script && m_data) {
#if PLATFORM(WKC)
        unsigned bytesToBeAllocated;
        const char* data;
        size_t encodedLength;
        int decodedLength;
        PassOwnPtr<TextCodec> codecPassOwnPtr;
        if (m_data->needsReallocationToMergeBuffers(&bytesToBeAllocated)
            && !wkcMemoryCheckMemoryAllocatablePeer(bytesToBeAllocated, WKC_MEMORYALLOC_TYPE_JAVASCRIPT)) {
            // Failed to allocate continuous memory area for encoded script.
            // The area is used for a text decoder to detect encoding of the script.
            goto setEmptyScript;
        }
        data = m_data->data();
        encodedLength = encodedSize();
        if (!data || !encodedLength) {
            goto setEmptyScript;
        }

        {
            TextEncoding detectedEncoding;
            if (detectTextEncoding(data, encodedLength, m_decoder->encoding().name(), &detectedEncoding)) {
                m_decoder->setEncoding(detectedEncoding, TextResourceDecoder::AutoDetectedEncoding);
            }
        }

        decodedLength = 0;
        codecPassOwnPtr = newTextCodec(m_decoder->encoding());
        if (codecPassOwnPtr->isTextCodecWKC()) {
            TextCodecWKC* codec = static_cast<TextCodecWKC*>(codecPassOwnPtr.get());
            decodedLength = codec->getDecodedTextLength(data, encodedLength);
        } else {
            // encoding is Latin1 or UserDefined
            decodedLength = encodedLength;
            //How about the case where encoding is UTF-16???
            //decodedLength = encodedLength / 2;
        }
        if (decodedLength <= 0) {
            goto setEmptyScript;
        }

        bytesToBeAllocated = sizeof(StringImpl) + decodedLength * sizeof(UChar);
        if (!wkcMemoryCheckMemoryAllocatablePeer(bytesToBeAllocated, WKC_MEMORYALLOC_TYPE_JAVASCRIPT)) {
            // Failed to allocate continuous memory area for decoded script.
            goto setEmptyScript;
        } else {
#endif
        m_script = m_decoder->decode(m_data->data(), encodedSize());
        m_script += m_decoder->flush();
        setDecodedSize(m_script.length() * sizeof(UChar));
#if PLATFORM(WKC)
        }
        goto junction;

setEmptyScript:
        m_script = String();
        setDecodedSize(0);
        m_errorOccurred = true;

junction:
        ;
#endif
    }

    m_decodedDataDeletionTimer.startOneShot(0);
    return m_script;
}

void CachedScript::data(PassRefPtr<SharedBuffer> data, bool allDataReceived)
{
    if (!allDataReceived)
        return;

    m_data = data;
    setEncodedSize(m_data.get() ? m_data->size() : 0);
    m_loading = false;
    checkNotify();
}

void CachedScript::checkNotify()
{
    if (m_loading)
        return;

    CachedResourceClientWalker w(m_clients);
    while (CachedResourceClient* c = w.next())
        c->notifyFinished(this);
}

void CachedScript::error()
{
    m_loading = false;
    m_errorOccurred = true;
    checkNotify();
}

void CachedScript::destroyDecodedData()
{
    m_script = String();
    setDecodedSize(0);
    if (isSafeToMakePurgeable())
        makePurgeable(true);
}

void CachedScript::decodedDataDeletionTimerFired(Timer<CachedScript>*)
{
    destroyDecodedData();
}

} // namespace WebCore
