/*
 * Copyright (C) 2007 Kevin Ollivier <kevino@theolliviers.com>
 * Copyright (c) 2010,2011 ACCESS CO., LTD. All rights reserved.
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
 *
 */
#include "config.h"
#include <stdio.h>

#include "CSSValueKeywords.h"
#include "GraphicsContext.h"
#include "RenderTheme.h"
#include "HTMLInputElement.h"
#include "HTMLSelectElement.h"
#include "Font.h"
#include "FontSelector.h"

#include <wkc/wkcpeer.h>

#include "NotImplemented.h"

namespace WebCore {

using namespace HTMLNames;

class RenderThemeWKC : public RenderTheme
{
public:
    RenderThemeWKC() : RenderTheme() { };
    virtual ~RenderThemeWKC();

public:
   static PassRefPtr<RenderTheme> create();

    // A method asking if the theme's controls actually care about redrawing when hovered.
    virtual bool supportsHover(const RenderStyle*) const { return true; }

    virtual bool paintCheckbox(RenderObject*, const RenderObject::PaintInfo&, const IntRect&);
 
    virtual void setCheckboxSize(RenderStyle*) const;

    virtual bool paintRadio(RenderObject*, const RenderObject::PaintInfo&, const IntRect&);

    virtual void setRadioSize(RenderStyle*) const;

    virtual void adjustRepaintRect(const RenderObject*, IntRect&);

    virtual void adjustButtonStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
    virtual bool paintButton(RenderObject*, const RenderObject::PaintInfo&, const IntRect&);

    virtual void adjustTextFieldStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
    virtual bool paintTextField(RenderObject*, const RenderObject::PaintInfo&, const IntRect&);

    virtual void adjustTextAreaStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
    virtual bool paintTextArea(RenderObject*, const RenderObject::PaintInfo&, const IntRect&);

    virtual void adjustSearchFieldStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
    virtual bool paintSearchField(RenderObject*, const RenderObject::PaintInfo&, const IntRect&);

    virtual void adjustSliderTrackStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
    virtual bool paintSliderTrack(RenderObject*, const RenderObject::PaintInfo&, const IntRect&);

    virtual void adjustSliderThumbStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
    virtual bool paintSliderThumb(RenderObject*, const RenderObject::PaintInfo&, const IntRect&);

    virtual void adjustSliderThumbSize(RenderObject*) const;

    virtual int minimumMenuListSize(RenderStyle*) const;

    virtual void adjustMenuListStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
    virtual bool paintMenuList(RenderObject*, const RenderObject::PaintInfo&, const IntRect&);

    virtual void adjustMenuListButtonStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
    virtual bool paintMenuListButton(RenderObject*, const RenderObject::PaintInfo&, const IntRect&);

    virtual bool isControlStyled(const RenderStyle*, const BorderData&,
                                 const FillLayer&, const Color&) const;

    virtual bool controlSupportsTints(const RenderObject*) const;

    virtual Color platformFocusRingColor() const;

    virtual void systemFont(int propId, FontDescription&) const;
    virtual Color systemColor(int cssValueId) const;

    virtual Color platformActiveSelectionBackgroundColor() const;
    virtual Color platformInactiveSelectionBackgroundColor() const;
    
    virtual Color platformActiveSelectionForegroundColor() const;
    virtual Color platformInactiveSelectionForegroundColor() const;

    virtual int popupInternalPaddingLeft(RenderStyle*) const;
    virtual int popupInternalPaddingRight(RenderStyle*) const;
    virtual int popupInternalPaddingTop(RenderStyle*) const;
    virtual int popupInternalPaddingBottom(RenderStyle*) const;

    static void resetVariables();

private:
    void addIntrinsicMargins(RenderStyle*) const;
    void close();

