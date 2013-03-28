/*
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2009 Gustavo Noronha Silva <gns@gnome.org>
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
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
#include "Logging.h"
#include "PlatformString.h"

#include "NotImplemented.h"

namespace WebCore {

void InitializeLoggingChannelsIfNecessary()
{
    WTFLogChannel* channel = NULL;

    channel = getChannelFromName(String("BackForward"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("Editing"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("Events"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("Frames"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("FTP"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("History"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("IconDatabase"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("Loading"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("Media"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("Network"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("NotYetImplemented"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("PageCache"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("PlatformLeaks"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("Plugins"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("PopupBlocking"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("SpellingAndGrammar"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("SQLDatabase"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("StorageAPI"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("TextConversion"));
    channel->state = WTFLogChannelOn;
    channel = getChannelFromName(String("Threading"));
    channel->state = WTFLogChannelOn;
}

} // namespace WebCore
