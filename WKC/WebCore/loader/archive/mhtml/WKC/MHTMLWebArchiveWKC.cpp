/*
 *  Copyright (c) 2010 ACCESS CO., LTD.
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

#include "config.h"
#include "mhtml/MHTMLWebArchive.h"
#include "CString.h"
#include "Base64.h"

#if ENABLE(WKC_PAGESAVE_MHTML)

#define PAGESAVE_BOUNDARY_STR  "--=WKC-PageSave-Boundary"
#define PAGESAVE_BOUNDARY_LEN  24
#define PAGESAVE_CRLF_LEN 2
#define min(a,b) ((a)<(b) ? (a) : (b))
//#define PAGESAVE_USE_BASE64_PROCESSING

enum
{
    PARSE_STATE_PARSE_HEADER = 0,
    PARSE_STATE_READ_CONTENT,
    PARSE_STATE_ADD_RESOURCE,
    PARSE_STATE_END
};
enum
{
    HEADER_STATE_NAME = 0,
    HEADER_STATE_VALUE,
    HEADER_STATE_VALUE_FOLDING,
    HEADER_STATE_END,
    HEADER_STATES
};

namespace WebCore {

static unsigned
makeMessageHeader(String type, bool multipart, Vector<char>* result)
{
    unsigned len;

    if (result) {
        result->append("Mime-Version: 1.0\015\012", 19);
	    if (multipart) {
            result->append("Content-Type: multipart/related; \015\012", 35);
            result->append("\tboundary=\"", 11);
            result->append(PAGESAVE_BOUNDARY_STR, PAGESAVE_BOUNDARY_LEN);
            if (!type.isEmpty()) {
                result->append("\";\015\012", 4);
                result->append("\ttype=\"", 7);
                result->append(type.characters(), type.length());
		    }
            result->append("\"\015\012\015\012", 5);
            result->append("This is a multi-part message in MIME format.\015\012", 46);
	    }
        len = result->size();
    } else {
        if (multipart) {
            len = PAGESAVE_BOUNDARY_LEN + 116;
            if (!type.isEmpty())
                len += type.length() + 11;
        } else {
            len = 19;
        }
    }

    return len;
}

static unsigned
makePartHeader(ArchiveResource* resource, Vector<char>* result)
{
    const String& url = resource->url();

    if (url.isEmpty()) {
        return 0;
    }
    unsigned len = 0;
    
    if (result) {
        //content-type, charset
        if (!resource->mimeType().isEmpty()) {
            result->append("Content-Type: ", 14);
            result->append(resource->mimeType().characters(), resource->mimeType().length());
            if (!resource->textEncoding().isEmpty()) {
                result->append(";\015\012", 3);
                result->append("\tcharset=\"", 10);
                result->append(resource->textEncoding().characters(), resource->textEncoding().length());
                result->append("\"", 1);
            }
            result->append("\015\012", 2);
        }

#ifdef PAGESAVE_USE_BASE64_PROCESSING
        //encoding
        result->append("Content-Transfer-Encoding: base64", 33);
        result->append("\015\012", 2);
#endif //PAGESAVE_USE_BASE64_PROCESSING

        //url
        result->append("Content-Location: ", 18);
        result->append(url.characters(), url.length());
        result->append("\015\012", 2);

        //delimiter
        result->append("\015\012", 2);
        len = result->size();
    } else {
        //content-type, charset
        if (!resource->mimeType().isEmpty()) {
            len += resource->mimeType().length() + 16;
            if (!resource->textEncoding().isEmpty())
                len += resource->textEncoding().length() + 14;
        }
#ifdef PAGESAVE_USE_BASE64_PROCESSING
        //encoding
        len += 35;
#endif //PAGESAVE_USE_BASE64_PROCESSING

        //url
        len += url.length() + 2;
        len += 18;
        //delimiter
        len += 2;
    }

    return len;
}

static unsigned
makeBoundary(bool end, Vector<char>* result)
{
    unsigned len;

    if (result) {
        result->append("\015\012--", 4);
        result->append(PAGESAVE_BOUNDARY_STR, PAGESAVE_BOUNDARY_LEN);
	    if (end)
		    result->append("--", 2);
	    result->append("\015\012", 2);
        len = result->size();
    } else {
        if (end)
            len = PAGESAVE_BOUNDARY_LEN + 8;
        else
            len = PAGESAVE_BOUNDARY_LEN + 6;
    }

    return len;
}

static int
isEndOfLine(char* buf,int bufLen)
{
    if (!buf || bufLen < 1) return 0;

    if (buf[0] == '\015') {
        if ((bufLen >= 2) && (buf[1] == '\012'))
            return 2;
        else
            return 1;
    } else if (buf[0] == '\012')
        return 1;

    return 0;
}

static int
calcLineLength(char* buf, int bufLen, bool* isEol)
{
    char* p;
    int rest_len,len;
    bool ret = false;

    ASSERT(buf);

    p = buf;
    rest_len = bufLen;
    while(rest_len > 0) {
        len = isEndOfLine(p, rest_len);
        if (len == 2 || len == 1) {
            p += len;
            rest_len -= len;
            ret = true;
            break;
        }
        p++;
        rest_len--;
    }
    if (isEol)
        *isEol = ret;
    return p - buf;
}

#ifdef PAGESAVE_USE_BASE64_PROCESSING
static int
calcEndLength(char* buf, int bufLen)
{
    if (!buf || bufLen < 1) return 0;
    char* p = buf + bufLen - 2;

    if (p[1] == '\012') {
        if ((bufLen >= 2) && (p[0] == '\015'))//CRLF
            return 2;
        else//LF
            return 1;
    } else if (p[1] == '\015')//CR
        return 1;

    return 0;    
}
#endif

static bool
isWSP(char* buf,unsigned bufLen)
{
	if (!buf || bufLen < 1) return false;
	if ((*buf == ' ') || (*buf == '\t'))
        return true;
	return false;
}

static bool
isAlphaNum(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

static int
parseHeader(char* buf, int bufLen, char** name, unsigned& nameLen, char** value, unsigned& valueLen, int& status)
{
    char* p = buf;
    char* pHBegin = buf;
    char* pVBegin = buf;
    int rest_len = bufLen, len;

    while (rest_len > 0) {
        switch (status) {
        case HEADER_STATE_NAME:
            if (p == pHBegin) {
                len = isEndOfLine(p, rest_len);
                if (len > 0) {
                    status = HEADER_STATE_END;
                    return len;
                } else if (isWSP(p, rest_len)) {
                    p++;
                    rest_len--;
                    pVBegin = p;
                    status = HEADER_STATE_VALUE_FOLDING;
                    break;
                }
            }
            if (isAlphaNum(*p) || (*p == '-')) {
                p++;
                rest_len--;
            } else if (*p == ':') {
                *name = pHBegin;
                nameLen = p - pHBegin;
                p++;
                rest_len--;
                pVBegin = p;
                status = HEADER_STATE_VALUE;
            } else {
                return -1;
            }
            break;
        case HEADER_STATE_VALUE:
        case HEADER_STATE_VALUE_FOLDING:
            len = isEndOfLine(p, rest_len);
            if (len > 0) {
                *value = pVBegin;
                valueLen = p - pVBegin;
                p += len;
                rest_len -= len;
                status = HEADER_STATE_NAME;
                return p - buf;
            } else {
                p++;
                rest_len--;
            }
            break;
        case HEADER_STATE_END:
            return 0;
        default:
            ASSERT(false);
            return -1;
        }
    }
	switch(status){
    case HEADER_STATE_NAME:
        return pHBegin - buf;
    case HEADER_STATE_VALUE:
    case HEADER_STATE_VALUE_FOLDING:
        *value = pVBegin;
        valueLen = p - pVBegin;
        return p - buf;
    case HEADER_STATE_END:
        return 0;
    default:
        return -1;
    }
}

static String
getQuotationString(String sValue)
{
    int pos_begin, pos_end;
    
    pos_begin = sValue.find('"', 0);
    if (pos_begin == -1)
        return String();
    pos_end = sValue.find('"', pos_begin + 1);
    if (pos_end == -1)
        return String();

    return String(sValue.characters() + pos_begin + 1, pos_end - pos_begin - 1);
}

static String
stripWhiteAndSemicolon(String sValue)
{
    String tmp;

    tmp = sValue.stripWhiteSpace();
    if (tmp.endsWith(";")) {
        if (tmp.length()>1) {
            return String(tmp.characters(), tmp.length() - 1);
        } else {
            return String();
        }
    } else
        return tmp;
}

static void
getHeaderInfo(String& boundary, String& type, String& location, String& charset, String sName, String sValue)
{
    if (!sName.isEmpty()) {
        if (equalIgnoringCase(sName.stripWhiteSpace(), "Content-Type")) {
            type = stripWhiteAndSemicolon(sValue);
        } else if (equalIgnoringCase(sName.stripWhiteSpace(), "Content-Location")) {
            location = sValue.stripWhiteSpace();
        }
    } else {
        if (sValue.find("boundary=", 0, false) != -1) {
            if (!boundary)
                boundary = getQuotationString(sValue);
        } else if (sValue.find("charset=", 0, false) != -1)
            charset = getQuotationString(sValue);
    }
}

static bool
isEndBoundary(char *buf, int  bufLen, String boundary)
{
    char *p;
    int rest_len;

    if (boundary.isEmpty())
        return false;
    if (bufLen < PAGESAVE_CRLF_LEN)
        return false;

    bufLen -= PAGESAVE_CRLF_LEN;

    if (bufLen < (boundary.length() + 2)) {
        return false;
    } else if (bufLen < (boundary.length() + 4)) {
        // boundary--...
        p = buf;
        rest_len = bufLen;
    } else {
        // --boundary--...
        if (!(buf[0] == '-' && buf[1] == '-'))
            return false;
        p = buf + 2;
        rest_len = bufLen - 2;
    }

    String tmp_s;
    tmp_s = String(p, rest_len);
    if (tmp_s.find(boundary, 0, true) != 0)
        return false;
    if (!(p[boundary.length()] == '-' && p[boundary.length() + 1] == '-'))
        return false;

    return true;
}

static bool
isBoundary(char *buf, int  bufLen, String boundary)
{
    if (boundary.isEmpty())
        return false;
    if (bufLen < (boundary.length() + 2))
        return false;
    if (!(buf[0] == '-' && buf[1] == '-'))
        return false;

    String tmp_s;
    tmp_s = String(buf + 2, bufLen - 2);
    if (tmp_s.find(boundary, 0, true) != 0)
        return false;

    return true;
}

bool MHTMLWebArchive::parseRawDataRepresentation(SharedBuffer* data)
{
    char* p = (char*)data->data();

    bool is_multipart = false;
#ifdef PAGESAVE_USE_BASE64_PROCESSING
    bool has_encoding = false;
#endif //PAGESAVE_USE_BASE64_PROCESSING
    int rest_len = data->size();
    int len, header_status = HEADER_STATE_NAME;
    String boundary, type, location, charset;
    Vector<char> raw_data;

    for (;;) {
        switch (m_status) {
        case PARSE_STATE_PARSE_HEADER:
        {
            char *name = 0, *value = 0;
            unsigned name_len, value_len;
            len = parseHeader(p, rest_len, &name, name_len, &value, value_len, header_status);
            if (len < 0) {
                goto error;
            } else if (len == 0) {
                if (is_multipart && boundary.isEmpty())
                    goto error;
                m_status = PARSE_STATE_READ_CONTENT;
            } else {
                p += len;
                rest_len -= len;
                String sName(name, name_len);
                String sValue(value, value_len);
                getHeaderInfo(boundary, type, location, charset, sName, sValue);
                if (equalIgnoringCase(sName.stripWhiteSpace(), "Content-Type")) {
                    if (sValue.find("multipart/related", 0, false) != -1)
                        is_multipart = true;
#ifdef PAGESAVE_USE_BASE64_PROCESSING
                } else if (equalIgnoringCase(sName.stripWhiteSpace(), "Content-Transfer-Encoding")) {
                    // Currently we only support base64 encoding. See also makePartHeader
                    if (equalIgnoringCase(sValue.stripWhiteSpace(), "base64"))
                        has_encoding = true;
#endif //PAGESAVE_USE_BASE64_PROCESSING
                }
            }
            break;
        }
        case PARSE_STATE_READ_CONTENT:
            len = calcLineLength(p, rest_len, 0);
            if (isEndBoundary(p, len, boundary) || isBoundary(p, len, boundary)) {
                p += len;
                rest_len -= len;
                m_status = PARSE_STATE_ADD_RESOURCE;
                break;
            }
#ifdef PAGESAVE_USE_BASE64_PROCESSING
            // ignore data if 
            // 1.encoding type is not found or not supported
            // 2.current field is main header 
            // 3.Content-Location is not found
            if (!has_encoding 
                || equalIgnoringCase(type, "multipart/related")
                || location.isEmpty()) {
                ;
            } else {
                // append one line
                int end_len = calcEndLength(p, len);
                raw_data.append(p, len - end_len);
            }
#else
            // ignore data if 
            // 1.current field is main header 
            // 2.Content-Location is not found
            if (equalIgnoringCase(type, "multipart/related")
                || location.isEmpty()) {
                ;
            } else {
                // append one line
                raw_data.append(p, len);
            }
#endif //PAGESAVE_USE_BASE64_PROCESSING
            p += len;
            rest_len -= len;
            if (rest_len > 0)                
                break;
            else {
                m_status = PARSE_STATE_ADD_RESOURCE;
                //fall through
            }
        case PARSE_STATE_ADD_RESOURCE:          
            if (!raw_data.isEmpty()) {
                if (!m_mainResourceType) {
                    if (!type.isEmpty())
                        m_mainResourceType = String(type.characters(), type.length());
                    else {
                        type = String("text/html", 9);
                        m_mainResourceType = String("text/html", 9);
                    }
                }
#ifdef PAGESAVE_USE_BASE64_PROCESSING
                Vector<char> content_data;
                if (!base64Decode(raw_data, content_data))
                    goto error;
                RefPtr<SharedBuffer> sharedBuffer = SharedBuffer::create(content_data.data(), content_data.size());
#else
                int rawdatasize = raw_data.size();
                if (rawdatasize>2 && is_multipart && raw_data[rawdatasize-2]==0x0d && raw_data[rawdatasize-1]==0x0a) {
                    rawdatasize-=2; /* exclude the last \015\012 which is added in makeBoundary */
                }
                RefPtr<SharedBuffer> sharedBuffer = SharedBuffer::create(raw_data.data(), rawdatasize);