    bool supportsFocus(ControlPart) const;
};

// parameters
// it should be in skins...

static const int cPopupInternalPaddingLeft   = 4;
static const int cPopupInternalPaddingRight  = 4;
static const int cPopupInternalPaddingTop    = 3;
static const int cPopupInternalPaddingBottom = 3;

// implementations

RenderThemeWKC::~RenderThemeWKC()
{
}

PassRefPtr<RenderTheme> RenderThemeWKC::create()
{
    return adoptRef(new RenderThemeWKC());
}

static RenderTheme* gTheme = 0;

PassRefPtr<RenderTheme> RenderTheme::themeForPage(Page* page)
{
    if (!gTheme) {
        gTheme = RenderThemeWKC::create().releaseRef();
    }
    return gTheme;
}

bool RenderThemeWKC::isControlStyled(const RenderStyle* style, const BorderData& border,
                                     const FillLayer& background, const Color& backgroundColor) const
{
    if (style->appearance() == TextFieldPart || style->appearance() == TextAreaPart || style->appearance() == ListboxPart)
        return style->border() != border;

    return RenderTheme::isControlStyled(style, border, background, backgroundColor);
}

void RenderThemeWKC::adjustRepaintRect(const RenderObject* o, IntRect& r)
{
    switch (o->style()->appearance()) {
        case MenulistPart: {
            r.setWidth(r.width() + 100);
            break;
        }
        default:
            break;
    }
}

bool RenderThemeWKC::controlSupportsTints(const RenderObject* o) const
{
    if (!isEnabled(o))
        return false;

    // Checkboxes only have tint when checked.
    if (o->style()->appearance() == CheckboxPart)
        return isChecked(o);

    // For now assume other controls have tint if enabled.
    return true;
}

void RenderThemeWKC::systemFont(int propId, FontDescription& fontDescription) const
{
    int type = 0;
    float size = 0.f;

    switch (propId) {
    case CSSValueCaption:
        type = WKC_SYSTEMFONT_TYPE_CAPTION; break;
    case CSSValueIcon:
        type = WKC_SYSTEMFONT_TYPE_ICON; break;
    case CSSValueMenu:
        type = WKC_SYSTEMFONT_TYPE_MENU; break;
    case CSSValueMessageBox:
        type = WKC_SYSTEMFONT_TYPE_MESSAGE_BOX; break;
    case CSSValueSmallCaption:
        type = WKC_SYSTEMFONT_TYPE_SMALL_CAPTION; break;
    case CSSValueWebkitMiniControl:
        type = WKC_SYSTEMFONT_TYPE_WEBKIT_MINI_CONTROL; break;
    case CSSValueWebkitSmallControl:
        type = WKC_SYSTEMFONT_TYPE_WEBKIT_SMALL_CONTROL; break;
    case CSSValueWebkitControl:
        type = WKC_SYSTEMFONT_TYPE_WEBKIT_CONTROL; break;
    case CSSValueStatusBar:
        type = WKC_SYSTEMFONT_TYPE_STATUS_BAR; break;
    default:
        return;
    }
    size = wkcStockImageGetSystemFontSizePeer(type);
    if (size) {
        fontDescription.setSpecifiedSize(size);
        fontDescription.setIsAbsoluteSize(true);
        fontDescription.setGenericFamily(FontDescription::NoFamily);
        fontDescription.setWeight(FontWeightNormal);
        fontDescription.setItalic(false);
    }
}

Color RenderThemeWKC::systemColor(int cssValueId) const
{
    int id = 0;

    switch (cssValueId) {
    case CSSValueActiveborder:
        id = WKC_SKINCOLOR_ACTIVEBORDER; break;
    case CSSValueActivecaption:
        id = WKC_SKINCOLOR_ACTIVECAPTION; break;
    case CSSValueAppworkspace:
        id = WKC_SKINCOLOR_APPWORKSPACE; break;
    case CSSValueBackground:
        id = WKC_SKINCOLOR_BACKGROUND; break;
    case CSSValueButtonface:
        id = WKC_SKINCOLOR_BUTTONFACE; break;
    case CSSValueButtonhighlight:
        id = WKC_SKINCOLOR_BUTTONHIGHLIGHT; break;
    case CSSValueButtonshadow:
        id = WKC_SKINCOLOR_BUTTONSHADOW; break;
    case CSSValueButtontext:
        id = WKC_SKINCOLOR_BUTTONTEXT; break;
    case CSSValueCaptiontext:
        id = WKC_SKINCOLOR_CAPTIONTEXT; break;
    case CSSValueGraytext:
        id = WKC_SKINCOLOR_GRAYTEXT; break;
    case CSSValueHighlight:
        id = WKC_SKINCOLOR_HIGHLIGHT; break;
    case CSSValueHighlighttext:
        id = WKC_SKINCOLOR_HIGHLIGHTTEXT; break;
    case CSSValueInactiveborder:
        id = WKC_SKINCOLOR_INACTIVEBORDER; break;
    case CSSValueInactivecaption:
        id = WKC_SKINCOLOR_INACTIVECAPTION; break;
    case CSSValueInactivecaptiontext:
        id = WKC_SKINCOLOR_INACTIVECAPTIONTEXT; break;
    case CSSValueInfobackground:
        id = WKC_SKINCOLOR_INFOBACKGROUND; break;
    case CSSValueInfotext:
        id = WKC_SKINCOLOR_INFOTEXT; break;
    case CSSValueMenu:
        id = WKC_SKINCOLOR_MENU; break;
    case CSSValueMenutext:
        id = WKC_SKINCOLOR_MENUTEXT; break;
    case CSSValueScrollbar:
        id = WKC_SKINCOLOR_SCROLLBAR; break;
    case CSSValueText:
        id = WKC_SKINCOLOR_TEXT; break;
    case CSSValueThreeddarkshadow:
        id = WKC_SKINCOLOR_THREEDDARKSHADOW; break;
    case CSSValueThreedface:
        id = WKC_SKINCOLOR_THREEDFACE; break;
    case CSSValueThreedhighlight:
        id = WKC_SKINCOLOR_THREEDHIGHLIGHTA; break;
    case CSSValueThreedlightshadow:
        id = WKC_SKINCOLOR_THREEDLIGHTSHADOW; break;
    case CSSValueThreedshadow:
        id = WKC_SKINCOLOR_THREEDSHADOW; break;
    case CSSValueWindow:
        id = WKC_SKINCOLOR_WINDOW; break;
    case CSSValueWindowframe:
        id = WKC_SKINCOLOR_WINDOWFRAME; break;
    case CSSValueWindowtext:
        id = WKC_SKINCOLOR_WINDOWTEXT; break;
    default:
        return RenderTheme::systemColor(cssValueId);

    }
    return wkcStockImageGetSkinColorPeer(id);
}

Color RenderThemeWKC::platformFocusRingColor() const
{
    return wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_FOCUSRING);
}

