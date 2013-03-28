/*
 * Copyright (C) 2009 Jan Michael C. Alonzo
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
 */

#ifndef WKCWebHistory_h
#define WKCWebHistory_h

#if 0
//#include "WebKit.h"

//#include "COMPtr.h"
//#include <CoreFoundation/CoreFoundation.h>
//#include <wtf/OwnArrayPtr.h>
//#include <wtf/RetainPtr.h>

namespace WebCore {
    class KURL;
    class PageGroup;
    class String;
}

//-----------------------------------------------------------------------------

//class WebHistory : public IWebHistory, public IWebHistoryPrivate {
class WKCWebHistory {
public:
    static WKCWebHistory* createInstance();
private:
    WKCWebHistory();
    ~WKCWebHistory();

public:
    // IUnknown
    // virtual long QueryInterface(REFIID riid, void** ppvObject);
    virtual unsigned long AddRef(void);
    virtual unsigned long Release(void);

#if 0
    // IWebHistory
    virtual long optionalSharedHistory( 
        /* [retval][out] */ IWebHistory** history);
    
    virtual long setOptionalSharedHistory( 
        /* [in] */ IWebHistory* history);
    
    virtual long loadFromURL( 
        /* [in] */ BSTR url,
        /* [out] */ IWebError** error,
        /* [retval][out] */ BOOL* succeeded);
    
    virtual long saveToURL( 
        /* [in] */ BSTR url,
        /* [out] */ IWebError** error,
        /* [retval][out] */ BOOL* succeeded);
    
    virtual long addItems( 
        /* [in] */ int itemCount,
        /* [in] */ IWebHistoryItem** items);
    
    virtual long removeItems( 
        /* [in] */ int itemCount,
        /* [in] */ IWebHistoryItem** items);
    
    virtual long removeAllItems( void);
    
    virtual long orderedLastVisitedDays( 
        /* [out][in] */ int* count,
        /* [in] */ DATE* calendarDates);
    
    virtual long orderedItemsLastVisitedOnDay( 
        /* [out][in] */ int* count,
        /* [in] */ IWebHistoryItem** items,
        /* [in] */ DATE calendarDate);

    virtual long itemForURL( 
        /* [in] */ BSTR url,
        /* [retval][out] */ IWebHistoryItem** item);
    
    virtual long setHistoryItemLimit( 
        /* [in] */ int limit);
    
    virtual long historyItemLimit( 
        /* [retval][out] */ int* limit);
    
    virtual long setHistoryAgeInDaysLimit( 
        /* [in] */ int limit);
    
    virtual long historyAgeInDaysLimit( 
        /* [retval][out] */ int* limit);

    // IWebHistoryPrivate

    virtual long allItems( 
        /* [out][in] */ int* count,
        /* [retval][out] */ IWebHistoryItem** items);

    virtual long data(IStream**);

    virtual long setVisitedLinkTrackingEnabled(BOOL visitedLinkTrackingEnable);
    virtual long removeAllVisitedLinks();

    // WebHistory
    static WebHistory* sharedHistory();
    void visitedURL(const WebCore::KURL&, const WebCore::String& title, const WebCore::String& httpMethod, bool wasFailure, bool increaseVisitCount);
    void addVisitedLinksToPageGroup(WebCore::PageGroup&);

    COMPtr<IWebHistoryItem> itemForURLString(const WebCore::String&) const;
#endif

    typedef long long int DateKey;
    // typedef HashMap<DateKey, RetainPtr<CFMutableArrayRef> > DateToEntriesMap;

private:

    enum NotificationType
    {
        kWebHistoryItemsAddedNotification = 0,
        kWebHistoryItemsRemovedNotification = 1,
        kWebHistoryAllItemsRemovedNotification = 2,
        kWebHistoryLoadedNotification = 3,
        kWebHistoryItemsDiscardedWhileLoadingNotification = 4,
        kWebHistorySavedNotification = 5
    };

#if 0
    long loadHistoryGutsFromURL(CFURLRef url, CFMutableArrayRef discardedItems, IWebError** error);
    long saveHistoryGuts(CFURLRef url, IWebError** error);
    long postNotification(NotificationType notifyType, IPropertyBag* userInfo = 0);
    long removeItem(IWebHistoryItem* entry);
    long addItem(IWebHistoryItem* entry, bool discardDuplicate, bool* added);
    long removeItemForURLString(CFStringRef urlString);
    long addItemToDateCaches(IWebHistoryItem* entry);
    long removeItemFromDateCaches(IWebHistoryItem* entry);
    long insertItem(IWebHistoryItem* entry, DateKey);
    long ageLimitDate(CFAbsoluteTime* time);
    bool findKey(DateKey*, CFAbsoluteTime forDay);
    static CFAbsoluteTime timeToDate(CFAbsoluteTime time);
    BSTR getNotificationString(NotificationType notifyType);
    long itemForURLString(CFStringRef urlString, IWebHistoryItem** item) const;
    RetainPtr<CFDataRef> data() const;
#endif

    unsigned long m_refCount;

#if 0
    RetainPtr<CFMutableDictionaryRef> m_entriesByURL;

    DateToEntriesMap m_entriesByDate;
    OwnArrayPtr<DATE> m_orderedLastVisitedDays;
    COMPtr<WebPreferences> m_preferences;
#endif
};

#endif  /* 0 */

#endif // WKCWebHistory_h
