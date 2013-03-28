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

#include "config.h"
#include "MHTMLWebArchive.h"

#include "Cache.h"
#include "CString.h"
#include "Document.h"
#include "DocumentLoader.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "FrameTree.h"
#include "HTMLFrameOwnerElement.h"
#include "HTMLNames.h"
#include "IconDatabase.h"
#include "Image.h"
#include "MIMETypeRegistry.h"
#include "Logging.h"
#include "markup.h"
#include "Node.h"
#include "Range.h"
#include "SelectionController.h"
#include "SharedBuffer.h"
#include <string.h>

#if ENABLE(WKC_PAGESAVE_MHTML)

namespace WebCore {

bool MHTMLWebArchive::init(SharedBuffer* data)
{
    //ASSERT(data);//data could be null if a null file is loaded
#if PLATFORM(WKC)
    // data size would be 0 if save a content loaded by mhtml
    if (!data || !data->size())
#else
    if (!data)
#endif
        return false;

    if (!parseRawDataRepresentation(data))
        return false;

    ASSERT(!m_mainResourceType.isEmpty());

    /* MHTML files do not have the concept of subframes, so resources are global
     * and are assigned to the main frame. To make the resources accessible
     * also to subframes we add all the subresources to all the subframes. */
    const Vector<RefPtr<Archive> >& subframes(subframeArchives());
    unsigned i;
    for (i = 0; i < subframes.size(); ++i) {
        const Vector<RefPtr<ArchiveResource> >& resources(subresources());
        unsigned j;
        for (j = 0; j < resources.size(); ++j)
            static_cast<MHTMLWebArchive*>(subframes[i].get())->addSubresource(resources[j].get());
    }

#if PLATFORM(WKC)
    // Ugh!: I have only this timing to free it.
    delete m_mainResourceType.impl();
#endif
    return true;
}

void MHTMLWebArchive::addMimeResource(PassRefPtr<SharedBuffer> buffer, ResourceResponse& response)
{
    if (!mainResource() && m_multipartStack.find(m_mainResourceType) != WTF::notFound)
        // If the main type is multipart/something then the real mime type of the main resource is the
        // first atomic type we find as a child of a multipart/something.
        m_mainResourceType = response.mimeType();

    if (response.mimeType() == "application/octet-stream" && m_multipartStack.find("multipart/alternative") != WTF::notFound) {
        /* For some reasons IE sometimes saves sub frames using the "application/octet-stream" mime type instead of
         * text/html. In all the cases we tested these HTML files always start with a UTF-8 BOM followed by a
         * "<!DOCTYPE ...", so we use it as an heuristic to fix the mime type. */
        const char* htmlHead = "\xef\xbb\xbf<!DOCTYPE ";
        CString octetHead(buffer->data(), strlen(htmlHead));
        if (!strcmp(octetHead.data(), htmlHead))
            response.setMimeType("text/html");
    }

    // MHTML files do not store subframe names, so we use the URL as the name.
    RefPtr<ArchiveResource> resource = ArchiveResource::create(buffer, response.url(), response.mimeType(), response.textEncodingName(), response.url(), response);

    if (!mainResource() && response.mimeType() == m_mainResourceType)
        // The first resource with the right MIME type is the main resource.
        setMainResource(resource.release());
    else if (response.mimeType() == "text/html" || response.mimeType() == "application/xhtml+xml") {
        RefPtr<MHTMLWebArchive> archive = create();
        archive->setMainResource(resource.release());
        addSubframeArchive(archive.release());
    } else
        addSubresource(resource.release());
}

void MHTMLWebArchive::serializeResourceIfNew(ArchiveResource* resource, HashSet<String>& uniqueSubresources)
{
#if PLATFORM(WKC)
    if (resource->data()->isEmpty())
        return;
#endif
    /* MHTML files do not have the concept of subframes, so resources are global.
     * To make them accessible also from subframes they have been also assigned to all
     * the subframes (see MHTMLWebArchive::init). When saving a MHTML we want to avoid
     * to save the duplicated resources, so we use uniqueSubresources to keep track of
     * what was already included. */
    if (uniqueSubresources.contains(resource->url().string()))
        return;

#if PLATFORM(WKC)
    if (serializeResource(resource)) {
        uniqueSubresources.add(resource->url().string());
    }
#else
    serializeResource(resource);
    uniqueSubresources.add(resource->url().string());
#endif
}

void MHTMLWebArchive::serializeTree(MHTMLWebArchive* rootArchive, HashSet<String>& uniqueSubresources)
{
    rootArchive->serializeResourceIfNew(mainResource(), uniqueSubresources);
#if PLATFORM(WKC)
    if (rootArchive->m_buffer_len == 0
        || rootArchive->m_status < 0
        || rootArchive->m_status == SERIALIZE_STATE_END)
        return;
#endif

    const Vector<RefPtr<ArchiveResource> >& resources(subresources());
    
#if PLATFORM(WKC)
    for (unsigned i = 0; i < resources.size(); ++i) {
        rootArchive->serializeResourceIfNew(resources[i].get(), uniqueSubresources);
        if (rootArchive->m_buffer_len == 0
            || rootArchive->m_status < 0
            || rootArchive->m_status == SERIALIZE_STATE_END)
            return;
    }

    const Vector<RefPtr<Archive> >& subframes(subframeArchives());
    for (unsigned i = 0; i < subframes.size(); ++i) {
        static_cast<MHTMLWebArchive*>(subframes[i].get())->serializeTree(rootArchive, uniqueSubresources);
        if (rootArchive->m_buffer_len == 0
            || rootArchive->m_status < 0
            || rootArchive->m_status == SERIALIZE_STATE_END)
            return;
    }
#else
    for (unsigned i = 0; i < resources.size(); ++i)
        rootArchive->serializeResourceIfNew(resources[i].get(), uniqueSubresources);

    const Vector<RefPtr<Archive> >& subframes(subframeArchives());
    for (unsigned i = 0; i < subframes.size(); ++i)
        static_cast<MHTMLWebArchive*>(subframes[i].get())->serializeTree(rootArchive, uniqueSubresources);
#endif
}

PassRefPtr<SharedBuffer> MHTMLWebArchive::rawDataRepresentation()
{
    ASSERT(mainResource());
    if (!mainResource())
        return 0;

    serializeStart();
    HashSet<String> uniqueSubresources;
    serializeTree(this, uniqueSubresources);
    RefPtr<SharedBuffer> sharedBuffer = serializeEnd();

    return sharedBuffer.release();
}

#if PLATFORM(WKC)
void
MHTMLWebArchive::freeTmpHeap()
{
    if (m_data) {
        delete [] m_data;
        m_data = 0;
    }
    m_uniqueSubresources.clear();
}

bool
MHTMLWebArchive::contentSerializeStart()
{
    ArchiveResource* mResource = mainResource();

    if (!mResource)
        return false;
    HashSet<String> uniqueSubresources;
    caculateItems(this, uniqueSubresources);

    // do not save a null file
    if (m_items == 0)
        return false;
    // do not save a single image file.
    if (m_items == 1) {
        if (MIMETypeRegistry::isSupportedImageMIMEType(mResource->mimeType()))
            return false;
    }
    m_status = SERIALIZE_STATE_MAKE_MESSAGE_HEADER;
    serializeStart();
    return true;
}

int
MHTMLWebArchive::contentSerializeProgress(void* buffer, unsigned int length)
{
    if (!buffer) {
        HashSet<String> uniqueSubresources;
        return getBufferSize(this, uniqueSubresources);
    }
    m_buffer = buffer;
    m_buffer_len = length;
    serializeTree(this, this->m_uniqueSubresources);

    if (this->m_status > 0) {
        unsigned tmp = m_buffer_off;
        m_buffer_off = 0;
        return tmp;
    } else
        return -1;
}

void
MHTMLWebArchive::contentSerializeEnd()
{
    freeTmpHeap();
}
#endif

PassRefPtr<MHTMLWebArchive> MHTMLWebArchive::create()
{
    return adoptRef(new MHTMLWebArchive);
}

PassRefPtr<MHTMLWebArchive> MHTMLWebArchive::create(SharedBuffer* data)
{
    LOG(Archives, "MHTMLWebArchive - Creating from raw data");
    
    RefPtr<MHTMLWebArchive> archive = create();
    if (!archive->init(data))
        return 0;
        
    return archive.release();
}

PassRefPtr<MHTMLWebArchive> MHTMLWebArchive::create(PassRefPtr<ArchiveResource> mainResource, Vector<PassRefPtr<ArchiveResource> >& subresources, Vector<PassRefPtr<MHTMLWebArchive> >& subframeArchives)
{
    ASSERT(mainResource);
    if (!mainResource)
        return 0;
    
    RefPtr<MHTMLWebArchive> archive = create();
    archive->setMainResource(mainResource);
    
    for (unsigned i = 0; i < subresources.size(); ++i)
        archive->addSubresource(subresources[i]);
    
    for (unsigned i = 0; i < subframeArchives.size(); ++i)
        archive->addSubframeArchive(subframeArchives[i]);  
        
    return archive.release();
}

MHTMLWebArchive::MHTMLWebArchive()
#if PLATFORM(GTK)
    : m_outMultipart(0)
#endif
#if PLATFORM(WKC)
    : m_status(0)
    , m_data(0)
    , m_data_len(0)
    , m_data_off(0)
    , m_buffer(0)
    , m_buffer_len(0)
    , m_buffer_off(0)
    , m_items(0)
    , m_cur_item(1)
#endif
{
}

PassRefPtr<MHTMLWebArchive> MHTMLWebArchive::create(Node* node)
{
    ASSERT(node);
    if (!node)
        return create();
        
    Document* document = node->document();
    Frame* frame = document ? document->frame() : 0;
    if (!frame)
        return create();
        
    Vector<Node*> nodeList;
    String markupString = createMarkup(node, IncludeNode, &nodeList);
    Node::NodeType nodeType = node->nodeType();
    if (nodeType != Node::DOCUMENT_NODE && nodeType != Node::DOCUMENT_TYPE_NODE)
        markupString = frame->documentTypeString() + markupString;

    return create(markupString, frame, nodeList);
}

PassRefPtr<MHTMLWebArchive> MHTMLWebArchive::create(Frame* frame)
{
    ASSERT(frame);
    
    DocumentLoader* documentLoader = frame->loader()->documentLoader();

    if (!documentLoader)
        return 0;
        
    // Without this hack a MHTML stream is saved inside another MHTML stream.
    // FIXME Figure out how can LegacyWebArchive work without a similar hack.
    if (documentLoader->mainResource()->mimeType() == WKC_PAGESAVE_MIMETYPE)
        return create(documentLoader->mainResource()->data());

    Vector<PassRefPtr<MHTMLWebArchive> > subframeArchives;

    unsigned children = frame->tree()->childCount();
    for (unsigned i = 0; i < children; ++i) {
        RefPtr<MHTMLWebArchive> childFrameArchive = create(frame->tree()->child(i));
        if (childFrameArchive)
            subframeArchives.append(childFrameArchive.release());
    }

    Vector<PassRefPtr<ArchiveResource> > subresources;
    documentLoader->getSubresources(subresources);

    return create(documentLoader->mainResource(), subresources, subframeArchives);
}

PassRefPtr<MHTMLWebArchive> MHTMLWebArchive::create(Range* range)
{
    if (!range)
        return 0;
    
    Node* startContainer = range->startContainer();
    if (!startContainer)
        return 0;
        
    Document* document = startContainer->document();
    if (!document)
        return 0;
        
    Frame* frame = document->frame();
    if (!frame)
        return 0;
    
    Vector<Node*> nodeList;
    
    // FIXME: This is always "for interchange". Is that right? See the previous method.
    String markupString = frame->documentTypeString() + createMarkup(range, &nodeList, AnnotateForInterchange);

    return create(markupString, frame, nodeList);
}

PassRefPtr<MHTMLWebArchive> MHTMLWebArchive::create(const String& markupString, Frame* frame, const Vector<Node*>& nodes)
{
    ASSERT(frame);
    
    const ResourceResponse& response = frame->loader()->documentLoader()->response();
    KURL responseURL = response.url();
    
    // it's possible to have a response without a URL here
    // <rdar://problem/5454935>
    if (responseURL.isNull())
        responseURL = KURL(ParsedURLString, "");
        
    PassRefPtr<ArchiveResource> mainResource = ArchiveResource::create(utf8Buffer(markupString), responseURL, response.mimeType(), "UTF-8", frame->tree()->name());

    Vector<PassRefPtr<MHTMLWebArchive> > subframeArchives;
    Vector<PassRefPtr<ArchiveResource> > subresources;
    HashSet<KURL> uniqueSubresources;

    size_t nodesSize = nodes.size();    
    for (size_t i = 0; i < nodesSize; ++i) {
        Node* node = nodes[i];
        Frame* childFrame;
        if ((node->hasTagName(HTMLNames::frameTag) || node->hasTagName(HTMLNames::iframeTag) || node->hasTagName(HTMLNames::objectTag)) &&
             (childFrame = static_cast<HTMLFrameOwnerElement*>(node)->contentFrame())) {
            RefPtr<MHTMLWebArchive> subframeArchive = create(childFrame->document());
            
            if (subframeArchive)
                subframeArchives.append(subframeArchive);
            else
                LOG_ERROR("Unabled to archive subframe %s", childFrame->tree()->name().string().utf8().data());
        } else {
            ListHashSet<KURL> subresourceURLs;
            node->getSubresourceURLs(subresourceURLs);
            
            DocumentLoader* documentLoader = frame->loader()->documentLoader();
            ListHashSet<KURL>::iterator iterEnd = subresourceURLs.end();
            for (ListHashSet<KURL>::iterator iter = subresourceURLs.begin(); iter != iterEnd; ++iter) {
                const KURL& subresourceURL = *iter;
                if (uniqueSubresources.contains(subresourceURL))
                    continue;

                uniqueSubresources.add(subresourceURL);

                RefPtr<ArchiveResource> resource = documentLoader->subresource(subresourceURL);
                if (resource) {
                    subresources.append(resource.release());
                    continue;
                }

                CachedResource *cachedResource = cache()->resourceForURL(subresourceURL);
                if (cachedResource) {
                    resource = ArchiveResource::create(cachedResource->data(), subresourceURL, cachedResource->response());
                    if (resource) {
                        subresources.append(resource.release());
                        continue;
                    }
                }

                // FIXME: should do something better than spew to console here
                LOG_ERROR("Failed to archive subresource for %s", subresourceURL.string().utf8().data());
            }
        }
    }

    // Add favicon if one exists for this page, if we are archiving the entire page.
    if (nodesSize && nodes[0]->isDocumentNode() && iconDatabase() && iconDatabase()->isEnabled()) {
        const String& iconURL = iconDatabase()->iconURLForPageURL(responseURL);
        if (!iconURL.isEmpty() && iconDatabase()->iconDataKnownForIconURL(iconURL)) {
            if (Image* iconImage = iconDatabase()->iconForPageURL(responseURL, IntSize(16, 16))) {
                if (RefPtr<ArchiveResource> resource = ArchiveResource::create(iconImage->data(), KURL(ParsedURLString, iconURL), "image/x-icon", "", ""))
                    subresources.append(resource.release());
            }
        }
    }
    
    return create(mainResource, subresources, subframeArchives);
}

PassRefPtr<MHTMLWebArchive> MHTMLWebArchive::createFromSelection(Frame* frame)
{
    if (!frame)
        return 0;
    
    RefPtr<Range> selectionRange = frame->selection()->toNormalizedRange();
    Vector<Node*> nodeList;
    String markupString = frame->documentTypeString() + createMarkup(selectionRange.get(), &nodeList, AnnotateForInterchange);
    
    RefPtr<MHTMLWebArchive> archive = create(markupString, frame, nodeList);
    
    if (!frame->document() || !frame->document()->isFrameSet())
        return archive.release();
        
    // Wrap the frameset document in an iframe so it can be pasted into
    // another document (which will have a body or frameset of its own). 
    String iframeMarkup = String::format("<iframe frameborder=\"no\" marginwidth=\"0\" marginheight=\"0\" width=\"98%%\" height=\"98%%\" src=\"%s\"></iframe>", 
                                         frame->loader()->documentLoader()->response().url().string().utf8().data());
    RefPtr<ArchiveResource> iframeResource = ArchiveResource::create(utf8Buffer(iframeMarkup), blankURL(), "text/html", "UTF-8", String());

    Vector<PassRefPtr<ArchiveResource> > subresources;

    Vector<PassRefPtr<MHTMLWebArchive> > subframeArchives;
    subframeArchives.append(archive);
    
    archive = create(iframeResource.release(), subresources, subframeArchives);
    
    return archive.release();
}

}
#endif // ENABLE(WKC_PAGESAVE_MHTML)