void RenderThemeWKC::addIntrinsicMargins(RenderStyle* style) const
{
    // Cut out the intrinsic margins completely if we end up using a small font size
    if (style->fontSize() < 11)
        return;

    // Intrinsic margin value.
    const int m = 2;

    // FIXME: Using width/height alone and not also dealing with min-width/max-width is flawed.
    if (style->width().isIntrinsicOrAuto()) {
        if (style->marginLeft().quirk())
            style->setMarginLeft(Length(m, Fixed));

        if (style->marginRight().quirk())
            style->setMarginRight(Length(m, Fixed));
    }

    if (style->height().isAuto()) {
        if (style->marginTop().quirk())
            style->setMarginTop(Length(m, Fixed));

        if (style->marginBottom().quirk())
            style->setMarginBottom(Length(m, Fixed));
    }
}

void RenderThemeWKC::setCheckboxSize(RenderStyle* style) const
{
    unsigned int w,dummy=0;
    // If the width and height are both specified, then we have nothing to do.
    if (!style->width().isIntrinsicOrAuto() && !style->height().isAuto())
        return;

    wkcStockImageGetSizePeer (WKC_IMAGE_CHECKBOX_UNCHECKED, &w, &dummy);
    if (style->width().isIntrinsicOrAuto())
        style->setWidth(Length((int)w, Fixed));

    if (style->height().isAuto())
        style->setHeight(Length((int)w, Fixed));
}

static void
_bitblt(void* ctx, int type, void* bitmap, int rowbytes, void* mask, int maskrowbytes, const WKCFloatRect* srcrect, const WKCFloatRect* destrect, int op)
{
    WKCPeerImage img = {0};

    img.fType = type;
    img.fBitmap = bitmap;
    img.fRowBytes = rowbytes;
    img.fMask = mask;
    img.fMaskRowBytes = maskrowbytes;
    WKCFloatRect_SetRect(&img.fSrcRect, srcrect);
    WKCFloatSize_Set(&img.fScale, 1, 1);
    WKCFloatSize_Set(&img.fiScale, 1, 1);
    WKCFloatPoint_Set(&img.fPhase, 0, 0);
    WKCFloatSize_Set(&img.fiTransform, 1, 1);

    wkcDrawContextBitBltPeer(ctx, &img, destrect, op);
}

