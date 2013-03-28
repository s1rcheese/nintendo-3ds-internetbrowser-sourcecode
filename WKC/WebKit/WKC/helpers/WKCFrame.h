/*
 *  Copyright (c) 2011 ACCESS CO., LTD. All rights reserved.
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef _WKC_HELPER_WKCFRAME_H_
#define _WKC_HELPER_WKCFRAME_H_

namespace WKC {

class Document;
class HTMLFrameOwnerElement;
class FrameView;
class Page;
class SelectionController;
class FrameLoader;
class FrameLoaderPrivate;
class FrameTree;

class FramePrivate;

class Frame {
public:
    Frame(FramePrivate&);
    Frame(Frame*, bool needsRef = false);
    ~Frame();

    bool compare(const Frame*) const ;

    Document* document() const;
    Page* page() const;
    FrameView* view() const;
    SelectionController* selection() const;
    FrameLoader* loader();
    FrameTree* tree() const;

    HTMLFrameOwnerElement* ownerElement() const;

    FramePrivate& priv() { return m_private; }

private:
    FramePrivate* m_ownedPrivate;
    FramePrivate& m_private;
    bool m_needsRef;
};
} // namespace

#endif // _WKC_HELPER_WKCFRAME_H_
