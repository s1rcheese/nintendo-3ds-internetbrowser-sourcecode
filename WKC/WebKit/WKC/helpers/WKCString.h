/*
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef _WKC_HELPER_WKC_STRING_H_
#define _WKC_HELPER_WKC_STRING_H_

namespace WKC {
class CString {
public:
    CString(const char*, int);
    ~CString();

    const char* data() const;
    int length() const;

    CString(const CString&);

private:
    void* m_parent;
};

class StringPrivate;

class String {
public:
    String();
    String(const String&);
    String(const char*);
    String(const char*, unsigned int);
    String(const unsigned short*);
    String(const unsigned short*, unsigned int);
    String(StringPrivate*);
    ~String();

    String& operator=(const String&);

    bool operator==(const char* b) const;
    bool operator!=(const char* b) const;

    void append(const String&);
    void append(const char*);
    void append(const unsigned short*);
    void append(const unsigned short*, unsigned int);

    int find(const String&);
    int find(unsigned short ch, int);

    String& replace(const unsigned short* a, const unsigned short* b);
    String& replace(const unsigned short* a, const String& b);
    String& replace(const String& a, const String& b);
    String& replace(unsigned index, unsigned len, const String& b);

    void truncate(unsigned int);

    void insert(const unsigned short*, unsigned int, unsigned int);
    void remove(unsigned int, unsigned int);

    String substring(unsigned int, unsigned int) const;

    static String format(const char *, ...);

    const unsigned short* characters() const;
    unsigned int length() const;

    CString& utf8() const;
    CString& latin1() const;

    const unsigned short* charactersWithNullTermination() const;

    bool isNull() const;
    bool isEmpty() const;

    StringPrivate* impl() const { return m_private; } 

private:
    StringPrivate* m_private;
};
} // namespace
#endif // _WKC_HELPER_WKC_STRING_H_