bool RenderThemeWKC::paintCheckbox(RenderObject* o, const RenderObject::PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCFloatRect src, dest;
    int index;
    unsigned int width, height;
    const unsigned char* image_buf;
    unsigned int rowbytes;

    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    index = 0;
    if (this->isEnabled(o)) {
        if (this->isHovered(o))
              index = this->isChecked(o) ? WKC_IMAGE_CHECKBOX_CHECKED_HOVERED : WKC_IMAGE_CHECKBOX_UNCHECKED_HOVERED;
        else if (this->isFocused(o))
              index = this->isChecked(o) ? WKC_IMAGE_CHECKBOX_CHECKED_FOCUSED : WKC_IMAGE_CHECKBOX_UNCHECKED_FOCUSED;
        else
            index = this->isChecked(o) ? WKC_IMAGE_CHECKBOX_CHECKED : WKC_IMAGE_CHECKBOX_UNCHECKED;
    }
    else {
        index = this->isChecked(o) ? WKC_IMAGE_CHECKBOX_CHECKED_DISABLED : WKC_IMAGE_CHECKBOX_UNCHECKED_DISABLED;
    }
    
    image_buf = wkcStockImageGetBitmapPeer (index);
    if (!image_buf)
          return false;

    wkcStockImageGetSizePeer (index, &width, &height);
    if (width == 0 || height == 0)
          return false;

    rowbytes = width * 4;

    src.fX = 0; src.fY = 0;src.fWidth = (int) width; src.fHeight = (int) height;
    dest.fX = r.x(); dest.fY = r.y() + (r.height() - src.fHeight) / 2; 
    dest.fWidth = src.fWidth; dest.fHeight = src.fHeight;
    _bitblt (drawContext, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    return false;
}

void RenderThemeWKC::setRadioSize(RenderStyle* style) const
{
    // This is the same as checkboxes.
    setCheckboxSize(style);
}

bool RenderThemeWKC::paintRadio(RenderObject* o, const RenderObject::PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCFloatRect src, dest;
    int index;
    unsigned int width, height;
    const unsigned char* image_buf;
    unsigned int rowbytes;

    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    index = 0;
    if (this->isEnabled(o)) {
        if (this->isHovered(o))
              index = this->isChecked(o) ? WKC_IMAGE_RADIO_CHECKED_HOVERED : WKC_IMAGE_RADIO_UNCHECKED_HOVERED;
        else if (this->isFocused(o))
              index = this->isChecked(o) ? WKC_IMAGE_RADIO_CHECKED_FOCUSED : WKC_IMAGE_RADIO_UNCHECKED_FOCUSED;
        else 
            index = this->isChecked(o) ? WKC_IMAGE_RADIO_CHECKED : WKC_IMAGE_RADIO_UNCHECKED;
    }
    else {
        index = this->isChecked(o) ? WKC_IMAGE_RADIO_CHECKED_DISABLED : WKC_IMAGE_RADIO_UNCHECKED_DISABLED;
    }
    
    image_buf = wkcStockImageGetBitmapPeer (index);
    if (!image_buf)
          return false;

    wkcStockImageGetSizePeer (index, &width, &height);
    if (width == 0 || height == 0)
          return false;

    rowbytes = width * 4;

    src.fX = 0; src.fY = 0;src.fWidth = (int) width; src.fHeight = (int) height;
    dest.fX = r.x(); dest.fY = r.y() + (r.height() - src.fHeight) / 2; 
    dest.fWidth = src.fWidth; dest.fHeight = src.fHeight;
    _bitblt (drawContext, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    return false;
}

bool RenderThemeWKC::supportsFocus(ControlPart part) const
{
    switch (part) {
          case RadioPart:
    case CheckboxPart:
        case PushButtonPart:
        case ButtonPart:
        case TextFieldPart:
            return true;
        default: // No for all others...
            return false;
    }
}

void RenderThemeWKC::adjustButtonStyle(CSSStyleSelector* selector, RenderStyle* style, Element* e) const
{
    style->resetBorder();

    if (style->appearance()==PushButtonPart) {
        const int padw = 8;
        const int padh = 4;
        style->setHeight(Length(Auto));
        style->setWhiteSpace(PRE);
        style->setPaddingLeft(Length(padw, Fixed)); 
        style->setPaddingTop(Length(padh, Fixed)); 
        style->setPaddingRight(Length(padw, Fixed)); 
        style->setPaddingBottom(Length(padh, Fixed)); 
    } else {
        style->resetMargin();
        //addIntrinsicMargins(style);
    }
}

static void drawScalingBitmapPeer(RenderObject* in_o, void* in_context, void* in_bitmap, int rowbytes, WKCSize *in_size, const WKCPoint *in_points, const WKCRect *in_destrect, int op)
{
    WKCFloatRect src, dest;

      // upper
    src.fX = in_points[0].fX; src.fY = 0; 
    src.fWidth = in_points[1].fX - in_points[0].fX;
    src.fHeight = in_points[0].fY;
    dest.fX = in_destrect->fX + in_points[0].fX;
    dest.fY = in_destrect->fY;
    dest.fWidth = in_destrect->fWidth - in_points[0].fX - (in_size->fWidth - in_points[1].fX);
    dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // lower
    src.fX = in_points[2].fX; src.fY = in_points[2].fY;
    src.fWidth = in_points[3].fX - in_points[2].fX;
    src.fHeight = in_size->fHeight - in_points[2].fY;
    dest.fX = in_destrect->fX + in_points[2].fX;
    dest.fY = in_destrect->fY + in_destrect->fHeight - src.fHeight;
    dest.fWidth = in_destrect->fWidth - in_points[2].fX - (in_size->fWidth - in_points[3].fX);
    dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // left
    src.fX = 0; src.fY = in_points[0].fY;
    src.fWidth = in_points[0].fX;
    src.fHeight = in_points[2].fY - in_points[0].fY;
    dest.fX = in_destrect->fX; dest.fY = in_destrect->fY + in_points[0].fY;
    dest.fWidth = src.fWidth;
    dest.fHeight = in_destrect->fHeight - in_points[0].fY - (in_size->fHeight - in_points[2].fY);
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    //right
    src.fX = in_points[1].fX; src.fY = in_points[1].fY;
    src.fWidth = in_size->fWidth - in_points[1].fX;
    src.fHeight = in_points[3].fY - in_points[1].fY;
    dest.fX = in_destrect->fX + in_destrect->fWidth - src.fWidth;
    dest.fY = in_destrect->fY + in_points[1].fY;
    dest.fWidth = src.fWidth;
    dest.fHeight = in_destrect->fHeight - in_points[1].fY - (in_size->fHeight - in_points[3].fY);
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // center
    src.fX = in_points[0].fX; src.fY = in_points[0].fY;
    src.fWidth = in_points[3].fX - in_points[0].fX;
    src.fHeight = in_points[3].fY - in_points[0].fY;
    dest.fX = in_destrect->fX + in_points[0].fX;
    dest.fY = in_destrect->fY + in_points[0].fY;
    dest.fWidth = in_destrect->fWidth - in_points[0].fX - (in_size->fWidth - in_points[3].fX);
    dest.fHeight = in_destrect->fHeight - in_points[0].fY - (in_size->fHeight - in_points[3].fY);
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0)) {
        _bitblt (in_context, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);
    }

    // top left corner
    src.fX = 0; src.fY = 0; src.fWidth = in_points[0].fX; src.fHeight = in_points[0].fY;
    dest.fX = in_destrect->fX; dest.fY = in_destrect->fY; dest.fWidth = src.fWidth; dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // top right
    src.fX = in_points[1].fX; src.fY = 0; src.fWidth = in_size->fWidth - in_points[1].fX; src.fHeight = in_points[0].fY;
    dest.fX = in_destrect->fX + in_destrect->fWidth - src.fWidth; dest.fY = in_destrect->fY;
    dest.fWidth = src.fWidth; dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // bottom left
    src.fX = 0; src.fY = in_points[2].fY; src.fWidth = in_points[2].fX; src.fHeight = in_size->fHeight - in_points[2].fY;
    dest.fX = in_destrect->fX; dest.fY = in_destrect->fY + in_destrect->fHeight - src.fHeight;
    dest.fWidth = src.fWidth; dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);

    // bottom right corner
    src.fX = in_points[3].fX; src.fY = in_points[3].fY;
    src.fWidth = in_size->fWidth - in_points[3].fX; src.fHeight = in_size->fHeight - in_points[3].fY;
    dest.fX = in_destrect->fX + in_destrect->fWidth - src.fWidth;
    dest.fY = in_destrect->fY + in_destrect->fHeight - src.fHeight;
    dest.fWidth = src.fWidth;
    dest.fHeight = src.fHeight;
    if ((src.fWidth > 0) && (src.fHeight > 0) && (dest.fWidth > 0) && (dest.fHeight > 0))
        _bitblt (in_context, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, in_bitmap, rowbytes, 0, 0, &src, &dest, op);
}

