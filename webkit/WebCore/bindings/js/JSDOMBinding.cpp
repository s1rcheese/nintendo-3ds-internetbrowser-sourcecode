/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Samuel Weinig <sam@webkit.org>
 *  Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "JSDOMBinding.h"

#include "debugger/DebuggerCallFrame.h"

#include "ActiveDOMObject.h"
#if 1
// added at webkit.org trunk r 57627
#include "CSSHelper.h"
#endif
#include "DOMCoreException.h"
#include "Document.h"
#include "EventException.h"
#include "ExceptionBase.h"
#include "ExceptionCode.h"
#include "Frame.h"
#include "HTMLAudioElement.h"
#if 1
// added at webkit.org trunk r 57627
#include "HTMLFrameElementBase.h"
#endif
#include "HTMLCanvasElement.h"
#include "HTMLImageElement.h"
#include "HTMLScriptElement.h"
#include "HTMLNames.h"
#include "JSDOMCoreException.h"
#include "JSDOMWindowCustom.h"
#include "JSEventException.h"
#include "JSExceptionBase.h"
#include "JSNode.h"
#include "JSRangeException.h"
#include "JSXMLHttpRequestException.h"
#include "KURL.h"
#include "MessagePort.h"
#include "RangeException.h"
#include "ScriptCachedFrameData.h"
#include "ScriptController.h"
#include "Settings.h"
#include "XMLHttpRequestException.h"
#include <runtime/DateInstance.h>
#include <runtime/Error.h>
#include <runtime/JSFunction.h>
#include <runtime/PrototypeFunction.h>
#include <wtf/MathExtras.h>
#include <wtf/StdLibExtras.h>

#if ENABLE(SVG)
#include "JSSVGException.h"
#include "SVGException.h"
#endif

#if ENABLE(XPATH)
#include "JSXPathException.h"
#include "XPathException.h"
#endif

#if ENABLE(WORKERS)
#include <wtf/ThreadSpecific.h>
using namespace WTF;
#endif

using namespace JSC;

namespace {

WebCore::DOMWrapperWorld* gCachedNormalWorld = 0;

#if ENABLE(WORKERS)
static ThreadSpecific<HashSet<WebCore::DOMObject*> >* gStaticWrapperSet = 0;
#else
static HashSet<WebCore::DOMObject*>* gStaticWrapperSet = 0;
#endif

}

