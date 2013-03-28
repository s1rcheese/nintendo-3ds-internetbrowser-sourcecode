/*
    WKCResetROMTables.cpp

    Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the
    Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
    Boston, MA  02110-1301, USA.
*/

#include "config.h"

#include <JSAbstractWorker.h>
#include <JSAttr.h>
#include <JSBarInfo.h>
#include <JSBeforeLoadEvent.h>
#include <JSCanvasGradient.h>
#include <JSCanvasPattern.h>
#include <JSCanvasRenderingContext.h>
#include <JSCanvasRenderingContext2D.h>
#include <JSCDATASection.h>
#include <JSCharacterData.h>
#include <JSClientRect.h>
#include <JSClientRectList.h>
#include <JSClipboard.h>
#include <JSComment.h>
#include <JSCompositionEvent.h>
#include <JSConsole.h>
#include <JSCoordinates.h>
#include <JSCounter.h>
#include <JSCSSCharsetRule.h>
#include <JSCSSFontFaceRule.h>
#include <JSCSSImportRule.h>
#include <JSCSSMediaRule.h>
#include <JSCSSPageRule.h>
#include <JSCSSPrimitiveValue.h>
#include <JSCSSRule.h>
#include <JSCSSRuleList.h>
#include <JSCSSStyleDeclaration.h>
#include <JSCSSStyleRule.h>
#include <JSCSSStyleSheet.h>
#include <JSCSSValue.h>
#include <JSCSSValueList.h>
#include <JSCSSVariablesDeclaration.h>
#include <JSCSSVariablesRule.h>
#include <JSDataGridColumn.h>
#include <JSDataGridColumnList.h>
#include <JSDedicatedWorkerContext.h>
#include <JSDocument.h>
#include <JSDocumentFragment.h>
#include <JSDocumentType.h>
#include <JSDOMCoreException.h>
#include <JSDOMImplementation.h>
#include <JSDOMParser.h>
#include <JSDOMSelection.h>
#include <JSDOMWindow.h>
#include <JSElement.h>
#include <JSEntity.h>
#include <JSEntityReference.h>
#include <JSErrorEvent.h>
#include <JSEvent.h>
#include <JSEventException.h>
#include <JSEventSource.h>
#include <JSFile.h>
#include <JSFileList.h>
#include <JSGeolocation.h>
#include <JSGeoposition.h>
#include <JSHistory.h>
#include <JSHTMLAllCollection.h>
#include <JSHTMLAnchorElement.h>
#include <JSHTMLAppletElement.h>
#include <JSHTMLAreaElement.h>
#include <JSHTMLAudioElement.h>
#include <JSHTMLBaseElement.h>
#include <JSHTMLBaseFontElement.h>
#include <JSHTMLBlockquoteElement.h>
#include <JSHTMLBodyElement.h>
#include <JSHTMLBRElement.h>
#include <JSHTMLButtonElement.h>
#include <JSHTMLCanvasElement.h>
#include <JSHTMLCollection.h>
#include <JSHTMLDataGridCellElement.h>
#include <JSHTMLDataGridColElement.h>
#include <JSHTMLDataGridElement.h>
#include <JSHTMLDataGridRowElement.h>
#include <JSHTMLDataListElement.h>
#include <JSHTMLDirectoryElement.h>
#include <JSHTMLDivElement.h>
#include <JSHTMLDListElement.h>
#include <JSHTMLDocument.h>
#include <JSHTMLElement.h>
#include <JSHTMLEmbedElement.h>
#include <JSHTMLFieldSetElement.h>
#include <JSHTMLFontElement.h>
#include <JSHTMLFormElement.h>
#include <JSHTMLFrameElement.h>
#include <JSHTMLFrameSetElement.h>
#include <JSHTMLHeadElement.h>
#include <JSHTMLHeadingElement.h>
#include <JSHTMLHRElement.h>
#include <JSHTMLHtmlElement.h>
#include <JSHTMLIFrameElement.h>
#include <JSHTMLImageElement.h>
#include <JSHTMLInputElement.h>
#include <JSHTMLIsIndexElement.h>
#include <JSHTMLLabelElement.h>
#include <JSHTMLLegendElement.h>
#include <JSHTMLLIElement.h>
#include <JSHTMLLinkElement.h>
#include <JSHTMLMapElement.h>
#include <JSHTMLMarqueeElement.h>
#include <JSHTMLMediaElement.h>
#include <JSHTMLMenuElement.h>
#include <JSHTMLMetaElement.h>
#include <JSHTMLModElement.h>
#include <JSHTMLObjectElement.h>
#include <JSHTMLOListElement.h>
#include <JSHTMLOptGroupElement.h>
#include <JSHTMLOptionElement.h>
#include <JSHTMLOptionsCollection.h>
#include <JSHTMLParagraphElement.h>
#include <JSHTMLParamElement.h>
#include <JSHTMLPreElement.h>
#include <JSHTMLQuoteElement.h>
#include <JSHTMLScriptElement.h>
#include <JSHTMLSelectElement.h>
#include <JSHTMLSourceElement.h>
#include <JSHTMLStyleElement.h>
#include <JSHTMLTableCaptionElement.h>
#include <JSHTMLTableCellElement.h>
#include <JSHTMLTableColElement.h>
#include <JSHTMLTableElement.h>
#include <JSHTMLTableRowElement.h>
#include <JSHTMLTableSectionElement.h>
#include <JSHTMLTextAreaElement.h>
#include <JSHTMLTitleElement.h>
#include <JSHTMLUListElement.h>
#include <JSHTMLVideoElement.h>
#include <JSImageData.h>
#include <JSInjectedScriptHost.h>
#include <JSInspectorBackend.h>
#include <JSInspectorFrontendHost.h>
#include <JSJavaScriptCallFrame.h>
#include <JSKeyboardEvent.h>
#include <JSLocation.h>
#include <JSMedia.h>
#include <JSMediaError.h>
#include <JSMediaList.h>
#include <JSMessageChannel.h>
#include <JSMessageEvent.h>
#include <JSMessagePort.h>
#include <JSMimeType.h>
#include <JSMimeTypeArray.h>
#include <JSMouseEvent.h>
#include <JSMutationEvent.h>
#include <JSNamedNodeMap.h>
#include <JSNavigator.h>
#include <JSNode.h>
#include <JSNodeFilter.h>
#include <JSNodeIterator.h>
#include <JSNodeList.h>
#include <JSNotation.h>
#include <JSNotification.h>
#include <JSNotificationCenter.h>
#include <JSOverflowEvent.h>
#include <JSPageTransitionEvent.h>
#include <JSPlugin.h>
#include <JSPluginArray.h>
#include <JSPopStateEvent.h>
#include <JSPositionError.h>
#include <JSProcessingInstruction.h>
#include <JSProgressEvent.h>
#include <JSRange.h>
#include <JSRangeException.h>
#include <JSRect.h>
#include <JSRGBColor.h>
#include <JSScreen.h>
#include <JSSharedWorker.h>
#include <JSSharedWorkerContext.h>
#include <JSStyleSheet.h>
#include <JSStyleSheetList.h>
#include <JSText.h>
#include <JSTextEvent.h>
#include <JSTextMetrics.h>
#include <JSTreeWalker.h>
#include <JSUIEvent.h>
#include <JSValidityState.h>
#include <JSVoidCallback.h>
#include <JSWebGLArray.h>
#include <JSWebGLArrayBuffer.h>
#include <JSWebGLByteArray.h>
#include <JSWebGLFloatArray.h>
#include <JSWebGLIntArray.h>
#include <JSWebGLRenderingContext.h>
#include <JSWebGLShortArray.h>
#include <JSWebGLUnsignedByteArray.h>
#include <JSWebGLUnsignedIntArray.h>
#include <JSWebGLUnsignedShortArray.h>
#include <JSWebKitAnimationEvent.h>
#include <JSWebKitCSSKeyframeRule.h>
#include <JSWebKitCSSKeyframesRule.h>
#include <JSWebKitCSSMatrix.h>
#include <JSWebKitCSSTransformValue.h>
#include <JSWebKitPoint.h>
#include <JSWebKitTransitionEvent.h>
#include <JSWheelEvent.h>
#include <JSWorker.h>
#include <JSWorkerContext.h>
#include <JSWorkerLocation.h>
#include <JSWorkerNavigator.h>
#include <JSXMLHttpRequest.h>
#include <JSXMLHttpRequestException.h>
#include <JSXMLHttpRequestProgressEvent.h>
#include <JSXMLHttpRequestUpload.h>
#include <JSXMLSerializer.h>
#include <JSXSLTProcessor.h>

