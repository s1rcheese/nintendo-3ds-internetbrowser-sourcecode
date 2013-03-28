/*
 *  WKCSettings.cpp
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

#include "config.h"
#include "WKCSettings.h"
#include "WKCWebViewPrivate.h"

#include "CString.h"
#include "Page.h"
#include "Settings.h"

#define PARENT() (((WebCore::Settings *)m_private))

namespace WKC {

WKCSettings::WKCSettings(WKC::WKCWebViewPrivate* parent)
    : m_private((WKCSettingsPrivate *)parent->core()->settings())
{
}

WKCSettings::~WKCSettings()
{
}


WKCSettings::LayoutAlgorithm WKCSettings::layoutAlgorithm() const
{
#if ENABLE(WKC_ANDROID_LAYOUT)
    return (WKCSettings::LayoutAlgorithm)PARENT()->layoutAlgorithm();
#else
    return kLayoutNormal;
#endif
}

void WKCSettings::setLayoutAlgorithm(WKCSettings::LayoutAlgorithm algorithm)
{
#if ENABLE(WKC_ANDROID_LAYOUT)
    PARENT()->setLayoutAlgorithm((WebCore::Settings::LayoutAlgorithm)algorithm);
#endif
}


bool WKCSettings::useWideViewport() const
{
#if ENABLE(WKC_ANDROID_LAYOUT)
    return PARENT()->useWideViewport();
#else
    return false;
#endif
}

void WKCSettings::setUseWideViewport(bool use)
{
#if ENABLE(WKC_ANDROID_LAYOUT)
    PARENT()->setUseWideViewport(use);
#endif
}

void WKCSettings::setFlatFrameSetLayoutEnabled(bool arg)
{
#if ENABLE(WKC_FRAME_FLATTENING)
    PARENT()->setFlatFrameSetLayoutEnabled(arg);
#endif
}

bool WKCSettings::flatFrameSetLayoutEnabled() const
{
#if ENABLE(WKC_FRAME_FLATTENING)
    return PARENT()->flatFrameSetLayoutEnabled();
#else
    return false;
#endif
}

void WKCSettings::setStandardFontFamily(const char* arg)
{
    PARENT()->setStandardFontFamily(arg);
}

const char* WKCSettings::standardFontFamily()
{
    return PARENT()->standardFontFamily().string().utf8().data();
}


void WKCSettings::setFixedFontFamily(const char* arg)
{
    PARENT()->setFixedFontFamily(arg);
}

const char* WKCSettings::fixedFontFamily() const
{
    return PARENT()->fixedFontFamily().string().utf8().data();
}


void WKCSettings::setSerifFontFamily(const char* arg)
{
    PARENT()->setSerifFontFamily(arg);
}

const char* WKCSettings::serifFontFamily()
{
    return PARENT()->serifFontFamily().string().utf8().data();
}


void WKCSettings::setSansSerifFontFamily(const char* arg)
{
    PARENT()->setSansSerifFontFamily(arg);
}

const char* WKCSettings::sansSerifFontFamily()
{
    return PARENT()->sansSerifFontFamily().string().utf8().data();
}


void WKCSettings::setCursiveFontFamily(const char* arg)
{
    PARENT()->setCursiveFontFamily(arg);
}

const char* WKCSettings::cursiveFontFamily() const
{
    return PARENT()->cursiveFontFamily().string().utf8().data();
}


void WKCSettings::setFantasyFontFamily(const char* arg)
{
    PARENT()->setFantasyFontFamily(arg);
}

const char* WKCSettings::fantasyFontFamily() const
{
    return PARENT()->fantasyFontFamily().string().utf8().data();
}


void WKCSettings::setMinimumFontSize(int arg)
{
    PARENT()->setMinimumFontSize(arg);
}

int WKCSettings::minimumFontSize() const
{
    return PARENT()->minimumFontSize();
}


void WKCSettings::setMinimumLogicalFontSize(int arg)
{
    PARENT()->setMinimumLogicalFontSize(arg);
}

int WKCSettings::minimumLogicalFontSize() const
{
    return PARENT()->minimumLogicalFontSize();
}


void WKCSettings::setDefaultFontSize(int arg)
{
    PARENT()->setDefaultFontSize(arg);
}

int WKCSettings::defaultFontSize() const
{
    return PARENT()->defaultFontSize();
}


void WKCSettings::setDefaultFixedFontSize(int arg)
{
    PARENT()->setDefaultFixedFontSize(arg);
}

int WKCSettings::defaultFixedFontSize() const
{
    return PARENT()->defaultFixedFontSize();
}


void WKCSettings::setLoadsImagesAutomatically(bool arg)
{
    PARENT()->setLoadsImagesAutomatically(arg);
}

bool WKCSettings::loadsImagesAutomatically() const
{
    return PARENT()->loadsImagesAutomatically();
}


void WKCSettings::setJavaScriptEnabled(bool arg)
{
    PARENT()->setJavaScriptEnabled(arg);
}

bool WKCSettings::isJavaScriptEnabled() const
{
    return PARENT()->isJavaScriptEnabled();
}


void WKCSettings::setWebSecurityEnabled(bool arg)
{
    PARENT()->setWebSecurityEnabled(arg);
}

bool WKCSettings::isWebSecurityEnabled() const
{
    return PARENT()->isWebSecurityEnabled();
}


void WKCSettings::setAllowUniversalAccessFromFileURLs(bool arg)
{
    PARENT()->setAllowUniversalAccessFromFileURLs(arg);
}

bool WKCSettings::allowUniversalAccessFromFileURLs() const
{
    return PARENT()->allowUniversalAccessFromFileURLs();
}


void WKCSettings::setJavaScriptCanOpenWindowsAutomatically(bool arg)
{
    PARENT()->setJavaScriptCanOpenWindowsAutomatically(arg);
}

bool WKCSettings::javaScriptCanOpenWindowsAutomatically() const
{
    return PARENT()->javaScriptCanOpenWindowsAutomatically();
}


#if 1
// added at webkit.org trunk r58703 (modification is changed for this platform)
void WKCSettings::setJavaScriptCanAccessClipboard(bool arg)
{
    PARENT()->setJavaScriptCanAccessClipboard(arg);
}

bool WKCSettings::javaScriptCanAccessClipboard() const
{
    return PARENT()->javaScriptCanAccessClipboard();
}
#endif


void WKCSettings::setSpatialNavigationEnabled(bool arg)
{
    PARENT()->setSpatialNavigationEnabled(arg);
}

bool WKCSettings::isSpatialNavigationEnabled() const
{
    return PARENT()->isSpatialNavigationEnabled();
}


void WKCSettings::setJavaEnabled(bool arg)
{
    PARENT()->setJavaEnabled(arg);
}

bool WKCSettings::isJavaEnabled() const
{
    return PARENT()->isJavaEnabled();
}


void WKCSettings::setPluginsEnabled(bool arg)
{
    PARENT()->setPluginsEnabled(arg);
}

bool WKCSettings::arePluginsEnabled() const
{
    return PARENT()->arePluginsEnabled();
}


void WKCSettings::setDatabasesEnabled(bool arg)
{
    PARENT()->setDatabasesEnabled(arg);
}

bool WKCSettings::databasesEnabled() const
{
    return PARENT()->databasesEnabled();
}


void WKCSettings::setLocalStorageEnabled(bool arg)
{
    PARENT()->setLocalStorageEnabled(arg);
}

bool WKCSettings::localStorageEnabled() const
{
    return PARENT()->localStorageEnabled();
}


void WKCSettings::setLocalStorageQuota(unsigned arg)
{
    PARENT()->setLocalStorageQuota(arg);
}

unsigned WKCSettings::localStorageQuota() const
{
    return PARENT()->localStorageQuota();
}


void WKCSettings::setPrivateBrowsingEnabled(bool arg)
{
    PARENT()->setPrivateBrowsingEnabled(arg);
}

bool WKCSettings::privateBrowsingEnabled() const
{
    return PARENT()->privateBrowsingEnabled();
}


void WKCSettings::setCaretBrowsingEnabled(bool arg)
{
    PARENT()->setCaretBrowsingEnabled(arg);
}

bool WKCSettings::caretBrowsingEnabled() const
{
    return PARENT()->caretBrowsingEnabled();
}


void WKCSettings::setDefaultTextEncodingName(const char* arg)
{
    PARENT()->setDefaultTextEncodingName(arg);
}

const char* WKCSettings::defaultTextEncodingName()
{
    return PARENT()->defaultTextEncodingName().utf8().data();
}


void WKCSettings::setUsesEncodingDetector(bool arg)
{
    PARENT()->setUsesEncodingDetector(arg);
}

bool WKCSettings::usesEncodingDetector() const
{
    return PARENT()->usesEncodingDetector();
}


void WKCSettings::setUserStyleSheetLocation(const char* arg)
{
    WebCore::KURL location(WebCore::KURL(),arg);
    PARENT()->setUserStyleSheetLocation(location);
}

const char* WKCSettings::userStyleSheetLocation() const
{
    return PARENT()->userStyleSheetLocation().string().utf8().data();
}


void WKCSettings::setShouldPrintBackgrounds(bool arg)
{
    PARENT()->setShouldPrintBackgrounds(arg);
}

bool WKCSettings::shouldPrintBackgrounds() const
{
    return PARENT()->shouldPrintBackgrounds();
}


void WKCSettings::setTextAreasAreResizable(bool arg)
{
    PARENT()->setTextAreasAreResizable(arg);
}

bool WKCSettings::textAreasAreResizable() const
{
    return PARENT()->textAreasAreResizable();
}


void WKCSettings::setEditableLinkBehavior(WKCSettings::EditableLinkBehavior arg)
{
    PARENT()->setEditableLinkBehavior((WebCore::EditableLinkBehavior)arg);
}

WKCSettings::EditableLinkBehavior WKCSettings::editableLinkBehavior()
{
    return (WKCSettings::EditableLinkBehavior)PARENT()->editableLinkBehavior();
}


void WKCSettings::setTextDirectionSubmenuInclusionBehavior(WKCSettings::TextDirectionSubmenuInclusionBehavior arg)
{
    PARENT()->setTextDirectionSubmenuInclusionBehavior((WebCore::TextDirectionSubmenuInclusionBehavior)arg);
}

WKCSettings::TextDirectionSubmenuInclusionBehavior WKCSettings::textDirectionSubmenuInclusionBehavior() const
{
    return (WKCSettings::TextDirectionSubmenuInclusionBehavior)PARENT()->textDirectionSubmenuInclusionBehavior();
}


void WKCSettings::setNeedsAdobeFrameReloadingQuirk(bool arg)
{
    PARENT()->setNeedsAdobeFrameReloadingQuirk(arg);
}

bool WKCSettings::needsAcrobatFrameReloadingQuirk() const
{
    return PARENT()->needsAcrobatFrameReloadingQuirk();
}


void WKCSettings::setNeedsKeyboardEventDisambiguationQuirks(bool arg)
{
    PARENT()->setNeedsKeyboardEventDisambiguationQuirks(arg);
}

bool WKCSettings::needsKeyboardEventDisambiguationQuirks() const
{
    return PARENT()->needsKeyboardEventDisambiguationQuirks();
}


void WKCSettings::setTreatsAnyTextCSSLinkAsStylesheet(bool arg)
{
    PARENT()->setTreatsAnyTextCSSLinkAsStylesheet(arg);
}

bool WKCSettings::treatsAnyTextCSSLinkAsStylesheet() const
{
    return PARENT()->treatsAnyTextCSSLinkAsStylesheet();
}


void WKCSettings::setNeedsLeopardMailQuirks(bool arg)
{
    PARENT()->setNeedsLeopardMailQuirks(arg);
}

bool WKCSettings::needsLeopardMailQuirks() const
{
    return PARENT()->needsLeopardMailQuirks();
}


void WKCSettings::setNeedsTigerMailQuirks(bool arg)
{
    PARENT()->setNeedsTigerMailQuirks(arg);
}

bool WKCSettings::needsTigerMailQuirks() const
{
    return PARENT()->needsTigerMailQuirks();
}


void WKCSettings::setDOMPasteAllowed(bool arg)
{
    PARENT()->setDOMPasteAllowed(arg);
}

bool WKCSettings::isDOMPasteAllowed() const
{
    return PARENT()->isDOMPasteAllowed();
}


void WKCSettings::setUsesPageCache(bool arg)
{
    PARENT()->setUsesPageCache(arg);
}

bool WKCSettings::usesPageCache() const
{
    return PARENT()->usesPageCache();
}


void WKCSettings::setShrinksStandaloneImagesToFit(bool arg)
{
    PARENT()->setShrinksStandaloneImagesToFit(arg);
}

bool WKCSettings::shrinksStandaloneImagesToFit() const
{
    return PARENT()->shrinksStandaloneImagesToFit();
}


void WKCSettings::setShowsURLsInToolTips(bool arg)
{
    PARENT()->setShowsURLsInToolTips(arg);
}

bool WKCSettings::showsURLsInToolTips() const
{
    return PARENT()->showsURLsInToolTips();
}


void WKCSettings::setFTPDirectoryTemplatePath(const char* arg)
{
    PARENT()->setFTPDirectoryTemplatePath(arg);
}

const char* WKCSettings::ftpDirectoryTemplatePath()
{
    return PARENT()->ftpDirectoryTemplatePath().utf8().data();
}


void WKCSettings::setForceFTPDirectoryListings(bool arg)
{
    PARENT()->setForceFTPDirectoryListings(arg);
}

bool WKCSettings::forceFTPDirectoryListings() const
{
    return PARENT()->forceFTPDirectoryListings();
}


void WKCSettings::setDeveloperExtrasEnabled(bool arg)
{
    PARENT()->setDeveloperExtrasEnabled(arg);
}

bool WKCSettings::developerExtrasEnabled() const
{
    return PARENT()->developerExtrasEnabled();
}


void WKCSettings::resetMetadataSettings()
{
}

void WKCSettings::setMetadataSettings(const char* key, const char* value)
{
    PARENT()->setMetadataSettings(key, value);
}


int WKCSettings::viewportWidth() const
{
    return PARENT()->viewportWidth();
}

int WKCSettings::viewportHeight() const
{
    return PARENT()->viewportHeight();
}

int WKCSettings::viewportInitialScale() const
{
    return PARENT()->viewportInitialScale();
}

int WKCSettings::viewportMinimumScale() const
{
    return PARENT()->viewportMinimumScale();
}

int WKCSettings::viewportMaximumScale() const
{
    return PARENT()->viewportMaximumScale();
}

bool WKCSettings::viewportUserScalable() const
{
    return PARENT()->viewportUserScalable();
}


void WKCSettings::setAuthorAndUserStylesEnabled(bool arg)
{
    PARENT()->setAuthorAndUserStylesEnabled(arg);
}

bool WKCSettings::authorAndUserStylesEnabled() const
{
    return PARENT()->authorAndUserStylesEnabled();
}


void WKCSettings::setFontRenderingMode(FontRenderingMode mode)
{
    PARENT()->setFontRenderingMode((WebCore::FontRenderingMode)mode);
}

WKCSettings::FontRenderingMode WKCSettings::fontRenderingMode() const
{
    return (WKCSettings::FontRenderingMode)PARENT()->fontRenderingMode();
}


void WKCSettings::setNeedsSiteSpecificQuirks(bool arg)
{
    PARENT()->setNeedsSiteSpecificQuirks(arg);
}

bool WKCSettings::needsSiteSpecificQuirks() const
{
    return PARENT()->needsSiteSpecificQuirks();
}


void WKCSettings::setWebArchiveDebugModeEnabled(bool arg)
{
    PARENT()->setWebArchiveDebugModeEnabled(arg);
}

bool WKCSettings::webArchiveDebugModeEnabled() const
{
    return PARENT()->webArchiveDebugModeEnabled();
}


void WKCSettings::setLocalFileContentSniffingEnabled(bool arg)
{
    PARENT()->setLocalFileContentSniffingEnabled(arg);
}

bool WKCSettings::localFileContentSniffingEnabled() const
{
    return PARENT()->localFileContentSniffingEnabled();
}


void WKCSettings::setLocalStorageDatabasePath(const char* arg)
{
    PARENT()->setLocalStorageDatabasePath(arg);
}

const char* WKCSettings::localStorageDatabasePath() const
{
    return PARENT()->localStorageDatabasePath().utf8().data();
}


void WKCSettings::setApplicationChromeMode(bool arg)
{
    PARENT()->setApplicationChromeMode(arg);
}

bool WKCSettings::inApplicationChromeMode() const
{
    return PARENT()->inApplicationChromeMode();
}


void WKCSettings::setOfflineWebApplicationCacheEnabled(bool arg)
{
    PARENT()->setOfflineWebApplicationCacheEnabled(arg);
}

bool WKCSettings::offlineWebApplicationCacheEnabled() const
{
    return PARENT()->offlineWebApplicationCacheEnabled();
}


void WKCSettings::setShouldPaintCustomScrollbars(bool arg)
{
    PARENT()->setShouldPaintCustomScrollbars(arg);
}

bool WKCSettings::shouldPaintCustomScrollbars() const
{
    return PARENT()->shouldPaintCustomScrollbars();
}


void WKCSettings::setZoomsTextOnly(bool arg)
{
    PARENT()->setZoomsTextOnly(arg);
}

bool WKCSettings::zoomsTextOnly() const
{
    return PARENT()->zoomsTextOnly();
}


void WKCSettings::setEnforceCSSMIMETypeInStrictMode(bool arg)
{
    PARENT()->setEnforceCSSMIMETypeInStrictMode(arg);
}

bool WKCSettings::enforceCSSMIMETypeInStrictMode()
{
    return PARENT()->enforceCSSMIMETypeInStrictMode();
}


void WKCSettings::setMaximumDecodedImageSize(size_t size)
{
    PARENT()->setMaximumDecodedImageSize(size);
}

size_t WKCSettings::maximumDecodedImageSize() const
{
    return PARENT()->maximumDecodedImageSize();
}


void WKCSettings::setAllowScriptsToCloseWindows(bool arg)
{
    PARENT()->setAllowScriptsToCloseWindows(arg);
}

bool WKCSettings::allowScriptsToCloseWindows() const
{
    return PARENT()->allowScriptsToCloseWindows();
}


void WKCSettings::setEditingBehavior(EditingBehavior behavior)
{
    PARENT()->setEditingBehavior((WebCore::EditingBehavior)behavior);
}

WKCSettings::EditingBehavior WKCSettings::editingBehavior() const
{
    return (WKCSettings::EditingBehavior)PARENT()->editingBehavior();
}


void WKCSettings::setDownloadableBinaryFontsEnabled(bool arg)
{
    PARENT()->setDownloadableBinaryFontsEnabled(arg);
}

bool WKCSettings::downloadableBinaryFontsEnabled() const
{
    return PARENT()->downloadableBinaryFontsEnabled();
}


void WKCSettings::setXSSAuditorEnabled(bool arg)
{
    PARENT()->setXSSAuditorEnabled(arg);
}

bool WKCSettings::xssAuditorEnabled() const
{
    return PARENT()->xssAuditorEnabled();
}


void WKCSettings::setAcceleratedCompositingEnabled(bool arg)
{
    PARENT()->setAcceleratedCompositingEnabled(arg);
}

bool WKCSettings::acceleratedCompositingEnabled() const
{
    return PARENT()->acceleratedCompositingEnabled();
}


void WKCSettings::setShowDebugBorders(bool arg)
{
    PARENT()->setShowDebugBorders(arg);
}

bool WKCSettings::showDebugBorders() const
{
    return PARENT()->showDebugBorders();
}


void WKCSettings::setShowRepaintCounter(bool arg)
{
    PARENT()->setShowRepaintCounter(arg);
}

bool WKCSettings::showRepaintCounter() const
{
    return PARENT()->showRepaintCounter();
}


void WKCSettings::setExperimentalNotificationsEnabled(bool arg)
{
    PARENT()->setExperimentalNotificationsEnabled(arg);
}

bool WKCSettings::experimentalNotificationsEnabled() const
{
    return PARENT()->experimentalNotificationsEnabled();
}


void WKCSettings::setPluginAllowedRunTime(unsigned arg)
{
    PARENT()->setPluginAllowedRunTime(arg);
}

unsigned WKCSettings::pluginAllowedRunTime() const
{
    return PARENT()->pluginAllowedRunTime();
}


void WKCSettings::setWebGLEnabled(bool arg)
{
    PARENT()->setWebGLEnabled(arg);
}

bool WKCSettings::webGLEnabled() const
{
    return PARENT()->webGLEnabled();
}


void WKCSettings::setGeolocationEnabled(bool arg)
{
    PARENT()->setGeolocationEnabled(arg);
}

bool WKCSettings::geolocationEnabled() const
{
    return PARENT()->geolocationEnabled();
}


}

namespace WKC {

WKCGlobalSettings::WKCGlobalSettings()
{
}

WKCGlobalSettings::~WKCGlobalSettings()
{
}

WKCGlobalSettings* WKCGlobalSettings::create()
{
    WKCGlobalSettings *self;

    self = new WKCGlobalSettings();
    if (!self)
        return 0;

    if (!self->construct()) {
        delete self;
        return 0;
    }

    return self;
}

bool WKCGlobalSettings::construct()
{
    return true;
}

WKCGlobalSettings* WKCGlobalSettings::m_sharedInstance = 0;
bool WKCGlobalSettings::m_autoFlag = false;

bool WKCGlobalSettings::createSharedInstance(bool autoflag)
{
    if (m_sharedInstance)
        return true;

    m_sharedInstance = create();
    if (!m_sharedInstance)
        return false;

    m_autoFlag = autoflag;

    return true;
}

void WKCGlobalSettings::deleteSharedInstance()
{
    if (m_sharedInstance)
        delete m_sharedInstance;

    m_sharedInstance = 0;
}

bool WKCGlobalSettings::isExistSharedInstance()
{
    return m_sharedInstance ? true : false;
}

bool WKCGlobalSettings::isAutomatic()
{
    return m_autoFlag;
}

}   // namespace
