/*
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "FileChooser.h"

#include "CString.h"
#include "FileSystem.h"
#include "Icon.h"
#include "LocalizedStrings.h"
#include "StringTruncator.h"

#include "NotImplemented.h"

namespace {
unsigned short gFilenameBuffer[MAX_PATH];
}

namespace WebCore {
typedef void (*ResolveFilenameForDisplayProc)(const unsigned short* path, const int path_len, unsigned short* out_path, int* out_path_len, const int path_maxlen);
static WebCore::ResolveFilenameForDisplayProc gResolveFilenameForDisplayProc = 0;

String FileChooser::basenameForWidth(const Font& font, int width) const
{
    String str;

    if (1 < m_filenames.size()) {
        str = multipleFileUploadText(m_filenames.size());
        return StringTruncator::rightTruncate(str, width, font);
    } else {
        if (gResolveFilenameForDisplayProc != 0) {
            String file;
            if (m_filenames.isEmpty()) {
                file = String();
            } else {
                file = m_filenames.first();
            }
            int len = 0;
            int cur_len = file.length();
            (*gResolveFilenameForDisplayProc)(file.charactersWithNullTermination(), cur_len, gFilenameBuffer, &len, MAX_PATH);
            str = String(gFilenameBuffer, len);
        } else {
            if (m_filenames.isEmpty()) {
                str = fileButtonNoFileSelectedLabel();
            } else {
                str = m_filenames.first();
            }
        }
        return StringTruncator::centerTruncate(str, width, font);
    }
}

void
FileChooser_SetResolveFilenameForDisplayProc(WebCore::ResolveFilenameForDisplayProc proc)
{
    gResolveFilenameForDisplayProc = proc;
}

}