bool RenderThemeWKC::paintButton(RenderObject* o, const RenderObject::PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCSize img_size;
    int index;
    unsigned int width, height;
    const WKCPoint* points;
    const unsigned char* image_buf;
    unsigned int rowbytes;
    WKCRect rect;
    
    if (!o)
          return false;

    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    index = 0;
    if (this->isEnabled(o)) {
        index = WKC_IMAGE_BUTTON;
        if (this->isHovered(o) || this->isFocused(o))
              index = WKC_IMAGE_BUTTON_HOVERED;
        if (this->isPressed(o))
              index = WKC_IMAGE_BUTTON_PRESSED;
    }
    else {
        index = WKC_IMAGE_BUTTON_DISABLED;
    }

    image_buf = wkcStockImageGetBitmapPeer (index);
    if (!image_buf)
          return false;

    wkcStockImageGetSizePeer (index, &width, &height);
    if (width == 0 || height == 0)
          return false;
    points = wkcStockImageGetLayoutPointsPeer (index);
    if (!points)
          return false;
    
    img_size.fWidth = width;
    img_size.fHeight = height;
    rowbytes = width * 4;

    rect.fX = r.x(); rect.fY = r.y(); rect.fWidth = r.width(); rect.fHeight = r.height();

    drawScalingBitmapPeer (o, drawContext, (void *)image_buf, rowbytes, &img_size, points, &rect, WKC_COMPOSITEOPERATION_SOURCEOVER);

    return false;
}

