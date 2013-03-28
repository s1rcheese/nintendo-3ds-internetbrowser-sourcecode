/*
 * WKCWebView.h
 *
 * Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.
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
 */

#ifndef WKCWebView_h
#define WKCWebView_h

// class definition

#include "WKCEnums.h"
#include "WKCMemoryEventHandler.h"
#include "WKCTimerEventHandler.h"
#include "WKCRSSLinkInfo.h"
#include "WKCSkin.h"

#include <wkc/wkcclib.h>

// prototypes
namespace WKC {
class Node;
class HistoryItem;
class Page;

class ResourceHandle;

class WKCWebView;
class WKCClientBuilders;
class WKCWebFrame;
class WKCWebViewPrivate;
class WKCWebInspector;
class WKCHitTestResult;
class WKCWebElementInfo;
class WKCSettings;
class String;
}

// callbacks
namespace WKC {
    typedef void (*ForceNotifyScrollProc)(WKCWebFrame* frame, const WKCPoint& scrollPoint);
    typedef void (*ResolveFilenameForDisplayProc)(const unsigned short* path, const int path_len, unsigned short* out_path, int* out_path_len, const int path_maxlen);

    typedef struct FileSystemProcs_ {
        FILE*  (*fOpenProc)(const char* in_filename, const char* in_mode);
        int    (*fCloseProc)(FILE* in_stream);
        size_t (*fReadProc)(void *out_buffer, size_t in_size, size_t in_count, FILE *in_stream);
        size_t (*fWriteProc)(const void *in_buffer, size_t in_size, size_t in_count, FILE *in_stream);
        int    (*fEOFProc)(FILE *in_stream);
        int    (*fStatProc)(int in_fd, struct stat *out_buf);
        int    (*fNoProc)(FILE *in_stream);
    } FileSystemProcs;

    typedef struct clientCertInfo_ {
        const char* issuer;
        const char* subject;
        const char* notbefore;
        const char* notafter;
        const char* serialNumber;
    } clientCertInfo;


    typedef struct HWOffscreenDeviceParams_ {
        void (*fLockProc)(void* in_opaque);
        void (*fUnlockProc)(void* in_opaque);
        bool fEnable;
        bool fEnableForImagebuffer;
    } HWOffscreenDeviceParams;

    typedef struct SystemStrings_ {
        const unsigned short* fNavigatorPlatform;
        const unsigned short* fNavigatorProduct;
        const unsigned short* fNavigatorProductSub;
        const unsigned short* fNavigatorVendor;
        const unsigned short* fNavigatorVendorSub;
        const unsigned short* fLanguage;
        const unsigned short* fButtonLabelSubmit;
        const unsigned short* fButtonLabelReset;
        const unsigned short* fButtonLabelFile;
    } SystemStrings;


    typedef struct ScreenDeviceParams_ {
        int fScreenWidth;
        int fScreenHeight;
        int fAvailableScreenWidth;
        int fAvailableScreenHeight;
        int fScreenDepth;
        int fScreenDepthPerComponent;
        bool fIsMonochrome;
    } ScreenDeviceParams;
}

