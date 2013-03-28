/*
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 * Copyright (C) 2011 Torch Mobile, Inc.
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

#include "config.h"

#include "CString.h"
#include "PlatformString.h"
#include "StringImpl.h"

#include "helpers/WKCString.h"

#include <stdarg.h>
#include <wchar.h>

namespace WKC {


CString::CString(const char* data, int len)
    : m_parent(new WebCore::CString(data,len))
{
}

CString::~CString()
{
    delete (WebCore::CString *)m_parent;
}

#define CPARENT() ((WebCore::CString *)m_parent)

const char*
CString::data() const
{
    return CPARENT()->data();
}


int 
CString::length() const
{
    return CPARENT()->length();
}

class StringPrivate {
public:
    StringPrivate();
    ~StringPrivate();

    inline void setImpl(WTF::RefPtr<WebCore::StringImpl> i) { if (i) m_impl = i; }
    inline WebCore::StringImpl* impl() const { return m_impl.get(); }

    const unsigned short* charactersWithNullTermination();
    CString& latin1();
    CString& utf8();

private:
    WTF::RefPtr<WebCore::StringImpl> m_impl;
    CString* m_latin1;
    CString* m_utf8;
};

StringPrivate::StringPrivate()
    : m_latin1(0)
    , m_utf8(0)
{
}

StringPrivate::~StringPrivate()
{
    delete m_latin1;
    delete m_utf8;
}

const unsigned short*
StringPrivate::charactersWithNullTermination()
{
    if (!m_impl)
        return 0;
    if (m_impl->hasTerminatingNullCharacter())
        return m_impl->characters();

    m_impl = WebCore::StringImpl::createWithTerminatingNullCharacter(*m_impl);
    return m_impl->characters();
}

CString&
StringPrivate::latin1()
{
    delete m_latin1;

    WebCore::String a(m_impl);
    WebCore::CString cs = a.latin1();
    if (!cs.isNull()) {
        m_latin1 = new CString(cs.data(), cs.length());
    } else {
        m_latin1 = new CString(0,0);
    }
    return *m_latin1;
}

CString&
StringPrivate::utf8()
{
    delete m_utf8;

    WebCore::String a(m_impl);
    WebCore::CString cs = a.utf8();
    if (!cs.isNull()) {
        m_utf8 = new CString(cs.data(), cs.length());
    } else {
        m_utf8 = new CString(0,0);
    }
    return *m_utf8;
}


#define IMPL() (m_private ? (m_private->impl()) : 0)

String::String()
    : m_private()
{
}

String::String(const char* str)
{
    m_private = new StringPrivate();
    m_private->setImpl(WebCore::StringImpl::create(str));
}

String::String(const char* str, unsigned int len)
{
    m_private = new StringPrivate();
    m_private->setImpl(WebCore::StringImpl::create(str, len));
}

String::String(const unsigned short* str)
{
    m_private = new StringPrivate();
    m_private->setImpl(WebCore::StringImpl::create(str, ::wcslen((const wchar_t*)str)));
}

String::String(const unsigned short* str, unsigned int len)
{
    m_private = new StringPrivate();
    m_private->setImpl(WebCore::StringImpl::create(str, len));
}

String::String(const String& str)
{
    m_private = new StringPrivate();

    WebCore::StringImpl* impl = str.impl()->impl();
    if (impl) {
        m_private->setImpl(impl);
    }
}

String::String(StringPrivate* parent)
{
    m_private = parent;
}

String::~String()
{
    delete m_private;
}

String&
String::operator=(const String& orig)
{
    if (this!=&orig) {
        if (!m_private)
            m_private = new StringPrivate();
        m_private->setImpl(orig.m_private->impl());
    }
    return *this;
}

bool
String::operator==(const char* b) const
{
    const WebCore::String a(IMPL());
    return (a==b);
}

bool
String::operator!=(const char* b) const
{
    const WebCore::String a(IMPL());
    return (a!=b);
}


void
String::append(const String& str)
{
    WebCore::String a(IMPL());
    a.append(str);
    m_private->setImpl(a.impl());
}


void
String::append(const char* str)
{
    WebCore::String a(IMPL());
    a.append(str);
    m_private->setImpl(a.impl());
}

void
String::append(const unsigned short* str)
{
    WebCore::String a(IMPL());
    a.append(str);
    m_private->setImpl(a.impl());
}

void 
String::append(const unsigned short* str, unsigned int len)
{
    WebCore::String a(IMPL());
    a.append(str, len);
    m_private->setImpl(a.impl());
}

int
String::find(const String& str)
{
    const WebCore::String a(IMPL());
    return a.find(str);
}

int
String::find(unsigned short ch, int len)
{
    const WebCore::String a(IMPL());
    return a.find(ch, len);
}

String&
String::replace(const unsigned short* a, const unsigned short* b)
{
    WebCore::String s(IMPL());
    s = s.replace(a, b);
    m_private->setImpl(s.impl());
    return *this;
}

String&
String::replace(const unsigned short* a, const String& b)
{
    WebCore::String s(IMPL());
    WebCore::String _b(b.impl());
    s = s.replace(a, _b);
    m_private->setImpl(s.impl());
    return *this;
}

String&
String::replace(const String& a, const String& b)
{
    WebCore::String s(IMPL());
    WebCore::String _a(a.impl());
    WebCore::String _b(b.impl());
    s = s.replace(_a, _b);
    m_private->setImpl(s.impl());
    return *this;
}

String&
String::replace(unsigned index, unsigned len, const String& b)
{
    WebCore::String s(IMPL());
    WebCore::String _b(b.impl());
    s = s.replace(index, len, _b);
    m_private->setImpl(s.impl());
    return *this;
}

void
String::truncate(unsigned int len)
{
    WebCore::String a(IMPL());
    a.truncate(len);
    m_private->setImpl(a.impl());
}

WKC::String
String::substring(unsigned int pos, unsigned int len) const
{
    WebCore::String a(IMPL());
    WebCore::String ret = a.substring(pos, len);
    return String(ret.characters(), ret.length());
}

void
String::remove(unsigned int pos, unsigned int len)
{
    WebCore::String a(IMPL());
    a.remove(pos, len);
    m_private->setImpl(a.impl());
}

void
String::insert(const unsigned short* str, unsigned int pos, unsigned int len)
{
    WebCore::String a(IMPL());
    a.insert(str, pos, len);
    m_private->setImpl(a.impl());
}

WKC::String
String::format(const char * format, ...)
{
    // copied from String.cpp

    va_list args;
    va_start(args, format);

    WTF::Vector<char, 256> buffer;

    // Do the format once to get the length.
#if COMPILER(MSVC)
    int result = _vscprintf(format, args);
#else
    char ch;
    int result = vsnprintf(&ch, 1, format, args);
    va_end(args);
    va_start(args, format);
#endif
    if (result==0) {
        return String("");
    } else if (result<0) {
        return String();
    }

    unsigned len = result;
    buffer.grow(len + 1);
    
    vsnprintf(buffer.data(), buffer.size(), format, args);

    va_end(args);

    return String(buffer.data(), len);
}



const unsigned short*
String::characters() const
{
    if (!m_private || !m_private->impl())
        return 0;
    return IMPL()->characters();
}

unsigned int
String::length() const
{
    if (!m_private || !m_private->impl())
        return 0;
    return IMPL()->length();
}

CString&
String::utf8() const
{
    return m_private->utf8();
}

CString&
String::latin1() const
{
    return m_private->latin1();
}

const unsigned short*
String::charactersWithNullTermination() const
{
    return m_private->charactersWithNullTermination();
}


bool 
String::isNull() const
{
    const WebCore::String a(IMPL());
    return a.isNull();
}

bool 
String::isEmpty() const
{
    const WebCore::String a(IMPL());
    return a.isEmpty();
}

} // namespace

namespace WebCore {
String::String(const WKC::String& str)
    : m_impl(0)
{
    if (str.impl()) {
        m_impl = str.impl()->impl();
    }
}

String::operator ::WKC::String() const
{
    ::WKC::StringPrivate* obj = new ::WKC::StringPrivate();

    obj->setImpl(impl());

    return ::WKC::String(obj);
}

} // namespace
