/*
 *  WKCTimerEventHandler.h
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

#ifndef WKCTimerEventHandler_h
#define WKCTimerEventHandler_h

// class definition

namespace WKC {

typedef bool (*TimeoutProc) (void*);

class WKCTimerEventHandler
{
public:
    // requests to call in_proc in browser thread
    virtual void requestWakeUp(TimeoutProc in_proc, void* in_data) = 0;
};

} // namespace

#endif // WKCTimerEventHandler_h
