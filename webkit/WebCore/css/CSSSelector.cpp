/*
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *               1999 Waldo Bastian (bastian@kde.org)
 *               2001 Andreas Schlapbach (schlpbch@iam.unibe.ch)
 *               2001-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2008 David Smith (catfish.man@gmail.com)
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
#include "CSSSelector.h"

#include "wtf/Assertions.h"
#include "HTMLNames.h"

#include <wtf/StdLibExtras.h>
#if 1
// added at webkit.org trunk r63747
#include <wtf/Vector.h>
#endif

namespace WebCore {
    
using namespace HTMLNames;

#if 1
// added at webkit.org trunk r63747 and modified at r63892
class CSSSelectorBag : public Noncopyable {
public:
    ~CSSSelectorBag()
    {
        ASSERT(isEmpty());
    }

    bool isEmpty() const
    {
        return m_stack.isEmpty();
    }

    void add(PassOwnPtr<CSSSelector> selector)
    {
        if (selector)
            m_stack.append(selector.release()); // This line is modified from r63747's for this revision.
    }

    PassOwnPtr<CSSSelector> takeAny()
    {
        ASSERT(!isEmpty());
        OwnPtr<CSSSelector> selector = PassOwnPtr<CSSSelector>(m_stack.last()); // This line is modified from r63747's for this revision.
        m_stack.removeLast();
        return PassOwnPtr<CSSSelector>(selector.release()); // This line is modified from r63747's for this revision.
    }

private:
    Vector<CSSSelector*, 16> m_stack;
};
#endif

#if 1
// modified at webkit.org trunk r65123
unsigned CSSSelector::specificity() const
{
    // make sure the result doesn't overflow
    static const unsigned maxValueMask = 0xffffff;
    unsigned total = 0;
    for (const CSSSelector* selector = this; selector; selector = selector->tagHistory()) {
        #if 0
        // This IF block in r65123 doesn't work in this revision, so it is commented out.
        if (selector->m_isForPage)
            return (total + selector->specificityForPage()) & maxValueMask;
        #endif
        total = (total + selector->specificityForOneSelector()) & maxValueMask;
    }
    return total;
}

inline unsigned CSSSelector::specificityForOneSelector() const
{
    // FIXME: Pseudo-elements and pseudo-classes do not have the same specificity. This function
    // isn't quite correct.
    unsigned s = (m_tag.localName() == starAtom ? 0 : 1);
    switch (m_match) {
    case Id:
        s += 0x10000;
        break;
    case Exact:
    case Class:
    case Set:
    case List:
    case Hyphen:
    case PseudoClass:
    case PseudoElement:
    case Contain:
    case Begin:
    case End:
        s += 0x100;
    case None:
        break;
    }
    return s;
}
#else
unsigned int CSSSelector::specificity()
{
    // FIXME: Pseudo-elements and pseudo-classes do not have the same specificity. This function
    // isn't quite correct.
    int s = (m_tag.localName() == starAtom ? 0 : 1);
    switch (m_match) {
        case Id:
            s += 0x10000;
            break;
        case Exact:
        case Class:
        case Set:
        case List:
        case Hyphen:
        case PseudoClass:
        case PseudoElement:
        case Contain:
        case Begin:
        case End:
            s += 0x100;
        case None:
            break;
    }

    if (CSSSelector* tagHistory = this->tagHistory())
        s += tagHistory->specificity();

    // make sure it doesn't overflow
    return s & 0xffffff;
}
#endif

static const AtomicString* active = 0;
static const AtomicString* after = 0;
static const AtomicString* anyLink = 0;
static const AtomicString* autofill = 0;
static const AtomicString* before = 0;
static const AtomicString* checked = 0;
static const AtomicString* fileUploadButton = 0;
static const AtomicString* defaultString = 0;
static const AtomicString* disabled = 0;
static const AtomicString* readOnly = 0;
static const AtomicString* readWrite = 0;
static const AtomicString* valid = 0;
static const AtomicString* invalid = 0;
static const AtomicString* drag = 0;
static const AtomicString* dragAlias = 0; // was documented with this name in Apple documentation, so keep an alia
static const AtomicString* empty = 0;
static const AtomicString* enabled = 0;
static const AtomicString* firstChild = 0;
static const AtomicString* firstLetter = 0;
static const AtomicString* firstLine = 0;
static const AtomicString* firstOfType = 0;
static const AtomicString* fullPageMedia = 0;
static const AtomicString* nthChild = 0;
static const AtomicString* nthOfType = 0;
static const AtomicString* nthLastChild = 0;
static const AtomicString* nthLastOfType = 0;
static const AtomicString* focus = 0;
static const AtomicString* hover = 0;
static const AtomicString* indeterminate = 0;
static const AtomicString* innerSpinButton = 0;
#if ENABLE(DATALIST)
static const AtomicString* inputListButton = 0;
#endif
static const AtomicString* inputPlaceholder = 0;
static const AtomicString* lastChild = 0;
static const AtomicString* lastOfType = 0;
static const AtomicString* link = 0;
static const AtomicString* lang = 0;
static const AtomicString* mediaControlsPanel = 0;
static const AtomicString* mediaControlsMuteButton = 0;
static const AtomicString* mediaControlsPlayButton = 0;
static const AtomicString* mediaControlsTimeline = 0;
static const AtomicString* mediaControlsVolumeSlider = 0;
static const AtomicString* mediaControlsSeekBackButton = 0;
static const AtomicString* mediaControlsSeekForwardButton = 0;
static const AtomicString* mediaControlsRewindButton = 0;
static const AtomicString* mediaControlsReturnToRealtimeButton = 0;
static const AtomicString* mediaControlsToggleClosedCaptionsButton = 0;
static const AtomicString* mediaControlsStatusDisplay = 0;
static const AtomicString* mediaControlsFullscreenButton = 0;
static const AtomicString* mediaControlsTimelineContainer = 0;
static const AtomicString* mediaControlsVolumeSliderContainer = 0;
static const AtomicString* mediaControlsCurrentTimeDisplay = 0;
static const AtomicString* mediaControlsTimeRemainingDisplay = 0;
static const AtomicString* notStr = 0;
static const AtomicString* onlyChild = 0;
static const AtomicString* onlyOfType = 0;
static const AtomicString* optional = 0;
static const AtomicString* outerSpinButton = 0;
static const AtomicString* required = 0;
static const AtomicString* resizer = 0;
static const AtomicString* root = 0;
static const AtomicString* scrollbar = 0;
static const AtomicString* scrollbarButton = 0;
static const AtomicString* scrollbarCorner = 0;
static const AtomicString* scrollbarThumb = 0;
static const AtomicString* scrollbarTrack = 0;
static const AtomicString* scrollbarTrackPiece = 0;
static const AtomicString* searchCancelButton = 0;
static const AtomicString* searchDecoration = 0;
static const AtomicString* searchResultsDecoration = 0;
static const AtomicString* searchResultsButton = 0;
static const AtomicString* selection = 0;
static const AtomicString* sliderThumb = 0;
static const AtomicString* target = 0;
static const AtomicString* visited = 0;
static const AtomicString* windowInactive = 0;
static const AtomicString* decrement = 0;
static const AtomicString* increment = 0;
static const AtomicString* start = 0;
static const AtomicString* end = 0;
static const AtomicString* horizontal = 0;
static const AtomicString* vertical = 0;
static const AtomicString* doubleButton = 0;
static const AtomicString* singleButton = 0;
static const AtomicString* noButton = 0;
static const AtomicString* cornerPresent = 0;

static bool gCSSSelectorStringInit = false;

void CSSSelector::extractPseudoType() const
{
    if (m_match != PseudoClass && m_match != PseudoElement)
        return;

    if (!gCSSSelectorStringInit)
        createSharedInstance();

    bool element = false; // pseudo-element
    bool compat = false; // single colon compatbility mode

    m_pseudoType = PseudoUnknown;
    if (m_value == *active)
        m_pseudoType = PseudoActive;
    else if (m_value == *after) {
        m_pseudoType = PseudoAfter;
        element = true;
        compat = true;
    } else if (m_value == *anyLink)
        m_pseudoType = PseudoAnyLink;
    else if (m_value == *autofill)
        m_pseudoType = PseudoAutofill;
    else if (m_value == *before) {
        m_pseudoType = PseudoBefore;
        element = true;
        compat = true;
    } else if (m_value == *checked)
        m_pseudoType = PseudoChecked;
    else if (m_value == *fileUploadButton) {
        m_pseudoType = PseudoFileUploadButton;
        element = true;
    } else if (m_value == *defaultString)
        m_pseudoType = PseudoDefault;
    else if (m_value == *disabled)
        m_pseudoType = PseudoDisabled;
    else if (m_value == *readOnly)
        m_pseudoType = PseudoReadOnly;
    else if (m_value == *readWrite)
        m_pseudoType = PseudoReadWrite;
    else if (m_value == *valid)
        m_pseudoType = PseudoValid;
    else if (m_value == *invalid)
        m_pseudoType = PseudoInvalid;
    else if (m_value == *drag || m_value == *dragAlias)
        m_pseudoType = PseudoDrag;
    else if (m_value == *enabled)
        m_pseudoType = PseudoEnabled;
    else if (m_value == *empty)
        m_pseudoType = PseudoEmpty;
    else if (m_value == *firstChild)
        m_pseudoType = PseudoFirstChild;
    else if (m_value == *fullPageMedia)
        m_pseudoType = PseudoFullPageMedia;
    else
#if ENABLE(DATALIST)
    if (m_value == *inputListButton) {
        m_pseudoType = PseudoInputListButton;
        element = true;
    } else
#endif
    if (m_value == *inputPlaceholder) {
        m_pseudoType = PseudoInputPlaceholder;
        element = true;
    } else if (m_value == *lastChild)
        m_pseudoType = PseudoLastChild;
    else if (m_value == *lastOfType)
        m_pseudoType = PseudoLastOfType;
    else if (m_value == *onlyChild)
        m_pseudoType = PseudoOnlyChild;
    else if (m_value == *onlyOfType)
        m_pseudoType = PseudoOnlyOfType;
    else if (m_value == *firstLetter) {
        m_pseudoType = PseudoFirstLetter;
        element = true;
        compat = true;
    } else if (m_value == *firstLine) {
        m_pseudoType = PseudoFirstLine;
        element = true;
        compat = true;
    } else if (m_value == *firstOfType)
        m_pseudoType = PseudoFirstOfType;
    else if (m_value == *focus)
        m_pseudoType = PseudoFocus;
    else if (m_value == *hover)
        m_pseudoType = PseudoHover;
    else if (m_value == *indeterminate)
        m_pseudoType = PseudoIndeterminate;
    else if (m_value == *innerSpinButton) {
        m_pseudoType = PseudoInnerSpinButton;
        element = true;
    } else if (m_value == *link)
        m_pseudoType = PseudoLink;
    else if (m_value == *lang)
        m_pseudoType = PseudoLang;
    else if (m_value == *mediaControlsPanel) {
        m_pseudoType = PseudoMediaControlsPanel;
        element = true;
    } else if (m_value == *mediaControlsMuteButton) {
        m_pseudoType = PseudoMediaControlsMuteButton;
        element = true;
    } else if (m_value == *mediaControlsPlayButton) {
        m_pseudoType = PseudoMediaControlsPlayButton;
        element = true;
    } else if (m_value == *mediaControlsCurrentTimeDisplay) {
        m_pseudoType = PseudoMediaControlsCurrentTimeDisplay;
        element = true;
    } else if (m_value == *mediaControlsTimeRemainingDisplay) {
        m_pseudoType = PseudoMediaControlsTimeRemainingDisplay;
        element = true;
    } else if (m_value == *mediaControlsTimeline) {
        m_pseudoType = PseudoMediaControlsTimeline;
        element = true;
    } else if (m_value == *mediaControlsVolumeSlider) {
        m_pseudoType = PseudoMediaControlsVolumeSlider;
        element = true;
    } else if (m_value == *mediaControlsSeekBackButton) {
        m_pseudoType = PseudoMediaControlsSeekBackButton;
        element = true;
    } else if (m_value == *mediaControlsSeekForwardButton) {
        m_pseudoType = PseudoMediaControlsSeekForwardButton;
        element = true;
    } else if (m_value == *mediaControlsRewindButton) {
        m_pseudoType = PseudoMediaControlsRewindButton;
        element = true;
    } else if (m_value == *mediaControlsReturnToRealtimeButton) {
        m_pseudoType = PseudoMediaControlsReturnToRealtimeButton;
        element = true;
    } else if (m_value == *mediaControlsToggleClosedCaptionsButton) {
        m_pseudoType = PseudoMediaControlsToggleClosedCaptions;
        element = true;
    } else if (m_value == *mediaControlsStatusDisplay) {
        m_pseudoType = PseudoMediaControlsStatusDisplay;
        element = true;
    } else if (m_value == *mediaControlsFullscreenButton) {
        m_pseudoType = PseudoMediaControlsFullscreenButton;
        element = true;
    } else if (m_value == *mediaControlsTimelineContainer) {
        m_pseudoType = PseudoMediaControlsTimelineContainer;
        element = true;
    } else if (m_value == *mediaControlsVolumeSliderContainer) {
        m_pseudoType = PseudoMediaControlsVolumeSliderContainer;
        element = true;
    } else if (m_value == *notStr)
        m_pseudoType = PseudoNot;
    else if (m_value == *nthChild)
        m_pseudoType = PseudoNthChild;
    else if (m_value == *nthOfType)
        m_pseudoType = PseudoNthOfType;
    else if (m_value == *nthLastChild)
        m_pseudoType = PseudoNthLastChild;
    else if (m_value == *nthLastOfType)
        m_pseudoType = PseudoNthLastOfType;
    else if (m_value == *outerSpinButton) {
        m_pseudoType = PseudoOuterSpinButton;
        element = true;
    } else if (m_value == *root)
        m_pseudoType = PseudoRoot;
    else if (m_value == *windowInactive)
        m_pseudoType = PseudoWindowInactive;
    else if (m_value == *decrement)
        m_pseudoType = PseudoDecrement;
    else if (m_value == *increment)
        m_pseudoType = PseudoIncrement;
    else if (m_value == *start)
        m_pseudoType = PseudoStart;
    else if (m_value == *end)
        m_pseudoType = PseudoEnd;
    else if (m_value == *horizontal)
        m_pseudoType = PseudoHorizontal;
    else if (m_value == *vertical)
        m_pseudoType = PseudoVertical;
    else if (m_value == *doubleButton)
        m_pseudoType = PseudoDoubleButton;
    else if (m_value == *singleButton)
        m_pseudoType = PseudoSingleButton;
    else if (m_value == *noButton)
        m_pseudoType = PseudoNoButton;
    else if (m_value == *optional)
        m_pseudoType = PseudoOptional;
    else if (m_value == *required)
        m_pseudoType = PseudoRequired;
    else if (m_value == *scrollbarCorner) {
        element = true;
        m_pseudoType = PseudoScrollbarCorner;
    } else if (m_value == *resizer) {
        element = true;
        m_pseudoType = PseudoResizer;
    } else if (m_value == *scrollbar) {
        element = true;
        m_pseudoType = PseudoScrollbar;
    } else if (m_value == *scrollbarButton) {
        element = true;
        m_pseudoType = PseudoScrollbarButton;
    } else if (m_value == *scrollbarCorner) {
        element = true;
        m_pseudoType = PseudoScrollbarCorner;
    } else if (m_value == *scrollbarThumb) {
        element = true;
        m_pseudoType = PseudoScrollbarThumb;
    } else if (m_value == *scrollbarTrack) {
        element = true;
        m_pseudoType = PseudoScrollbarTrack;
    } else if (m_value == *scrollbarTrackPiece) {
        element = true;
        m_pseudoType = PseudoScrollbarTrackPiece;
    } else if (m_value == *cornerPresent)
         m_pseudoType = PseudoCornerPresent;
    else if (m_value == *searchCancelButton) {
        m_pseudoType = PseudoSearchCancelButton;
        element = true;
    } else if (m_value == *searchDecoration) {
        m_pseudoType = PseudoSearchDecoration;
        element = true;
    } else if (m_value == *searchResultsDecoration) {
        m_pseudoType = PseudoSearchResultsDecoration;
        element = true;
    } else if (m_value == *searchResultsButton) {
        m_pseudoType = PseudoSearchResultsButton;
        element = true;
    }  else if (m_value == *selection) {
        m_pseudoType = PseudoSelection;
        element = true;
    } else if (m_value == *sliderThumb) {
        m_pseudoType = PseudoSliderThumb;
        element = true;
    } else if (m_value == *target)
        m_pseudoType = PseudoTarget;
    else if (m_value == *visited)
        m_pseudoType = PseudoVisited;

    if (m_match == PseudoClass && element) {
        if (!compat) 
            m_pseudoType = PseudoUnknown;
        else 
           m_match = PseudoElement;
    } else if (m_match == PseudoElement && !element)
        m_pseudoType = PseudoUnknown;
}

bool CSSSelector::operator==(const CSSSelector& other)
{
    const CSSSelector* sel1 = this;
    const CSSSelector* sel2 = &other;

    while (sel1 && sel2) {
        if (sel1->m_tag != sel2->m_tag || sel1->attribute() != sel2->attribute() ||
             sel1->relation() != sel2->relation() || sel1->m_match != sel2->m_match ||
             sel1->m_value != sel2->m_value ||
             sel1->pseudoType() != sel2->pseudoType() ||
             sel1->argument() != sel2->argument())
            return false;
        sel1 = sel1->tagHistory();
        sel2 = sel2->tagHistory();
    }

    if (sel1 || sel2)
        return false;

    return true;
}

String CSSSelector::selectorText() const
{
    String str = "";

    const AtomicString& prefix = m_tag.prefix();
    const AtomicString& localName = m_tag.localName();
    if (m_match == CSSSelector::None || !prefix.isNull() || localName != starAtom) {
        if (prefix.isNull())
            str = localName;
        else
            str = prefix + "|" + localName;
    }

    const CSSSelector* cs = this;
    while (true) {
        if (cs->m_match == CSSSelector::Id) {
            str += "#";
            str += cs->m_value;
        } else if (cs->m_match == CSSSelector::Class) {
            str += ".";
            str += cs->m_value;
        } else if (cs->m_match == CSSSelector::PseudoClass) {
            str += ":";
            str += cs->m_value;
            if (cs->pseudoType() == PseudoNot) {
                if (CSSSelector* subSel = cs->simpleSelector())
                    str += subSel->selectorText();
                str += ")";
            } else if (cs->pseudoType() == PseudoLang
                    || cs->pseudoType() == PseudoNthChild
                    || cs->pseudoType() == PseudoNthLastChild
                    || cs->pseudoType() == PseudoNthOfType
                    || cs->pseudoType() == PseudoNthLastOfType) {
                str += cs->argument();
                str += ")";
            }
        } else if (cs->m_match == CSSSelector::PseudoElement) {
            str += "::";
            str += cs->m_value;
        } else if (cs->hasAttribute()) {
            str += "[";
            const AtomicString& prefix = cs->attribute().prefix();
            if (!prefix.isNull())
                str += prefix + "|";
            str += cs->attribute().localName();
            switch (cs->m_match) {
                case CSSSelector::Exact:
                    str += "=";
                    break;
                case CSSSelector::Set:
                    // set has no operator or value, just the attrName
                    str += "]";
                    break;
                case CSSSelector::List:
                    str += "~=";
                    break;
                case CSSSelector::Hyphen:
                    str += "|=";
                    break;
                case CSSSelector::Begin:
                    str += "^=";
                    break;
                case CSSSelector::End:
                    str += "$=";
                    break;
                case CSSSelector::Contain:
                    str += "*=";
                    break;
                default:
                    break;
            }
            if (cs->m_match != CSSSelector::Set) {
                str += "\"";
                str += cs->m_value;
                str += "\"]";
            }
        }
        if (cs->relation() != CSSSelector::SubSelector || !cs->tagHistory())
            break;
        cs = cs->tagHistory();
    }

    if (CSSSelector* tagHistory = cs->tagHistory()) {
        String tagHistoryText = tagHistory->selectorText();
        if (cs->relation() == CSSSelector::DirectAdjacent)
            str = tagHistoryText + " + " + str;
        else if (cs->relation() == CSSSelector::IndirectAdjacent)
            str = tagHistoryText + " ~ " + str;
        else if (cs->relation() == CSSSelector::Child)
            str = tagHistoryText + " > " + str;
        else
            // Descendant
            str = tagHistoryText + " " + str;
    }

    return str;
}
    
void CSSSelector::setTagHistory(CSSSelector* tagHistory) 
{ 
    if (m_hasRareData) 
        m_data.m_rareData->m_tagHistory.set(tagHistory); 
    else 
        m_data.m_tagHistory = tagHistory; 
}

const QualifiedName& CSSSelector::attribute() const
{ 
    switch (m_match) {
    case Id:
        return idAttr;
    case Class:
        return classAttr;
    default:
        return m_hasRareData ? m_data.m_rareData->m_attribute : anyQName();
    }
}

void CSSSelector::setAttribute(const QualifiedName& value) 
{ 
    createRareData(); 
    m_data.m_rareData->m_attribute = value; 
}
    
void CSSSelector::setArgument(const AtomicString& value) 
{ 
    createRareData(); 
    m_data.m_rareData->m_argument = value; 
}

void CSSSelector::setSimpleSelector(CSSSelector* value)
{
    createRareData(); 
    m_data.m_rareData->m_simpleSelector.set(value); 
}

bool CSSSelector::parseNth()
{
    if (!m_hasRareData)
        return false;
    if (m_parsedNth)
        return true;
    m_parsedNth = m_data.m_rareData->parseNth();
    return m_parsedNth;
}

bool CSSSelector::matchNth(int count)
{
    ASSERT(m_hasRareData);
    return m_data.m_rareData->matchNth(count);
}

// a helper function for parsing nth-arguments
bool CSSSelector::RareData::parseNth()
{    
    const String& argument = m_argument;
    
    if (argument.isEmpty())
        return false;
    
    m_a = 0;
    m_b = 0;
    if (argument == "odd") {
        m_a = 2;
        m_b = 1;
    } else if (argument == "even") {
        m_a = 2;
        m_b = 0;
    } else {
        int n = argument.find('n');
        if (n != -1) {
            if (argument[0] == '-') {
                if (n == 1)
                    m_a = -1; // -n == -1n
                else
                    m_a = argument.substring(0, n).toInt();
            } else if (!n)
                m_a = 1; // n == 1n
            else
                m_a = argument.substring(0, n).toInt();
            
            int p = argument.find('+', n);
            if (p != -1)
                m_b = argument.substring(p + 1, argument.length() - p - 1).toInt();
            else {
                p = argument.find('-', n);
                m_b = -argument.substring(p + 1, argument.length() - p - 1).toInt();
            }
        } else
            m_b = argument.toInt();
    }
    return true;
}

// a helper function for checking nth-arguments
bool CSSSelector::RareData::matchNth(int count)
{
    if (!m_a)
        return count == m_b;
    else if (m_a > 0) {
        if (count < m_b)
            return false;
        return (count - m_b) % m_a == 0;
    } else {
        if (count > m_b)
            return false;
        return (m_b - count) % (-m_a) == 0;
    }
}

#if 1
// added at webkit.org trunk r63892
inline void CSSSelector::releaseOwnedSelectorsToBag(CSSSelectorBag& bag)
{
    if (m_hasRareData) {
        ASSERT(m_data.m_rareData);
        bag.add(PassOwnPtr<CSSSelector>(m_data.m_rareData->m_tagHistory.release())); // This line is modified from r63892's for this revision.
        bag.add(PassOwnPtr<CSSSelector>(m_data.m_rareData->m_simpleSelector.release())); // This line is modified from r63892's for this revision.
        delete m_data.m_rareData;
        // Clear the pointer so that a destructor of this selector will not
        // traverse this chain.
        m_data.m_rareData = 0;
    } else {
        bag.add(PassOwnPtr<CSSSelector>(m_data.m_tagHistory)); // This line is modified from r63892's for this revision.
        // Clear the pointer for the same reason.
        m_data.m_tagHistory = 0;
    }
}

void CSSSelector::deleteReachableSelectors()
{
    // Traverse the chain of selectors and delete each iteratively.
    CSSSelectorBag selectorsToBeDeleted;
    releaseOwnedSelectorsToBag(selectorsToBeDeleted);
    while (!selectorsToBeDeleted.isEmpty()) {
        OwnPtr<CSSSelector> selector(selectorsToBeDeleted.takeAny());
        ASSERT(selector);
        selector->releaseOwnedSelectorsToBag(selectorsToBeDeleted);
    }
}
#endif

/* static */bool CSSSelector::createSharedInstance()
{
    bool ret = false;

    active = new AtomicString("active");
    if (!active)
        goto exit_func;

    after = new AtomicString("after");
    if (!after)
        goto exit_func;

    anyLink = new AtomicString("-webkit-any-link");
    if (!anyLink)
        goto exit_func;

    autofill = new AtomicString("-webkit-autofill");
    if (!autofill)
        goto exit_func;

    before = new AtomicString("before");
    if (!before)
        goto exit_func;

    checked = new AtomicString("checked");
    if (!checked)
        goto exit_func;

    fileUploadButton = new AtomicString("-webkit-file-upload-button");
    if (!fileUploadButton)
        goto exit_func;

    defaultString = new AtomicString("default");
    if (!defaultString)
        goto exit_func;

    disabled = new AtomicString("disabled");
    if (!disabled)
        goto exit_func;

    readOnly = new AtomicString("read-only");
    if (!readOnly)
        goto exit_func;

    readWrite = new AtomicString("read-write");
    if (!readWrite)
        goto exit_func;

    valid = new AtomicString("valid");
    if (!valid)
        goto exit_func;

    invalid = new AtomicString("invalid");
    if (!invalid)
        goto exit_func;

    drag = new AtomicString("-webkit-drag");
    if (!drag)
        goto exit_func;

    dragAlias = new AtomicString("-khtml-drag"); // was documented with this name in Apple documentation, so keep an alia
    if (!dragAlias)
        goto exit_func;

    empty = new AtomicString("empty");
    if (!empty)
        goto exit_func;

    enabled = new AtomicString("enabled");
    if (!enabled)
        goto exit_func;

    firstChild = new AtomicString("first-child");
    if (!firstChild)
        goto exit_func;

    firstLetter = new AtomicString("first-letter");
    if (!firstLetter)
        goto exit_func;

    firstLine = new AtomicString("first-line");
    if (!firstLine)
        goto exit_func;

    firstOfType = new AtomicString("first-of-type");
    if (!firstOfType)
        goto exit_func;

    fullPageMedia = new AtomicString("-webkit-full-page-media");
    if (!fullPageMedia)
        goto exit_func;

    nthChild = new AtomicString("nth-child(");
    if (!nthChild)
        goto exit_func;

    nthOfType = new AtomicString("nth-of-type(");
    if (!nthOfType)
        goto exit_func;

    nthLastChild = new AtomicString("nth-last-child(");
    if (!nthLastChild)
        goto exit_func;

    nthLastOfType = new AtomicString("nth-last-of-type(");
    if (!nthLastOfType)
        goto exit_func;

    focus = new AtomicString("focus");
    if (!focus)
        goto exit_func;

    hover = new AtomicString("hover");
    if (!hover)
        goto exit_func;

    indeterminate = new AtomicString("indeterminate");
    if (!indeterminate)
        goto exit_func;

    innerSpinButton = new AtomicString("-webkit-inner-spin-button");
    if (!innerSpinButton)
        goto exit_func;

#if ENABLE(DATALIST)
    inputListButton = new AtomicString("-webkit-input-list-button");
    if (!inputListButton)
        goto exit_func;
#endif

    inputPlaceholder = new AtomicString("-webkit-input-placeholder");
    if (!inputPlaceholder)
        goto exit_func;

    lastChild = new AtomicString("last-child");
    if (!lastChild)
        goto exit_func;

    lastOfType = new AtomicString("last-of-type");
    if (!lastOfType)
        goto exit_func;

    link = new AtomicString("link");
    if (!link)
        goto exit_func;

    lang = new AtomicString("lang(");
    if (!lang)
        goto exit_func;

    mediaControlsPanel = new AtomicString("-webkit-media-controls-panel");
    if (!mediaControlsPanel)
        goto exit_func;

    mediaControlsMuteButton = new AtomicString("-webkit-media-controls-mute-button");
    if (!mediaControlsMuteButton)
        goto exit_func;

    mediaControlsPlayButton = new AtomicString("-webkit-media-controls-play-button");
    if (!mediaControlsPlayButton)
        goto exit_func;

    mediaControlsTimeline = new AtomicString("-webkit-media-controls-timeline");
    if (!mediaControlsTimeline)
        goto exit_func;

    mediaControlsVolumeSlider = new AtomicString("-webkit-media-controls-volume-slider");
    if (!mediaControlsVolumeSlider)
        goto exit_func;

    mediaControlsSeekBackButton = new AtomicString("-webkit-media-controls-seek-back-button");
    if (!mediaControlsSeekBackButton)
        goto exit_func;

    mediaControlsSeekForwardButton = new AtomicString("-webkit-media-controls-seek-forward-button");
    if (!mediaControlsSeekForwardButton)
        goto exit_func;

    mediaControlsRewindButton = new AtomicString("-webkit-media-controls-rewind-button");
    if (!mediaControlsRewindButton)
        goto exit_func;

    mediaControlsReturnToRealtimeButton = new AtomicString("-webkit-media-controls-return-to-realtime-button");
    if (!mediaControlsReturnToRealtimeButton)
        goto exit_func;

    mediaControlsToggleClosedCaptionsButton = new AtomicString("-webkit-media-controls-toggle-closed-captions-button");
    if (!mediaControlsToggleClosedCaptionsButton)
        goto exit_func;

    mediaControlsStatusDisplay = new AtomicString("-webkit-media-controls-status-display");
    if (!mediaControlsStatusDisplay)
        goto exit_func;

    mediaControlsFullscreenButton = new AtomicString("-webkit-media-controls-fullscreen-button");
    if (!mediaControlsFullscreenButton)
        goto exit_func;

    mediaControlsTimelineContainer = new AtomicString("-webkit-media-controls-timeline-container");
    if (!mediaControlsTimelineContainer)
        goto exit_func;

    mediaControlsVolumeSliderContainer = new AtomicString("-webkit-media-controls-volume-slider-container");
    if (!mediaControlsVolumeSliderContainer)
        goto exit_func;

    mediaControlsCurrentTimeDisplay = new AtomicString("-webkit-media-controls-current-time-display");
    if (!mediaControlsCurrentTimeDisplay)
        goto exit_func;

    mediaControlsTimeRemainingDisplay = new AtomicString("-webkit-media-controls-time-remaining-display");
    if (!mediaControlsTimeRemainingDisplay)
        goto exit_func;

    notStr = new AtomicString("not(");
    if (!notStr)
        goto exit_func;

    onlyChild = new AtomicString("only-child");
    if (!onlyChild)
        goto exit_func;

    onlyOfType = new AtomicString("only-of-type");
    if (!onlyOfType)
        goto exit_func;

    optional = new AtomicString("optional");
    if (!optional)
        goto exit_func;

    outerSpinButton = new AtomicString("-webkit-outer-spin-button");
    if (!outerSpinButton)
        goto exit_func;

    required = new AtomicString("required");
    if (!required)
        goto exit_func;

    resizer = new AtomicString("-webkit-resizer");
    if (!resizer)
        goto exit_func;

    root = new AtomicString("root");
    if (!root)
        goto exit_func;

    scrollbar = new AtomicString("-webkit-scrollbar");
    if (!scrollbar)
        goto exit_func;

    scrollbarButton = new AtomicString("-webkit-scrollbar-button");
    if (!scrollbarButton)
        goto exit_func;

    scrollbarCorner = new AtomicString("-webkit-scrollbar-corner");
    if (!scrollbarCorner)
        goto exit_func;

    scrollbarThumb = new AtomicString("-webkit-scrollbar-thumb");
    if (!scrollbarThumb)
        goto exit_func;

    scrollbarTrack = new AtomicString("-webkit-scrollbar-track");
    if (!scrollbarTrack)
        goto exit_func;

    scrollbarTrackPiece = new AtomicString("-webkit-scrollbar-track-piece");
    if (!scrollbarTrackPiece)
        goto exit_func;

    searchCancelButton = new AtomicString("-webkit-search-cancel-button");
    if (!searchCancelButton)
        goto exit_func;

    searchDecoration = new AtomicString("-webkit-search-decoration");
    if (!searchDecoration)
        goto exit_func;

    searchResultsDecoration = new AtomicString("-webkit-search-results-decoration");
    if (!searchResultsDecoration)
        goto exit_func;

    searchResultsButton = new AtomicString("-webkit-search-results-button");
    if (!searchResultsButton)
        goto exit_func;

    selection = new AtomicString("selection");
    if (!selection)
        goto exit_func;

    sliderThumb = new AtomicString("-webkit-slider-thumb");
    if (!sliderThumb)
        goto exit_func;

    target = new AtomicString("target");
    if (!target)
        goto exit_func;

    visited = new AtomicString("visited");
    if (!visited)
        goto exit_func;

    windowInactive = new AtomicString("window-inactive");
    if (!windowInactive)
        goto exit_func;

    decrement = new AtomicString("decrement");
    if (!decrement)
        goto exit_func;

    increment = new AtomicString("increment");
    if (!increment)
        goto exit_func;

    start = new AtomicString("start");
    if (!start)
        goto exit_func;

    end = new AtomicString("end");
    if (!end)
        goto exit_func;

    horizontal = new AtomicString("horizontal");
    if (!horizontal)
        goto exit_func;

    vertical = new AtomicString("vertical");
    if (!vertical)
        goto exit_func;

    doubleButton = new AtomicString("double-button");
    if (!doubleButton)
        goto exit_func;

    singleButton = new AtomicString("single-button");
    if (!singleButton)
        goto exit_func;

    noButton = new AtomicString("no-button");
    if (!noButton)
        goto exit_func;

    cornerPresent = new AtomicString("corner-present");
    if (!cornerPresent)
        goto exit_func;

    ret = true;

    gCSSSelectorStringInit = true;

exit_func:
    if (!ret)
        deleteSharedInstance();

    return ret;
}

