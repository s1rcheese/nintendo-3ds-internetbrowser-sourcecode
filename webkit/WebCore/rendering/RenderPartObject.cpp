/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
 *           (C) 2000 Stefan Schimanski (1Stein@gmx.de)
 * Copyright (C) 2004, 2005, 2006, 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "RenderPartObject.h"

#include "Frame.h"
#include "FrameLoaderClient.h"
#include "HTMLEmbedElement.h"
#include "HTMLIFrameElement.h"
#include "HTMLNames.h"
#include "HTMLObjectElement.h"
#include "HTMLParamElement.h"
#include "MIMETypeRegistry.h"
#include "Page.h"
#include "RenderView.h"
#include "RenderWidgetProtector.h"
#include "Text.h"
#if ENABLE(WKC_FRAME_FLATTENING)
#include "Settings.h"
#endif

#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
#include "HTMLVideoElement.h"
#endif

namespace WebCore {

using namespace HTMLNames;

RenderPartObject::RenderPartObject(Element* element)
    : RenderPart(element)
{
    // init RenderObject attributes
    setInline(true);
    m_hasFallbackContent = false;
    
    if (element->hasTagName(embedTag) || element->hasTagName(objectTag))
        view()->frameView()->setIsVisuallyNonEmpty();
}

RenderPartObject::~RenderPartObject()
{
    if (frameView())
        frameView()->removeWidgetToUpdate(this);
}

static bool isURLAllowed(Document* doc, const String& url)
{
    if (doc->frame()->page()->frameCount() >= 200)
        return false;

    // We allow one level of self-reference because some sites depend on that.
    // But we don't allow more than one.
    KURL completeURL = doc->completeURL(url);
    bool foundSelfReference = false;
    for (Frame* frame = doc->frame(); frame; frame = frame->tree()->parent()) {
        if (equalIgnoringFragmentIdentifier(frame->loader()->url(), completeURL)) {
            if (foundSelfReference)
                return false;
            foundSelfReference = true;
        }
    }
    return true;
}

typedef HashMap<String, String, CaseFoldingHash> ClassIdToTypeMap;

static ClassIdToTypeMap* createClassIdToTypeMap()
{
    ClassIdToTypeMap* map = new ClassIdToTypeMap;
    map->add("clsid:D27CDB6E-AE6D-11CF-96B8-444553540000", "application/x-shockwave-flash");
    map->add("clsid:CFCDAA03-8BE4-11CF-B84B-0020AFBBCCFA", "audio/x-pn-realaudio-plugin");
    map->add("clsid:02BF25D5-8C17-4B23-BC80-D3488ABDDC6B", "video/quicktime");
    map->add("clsid:166B1BCA-3F9C-11CF-8075-444553540000", "application/x-director");
    map->add("clsid:6BF52A52-394A-11D3-B153-00C04F79FAA6", "application/x-mplayer2");
    map->add("clsid:22D6F312-B0F6-11D0-94AB-0080C74C7E95", "application/x-mplayer2");
    return map;
}

#if PLATFORM(WKC)
static ClassIdToTypeMap* gMap = 0;

static String serviceTypeForClassId(const String& classId)
{
    // Return early if classId is empty (since we won't do anything below).
    // Furthermore, if classId is null, calling get() below will crash.
    if (classId.isEmpty())
        return String();

    if (!gMap) {
        gMap = createClassIdToTypeMap();
    }
    return gMap->get(classId);
}

void RenderPartObject::deleteSharedInstance()
{
    delete gMap;
}

void RenderPartObject::resetVariables()
{
    gMap = 0;
}

#else
static String serviceTypeForClassId(const String& classId)
{
    // Return early if classId is empty (since we won't do anything below).
    // Furthermore, if classId is null, calling get() below will crash.
    if (classId.isEmpty())
        return String();

    static ClassIdToTypeMap* map = createClassIdToTypeMap();
    return map->get(classId);
}
#endif

static void mapDataParamToSrc(Vector<String>* paramNames, Vector<String>* paramValues)
{
    // Some plugins don't understand the "data" attribute of the OBJECT tag (i.e. Real and WMP
    // require "src" attribute).
    int srcIndex = -1, dataIndex = -1;
    for (unsigned int i = 0; i < paramNames->size(); ++i) {
        if (equalIgnoringCase((*paramNames)[i], "src"))
            srcIndex = i;
        else if (equalIgnoringCase((*paramNames)[i], "data"))
            dataIndex = i;
    }

    if (srcIndex == -1 && dataIndex != -1) {
        paramNames->append("src");
        paramValues->append((*paramValues)[dataIndex]);
    }
}

void RenderPartObject::updateWidget(bool onlyCreateNonNetscapePlugins)
{
    String url;
    String serviceType;
    Vector<String> paramNames;
    Vector<String> paramValues;
    Frame* frame = frameView()->frame();

    // The calls to FrameLoader::requestObject within this function can result in a plug-in being initialized.
    // This can run cause arbitrary JavaScript to run and may result in this RenderObject being detached from
    // the render tree and destroyed, causing a crash like <rdar://problem/6954546>.  By extending our lifetime
    // artifically to ensure that we remain alive for the duration of plug-in initialization.
    RenderWidgetProtector protector(this);

    if (node()->hasTagName(objectTag)) {
        HTMLObjectElement* o = static_cast<HTMLObjectElement*>(node());

        o->setNeedWidgetUpdate(false);
        if (!o->isFinishedParsingChildren())
          return;

        // Check for a child EMBED tag.
        HTMLEmbedElement* embed = 0;
        for (Node* child = o->firstChild(); child; ) {
            if (child->hasTagName(embedTag)) {
                embed = static_cast<HTMLEmbedElement*>(child);
                break;
            } else if (child->hasTagName(objectTag))
                child = child->nextSibling();         // Don't descend into nested OBJECT tags
            else
                child = child->traverseNextNode(o);   // Otherwise descend (EMBEDs may be inside COMMENT tags)
        }

        // Use the attributes from the EMBED tag instead of the OBJECT tag including WIDTH and HEIGHT.
        HTMLElement *embedOrObject;
        if (embed) {
            embedOrObject = (HTMLElement *)embed;
            url = embed->url();
            serviceType = embed->serviceType();
        } else
            embedOrObject = (HTMLElement *)o;

        // If there was no URL or type defined in EMBED, try the OBJECT tag.
        if (url.isEmpty())
            url = o->url();
        if (serviceType.isEmpty())
            serviceType = o->serviceType();

        HashSet<StringImpl*, CaseFoldingHash> uniqueParamNames;

        // Scan the PARAM children.
        // Get the URL and type from the params if we don't already have them.
        // Get the attributes from the params if there is no EMBED tag.
        Node *child = o->firstChild();
        while (child && (url.isEmpty() || serviceType.isEmpty() || !embed)) {
            if (child->hasTagName(paramTag)) {
                HTMLParamElement* p = static_cast<HTMLParamElement*>(child);
                String name = p->name();
                if (url.isEmpty() && (equalIgnoringCase(name, "src") || equalIgnoringCase(name, "movie") || equalIgnoringCase(name, "code") || equalIgnoringCase(name, "url")))
                    url = p->value();
                if (serviceType.isEmpty() && equalIgnoringCase(name, "type")) {
                    serviceType = p->value();
                    int pos = serviceType.find(";");
                    if (pos != -1)
                        serviceType = serviceType.left(pos);
                }
                if (!embed && !name.isEmpty()) {
                    uniqueParamNames.add(name.impl());
                    paramNames.append(p->name());
                    paramValues.append(p->value());
                }
            }
            child = child->nextSibling();
        }

        // When OBJECT is used for an applet via Sun's Java plugin, the CODEBASE attribute in the tag
        // points to the Java plugin itself (an ActiveX component) while the actual applet CODEBASE is
        // in a PARAM tag. See <http://java.sun.com/products/plugin/1.2/docs/tags.html>. This means
        // we have to explicitly suppress the tag's CODEBASE attribute if there is none in a PARAM,
        // else our Java plugin will misinterpret it. [4004531]
        String codebase;
        if (!embed && MIMETypeRegistry::isJavaAppletMIMEType(serviceType)) {
            codebase = "codebase";
            uniqueParamNames.add(codebase.impl()); // pretend we found it in a PARAM already
        }
        
        // Turn the attributes of either the EMBED tag or OBJECT tag into arrays, but don't override PARAM values.
        NamedNodeMap* attributes = embedOrObject->attributes();
        if (attributes) {
            for (unsigned i = 0; i < attributes->length(); ++i) {
                Attribute* it = attributes->attributeItem(i);
                const AtomicString& name = it->name().localName();
                if (embed || !uniqueParamNames.contains(name.impl())) {
                    paramNames.append(name.string());
                    paramValues.append(it->value().string());
                }
            }
        }

        mapDataParamToSrc(&paramNames, &paramValues);

        // If we still don't have a type, try to map from a specific CLASSID to a type.
        if (serviceType.isEmpty())
            serviceType = serviceTypeForClassId(o->classId());

        if (!isURLAllowed(document(), url))
            return;

        // Find out if we support fallback content.
        m_hasFallbackContent = false;
        for (Node *child = o->firstChild(); child && !m_hasFallbackContent; child = child->nextSibling()) {
            if ((!child->isTextNode() && !child->hasTagName(embedTag) && !child->hasTagName(paramTag)) || // Discount <embed> and <param>
                (child->isTextNode() && !static_cast<Text*>(child)->containsOnlyWhitespace()))
                m_hasFallbackContent = true;
        }

        if (onlyCreateNonNetscapePlugins) {
            KURL completedURL;
            if (!url.isEmpty())
                completedURL = frame->loader()->completeURL(url);

            if (frame->loader()->client()->objectContentType(completedURL, serviceType) == ObjectContentNetscapePlugin)
                return;
        }

        bool success = o->dispatchBeforeLoadEvent(url) &&
                       frame->loader()->requestObject(this, url, o->getAttribute(nameAttr), serviceType, paramNames, paramValues);
        if (!success && m_hasFallbackContent)
            o->renderFallbackContent();
    } else if (node()->hasTagName(embedTag)) {
        HTMLEmbedElement *o = static_cast<HTMLEmbedElement*>(node());
        o->setNeedWidgetUpdate(false);
        url = o->url();
        serviceType = o->serviceType();

        if (url.isEmpty() && serviceType.isEmpty())
            return;
        if (!isURLAllowed(document(), url))
            return;

        // add all attributes set on the embed object
        NamedNodeMap* a = o->attributes();
        if (a) {
            for (unsigned i = 0; i < a->length(); ++i) {
                Attribute* it = a->attributeItem(i);
                paramNames.append(it->name().localName().string());
                paramValues.append(it->value().string());
            }
        }

        if (onlyCreateNonNetscapePlugins) {
            KURL completedURL;
            if (!url.isEmpty())
                completedURL = frame->loader()->completeURL(url);

            if (frame->loader()->client()->objectContentType(completedURL, serviceType) == ObjectContentNetscapePlugin)
                return;

        }

        if (o->dispatchBeforeLoadEvent(url))
            frame->loader()->requestObject(this, url, o->getAttribute(nameAttr), serviceType, paramNames, paramValues);
    }
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)        
    else if (node()->hasTagName(videoTag) || node()->hasTagName(audioTag)) {
        HTMLMediaElement* o = static_cast<HTMLMediaElement*>(node());

        o->setNeedWidgetUpdate(false);
        if (node()->hasTagName(videoTag)) {
            HTMLVideoElement* vid = static_cast<HTMLVideoElement*>(node());
            String poster = vid->poster();
            if (!poster.isEmpty()) {
                paramNames.append("_media_element_poster_");
                paramValues.append(poster);
            }
        }

        url = o->initialURL();
        if (!url.isEmpty()) {
            paramNames.append("_media_element_src_");
            paramValues.append(url);
        }

        serviceType = "application/x-media-element-proxy-plugin";
        
        if (o->dispatchBeforeLoadEvent(url))
            frame->loader()->requestObject(this, url, nullAtom, serviceType, paramNames, paramValues);
    }
#endif
}

