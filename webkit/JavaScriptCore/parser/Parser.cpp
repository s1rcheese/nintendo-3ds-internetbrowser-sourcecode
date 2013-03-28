/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "Parser.h"

#include "Debugger.h"
#include "Lexer.h"
#include <wtf/HashSet.h>
#include <wtf/Vector.h>
#include <memory>

#if PLATFORM(WKC)
#ifndef IS_STACK_OVERFLOW
#define IS_STACK_OVERFLOW(margin)   JSC::isStackOverflow((margin), __FILE__, __LINE__, WTF_PRETTY_FUNCTION)
#endif
#endif // PLATFORM(WKC)

using std::auto_ptr;

#ifndef yyparse
extern int jscyyparse(void*);
#endif

namespace JSC {

#if PLATFORM(WKC)
bool isStackOverflow(unsigned int margin, const char* file, int line, const char* function)
{
    unsigned int consumption;
    void* stack_top;
    bool stack_overflow;

    stack_overflow = wkcMemoryIsStackOverflowPeer(margin, &consumption, &stack_top);
    if (stack_overflow) {
        wkcMemoryNotifyStackOverflowPeer(false, margin, consumption, stack_top, file, line, function);
    }
    return stack_overflow;
}
#endif

void Parser::parse(JSGlobalData* globalData, int* errLine, UString* errMsg)
{
    m_sourceElements = 0;

    int defaultErrLine;
    UString defaultErrMsg;

    if (!errLine)
        errLine = &defaultErrLine;
    if (!errMsg)
        errMsg = &defaultErrMsg;

    *errLine = -1;
    *errMsg = 0;

    Lexer& lexer = *globalData->lexer;
    lexer.setCode(*m_source, m_arena);

#if PLATFORM(WKC)
    // An error may be seen in lexer.setCode() mainly because of memory allocation failure.
    bool lexError = lexer.sawError();
    int parseError = 0;
    if (!lexError && !IS_STACK_OVERFLOW(WKC_STACK_MARGIN_JSC_YY_PARSE)) {
        // Executing jscyyparse() requires that no error has occured in lexer.
        parseError = jscyyparse(globalData);
        lexError = lexer.sawError();
    }
#else
    int parseError = jscyyparse(globalData);
    bool lexError = lexer.sawError();
#endif
    int lineNumber = lexer.lineNumber();
    lexer.clear();

    if (parseError || lexError) {
        *errLine = lineNumber;
        *errMsg = "Parse error";
        m_sourceElements = 0;
    }
}

void Parser::didFinishParsing(SourceElements* sourceElements, ParserArenaData<DeclarationStacks::VarStack>* varStack, 
                              ParserArenaData<DeclarationStacks::FunctionStack>* funcStack, CodeFeatures features, int lastLine, int numConstants)
{
    m_sourceElements = sourceElements;
    m_varDeclarations = varStack;
    m_funcDeclarations = funcStack;
    m_features = features;
    m_lastLine = lastLine;
    m_numConstants = numConstants;
}

} // namespace JSC