/* static */void CSSSelector::resetVariables()
{
    active = 0;
    after = 0;
    anyLink = 0;
    autofill = 0;
    before = 0;
    checked = 0;
    fileUploadButton = 0;
    defaultString = 0;
    disabled = 0;
    readOnly = 0;
    readWrite = 0;
    valid = 0;
    invalid = 0;
    drag = 0;
    dragAlias = 0; // was documented with this name in Apple documentation, so keep an alia
    empty = 0;
    enabled = 0;
    firstChild = 0;
    firstLetter = 0;
    firstLine = 0;
    firstOfType = 0;
    fullPageMedia = 0;
    nthChild = 0;
    nthOfType = 0;
    nthLastChild = 0;
    nthLastOfType = 0;
    focus = 0;
    hover = 0;
    indeterminate = 0;
    innerSpinButton = 0;
#if ENABLE(DATALIST)
    inputListButton = 0;
#endif
    inputPlaceholder = 0;
    lastChild = 0;
    lastOfType = 0;
    link = 0;
    lang = 0;
    mediaControlsPanel = 0;
    mediaControlsMuteButton = 0;
    mediaControlsPlayButton = 0;
    mediaControlsTimeline = 0;
    mediaControlsVolumeSlider = 0;
    mediaControlsSeekBackButton = 0;
    mediaControlsSeekForwardButton = 0;
    mediaControlsRewindButton = 0;
    mediaControlsReturnToRealtimeButton = 0;
    mediaControlsToggleClosedCaptionsButton = 0;
    mediaControlsStatusDisplay = 0;
    mediaControlsFullscreenButton = 0;
    mediaControlsTimelineContainer = 0;
    mediaControlsVolumeSliderContainer = 0;
    mediaControlsCurrentTimeDisplay = 0;
    mediaControlsTimeRemainingDisplay = 0;
    notStr = 0;
    onlyChild = 0;
    onlyOfType = 0;
    optional = 0;
    outerSpinButton = 0;
    required = 0;
    resizer = 0;
    root = 0;
    scrollbar = 0;
    scrollbarButton = 0;
    scrollbarCorner = 0;
    scrollbarThumb = 0;
    scrollbarTrack = 0;
    scrollbarTrackPiece = 0;
    searchCancelButton = 0;
    searchDecoration = 0;
    searchResultsDecoration = 0;
    searchResultsButton = 0;
    selection = 0;
    sliderThumb = 0;
    target = 0;
    visited = 0;
    windowInactive = 0;
    decrement = 0;
    increment = 0;
    start = 0;
    end = 0;
    horizontal = 0;
    vertical = 0;
    doubleButton = 0;
    singleButton = 0;
    noButton = 0;
    cornerPresent = 0;

    gCSSSelectorStringInit = false;
}