void RenderPartObject::layout()
{
    ASSERT(needsLayout());

#if ENABLE(WKC_FRAME_FLATTENING)
    if (shouldResizeFrameToContent()) {
        //FrameView* childFrameView = static_cast<FrameView*>(m_widget);
        FrameView* childFrameView = static_cast<FrameView*>(m_widget.get());
        //RenderView* childRoot = childFrameView ? static_cast<RenderView*>(childFrameView->frame()->renderer()) : 0;
        RenderView* childRoot = childFrameView ? static_cast<RenderView*>(childFrameView->frame()->contentRenderer()) : 0;
        if (childRoot && childRoot->prefWidthsDirty())
            childRoot->calcPrefWidths();
        
        RenderPart::calcWidth();
        RenderPart::calcHeight();
        // FIXME: PURPLE: Do we need to call this here:  adjustOverflowForBoxShadow();
        
        //bool scrolling = static_cast<HTMLIFrameElement*>(element())->scrollingMode() != ScrollbarAlwaysOff;
        bool scrolling = static_cast<HTMLIFrameElement*>(node())->scrollingMode() != ScrollbarAlwaysOff;
        if (childRoot && (scrolling || !style()->width().isFixed()))
            //m_width = max(m_width, childRoot->minPrefWidth());
            setWidth(max(width(), childRoot->minPrefWidth()));

        updateWidgetPosition();
        do
            childFrameView->layout();
        while (childFrameView->layoutPending() || (childRoot && childRoot->needsLayout()));
        
        if (scrolling || !style()->height().isFixed())
            //m_height = max(m_height, childFrameView->contentsHeight());
            setHeight(max(height(), childFrameView->contentsHeight()));
        if (scrolling || !style()->width().isFixed())
            //m_width = max(m_width, childFrameView->contentsWidth());
            setWidth(max(width(), childFrameView->contentsWidth()));
        
        updateWidgetPosition();
    }
    else
    {
#endif
    calcWidth();
    calcHeight();

    RenderPart::layout();
#if ENABLE(WKC_FRAME_FLATTENING)
    }
#endif

    m_overflow.clear();
    addShadowOverflow();

    if (!widget() && frameView())
        frameView()->addWidgetToUpdate(this);

    setNeedsLayout(false);
}