namespace WebCore {

using namespace HTMLNames;

typedef Document::JSWrapperCache JSWrapperCache;
typedef Document::JSWrapperCacheMap JSWrapperCacheMap;

// For debugging, keep a set of wrappers currently registered, and check that
// all are unregistered before they are destroyed. This has helped us fix at
// least one bug.

static void addWrapper(DOMObject* wrapper);
static void removeWrapper(DOMObject* wrapper);
static void removeWrappers(const JSWrapperCache& wrappers);
static void removeWrappers(const DOMObjectWrapperMap& wrappers);

#ifdef NDEBUG

static inline void addWrapper(DOMObject*)
{
}

static inline void removeWrapper(DOMObject*)
{
}

static inline void removeWrappers(const JSWrapperCache&)
{
}

static inline void removeWrappers(const DOMObjectWrapperMap&)
{
}

#else

static HashSet<DOMObject*>& wrapperSet()
{
    if (!gStaticWrapperSet) {
#if ENABLE(WORKERS)
        gStaticWrapperSet = new ThreadSpecific<HashSet<WebCore::DOMObject*> >();
#else
        gStaticWrapperSet = new HashSet<WebCore::DOMObject*>();
#endif
    }

    return *gStaticWrapperSet;
}

static void addWrapper(DOMObject* wrapper)
{
    ASSERT(!wrapperSet().contains(wrapper));
    wrapperSet().add(wrapper);
}

static void removeWrapper(DOMObject* wrapper)
{
    if (!wrapper)
        return;
    ASSERT(wrapperSet().contains(wrapper));
    wrapperSet().remove(wrapper);
}

static void removeWrappers(const JSWrapperCache& wrappers)
{
    JSWrapperCache::const_iterator wrappersEnd = wrappers.uncheckedEnd();
    for (JSWrapperCache::const_iterator it = wrappers.uncheckedBegin(); it != wrappersEnd; ++it)
        removeWrapper(it->second);
}

static inline void removeWrappers(const DOMObjectWrapperMap& wrappers)
{
    DOMObjectWrapperMap::const_iterator wrappersEnd = wrappers.uncheckedEnd();
    for (DOMObjectWrapperMap::const_iterator it = wrappers.uncheckedBegin(); it != wrappersEnd; ++it)
        removeWrapper(it->second);
}

DOMObject::~DOMObject()
{
    ASSERT(!wrapperSet().contains(this));
}

#endif

DOMWrapperWorld::DOMWrapperWorld(JSC::JSGlobalData* globalData)
    : m_globalData(globalData)
{
}

DOMWrapperWorld::~DOMWrapperWorld()
{
    JSGlobalData::ClientData* clientData = m_globalData->clientData;
    ASSERT(clientData);
    static_cast<WebCoreJSClientData*>(clientData)->forgetWorld(this);

    removeWrappers(m_wrappers);

    for (HashSet<Document*>::iterator iter = documentsWithWrappers.begin(); iter != documentsWithWrappers.end(); ++iter)
        forgetWorldOfDOMNodesForDocument(*iter, this);
}

class JSGlobalDataWorldIterator {
public:
    JSGlobalDataWorldIterator(JSGlobalData* globalData)
        : m_pos(static_cast<WebCoreJSClientData*>(globalData->clientData)->m_worldSet.begin())
        , m_end(static_cast<WebCoreJSClientData*>(globalData->clientData)->m_worldSet.end())
    {
    }

    operator bool()
    {
        return m_pos != m_end;
    }

    DOMWrapperWorld* operator*()
    {
        ASSERT(m_pos != m_end);
        return *m_pos;
    }

    DOMWrapperWorld* operator->()
    {
        ASSERT(m_pos != m_end);
        return *m_pos;
    }