#endif //PAGESAVE_USE_BASE64_PROCESSING
                ResourceResponse response;
                response.setHTTPStatusCode(200);
                response.setMimeType(type);
                response.setURL(KURL(ParsedURLString, location));
                response.setTextEncodingName(charset);
                addMimeResource(sharedBuffer.release(), response);
#ifdef PAGESAVE_USE_BASE64_PROCESSING
                response.setExpectedContentLength(content_data.size());
#else
                response.setExpectedContentLength(raw_data.size());
#endif //PAGESAVE_USE_BASE64_PROCESSING
                raw_data.remove(0, raw_data.size());
            }
            // clear tmp data
            type.remove(0, type.length());
            location.remove(0, location.length());
            charset.remove(0, charset.length());

            if (rest_len > 0) {
                header_status = HEADER_STATE_NAME;
#ifdef PAGESAVE_USE_BASE64_PROCESSING
                has_encoding = false;
#endif //PAGESAVE_USE_BASE64_PROCESSING
                m_status = PARSE_STATE_PARSE_HEADER;
            } else {
                if (!m_mainResourceType)
                    goto error;//no valid content
                else
                    m_status = PARSE_STATE_END;
            }
            break;
        case PARSE_STATE_END:
            return true;
        default:
            ASSERT(false);
            goto error;
        }
    }
