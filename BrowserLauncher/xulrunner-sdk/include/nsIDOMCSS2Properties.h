/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/dom/interfaces/css/nsIDOMCSS2Properties.idl
 */

#ifndef __gen_nsIDOMCSS2Properties_h__
#define __gen_nsIDOMCSS2Properties_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMCSS2Properties */
#define NS_IDOMCSS2PROPERTIES_IID_STR "529b987a-cb21-4d58-99d7-9586e7662801"

#define NS_IDOMCSS2PROPERTIES_IID \
  {0x529b987a, 0xcb21, 0x4d58, \
    { 0x99, 0xd7, 0x95, 0x86, 0xe7, 0x66, 0x28, 0x01 }}

/**
 * The nsIDOMCSS2Properties interface is a datatype for additional
 * reflection of data already provided in nsIDOMCSSStyleDeclaration in
 * the Document Object Model.
 *
 * For more information on this interface please see
 * http://www.w3.org/TR/DOM-Level-2-Style
 *
 * This interface matches what is specified in the DOM Level 2
 * specification; new properties are added to nsIDOMNSCSS2Properties.
 */
class NS_NO_VTABLE NS_SCRIPTABLE nsIDOMCSS2Properties : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMCSS2PROPERTIES_IID)

  /* attribute DOMString azimuth; */
  NS_SCRIPTABLE NS_IMETHOD GetAzimuth(nsAString & aAzimuth) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetAzimuth(const nsAString & aAzimuth) = 0;

  /* attribute DOMString background; */
  NS_SCRIPTABLE NS_IMETHOD GetBackground(nsAString & aBackground) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBackground(const nsAString & aBackground) = 0;

  /* attribute DOMString backgroundAttachment; */
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundAttachment(nsAString & aBackgroundAttachment) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundAttachment(const nsAString & aBackgroundAttachment) = 0;

  /* attribute DOMString backgroundColor; */
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundColor(nsAString & aBackgroundColor) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundColor(const nsAString & aBackgroundColor) = 0;

  /* attribute DOMString backgroundImage; */
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundImage(nsAString & aBackgroundImage) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundImage(const nsAString & aBackgroundImage) = 0;

  /* attribute DOMString backgroundPosition; */
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundPosition(nsAString & aBackgroundPosition) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundPosition(const nsAString & aBackgroundPosition) = 0;

  /* attribute DOMString backgroundRepeat; */
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundRepeat(nsAString & aBackgroundRepeat) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundRepeat(const nsAString & aBackgroundRepeat) = 0;

  /* attribute DOMString border; */
  NS_SCRIPTABLE NS_IMETHOD GetBorder(nsAString & aBorder) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorder(const nsAString & aBorder) = 0;

  /* attribute DOMString borderCollapse; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderCollapse(nsAString & aBorderCollapse) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderCollapse(const nsAString & aBorderCollapse) = 0;

  /* attribute DOMString borderColor; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderColor(nsAString & aBorderColor) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderColor(const nsAString & aBorderColor) = 0;

  /* attribute DOMString borderSpacing; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderSpacing(nsAString & aBorderSpacing) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderSpacing(const nsAString & aBorderSpacing) = 0;

  /* attribute DOMString borderStyle; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderStyle(nsAString & aBorderStyle) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderStyle(const nsAString & aBorderStyle) = 0;

  /* attribute DOMString borderTop; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderTop(nsAString & aBorderTop) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderTop(const nsAString & aBorderTop) = 0;

  /* attribute DOMString borderRight; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderRight(nsAString & aBorderRight) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderRight(const nsAString & aBorderRight) = 0;

  /* attribute DOMString borderBottom; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottom(nsAString & aBorderBottom) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottom(const nsAString & aBorderBottom) = 0;

  /* attribute DOMString borderLeft; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeft(nsAString & aBorderLeft) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeft(const nsAString & aBorderLeft) = 0;

  /* attribute DOMString borderTopColor; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderTopColor(nsAString & aBorderTopColor) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderTopColor(const nsAString & aBorderTopColor) = 0;

  /* attribute DOMString borderRightColor; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderRightColor(nsAString & aBorderRightColor) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderRightColor(const nsAString & aBorderRightColor) = 0;

  /* attribute DOMString borderBottomColor; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottomColor(nsAString & aBorderBottomColor) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottomColor(const nsAString & aBorderBottomColor) = 0;

  /* attribute DOMString borderLeftColor; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeftColor(nsAString & aBorderLeftColor) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeftColor(const nsAString & aBorderLeftColor) = 0;

  /* attribute DOMString borderTopStyle; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderTopStyle(nsAString & aBorderTopStyle) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderTopStyle(const nsAString & aBorderTopStyle) = 0;

  /* attribute DOMString borderRightStyle; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderRightStyle(nsAString & aBorderRightStyle) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderRightStyle(const nsAString & aBorderRightStyle) = 0;

  /* attribute DOMString borderBottomStyle; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottomStyle(nsAString & aBorderBottomStyle) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottomStyle(const nsAString & aBorderBottomStyle) = 0;

  /* attribute DOMString borderLeftStyle; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeftStyle(nsAString & aBorderLeftStyle) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeftStyle(const nsAString & aBorderLeftStyle) = 0;

  /* attribute DOMString borderTopWidth; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderTopWidth(nsAString & aBorderTopWidth) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderTopWidth(const nsAString & aBorderTopWidth) = 0;

  /* attribute DOMString borderRightWidth; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderRightWidth(nsAString & aBorderRightWidth) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderRightWidth(const nsAString & aBorderRightWidth) = 0;

  /* attribute DOMString borderBottomWidth; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottomWidth(nsAString & aBorderBottomWidth) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottomWidth(const nsAString & aBorderBottomWidth) = 0;

  /* attribute DOMString borderLeftWidth; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeftWidth(nsAString & aBorderLeftWidth) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeftWidth(const nsAString & aBorderLeftWidth) = 0;

  /* attribute DOMString borderWidth; */
  NS_SCRIPTABLE NS_IMETHOD GetBorderWidth(nsAString & aBorderWidth) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBorderWidth(const nsAString & aBorderWidth) = 0;

  /* attribute DOMString bottom; */
  NS_SCRIPTABLE NS_IMETHOD GetBottom(nsAString & aBottom) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetBottom(const nsAString & aBottom) = 0;

  /* attribute DOMString captionSide; */
  NS_SCRIPTABLE NS_IMETHOD GetCaptionSide(nsAString & aCaptionSide) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetCaptionSide(const nsAString & aCaptionSide) = 0;

  /* attribute DOMString clear; */
  NS_SCRIPTABLE NS_IMETHOD GetClear(nsAString & aClear) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetClear(const nsAString & aClear) = 0;

  /* attribute DOMString clip; */
  NS_SCRIPTABLE NS_IMETHOD GetClip(nsAString & aClip) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetClip(const nsAString & aClip) = 0;

  /* attribute DOMString color; */
  NS_SCRIPTABLE NS_IMETHOD GetColor(nsAString & aColor) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetColor(const nsAString & aColor) = 0;

  /* attribute DOMString content; */
  NS_SCRIPTABLE NS_IMETHOD GetContent(nsAString & aContent) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetContent(const nsAString & aContent) = 0;

  /* attribute DOMString counterIncrement; */
  NS_SCRIPTABLE NS_IMETHOD GetCounterIncrement(nsAString & aCounterIncrement) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetCounterIncrement(const nsAString & aCounterIncrement) = 0;

  /* attribute DOMString counterReset; */
  NS_SCRIPTABLE NS_IMETHOD GetCounterReset(nsAString & aCounterReset) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetCounterReset(const nsAString & aCounterReset) = 0;

  /* attribute DOMString cue; */
  NS_SCRIPTABLE NS_IMETHOD GetCue(nsAString & aCue) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetCue(const nsAString & aCue) = 0;

  /* attribute DOMString cueAfter; */
  NS_SCRIPTABLE NS_IMETHOD GetCueAfter(nsAString & aCueAfter) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetCueAfter(const nsAString & aCueAfter) = 0;

  /* attribute DOMString cueBefore; */
  NS_SCRIPTABLE NS_IMETHOD GetCueBefore(nsAString & aCueBefore) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetCueBefore(const nsAString & aCueBefore) = 0;

  /* attribute DOMString cursor; */
  NS_SCRIPTABLE NS_IMETHOD GetCursor(nsAString & aCursor) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetCursor(const nsAString & aCursor) = 0;

  /* attribute DOMString direction; */
  NS_SCRIPTABLE NS_IMETHOD GetDirection(nsAString & aDirection) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetDirection(const nsAString & aDirection) = 0;

  /* attribute DOMString display; */
  NS_SCRIPTABLE NS_IMETHOD GetDisplay(nsAString & aDisplay) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetDisplay(const nsAString & aDisplay) = 0;

  /* attribute DOMString elevation; */
  NS_SCRIPTABLE NS_IMETHOD GetElevation(nsAString & aElevation) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetElevation(const nsAString & aElevation) = 0;

  /* attribute DOMString emptyCells; */
  NS_SCRIPTABLE NS_IMETHOD GetEmptyCells(nsAString & aEmptyCells) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetEmptyCells(const nsAString & aEmptyCells) = 0;

  /* attribute DOMString cssFloat; */
  NS_SCRIPTABLE NS_IMETHOD GetCssFloat(nsAString & aCssFloat) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetCssFloat(const nsAString & aCssFloat) = 0;

  /* attribute DOMString font; */
  NS_SCRIPTABLE NS_IMETHOD GetFont(nsAString & aFont) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetFont(const nsAString & aFont) = 0;

  /* attribute DOMString fontFamily; */
  NS_SCRIPTABLE NS_IMETHOD GetFontFamily(nsAString & aFontFamily) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetFontFamily(const nsAString & aFontFamily) = 0;

  /* attribute DOMString fontSize; */
  NS_SCRIPTABLE NS_IMETHOD GetFontSize(nsAString & aFontSize) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetFontSize(const nsAString & aFontSize) = 0;

  /* attribute DOMString fontSizeAdjust; */
  NS_SCRIPTABLE NS_IMETHOD GetFontSizeAdjust(nsAString & aFontSizeAdjust) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetFontSizeAdjust(const nsAString & aFontSizeAdjust) = 0;

  /* attribute DOMString fontStretch; */
  NS_SCRIPTABLE NS_IMETHOD GetFontStretch(nsAString & aFontStretch) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetFontStretch(const nsAString & aFontStretch) = 0;

  /* attribute DOMString fontStyle; */
  NS_SCRIPTABLE NS_IMETHOD GetFontStyle(nsAString & aFontStyle) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetFontStyle(const nsAString & aFontStyle) = 0;

  /* attribute DOMString fontVariant; */
  NS_SCRIPTABLE NS_IMETHOD GetFontVariant(nsAString & aFontVariant) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetFontVariant(const nsAString & aFontVariant) = 0;

  /* attribute DOMString fontWeight; */
  NS_SCRIPTABLE NS_IMETHOD GetFontWeight(nsAString & aFontWeight) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetFontWeight(const nsAString & aFontWeight) = 0;

  /* attribute DOMString height; */
  NS_SCRIPTABLE NS_IMETHOD GetHeight(nsAString & aHeight) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetHeight(const nsAString & aHeight) = 0;

  /* attribute DOMString left; */
  NS_SCRIPTABLE NS_IMETHOD GetLeft(nsAString & aLeft) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetLeft(const nsAString & aLeft) = 0;

  /* attribute DOMString letterSpacing; */
  NS_SCRIPTABLE NS_IMETHOD GetLetterSpacing(nsAString & aLetterSpacing) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetLetterSpacing(const nsAString & aLetterSpacing) = 0;

  /* attribute DOMString lineHeight; */
  NS_SCRIPTABLE NS_IMETHOD GetLineHeight(nsAString & aLineHeight) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetLineHeight(const nsAString & aLineHeight) = 0;

  /* attribute DOMString listStyle; */
  NS_SCRIPTABLE NS_IMETHOD GetListStyle(nsAString & aListStyle) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetListStyle(const nsAString & aListStyle) = 0;

  /* attribute DOMString listStyleImage; */
  NS_SCRIPTABLE NS_IMETHOD GetListStyleImage(nsAString & aListStyleImage) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetListStyleImage(const nsAString & aListStyleImage) = 0;

  /* attribute DOMString listStylePosition; */
  NS_SCRIPTABLE NS_IMETHOD GetListStylePosition(nsAString & aListStylePosition) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetListStylePosition(const nsAString & aListStylePosition) = 0;

  /* attribute DOMString listStyleType; */
  NS_SCRIPTABLE NS_IMETHOD GetListStyleType(nsAString & aListStyleType) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetListStyleType(const nsAString & aListStyleType) = 0;

  /* attribute DOMString margin; */
  NS_SCRIPTABLE NS_IMETHOD GetMargin(nsAString & aMargin) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetMargin(const nsAString & aMargin) = 0;

  /* attribute DOMString marginTop; */
  NS_SCRIPTABLE NS_IMETHOD GetMarginTop(nsAString & aMarginTop) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetMarginTop(const nsAString & aMarginTop) = 0;

  /* attribute DOMString marginRight; */
  NS_SCRIPTABLE NS_IMETHOD GetMarginRight(nsAString & aMarginRight) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetMarginRight(const nsAString & aMarginRight) = 0;

  /* attribute DOMString marginBottom; */
  NS_SCRIPTABLE NS_IMETHOD GetMarginBottom(nsAString & aMarginBottom) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetMarginBottom(const nsAString & aMarginBottom) = 0;

  /* attribute DOMString marginLeft; */
  NS_SCRIPTABLE NS_IMETHOD GetMarginLeft(nsAString & aMarginLeft) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetMarginLeft(const nsAString & aMarginLeft) = 0;

  /* attribute DOMString markerOffset; */
  NS_SCRIPTABLE NS_IMETHOD GetMarkerOffset(nsAString & aMarkerOffset) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetMarkerOffset(const nsAString & aMarkerOffset) = 0;

  /* attribute DOMString marks; */
  NS_SCRIPTABLE NS_IMETHOD GetMarks(nsAString & aMarks) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetMarks(const nsAString & aMarks) = 0;

  /* attribute DOMString maxHeight; */
  NS_SCRIPTABLE NS_IMETHOD GetMaxHeight(nsAString & aMaxHeight) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetMaxHeight(const nsAString & aMaxHeight) = 0;

  /* attribute DOMString maxWidth; */
  NS_SCRIPTABLE NS_IMETHOD GetMaxWidth(nsAString & aMaxWidth) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetMaxWidth(const nsAString & aMaxWidth) = 0;

  /* attribute DOMString minHeight; */
  NS_SCRIPTABLE NS_IMETHOD GetMinHeight(nsAString & aMinHeight) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetMinHeight(const nsAString & aMinHeight) = 0;

  /* attribute DOMString minWidth; */
  NS_SCRIPTABLE NS_IMETHOD GetMinWidth(nsAString & aMinWidth) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetMinWidth(const nsAString & aMinWidth) = 0;

  /* attribute DOMString orphans; */
  NS_SCRIPTABLE NS_IMETHOD GetOrphans(nsAString & aOrphans) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetOrphans(const nsAString & aOrphans) = 0;

  /* attribute DOMString outline; */
  NS_SCRIPTABLE NS_IMETHOD GetOutline(nsAString & aOutline) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetOutline(const nsAString & aOutline) = 0;

  /* attribute DOMString outlineColor; */
  NS_SCRIPTABLE NS_IMETHOD GetOutlineColor(nsAString & aOutlineColor) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetOutlineColor(const nsAString & aOutlineColor) = 0;

  /* attribute DOMString outlineStyle; */
  NS_SCRIPTABLE NS_IMETHOD GetOutlineStyle(nsAString & aOutlineStyle) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetOutlineStyle(const nsAString & aOutlineStyle) = 0;

  /* attribute DOMString outlineWidth; */
  NS_SCRIPTABLE NS_IMETHOD GetOutlineWidth(nsAString & aOutlineWidth) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetOutlineWidth(const nsAString & aOutlineWidth) = 0;

  /* attribute DOMString overflow; */
  NS_SCRIPTABLE NS_IMETHOD GetOverflow(nsAString & aOverflow) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetOverflow(const nsAString & aOverflow) = 0;

  /* attribute DOMString padding; */
  NS_SCRIPTABLE NS_IMETHOD GetPadding(nsAString & aPadding) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPadding(const nsAString & aPadding) = 0;

  /* attribute DOMString paddingTop; */
  NS_SCRIPTABLE NS_IMETHOD GetPaddingTop(nsAString & aPaddingTop) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPaddingTop(const nsAString & aPaddingTop) = 0;

  /* attribute DOMString paddingRight; */
  NS_SCRIPTABLE NS_IMETHOD GetPaddingRight(nsAString & aPaddingRight) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPaddingRight(const nsAString & aPaddingRight) = 0;

  /* attribute DOMString paddingBottom; */
  NS_SCRIPTABLE NS_IMETHOD GetPaddingBottom(nsAString & aPaddingBottom) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPaddingBottom(const nsAString & aPaddingBottom) = 0;

  /* attribute DOMString paddingLeft; */
  NS_SCRIPTABLE NS_IMETHOD GetPaddingLeft(nsAString & aPaddingLeft) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPaddingLeft(const nsAString & aPaddingLeft) = 0;

  /* attribute DOMString page; */
  NS_SCRIPTABLE NS_IMETHOD GetPage(nsAString & aPage) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPage(const nsAString & aPage) = 0;

  /* attribute DOMString pageBreakAfter; */
  NS_SCRIPTABLE NS_IMETHOD GetPageBreakAfter(nsAString & aPageBreakAfter) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPageBreakAfter(const nsAString & aPageBreakAfter) = 0;

  /* attribute DOMString pageBreakBefore; */
  NS_SCRIPTABLE NS_IMETHOD GetPageBreakBefore(nsAString & aPageBreakBefore) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPageBreakBefore(const nsAString & aPageBreakBefore) = 0;

  /* attribute DOMString pageBreakInside; */
  NS_SCRIPTABLE NS_IMETHOD GetPageBreakInside(nsAString & aPageBreakInside) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPageBreakInside(const nsAString & aPageBreakInside) = 0;

  /* attribute DOMString pause; */
  NS_SCRIPTABLE NS_IMETHOD GetPause(nsAString & aPause) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPause(const nsAString & aPause) = 0;

  /* attribute DOMString pauseAfter; */
  NS_SCRIPTABLE NS_IMETHOD GetPauseAfter(nsAString & aPauseAfter) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPauseAfter(const nsAString & aPauseAfter) = 0;

  /* attribute DOMString pauseBefore; */
  NS_SCRIPTABLE NS_IMETHOD GetPauseBefore(nsAString & aPauseBefore) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPauseBefore(const nsAString & aPauseBefore) = 0;

  /* attribute DOMString pitch; */
  NS_SCRIPTABLE NS_IMETHOD GetPitch(nsAString & aPitch) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPitch(const nsAString & aPitch) = 0;

  /* attribute DOMString pitchRange; */
  NS_SCRIPTABLE NS_IMETHOD GetPitchRange(nsAString & aPitchRange) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPitchRange(const nsAString & aPitchRange) = 0;

  /* attribute DOMString position; */
  NS_SCRIPTABLE NS_IMETHOD GetPosition(nsAString & aPosition) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetPosition(const nsAString & aPosition) = 0;

  /* attribute DOMString quotes; */
  NS_SCRIPTABLE NS_IMETHOD GetQuotes(nsAString & aQuotes) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetQuotes(const nsAString & aQuotes) = 0;

  /* attribute DOMString richness; */
  NS_SCRIPTABLE NS_IMETHOD GetRichness(nsAString & aRichness) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetRichness(const nsAString & aRichness) = 0;

  /* attribute DOMString right; */
  NS_SCRIPTABLE NS_IMETHOD GetRight(nsAString & aRight) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetRight(const nsAString & aRight) = 0;

  /* attribute DOMString size; */
  NS_SCRIPTABLE NS_IMETHOD GetSize(nsAString & aSize) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetSize(const nsAString & aSize) = 0;

  /* attribute DOMString speak; */
  NS_SCRIPTABLE NS_IMETHOD GetSpeak(nsAString & aSpeak) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetSpeak(const nsAString & aSpeak) = 0;

  /* attribute DOMString speakHeader; */
  NS_SCRIPTABLE NS_IMETHOD GetSpeakHeader(nsAString & aSpeakHeader) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetSpeakHeader(const nsAString & aSpeakHeader) = 0;

  /* attribute DOMString speakNumeral; */
  NS_SCRIPTABLE NS_IMETHOD GetSpeakNumeral(nsAString & aSpeakNumeral) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetSpeakNumeral(const nsAString & aSpeakNumeral) = 0;

  /* attribute DOMString speakPunctuation; */
  NS_SCRIPTABLE NS_IMETHOD GetSpeakPunctuation(nsAString & aSpeakPunctuation) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetSpeakPunctuation(const nsAString & aSpeakPunctuation) = 0;

  /* attribute DOMString speechRate; */
  NS_SCRIPTABLE NS_IMETHOD GetSpeechRate(nsAString & aSpeechRate) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetSpeechRate(const nsAString & aSpeechRate) = 0;

  /* attribute DOMString stress; */
  NS_SCRIPTABLE NS_IMETHOD GetStress(nsAString & aStress) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetStress(const nsAString & aStress) = 0;

  /* attribute DOMString tableLayout; */
  NS_SCRIPTABLE NS_IMETHOD GetTableLayout(nsAString & aTableLayout) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetTableLayout(const nsAString & aTableLayout) = 0;

  /* attribute DOMString textAlign; */
  NS_SCRIPTABLE NS_IMETHOD GetTextAlign(nsAString & aTextAlign) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetTextAlign(const nsAString & aTextAlign) = 0;

  /* attribute DOMString textDecoration; */
  NS_SCRIPTABLE NS_IMETHOD GetTextDecoration(nsAString & aTextDecoration) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetTextDecoration(const nsAString & aTextDecoration) = 0;

  /* attribute DOMString textIndent; */
  NS_SCRIPTABLE NS_IMETHOD GetTextIndent(nsAString & aTextIndent) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetTextIndent(const nsAString & aTextIndent) = 0;

  /* attribute DOMString textShadow; */
  NS_SCRIPTABLE NS_IMETHOD GetTextShadow(nsAString & aTextShadow) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetTextShadow(const nsAString & aTextShadow) = 0;

  /* attribute DOMString textTransform; */
  NS_SCRIPTABLE NS_IMETHOD GetTextTransform(nsAString & aTextTransform) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetTextTransform(const nsAString & aTextTransform) = 0;

  /* attribute DOMString top; */
  NS_SCRIPTABLE NS_IMETHOD GetTop(nsAString & aTop) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetTop(const nsAString & aTop) = 0;

  /* attribute DOMString unicodeBidi; */
  NS_SCRIPTABLE NS_IMETHOD GetUnicodeBidi(nsAString & aUnicodeBidi) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetUnicodeBidi(const nsAString & aUnicodeBidi) = 0;

  /* attribute DOMString verticalAlign; */
  NS_SCRIPTABLE NS_IMETHOD GetVerticalAlign(nsAString & aVerticalAlign) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetVerticalAlign(const nsAString & aVerticalAlign) = 0;

  /* attribute DOMString visibility; */
  NS_SCRIPTABLE NS_IMETHOD GetVisibility(nsAString & aVisibility) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetVisibility(const nsAString & aVisibility) = 0;

  /* attribute DOMString voiceFamily; */
  NS_SCRIPTABLE NS_IMETHOD GetVoiceFamily(nsAString & aVoiceFamily) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetVoiceFamily(const nsAString & aVoiceFamily) = 0;

  /* attribute DOMString volume; */
  NS_SCRIPTABLE NS_IMETHOD GetVolume(nsAString & aVolume) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetVolume(const nsAString & aVolume) = 0;

  /* attribute DOMString whiteSpace; */
  NS_SCRIPTABLE NS_IMETHOD GetWhiteSpace(nsAString & aWhiteSpace) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetWhiteSpace(const nsAString & aWhiteSpace) = 0;

  /* attribute DOMString widows; */
  NS_SCRIPTABLE NS_IMETHOD GetWidows(nsAString & aWidows) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetWidows(const nsAString & aWidows) = 0;

  /* attribute DOMString width; */
  NS_SCRIPTABLE NS_IMETHOD GetWidth(nsAString & aWidth) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetWidth(const nsAString & aWidth) = 0;

  /* attribute DOMString wordSpacing; */
  NS_SCRIPTABLE NS_IMETHOD GetWordSpacing(nsAString & aWordSpacing) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetWordSpacing(const nsAString & aWordSpacing) = 0;

  /* attribute DOMString zIndex; */
  NS_SCRIPTABLE NS_IMETHOD GetZIndex(nsAString & aZIndex) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetZIndex(const nsAString & aZIndex) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMCSS2Properties, NS_IDOMCSS2PROPERTIES_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMCSS2PROPERTIES \
  NS_SCRIPTABLE NS_IMETHOD GetAzimuth(nsAString & aAzimuth); \
  NS_SCRIPTABLE NS_IMETHOD SetAzimuth(const nsAString & aAzimuth); \
  NS_SCRIPTABLE NS_IMETHOD GetBackground(nsAString & aBackground); \
  NS_SCRIPTABLE NS_IMETHOD SetBackground(const nsAString & aBackground); \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundAttachment(nsAString & aBackgroundAttachment); \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundAttachment(const nsAString & aBackgroundAttachment); \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundColor(nsAString & aBackgroundColor); \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundColor(const nsAString & aBackgroundColor); \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundImage(nsAString & aBackgroundImage); \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundImage(const nsAString & aBackgroundImage); \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundPosition(nsAString & aBackgroundPosition); \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundPosition(const nsAString & aBackgroundPosition); \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundRepeat(nsAString & aBackgroundRepeat); \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundRepeat(const nsAString & aBackgroundRepeat); \
  NS_SCRIPTABLE NS_IMETHOD GetBorder(nsAString & aBorder); \
  NS_SCRIPTABLE NS_IMETHOD SetBorder(const nsAString & aBorder); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderCollapse(nsAString & aBorderCollapse); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderCollapse(const nsAString & aBorderCollapse); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderColor(nsAString & aBorderColor); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderColor(const nsAString & aBorderColor); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderSpacing(nsAString & aBorderSpacing); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderSpacing(const nsAString & aBorderSpacing); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderStyle(nsAString & aBorderStyle); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderStyle(const nsAString & aBorderStyle); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderTop(nsAString & aBorderTop); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderTop(const nsAString & aBorderTop); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderRight(nsAString & aBorderRight); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderRight(const nsAString & aBorderRight); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottom(nsAString & aBorderBottom); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottom(const nsAString & aBorderBottom); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeft(nsAString & aBorderLeft); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeft(const nsAString & aBorderLeft); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderTopColor(nsAString & aBorderTopColor); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderTopColor(const nsAString & aBorderTopColor); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderRightColor(nsAString & aBorderRightColor); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderRightColor(const nsAString & aBorderRightColor); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottomColor(nsAString & aBorderBottomColor); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottomColor(const nsAString & aBorderBottomColor); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeftColor(nsAString & aBorderLeftColor); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeftColor(const nsAString & aBorderLeftColor); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderTopStyle(nsAString & aBorderTopStyle); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderTopStyle(const nsAString & aBorderTopStyle); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderRightStyle(nsAString & aBorderRightStyle); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderRightStyle(const nsAString & aBorderRightStyle); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottomStyle(nsAString & aBorderBottomStyle); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottomStyle(const nsAString & aBorderBottomStyle); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeftStyle(nsAString & aBorderLeftStyle); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeftStyle(const nsAString & aBorderLeftStyle); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderTopWidth(nsAString & aBorderTopWidth); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderTopWidth(const nsAString & aBorderTopWidth); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderRightWidth(nsAString & aBorderRightWidth); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderRightWidth(const nsAString & aBorderRightWidth); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottomWidth(nsAString & aBorderBottomWidth); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottomWidth(const nsAString & aBorderBottomWidth); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeftWidth(nsAString & aBorderLeftWidth); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeftWidth(const nsAString & aBorderLeftWidth); \
  NS_SCRIPTABLE NS_IMETHOD GetBorderWidth(nsAString & aBorderWidth); \
  NS_SCRIPTABLE NS_IMETHOD SetBorderWidth(const nsAString & aBorderWidth); \
  NS_SCRIPTABLE NS_IMETHOD GetBottom(nsAString & aBottom); \
  NS_SCRIPTABLE NS_IMETHOD SetBottom(const nsAString & aBottom); \
  NS_SCRIPTABLE NS_IMETHOD GetCaptionSide(nsAString & aCaptionSide); \
  NS_SCRIPTABLE NS_IMETHOD SetCaptionSide(const nsAString & aCaptionSide); \
  NS_SCRIPTABLE NS_IMETHOD GetClear(nsAString & aClear); \
  NS_SCRIPTABLE NS_IMETHOD SetClear(const nsAString & aClear); \
  NS_SCRIPTABLE NS_IMETHOD GetClip(nsAString & aClip); \
  NS_SCRIPTABLE NS_IMETHOD SetClip(const nsAString & aClip); \
  NS_SCRIPTABLE NS_IMETHOD GetColor(nsAString & aColor); \
  NS_SCRIPTABLE NS_IMETHOD SetColor(const nsAString & aColor); \
  NS_SCRIPTABLE NS_IMETHOD GetContent(nsAString & aContent); \
  NS_SCRIPTABLE NS_IMETHOD SetContent(const nsAString & aContent); \
  NS_SCRIPTABLE NS_IMETHOD GetCounterIncrement(nsAString & aCounterIncrement); \
  NS_SCRIPTABLE NS_IMETHOD SetCounterIncrement(const nsAString & aCounterIncrement); \
  NS_SCRIPTABLE NS_IMETHOD GetCounterReset(nsAString & aCounterReset); \
  NS_SCRIPTABLE NS_IMETHOD SetCounterReset(const nsAString & aCounterReset); \
  NS_SCRIPTABLE NS_IMETHOD GetCue(nsAString & aCue); \
  NS_SCRIPTABLE NS_IMETHOD SetCue(const nsAString & aCue); \
  NS_SCRIPTABLE NS_IMETHOD GetCueAfter(nsAString & aCueAfter); \
  NS_SCRIPTABLE NS_IMETHOD SetCueAfter(const nsAString & aCueAfter); \
  NS_SCRIPTABLE NS_IMETHOD GetCueBefore(nsAString & aCueBefore); \
  NS_SCRIPTABLE NS_IMETHOD SetCueBefore(const nsAString & aCueBefore); \
  NS_SCRIPTABLE NS_IMETHOD GetCursor(nsAString & aCursor); \
  NS_SCRIPTABLE NS_IMETHOD SetCursor(const nsAString & aCursor); \
  NS_SCRIPTABLE NS_IMETHOD GetDirection(nsAString & aDirection); \
  NS_SCRIPTABLE NS_IMETHOD SetDirection(const nsAString & aDirection); \
  NS_SCRIPTABLE NS_IMETHOD GetDisplay(nsAString & aDisplay); \
  NS_SCRIPTABLE NS_IMETHOD SetDisplay(const nsAString & aDisplay); \
  NS_SCRIPTABLE NS_IMETHOD GetElevation(nsAString & aElevation); \
  NS_SCRIPTABLE NS_IMETHOD SetElevation(const nsAString & aElevation); \
  NS_SCRIPTABLE NS_IMETHOD GetEmptyCells(nsAString & aEmptyCells); \
  NS_SCRIPTABLE NS_IMETHOD SetEmptyCells(const nsAString & aEmptyCells); \
  NS_SCRIPTABLE NS_IMETHOD GetCssFloat(nsAString & aCssFloat); \
  NS_SCRIPTABLE NS_IMETHOD SetCssFloat(const nsAString & aCssFloat); \
  NS_SCRIPTABLE NS_IMETHOD GetFont(nsAString & aFont); \
  NS_SCRIPTABLE NS_IMETHOD SetFont(const nsAString & aFont); \
  NS_SCRIPTABLE NS_IMETHOD GetFontFamily(nsAString & aFontFamily); \
  NS_SCRIPTABLE NS_IMETHOD SetFontFamily(const nsAString & aFontFamily); \
  NS_SCRIPTABLE NS_IMETHOD GetFontSize(nsAString & aFontSize); \
  NS_SCRIPTABLE NS_IMETHOD SetFontSize(const nsAString & aFontSize); \
  NS_SCRIPTABLE NS_IMETHOD GetFontSizeAdjust(nsAString & aFontSizeAdjust); \
  NS_SCRIPTABLE NS_IMETHOD SetFontSizeAdjust(const nsAString & aFontSizeAdjust); \
  NS_SCRIPTABLE NS_IMETHOD GetFontStretch(nsAString & aFontStretch); \
  NS_SCRIPTABLE NS_IMETHOD SetFontStretch(const nsAString & aFontStretch); \
  NS_SCRIPTABLE NS_IMETHOD GetFontStyle(nsAString & aFontStyle); \
  NS_SCRIPTABLE NS_IMETHOD SetFontStyle(const nsAString & aFontStyle); \
  NS_SCRIPTABLE NS_IMETHOD GetFontVariant(nsAString & aFontVariant); \
  NS_SCRIPTABLE NS_IMETHOD SetFontVariant(const nsAString & aFontVariant); \
  NS_SCRIPTABLE NS_IMETHOD GetFontWeight(nsAString & aFontWeight); \
  NS_SCRIPTABLE NS_IMETHOD SetFontWeight(const nsAString & aFontWeight); \
  NS_SCRIPTABLE NS_IMETHOD GetHeight(nsAString & aHeight); \
  NS_SCRIPTABLE NS_IMETHOD SetHeight(const nsAString & aHeight); \
  NS_SCRIPTABLE NS_IMETHOD GetLeft(nsAString & aLeft); \
  NS_SCRIPTABLE NS_IMETHOD SetLeft(const nsAString & aLeft); \
  NS_SCRIPTABLE NS_IMETHOD GetLetterSpacing(nsAString & aLetterSpacing); \
  NS_SCRIPTABLE NS_IMETHOD SetLetterSpacing(const nsAString & aLetterSpacing); \
  NS_SCRIPTABLE NS_IMETHOD GetLineHeight(nsAString & aLineHeight); \
  NS_SCRIPTABLE NS_IMETHOD SetLineHeight(const nsAString & aLineHeight); \
  NS_SCRIPTABLE NS_IMETHOD GetListStyle(nsAString & aListStyle); \
  NS_SCRIPTABLE NS_IMETHOD SetListStyle(const nsAString & aListStyle); \
  NS_SCRIPTABLE NS_IMETHOD GetListStyleImage(nsAString & aListStyleImage); \
  NS_SCRIPTABLE NS_IMETHOD SetListStyleImage(const nsAString & aListStyleImage); \
  NS_SCRIPTABLE NS_IMETHOD GetListStylePosition(nsAString & aListStylePosition); \
  NS_SCRIPTABLE NS_IMETHOD SetListStylePosition(const nsAString & aListStylePosition); \
  NS_SCRIPTABLE NS_IMETHOD GetListStyleType(nsAString & aListStyleType); \
  NS_SCRIPTABLE NS_IMETHOD SetListStyleType(const nsAString & aListStyleType); \
  NS_SCRIPTABLE NS_IMETHOD GetMargin(nsAString & aMargin); \
  NS_SCRIPTABLE NS_IMETHOD SetMargin(const nsAString & aMargin); \
  NS_SCRIPTABLE NS_IMETHOD GetMarginTop(nsAString & aMarginTop); \
  NS_SCRIPTABLE NS_IMETHOD SetMarginTop(const nsAString & aMarginTop); \
  NS_SCRIPTABLE NS_IMETHOD GetMarginRight(nsAString & aMarginRight); \
  NS_SCRIPTABLE NS_IMETHOD SetMarginRight(const nsAString & aMarginRight); \
  NS_SCRIPTABLE NS_IMETHOD GetMarginBottom(nsAString & aMarginBottom); \
  NS_SCRIPTABLE NS_IMETHOD SetMarginBottom(const nsAString & aMarginBottom); \
  NS_SCRIPTABLE NS_IMETHOD GetMarginLeft(nsAString & aMarginLeft); \
  NS_SCRIPTABLE NS_IMETHOD SetMarginLeft(const nsAString & aMarginLeft); \
  NS_SCRIPTABLE NS_IMETHOD GetMarkerOffset(nsAString & aMarkerOffset); \
  NS_SCRIPTABLE NS_IMETHOD SetMarkerOffset(const nsAString & aMarkerOffset); \
  NS_SCRIPTABLE NS_IMETHOD GetMarks(nsAString & aMarks); \
  NS_SCRIPTABLE NS_IMETHOD SetMarks(const nsAString & aMarks); \
  NS_SCRIPTABLE NS_IMETHOD GetMaxHeight(nsAString & aMaxHeight); \
  NS_SCRIPTABLE NS_IMETHOD SetMaxHeight(const nsAString & aMaxHeight); \
  NS_SCRIPTABLE NS_IMETHOD GetMaxWidth(nsAString & aMaxWidth); \
  NS_SCRIPTABLE NS_IMETHOD SetMaxWidth(const nsAString & aMaxWidth); \
  NS_SCRIPTABLE NS_IMETHOD GetMinHeight(nsAString & aMinHeight); \
  NS_SCRIPTABLE NS_IMETHOD SetMinHeight(const nsAString & aMinHeight); \
  NS_SCRIPTABLE NS_IMETHOD GetMinWidth(nsAString & aMinWidth); \
  NS_SCRIPTABLE NS_IMETHOD SetMinWidth(const nsAString & aMinWidth); \
  NS_SCRIPTABLE NS_IMETHOD GetOrphans(nsAString & aOrphans); \
  NS_SCRIPTABLE NS_IMETHOD SetOrphans(const nsAString & aOrphans); \
  NS_SCRIPTABLE NS_IMETHOD GetOutline(nsAString & aOutline); \
  NS_SCRIPTABLE NS_IMETHOD SetOutline(const nsAString & aOutline); \
  NS_SCRIPTABLE NS_IMETHOD GetOutlineColor(nsAString & aOutlineColor); \
  NS_SCRIPTABLE NS_IMETHOD SetOutlineColor(const nsAString & aOutlineColor); \
  NS_SCRIPTABLE NS_IMETHOD GetOutlineStyle(nsAString & aOutlineStyle); \
  NS_SCRIPTABLE NS_IMETHOD SetOutlineStyle(const nsAString & aOutlineStyle); \
  NS_SCRIPTABLE NS_IMETHOD GetOutlineWidth(nsAString & aOutlineWidth); \
  NS_SCRIPTABLE NS_IMETHOD SetOutlineWidth(const nsAString & aOutlineWidth); \
  NS_SCRIPTABLE NS_IMETHOD GetOverflow(nsAString & aOverflow); \
  NS_SCRIPTABLE NS_IMETHOD SetOverflow(const nsAString & aOverflow); \
  NS_SCRIPTABLE NS_IMETHOD GetPadding(nsAString & aPadding); \
  NS_SCRIPTABLE NS_IMETHOD SetPadding(const nsAString & aPadding); \
  NS_SCRIPTABLE NS_IMETHOD GetPaddingTop(nsAString & aPaddingTop); \
  NS_SCRIPTABLE NS_IMETHOD SetPaddingTop(const nsAString & aPaddingTop); \
  NS_SCRIPTABLE NS_IMETHOD GetPaddingRight(nsAString & aPaddingRight); \
  NS_SCRIPTABLE NS_IMETHOD SetPaddingRight(const nsAString & aPaddingRight); \
  NS_SCRIPTABLE NS_IMETHOD GetPaddingBottom(nsAString & aPaddingBottom); \
  NS_SCRIPTABLE NS_IMETHOD SetPaddingBottom(const nsAString & aPaddingBottom); \
  NS_SCRIPTABLE NS_IMETHOD GetPaddingLeft(nsAString & aPaddingLeft); \
  NS_SCRIPTABLE NS_IMETHOD SetPaddingLeft(const nsAString & aPaddingLeft); \
  NS_SCRIPTABLE NS_IMETHOD GetPage(nsAString & aPage); \
  NS_SCRIPTABLE NS_IMETHOD SetPage(const nsAString & aPage); \
  NS_SCRIPTABLE NS_IMETHOD GetPageBreakAfter(nsAString & aPageBreakAfter); \
  NS_SCRIPTABLE NS_IMETHOD SetPageBreakAfter(const nsAString & aPageBreakAfter); \
  NS_SCRIPTABLE NS_IMETHOD GetPageBreakBefore(nsAString & aPageBreakBefore); \
  NS_SCRIPTABLE NS_IMETHOD SetPageBreakBefore(const nsAString & aPageBreakBefore); \
  NS_SCRIPTABLE NS_IMETHOD GetPageBreakInside(nsAString & aPageBreakInside); \
  NS_SCRIPTABLE NS_IMETHOD SetPageBreakInside(const nsAString & aPageBreakInside); \
  NS_SCRIPTABLE NS_IMETHOD GetPause(nsAString & aPause); \
  NS_SCRIPTABLE NS_IMETHOD SetPause(const nsAString & aPause); \
  NS_SCRIPTABLE NS_IMETHOD GetPauseAfter(nsAString & aPauseAfter); \
  NS_SCRIPTABLE NS_IMETHOD SetPauseAfter(const nsAString & aPauseAfter); \
  NS_SCRIPTABLE NS_IMETHOD GetPauseBefore(nsAString & aPauseBefore); \
  NS_SCRIPTABLE NS_IMETHOD SetPauseBefore(const nsAString & aPauseBefore); \
  NS_SCRIPTABLE NS_IMETHOD GetPitch(nsAString & aPitch); \
  NS_SCRIPTABLE NS_IMETHOD SetPitch(const nsAString & aPitch); \
  NS_SCRIPTABLE NS_IMETHOD GetPitchRange(nsAString & aPitchRange); \
  NS_SCRIPTABLE NS_IMETHOD SetPitchRange(const nsAString & aPitchRange); \
  NS_SCRIPTABLE NS_IMETHOD GetPosition(nsAString & aPosition); \
  NS_SCRIPTABLE NS_IMETHOD SetPosition(const nsAString & aPosition); \
  NS_SCRIPTABLE NS_IMETHOD GetQuotes(nsAString & aQuotes); \
  NS_SCRIPTABLE NS_IMETHOD SetQuotes(const nsAString & aQuotes); \
  NS_SCRIPTABLE NS_IMETHOD GetRichness(nsAString & aRichness); \
  NS_SCRIPTABLE NS_IMETHOD SetRichness(const nsAString & aRichness); \
  NS_SCRIPTABLE NS_IMETHOD GetRight(nsAString & aRight); \
  NS_SCRIPTABLE NS_IMETHOD SetRight(const nsAString & aRight); \
  NS_SCRIPTABLE NS_IMETHOD GetSize(nsAString & aSize); \
  NS_SCRIPTABLE NS_IMETHOD SetSize(const nsAString & aSize); \
  NS_SCRIPTABLE NS_IMETHOD GetSpeak(nsAString & aSpeak); \
  NS_SCRIPTABLE NS_IMETHOD SetSpeak(const nsAString & aSpeak); \
  NS_SCRIPTABLE NS_IMETHOD GetSpeakHeader(nsAString & aSpeakHeader); \
  NS_SCRIPTABLE NS_IMETHOD SetSpeakHeader(const nsAString & aSpeakHeader); \
  NS_SCRIPTABLE NS_IMETHOD GetSpeakNumeral(nsAString & aSpeakNumeral); \
  NS_SCRIPTABLE NS_IMETHOD SetSpeakNumeral(const nsAString & aSpeakNumeral); \
  NS_SCRIPTABLE NS_IMETHOD GetSpeakPunctuation(nsAString & aSpeakPunctuation); \
  NS_SCRIPTABLE NS_IMETHOD SetSpeakPunctuation(const nsAString & aSpeakPunctuation); \
  NS_SCRIPTABLE NS_IMETHOD GetSpeechRate(nsAString & aSpeechRate); \
  NS_SCRIPTABLE NS_IMETHOD SetSpeechRate(const nsAString & aSpeechRate); \
  NS_SCRIPTABLE NS_IMETHOD GetStress(nsAString & aStress); \
  NS_SCRIPTABLE NS_IMETHOD SetStress(const nsAString & aStress); \
  NS_SCRIPTABLE NS_IMETHOD GetTableLayout(nsAString & aTableLayout); \
  NS_SCRIPTABLE NS_IMETHOD SetTableLayout(const nsAString & aTableLayout); \
  NS_SCRIPTABLE NS_IMETHOD GetTextAlign(nsAString & aTextAlign); \
  NS_SCRIPTABLE NS_IMETHOD SetTextAlign(const nsAString & aTextAlign); \
  NS_SCRIPTABLE NS_IMETHOD GetTextDecoration(nsAString & aTextDecoration); \
  NS_SCRIPTABLE NS_IMETHOD SetTextDecoration(const nsAString & aTextDecoration); \
  NS_SCRIPTABLE NS_IMETHOD GetTextIndent(nsAString & aTextIndent); \
  NS_SCRIPTABLE NS_IMETHOD SetTextIndent(const nsAString & aTextIndent); \
  NS_SCRIPTABLE NS_IMETHOD GetTextShadow(nsAString & aTextShadow); \
  NS_SCRIPTABLE NS_IMETHOD SetTextShadow(const nsAString & aTextShadow); \
  NS_SCRIPTABLE NS_IMETHOD GetTextTransform(nsAString & aTextTransform); \
  NS_SCRIPTABLE NS_IMETHOD SetTextTransform(const nsAString & aTextTransform); \
  NS_SCRIPTABLE NS_IMETHOD GetTop(nsAString & aTop); \
  NS_SCRIPTABLE NS_IMETHOD SetTop(const nsAString & aTop); \
  NS_SCRIPTABLE NS_IMETHOD GetUnicodeBidi(nsAString & aUnicodeBidi); \
  NS_SCRIPTABLE NS_IMETHOD SetUnicodeBidi(const nsAString & aUnicodeBidi); \
  NS_SCRIPTABLE NS_IMETHOD GetVerticalAlign(nsAString & aVerticalAlign); \
  NS_SCRIPTABLE NS_IMETHOD SetVerticalAlign(const nsAString & aVerticalAlign); \
  NS_SCRIPTABLE NS_IMETHOD GetVisibility(nsAString & aVisibility); \
  NS_SCRIPTABLE NS_IMETHOD SetVisibility(const nsAString & aVisibility); \
  NS_SCRIPTABLE NS_IMETHOD GetVoiceFamily(nsAString & aVoiceFamily); \
  NS_SCRIPTABLE NS_IMETHOD SetVoiceFamily(const nsAString & aVoiceFamily); \
  NS_SCRIPTABLE NS_IMETHOD GetVolume(nsAString & aVolume); \
  NS_SCRIPTABLE NS_IMETHOD SetVolume(const nsAString & aVolume); \
  NS_SCRIPTABLE NS_IMETHOD GetWhiteSpace(nsAString & aWhiteSpace); \
  NS_SCRIPTABLE NS_IMETHOD SetWhiteSpace(const nsAString & aWhiteSpace); \
  NS_SCRIPTABLE NS_IMETHOD GetWidows(nsAString & aWidows); \
  NS_SCRIPTABLE NS_IMETHOD SetWidows(const nsAString & aWidows); \
  NS_SCRIPTABLE NS_IMETHOD GetWidth(nsAString & aWidth); \
  NS_SCRIPTABLE NS_IMETHOD SetWidth(const nsAString & aWidth); \
  NS_SCRIPTABLE NS_IMETHOD GetWordSpacing(nsAString & aWordSpacing); \
  NS_SCRIPTABLE NS_IMETHOD SetWordSpacing(const nsAString & aWordSpacing); \
  NS_SCRIPTABLE NS_IMETHOD GetZIndex(nsAString & aZIndex); \
  NS_SCRIPTABLE NS_IMETHOD SetZIndex(const nsAString & aZIndex); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMCSS2PROPERTIES(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetAzimuth(nsAString & aAzimuth) { return _to GetAzimuth(aAzimuth); } \
  NS_SCRIPTABLE NS_IMETHOD SetAzimuth(const nsAString & aAzimuth) { return _to SetAzimuth(aAzimuth); } \
  NS_SCRIPTABLE NS_IMETHOD GetBackground(nsAString & aBackground) { return _to GetBackground(aBackground); } \
  NS_SCRIPTABLE NS_IMETHOD SetBackground(const nsAString & aBackground) { return _to SetBackground(aBackground); } \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundAttachment(nsAString & aBackgroundAttachment) { return _to GetBackgroundAttachment(aBackgroundAttachment); } \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundAttachment(const nsAString & aBackgroundAttachment) { return _to SetBackgroundAttachment(aBackgroundAttachment); } \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundColor(nsAString & aBackgroundColor) { return _to GetBackgroundColor(aBackgroundColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundColor(const nsAString & aBackgroundColor) { return _to SetBackgroundColor(aBackgroundColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundImage(nsAString & aBackgroundImage) { return _to GetBackgroundImage(aBackgroundImage); } \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundImage(const nsAString & aBackgroundImage) { return _to SetBackgroundImage(aBackgroundImage); } \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundPosition(nsAString & aBackgroundPosition) { return _to GetBackgroundPosition(aBackgroundPosition); } \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundPosition(const nsAString & aBackgroundPosition) { return _to SetBackgroundPosition(aBackgroundPosition); } \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundRepeat(nsAString & aBackgroundRepeat) { return _to GetBackgroundRepeat(aBackgroundRepeat); } \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundRepeat(const nsAString & aBackgroundRepeat) { return _to SetBackgroundRepeat(aBackgroundRepeat); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorder(nsAString & aBorder) { return _to GetBorder(aBorder); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorder(const nsAString & aBorder) { return _to SetBorder(aBorder); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderCollapse(nsAString & aBorderCollapse) { return _to GetBorderCollapse(aBorderCollapse); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderCollapse(const nsAString & aBorderCollapse) { return _to SetBorderCollapse(aBorderCollapse); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderColor(nsAString & aBorderColor) { return _to GetBorderColor(aBorderColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderColor(const nsAString & aBorderColor) { return _to SetBorderColor(aBorderColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderSpacing(nsAString & aBorderSpacing) { return _to GetBorderSpacing(aBorderSpacing); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderSpacing(const nsAString & aBorderSpacing) { return _to SetBorderSpacing(aBorderSpacing); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderStyle(nsAString & aBorderStyle) { return _to GetBorderStyle(aBorderStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderStyle(const nsAString & aBorderStyle) { return _to SetBorderStyle(aBorderStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderTop(nsAString & aBorderTop) { return _to GetBorderTop(aBorderTop); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderTop(const nsAString & aBorderTop) { return _to SetBorderTop(aBorderTop); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderRight(nsAString & aBorderRight) { return _to GetBorderRight(aBorderRight); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderRight(const nsAString & aBorderRight) { return _to SetBorderRight(aBorderRight); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottom(nsAString & aBorderBottom) { return _to GetBorderBottom(aBorderBottom); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottom(const nsAString & aBorderBottom) { return _to SetBorderBottom(aBorderBottom); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeft(nsAString & aBorderLeft) { return _to GetBorderLeft(aBorderLeft); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeft(const nsAString & aBorderLeft) { return _to SetBorderLeft(aBorderLeft); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderTopColor(nsAString & aBorderTopColor) { return _to GetBorderTopColor(aBorderTopColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderTopColor(const nsAString & aBorderTopColor) { return _to SetBorderTopColor(aBorderTopColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderRightColor(nsAString & aBorderRightColor) { return _to GetBorderRightColor(aBorderRightColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderRightColor(const nsAString & aBorderRightColor) { return _to SetBorderRightColor(aBorderRightColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottomColor(nsAString & aBorderBottomColor) { return _to GetBorderBottomColor(aBorderBottomColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottomColor(const nsAString & aBorderBottomColor) { return _to SetBorderBottomColor(aBorderBottomColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeftColor(nsAString & aBorderLeftColor) { return _to GetBorderLeftColor(aBorderLeftColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeftColor(const nsAString & aBorderLeftColor) { return _to SetBorderLeftColor(aBorderLeftColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderTopStyle(nsAString & aBorderTopStyle) { return _to GetBorderTopStyle(aBorderTopStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderTopStyle(const nsAString & aBorderTopStyle) { return _to SetBorderTopStyle(aBorderTopStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderRightStyle(nsAString & aBorderRightStyle) { return _to GetBorderRightStyle(aBorderRightStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderRightStyle(const nsAString & aBorderRightStyle) { return _to SetBorderRightStyle(aBorderRightStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottomStyle(nsAString & aBorderBottomStyle) { return _to GetBorderBottomStyle(aBorderBottomStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottomStyle(const nsAString & aBorderBottomStyle) { return _to SetBorderBottomStyle(aBorderBottomStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeftStyle(nsAString & aBorderLeftStyle) { return _to GetBorderLeftStyle(aBorderLeftStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeftStyle(const nsAString & aBorderLeftStyle) { return _to SetBorderLeftStyle(aBorderLeftStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderTopWidth(nsAString & aBorderTopWidth) { return _to GetBorderTopWidth(aBorderTopWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderTopWidth(const nsAString & aBorderTopWidth) { return _to SetBorderTopWidth(aBorderTopWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderRightWidth(nsAString & aBorderRightWidth) { return _to GetBorderRightWidth(aBorderRightWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderRightWidth(const nsAString & aBorderRightWidth) { return _to SetBorderRightWidth(aBorderRightWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottomWidth(nsAString & aBorderBottomWidth) { return _to GetBorderBottomWidth(aBorderBottomWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottomWidth(const nsAString & aBorderBottomWidth) { return _to SetBorderBottomWidth(aBorderBottomWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeftWidth(nsAString & aBorderLeftWidth) { return _to GetBorderLeftWidth(aBorderLeftWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeftWidth(const nsAString & aBorderLeftWidth) { return _to SetBorderLeftWidth(aBorderLeftWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderWidth(nsAString & aBorderWidth) { return _to GetBorderWidth(aBorderWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderWidth(const nsAString & aBorderWidth) { return _to SetBorderWidth(aBorderWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetBottom(nsAString & aBottom) { return _to GetBottom(aBottom); } \
  NS_SCRIPTABLE NS_IMETHOD SetBottom(const nsAString & aBottom) { return _to SetBottom(aBottom); } \
  NS_SCRIPTABLE NS_IMETHOD GetCaptionSide(nsAString & aCaptionSide) { return _to GetCaptionSide(aCaptionSide); } \
  NS_SCRIPTABLE NS_IMETHOD SetCaptionSide(const nsAString & aCaptionSide) { return _to SetCaptionSide(aCaptionSide); } \
  NS_SCRIPTABLE NS_IMETHOD GetClear(nsAString & aClear) { return _to GetClear(aClear); } \
  NS_SCRIPTABLE NS_IMETHOD SetClear(const nsAString & aClear) { return _to SetClear(aClear); } \
  NS_SCRIPTABLE NS_IMETHOD GetClip(nsAString & aClip) { return _to GetClip(aClip); } \
  NS_SCRIPTABLE NS_IMETHOD SetClip(const nsAString & aClip) { return _to SetClip(aClip); } \
  NS_SCRIPTABLE NS_IMETHOD GetColor(nsAString & aColor) { return _to GetColor(aColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetColor(const nsAString & aColor) { return _to SetColor(aColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetContent(nsAString & aContent) { return _to GetContent(aContent); } \
  NS_SCRIPTABLE NS_IMETHOD SetContent(const nsAString & aContent) { return _to SetContent(aContent); } \
  NS_SCRIPTABLE NS_IMETHOD GetCounterIncrement(nsAString & aCounterIncrement) { return _to GetCounterIncrement(aCounterIncrement); } \
  NS_SCRIPTABLE NS_IMETHOD SetCounterIncrement(const nsAString & aCounterIncrement) { return _to SetCounterIncrement(aCounterIncrement); } \
  NS_SCRIPTABLE NS_IMETHOD GetCounterReset(nsAString & aCounterReset) { return _to GetCounterReset(aCounterReset); } \
  NS_SCRIPTABLE NS_IMETHOD SetCounterReset(const nsAString & aCounterReset) { return _to SetCounterReset(aCounterReset); } \
  NS_SCRIPTABLE NS_IMETHOD GetCue(nsAString & aCue) { return _to GetCue(aCue); } \
  NS_SCRIPTABLE NS_IMETHOD SetCue(const nsAString & aCue) { return _to SetCue(aCue); } \
  NS_SCRIPTABLE NS_IMETHOD GetCueAfter(nsAString & aCueAfter) { return _to GetCueAfter(aCueAfter); } \
  NS_SCRIPTABLE NS_IMETHOD SetCueAfter(const nsAString & aCueAfter) { return _to SetCueAfter(aCueAfter); } \
  NS_SCRIPTABLE NS_IMETHOD GetCueBefore(nsAString & aCueBefore) { return _to GetCueBefore(aCueBefore); } \
  NS_SCRIPTABLE NS_IMETHOD SetCueBefore(const nsAString & aCueBefore) { return _to SetCueBefore(aCueBefore); } \
  NS_SCRIPTABLE NS_IMETHOD GetCursor(nsAString & aCursor) { return _to GetCursor(aCursor); } \
  NS_SCRIPTABLE NS_IMETHOD SetCursor(const nsAString & aCursor) { return _to SetCursor(aCursor); } \
  NS_SCRIPTABLE NS_IMETHOD GetDirection(nsAString & aDirection) { return _to GetDirection(aDirection); } \
  NS_SCRIPTABLE NS_IMETHOD SetDirection(const nsAString & aDirection) { return _to SetDirection(aDirection); } \
  NS_SCRIPTABLE NS_IMETHOD GetDisplay(nsAString & aDisplay) { return _to GetDisplay(aDisplay); } \
  NS_SCRIPTABLE NS_IMETHOD SetDisplay(const nsAString & aDisplay) { return _to SetDisplay(aDisplay); } \
  NS_SCRIPTABLE NS_IMETHOD GetElevation(nsAString & aElevation) { return _to GetElevation(aElevation); } \
  NS_SCRIPTABLE NS_IMETHOD SetElevation(const nsAString & aElevation) { return _to SetElevation(aElevation); } \
  NS_SCRIPTABLE NS_IMETHOD GetEmptyCells(nsAString & aEmptyCells) { return _to GetEmptyCells(aEmptyCells); } \
  NS_SCRIPTABLE NS_IMETHOD SetEmptyCells(const nsAString & aEmptyCells) { return _to SetEmptyCells(aEmptyCells); } \
  NS_SCRIPTABLE NS_IMETHOD GetCssFloat(nsAString & aCssFloat) { return _to GetCssFloat(aCssFloat); } \
  NS_SCRIPTABLE NS_IMETHOD SetCssFloat(const nsAString & aCssFloat) { return _to SetCssFloat(aCssFloat); } \
  NS_SCRIPTABLE NS_IMETHOD GetFont(nsAString & aFont) { return _to GetFont(aFont); } \
  NS_SCRIPTABLE NS_IMETHOD SetFont(const nsAString & aFont) { return _to SetFont(aFont); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontFamily(nsAString & aFontFamily) { return _to GetFontFamily(aFontFamily); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontFamily(const nsAString & aFontFamily) { return _to SetFontFamily(aFontFamily); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontSize(nsAString & aFontSize) { return _to GetFontSize(aFontSize); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontSize(const nsAString & aFontSize) { return _to SetFontSize(aFontSize); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontSizeAdjust(nsAString & aFontSizeAdjust) { return _to GetFontSizeAdjust(aFontSizeAdjust); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontSizeAdjust(const nsAString & aFontSizeAdjust) { return _to SetFontSizeAdjust(aFontSizeAdjust); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontStretch(nsAString & aFontStretch) { return _to GetFontStretch(aFontStretch); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontStretch(const nsAString & aFontStretch) { return _to SetFontStretch(aFontStretch); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontStyle(nsAString & aFontStyle) { return _to GetFontStyle(aFontStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontStyle(const nsAString & aFontStyle) { return _to SetFontStyle(aFontStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontVariant(nsAString & aFontVariant) { return _to GetFontVariant(aFontVariant); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontVariant(const nsAString & aFontVariant) { return _to SetFontVariant(aFontVariant); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontWeight(nsAString & aFontWeight) { return _to GetFontWeight(aFontWeight); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontWeight(const nsAString & aFontWeight) { return _to SetFontWeight(aFontWeight); } \
  NS_SCRIPTABLE NS_IMETHOD GetHeight(nsAString & aHeight) { return _to GetHeight(aHeight); } \
  NS_SCRIPTABLE NS_IMETHOD SetHeight(const nsAString & aHeight) { return _to SetHeight(aHeight); } \
  NS_SCRIPTABLE NS_IMETHOD GetLeft(nsAString & aLeft) { return _to GetLeft(aLeft); } \
  NS_SCRIPTABLE NS_IMETHOD SetLeft(const nsAString & aLeft) { return _to SetLeft(aLeft); } \
  NS_SCRIPTABLE NS_IMETHOD GetLetterSpacing(nsAString & aLetterSpacing) { return _to GetLetterSpacing(aLetterSpacing); } \
  NS_SCRIPTABLE NS_IMETHOD SetLetterSpacing(const nsAString & aLetterSpacing) { return _to SetLetterSpacing(aLetterSpacing); } \
  NS_SCRIPTABLE NS_IMETHOD GetLineHeight(nsAString & aLineHeight) { return _to GetLineHeight(aLineHeight); } \
  NS_SCRIPTABLE NS_IMETHOD SetLineHeight(const nsAString & aLineHeight) { return _to SetLineHeight(aLineHeight); } \
  NS_SCRIPTABLE NS_IMETHOD GetListStyle(nsAString & aListStyle) { return _to GetListStyle(aListStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetListStyle(const nsAString & aListStyle) { return _to SetListStyle(aListStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetListStyleImage(nsAString & aListStyleImage) { return _to GetListStyleImage(aListStyleImage); } \
  NS_SCRIPTABLE NS_IMETHOD SetListStyleImage(const nsAString & aListStyleImage) { return _to SetListStyleImage(aListStyleImage); } \
  NS_SCRIPTABLE NS_IMETHOD GetListStylePosition(nsAString & aListStylePosition) { return _to GetListStylePosition(aListStylePosition); } \
  NS_SCRIPTABLE NS_IMETHOD SetListStylePosition(const nsAString & aListStylePosition) { return _to SetListStylePosition(aListStylePosition); } \
  NS_SCRIPTABLE NS_IMETHOD GetListStyleType(nsAString & aListStyleType) { return _to GetListStyleType(aListStyleType); } \
  NS_SCRIPTABLE NS_IMETHOD SetListStyleType(const nsAString & aListStyleType) { return _to SetListStyleType(aListStyleType); } \
  NS_SCRIPTABLE NS_IMETHOD GetMargin(nsAString & aMargin) { return _to GetMargin(aMargin); } \
  NS_SCRIPTABLE NS_IMETHOD SetMargin(const nsAString & aMargin) { return _to SetMargin(aMargin); } \
  NS_SCRIPTABLE NS_IMETHOD GetMarginTop(nsAString & aMarginTop) { return _to GetMarginTop(aMarginTop); } \
  NS_SCRIPTABLE NS_IMETHOD SetMarginTop(const nsAString & aMarginTop) { return _to SetMarginTop(aMarginTop); } \
  NS_SCRIPTABLE NS_IMETHOD GetMarginRight(nsAString & aMarginRight) { return _to GetMarginRight(aMarginRight); } \
  NS_SCRIPTABLE NS_IMETHOD SetMarginRight(const nsAString & aMarginRight) { return _to SetMarginRight(aMarginRight); } \
  NS_SCRIPTABLE NS_IMETHOD GetMarginBottom(nsAString & aMarginBottom) { return _to GetMarginBottom(aMarginBottom); } \
  NS_SCRIPTABLE NS_IMETHOD SetMarginBottom(const nsAString & aMarginBottom) { return _to SetMarginBottom(aMarginBottom); } \
  NS_SCRIPTABLE NS_IMETHOD GetMarginLeft(nsAString & aMarginLeft) { return _to GetMarginLeft(aMarginLeft); } \
  NS_SCRIPTABLE NS_IMETHOD SetMarginLeft(const nsAString & aMarginLeft) { return _to SetMarginLeft(aMarginLeft); } \
  NS_SCRIPTABLE NS_IMETHOD GetMarkerOffset(nsAString & aMarkerOffset) { return _to GetMarkerOffset(aMarkerOffset); } \
  NS_SCRIPTABLE NS_IMETHOD SetMarkerOffset(const nsAString & aMarkerOffset) { return _to SetMarkerOffset(aMarkerOffset); } \
  NS_SCRIPTABLE NS_IMETHOD GetMarks(nsAString & aMarks) { return _to GetMarks(aMarks); } \
  NS_SCRIPTABLE NS_IMETHOD SetMarks(const nsAString & aMarks) { return _to SetMarks(aMarks); } \
  NS_SCRIPTABLE NS_IMETHOD GetMaxHeight(nsAString & aMaxHeight) { return _to GetMaxHeight(aMaxHeight); } \
  NS_SCRIPTABLE NS_IMETHOD SetMaxHeight(const nsAString & aMaxHeight) { return _to SetMaxHeight(aMaxHeight); } \
  NS_SCRIPTABLE NS_IMETHOD GetMaxWidth(nsAString & aMaxWidth) { return _to GetMaxWidth(aMaxWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetMaxWidth(const nsAString & aMaxWidth) { return _to SetMaxWidth(aMaxWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetMinHeight(nsAString & aMinHeight) { return _to GetMinHeight(aMinHeight); } \
  NS_SCRIPTABLE NS_IMETHOD SetMinHeight(const nsAString & aMinHeight) { return _to SetMinHeight(aMinHeight); } \
  NS_SCRIPTABLE NS_IMETHOD GetMinWidth(nsAString & aMinWidth) { return _to GetMinWidth(aMinWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetMinWidth(const nsAString & aMinWidth) { return _to SetMinWidth(aMinWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetOrphans(nsAString & aOrphans) { return _to GetOrphans(aOrphans); } \
  NS_SCRIPTABLE NS_IMETHOD SetOrphans(const nsAString & aOrphans) { return _to SetOrphans(aOrphans); } \
  NS_SCRIPTABLE NS_IMETHOD GetOutline(nsAString & aOutline) { return _to GetOutline(aOutline); } \
  NS_SCRIPTABLE NS_IMETHOD SetOutline(const nsAString & aOutline) { return _to SetOutline(aOutline); } \
  NS_SCRIPTABLE NS_IMETHOD GetOutlineColor(nsAString & aOutlineColor) { return _to GetOutlineColor(aOutlineColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetOutlineColor(const nsAString & aOutlineColor) { return _to SetOutlineColor(aOutlineColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetOutlineStyle(nsAString & aOutlineStyle) { return _to GetOutlineStyle(aOutlineStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetOutlineStyle(const nsAString & aOutlineStyle) { return _to SetOutlineStyle(aOutlineStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetOutlineWidth(nsAString & aOutlineWidth) { return _to GetOutlineWidth(aOutlineWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetOutlineWidth(const nsAString & aOutlineWidth) { return _to SetOutlineWidth(aOutlineWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetOverflow(nsAString & aOverflow) { return _to GetOverflow(aOverflow); } \
  NS_SCRIPTABLE NS_IMETHOD SetOverflow(const nsAString & aOverflow) { return _to SetOverflow(aOverflow); } \
  NS_SCRIPTABLE NS_IMETHOD GetPadding(nsAString & aPadding) { return _to GetPadding(aPadding); } \
  NS_SCRIPTABLE NS_IMETHOD SetPadding(const nsAString & aPadding) { return _to SetPadding(aPadding); } \
  NS_SCRIPTABLE NS_IMETHOD GetPaddingTop(nsAString & aPaddingTop) { return _to GetPaddingTop(aPaddingTop); } \
  NS_SCRIPTABLE NS_IMETHOD SetPaddingTop(const nsAString & aPaddingTop) { return _to SetPaddingTop(aPaddingTop); } \
  NS_SCRIPTABLE NS_IMETHOD GetPaddingRight(nsAString & aPaddingRight) { return _to GetPaddingRight(aPaddingRight); } \
  NS_SCRIPTABLE NS_IMETHOD SetPaddingRight(const nsAString & aPaddingRight) { return _to SetPaddingRight(aPaddingRight); } \
  NS_SCRIPTABLE NS_IMETHOD GetPaddingBottom(nsAString & aPaddingBottom) { return _to GetPaddingBottom(aPaddingBottom); } \
  NS_SCRIPTABLE NS_IMETHOD SetPaddingBottom(const nsAString & aPaddingBottom) { return _to SetPaddingBottom(aPaddingBottom); } \
  NS_SCRIPTABLE NS_IMETHOD GetPaddingLeft(nsAString & aPaddingLeft) { return _to GetPaddingLeft(aPaddingLeft); } \
  NS_SCRIPTABLE NS_IMETHOD SetPaddingLeft(const nsAString & aPaddingLeft) { return _to SetPaddingLeft(aPaddingLeft); } \
  NS_SCRIPTABLE NS_IMETHOD GetPage(nsAString & aPage) { return _to GetPage(aPage); } \
  NS_SCRIPTABLE NS_IMETHOD SetPage(const nsAString & aPage) { return _to SetPage(aPage); } \
  NS_SCRIPTABLE NS_IMETHOD GetPageBreakAfter(nsAString & aPageBreakAfter) { return _to GetPageBreakAfter(aPageBreakAfter); } \
  NS_SCRIPTABLE NS_IMETHOD SetPageBreakAfter(const nsAString & aPageBreakAfter) { return _to SetPageBreakAfter(aPageBreakAfter); } \
  NS_SCRIPTABLE NS_IMETHOD GetPageBreakBefore(nsAString & aPageBreakBefore) { return _to GetPageBreakBefore(aPageBreakBefore); } \
  NS_SCRIPTABLE NS_IMETHOD SetPageBreakBefore(const nsAString & aPageBreakBefore) { return _to SetPageBreakBefore(aPageBreakBefore); } \
  NS_SCRIPTABLE NS_IMETHOD GetPageBreakInside(nsAString & aPageBreakInside) { return _to GetPageBreakInside(aPageBreakInside); } \
  NS_SCRIPTABLE NS_IMETHOD SetPageBreakInside(const nsAString & aPageBreakInside) { return _to SetPageBreakInside(aPageBreakInside); } \
  NS_SCRIPTABLE NS_IMETHOD GetPause(nsAString & aPause) { return _to GetPause(aPause); } \
  NS_SCRIPTABLE NS_IMETHOD SetPause(const nsAString & aPause) { return _to SetPause(aPause); } \
  NS_SCRIPTABLE NS_IMETHOD GetPauseAfter(nsAString & aPauseAfter) { return _to GetPauseAfter(aPauseAfter); } \
  NS_SCRIPTABLE NS_IMETHOD SetPauseAfter(const nsAString & aPauseAfter) { return _to SetPauseAfter(aPauseAfter); } \
  NS_SCRIPTABLE NS_IMETHOD GetPauseBefore(nsAString & aPauseBefore) { return _to GetPauseBefore(aPauseBefore); } \
  NS_SCRIPTABLE NS_IMETHOD SetPauseBefore(const nsAString & aPauseBefore) { return _to SetPauseBefore(aPauseBefore); } \
  NS_SCRIPTABLE NS_IMETHOD GetPitch(nsAString & aPitch) { return _to GetPitch(aPitch); } \
  NS_SCRIPTABLE NS_IMETHOD SetPitch(const nsAString & aPitch) { return _to SetPitch(aPitch); } \
  NS_SCRIPTABLE NS_IMETHOD GetPitchRange(nsAString & aPitchRange) { return _to GetPitchRange(aPitchRange); } \
  NS_SCRIPTABLE NS_IMETHOD SetPitchRange(const nsAString & aPitchRange) { return _to SetPitchRange(aPitchRange); } \
  NS_SCRIPTABLE NS_IMETHOD GetPosition(nsAString & aPosition) { return _to GetPosition(aPosition); } \
  NS_SCRIPTABLE NS_IMETHOD SetPosition(const nsAString & aPosition) { return _to SetPosition(aPosition); } \
  NS_SCRIPTABLE NS_IMETHOD GetQuotes(nsAString & aQuotes) { return _to GetQuotes(aQuotes); } \
  NS_SCRIPTABLE NS_IMETHOD SetQuotes(const nsAString & aQuotes) { return _to SetQuotes(aQuotes); } \
  NS_SCRIPTABLE NS_IMETHOD GetRichness(nsAString & aRichness) { return _to GetRichness(aRichness); } \
  NS_SCRIPTABLE NS_IMETHOD SetRichness(const nsAString & aRichness) { return _to SetRichness(aRichness); } \
  NS_SCRIPTABLE NS_IMETHOD GetRight(nsAString & aRight) { return _to GetRight(aRight); } \
  NS_SCRIPTABLE NS_IMETHOD SetRight(const nsAString & aRight) { return _to SetRight(aRight); } \
  NS_SCRIPTABLE NS_IMETHOD GetSize(nsAString & aSize) { return _to GetSize(aSize); } \
  NS_SCRIPTABLE NS_IMETHOD SetSize(const nsAString & aSize) { return _to SetSize(aSize); } \
  NS_SCRIPTABLE NS_IMETHOD GetSpeak(nsAString & aSpeak) { return _to GetSpeak(aSpeak); } \
  NS_SCRIPTABLE NS_IMETHOD SetSpeak(const nsAString & aSpeak) { return _to SetSpeak(aSpeak); } \
  NS_SCRIPTABLE NS_IMETHOD GetSpeakHeader(nsAString & aSpeakHeader) { return _to GetSpeakHeader(aSpeakHeader); } \
  NS_SCRIPTABLE NS_IMETHOD SetSpeakHeader(const nsAString & aSpeakHeader) { return _to SetSpeakHeader(aSpeakHeader); } \
  NS_SCRIPTABLE NS_IMETHOD GetSpeakNumeral(nsAString & aSpeakNumeral) { return _to GetSpeakNumeral(aSpeakNumeral); } \
  NS_SCRIPTABLE NS_IMETHOD SetSpeakNumeral(const nsAString & aSpeakNumeral) { return _to SetSpeakNumeral(aSpeakNumeral); } \
  NS_SCRIPTABLE NS_IMETHOD GetSpeakPunctuation(nsAString & aSpeakPunctuation) { return _to GetSpeakPunctuation(aSpeakPunctuation); } \
  NS_SCRIPTABLE NS_IMETHOD SetSpeakPunctuation(const nsAString & aSpeakPunctuation) { return _to SetSpeakPunctuation(aSpeakPunctuation); } \
  NS_SCRIPTABLE NS_IMETHOD GetSpeechRate(nsAString & aSpeechRate) { return _to GetSpeechRate(aSpeechRate); } \
  NS_SCRIPTABLE NS_IMETHOD SetSpeechRate(const nsAString & aSpeechRate) { return _to SetSpeechRate(aSpeechRate); } \
  NS_SCRIPTABLE NS_IMETHOD GetStress(nsAString & aStress) { return _to GetStress(aStress); } \
  NS_SCRIPTABLE NS_IMETHOD SetStress(const nsAString & aStress) { return _to SetStress(aStress); } \
  NS_SCRIPTABLE NS_IMETHOD GetTableLayout(nsAString & aTableLayout) { return _to GetTableLayout(aTableLayout); } \
  NS_SCRIPTABLE NS_IMETHOD SetTableLayout(const nsAString & aTableLayout) { return _to SetTableLayout(aTableLayout); } \
  NS_SCRIPTABLE NS_IMETHOD GetTextAlign(nsAString & aTextAlign) { return _to GetTextAlign(aTextAlign); } \
  NS_SCRIPTABLE NS_IMETHOD SetTextAlign(const nsAString & aTextAlign) { return _to SetTextAlign(aTextAlign); } \
  NS_SCRIPTABLE NS_IMETHOD GetTextDecoration(nsAString & aTextDecoration) { return _to GetTextDecoration(aTextDecoration); } \
  NS_SCRIPTABLE NS_IMETHOD SetTextDecoration(const nsAString & aTextDecoration) { return _to SetTextDecoration(aTextDecoration); } \
  NS_SCRIPTABLE NS_IMETHOD GetTextIndent(nsAString & aTextIndent) { return _to GetTextIndent(aTextIndent); } \
  NS_SCRIPTABLE NS_IMETHOD SetTextIndent(const nsAString & aTextIndent) { return _to SetTextIndent(aTextIndent); } \
  NS_SCRIPTABLE NS_IMETHOD GetTextShadow(nsAString & aTextShadow) { return _to GetTextShadow(aTextShadow); } \
  NS_SCRIPTABLE NS_IMETHOD SetTextShadow(const nsAString & aTextShadow) { return _to SetTextShadow(aTextShadow); } \
  NS_SCRIPTABLE NS_IMETHOD GetTextTransform(nsAString & aTextTransform) { return _to GetTextTransform(aTextTransform); } \
  NS_SCRIPTABLE NS_IMETHOD SetTextTransform(const nsAString & aTextTransform) { return _to SetTextTransform(aTextTransform); } \
  NS_SCRIPTABLE NS_IMETHOD GetTop(nsAString & aTop) { return _to GetTop(aTop); } \
  NS_SCRIPTABLE NS_IMETHOD SetTop(const nsAString & aTop) { return _to SetTop(aTop); } \
  NS_SCRIPTABLE NS_IMETHOD GetUnicodeBidi(nsAString & aUnicodeBidi) { return _to GetUnicodeBidi(aUnicodeBidi); } \
  NS_SCRIPTABLE NS_IMETHOD SetUnicodeBidi(const nsAString & aUnicodeBidi) { return _to SetUnicodeBidi(aUnicodeBidi); } \
  NS_SCRIPTABLE NS_IMETHOD GetVerticalAlign(nsAString & aVerticalAlign) { return _to GetVerticalAlign(aVerticalAlign); } \
  NS_SCRIPTABLE NS_IMETHOD SetVerticalAlign(const nsAString & aVerticalAlign) { return _to SetVerticalAlign(aVerticalAlign); } \
  NS_SCRIPTABLE NS_IMETHOD GetVisibility(nsAString & aVisibility) { return _to GetVisibility(aVisibility); } \
  NS_SCRIPTABLE NS_IMETHOD SetVisibility(const nsAString & aVisibility) { return _to SetVisibility(aVisibility); } \
  NS_SCRIPTABLE NS_IMETHOD GetVoiceFamily(nsAString & aVoiceFamily) { return _to GetVoiceFamily(aVoiceFamily); } \
  NS_SCRIPTABLE NS_IMETHOD SetVoiceFamily(const nsAString & aVoiceFamily) { return _to SetVoiceFamily(aVoiceFamily); } \
  NS_SCRIPTABLE NS_IMETHOD GetVolume(nsAString & aVolume) { return _to GetVolume(aVolume); } \
  NS_SCRIPTABLE NS_IMETHOD SetVolume(const nsAString & aVolume) { return _to SetVolume(aVolume); } \
  NS_SCRIPTABLE NS_IMETHOD GetWhiteSpace(nsAString & aWhiteSpace) { return _to GetWhiteSpace(aWhiteSpace); } \
  NS_SCRIPTABLE NS_IMETHOD SetWhiteSpace(const nsAString & aWhiteSpace) { return _to SetWhiteSpace(aWhiteSpace); } \
  NS_SCRIPTABLE NS_IMETHOD GetWidows(nsAString & aWidows) { return _to GetWidows(aWidows); } \
  NS_SCRIPTABLE NS_IMETHOD SetWidows(const nsAString & aWidows) { return _to SetWidows(aWidows); } \
  NS_SCRIPTABLE NS_IMETHOD GetWidth(nsAString & aWidth) { return _to GetWidth(aWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetWidth(const nsAString & aWidth) { return _to SetWidth(aWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetWordSpacing(nsAString & aWordSpacing) { return _to GetWordSpacing(aWordSpacing); } \
  NS_SCRIPTABLE NS_IMETHOD SetWordSpacing(const nsAString & aWordSpacing) { return _to SetWordSpacing(aWordSpacing); } \
  NS_SCRIPTABLE NS_IMETHOD GetZIndex(nsAString & aZIndex) { return _to GetZIndex(aZIndex); } \
  NS_SCRIPTABLE NS_IMETHOD SetZIndex(const nsAString & aZIndex) { return _to SetZIndex(aZIndex); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMCSS2PROPERTIES(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetAzimuth(nsAString & aAzimuth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAzimuth(aAzimuth); } \
  NS_SCRIPTABLE NS_IMETHOD SetAzimuth(const nsAString & aAzimuth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetAzimuth(aAzimuth); } \
  NS_SCRIPTABLE NS_IMETHOD GetBackground(nsAString & aBackground) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBackground(aBackground); } \
  NS_SCRIPTABLE NS_IMETHOD SetBackground(const nsAString & aBackground) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBackground(aBackground); } \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundAttachment(nsAString & aBackgroundAttachment) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBackgroundAttachment(aBackgroundAttachment); } \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundAttachment(const nsAString & aBackgroundAttachment) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBackgroundAttachment(aBackgroundAttachment); } \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundColor(nsAString & aBackgroundColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBackgroundColor(aBackgroundColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundColor(const nsAString & aBackgroundColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBackgroundColor(aBackgroundColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundImage(nsAString & aBackgroundImage) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBackgroundImage(aBackgroundImage); } \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundImage(const nsAString & aBackgroundImage) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBackgroundImage(aBackgroundImage); } \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundPosition(nsAString & aBackgroundPosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBackgroundPosition(aBackgroundPosition); } \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundPosition(const nsAString & aBackgroundPosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBackgroundPosition(aBackgroundPosition); } \
  NS_SCRIPTABLE NS_IMETHOD GetBackgroundRepeat(nsAString & aBackgroundRepeat) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBackgroundRepeat(aBackgroundRepeat); } \
  NS_SCRIPTABLE NS_IMETHOD SetBackgroundRepeat(const nsAString & aBackgroundRepeat) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBackgroundRepeat(aBackgroundRepeat); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorder(nsAString & aBorder) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorder(aBorder); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorder(const nsAString & aBorder) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorder(aBorder); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderCollapse(nsAString & aBorderCollapse) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderCollapse(aBorderCollapse); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderCollapse(const nsAString & aBorderCollapse) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderCollapse(aBorderCollapse); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderColor(nsAString & aBorderColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderColor(aBorderColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderColor(const nsAString & aBorderColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderColor(aBorderColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderSpacing(nsAString & aBorderSpacing) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderSpacing(aBorderSpacing); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderSpacing(const nsAString & aBorderSpacing) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderSpacing(aBorderSpacing); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderStyle(nsAString & aBorderStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderStyle(aBorderStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderStyle(const nsAString & aBorderStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderStyle(aBorderStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderTop(nsAString & aBorderTop) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderTop(aBorderTop); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderTop(const nsAString & aBorderTop) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderTop(aBorderTop); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderRight(nsAString & aBorderRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderRight(aBorderRight); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderRight(const nsAString & aBorderRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderRight(aBorderRight); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottom(nsAString & aBorderBottom) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderBottom(aBorderBottom); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottom(const nsAString & aBorderBottom) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderBottom(aBorderBottom); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeft(nsAString & aBorderLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderLeft(aBorderLeft); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeft(const nsAString & aBorderLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderLeft(aBorderLeft); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderTopColor(nsAString & aBorderTopColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderTopColor(aBorderTopColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderTopColor(const nsAString & aBorderTopColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderTopColor(aBorderTopColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderRightColor(nsAString & aBorderRightColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderRightColor(aBorderRightColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderRightColor(const nsAString & aBorderRightColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderRightColor(aBorderRightColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottomColor(nsAString & aBorderBottomColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderBottomColor(aBorderBottomColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottomColor(const nsAString & aBorderBottomColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderBottomColor(aBorderBottomColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeftColor(nsAString & aBorderLeftColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderLeftColor(aBorderLeftColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeftColor(const nsAString & aBorderLeftColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderLeftColor(aBorderLeftColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderTopStyle(nsAString & aBorderTopStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderTopStyle(aBorderTopStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderTopStyle(const nsAString & aBorderTopStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderTopStyle(aBorderTopStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderRightStyle(nsAString & aBorderRightStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderRightStyle(aBorderRightStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderRightStyle(const nsAString & aBorderRightStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderRightStyle(aBorderRightStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottomStyle(nsAString & aBorderBottomStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderBottomStyle(aBorderBottomStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottomStyle(const nsAString & aBorderBottomStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderBottomStyle(aBorderBottomStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeftStyle(nsAString & aBorderLeftStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderLeftStyle(aBorderLeftStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeftStyle(const nsAString & aBorderLeftStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderLeftStyle(aBorderLeftStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderTopWidth(nsAString & aBorderTopWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderTopWidth(aBorderTopWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderTopWidth(const nsAString & aBorderTopWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderTopWidth(aBorderTopWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderRightWidth(nsAString & aBorderRightWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderRightWidth(aBorderRightWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderRightWidth(const nsAString & aBorderRightWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderRightWidth(aBorderRightWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderBottomWidth(nsAString & aBorderBottomWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderBottomWidth(aBorderBottomWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderBottomWidth(const nsAString & aBorderBottomWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderBottomWidth(aBorderBottomWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderLeftWidth(nsAString & aBorderLeftWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderLeftWidth(aBorderLeftWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderLeftWidth(const nsAString & aBorderLeftWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderLeftWidth(aBorderLeftWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetBorderWidth(nsAString & aBorderWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBorderWidth(aBorderWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetBorderWidth(const nsAString & aBorderWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBorderWidth(aBorderWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetBottom(nsAString & aBottom) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBottom(aBottom); } \
  NS_SCRIPTABLE NS_IMETHOD SetBottom(const nsAString & aBottom) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetBottom(aBottom); } \
  NS_SCRIPTABLE NS_IMETHOD GetCaptionSide(nsAString & aCaptionSide) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCaptionSide(aCaptionSide); } \
  NS_SCRIPTABLE NS_IMETHOD SetCaptionSide(const nsAString & aCaptionSide) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCaptionSide(aCaptionSide); } \
  NS_SCRIPTABLE NS_IMETHOD GetClear(nsAString & aClear) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetClear(aClear); } \
  NS_SCRIPTABLE NS_IMETHOD SetClear(const nsAString & aClear) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetClear(aClear); } \
  NS_SCRIPTABLE NS_IMETHOD GetClip(nsAString & aClip) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetClip(aClip); } \
  NS_SCRIPTABLE NS_IMETHOD SetClip(const nsAString & aClip) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetClip(aClip); } \
  NS_SCRIPTABLE NS_IMETHOD GetColor(nsAString & aColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetColor(aColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetColor(const nsAString & aColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetColor(aColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetContent(nsAString & aContent) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetContent(aContent); } \
  NS_SCRIPTABLE NS_IMETHOD SetContent(const nsAString & aContent) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetContent(aContent); } \
  NS_SCRIPTABLE NS_IMETHOD GetCounterIncrement(nsAString & aCounterIncrement) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCounterIncrement(aCounterIncrement); } \
  NS_SCRIPTABLE NS_IMETHOD SetCounterIncrement(const nsAString & aCounterIncrement) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCounterIncrement(aCounterIncrement); } \
  NS_SCRIPTABLE NS_IMETHOD GetCounterReset(nsAString & aCounterReset) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCounterReset(aCounterReset); } \
  NS_SCRIPTABLE NS_IMETHOD SetCounterReset(const nsAString & aCounterReset) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCounterReset(aCounterReset); } \
  NS_SCRIPTABLE NS_IMETHOD GetCue(nsAString & aCue) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCue(aCue); } \
  NS_SCRIPTABLE NS_IMETHOD SetCue(const nsAString & aCue) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCue(aCue); } \
  NS_SCRIPTABLE NS_IMETHOD GetCueAfter(nsAString & aCueAfter) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCueAfter(aCueAfter); } \
  NS_SCRIPTABLE NS_IMETHOD SetCueAfter(const nsAString & aCueAfter) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCueAfter(aCueAfter); } \
  NS_SCRIPTABLE NS_IMETHOD GetCueBefore(nsAString & aCueBefore) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCueBefore(aCueBefore); } \
  NS_SCRIPTABLE NS_IMETHOD SetCueBefore(const nsAString & aCueBefore) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCueBefore(aCueBefore); } \
  NS_SCRIPTABLE NS_IMETHOD GetCursor(nsAString & aCursor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCursor(aCursor); } \
  NS_SCRIPTABLE NS_IMETHOD SetCursor(const nsAString & aCursor) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCursor(aCursor); } \
  NS_SCRIPTABLE NS_IMETHOD GetDirection(nsAString & aDirection) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDirection(aDirection); } \
  NS_SCRIPTABLE NS_IMETHOD SetDirection(const nsAString & aDirection) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDirection(aDirection); } \
  NS_SCRIPTABLE NS_IMETHOD GetDisplay(nsAString & aDisplay) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDisplay(aDisplay); } \
  NS_SCRIPTABLE NS_IMETHOD SetDisplay(const nsAString & aDisplay) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetDisplay(aDisplay); } \
  NS_SCRIPTABLE NS_IMETHOD GetElevation(nsAString & aElevation) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetElevation(aElevation); } \
  NS_SCRIPTABLE NS_IMETHOD SetElevation(const nsAString & aElevation) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetElevation(aElevation); } \
  NS_SCRIPTABLE NS_IMETHOD GetEmptyCells(nsAString & aEmptyCells) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetEmptyCells(aEmptyCells); } \
  NS_SCRIPTABLE NS_IMETHOD SetEmptyCells(const nsAString & aEmptyCells) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetEmptyCells(aEmptyCells); } \
  NS_SCRIPTABLE NS_IMETHOD GetCssFloat(nsAString & aCssFloat) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCssFloat(aCssFloat); } \
  NS_SCRIPTABLE NS_IMETHOD SetCssFloat(const nsAString & aCssFloat) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetCssFloat(aCssFloat); } \
  NS_SCRIPTABLE NS_IMETHOD GetFont(nsAString & aFont) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFont(aFont); } \
  NS_SCRIPTABLE NS_IMETHOD SetFont(const nsAString & aFont) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFont(aFont); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontFamily(nsAString & aFontFamily) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFontFamily(aFontFamily); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontFamily(const nsAString & aFontFamily) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFontFamily(aFontFamily); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontSize(nsAString & aFontSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFontSize(aFontSize); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontSize(const nsAString & aFontSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFontSize(aFontSize); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontSizeAdjust(nsAString & aFontSizeAdjust) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFontSizeAdjust(aFontSizeAdjust); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontSizeAdjust(const nsAString & aFontSizeAdjust) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFontSizeAdjust(aFontSizeAdjust); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontStretch(nsAString & aFontStretch) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFontStretch(aFontStretch); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontStretch(const nsAString & aFontStretch) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFontStretch(aFontStretch); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontStyle(nsAString & aFontStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFontStyle(aFontStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontStyle(const nsAString & aFontStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFontStyle(aFontStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontVariant(nsAString & aFontVariant) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFontVariant(aFontVariant); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontVariant(const nsAString & aFontVariant) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFontVariant(aFontVariant); } \
  NS_SCRIPTABLE NS_IMETHOD GetFontWeight(nsAString & aFontWeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFontWeight(aFontWeight); } \
  NS_SCRIPTABLE NS_IMETHOD SetFontWeight(const nsAString & aFontWeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFontWeight(aFontWeight); } \
  NS_SCRIPTABLE NS_IMETHOD GetHeight(nsAString & aHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetHeight(aHeight); } \
  NS_SCRIPTABLE NS_IMETHOD SetHeight(const nsAString & aHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetHeight(aHeight); } \
  NS_SCRIPTABLE NS_IMETHOD GetLeft(nsAString & aLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLeft(aLeft); } \
  NS_SCRIPTABLE NS_IMETHOD SetLeft(const nsAString & aLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLeft(aLeft); } \
  NS_SCRIPTABLE NS_IMETHOD GetLetterSpacing(nsAString & aLetterSpacing) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLetterSpacing(aLetterSpacing); } \
  NS_SCRIPTABLE NS_IMETHOD SetLetterSpacing(const nsAString & aLetterSpacing) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLetterSpacing(aLetterSpacing); } \
  NS_SCRIPTABLE NS_IMETHOD GetLineHeight(nsAString & aLineHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLineHeight(aLineHeight); } \
  NS_SCRIPTABLE NS_IMETHOD SetLineHeight(const nsAString & aLineHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLineHeight(aLineHeight); } \
  NS_SCRIPTABLE NS_IMETHOD GetListStyle(nsAString & aListStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetListStyle(aListStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetListStyle(const nsAString & aListStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetListStyle(aListStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetListStyleImage(nsAString & aListStyleImage) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetListStyleImage(aListStyleImage); } \
  NS_SCRIPTABLE NS_IMETHOD SetListStyleImage(const nsAString & aListStyleImage) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetListStyleImage(aListStyleImage); } \
  NS_SCRIPTABLE NS_IMETHOD GetListStylePosition(nsAString & aListStylePosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetListStylePosition(aListStylePosition); } \
  NS_SCRIPTABLE NS_IMETHOD SetListStylePosition(const nsAString & aListStylePosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetListStylePosition(aListStylePosition); } \
  NS_SCRIPTABLE NS_IMETHOD GetListStyleType(nsAString & aListStyleType) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetListStyleType(aListStyleType); } \
  NS_SCRIPTABLE NS_IMETHOD SetListStyleType(const nsAString & aListStyleType) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetListStyleType(aListStyleType); } \
  NS_SCRIPTABLE NS_IMETHOD GetMargin(nsAString & aMargin) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMargin(aMargin); } \
  NS_SCRIPTABLE NS_IMETHOD SetMargin(const nsAString & aMargin) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMargin(aMargin); } \
  NS_SCRIPTABLE NS_IMETHOD GetMarginTop(nsAString & aMarginTop) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMarginTop(aMarginTop); } \
  NS_SCRIPTABLE NS_IMETHOD SetMarginTop(const nsAString & aMarginTop) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMarginTop(aMarginTop); } \
  NS_SCRIPTABLE NS_IMETHOD GetMarginRight(nsAString & aMarginRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMarginRight(aMarginRight); } \
  NS_SCRIPTABLE NS_IMETHOD SetMarginRight(const nsAString & aMarginRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMarginRight(aMarginRight); } \
  NS_SCRIPTABLE NS_IMETHOD GetMarginBottom(nsAString & aMarginBottom) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMarginBottom(aMarginBottom); } \
  NS_SCRIPTABLE NS_IMETHOD SetMarginBottom(const nsAString & aMarginBottom) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMarginBottom(aMarginBottom); } \
  NS_SCRIPTABLE NS_IMETHOD GetMarginLeft(nsAString & aMarginLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMarginLeft(aMarginLeft); } \
  NS_SCRIPTABLE NS_IMETHOD SetMarginLeft(const nsAString & aMarginLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMarginLeft(aMarginLeft); } \
  NS_SCRIPTABLE NS_IMETHOD GetMarkerOffset(nsAString & aMarkerOffset) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMarkerOffset(aMarkerOffset); } \
  NS_SCRIPTABLE NS_IMETHOD SetMarkerOffset(const nsAString & aMarkerOffset) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMarkerOffset(aMarkerOffset); } \
  NS_SCRIPTABLE NS_IMETHOD GetMarks(nsAString & aMarks) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMarks(aMarks); } \
  NS_SCRIPTABLE NS_IMETHOD SetMarks(const nsAString & aMarks) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMarks(aMarks); } \
  NS_SCRIPTABLE NS_IMETHOD GetMaxHeight(nsAString & aMaxHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMaxHeight(aMaxHeight); } \
  NS_SCRIPTABLE NS_IMETHOD SetMaxHeight(const nsAString & aMaxHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMaxHeight(aMaxHeight); } \
  NS_SCRIPTABLE NS_IMETHOD GetMaxWidth(nsAString & aMaxWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMaxWidth(aMaxWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetMaxWidth(const nsAString & aMaxWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMaxWidth(aMaxWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetMinHeight(nsAString & aMinHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMinHeight(aMinHeight); } \
  NS_SCRIPTABLE NS_IMETHOD SetMinHeight(const nsAString & aMinHeight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMinHeight(aMinHeight); } \
  NS_SCRIPTABLE NS_IMETHOD GetMinWidth(nsAString & aMinWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMinWidth(aMinWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetMinWidth(const nsAString & aMinWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMinWidth(aMinWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetOrphans(nsAString & aOrphans) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOrphans(aOrphans); } \
  NS_SCRIPTABLE NS_IMETHOD SetOrphans(const nsAString & aOrphans) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOrphans(aOrphans); } \
  NS_SCRIPTABLE NS_IMETHOD GetOutline(nsAString & aOutline) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOutline(aOutline); } \
  NS_SCRIPTABLE NS_IMETHOD SetOutline(const nsAString & aOutline) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOutline(aOutline); } \
  NS_SCRIPTABLE NS_IMETHOD GetOutlineColor(nsAString & aOutlineColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOutlineColor(aOutlineColor); } \
  NS_SCRIPTABLE NS_IMETHOD SetOutlineColor(const nsAString & aOutlineColor) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOutlineColor(aOutlineColor); } \
  NS_SCRIPTABLE NS_IMETHOD GetOutlineStyle(nsAString & aOutlineStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOutlineStyle(aOutlineStyle); } \
  NS_SCRIPTABLE NS_IMETHOD SetOutlineStyle(const nsAString & aOutlineStyle) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOutlineStyle(aOutlineStyle); } \
  NS_SCRIPTABLE NS_IMETHOD GetOutlineWidth(nsAString & aOutlineWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOutlineWidth(aOutlineWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetOutlineWidth(const nsAString & aOutlineWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOutlineWidth(aOutlineWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetOverflow(nsAString & aOverflow) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOverflow(aOverflow); } \
  NS_SCRIPTABLE NS_IMETHOD SetOverflow(const nsAString & aOverflow) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOverflow(aOverflow); } \
  NS_SCRIPTABLE NS_IMETHOD GetPadding(nsAString & aPadding) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPadding(aPadding); } \
  NS_SCRIPTABLE NS_IMETHOD SetPadding(const nsAString & aPadding) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPadding(aPadding); } \
  NS_SCRIPTABLE NS_IMETHOD GetPaddingTop(nsAString & aPaddingTop) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPaddingTop(aPaddingTop); } \
  NS_SCRIPTABLE NS_IMETHOD SetPaddingTop(const nsAString & aPaddingTop) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPaddingTop(aPaddingTop); } \
  NS_SCRIPTABLE NS_IMETHOD GetPaddingRight(nsAString & aPaddingRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPaddingRight(aPaddingRight); } \
  NS_SCRIPTABLE NS_IMETHOD SetPaddingRight(const nsAString & aPaddingRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPaddingRight(aPaddingRight); } \
  NS_SCRIPTABLE NS_IMETHOD GetPaddingBottom(nsAString & aPaddingBottom) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPaddingBottom(aPaddingBottom); } \
  NS_SCRIPTABLE NS_IMETHOD SetPaddingBottom(const nsAString & aPaddingBottom) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPaddingBottom(aPaddingBottom); } \
  NS_SCRIPTABLE NS_IMETHOD GetPaddingLeft(nsAString & aPaddingLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPaddingLeft(aPaddingLeft); } \
  NS_SCRIPTABLE NS_IMETHOD SetPaddingLeft(const nsAString & aPaddingLeft) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPaddingLeft(aPaddingLeft); } \
  NS_SCRIPTABLE NS_IMETHOD GetPage(nsAString & aPage) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPage(aPage); } \
  NS_SCRIPTABLE NS_IMETHOD SetPage(const nsAString & aPage) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPage(aPage); } \
  NS_SCRIPTABLE NS_IMETHOD GetPageBreakAfter(nsAString & aPageBreakAfter) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPageBreakAfter(aPageBreakAfter); } \
  NS_SCRIPTABLE NS_IMETHOD SetPageBreakAfter(const nsAString & aPageBreakAfter) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPageBreakAfter(aPageBreakAfter); } \
  NS_SCRIPTABLE NS_IMETHOD GetPageBreakBefore(nsAString & aPageBreakBefore) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPageBreakBefore(aPageBreakBefore); } \
  NS_SCRIPTABLE NS_IMETHOD SetPageBreakBefore(const nsAString & aPageBreakBefore) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPageBreakBefore(aPageBreakBefore); } \
  NS_SCRIPTABLE NS_IMETHOD GetPageBreakInside(nsAString & aPageBreakInside) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPageBreakInside(aPageBreakInside); } \
  NS_SCRIPTABLE NS_IMETHOD SetPageBreakInside(const nsAString & aPageBreakInside) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPageBreakInside(aPageBreakInside); } \
  NS_SCRIPTABLE NS_IMETHOD GetPause(nsAString & aPause) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPause(aPause); } \
  NS_SCRIPTABLE NS_IMETHOD SetPause(const nsAString & aPause) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPause(aPause); } \
  NS_SCRIPTABLE NS_IMETHOD GetPauseAfter(nsAString & aPauseAfter) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPauseAfter(aPauseAfter); } \
  NS_SCRIPTABLE NS_IMETHOD SetPauseAfter(const nsAString & aPauseAfter) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPauseAfter(aPauseAfter); } \
  NS_SCRIPTABLE NS_IMETHOD GetPauseBefore(nsAString & aPauseBefore) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPauseBefore(aPauseBefore); } \
  NS_SCRIPTABLE NS_IMETHOD SetPauseBefore(const nsAString & aPauseBefore) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPauseBefore(aPauseBefore); } \
  NS_SCRIPTABLE NS_IMETHOD GetPitch(nsAString & aPitch) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPitch(aPitch); } \
  NS_SCRIPTABLE NS_IMETHOD SetPitch(const nsAString & aPitch) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPitch(aPitch); } \
  NS_SCRIPTABLE NS_IMETHOD GetPitchRange(nsAString & aPitchRange) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPitchRange(aPitchRange); } \
  NS_SCRIPTABLE NS_IMETHOD SetPitchRange(const nsAString & aPitchRange) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPitchRange(aPitchRange); } \
  NS_SCRIPTABLE NS_IMETHOD GetPosition(nsAString & aPosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPosition(aPosition); } \
  NS_SCRIPTABLE NS_IMETHOD SetPosition(const nsAString & aPosition) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetPosition(aPosition); } \
  NS_SCRIPTABLE NS_IMETHOD GetQuotes(nsAString & aQuotes) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetQuotes(aQuotes); } \
  NS_SCRIPTABLE NS_IMETHOD SetQuotes(const nsAString & aQuotes) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetQuotes(aQuotes); } \
  NS_SCRIPTABLE NS_IMETHOD GetRichness(nsAString & aRichness) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRichness(aRichness); } \
  NS_SCRIPTABLE NS_IMETHOD SetRichness(const nsAString & aRichness) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetRichness(aRichness); } \
  NS_SCRIPTABLE NS_IMETHOD GetRight(nsAString & aRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRight(aRight); } \
  NS_SCRIPTABLE NS_IMETHOD SetRight(const nsAString & aRight) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetRight(aRight); } \
  NS_SCRIPTABLE NS_IMETHOD GetSize(nsAString & aSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSize(aSize); } \
  NS_SCRIPTABLE NS_IMETHOD SetSize(const nsAString & aSize) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSize(aSize); } \
  NS_SCRIPTABLE NS_IMETHOD GetSpeak(nsAString & aSpeak) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSpeak(aSpeak); } \
  NS_SCRIPTABLE NS_IMETHOD SetSpeak(const nsAString & aSpeak) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSpeak(aSpeak); } \
  NS_SCRIPTABLE NS_IMETHOD GetSpeakHeader(nsAString & aSpeakHeader) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSpeakHeader(aSpeakHeader); } \
  NS_SCRIPTABLE NS_IMETHOD SetSpeakHeader(const nsAString & aSpeakHeader) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSpeakHeader(aSpeakHeader); } \
  NS_SCRIPTABLE NS_IMETHOD GetSpeakNumeral(nsAString & aSpeakNumeral) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSpeakNumeral(aSpeakNumeral); } \
  NS_SCRIPTABLE NS_IMETHOD SetSpeakNumeral(const nsAString & aSpeakNumeral) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSpeakNumeral(aSpeakNumeral); } \
  NS_SCRIPTABLE NS_IMETHOD GetSpeakPunctuation(nsAString & aSpeakPunctuation) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSpeakPunctuation(aSpeakPunctuation); } \
  NS_SCRIPTABLE NS_IMETHOD SetSpeakPunctuation(const nsAString & aSpeakPunctuation) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSpeakPunctuation(aSpeakPunctuation); } \
  NS_SCRIPTABLE NS_IMETHOD GetSpeechRate(nsAString & aSpeechRate) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSpeechRate(aSpeechRate); } \
  NS_SCRIPTABLE NS_IMETHOD SetSpeechRate(const nsAString & aSpeechRate) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSpeechRate(aSpeechRate); } \
  NS_SCRIPTABLE NS_IMETHOD GetStress(nsAString & aStress) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStress(aStress); } \
  NS_SCRIPTABLE NS_IMETHOD SetStress(const nsAString & aStress) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetStress(aStress); } \
  NS_SCRIPTABLE NS_IMETHOD GetTableLayout(nsAString & aTableLayout) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTableLayout(aTableLayout); } \
  NS_SCRIPTABLE NS_IMETHOD SetTableLayout(const nsAString & aTableLayout) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTableLayout(aTableLayout); } \
  NS_SCRIPTABLE NS_IMETHOD GetTextAlign(nsAString & aTextAlign) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTextAlign(aTextAlign); } \
  NS_SCRIPTABLE NS_IMETHOD SetTextAlign(const nsAString & aTextAlign) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTextAlign(aTextAlign); } \
  NS_SCRIPTABLE NS_IMETHOD GetTextDecoration(nsAString & aTextDecoration) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTextDecoration(aTextDecoration); } \
  NS_SCRIPTABLE NS_IMETHOD SetTextDecoration(const nsAString & aTextDecoration) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTextDecoration(aTextDecoration); } \
  NS_SCRIPTABLE NS_IMETHOD GetTextIndent(nsAString & aTextIndent) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTextIndent(aTextIndent); } \
  NS_SCRIPTABLE NS_IMETHOD SetTextIndent(const nsAString & aTextIndent) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTextIndent(aTextIndent); } \
  NS_SCRIPTABLE NS_IMETHOD GetTextShadow(nsAString & aTextShadow) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTextShadow(aTextShadow); } \
  NS_SCRIPTABLE NS_IMETHOD SetTextShadow(const nsAString & aTextShadow) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTextShadow(aTextShadow); } \
  NS_SCRIPTABLE NS_IMETHOD GetTextTransform(nsAString & aTextTransform) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTextTransform(aTextTransform); } \
  NS_SCRIPTABLE NS_IMETHOD SetTextTransform(const nsAString & aTextTransform) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTextTransform(aTextTransform); } \
  NS_SCRIPTABLE NS_IMETHOD GetTop(nsAString & aTop) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTop(aTop); } \
  NS_SCRIPTABLE NS_IMETHOD SetTop(const nsAString & aTop) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetTop(aTop); } \
  NS_SCRIPTABLE NS_IMETHOD GetUnicodeBidi(nsAString & aUnicodeBidi) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetUnicodeBidi(aUnicodeBidi); } \
  NS_SCRIPTABLE NS_IMETHOD SetUnicodeBidi(const nsAString & aUnicodeBidi) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetUnicodeBidi(aUnicodeBidi); } \
  NS_SCRIPTABLE NS_IMETHOD GetVerticalAlign(nsAString & aVerticalAlign) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetVerticalAlign(aVerticalAlign); } \
  NS_SCRIPTABLE NS_IMETHOD SetVerticalAlign(const nsAString & aVerticalAlign) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetVerticalAlign(aVerticalAlign); } \
  NS_SCRIPTABLE NS_IMETHOD GetVisibility(nsAString & aVisibility) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetVisibility(aVisibility); } \
  NS_SCRIPTABLE NS_IMETHOD SetVisibility(const nsAString & aVisibility) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetVisibility(aVisibility); } \
  NS_SCRIPTABLE NS_IMETHOD GetVoiceFamily(nsAString & aVoiceFamily) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetVoiceFamily(aVoiceFamily); } \
  NS_SCRIPTABLE NS_IMETHOD SetVoiceFamily(const nsAString & aVoiceFamily) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetVoiceFamily(aVoiceFamily); } \
  NS_SCRIPTABLE NS_IMETHOD GetVolume(nsAString & aVolume) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetVolume(aVolume); } \
  NS_SCRIPTABLE NS_IMETHOD SetVolume(const nsAString & aVolume) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetVolume(aVolume); } \
  NS_SCRIPTABLE NS_IMETHOD GetWhiteSpace(nsAString & aWhiteSpace) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWhiteSpace(aWhiteSpace); } \
  NS_SCRIPTABLE NS_IMETHOD SetWhiteSpace(const nsAString & aWhiteSpace) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetWhiteSpace(aWhiteSpace); } \
  NS_SCRIPTABLE NS_IMETHOD GetWidows(nsAString & aWidows) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWidows(aWidows); } \
  NS_SCRIPTABLE NS_IMETHOD SetWidows(const nsAString & aWidows) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetWidows(aWidows); } \
  NS_SCRIPTABLE NS_IMETHOD GetWidth(nsAString & aWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWidth(aWidth); } \
  NS_SCRIPTABLE NS_IMETHOD SetWidth(const nsAString & aWidth) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetWidth(aWidth); } \
  NS_SCRIPTABLE NS_IMETHOD GetWordSpacing(nsAString & aWordSpacing) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWordSpacing(aWordSpacing); } \
  NS_SCRIPTABLE NS_IMETHOD SetWordSpacing(const nsAString & aWordSpacing) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetWordSpacing(aWordSpacing); } \
  NS_SCRIPTABLE NS_IMETHOD GetZIndex(nsAString & aZIndex) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetZIndex(aZIndex); } \
  NS_SCRIPTABLE NS_IMETHOD SetZIndex(const nsAString & aZIndex) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetZIndex(aZIndex); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMCSS2Properties : public nsIDOMCSS2Properties
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMCSS2PROPERTIES

  nsDOMCSS2Properties();

private:
  ~nsDOMCSS2Properties();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMCSS2Properties, nsIDOMCSS2Properties)

nsDOMCSS2Properties::nsDOMCSS2Properties()
{
  /* member initializers and constructor code */
}

nsDOMCSS2Properties::~nsDOMCSS2Properties()
{
  /* destructor code */
}

/* attribute DOMString azimuth; */
NS_IMETHODIMP nsDOMCSS2Properties::GetAzimuth(nsAString & aAzimuth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetAzimuth(const nsAString & aAzimuth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString background; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBackground(nsAString & aBackground)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBackground(const nsAString & aBackground)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString backgroundAttachment; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBackgroundAttachment(nsAString & aBackgroundAttachment)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBackgroundAttachment(const nsAString & aBackgroundAttachment)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString backgroundColor; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBackgroundColor(nsAString & aBackgroundColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBackgroundColor(const nsAString & aBackgroundColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString backgroundImage; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBackgroundImage(nsAString & aBackgroundImage)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBackgroundImage(const nsAString & aBackgroundImage)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString backgroundPosition; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBackgroundPosition(nsAString & aBackgroundPosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBackgroundPosition(const nsAString & aBackgroundPosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString backgroundRepeat; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBackgroundRepeat(nsAString & aBackgroundRepeat)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBackgroundRepeat(const nsAString & aBackgroundRepeat)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString border; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorder(nsAString & aBorder)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorder(const nsAString & aBorder)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderCollapse; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderCollapse(nsAString & aBorderCollapse)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderCollapse(const nsAString & aBorderCollapse)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderColor; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderColor(nsAString & aBorderColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderColor(const nsAString & aBorderColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderSpacing; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderSpacing(nsAString & aBorderSpacing)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderSpacing(const nsAString & aBorderSpacing)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderStyle; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderStyle(nsAString & aBorderStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderStyle(const nsAString & aBorderStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderTop; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderTop(nsAString & aBorderTop)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderTop(const nsAString & aBorderTop)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderRight; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderRight(nsAString & aBorderRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderRight(const nsAString & aBorderRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderBottom; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderBottom(nsAString & aBorderBottom)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderBottom(const nsAString & aBorderBottom)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderLeft; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderLeft(nsAString & aBorderLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderLeft(const nsAString & aBorderLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderTopColor; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderTopColor(nsAString & aBorderTopColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderTopColor(const nsAString & aBorderTopColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderRightColor; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderRightColor(nsAString & aBorderRightColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderRightColor(const nsAString & aBorderRightColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderBottomColor; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderBottomColor(nsAString & aBorderBottomColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderBottomColor(const nsAString & aBorderBottomColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderLeftColor; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderLeftColor(nsAString & aBorderLeftColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderLeftColor(const nsAString & aBorderLeftColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderTopStyle; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderTopStyle(nsAString & aBorderTopStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderTopStyle(const nsAString & aBorderTopStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderRightStyle; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderRightStyle(nsAString & aBorderRightStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderRightStyle(const nsAString & aBorderRightStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderBottomStyle; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderBottomStyle(nsAString & aBorderBottomStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderBottomStyle(const nsAString & aBorderBottomStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderLeftStyle; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderLeftStyle(nsAString & aBorderLeftStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderLeftStyle(const nsAString & aBorderLeftStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderTopWidth; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderTopWidth(nsAString & aBorderTopWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderTopWidth(const nsAString & aBorderTopWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderRightWidth; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderRightWidth(nsAString & aBorderRightWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderRightWidth(const nsAString & aBorderRightWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderBottomWidth; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderBottomWidth(nsAString & aBorderBottomWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderBottomWidth(const nsAString & aBorderBottomWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderLeftWidth; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderLeftWidth(nsAString & aBorderLeftWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderLeftWidth(const nsAString & aBorderLeftWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString borderWidth; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBorderWidth(nsAString & aBorderWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBorderWidth(const nsAString & aBorderWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString bottom; */
NS_IMETHODIMP nsDOMCSS2Properties::GetBottom(nsAString & aBottom)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetBottom(const nsAString & aBottom)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString captionSide; */
NS_IMETHODIMP nsDOMCSS2Properties::GetCaptionSide(nsAString & aCaptionSide)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetCaptionSide(const nsAString & aCaptionSide)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString clear; */
NS_IMETHODIMP nsDOMCSS2Properties::GetClear(nsAString & aClear)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetClear(const nsAString & aClear)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString clip; */
NS_IMETHODIMP nsDOMCSS2Properties::GetClip(nsAString & aClip)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetClip(const nsAString & aClip)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString color; */
NS_IMETHODIMP nsDOMCSS2Properties::GetColor(nsAString & aColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetColor(const nsAString & aColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString content; */
NS_IMETHODIMP nsDOMCSS2Properties::GetContent(nsAString & aContent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetContent(const nsAString & aContent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString counterIncrement; */
NS_IMETHODIMP nsDOMCSS2Properties::GetCounterIncrement(nsAString & aCounterIncrement)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetCounterIncrement(const nsAString & aCounterIncrement)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString counterReset; */
NS_IMETHODIMP nsDOMCSS2Properties::GetCounterReset(nsAString & aCounterReset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetCounterReset(const nsAString & aCounterReset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString cue; */
NS_IMETHODIMP nsDOMCSS2Properties::GetCue(nsAString & aCue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetCue(const nsAString & aCue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString cueAfter; */
NS_IMETHODIMP nsDOMCSS2Properties::GetCueAfter(nsAString & aCueAfter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetCueAfter(const nsAString & aCueAfter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString cueBefore; */
NS_IMETHODIMP nsDOMCSS2Properties::GetCueBefore(nsAString & aCueBefore)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetCueBefore(const nsAString & aCueBefore)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString cursor; */
NS_IMETHODIMP nsDOMCSS2Properties::GetCursor(nsAString & aCursor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetCursor(const nsAString & aCursor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString direction; */
NS_IMETHODIMP nsDOMCSS2Properties::GetDirection(nsAString & aDirection)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetDirection(const nsAString & aDirection)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString display; */
NS_IMETHODIMP nsDOMCSS2Properties::GetDisplay(nsAString & aDisplay)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetDisplay(const nsAString & aDisplay)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString elevation; */
NS_IMETHODIMP nsDOMCSS2Properties::GetElevation(nsAString & aElevation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetElevation(const nsAString & aElevation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString emptyCells; */
NS_IMETHODIMP nsDOMCSS2Properties::GetEmptyCells(nsAString & aEmptyCells)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetEmptyCells(const nsAString & aEmptyCells)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString cssFloat; */
NS_IMETHODIMP nsDOMCSS2Properties::GetCssFloat(nsAString & aCssFloat)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetCssFloat(const nsAString & aCssFloat)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString font; */
NS_IMETHODIMP nsDOMCSS2Properties::GetFont(nsAString & aFont)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetFont(const nsAString & aFont)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString fontFamily; */
NS_IMETHODIMP nsDOMCSS2Properties::GetFontFamily(nsAString & aFontFamily)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetFontFamily(const nsAString & aFontFamily)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString fontSize; */
NS_IMETHODIMP nsDOMCSS2Properties::GetFontSize(nsAString & aFontSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetFontSize(const nsAString & aFontSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString fontSizeAdjust; */
NS_IMETHODIMP nsDOMCSS2Properties::GetFontSizeAdjust(nsAString & aFontSizeAdjust)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetFontSizeAdjust(const nsAString & aFontSizeAdjust)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString fontStretch; */
NS_IMETHODIMP nsDOMCSS2Properties::GetFontStretch(nsAString & aFontStretch)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetFontStretch(const nsAString & aFontStretch)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString fontStyle; */
NS_IMETHODIMP nsDOMCSS2Properties::GetFontStyle(nsAString & aFontStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetFontStyle(const nsAString & aFontStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString fontVariant; */
NS_IMETHODIMP nsDOMCSS2Properties::GetFontVariant(nsAString & aFontVariant)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetFontVariant(const nsAString & aFontVariant)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString fontWeight; */
NS_IMETHODIMP nsDOMCSS2Properties::GetFontWeight(nsAString & aFontWeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetFontWeight(const nsAString & aFontWeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString height; */
NS_IMETHODIMP nsDOMCSS2Properties::GetHeight(nsAString & aHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetHeight(const nsAString & aHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString left; */
NS_IMETHODIMP nsDOMCSS2Properties::GetLeft(nsAString & aLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetLeft(const nsAString & aLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString letterSpacing; */
NS_IMETHODIMP nsDOMCSS2Properties::GetLetterSpacing(nsAString & aLetterSpacing)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetLetterSpacing(const nsAString & aLetterSpacing)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString lineHeight; */
NS_IMETHODIMP nsDOMCSS2Properties::GetLineHeight(nsAString & aLineHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetLineHeight(const nsAString & aLineHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString listStyle; */
NS_IMETHODIMP nsDOMCSS2Properties::GetListStyle(nsAString & aListStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetListStyle(const nsAString & aListStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString listStyleImage; */
NS_IMETHODIMP nsDOMCSS2Properties::GetListStyleImage(nsAString & aListStyleImage)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetListStyleImage(const nsAString & aListStyleImage)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString listStylePosition; */
NS_IMETHODIMP nsDOMCSS2Properties::GetListStylePosition(nsAString & aListStylePosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetListStylePosition(const nsAString & aListStylePosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString listStyleType; */
NS_IMETHODIMP nsDOMCSS2Properties::GetListStyleType(nsAString & aListStyleType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetListStyleType(const nsAString & aListStyleType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString margin; */
NS_IMETHODIMP nsDOMCSS2Properties::GetMargin(nsAString & aMargin)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetMargin(const nsAString & aMargin)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString marginTop; */
NS_IMETHODIMP nsDOMCSS2Properties::GetMarginTop(nsAString & aMarginTop)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetMarginTop(const nsAString & aMarginTop)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString marginRight; */
NS_IMETHODIMP nsDOMCSS2Properties::GetMarginRight(nsAString & aMarginRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetMarginRight(const nsAString & aMarginRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString marginBottom; */
NS_IMETHODIMP nsDOMCSS2Properties::GetMarginBottom(nsAString & aMarginBottom)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetMarginBottom(const nsAString & aMarginBottom)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString marginLeft; */
NS_IMETHODIMP nsDOMCSS2Properties::GetMarginLeft(nsAString & aMarginLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetMarginLeft(const nsAString & aMarginLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString markerOffset; */
NS_IMETHODIMP nsDOMCSS2Properties::GetMarkerOffset(nsAString & aMarkerOffset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetMarkerOffset(const nsAString & aMarkerOffset)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString marks; */
NS_IMETHODIMP nsDOMCSS2Properties::GetMarks(nsAString & aMarks)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetMarks(const nsAString & aMarks)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString maxHeight; */
NS_IMETHODIMP nsDOMCSS2Properties::GetMaxHeight(nsAString & aMaxHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetMaxHeight(const nsAString & aMaxHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString maxWidth; */
NS_IMETHODIMP nsDOMCSS2Properties::GetMaxWidth(nsAString & aMaxWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetMaxWidth(const nsAString & aMaxWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString minHeight; */
NS_IMETHODIMP nsDOMCSS2Properties::GetMinHeight(nsAString & aMinHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetMinHeight(const nsAString & aMinHeight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString minWidth; */
NS_IMETHODIMP nsDOMCSS2Properties::GetMinWidth(nsAString & aMinWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetMinWidth(const nsAString & aMinWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString orphans; */
NS_IMETHODIMP nsDOMCSS2Properties::GetOrphans(nsAString & aOrphans)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetOrphans(const nsAString & aOrphans)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString outline; */
NS_IMETHODIMP nsDOMCSS2Properties::GetOutline(nsAString & aOutline)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetOutline(const nsAString & aOutline)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString outlineColor; */
NS_IMETHODIMP nsDOMCSS2Properties::GetOutlineColor(nsAString & aOutlineColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetOutlineColor(const nsAString & aOutlineColor)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString outlineStyle; */
NS_IMETHODIMP nsDOMCSS2Properties::GetOutlineStyle(nsAString & aOutlineStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetOutlineStyle(const nsAString & aOutlineStyle)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString outlineWidth; */
NS_IMETHODIMP nsDOMCSS2Properties::GetOutlineWidth(nsAString & aOutlineWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetOutlineWidth(const nsAString & aOutlineWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString overflow; */
NS_IMETHODIMP nsDOMCSS2Properties::GetOverflow(nsAString & aOverflow)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetOverflow(const nsAString & aOverflow)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString padding; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPadding(nsAString & aPadding)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPadding(const nsAString & aPadding)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString paddingTop; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPaddingTop(nsAString & aPaddingTop)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPaddingTop(const nsAString & aPaddingTop)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString paddingRight; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPaddingRight(nsAString & aPaddingRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPaddingRight(const nsAString & aPaddingRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString paddingBottom; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPaddingBottom(nsAString & aPaddingBottom)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPaddingBottom(const nsAString & aPaddingBottom)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString paddingLeft; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPaddingLeft(nsAString & aPaddingLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPaddingLeft(const nsAString & aPaddingLeft)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString page; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPage(nsAString & aPage)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPage(const nsAString & aPage)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString pageBreakAfter; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPageBreakAfter(nsAString & aPageBreakAfter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPageBreakAfter(const nsAString & aPageBreakAfter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString pageBreakBefore; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPageBreakBefore(nsAString & aPageBreakBefore)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPageBreakBefore(const nsAString & aPageBreakBefore)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString pageBreakInside; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPageBreakInside(nsAString & aPageBreakInside)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPageBreakInside(const nsAString & aPageBreakInside)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString pause; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPause(nsAString & aPause)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPause(const nsAString & aPause)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString pauseAfter; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPauseAfter(nsAString & aPauseAfter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPauseAfter(const nsAString & aPauseAfter)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString pauseBefore; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPauseBefore(nsAString & aPauseBefore)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPauseBefore(const nsAString & aPauseBefore)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString pitch; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPitch(nsAString & aPitch)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPitch(const nsAString & aPitch)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString pitchRange; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPitchRange(nsAString & aPitchRange)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPitchRange(const nsAString & aPitchRange)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString position; */
NS_IMETHODIMP nsDOMCSS2Properties::GetPosition(nsAString & aPosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetPosition(const nsAString & aPosition)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString quotes; */
NS_IMETHODIMP nsDOMCSS2Properties::GetQuotes(nsAString & aQuotes)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetQuotes(const nsAString & aQuotes)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString richness; */
NS_IMETHODIMP nsDOMCSS2Properties::GetRichness(nsAString & aRichness)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetRichness(const nsAString & aRichness)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString right; */
NS_IMETHODIMP nsDOMCSS2Properties::GetRight(nsAString & aRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetRight(const nsAString & aRight)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString size; */
NS_IMETHODIMP nsDOMCSS2Properties::GetSize(nsAString & aSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetSize(const nsAString & aSize)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString speak; */
NS_IMETHODIMP nsDOMCSS2Properties::GetSpeak(nsAString & aSpeak)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetSpeak(const nsAString & aSpeak)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString speakHeader; */
NS_IMETHODIMP nsDOMCSS2Properties::GetSpeakHeader(nsAString & aSpeakHeader)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetSpeakHeader(const nsAString & aSpeakHeader)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString speakNumeral; */
NS_IMETHODIMP nsDOMCSS2Properties::GetSpeakNumeral(nsAString & aSpeakNumeral)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetSpeakNumeral(const nsAString & aSpeakNumeral)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString speakPunctuation; */
NS_IMETHODIMP nsDOMCSS2Properties::GetSpeakPunctuation(nsAString & aSpeakPunctuation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetSpeakPunctuation(const nsAString & aSpeakPunctuation)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString speechRate; */
NS_IMETHODIMP nsDOMCSS2Properties::GetSpeechRate(nsAString & aSpeechRate)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetSpeechRate(const nsAString & aSpeechRate)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString stress; */
NS_IMETHODIMP nsDOMCSS2Properties::GetStress(nsAString & aStress)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetStress(const nsAString & aStress)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString tableLayout; */
NS_IMETHODIMP nsDOMCSS2Properties::GetTableLayout(nsAString & aTableLayout)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetTableLayout(const nsAString & aTableLayout)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString textAlign; */
NS_IMETHODIMP nsDOMCSS2Properties::GetTextAlign(nsAString & aTextAlign)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetTextAlign(const nsAString & aTextAlign)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString textDecoration; */
NS_IMETHODIMP nsDOMCSS2Properties::GetTextDecoration(nsAString & aTextDecoration)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetTextDecoration(const nsAString & aTextDecoration)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString textIndent; */
NS_IMETHODIMP nsDOMCSS2Properties::GetTextIndent(nsAString & aTextIndent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetTextIndent(const nsAString & aTextIndent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString textShadow; */
NS_IMETHODIMP nsDOMCSS2Properties::GetTextShadow(nsAString & aTextShadow)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetTextShadow(const nsAString & aTextShadow)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString textTransform; */
NS_IMETHODIMP nsDOMCSS2Properties::GetTextTransform(nsAString & aTextTransform)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetTextTransform(const nsAString & aTextTransform)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString top; */
NS_IMETHODIMP nsDOMCSS2Properties::GetTop(nsAString & aTop)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetTop(const nsAString & aTop)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString unicodeBidi; */
NS_IMETHODIMP nsDOMCSS2Properties::GetUnicodeBidi(nsAString & aUnicodeBidi)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetUnicodeBidi(const nsAString & aUnicodeBidi)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString verticalAlign; */
NS_IMETHODIMP nsDOMCSS2Properties::GetVerticalAlign(nsAString & aVerticalAlign)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetVerticalAlign(const nsAString & aVerticalAlign)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString visibility; */
NS_IMETHODIMP nsDOMCSS2Properties::GetVisibility(nsAString & aVisibility)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetVisibility(const nsAString & aVisibility)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString voiceFamily; */
NS_IMETHODIMP nsDOMCSS2Properties::GetVoiceFamily(nsAString & aVoiceFamily)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetVoiceFamily(const nsAString & aVoiceFamily)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString volume; */
NS_IMETHODIMP nsDOMCSS2Properties::GetVolume(nsAString & aVolume)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetVolume(const nsAString & aVolume)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString whiteSpace; */
NS_IMETHODIMP nsDOMCSS2Properties::GetWhiteSpace(nsAString & aWhiteSpace)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetWhiteSpace(const nsAString & aWhiteSpace)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString widows; */
NS_IMETHODIMP nsDOMCSS2Properties::GetWidows(nsAString & aWidows)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetWidows(const nsAString & aWidows)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString width; */
NS_IMETHODIMP nsDOMCSS2Properties::GetWidth(nsAString & aWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetWidth(const nsAString & aWidth)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString wordSpacing; */
NS_IMETHODIMP nsDOMCSS2Properties::GetWordSpacing(nsAString & aWordSpacing)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetWordSpacing(const nsAString & aWordSpacing)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute DOMString zIndex; */
NS_IMETHODIMP nsDOMCSS2Properties::GetZIndex(nsAString & aZIndex)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMCSS2Properties::SetZIndex(const nsAString & aZIndex)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMCSS2Properties_h__ */
