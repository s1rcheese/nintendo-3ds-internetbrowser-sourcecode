/*
 * WKCDownload.h
 *
 * Copyright (c) 2010, 2011 ACCESS CO., LTD. All rights reserved.
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

#ifndef WKCDownload_h
#define WKCDownload_h

// class definition

namespace WebCore {
}

namespace WKC {

class WKCDownload;

class WKCWebView;
class WKCDownloadPrivate;

class ResourceHandle;
class ResourceRequest;
class ResourceResponse;

class WKCDownloadClient {
public:
    virtual void didReceiveData(WKCDownload*, const char*, long long, long long) = 0;
    virtual void didFinishLoading(WKCDownload*) = 0;
    virtual void didFail(WKCDownload*, int) = 0;
    virtual void wasBlocked(WKCDownload*) = 0;
    virtual void cannotShowURL(WKCDownload*) = 0;
};

class WKCDownload {
public:
    static WKCDownload* create(WKCWebView*, WKCDownloadClient&, const WKC::ResourceRequest&);
    static void deleteWKCDownload(WKCDownload* self);

    void notifyForceTerminate();

    bool setResponse(WKC::ResourceHandle*, const WKC::ResourceResponse&);
    bool start();
    void cancel();
    const char* getUri() const;
    const char* getSuggestedFilename() const;

    const WKC::ResourceRequest& request() const;

    int getProgressInPercent() const;
    int getElapsedTimeInMilliSeconds() const;
    long long getTotalSize() const;
    long long getCurrentSize() const;

    enum Status {
        EError = -1,
        ECreated = 0,
        EStarted,
        ECancelled,
        EFinished,
        EStatus
    };
    int getStatus() const;

    enum Error {
        EErrorNone = 0,
        EErrorCancelled,
        EErrorNetwork,
        EErrors
    };
    int getError() const;

private:
    WKCDownload();
    ~WKCDownload();

    bool construct(WKCWebView*, WKCDownloadClient&, const ResourceRequest&);

private:
    WKCDownloadPrivate* m_private;
};

} // namespace

#endif // WKCDownload_h