error:
    return false;
}

void MHTMLWebArchive::serializeStart()
{
    return;
}

bool MHTMLWebArchive::serializeResource(ArchiveResource* resource)
{
    char* p = (char *)m_buffer + m_buffer_off;
    unsigned int len, remain, nwrite;
    bool multipart = m_items > 1;

    switch (m_status) {
    case SERIALIZE_STATE_MAKE_MESSAGE_HEADER:
    {
        Vector<char> header;
        ASSERT(m_buffer_off == 0);
        ASSERT(m_data == 0);
        len = makeMessageHeader(resource->mimeType(), multipart, &header);
        if (len == 0)
            goto error;
        m_data = new char[header.size()];
        if (!m_data)
            goto error;
        memcpy(m_data, header.data(), header.size());
        m_data_len = header.size();
        m_status = SERIALIZE_STATE_WRITE_MESSAGE_HEADER;
        m_data_off = 0;
    }
        //fall through
    case SERIALIZE_STATE_WRITE_MESSAGE_HEADER:
        ASSERT(m_data != 0);
        len = m_data_len;
        remain = len - m_data_off;
        nwrite = min(m_buffer_len, remain);
        memcpy(p, m_data + m_data_off, nwrite);
        p += nwrite;
        m_buffer_len -= nwrite;
        m_data_off += nwrite;
        m_buffer_off += nwrite;
        if (m_data_off != len)
            break;
        delete [] m_data; m_data = 0; m_data_len = 0;
        m_status = SERIALIZE_STATE_MAKE_BOUNDARY;
        m_data_off = 0;
        if (m_buffer_len == 0)
            break;
        //fall through
    case SERIALIZE_STATE_MAKE_BOUNDARY:
make_boundary:
        ASSERT(m_data == 0);
        if (multipart) {
            Vector<char> boundary;
            len = makeBoundary(m_cur_item > m_items, &boundary);
            if (len == 0)
                goto error;
            m_data = new char[boundary.size()];
            if (!m_data)
                goto error;
            memcpy(m_data, boundary.data(), boundary.size());
            m_data_len = boundary.size();
        }
        m_status = SERIALIZE_STATE_WRITE_BOUNDARY;
        m_data_off = 0;
        //fall through
    case SERIALIZE_STATE_WRITE_BOUNDARY:
        if (m_data) {
            len = m_data_len;
            remain = len - m_data_off;
            nwrite = min(m_buffer_len, remain);
            memcpy(p, m_data + m_data_off, nwrite);
            p += nwrite;
            m_buffer_len -= nwrite;
            m_data_off += nwrite;
            m_buffer_off += nwrite;
            if (m_data_off != len)
                break;
            delete [] m_data; m_data = 0; m_data_len = 0;
        }
        if (m_cur_item > m_items) {
            m_status = SERIALIZE_STATE_END;
            break;
        }
        m_status = SERIALIZE_STATE_MAKE_PART_HEADER;
        m_data_off = 0;
        if (m_buffer_len == 0)
            break;
        //fall through
    case SERIALIZE_STATE_MAKE_PART_HEADER:
    {
        Vector<char> header;
        ASSERT(m_data == 0);
        len = makePartHeader(resource, &header);
        if (len == 0)
            goto error;
        m_data = new char[header.size()];
        if (!m_data)
            goto error;
        memcpy(m_data, header.data(), header.size());
        m_data_len = header.size();
        m_status = SERIALIZE_STATE_WRITE_PART_HEADER;
        m_data_off = 0;
    }
        //fall through
    case SERIALIZE_STATE_WRITE_PART_HEADER:
        len = m_data_len;
        remain = len - m_data_off;
        nwrite = min(m_buffer_len, remain);
        memcpy(p, m_data + m_data_off, nwrite);
        p += nwrite;
        m_buffer_len -= nwrite;
        m_data_off += nwrite;
        m_buffer_off += nwrite;
        if (m_data_off != len)
            break;
        delete [] m_data; m_data = 0; m_data_len = 0;
        m_status = SERIALIZE_STATE_MAKE_CONTENT;
        m_data_off = 0;
        if (m_buffer_len == 0)
            break;
        //fall through
    case SERIALIZE_STATE_MAKE_CONTENT:
    {
#ifdef PAGESAVE_USE_BASE64_PROCESSING
        Vector<char> base64_data;
        ASSERT(m_data == 0);
        base64Encode(resource->data()->buffer(), base64_data, true);
        if (base64_data.isEmpty())
            goto error;
        m_data = new char[base64_data.size()];
        if (!m_data)
            goto error;
        memcpy(m_data, base64_data.data(), base64_data.size());
        m_data_len = base64_data.size();
#else
        ASSERT(m_data == 0);
        if (resource->data()->buffer().isEmpty())
            goto error;
        m_data = (char *)resource->data()->buffer().data();
        if (!m_data)
            goto error;
        m_data_len = resource->data()->buffer().size();
#endif //PAGESAVE_USE_BASE64_PROCESSING
        m_status = SERIALIZE_STATE_WRITE_CONTENT;
        m_data_off = 0;
    }
        //fall through
    case SERIALIZE_STATE_WRITE_CONTENT:
        len = m_data_len;
        remain = len - m_data_off;
        nwrite = min(m_buffer_len, remain);
        memcpy(p, m_data + m_data_off, nwrite);
        p += nwrite;
        m_buffer_len -= nwrite;
        m_data_off += nwrite;
        m_buffer_off += nwrite;
        if (m_data_off != len)
            break;
#ifdef PAGESAVE_USE_BASE64_PROCESSING
        delete [] m_data;
#endif //PAGESAVE_USE_BASE64_PROCESSING
        m_data = 0; m_data_len = 0;
        m_status = SERIALIZE_STATE_MAKE_BOUNDARY;
        m_data_off = 0;
        if (m_buffer_len == 0)
            break;
        m_cur_item++;
        if (m_cur_item > m_items)
            goto make_boundary;
        //return to get next content
        return true;
    case SERIALIZE_STATE_END:
        ASSERT(m_buffer_off == 0);
        return true;
    case SERIALIZE_STATE_ERROR:
        goto error;
    default:
        ASSERT(false);
        break;
    }
    return false;
error:
    m_status = SERIALIZE_STATE_ERROR;
    return false;
}