    JSGlobalDataWorldIterator& operator++()
    {
        ++m_pos;
        return *this;
    }

private:
    HashSet<DOMWrapperWorld*>::iterator m_pos;
    HashSet<DOMWrapperWorld*>::iterator m_end;
};

DOMWrapperWorld* currentWorld(JSC::ExecState* exec)
{
    return static_cast<JSDOMGlobalObject*>(exec->lexicalGlobalObject())->world();
}

DOMWrapperWorld* normalWorld(JSC::JSGlobalData& globalData)
{
    JSGlobalData::ClientData* clientData = globalData.clientData;
    ASSERT(clientData);
    return static_cast<WebCoreJSClientData*>(clientData)->normalWorld();
}

DOMWrapperWorld* mainThreadNormalWorld()
{
    ASSERT(isMainThread());

    if (!gCachedNormalWorld) {
        gCachedNormalWorld = normalWorld(*JSDOMWindow::commonJSGlobalData());
        if (!gCachedNormalWorld)
            return false;
    }

    return gCachedNormalWorld;
}

void cachedNormalWorld_deletesSharedInstance()
{
    delete gStaticWrapperSet;
    gStaticWrapperSet = 0;
    gCachedNormalWorld = 0;
}

void cachedNormalWorld_resetVariables()
{
    gStaticWrapperSet = 0;
    gCachedNormalWorld = 0;
}

DOMObjectHashTableMap& DOMObjectHashTableMap::mapFor(JSGlobalData& globalData)
{
    JSGlobalData::ClientData* clientData = globalData.clientData;
    ASSERT(clientData);
    return static_cast<WebCoreJSClientData*>(clientData)->hashTableMap;
}

const JSC::HashTable* getHashTableForGlobalData(JSGlobalData& globalData, const JSC::HashTable* staticTable)
{
    return DOMObjectHashTableMap::mapFor(globalData).get(staticTable);
}

static inline DOMObjectWrapperMap& DOMObjectWrapperMapFor(JSC::ExecState* exec)
{
    return currentWorld(exec)->m_wrappers;
}

bool hasCachedDOMObjectWrapperUnchecked(JSGlobalData* globalData, void* objectHandle)
{
    for (JSGlobalDataWorldIterator worldIter(globalData); worldIter; ++worldIter) {
        if (worldIter->m_wrappers.uncheckedGet(objectHandle))
            return true;
    }
    return false;
}

bool hasCachedDOMObjectWrapper(JSGlobalData* globalData, void* objectHandle)
{
    for (JSGlobalDataWorldIterator worldIter(globalData); worldIter; ++worldIter) {
        if (worldIter->m_wrappers.get(objectHandle))
            return true;
    }
    return false;
}

DOMObject* getCachedDOMObjectWrapper(JSC::ExecState* exec, void* objectHandle) 
{
    return DOMObjectWrapperMapFor(exec).get(objectHandle);
}

void cacheDOMObjectWrapper(JSC::ExecState* exec, void* objectHandle, DOMObject* wrapper) 
{
    addWrapper(wrapper);
    DOMObjectWrapperMapFor(exec).set(objectHandle, wrapper);
}

bool hasCachedDOMNodeWrapperUnchecked(Document* document, Node* node)
{
    if (!document)
        return hasCachedDOMObjectWrapperUnchecked(JSDOMWindow::commonJSGlobalData(), node);

    JSWrapperCacheMap& wrapperCacheMap = document->wrapperCacheMap();
    for (JSWrapperCacheMap::iterator iter = wrapperCacheMap.begin(); iter != wrapperCacheMap.end(); ++iter) {
        if (iter->second->uncheckedGet(node))
            return true;
    }
    return false;
}

JSNode* getCachedDOMNodeWrapper(JSC::ExecState* exec, Document* document, Node* node)
{
    if (document)
        return document->getWrapperCache(currentWorld(exec))->get(node);
    return static_cast<JSNode*>(DOMObjectWrapperMapFor(exec).get(node));
}

void forgetDOMObject(DOMObject* wrapper, void* objectHandle)
{
    JSC::JSGlobalData* globalData = Heap::heap(wrapper)->globalData();
    for (JSGlobalDataWorldIterator worldIter(globalData); worldIter; ++worldIter) {
        if (worldIter->m_wrappers.uncheckedRemove(objectHandle, wrapper))
            break;
    }
    removeWrapper(wrapper);
}

void forgetDOMNode(JSNode* wrapper, Node* node, Document* document)
{
    if (!document) {
        forgetDOMObject(wrapper, node);
        return;
    }

    JSWrapperCacheMap& wrapperCacheMap = document->wrapperCacheMap();
    for (JSWrapperCacheMap::iterator wrappersIter = wrapperCacheMap.begin(); wrappersIter != wrapperCacheMap.end(); ++wrappersIter) {
        if (wrappersIter->second->uncheckedRemove(node, wrapper))
            break;
    }
    removeWrapper(wrapper);
}

void cacheDOMNodeWrapper(JSC::ExecState* exec, Document* document, Node* node, JSNode* wrapper)
{
    if (!document) {
        addWrapper(wrapper);
        DOMObjectWrapperMapFor(exec).set(node, wrapper);
        return;
    }
    addWrapper(wrapper);
    document->getWrapperCache(currentWorld(exec))->set(node, wrapper);
}

void forgetAllDOMNodesForDocument(Document* document)
{
    ASSERT(document);
    JSWrapperCacheMap& wrapperCacheMap = document->wrapperCacheMap();
    JSWrapperCacheMap::const_iterator wrappersMapEnd = wrapperCacheMap.end();
    for (JSWrapperCacheMap::const_iterator wrappersMapIter = wrapperCacheMap.begin(); wrappersMapIter != wrappersMapEnd; ++wrappersMapIter) {
        JSWrapperCache* wrappers = wrappersMapIter->second;
        removeWrappers(*wrappers);
        delete wrappers;
        wrappersMapIter->first->forgetDocument(document);
    }
}

void forgetWorldOfDOMNodesForDocument(Document* document, DOMWrapperWorld* world)
{
    JSWrapperCache* wrappers = document->wrapperCacheMap().take(world);
    ASSERT(wrappers); // 'world' should only know about 'document' if 'document' knows about 'world'!
    removeWrappers(*wrappers);
    delete wrappers;
}

static inline bool isObservableThroughDOM(JSNode* jsNode, DOMWrapperWorld* world)
{
    // Certain conditions implicitly make a JS DOM node wrapper observable
    // through the DOM, even if no explicit reference to it remains.

    Node* node = jsNode->impl();

    if (node->inDocument()) {
        // If a node is in the document, and its wrapper has custom properties,
        // the wrapper is observable because future access to the node through the
        // DOM must reflect those properties.
        if (jsNode->hasCustomProperties())
            return true;

        // If a node is in the document, and has event listeners, its wrapper is
        // observable because its wrapper is responsible for marking those event listeners.
        if (node->hasEventListeners())
            return true; // Technically, we may overzealously mark a wrapper for a node that has only non-JS event listeners. Oh well.

        // If a node owns another object with a wrapper with custom properties,
        // the wrapper must be treated as observable, because future access to
        // those objects through the DOM must reflect those properties.
        // FIXME: It would be better if this logic could be in the node next to
        // the custom markChildren functions rather than here.
        if (node->isElementNode()) {
            if (NamedNodeMap* attributes = static_cast<Element*>(node)->attributeMap()) {
                if (DOMObject* wrapper = world->m_wrappers.uncheckedGet(attributes)) {
                    if (wrapper->hasCustomProperties())
                        return true;
                }
            }
            if (node->isStyledElement()) {
                if (CSSMutableStyleDeclaration* style = static_cast<StyledElement*>(node)->inlineStyleDecl()) {
                    if (DOMObject* wrapper = world->m_wrappers.uncheckedGet(style)) {
                        if (wrapper->hasCustomProperties())
                            return true;
                    }
                }
            }
            if (static_cast<Element*>(node)->hasTagName(canvasTag)) {
                if (CanvasRenderingContext* context = static_cast<HTMLCanvasElement*>(node)->renderingContext()) {
                    if (DOMObject* wrapper = world->m_wrappers.uncheckedGet(context)) {
                        if (wrapper->hasCustomProperties())
                            return true;
                    }
                }
            }
        }
    } else {
        // If a wrapper is the last reference to an image or script element
        // that is loading but not in the document, the wrapper is observable
        // because it is the only thing keeping the image element alive, and if
        // the image element is destroyed, its load event will not fire.
        // FIXME: The DOM should manage this issue without the help of JavaScript wrappers.
        if (node->hasTagName(imgTag) && !static_cast<HTMLImageElement*>(node)->haveFiredLoadEvent())
            return true;
        if (node->hasTagName(scriptTag) && !static_cast<HTMLScriptElement*>(node)->haveFiredLoadEvent())
            return true;
#if ENABLE(VIDEO)
        if (node->hasTagName(audioTag) && !static_cast<HTMLAudioElement*>(node)->paused())
            return true;
#endif
    }

    // If a node is firing event listeners, its wrapper is observable because
    // its wrapper is responsible for marking those event listeners.
    if (node->isFiringEventListeners())
        return true;

    return false;
}

void markDOMNodesForDocument(MarkStack& markStack, Document* document)
{
    JSWrapperCacheMap& wrapperCacheMap = document->wrapperCacheMap();
    for (JSWrapperCacheMap::iterator wrappersIter = wrapperCacheMap.begin(); wrappersIter != wrapperCacheMap.end(); ++wrappersIter) {
        DOMWrapperWorld* world = wrappersIter->first;
        JSWrapperCache* nodeDict = wrappersIter->second;

        JSWrapperCache::iterator nodeEnd = nodeDict->uncheckedEnd();
        for (JSWrapperCache::iterator nodeIt = nodeDict->uncheckedBegin(); nodeIt != nodeEnd; ++nodeIt) {
            JSNode* jsNode = nodeIt->second;
            if (isObservableThroughDOM(jsNode, world))
                markStack.append(jsNode);
        }
    }
}

void markActiveObjectsForContext(MarkStack& markStack, JSGlobalData& globalData, ScriptExecutionContext* scriptExecutionContext)
{
    // If an element has pending activity that may result in event listeners being called
    // (e.g. an XMLHttpRequest), we need to keep JS wrappers alive.

    const HashMap<ActiveDOMObject*, void*>& activeObjects = scriptExecutionContext->activeDOMObjects();
    HashMap<ActiveDOMObject*, void*>::const_iterator activeObjectsEnd = activeObjects.end();
    for (HashMap<ActiveDOMObject*, void*>::const_iterator iter = activeObjects.begin(); iter != activeObjectsEnd; ++iter) {
        if (iter->first->hasPendingActivity()) {
            // Generally, an active object with pending activity must have a wrapper to mark its listeners.
            // However, some ActiveDOMObjects don't have JS wrappers (timers created by setTimeout is one example).
            // FIXME: perhaps need to make sure even timers have a markable 'wrapper'.
            markDOMObjectWrapper(markStack, globalData, iter->second);
        }
    }

    const HashSet<MessagePort*>& messagePorts = scriptExecutionContext->messagePorts();
    HashSet<MessagePort*>::const_iterator portsEnd = messagePorts.end();
    for (HashSet<MessagePort*>::const_iterator iter = messagePorts.begin(); iter != portsEnd; ++iter) {
        // If the message port is remotely entangled, then always mark it as in-use because we can't determine reachability across threads.
        if (!(*iter)->locallyEntangledPort() || (*iter)->hasPendingActivity())
            markDOMObjectWrapper(markStack, globalData, *iter);
    }
}

typedef std::pair<JSNode*, DOMWrapperWorld*> WrapperAndWorld;
typedef WTF::Vector<WrapperAndWorld, 8> WrapperSet;

static inline void takeWrappers(Node* node, Document* document, WrapperSet& wrapperSet)
{
    if (document) {
        JSWrapperCacheMap& wrapperCacheMap = document->wrapperCacheMap();
        for (JSWrapperCacheMap::iterator iter = wrapperCacheMap.begin(); iter != wrapperCacheMap.end(); ++iter) {
            if (JSNode* wrapper = iter->second->take(node)) {
                removeWrapper(wrapper);
                wrapperSet.append(WrapperAndWorld(wrapper, iter->first));
            }
        }
    } else {
        for (JSGlobalDataWorldIterator worldIter(JSDOMWindow::commonJSGlobalData()); worldIter; ++worldIter) {
            DOMWrapperWorld* world = *worldIter;
            if (JSNode* wrapper = static_cast<JSNode*>(world->m_wrappers.take(node))) {
                removeWrapper(wrapper);
                wrapperSet.append(WrapperAndWorld(wrapper, world));
            }
        }
    }
}

void updateDOMNodeDocument(Node* node, Document* oldDocument, Document* newDocument)
{
    ASSERT(oldDocument != newDocument);

    WrapperSet wrapperSet;
    takeWrappers(node, oldDocument, wrapperSet);

    for (unsigned i = 0; i < wrapperSet.size(); ++i) {
        JSNode* wrapper = wrapperSet[i].first;
        if (newDocument)
            newDocument->getWrapperCache(wrapperSet[i].second)->set(node, wrapper);
        else
            wrapperSet[i].second->m_wrappers.set(node, wrapper);
        addWrapper(wrapper);
    }
}

void markDOMObjectWrapper(MarkStack& markStack, JSGlobalData& globalData, void* object)
{
    // FIXME: This could be changed to only mark wrappers that are "observable"
    // as markDOMNodesForDocument does, allowing us to collect more wrappers,
    // but doing this correctly would be challenging.
    if (!object)
        return;

    for (JSGlobalDataWorldIterator worldIter(&globalData); worldIter; ++worldIter) {
        if (DOMObject* wrapper = worldIter->m_wrappers.uncheckedGet(object))
            markStack.append(wrapper);
    }
}

void markDOMNodeWrapper(MarkStack& markStack, Document* document, Node* node)
{
    if (document) {
        JSWrapperCacheMap& wrapperCacheMap = document->wrapperCacheMap();
        for (JSWrapperCacheMap::iterator iter = wrapperCacheMap.begin(); iter != wrapperCacheMap.end(); ++iter) {
            if (JSNode* wrapper = iter->second->uncheckedGet(node))
                markStack.append(wrapper);
        }
        return;
    }

    for (JSGlobalDataWorldIterator worldIter(JSDOMWindow::commonJSGlobalData()); worldIter; ++worldIter) {
        if (DOMObject* wrapper = worldIter->m_wrappers.uncheckedGet(node))
            markStack.append(wrapper);
    }
}

JSValue jsStringOrNull(ExecState* exec, const String& s)
{
    if (s.isNull())
        return jsNull();
    return jsString(exec, s);
}

JSValue jsOwnedStringOrNull(ExecState* exec, const UString& s)
{
    if (s.isNull())
        return jsNull();
    return jsOwnedString(exec, s);
}

JSValue jsStringOrUndefined(ExecState* exec, const String& s)
{
    if (s.isNull())
        return jsUndefined();
    return jsString(exec, s);
}

JSValue jsStringOrFalse(ExecState* exec, const String& s)
{
    if (s.isNull())
        return jsBoolean(false);
    return jsString(exec, s);
}

JSValue jsStringOrNull(ExecState* exec, const KURL& url)
{
    if (url.isNull())
        return jsNull();
    return jsString(exec, url.string());
}

JSValue jsStringOrUndefined(ExecState* exec, const KURL& url)
{
    if (url.isNull())
        return jsUndefined();
    return jsString(exec, url.string());
}

JSValue jsStringOrFalse(ExecState* exec, const KURL& url)
{
    if (url.isNull())
        return jsBoolean(false);
    return jsString(exec, url.string());
}

UString valueToStringWithNullCheck(ExecState* exec, JSValue value)
{
    if (value.isNull())
        return UString();
    return value.toString(exec);
}

UString valueToStringWithUndefinedOrNullCheck(ExecState* exec, JSValue value)
{
    if (value.isUndefinedOrNull())
        return UString();
    return value.toString(exec);
}

JSValue jsDateOrNull(ExecState* exec, double value)
{
    if (!isfinite(value))
        return jsNull();
    return new (exec) DateInstance(exec, value);
}

double valueToDate(ExecState* exec, JSValue value)
{
    if (value.isNumber())
        return value.uncheckedGetNumber();
    if (!value.inherits(&DateInstance::info))
        return std::numeric_limits<double>::quiet_NaN();
    return static_cast<DateInstance*>(value.toObject(exec))->internalNumber();
}

void reportException(ExecState* exec, JSValue exception)
{
    UString errorMessage = exception.toString(exec);
    JSObject* exceptionObject = exception.toObject(exec);
    int lineNumber = exceptionObject->get(exec, Identifier(exec, "line")).toInt32(exec);
    UString exceptionSourceURL = exceptionObject->get(exec, Identifier(exec, "sourceURL")).toString(exec);
    exec->clearException();

    if (ExceptionBase* exceptionBase = toExceptionBase(exception))
        errorMessage = exceptionBase->message() + ": "  + exceptionBase->description();

    ScriptExecutionContext* scriptExecutionContext = static_cast<JSDOMGlobalObject*>(exec->lexicalGlobalObject())->scriptExecutionContext();
    ASSERT(scriptExecutionContext);

    // Crash data indicates null-dereference crashes at this point in the Safari 4 Public Beta.
    // It's harmless to return here without reporting the exception to the log and the debugger in this case.
    if (!scriptExecutionContext)
        return;

    scriptExecutionContext->reportException(errorMessage, lineNumber, exceptionSourceURL);
}

void reportCurrentException(ExecState* exec)
{
    JSValue exception = exec->exception();
    exec->clearException();
    reportException(exec, exception);
}

void setDOMException(ExecState* exec, ExceptionCode ec)
{
    if (!ec || exec->hadException())
        return;

    // FIXME: All callers to setDOMException need to pass in the right global object
    // for now, we're going to assume the lexicalGlobalObject.  Which is wrong in cases like this:
    // frames[0].document.createElement(null, null); // throws an exception which should have the subframes prototypes.
    JSDOMGlobalObject* globalObject = deprecatedGlobalObjectForPrototype(exec);

    ExceptionCodeDescription description;
    getExceptionCodeDescription(ec, description);

    JSValue errorObject;
    switch (description.type) {
        case DOMExceptionType:
            errorObject = toJS(exec, globalObject, DOMCoreException::create(description));
            break;
        case RangeExceptionType:
            errorObject = toJS(exec, globalObject, RangeException::create(description));
            break;
        case EventExceptionType:
            errorObject = toJS(exec, globalObject, EventException::create(description));
            break;
        case XMLHttpRequestExceptionType:
            errorObject = toJS(exec, globalObject, XMLHttpRequestException::create(description));
            break;
#if ENABLE(SVG)
        case SVGExceptionType:
            errorObject = toJS(exec, globalObject, SVGException::create(description).get(), 0);
            break;
#endif
#if ENABLE(XPATH)
        case XPathExceptionType:
            errorObject = toJS(exec, globalObject, XPathException::create(description));
            break;
#endif
    }

    ASSERT(errorObject);
    exec->setException(errorObject);
}

bool checkNodeSecurity(ExecState* exec, Node* node)
{
    return node && allowsAccessFromFrame(exec, node->document()->frame());
}

bool allowsAccessFromFrame(ExecState* exec, Frame* frame)
{
    if (!frame)
        return false;
    JSDOMWindow* window = toJSDOMWindow(frame, currentWorld(exec));
    return window && window->allowsAccessFrom(exec);
}

bool allowsAccessFromFrame(ExecState* exec, Frame* frame, String& message)
{
    if (!frame)
        return false;
    JSDOMWindow* window = toJSDOMWindow(frame, currentWorld(exec));
    return window && window->allowsAccessFrom(exec, message);
}

bool shouldAllowNavigation(ExecState* exec, Frame* frame)
{
    Frame* lexicalFrame = toLexicalFrame(exec);
    return lexicalFrame && lexicalFrame->loader()->shouldAllowNavigation(frame);
}

#if 1
// added at webkit.org trunk r 57627
bool allowSettingSrcToJavascriptURL(ExecState* exec, Element* element, const String& name, const String& value)
{
    if ((element->hasTagName(iframeTag) || element->hasTagName(frameTag)) && equalIgnoringCase(name, "src") && protocolIsJavaScript(deprecatedParseURL(value))) {
          Document* contentDocument = static_cast<HTMLFrameElementBase*>(element)->contentDocument();
          if (contentDocument && !checkNodeSecurity(exec, contentDocument))
              return false;
      }
      return true;
}
#endif

void printErrorMessageForFrame(Frame* frame, const String& message)
{
    if (!frame)
        return;
    if (message.isEmpty())
        return;

    Settings* settings = frame->settings();
    if (!settings)
        return;
    if (settings->privateBrowsingEnabled())
        return;

    frame->domWindow()->console()->addMessage(JSMessageSource, LogMessageType, ErrorMessageLevel, message, 1, String()); // FIXME: provide a real line number and source URL.
}

Frame* toLexicalFrame(ExecState* exec)
{
    return asJSDOMWindow(exec->lexicalGlobalObject())->impl()->frame();
}

Frame* toDynamicFrame(ExecState* exec)
{
    return asJSDOMWindow(exec->dynamicGlobalObject())->impl()->frame();
}

bool processingUserGesture(ExecState* exec)
{
    Frame* frame = toDynamicFrame(exec);
    return frame && frame->script()->processingUserGesture();
}

KURL completeURL(ExecState* exec, const String& relativeURL)
{
    // For histoical reasons, we need to complete the URL using the dynamic frame.
    Frame* frame = toDynamicFrame(exec);
    if (!frame)
        return KURL();
    return frame->loader()->completeURL(relativeURL);
}

JSValue objectToStringFunctionGetter(ExecState* exec, const Identifier& propertyName, const PropertySlot&)
{
    return new (exec) NativeFunctionWrapper(exec, exec->lexicalGlobalObject()->prototypeFunctionStructure(), 0, propertyName, objectProtoFuncToString);
}

Structure* getCachedDOMStructure(JSDOMGlobalObject* globalObject, const ClassInfo* classInfo)
{
    JSDOMStructureMap& structures = globalObject->structures();
    return structures.get(classInfo).get();
}

Structure* cacheDOMStructure(JSDOMGlobalObject* globalObject, NonNullPassRefPtr<Structure> structure, const ClassInfo* classInfo)
{
    JSDOMStructureMap& structures = globalObject->structures();
    ASSERT(!structures.contains(classInfo));
    return structures.set(classInfo, structure).first->second.get();
}

Structure* getCachedDOMStructure(ExecState* exec, const ClassInfo* classInfo)
{
    return getCachedDOMStructure(static_cast<JSDOMGlobalObject*>(exec->lexicalGlobalObject()), classInfo);
}

Structure* cacheDOMStructure(ExecState* exec, NonNullPassRefPtr<Structure> structure, const ClassInfo* classInfo)
{
    return cacheDOMStructure(static_cast<JSDOMGlobalObject*>(exec->lexicalGlobalObject()), structure, classInfo);
}

JSObject* getCachedDOMConstructor(ExecState* exec, const ClassInfo* classInfo)
{
    JSDOMConstructorMap& constructors = static_cast<JSDOMGlobalObject*>(exec->lexicalGlobalObject())->constructors();
    return constructors.get(classInfo);
}

void cacheDOMConstructor(ExecState* exec, const ClassInfo* classInfo, JSObject* constructor)
{
    JSDOMConstructorMap& constructors = static_cast<JSDOMGlobalObject*>(exec->lexicalGlobalObject())->constructors();
    ASSERT(!constructors.contains(classInfo));
    constructors.set(classInfo, constructor);
}

JSC::JSObject* toJSSequence(ExecState* exec, JSValue value, unsigned& length)
{
    JSObject* object = value.getObject();
    if (!object) {
        throwError(exec, TypeError);
        return 0;
    }
    JSValue lengthValue = object->get(exec, exec->propertyNames().length);
    if (exec->hadException())
        return 0;

    if (lengthValue.isUndefinedOrNull()) {
        throwError(exec, TypeError);
        return 0;
    }

    length = lengthValue.toUInt32(exec);
    if (exec->hadException())
        return 0;

    return object;
}

bool DOMObject::defineOwnProperty(ExecState* exec, const Identifier&, PropertyDescriptor&, bool)
{
    throwError(exec, TypeError, "defineProperty is not supported on DOM Objects");
    return false;
}

} // namespace WebCore