// APIs
namespace WKC {

void WKCWebKitInitialize(void* memory, unsigned int memory_size, void* font_memory, unsigned int font_memory_size, WKCMemoryEventHandler& memory_event_handler, WKCTimerEventHandler& timer_event_handler);
void WKCWebKitFinalize();
void WKCWebKitResetVariables();
void WKCWebKitForceTerminate();
void WKCWebKitForceFinalize();
void WKCWebKitResetROMTables();
unsigned int WKCWebKitAvailableMemory();
unsigned int WKCWebKitMaxAvailableBlock();
void WKCWebKitRequestGarbageCollect(void);
unsigned int WKCWebKitFontHeapSize();
void WKCWebKitSuspendFont();
void WKCWebKitResumeFont(void* font_memory, unsigned int font_memory_size);
int WKCWebKitRegisterFontOnMemory(const unsigned char* memPtr);
int WKCWebKitRegisterFontInFile(const char* filePath);
void WKCWebKitUnregisterFonts();
bool WKCWebKitSetPrimaryFont(int fontID);
void WKCWebKitEnableScalingMonosizeFont(bool flag);
void WKCWebKitSetResolveFilenameForDisplayProc(WKC::ResolveFilenameForDisplayProc proc);
void WKCWebKitSetForceNotifyScrollProc(WKC::ForceNotifyScrollProc proc);
void WKCWebKitRegisterSkin(const WKC::WKCSkin* skin);

void WKCWebKitSetHWOffscreenDeviceParams(const HWOffscreenDeviceParams* params, void* opaque);
void WKCWebKitSetScreenDeviceParams(const ScreenDeviceParams& params);

void* WKCWebKitSSLRegisterRootCA(const char* cert, int cert_len);
int   WKCWebKitSSLUnregisterRootCA(void* certid);
void* WKCWebKitSSLRegisterCRL(const char* crl, int crl_len);
int   WKCWebKitSSLUnregisterCRL(void* crlid);
void* WKCWebKitSSLRegisterClientCert(const unsigned char* pkcs12, int pkcs12_len, const unsigned char* pass, int pass_len);
int   WKCWebKitSSLUnregisterClientCert(void* certid);
void  WKCWebKitSSLSetServerCA(const char *host_w_port, const char *ca);
void  WKCWebKitSSLSetAllowServerHost(const char *host_w_port);

void WKCWebKitSetFileSystemProcs(const WKC::FileSystemProcs* procs);

int WKCWebKitSetTimeZone(int offset, bool isSummerTime);

bool WKCWebKitSetGlyphCache(int format, void* cache, const WKCSize* size);
bool WKCWebKitSetImageCache(int format, void* cache, const WKCSize* size);

void WKCWebKitSetSystemStrings(const WKC::SystemStrings* strings);

void WKCWebKitSetThreadInfo(void* thread_id, void* stack_base);
void WKCWebKitSetStackSize(unsigned int stack_size);

bool WKCWebKitRegisterI18NData(const void* data, unsigned int length);
void WKCWebKitUnregisterI18NData();

void WKCWebKitSetReservedMemorySizeAtCrashing(unsigned int size);

const WKC::String WKCWebKitGetNodeAttributeValue(const WKC::Node* node, const char* attributeName);

enum {
    EEncodingDetectorNone      = 0x00000000,
    EEncodingDetectorUniversal = 0x00000001,
    EEncodingDetectorJapanese  = 0x00000002,

    EEncodingDetectorAll       = EEncodingDetectorUniversal | EEncodingDetectorJapanese,

    EEncodingDetectorEndOfEnum
};

void WKCWebKitSetEncodingDetectorLanguageSet(int languageSetFlag);


// For Memory Debug
enum {
    ECollectorHeapTypeNone = 0,
    ECollectorHeapTypeJSGlobalData,
    ECollectorHeapTypeJSDOMWindowBase,
    ECollectorHeapTypes
};

enum {
    ECollectorHeapBlockInfoFlagNone = 0,
    ECollectorHeapBlockInfoFlagPeer = (1 << 0),
    ECollectorHeapBlockInfoFlagTCMalloc = (1 << 1),
    ECollectorHeapBlockInfoFlagBackup = (1 << 2),
    ECollectorHeapBlockInfoFlagNoCellUsed = (1 << 3),