PassRefPtr<SharedBuffer> MHTMLWebArchive::serializeEnd()
{
    return 0;
}

void
MHTMLWebArchive::caculateItems(MHTMLWebArchive* rootArchive, HashSet<String>& uniqueSubresources)
{
    if (mainResource()->data()->isEmpty()
        || uniqueSubresources.contains(mainResource()->url().string()))
        return;

    rootArchive->m_items++;
    uniqueSubresources.add(mainResource()->url().string());

    const Vector<RefPtr<ArchiveResource> >& resources(subresources());
    for (unsigned i = 0; i < resources.size(); ++i) {
        if (!resources[i].get()->data()->isEmpty()
            && !uniqueSubresources.contains(resources[i].get()->url().string())) {
            rootArchive->m_items++;
            uniqueSubresources.add(resources[i].get()->url().string());
        }
    }

    const Vector<RefPtr<Archive> >& subframes(subframeArchives());
    for (unsigned i = 0; i < subframes.size(); ++i) {
        static_cast<MHTMLWebArchive*>(subframes[i].get())->caculateItems(rootArchive, uniqueSubresources);
    }
}

int
MHTMLWebArchive::getBufferSize(MHTMLWebArchive* rootArchive, HashSet<String>& uniqueSubresources)
{
    int len = 0, size = 0;
    bool multipart = rootArchive->m_items > 1;

    len = makeMessageHeader(mainResource()->mimeType(), multipart, 0);
    if (len == 0)
        goto error;
    size += len;

    len = getItemsSize(rootArchive, uniqueSubresources);
    if (len <= 0)
        goto error;
    size += len;

    if (multipart) {
        //end boundary
        len = makeBoundary(true, 0);
        if (len == 0)
            goto error;
        size += len;
    }

    return size;
error:
    return -1;
}

