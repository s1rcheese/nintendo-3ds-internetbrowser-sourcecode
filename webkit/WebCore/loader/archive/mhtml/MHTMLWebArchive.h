/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Collabora Ltd.
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MHTMLWebArchive_h
#define MHTMLWebArchive_h

#include "Archive.h"

#include <wtf/HashSet.h>
#include <wtf/PassRefPtr.h>
#include <wtf/Vector.h>

#if PLATFORM(GTK)
typedef struct _GMimeObject GMimeObject;
typedef struct _GMimePart GMimePart;
typedef struct _GMimeMultipart GMimeMultipart;

namespace WebCore {
void handleMimeObject(GMimeObject*, void*);
}
#endif

namespace WebCore {

class Frame;
class Node;
class Range;

class MHTMLWebArchive : public Archive {
public:    
    static PassRefPtr<MHTMLWebArchive> create();
    static PassRefPtr<MHTMLWebArchive> create(SharedBuffer*);
    static PassRefPtr<MHTMLWebArchive> create(PassRefPtr<ArchiveResource> mainResource, Vector<PassRefPtr<ArchiveResource> >& subresources, Vector<PassRefPtr<MHTMLWebArchive> >& subframeArchives);
    static PassRefPtr<MHTMLWebArchive> create(Node*);
    static PassRefPtr<MHTMLWebArchive> create(Frame*);
    static PassRefPtr<MHTMLWebArchive> createFromSelection(Frame* frame);
    static PassRefPtr<MHTMLWebArchive> create(Range*);
    static PassRefPtr<MHTMLWebArchive> create(const String& markupString, Frame*, const Vector<Node*>& nodes);

    PassRefPtr<SharedBuffer> rawDataRepresentation();

#if PLATFORM(WKC)
    #define WKC_PAGESAVE_MIMETYPE  "message/rfc2557"
    bool contentSerializeStart();
    int contentSerializeProgress(void* buffer, unsigned int length);
    void contentSerializeEnd();
    void freeTmpHeap();
#endif
private:
    MHTMLWebArchive();
    bool init(SharedBuffer*);
    bool parseRawDataRepresentation(SharedBuffer*);
    void addMimeResource(PassRefPtr<SharedBuffer>, ResourceResponse&);

    void serializeStart();
    void serializeTree(MHTMLWebArchive*, HashSet<String>& uniqueSubresources);
    void serializeResourceIfNew(ArchiveResource*, HashSet<String>& uniqueSubresources);
#if PLATFORM(WKC)
    bool serializeResource(ArchiveResource*);
#else
    void serializeResource(ArchiveResource*);
#endif
    PassRefPtr<SharedBuffer> serializeEnd();

#if PLATFORM(GTK)
    friend void handleMimeObject(GMimeObject*, void*);
    void addPart(GMimePart*);
#endif
#if PLATFORM(WKC)
    void caculateItems(MHTMLWebArchive*, HashSet<String>& uniqueSubresources);
    int getBufferSize(MHTMLWebArchive*, HashSet<String>& uniqueSubresources);
    int getItemsSize(MHTMLWebArchive*, HashSet<String>& uniqueSubresources);
#endif

    String m_mainResourceType;
    Vector<String> m_multipartStack;
#if PLATFORM(GTK)
    GMimeMultipart* m_outMultipart;
#endif
#if PLATFORM(WKC)
    enum {
        SERIALIZE_STATE_ERROR = -1,
	    SERIALIZE_STATE_MAKE_MESSAGE_HEADER = 0,
	    SERIALIZE_STATE_WRITE_MESSAGE_HEADER,
	    SERIALIZE_STATE_MAKE_BOUNDARY,
	    SERIALIZE_STATE_WRITE_BOUNDARY,
	    SERIALIZE_STATE_MAKE_PART_HEADER,
	    SERIALIZE_STATE_WRITE_PART_HEADER,
        SERIALIZE_STATE_MAKE_CONTENT,
	    SERIALIZE_STATE_WRITE_CONTENT,
	    SERIALIZE_STATE_END
    };

    int m_status;//data serialize, parse

    char* m_data;
    unsigned int m_data_len;
    unsigned int m_data_off;
    void* m_buffer;
    unsigned int m_buffer_len;
    unsigned int m_buffer_off;

    unsigned int m_items;
    unsigned int m_cur_item;

    HashSet<String> m_uniqueSubresources;
#endif
};

}

#endif // MHTMLWebArchive
