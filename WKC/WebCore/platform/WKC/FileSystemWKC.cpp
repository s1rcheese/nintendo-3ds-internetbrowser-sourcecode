/*
 * Copyright (C) 2007, 2009 Holger Hans Peter Freyther
 * Copyright (C) 2008 Collabora, Ltd.
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
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
#include "FileSystem.h"

#include "PlatformString.h"
#include "CString.h"

#include "NotImplemented.h"

#ifdef __WKC_IMPLICIT_INCLUDE_SYSSTAT
#include <sys/stat.h>
#endif

namespace WebCore {

String filenameToString(const char* filename)
{
    notImplemented();
    return String();
}

char* filenameFromString(const String& string)
{
    notImplemented();
    return NULL;
}

// Converts a string to something suitable to be displayed to the user.
String filenameForDisplay(const String& string)
{
    notImplemented();
    return string;
}

bool fileExists(const String& path)
{
    notImplemented();
    return false;
}

bool deleteFile(const String& path)
{
    notImplemented();
    return false;
}

bool deleteEmptyDirectory(const String& path)
{
    notImplemented();
    return false;
}

bool getFileSize(const String& path, long long& resultSize)
{
    if (path.isNull() || path.isEmpty()) {
        return false;
    }

    struct stat st;
    int err;

    FILE* fp = wkcFileFOpenPeer(path.utf8().data(), "rb");
    if (!fp)
        return false;

    err = wkcFileFStatPeer(wkcFileNoPeer(fp), &st);
    wkcFileFClosePeer(fp);
    if (err == -1) {
        return false;
    }
    resultSize = st.st_size;

    return true;
}

bool getFileModificationTime(const String& path, time_t& modifiedTime)
{
    notImplemented();
    return false;
}

String pathByAppendingComponent(const String& path, const String& component)
{
    notImplemented();
    return String();
}

bool makeAllDirectories(const String& path)
{
    notImplemented();
    return false;
}

String homeDirectoryPath()
{
    return String();
}

String pathGetFileName(const String& pathName)
{
    if (pathName.isEmpty() || pathName.isNull())
        return String();

    int slash_pos = pathName.reverseFind('/');
    int backslash_pos = pathName.reverseFind('\\');
    int pos = (backslash_pos > slash_pos) ? backslash_pos : slash_pos;
    if (pos == -1) {
        return String();
    }
    pos++;
    if (pathName.length() <= pos) {
        return String();
    }
    String str(pathName.substring(pos));
    return str;
}

String directoryName(const String& path)
{
    notImplemented();
    return String();
}

Vector<String> listDirectory(const String& path, const String& filter)
{
    notImplemented();
    Vector<String> entries;
    return entries;
}

CString openTemporaryFile(const char* prefix, PlatformFileHandle& handle)
{
    notImplemented();
    return CString();
}

void closeFile(PlatformFileHandle& handle)
{
    notImplemented();
}

int writeToFile(PlatformFileHandle handle, const char* data, int length)
{
    notImplemented();
    return -1;
}

bool unloadModule(PlatformModule module)
{
    notImplemented();
    return false;
}
}