static int
getOneItemSize(ArchiveResource* resource, bool multipart)
{
    int len, size = 0;

    if (multipart) {
        //begin boundary
        len = makeBoundary(false, 0);
        if (len == 0)
            return -1;
        size += len;
    }
    len = makePartHeader(resource, 0);
    if (len == 0)
        return -1;
    size += len;

#ifdef PAGESAVE_USE_BASE64_PROCESSING
    Vector<char> base64_data;
    base64Encode(resource->data()->buffer(), base64_data, true);
    size += base64_data.size();
#else
    size += resource->data()->buffer().size();
#endif //PAGESAVE_USE_BASE64_PROCESSING

    return size;
}

int
MHTMLWebArchive::getItemsSize(MHTMLWebArchive* rootArchive, HashSet<String>& uniqueSubresources)
{
    if (uniqueSubresources.contains(mainResource()->url().string()))
        return 0;

    bool multipart = rootArchive->m_items > 1;
    int len, size = 0;
    if (!mainResource()->data()->isEmpty()) {
        len = getOneItemSize(mainResource(), multipart);
        if (len == -1)
            return -1;
        size += len;
    }
    uniqueSubresources.add(mainResource()->url().string());

    const Vector<RefPtr<ArchiveResource> >& resources(subresources());

    for (unsigned i = 0; i < resources.size(); ++i) {
        if (!uniqueSubresources.contains(resources[i].get()->url().string())) {
            if (!resources[i]->data()->isEmpty()) {
                len = getOneItemSize(resources[i].get(), multipart);
                if (len == -1)
                    return -1;
                size += len;
            }
            uniqueSubresources.add(resources[i].get()->url().string());
        }
    }

    const Vector<RefPtr<Archive> >& subframes(subframeArchives());
    for (unsigned i = 0; i < subframes.size(); ++i) {
        len = static_cast<MHTMLWebArchive*>(subframes[i].get())->getItemsSize(rootArchive, uniqueSubresources);
        if (len == -1)
            return -1;
        size += len;
    }

    return size;
}

}

#endif// ENABLE(WKC_PAGESAVE_MHTML)
