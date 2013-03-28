/*
 *  WKCRSSLinkInfo.h
 *
 *  Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 * 
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 * 
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 */

#ifndef WKCRSSLinkInfo_h
#define WKCRSSLinkInfo_h

namespace WKC {
    enum RSSLinkMaxLength {
        ERSSTitleLenMax = 128,
        ERSSUrlLenMax = 1024
    };
    enum RSSLinkFlag {
        ERSSLinkFlagNone = 0x0,
        ERSSLinkFlagTitleTruncate = 0x1,
        ERSSLinkFlagUrlTruncate = 0x2
    };
    typedef struct WKCRSSLinkInfo_ {
        unsigned int m_flag;
        unsigned short m_title[ERSSTitleLenMax + 1];
        char m_url[ERSSUrlLenMax + 1];
    } WKCRSSLinkInfo;
} // namespace

#endif // WKCRSSLinkInfo_h