void RenderThemeWKC::adjustTextFieldStyle(CSSStyleSelector* css, RenderStyle* style, Element* e) const
{
   if (style->hasBackgroundImage()) {
        style->resetBorder();
        const unsigned int defaultBorderColor = wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BORDER);
        const short defaultBorderWidth = 1;
        const EBorderStyle defaultBorderStyle = SOLID;
        style->setBorderLeftWidth(defaultBorderWidth);
        style->setBorderLeftStyle(defaultBorderStyle);
        style->setBorderLeftColor(defaultBorderColor);
        style->setBorderRightWidth(defaultBorderWidth);
        style->setBorderRightStyle(defaultBorderStyle);
        style->setBorderRightColor(defaultBorderColor);
        style->setBorderTopWidth(defaultBorderWidth);
        style->setBorderTopStyle(defaultBorderStyle);    
        style->setBorderTopColor(defaultBorderColor);
        style->setBorderBottomWidth(defaultBorderWidth);
        style->setBorderBottomStyle(defaultBorderStyle);
        style->setBorderBottomColor(defaultBorderColor);
   }
 
}

bool RenderThemeWKC::paintTextField(RenderObject* o, const RenderObject::PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCFloatRect rect;
    int x,y,width,height;
    unsigned int backgroundColor;
    const unsigned int defaultBorderColor = wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BORDER);
    bool ret;

    if (!o)
        return false;
    drawContext = i.context->platformContext();
    if (!drawContext)
        return false;

    x = r.x(); y = r.y(); width = r.width(); height = r.height();
    rect.fX = x; rect.fY = y; rect.fWidth = width; rect.fHeight = height;

    ret = false;
    i.context->save();

    wkcDrawContextSetPenStylePeer(drawContext, defaultBorderColor, 1, WKC_STROKESTYLE_SOLID);
    backgroundColor = wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BACKGROUND);
    if (!this->isEnabled(o))
          backgroundColor = wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BACKGROUND_DISABLED);
    else if (o->style()->hasBackground())
          backgroundColor = (unsigned int)o->style()->backgroundColor().rgb();
    
    if (!o->style()->hasBackgroundImage()) {
        wkcDrawContextFillRectPeer (drawContext, &rect, backgroundColor);
    } else {
        ret = true;
    }

    i.context->restore();

    return ret;
}

void RenderThemeWKC::adjustTextAreaStyle(CSSStyleSelector* css, RenderStyle* style, Element* e) const
{
    adjustTextFieldStyle(css, style, e);
}

bool RenderThemeWKC::paintTextArea(RenderObject* o, const RenderObject::PaintInfo& info, const IntRect& r)
{
    return paintTextField(o, info, r);
}

void RenderThemeWKC::adjustSearchFieldStyle(CSSStyleSelector* css, RenderStyle* style, Element* e) const
{
    adjustTextFieldStyle(css, style, e);
}

bool RenderThemeWKC::paintSearchField(RenderObject* o, const RenderObject::PaintInfo& info, const IntRect& r)
{
    return paintTextField(o, info, r);
}

int RenderThemeWKC::minimumMenuListSize(RenderStyle* style) const 
{
    // same as safari.
    int fontsize = style->fontSize();
    if (fontsize >= 13) {
        return 9;
    } else if (fontsize >= 11) {
        return 5;
    }
    return 0;
}

void RenderThemeWKC::adjustMenuListStyle(CSSStyleSelector* selector, RenderStyle* style, Element* e) const
{
    style->resetBorder();
    style->resetPadding();
    style->setHeight(Length(Auto));
    style->setWhiteSpace(PRE);
}

bool RenderThemeWKC::paintMenuList(RenderObject* o, const RenderObject::PaintInfo& i, const IntRect& r)
{
    paintTextField(o, i, r);
    return paintMenuListButton (o, i, r);
}