/* static */void CSSSelector::deleteSharedInstance()
{
    delete active;
    delete after;
    delete anyLink;
    delete autofill;
    delete before;
    delete checked;
    delete fileUploadButton;
    delete defaultString;
    delete disabled;
    delete readOnly;
    delete readWrite;
    delete valid;
    delete invalid;
    delete drag;
    delete dragAlias; // was documented with this name in Apple documentation, so keep an alia
    delete empty;
    delete enabled;
    delete firstChild;
    delete firstLetter;
    delete firstLine;
    delete firstOfType;
    delete fullPageMedia;
    delete nthChild;
    delete nthOfType;
    delete nthLastChild;
    delete nthLastOfType;
    delete focus;
    delete hover;
    delete indeterminate;
    delete innerSpinButton;
#if ENABLE(DATALIST)
    delete inputListButton;
#endif
    delete inputPlaceholder;
    delete lastChild;
    delete lastOfType;
    delete link;
    delete lang;
    delete mediaControlsPanel;
    delete mediaControlsMuteButton;
    delete mediaControlsPlayButton;
    delete mediaControlsTimeline;
    delete mediaControlsVolumeSlider;
    delete mediaControlsSeekBackButton;
    delete mediaControlsSeekForwardButton;
    delete mediaControlsRewindButton;
    delete mediaControlsReturnToRealtimeButton;
    delete mediaControlsToggleClosedCaptionsButton;
    delete mediaControlsStatusDisplay;
    delete mediaControlsFullscreenButton;
    delete mediaControlsTimelineContainer;
    delete mediaControlsVolumeSliderContainer;
    delete mediaControlsCurrentTimeDisplay;
    delete mediaControlsTimeRemainingDisplay;
    delete notStr;
    delete onlyChild;
    delete onlyOfType;
    delete optional;
    delete outerSpinButton;
    delete required;
    delete resizer;
    delete root;
    delete scrollbar;
    delete scrollbarButton;
    delete scrollbarCorner;
    delete scrollbarThumb;
    delete scrollbarTrack;
    delete scrollbarTrackPiece;
    delete searchCancelButton;
    delete searchDecoration;
    delete searchResultsDecoration;
    delete searchResultsButton;
    delete selection;
    delete sliderThumb;
    delete target;
    delete visited;
    delete windowInactive;
    delete decrement;
    delete increment;
    delete start;
    delete end;
    delete horizontal;
    delete vertical;
    delete doubleButton;
    delete singleButton;
    delete noButton;
    delete cornerPresent;

    active = 0;
    after = 0;
    anyLink = 0;
    autofill = 0;
    before = 0;
    checked = 0;
    fileUploadButton = 0;
    defaultString = 0;
    disabled = 0;
    readOnly = 0;
    readWrite = 0;
    valid = 0;
    invalid = 0;
    drag = 0;
    dragAlias = 0; // was documented with this name in Apple documentation, so keep an alia
    empty = 0;
    enabled = 0;
    firstChild = 0;
    firstLetter = 0;
    firstLine = 0;
    firstOfType = 0;
    fullPageMedia = 0;
    nthChild = 0;
    nthOfType = 0;
    nthLastChild = 0;
    nthLastOfType = 0;
    focus = 0;
    hover = 0;
    indeterminate = 0;
    innerSpinButton = 0;
#if ENABLE(DATALIST)
    inputListButton = 0;
#endif
    inputPlaceholder = 0;
    lastChild = 0;
    lastOfType = 0;
    link = 0;
    lang = 0;
    mediaControlsPanel = 0;
    mediaControlsMuteButton = 0;
    mediaControlsPlayButton = 0;
    mediaControlsTimeline = 0;
    mediaControlsVolumeSlider = 0;
    mediaControlsSeekBackButton = 0;
    mediaControlsSeekForwardButton = 0;
    mediaControlsRewindButton = 0;
    mediaControlsReturnToRealtimeButton = 0;
    mediaControlsToggleClosedCaptionsButton = 0;
    mediaControlsStatusDisplay = 0;
    mediaControlsFullscreenButton = 0;
    mediaControlsTimelineContainer = 0;
    mediaControlsVolumeSliderContainer = 0;
    mediaControlsCurrentTimeDisplay = 0;
    mediaControlsTimeRemainingDisplay = 0;
    notStr = 0;
    onlyChild = 0;
    onlyOfType = 0;
    optional = 0;
    outerSpinButton = 0;
    required = 0;
    resizer = 0;
    root = 0;
    scrollbar = 0;
    scrollbarButton = 0;
    scrollbarCorner = 0;
    scrollbarThumb = 0;
    scrollbarTrack = 0;
    scrollbarTrackPiece = 0;
    searchCancelButton = 0;
    searchDecoration = 0;
    searchResultsDecoration = 0;
    searchResultsButton = 0;
    selection = 0;
    sliderThumb = 0;
    target = 0;
    visited = 0;
    windowInactive = 0;
    decrement = 0;
    increment = 0;
    start = 0;
    end = 0;
    horizontal = 0;
    vertical = 0;
    doubleButton = 0;
    singleButton = 0;
    noButton = 0;
    cornerPresent = 0;

    gCSSSelectorStringInit = false;
}

} // namespace WebCore