    ECollectorHeapBlockInfoFlagEndOfEnum
};

enum {
    kMaxCollectorHeapBlockInfo = 350 // Initial Memory Size / BLOCK_SIZE
};

struct WKCWebKitCollectorHeapBlockInfo_ {
    size_t size;
    int type;   // Enumeration
    int flag;
    size_t usedCellCount;
};
typedef struct WKCWebKitCollectorHeapBlockInfo_ WKCWebKitCollectorHeapBlockInfo;

struct WKCWebKitCollectorHeapStatistics_ {
    int numBlockInfo;
    WKCWebKitCollectorHeapBlockInfo blockInfo[kMaxCollectorHeapBlockInfo];
};
typedef struct WKCWebKitCollectorHeapStatistics_ WKCWebKitCollectorHeapStatistics;


typedef struct TMemoryInfo_ TMemoryInfo;

#ifndef WKC_ENABLE_DYNAMIC_ALLOCATION
const unsigned int KMemoryMaxNumDetailedMemoryArray = 336;  // Initial Memory Size / kPageSize -> Initial Memory Size is 21Mbytes
#endif // !WKC_ENABLE_DYNAMIC_ALLOCATION

enum MemoryRangeType {
    EMemoryRangeTypeNormal  = 0,    // Must be the first
    EMemoryRangeTypeManagement,     // Must be the second
    EMemoryRangeTypeJSHeap,
    EMemoryRangeTypeJSStack,
    EMemoryRangeTypeHash,
    EMemoryRangeTypeSpan,
    EMemoryRangeTypeThreadHeap,
    EMemoryRangeTypes
};

enum MemoryBlockType {
    EMemoryBlockTypeNone        = 0x00000000,
    EMemoryBlockTypeAvailable   = 0x00000001,
    EMemoryBlockTypeUsed        = 0x00000010,
    EMemoryBlockTypeFreed       = 0x00000020,
    EMemoryBlockTypeHeap        = 0x00000100,
    EMemoryBlockTypeStack       = 0x00000200,
    EMemoryBlockTypeAligned     = 0x00001000,
    EMemoryBlockTypeMask        = 0x0000FFFF
};

struct TMemoryRangeInfo_ {
    void* fHead;
    void* fTail;
    unsigned int fSize;
};

typedef struct TMemoryRangeInfo_ TMemoryRangeInfo;

struct TMemoryDetailedInfo_ {
    void* fAdr;
    unsigned int fRequestedSize;
    unsigned int fSize;
    unsigned int fRequestedRangeType;   // EMemoryRangeTypeXXX
    unsigned int fDefaultRangeType;     // EMemoryRangeTypeXXX
    unsigned int fBlockType;            // EMemoryBlockTypeXXX
    unsigned int fRequestedAlignment;
    unsigned int fAlignment;
};

typedef struct TMemoryDetailedInfo_ TMemoryDetailedInfo;

struct TMemoryInfo_ {
    unsigned int fAlignment;
    unsigned int fPage;
    TMemoryRangeInfo fSystem;
    TMemoryRangeInfo fRangeArray[EMemoryRangeTypes];
    unsigned int fNumDetailedArray;
#ifdef WKC_ENABLE_DYNAMIC_ALLOCATION
    TMemoryDetailedInfo* fDetailedArray;
    TMemoryDetailedInfo** fDetailedPtrArray;
#else
    TMemoryDetailedInfo fDetailedArray[KMemoryMaxNumDetailedMemoryArray];
    TMemoryDetailedInfo* fDetailedPtrArray[KMemoryMaxNumDetailedMemoryArray];
#endif // WKC_ENABLE_DYNAMIC_ALLOCATION
};

class WKCWebView
{
    friend class WKCWebFrame;

public:
    // life and death
    static WKCWebView* create(WKCClientBuilders& builders);
    static void deleteWKCWebView(WKCWebView *self);

    void notifyForceTerminate();

    // off-screen draw
    bool setOffscreen(WKC::OffscreenFormat format, void* bitmap, int rowbytes, const WKCSize& desktopsize, const WKCSize& viewsize, bool fixedlayout);
    void notifyResizeViewSize(const WKCSize& size);
    void notifyResizeDesktopSize(const WKCSize& size);
    void notifyRelayout(bool force = false);
    void notifyPaintOffscreen(const WKCRect& rect);
    void notifyScrollOffscreen(const WKCRect& rect, const WKCSize& diff);

