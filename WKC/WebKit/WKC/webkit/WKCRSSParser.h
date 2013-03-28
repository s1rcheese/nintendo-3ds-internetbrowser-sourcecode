/*
 * WKCRSSParser.h
 *
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

#ifndef WKCRSSParser_h
#define WKCRSSParser_h

#include <time.h>

namespace WKC {

class WKCRSSParserPrivate;

class WKCRSSFeed {
public:
    const unsigned short* m_title;
    const unsigned short* m_description;
    const char* m_link;
    struct Date {
        enum Type {
            EType_Y,
            EType_YM,
            EType_YMD,
            EType_YMDHMZ,
            EType_YMDHMSZ
        } m_type;
        struct tm m_tm;
        int m_zone;
    };
    struct Item {
        struct Item* m_next;
        const unsigned short* m_title;
        const unsigned short* m_description;
        const unsigned short* m_content;
        const char* m_link;
        const struct Date* m_date;
    } *m_item;

public:
    unsigned int itemLength() const;

private:
    WKCRSSFeed();
    ~WKCRSSFeed();
    WKCRSSFeed::Item* appendItem();

    friend class WKCRSSParserPrivate;
};


class WKCRSSParser {
public:
    static WKCRSSParser* create();
    static void deleteWKCRSSParser(WKCRSSParser *self);
    void write(const char* in_str, unsigned int in_len, bool in_flush);
    const WKCRSSFeed* feed() const;
    enum Status {
        EStatus_OK,
        EStatus_NoInput,
        EStatus_NoMemory,
        EStatus_XMLError
    };
    enum Status status() const;

private:
    WKCRSSParser();
    ~WKCRSSParser();
    bool construct();

private:
    WKCRSSParserPrivate* m_private;
};

} // namespace

#endif  // WKCRSSParser_h
