/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef Profiler_h
#define Profiler_h

#include "Profile.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

namespace JSC {

    class ExecState;
    class JSGlobalData;
    class JSObject;
    class JSValue;
    class ProfileGenerator;
    class UString;
    struct CallIdentifier;    

    class Profiler : public FastAllocBase {
    public:
        static Profiler** enabledProfilerReference()
        {
            return &s_sharedEnabledProfilerReference;
        }

        static Profiler* profiler(); 
        static CallIdentifier createCallIdentifier(ExecState* exec, JSValue, const UString& sourceURL, int lineNumber);

        void startProfiling(ExecState*, const UString& title);
        PassRefPtr<Profile> stopProfiling(ExecState*, const UString& title);

        void willExecute(ExecState*, JSValue function);
        void willExecute(ExecState*, const UString& sourceURL, int startingLineNumber);
        void didExecute(ExecState*, JSValue function);
        void didExecute(ExecState*, const UString& sourceURL, int startingLineNumber);

        const Vector<RefPtr<ProfileGenerator> >& currentProfiles() { return m_currentProfiles; };

#if PLATFORM(WKC)
        static void deleteSharedInstance();
        static void resetVariables();
#endif

    private:
        Vector<RefPtr<ProfileGenerator> > m_currentProfiles;
        static Profiler* s_sharedProfiler;
        static Profiler* s_sharedEnabledProfilerReference;
    };

} // namespace JSC

#endif // Profiler_h