    // events
    bool notifyKeyPress(WKC::Key key, WKC::Modifier modifiers);
    bool notifyKeyRelease(WKC::Key key, WKC::Modifier modifiers);
    bool notifyMouseDown(const WKCPoint& pos, WKC::MouseButton button, WKC::Modifier modifiers);
    bool notifyMouseUp(const WKCPoint& pos, WKC::MouseButton button, WKC::Modifier modifiers);
    bool notifyMouseMove(const WKCPoint& pos, WKC::MouseButton button, Modifier modifiers);
    bool notifyMouseDoubleClick(const WKCPoint& pos, WKC::MouseButton button, Modifier modifiers);
    bool notifyScroll(WKC::ScrollType scrolltype);
    void notifyFocusIn();
    void notifyFocusOut();
    WKC::Node* findFocusableNode(const WKC::FocusDirection direction, const WKCRect* specificRect = 0);
    WKC::Node* findFocusableNodeInRect(const WKC::FocusDirection direction, const WKCRect* rect);
    bool setFocusedNode(WKC::Node* node);
    void notifySuspend();
    void notifyResume();
    void notifyScrollPositionChanged();

    bool notifyScroll(int dx, int dy);
    bool notifyScrollTo(int x, int y);
    void scrollPosition(WKCPoint& pos);
    void contentsSize(WKCSize& size);

    // APIs
    const unsigned short* title();
    const char* uri();

    WKC::Page* core();
    WKC::WKCSettings* settings();

    void setMaintainsBackForwardList(bool flag);
    bool canGoBack();
    bool canGoBackOrForward(int steps);
    bool canGoForward();
    bool goBack();
    void goBackOrForward(int steps);
    bool goForward();

    void stopLoading();
    void reload();
    void reloadBypassCache();
    void loadURI(const char* uri);
    void loadString(const char* content, const unsigned short* mimetype, const unsigned short* encoding, const char* base_uri);
    void loadHTMLString(const char* content, const char* base_uri);

    bool searchText(const unsigned short* text, bool case_sensitive, bool forward, bool wrap);
    unsigned int markTextMatches(const unsigned short* string, bool case_sensitive, unsigned int limit);
    void setHighlightTextMatches(bool highlight);
    void unmarkTextMatches();

    WKCWebFrame* mainFrame();
    WKCWebFrame* focusedFrame();

    void executeScript(const char* script);
    void setJavaScriptURLsAreAllowed(bool flag);

    bool canCutClipboard();
    bool canCopyClipboard();
    bool canPasteClipboard();
    void cutClipboard();
    void copyClipboard();
    void pasteClipboard();
    void deleteSelection();
    bool hasSelection();
    void clearSelection();
//    bool selectionRects(WKC::Vector<WKCRect, 0>& rects, bool textonly, bool useSelectionHeight);
    WKCRect selectionBoundingBox(bool textonly, bool useSelectionHeight);
    const unsigned short* selectionText();
    void selectAll();

    bool editable();
    void setEditable(bool flag);

    WKCWebInspector* inspector();
    bool canShowMimeType(const unsigned short* mime_type);

    bool transparent();
    void setTransparent(bool flag);

    float zoomLevel();
    float setZoomLevel(float zoom_level);
    void zoomIn(float ratio);
    void zoomOut(float ratio);
    bool fullContentZoom();
    void setFullContentZoom(bool full_content_zoom);

    float opticalZoomLevel() const;
    const WKCFloatPoint& opticalZoomOffset() const;
    float setOpticalZoom(float zoom_level, const WKCFloatPoint& offset);

    const unsigned short* encoding();
    void setCustomEncoding(const unsigned short* encoding);
    const unsigned short* customEncoding();

    WKC::LoadStatus loadStatus();
    double progress();

    void undo();
    bool canUndo();
    void redo();
    bool canRedo();

    void setViewSourceMode(bool mode);
    bool viewSourceMode();

    WKCHitTestResult* hitTestResult();

    // caches
    static void setCacheCapacities(unsigned int min_dead_resource, unsigned int max_dead_resource, unsigned int total);
    static void setDeadDecodedDataDeletionInterval(double interval);
    static void setMinDelayBeforeLiveDecodedPruneCaches(double delay);
    static void cachedSize(unsigned int& dead_resource, unsigned int& live_resource);
    static void clearCaches();
    /* this api is obsolete */
    static void setCacheModel(WKC::CacheModel model, unsigned int total);

