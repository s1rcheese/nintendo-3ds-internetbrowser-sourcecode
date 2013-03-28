/*
 *  WKCMemoryEventHandler.h
 *
 *  Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.
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

#ifndef WKCMemoryEventHandler_h
#define WKCMemoryEventHandler_h

// class definition

namespace WKC {

class WKCMemoryEventHandler
{
public:
    // checks whether requested size of memory is available.
    virtual bool checkMemoryAvailability(unsigned int request_size, bool in_forimage) = 0;

    // checks whether specified size of images should be decoded.
    // allow:          allow to decode
    // scale:          scale decoded image to fit specified width / height
    // reduce:         reduce color depth to 8bpp (with dither)
    // reduceandscale: both reduce and scale
    // deny:           deny to decode image
    enum {
        AllowDecode,
        ScaleDecode,
        ReduceDecode,
        ReduceAndScaleDecode,
        DenyDecode
    };
    virtual int checkImageDecodeMode(int w, int h, float& scalex, float& scaley) = 0;

    // check whether requested size of something should be allocatable.
    enum AllocationReason {
        Image,
        AnimeGif,
        JavaScript,
        ImageSharedBuffer,
        JavaScriptHeap,
        Normal
    };
    virtual bool checkMemoryAllocatable(unsigned int request_size, AllocationReason reason) = 0;

    enum BusyArea {
        None,
        ClearCache
    };
    // notifies need memory reduction because of memory exhaust.
    virtual void notifyNeedMemoryReduction(unsigned int request_size, BusyArea busy_area) = 0;

    // notifies memory exhaust event.
    // arguments and return value are for future implementations.
    // at this point please do exit / finalize application.
    virtual void* notifyMemoryExhaust(unsigned int request_size, unsigned int& out_allocated_size) = 0;

    // notifies memory exhaust event at allocation.
    virtual void notifyMemoryAllocationError(unsigned int request_size, AllocationReason reason) = 0;

    // notifies some CRASH() event
    // applications should do exit at this calling.
    virtual void notifyCrash(const char* file, int line, const char* function, const char* assertion) = 0;

    // notifies stack overflow.
    // applications should do exit at this calling if need_restart is true.
    virtual void notifyStackOverflow(bool need_restart, unsigned int stack_size, unsigned int consumption, unsigned int margin, void* stack_top, void* stack_base, void* current_stack_top, const char* file, int line, const char* function) = 0;
};

} // namespace

#endif // WKCMemoryEventHandler_h