void RenderPartObject::viewCleared()
{
    if (node() && widget() && widget()->isFrameView()) {
        FrameView* view = static_cast<FrameView*>(widget());
        int marginw = -1;
        int marginh = -1;
        if (node()->hasTagName(iframeTag)) {
            HTMLIFrameElement* frame = static_cast<HTMLIFrameElement*>(node());
            marginw = frame->getMarginWidth();
            marginh = frame->getMarginHeight();
        }
        if (marginw != -1)
            view->setMarginWidth(marginw);
        if (marginh != -1)
            view->setMarginHeight(marginh);
    }
}

#if ENABLE(WKC_FRAME_FLATTENING)
void RenderPartObject::calcWidth()
{
    RenderPart::calcWidth();
    if (!shouldResizeFrameToContent())
        return;
    //if (!style()->width().isFixed() || static_cast<HTMLIFrameElement*>(element())->scrollingMode() != ScrollbarAlwaysOff)
    if (!style()->width().isFixed() || static_cast<HTMLIFrameElement*>(node())->scrollingMode() != ScrollbarAlwaysOff)
        //m_width = max(m_width,  static_cast<FrameView*>(m_widget)->contentsWidth());
        setWidth(max(width(),  static_cast<FrameView*>(m_widget.get())->contentsWidth()));
}
    