    // network related
    enum ProxyAuth {
        Basic  = 0,
        Digest = 1,
        NTLM   = 2,
        NONE   = 99
    };
    static void setProxy(bool enable, const char* host, int port, bool isHTTP10 = false, const char* proxyuser = 0, const char* proxypass = 0, ProxyAuth auth = NONE);

    static void setMaxTCPConnections(long num);

    unsigned int getRSSLinkNum();
    unsigned int getRSSLinkInfo(WKCRSSLinkInfo* info, unsigned int info_len);
    WKC::Node* getFocusedNode();
    WKC::Node* getNodeFromPoint(int x, int y);
    bool clickableFromPoint(int x, int y);
    bool draggableFromPoint(int x, int y);

    enum ScrollbarPart {
        NoPart,
        BackButtonPart,
        ForwardButtonPart,
        BackTrackPart,
        ThumbPart,
        ForwardTrackPart,
        ScrollbarBGPart,
        TrackBGPart,
        BackButtonStartPart,
        BackButtonEndPart,
        ForwardButtonStartPart,
        ForwardButtonEndPart,
    };
    bool isScrollbarFromPoint(int x, int y, ScrollbarPart& part, WKCRect& rect);

    WKCRect zoomRegionForPoint(const WKCPoint& hit, int minwidth);

    // cookie
    static void clearCookies();
    static void setMaxCookieEntries(long number);
    static void CookieSerializeStart();
    static int  CookieSerializeProgress(char*, unsigned int, unsigned int*);
    static void CookieSerializeEnd();
    static void CookieDeserializeStart(bool);
    static int  CookieDeserializeProgress(const char*, unsigned int);
    static void CookieDeserializeEnd();

    // History
    bool addVisitedLink(const char* uri, const unsigned short* title, const struct tm* date);
    void addHistoryItem(const char* uri, const unsigned short* title);
    unsigned int getHistoryLength();
    bool getHistoryCurrentIndex(unsigned int& index);
    bool getHistoryIndexByItem(WKC::HistoryItem* item, unsigned int& index);
    void removeHistoryItemByIndex(unsigned int index);
    bool getHistoryItemByIndex(unsigned int index, char* const uri, unsigned int& uriLen, unsigned short* const title, unsigned int& titleLen);
    void gotoHistoryItemByIndex(unsigned int index);

    // images
    enum {
        EInternalColorFormat8888,
        EInternalColorFormat5515withMask,
        EInternalColorFormats
    };
    static void setInternalColorFormat(int fmt);
    static void setMaxSizeOfImageForDownsampling(int width, int height);
    static void setUseDitherForImageDecode(int bpp, bool flag);
    void setUseBilinearForScaledImages(bool flag);
    void setUseAntiAliasForDrawings(bool flag);
    static void setUseBilinearForCanvasImages(bool flag);
    static void setUseAntiAliasForCanvas(bool flag);

    void setScreenWidth(int width);
    void setScrollPositionForOffscreen(const WKCPoint& scrollPosition);

    // JS Heap
    static void jsHeapStatistics(WKC::WKCWebKitCollectorHeapStatistics& stat);

    // Memory Debug
    static bool getTMemoryInfo(WKC::TMemoryInfo& memInfo);
    static bool allocTMemoryInfo(WKC::TMemoryInfo& memInfo);
    static void releaseTMemoryInfo(WKC::TMemoryInfo& memInfo);

private:
    WKCWebView();
    ~WKCWebView();
    bool construct(WKCClientBuilders& builders);

private:
    WKCWebViewPrivate* m_private;
};

namespace IDN {
int fromUnicode(const unsigned short* idn, char* host, int maxhost);
int toUnicode(const char* host, unsigned short* idn, int maxidn);
} // namespace IDN

} // namespace

#endif  // WKCWebView_h