void RenderThemeWKC::adjustMenuListButtonStyle(CSSStyleSelector* selector, RenderStyle* style, Element* e) const
{
    style->resetPadding();
    style->setLineHeight(RenderStyle::initialLineHeight());
}

bool RenderThemeWKC::paintMenuListButton(RenderObject* o, const RenderObject::PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCFloatRect src, dest;
    int index;
    unsigned int width, height;
    const WKCPoint* points;
    const unsigned char* image_buf;
    const int menulistPadding = 0;
    const int minimumHeight = 8;
    unsigned int rowbytes;
    int tmp_height;

    drawContext = i.context->platformContext();
    if (!drawContext)
          return false;

    index = 0;
    if (this->isEnabled(o)) {
        if (this->isHovered(o))
              index = WKC_IMAGE_MENU_LIST_BUTTON_HOVERED;
        else if (this->isFocused(o))
              index = WKC_IMAGE_MENU_LIST_BUTTON_FOCUSED;
        else
              index = WKC_IMAGE_MENU_LIST_BUTTON;
    }
    else {
        index = WKC_IMAGE_MENU_LIST_BUTTON_DISABLED;
    }

    image_buf = wkcStockImageGetBitmapPeer (index);
    if (!image_buf)
          return false;

    wkcStockImageGetSizePeer (index, &width, &height);
    if (width == 0 || height == 0)
          return false;
    points = wkcStockImageGetLayoutPointsPeer (index);
    if (!points)
          return false;

    rowbytes = width * 4;

    // center
    if (r.height() <= minimumHeight) { 
        src.fX = 0; src.fY = points[1].fY; 
        src.fWidth = (int) width; 
        src.fHeight = points[2].fY - points[1].fY;
        dest.fX = r.x() + r.width() - (int) width;
        dest.fY = r.y() + (r.height() - src.fHeight)/2;
        dest.fWidth = (int) width;
        dest.fHeight = src.fHeight;
        _bitblt (drawContext, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
        return false; // only paint the cneter
    }

    // upper + lower + center
    if (r.height() <= (points[3].fY - points[0].fY + 2 * menulistPadding)) {
          // upper
          src.fX = 0;
          src.fY = points[0].fY;
        src.fWidth = (int)width;
        src.fHeight = points[1].fY - points[0].fY;
        dest.fX = r.x() + r.width() - (int)width;
        dest.fY = r.y() + menulistPadding;
        dest.fWidth = (int)width;
        tmp_height = r.height() - (points[2].fY - points[1].fY);
        dest.fHeight = tmp_height / 2;
        if (dest.fHeight > 0)
            _bitblt (drawContext, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

        // center
        src.fY += src.fHeight;
        src.fHeight = points[2].fY - points[1].fY;
        dest.fY += dest.fHeight;
        dest.fHeight = src.fHeight;
        _bitblt (drawContext, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

        // lower
        src.fY += src.fHeight;
        src.fHeight = points[3].fY - points[2].fY;
        dest.fY += dest.fHeight;
        dest.fHeight = tmp_height / 2 + tmp_height % 2;
        if (dest.fHeight > 0)
            _bitblt (drawContext, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
        
        return false;
    }
    // paint all
    // top
    src.fX = 0; src.fY = 0;
    src.fWidth = (int) width;
    src.fHeight = points[0].fY;
    dest.fX = r.x() + r.width() - (int)width;
    dest.fY = r.y() + menulistPadding; 
    dest.fWidth = (int)width;
    dest.fHeight = src.fHeight; 
    _bitblt (drawContext, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    // upper
    src.fY += src.fHeight;
    src.fHeight = points[1].fY - points[0].fY;
    dest.fY += dest.fHeight;
    tmp_height = r.height() - ((int)height - points[3].fY + points[0].fY + points[2].fY - points[1].fY);
    dest.fHeight = tmp_height / 2;
    _bitblt (drawContext, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    // center
    src.fY += src.fHeight;
    src.fHeight = points[2].fY - points[1].fY;
    dest.fY += dest.fHeight;
    dest.fHeight = src.fHeight;
    _bitblt (drawContext, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    // lower
    src.fY += src.fHeight;
    src.fHeight = points[3].fY - points[2].fY;
    dest.fY += dest.fHeight;
    dest.fHeight = tmp_height / 2 + tmp_height % 2;
    _bitblt (drawContext, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);
    
    // bottom
    src.fY += src.fHeight;
    src.fHeight = (int) height - points[3].fY;
    dest.fY += dest.fHeight;
    dest.fHeight = src.fHeight;
    _bitblt (drawContext, WKC_IMAGETYPE_RGBA8888 | WKC_IMAGETYPE_FLAG_HASTRUEALPHA | WKC_IMAGETYPE_FLAG_FORSKIN, (void *)image_buf, rowbytes, 0, 0, &src, &dest, WKC_COMPOSITEOPERATION_SOURCEOVER);

    return false;
}


void RenderThemeWKC::adjustSliderTrackStyle(CSSStyleSelector* selector, RenderStyle* style, Element* ) const
{
}

bool RenderThemeWKC::paintSliderTrack(RenderObject* o, const RenderObject::PaintInfo& i, const IntRect& r)
{
    void *drawContext;
    WKCFloatRect rect;
    int x,y,width,height;
    unsigned int backgroundColor;
    const unsigned int defaultBorderColor = wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_THREEDFACE);

    if (!o)
        return false;
    drawContext = i.context->platformContext();
    if (!drawContext)
        return false;

    x = r.x(); y = r.y(); width = r.width(); height = r.height();
    rect.fX = x; rect.fY = y; rect.fWidth = width; rect.fHeight = height;

    if (o->style()->appearance() == SliderHorizontalPart) {
        rect.fHeight = 4;
        rect.fY += (height - rect.fHeight)/2;
    } else if (o->style()->appearance() == SliderVerticalPart) {
        rect.fWidth = 4;
    } else {
        return false;
    }

    i.context->save();

    wkcDrawContextSetPenStylePeer(drawContext, defaultBorderColor, 1, WKC_STROKESTYLE_SOLID);
    backgroundColor = wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_THREEDDARKSHADOW);
    if (!this->isEnabled(o))
          backgroundColor = wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_TEXTFIELD_BACKGROUND_DISABLED);
    else if (o->style()->hasBackground())
          backgroundColor = (unsigned int)o->style()->backgroundColor().rgb();
    wkcDrawContextFillRectPeer (drawContext, &rect, backgroundColor);

    i.context->restore();

    return false;
}

void RenderThemeWKC::adjustSliderThumbStyle(CSSStyleSelector*, RenderStyle* style, Element* e) const
{
    style->setBoxShadow(0);
}

bool RenderThemeWKC::paintSliderThumb(RenderObject* o, const RenderObject::PaintInfo& i, const IntRect& r)
{
    return paintButton(o, i, r);
}

void RenderThemeWKC::adjustSliderThumbSize(RenderObject* o) const
{
    const int sliderSize = 6;
    const int sliderThumbWidth = 15;
    const int sliderThumbHeight = 15;
    if (o->style()->appearance() == SliderThumbHorizontalPart) {
        o->style()->setWidth(Length(sliderSize, Fixed));
        o->style()->setHeight(Length(sliderThumbHeight, Fixed));
    } else if (o->style()->appearance() == SliderThumbVerticalPart) {
        o->style()->setHeight(Length(sliderSize, Fixed));
        o->style()->setWidth(Length(sliderThumbWidth, Fixed));
    }
}

Color RenderThemeWKC::platformActiveSelectionBackgroundColor() const
{
    return wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_ACTIVESELECTIONBACKGROUND);
}

Color RenderThemeWKC::platformInactiveSelectionBackgroundColor() const
{
    return wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_INACTIVESELECTIONBACKGROUND);
}

Color RenderThemeWKC::platformActiveSelectionForegroundColor() const
{
    return wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_ACTIVESELECTIONFOREGROUND);
}

Color RenderThemeWKC::platformInactiveSelectionForegroundColor() const
{
    return wkcStockImageGetSkinColorPeer(WKC_SKINCOLOR_INACTIVESELECTIONFOREGROUND);
}

int RenderThemeWKC::popupInternalPaddingLeft(RenderStyle* style) const 
{ 
    return cPopupInternalPaddingLeft;
}

int RenderThemeWKC::popupInternalPaddingRight(RenderStyle* style) const 
{
    unsigned int w=0, h=0;
    wkcStockImageGetSizePeer(WKC_IMAGE_MENU_LIST_BUTTON, &w, &h);
    return w + cPopupInternalPaddingRight;
}

int RenderThemeWKC::popupInternalPaddingTop(RenderStyle* style) const 
{
    return cPopupInternalPaddingTop;
}

int RenderThemeWKC::popupInternalPaddingBottom(RenderStyle* style) const
{
    return cPopupInternalPaddingBottom;
}

void RenderThemeWKC_resetVariables()
{
    gTheme = 0;
}

}