void RenderPartObject::calcHeight()
{
    RenderPart::calcHeight();
    if (!shouldResizeFrameToContent())
        return;
    //if (!style()->height().isFixed() || static_cast<HTMLIFrameElement*>(element())->scrollingMode() != ScrollbarAlwaysOff)
    if (!style()->height().isFixed() || static_cast<HTMLIFrameElement*>(node())->scrollingMode() != ScrollbarAlwaysOff)
        //m_height = max(m_height,  static_cast<FrameView*>(m_widget)->contentsHeight());
        setHeight(max(height(),  static_cast<FrameView*>(m_widget.get())->contentsHeight()));
}
  
bool RenderPartObject::shouldResizeFrameToContent() const
{
    /*Document* document = element() ? element()->document() : 0;
    return m_widget && m_widget->isFrameView() && document && element()->hasTagName(iframeTag) && document->frame() && 
        document->frame()->settings() && document->frame()->settings()->flatFrameSetLayoutEnabled() && 
        (static_cast<HTMLIFrameElement*>(element())->scrollingMode() != ScrollbarAlwaysOff || !style()->width().isFixed() || !style()->height().isFixed());
    */

    Document* document = node() ? node()->document() : 0;
    return m_widget && m_widget->isFrameView() && document && !node()->hasTagName(iframeTag) && !node()->hasTagName(embedTag) && !node()->hasTagName(objectTag) && document->frame() && 
        document->frame()->settings() && document->frame()->settings()->flatFrameSetLayoutEnabled() && 
        !style()->width().isZero() && !style()->height().isZero() &&
        (static_cast<HTMLFrameElementBase*>(node())->scrollingMode() != ScrollbarAlwaysOff || !style()->width().isFixed() || !style()->height().isFixed());
}
#endif
}
