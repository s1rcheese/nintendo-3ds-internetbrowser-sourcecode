/*
 * Copyright (C) 2003, 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2004 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2006 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
 * Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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

#ifndef WKCHELPERSENUMSBSDS_H
#define WKCHELPERSENUMSBSDS_H

#include <limits.h>

namespace WKC {

    // from WebCore/page/Console.h
    enum MessageSource {
        HTMLMessageSource,
        WMLMessageSource,
        XMLMessageSource,
        JSMessageSource,
        CSSMessageSource,
        OtherMessageSource
    };

    enum MessageType {
        LogMessageType,
        ObjectMessageType,
        TraceMessageType,
        StartGroupMessageType,
        EndGroupMessageType,
        AssertMessageType
    };

    enum MessageLevel {
        TipMessageLevel,
        LogMessageLevel,
        WarningMessageLevel,
        ErrorMessageLevel,
        DebugMessageLevel
    };

    // from platform/text/TextDirection.h
    enum TextDirection { RTL, LTR };

    // from page/FocusDirection.h
    enum WKCFocusDirection {
        FocusDirectionNone = 0,
        FocusDirectionForward,
        FocusDirectionBackward,
        FocusDirectionUp,
        FocusDirectionDown,
        FocusDirectionLeft,
        FocusDirectionRight
    };

    // from page/DragActions.h
    typedef enum {
        DragDestinationActionNone    = 0,
        DragDestinationActionDHTML   = 1,
        DragDestinationActionEdit    = 2,
        DragDestinationActionLoad    = 4,
        DragDestinationActionAny     = UINT_MAX
    } DragDestinationAction;
    
    typedef enum {
        DragSourceActionNone         = 0,
        DragSourceActionDHTML        = 1,
        DragSourceActionImage        = 2,
        DragSourceActionLink         = 4,
        DragSourceActionSelection    = 8,
        DragSourceActionAny          = UINT_MAX
    } DragSourceAction;
    
    typedef enum {
        DragOperationNone    = 0,
        DragOperationCopy    = 1,
        DragOperationLink    = 2,
        DragOperationGeneric = 4,
        DragOperationPrivate = 8,
        DragOperationMove    = 16,
        DragOperationDelete  = 32,
        DragOperationEvery   = UINT_MAX
    } DragOperation;

    // from platform/ScrollTypes.h
    enum ScrollbarOrientation { HorizontalScrollbar, VerticalScrollbar };
    enum ScrollbarControlSize { RegularScrollbar, SmallScrollbar };

    // from editing/EditorInsertAction.h
    enum EditorInsertAction {
        EditorInsertActionTyped,
        EditorInsertActionPasted,
        EditorInsertActionDropped,
    };

    // from TextAffinity.h
    enum EAffinity { UPSTREAM = 0, DOWNSTREAM = 1 };

    // from loader/FrameLoaderTypes.h
    enum PolicyAction {
        PolicyUse,
        PolicyDownload,
        PolicyIgnore,
    };
    enum ObjectContentType {
        ObjectContentNone,
        ObjectContentImage,
        ObjectContentFrame,
        ObjectContentNetscapePlugin,
        ObjectContentOtherPlugin
    };
    enum FrameLoadType {
        FrameLoadTypeStandard,
        FrameLoadTypeBack,
        FrameLoadTypeForward,
        FrameLoadTypeIndexedBackForward, // a multi-item hop in the backforward list
        FrameLoadTypeReload,
        // Skipped value: 'FrameLoadTypeReloadAllowingStaleData', still present in mac/win public API. Ready to be reused
        FrameLoadTypeSame = FrameLoadTypeReload + 2, // user loads same URL again (but not reload button)
        FrameLoadTypeRedirectWithLockedBackForwardList, // FIXME: Merge "lockBackForwardList", "lockHistory", "quickRedirect" and "clientRedirect" into a single concept of redirect.
        FrameLoadTypeReplace,
        FrameLoadTypeReloadFromOrigin,
        FrameLoadTypeBackWMLDeckNotAccessible
    };

    // from ProtectionSpace.h
    enum ProtectionSpaceAuthenticationScheme {
        ProtectionSpaceAuthenticationSchemeDefault = 1,
        ProtectionSpaceAuthenticationSchemeHTTPBasic = 2,
        ProtectionSpaceAuthenticationSchemeHTTPDigest = 3,
        ProtectionSpaceAuthenticationSchemeHTMLForm = 4,
        ProtectionSpaceAuthenticationSchemeNTLM = 5,
        ProtectionSpaceAuthenticationSchemeNegotiate = 6,
    };

    // from Credential.h
    enum CredentialPersistence {
        CredentialPersistenceNone,
        CredentialPersistenceForSession,
        CredentialPersistencePermanent
    };
} // namespace

#endif // WKCHELPERSENUMSBSDS_H