namespace WKC {

void WKCWebKitResetROMTables()
{
#if ENABLE(WORKERS)
    WebCore::JSAbstractWorker_resetROMTables();
#endif
    WebCore::JSAttr_resetROMTables();
    WebCore::JSBarInfo_resetROMTables();
    WebCore::JSBeforeLoadEvent_resetROMTables();
    WebCore::JSCanvasGradient_resetROMTables();
    WebCore::JSCanvasPattern_resetROMTables();
    WebCore::JSCanvasRenderingContext_resetROMTables();
    WebCore::JSCanvasRenderingContext2D_resetROMTables();
    WebCore::JSCDATASection_resetROMTables();
    WebCore::JSCharacterData_resetROMTables();
    WebCore::JSClientRect_resetROMTables();
    WebCore::JSClientRectList_resetROMTables();
    WebCore::JSClipboard_resetROMTables();
    WebCore::JSComment_resetROMTables();
    WebCore::JSCompositionEvent_resetROMTables();
    WebCore::JSConsole_resetROMTables();
    WebCore::JSCoordinates_resetROMTables();
    WebCore::JSCounter_resetROMTables();
    WebCore::JSCSSCharsetRule_resetROMTables();
    WebCore::JSCSSFontFaceRule_resetROMTables();
    WebCore::JSCSSImportRule_resetROMTables();
    WebCore::JSCSSMediaRule_resetROMTables();
    WebCore::JSCSSPageRule_resetROMTables();
    WebCore::JSCSSPrimitiveValue_resetROMTables();
    WebCore::JSCSSRule_resetROMTables();
    WebCore::JSCSSRuleList_resetROMTables();
    WebCore::JSCSSStyleDeclaration_resetROMTables();
    WebCore::JSCSSStyleRule_resetROMTables();
    WebCore::JSCSSStyleSheet_resetROMTables();
    WebCore::JSCSSValue_resetROMTables();
    WebCore::JSCSSValueList_resetROMTables();
    WebCore::JSCSSVariablesDeclaration_resetROMTables();
    WebCore::JSCSSVariablesRule_resetROMTables();
#if ENABLE(DATAGRID)
    WebCore::JSDataGridColumn_resetROMTables();
    WebCore::JSDataGridColumnList_resetROMTables();
#endif
#if ENABLE(WORKERS)
    WebCore::JSDedicatedWorkerContext_resetROMTables();
#endif
    WebCore::JSDocument_resetROMTables();
    WebCore::JSDocumentFragment_resetROMTables();
    WebCore::JSDocumentType_resetROMTables();
    WebCore::JSDOMCoreException_resetROMTables();
    WebCore::JSDOMImplementation_resetROMTables();
    WebCore::JSDOMParser_resetROMTables();
    WebCore::JSDOMSelection_resetROMTables();
    WebCore::JSDOMWindow_resetROMTables();
    WebCore::JSElement_resetROMTables();
    WebCore::JSEntity_resetROMTables();
    WebCore::JSEntityReference_resetROMTables();
#if ENABLE(WORKERS)
    WebCore::JSErrorEvent_resetROMTables();
#endif
    WebCore::JSEvent_resetROMTables();
    WebCore::JSEventException_resetROMTables();
#if ENABLE(EVENTSOURCE)
    WebCore::JSEventSource_resetROMTables();
#endif
    WebCore::JSFile_resetROMTables();
    WebCore::JSFileList_resetROMTables();
    WebCore::JSGeolocation_resetROMTables();
    WebCore::JSGeoposition_resetROMTables();
    WebCore::JSHistory_resetROMTables();
    WebCore::JSHTMLAllCollection_resetROMTables();
    WebCore::JSHTMLAnchorElement_resetROMTables();
    WebCore::JSHTMLAppletElement_resetROMTables();
    WebCore::JSHTMLAreaElement_resetROMTables();
#if ENABLE(VIDEO)
    WebCore::JSHTMLAudioElement_resetROMTables();
#endif
    WebCore::JSHTMLBaseElement_resetROMTables();
    WebCore::JSHTMLBaseFontElement_resetROMTables();
    WebCore::JSHTMLBlockquoteElement_resetROMTables();
    WebCore::JSHTMLBodyElement_resetROMTables();
    WebCore::JSHTMLBRElement_resetROMTables();
    WebCore::JSHTMLButtonElement_resetROMTables();
    WebCore::JSHTMLCanvasElement_resetROMTables();
    WebCore::JSHTMLCollection_resetROMTables();
#if ENABLE(DATAGRID)
    WebCore::JSHTMLDataGridCellElement_resetROMTables();
    WebCore::JSHTMLDataGridColElement_resetROMTables();
    WebCore::JSHTMLDataGridElement_resetROMTables();
    WebCore::JSHTMLDataGridRowElement_resetROMTables();
#endif
#if ENABLE(DATALIST)
    WebCore::JSHTMLDataListElement_resetROMTables();
#endif
    WebCore::JSHTMLDirectoryElement_resetROMTables();
    WebCore::JSHTMLDivElement_resetROMTables();
    WebCore::JSHTMLDListElement_resetROMTables();
    WebCore::JSHTMLDocument_resetROMTables();
    WebCore::JSHTMLElement_resetROMTables();
    WebCore::JSHTMLEmbedElement_resetROMTables();
    WebCore::JSHTMLFieldSetElement_resetROMTables();
    WebCore::JSHTMLFontElement_resetROMTables();
    WebCore::JSHTMLFormElement_resetROMTables();
    WebCore::JSHTMLFrameElement_resetROMTables();
    WebCore::JSHTMLFrameSetElement_resetROMTables();
    WebCore::JSHTMLHeadElement_resetROMTables();
    WebCore::JSHTMLHeadingElement_resetROMTables();
    WebCore::JSHTMLHRElement_resetROMTables();
    WebCore::JSHTMLHtmlElement_resetROMTables();
    WebCore::JSHTMLIFrameElement_resetROMTables();
    WebCore::JSHTMLImageElement_resetROMTables();
    WebCore::JSHTMLInputElement_resetROMTables();
    WebCore::JSHTMLIsIndexElement_resetROMTables();
    WebCore::JSHTMLLabelElement_resetROMTables();
    WebCore::JSHTMLLegendElement_resetROMTables();
    WebCore::JSHTMLLIElement_resetROMTables();
    WebCore::JSHTMLLinkElement_resetROMTables();
    WebCore::JSHTMLMapElement_resetROMTables();
    WebCore::JSHTMLMarqueeElement_resetROMTables();
#if ENABLE(VIDEO)
    WebCore::JSHTMLMediaElement_resetROMTables();
#endif
    WebCore::JSHTMLMenuElement_resetROMTables();
    WebCore::JSHTMLMetaElement_resetROMTables();
    WebCore::JSHTMLModElement_resetROMTables();
    WebCore::JSHTMLObjectElement_resetROMTables();
    WebCore::JSHTMLOListElement_resetROMTables();
    WebCore::JSHTMLOptGroupElement_resetROMTables();
    WebCore::JSHTMLOptionElement_resetROMTables();
    WebCore::JSHTMLOptionsCollection_resetROMTables();
    WebCore::JSHTMLParagraphElement_resetROMTables();
    WebCore::JSHTMLParamElement_resetROMTables();
    WebCore::JSHTMLPreElement_resetROMTables();
    WebCore::JSHTMLQuoteElement_resetROMTables();
    WebCore::JSHTMLScriptElement_resetROMTables();
    WebCore::JSHTMLSelectElement_resetROMTables();
#if ENABLE(VIDEO)
    WebCore::JSHTMLSourceElement_resetROMTables();
#endif
    WebCore::JSHTMLStyleElement_resetROMTables();
    WebCore::JSHTMLTableCaptionElement_resetROMTables();
    WebCore::JSHTMLTableCellElement_resetROMTables();
    WebCore::JSHTMLTableColElement_resetROMTables();
    WebCore::JSHTMLTableElement_resetROMTables();
    WebCore::JSHTMLTableRowElement_resetROMTables();
    WebCore::JSHTMLTableSectionElement_resetROMTables();
    WebCore::JSHTMLTextAreaElement_resetROMTables();
    WebCore::JSHTMLTitleElement_resetROMTables();
    WebCore::JSHTMLUListElement_resetROMTables();
#if ENABLE(VIDEO)
    WebCore::JSHTMLVideoElement_resetROMTables();
#endif
    WebCore::JSImageData_resetROMTables();
#if ENABLE(INSPECTOR)
    WebCore::JSInjectedScriptHost_resetROMTables();
    WebCore::JSInspectorBackend_resetROMTables();
    WebCore::JSInspectorFrontendHost_resetROMTables();
#endif
#if ENABLE(JAVASCRIPT_DEBUGGER)
    WebCore::JSJavaScriptCallFrame_resetROMTables();
#endif
    WebCore::JSKeyboardEvent_resetROMTables();
    WebCore::JSLocation_resetROMTables();
    WebCore::JSMedia_resetROMTables();
#if ENABLE(VIDEO)
    WebCore::JSMediaError_resetROMTables();
#endif
    WebCore::JSMediaList_resetROMTables();
    WebCore::JSMessageChannel_resetROMTables();
    WebCore::JSMessageEvent_resetROMTables();
    WebCore::JSMessagePort_resetROMTables();
    WebCore::JSMimeType_resetROMTables();
    WebCore::JSMimeTypeArray_resetROMTables();
    WebCore::JSMouseEvent_resetROMTables();
    WebCore::JSMutationEvent_resetROMTables();
    WebCore::JSNamedNodeMap_resetROMTables();
    WebCore::JSNavigator_resetROMTables();
    WebCore::JSNode_resetROMTables();
    WebCore::JSNodeFilter_resetROMTables();
    WebCore::JSNodeIterator_resetROMTables();
    WebCore::JSNodeList_resetROMTables();
    WebCore::JSNotation_resetROMTables();
#if ENABLE(NOTIFICATIONS)
    WebCore::JSNotification_resetROMTables();
    WebCore::JSNotificationCenter_resetROMTables();
#endif
    WebCore::JSOverflowEvent_resetROMTables();
    WebCore::JSPageTransitionEvent_resetROMTables();
    WebCore::JSPlugin_resetROMTables();
    WebCore::JSPluginArray_resetROMTables();
    WebCore::JSPopStateEvent_resetROMTables();
    WebCore::JSPositionError_resetROMTables();
    WebCore::JSProcessingInstruction_resetROMTables();
    WebCore::JSProgressEvent_resetROMTables();
    WebCore::JSRange_resetROMTables();
    WebCore::JSRangeException_resetROMTables();
    WebCore::JSRect_resetROMTables();
    WebCore::JSRGBColor_resetROMTables();
    WebCore::JSScreen_resetROMTables();
#if ENABLE(SHARED_WORKERS)
    WebCore::JSSharedWorker_resetROMTables();
    WebCore::JSSharedWorkerContext_resetROMTables();
#endif
    WebCore::JSStyleSheet_resetROMTables();
    WebCore::JSStyleSheetList_resetROMTables();
    WebCore::JSText_resetROMTables();
    WebCore::JSTextEvent_resetROMTables();
    WebCore::JSTextMetrics_resetROMTables();
    WebCore::JSTreeWalker_resetROMTables();
    WebCore::JSUIEvent_resetROMTables();
    WebCore::JSValidityState_resetROMTables();
    WebCore::JSVoidCallback_resetROMTables();
#if ENABLE(3D_CANVAS)
    WebCore::JSWebGLArray_resetROMTables();
    WebCore::JSWebGLArrayBuffer_resetROMTables();
    WebCore::JSWebGLByteArray_resetROMTables();
    WebCore::JSWebGLFloatArray_resetROMTables();
    WebCore::JSWebGLIntArray_resetROMTables();
    WebCore::JSWebGLRenderingContext_resetROMTables();
    WebCore::JSWebGLShortArray_resetROMTables();
    WebCore::JSWebGLUnsignedByteArray_resetROMTables();
    WebCore::JSWebGLUnsignedIntArray_resetROMTables();
    WebCore::JSWebGLUnsignedShortArray_resetROMTables();
#endif
    WebCore::JSWebKitAnimationEvent_resetROMTables();
    WebCore::JSWebKitCSSKeyframeRule_resetROMTables();
    WebCore::JSWebKitCSSKeyframesRule_resetROMTables();
    WebCore::JSWebKitCSSMatrix_resetROMTables();
    WebCore::JSWebKitCSSTransformValue_resetROMTables();
    WebCore::JSWebKitPoint_resetROMTables();
    WebCore::JSWebKitTransitionEvent_resetROMTables();
    WebCore::JSWheelEvent_resetROMTables();
#if ENABLE(WORKERS)
    WebCore::JSWorker_resetROMTables();
    WebCore::JSWorkerContext_resetROMTables();
    WebCore::JSWorkerLocation_resetROMTables();
    WebCore::JSWorkerNavigator_resetROMTables();
#endif
    WebCore::JSXMLHttpRequest_resetROMTables();
    WebCore::JSXMLHttpRequestException_resetROMTables();
    WebCore::JSXMLHttpRequestProgressEvent_resetROMTables();
    WebCore::JSXMLHttpRequestUpload_resetROMTables();
    WebCore::JSXMLSerializer_resetROMTables();
#if ENABLE(XSLT)
    WebCore::JSXSLTProcessor_resetROMTables();
#endif
}

} // namespace

