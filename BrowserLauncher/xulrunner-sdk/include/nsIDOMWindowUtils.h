/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/dom/interfaces/base/nsIDOMWindowUtils.idl
 */

#ifndef __gen_nsIDOMWindowUtils_h__
#define __gen_nsIDOMWindowUtils_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMNode; /* forward declaration */

class nsIDOMNodeList; /* forward declaration */

class nsIDOMElement; /* forward declaration */

class nsIDOMHTMLCanvasElement; /* forward declaration */

class nsIDOMEvent; /* forward declaration */

class nsIQueryContentEventResult; /* forward declaration */


/* starting interface:    nsIDOMWindowUtils */
#define NS_IDOMWINDOWUTILS_IID_STR "6a60fde5-a00a-4732-bbea-2787c174c04f"

#define NS_IDOMWINDOWUTILS_IID \
  {0x6a60fde5, 0xa00a, 0x4732, \
    { 0xbb, 0xea, 0x27, 0x87, 0xc1, 0x74, 0xc0, 0x4f }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIDOMWindowUtils : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMWINDOWUTILS_IID)

  /**
   * Image animation mode of the window. When this attribute's value
   * is changed, the implementation should set all images in the window
   * to the given value. That is, when set to kDontAnimMode, all images
   * will stop animating. The attribute's value must be one of the
   * animationMode values from imgIContainer.
   * @note Images may individually override the window's setting after
   *       the window's mode is set. Therefore images given different modes
   *       since the last setting of the window's mode may behave
   *       out of line with the window's overall mode.
   * @note The attribute's value is the window's overall mode. It may
   *       for example continue to report kDontAnimMode after all images
   *       have subsequently been individually animated.
   * @note Only images immediately in this window are affected;
   *       this is not recursive to subwindows.
   * @see imgIContainer
   */
  /* attribute unsigned short imageAnimationMode; */
  NS_SCRIPTABLE NS_IMETHOD GetImageAnimationMode(PRUint16 *aImageAnimationMode) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetImageAnimationMode(PRUint16 aImageAnimationMode) = 0;

  /**
   * Whether the charset of the window's current document has been forced by
   * the user.
   * Cannot be accessed from unprivileged context (not content-accessible)
   */
  /* readonly attribute boolean docCharsetIsForced; */
  NS_SCRIPTABLE NS_IMETHOD GetDocCharsetIsForced(PRBool *aDocCharsetIsForced) = 0;

  /**
   * Function to get metadata associated with the window's current document
   * @param aName the name of the metadata.  This should be all lowercase.
   * @return the value of the metadata, or the empty string if it's not set
   *
   * Will throw a DOM security error if called without UniversalXPConnect
   * privileges.
   */
  /* AString getDocumentMetadata (in AString aName); */
  NS_SCRIPTABLE NS_IMETHOD GetDocumentMetadata(const nsAString & aName, nsAString & _retval NS_OUTPARAM) = 0;

  /**
   * Force an immediate redraw of this window.  The parameter specifies
   * the number of times to redraw, and the return value is the length,
   * in milliseconds, that the redraws took.  If aCount is not specified
   * or is 0, it is taken to be 1.
   */
  /* unsigned long redraw ([optional] in unsigned long aCount); */
  NS_SCRIPTABLE NS_IMETHOD Redraw(PRUint32 aCount, PRUint32 *_retval NS_OUTPARAM) = 0;

  /** Synthesize a mouse event for a window. The event types supported
   *  are: 
   *    mousedown, mouseup, mousemove, mouseover, mouseout, contextmenu
   *
   * Events are sent in coordinates offset by aX and aY from the window.
   *
   * Note that additional events may be fired as a result of this call. For
   * instance, typically a click event will be fired as a result of a
   * mousedown and mouseup in sequence.
   *
   * Normally at this level of events, the mouseover and mouseout events are
   * only fired when the window is entered or exited. For inter-element
   * mouseover and mouseout events, a movemove event fired on the new element
   * should be sufficient to generate the correct over and out events as well.
   *
   * Cannot be accessed from unprivileged context (not content-accessible)
   * Will throw a DOM security error if called without UniversalXPConnect
   * privileges.
   *
   * @param aType event type
   * @param aX x offset in CSS pixels
   * @param aY y offset in CSS pixels
   * @param aButton button to synthesize
   * @param aClickCount number of clicks that have been performed
   * @param aModifiers modifiers pressed, using constants defined in nsIDOMNSEvent
   * @param aIgnoreRootScrollFrame whether the event should ignore viewport bounds
   *                           during dispatch
   */
  /* void sendMouseEvent (in AString aType, in float aX, in float aY, in long aButton, in long aClickCount, in long aModifiers, [optional] in boolean aIgnoreRootScrollFrame); */
  NS_SCRIPTABLE NS_IMETHOD SendMouseEvent(const nsAString & aType, float aX, float aY, PRInt32 aButton, PRInt32 aClickCount, PRInt32 aModifiers, PRBool aIgnoreRootScrollFrame) = 0;

  /** Synthesize a mouse scroll event for a window. The event types supported
   *  are: 
   *    DOMMouseScroll
   *    MozMousePixelScroll
   *
   * Events are sent in coordinates offset by aX and aY from the window.
   *
   * Cannot be accessed from unprivileged context (not content-accessible)
   * Will throw a DOM security error if called without UniversalXPConnect
   * privileges.
   *
   * @param aType event type
   * @param aX x offset in CSS pixels
   * @param aY y offset in CSS pixels
   * @param aButton button to synthesize
   * @param aScrollFlags flag bits --- see nsMouseScrollFlags in nsGUIEvent.h
   * @param aDelta the direction and amount to scroll (in lines or pixels,
   * depending on the event type)
   * @param aModifiers modifiers pressed, using constants defined in nsIDOMNSEvent
   */
  /* void sendMouseScrollEvent (in AString aType, in float aX, in float aY, in long aButton, in long aScrollFlags, in long aDelta, in long aModifiers); */
  NS_SCRIPTABLE NS_IMETHOD SendMouseScrollEvent(const nsAString & aType, float aX, float aY, PRInt32 aButton, PRInt32 aScrollFlags, PRInt32 aDelta, PRInt32 aModifiers) = 0;

  /**
   * Synthesize a key event to the window. The event types supported are:
   *   keydown, keyup, keypress
   *
   * Key events generally end up being sent to the focused node.
   *
   * Cannot be accessed from unprivileged context (not content-accessible)
   * Will throw a DOM security error if called without UniversalXPConnect
   * privileges.
   *
   * @param aType event type
   * @param aKeyCode key code
   * @param aCharCode character code
   * @param aModifiers modifiers pressed, using constants defined in nsIDOMNSEvent
   * @param aPreventDefault if true, preventDefault() the event before dispatch
   *
   * @return false if the event had preventDefault() called on it,
   *               true otherwise.  In other words, true if and only if the
   *               default action was taken.
   */
  /* boolean sendKeyEvent (in AString aType, in long aKeyCode, in long aCharCode, in long aModifiers, [optional] in boolean aPreventDefault); */
  NS_SCRIPTABLE NS_IMETHOD SendKeyEvent(const nsAString & aType, PRInt32 aKeyCode, PRInt32 aCharCode, PRInt32 aModifiers, PRBool aPreventDefault, PRBool *_retval NS_OUTPARAM) = 0;

  /**
   * See nsIWidget::SynthesizeNativeKeyEvent
   *
   * Cannot be accessed from unprivileged context (not content-accessible)
   * Will throw a DOM security error if called without UniversalXPConnect
   * privileges.
   */
  /* void sendNativeKeyEvent (in long aNativeKeyboardLayout, in long aNativeKeyCode, in long aModifierFlags, in AString aCharacters, in AString aUnmodifiedCharacters); */
  NS_SCRIPTABLE NS_IMETHOD SendNativeKeyEvent(PRInt32 aNativeKeyboardLayout, PRInt32 aNativeKeyCode, PRInt32 aModifierFlags, const nsAString & aCharacters, const nsAString & aUnmodifiedCharacters) = 0;

  /**
   * See nsIWidget::ActivateNativeMenuItemAt
   *
   * Cannot be accessed from unprivileged context (not content-accessible)
   * Will throw a DOM security error if called without UniversalXPConnect
   * privileges.
   */
  /* void activateNativeMenuItemAt (in AString indexString); */
  NS_SCRIPTABLE NS_IMETHOD ActivateNativeMenuItemAt(const nsAString & indexString) = 0;

  /**
   * See nsIWidget::ForceUpdateNativeMenuAt
   *
   * Cannot be accessed from unprivileged context (not content-accessible)
   * Will throw a DOM security error if called without UniversalXPConnect
   * privileges.
   */
  /* void forceUpdateNativeMenuAt (in AString indexString); */
  NS_SCRIPTABLE NS_IMETHOD ForceUpdateNativeMenuAt(const nsAString & indexString) = 0;

  /**
   * Focus the element aElement. The element should be in the same document
   * that the window is displaying. Pass null to blur the element, if any,
   * that currently has focus, and focus the document.
   *
   * Cannot be accessed from unprivileged context (not content-accessible)
   * Will throw a DOM security error if called without UniversalXPConnect
   * privileges.
   *
   * @param aElement the element to focus
   *
   * Do not use this method. Just use element.focus if available or
   * nsIFocusManager::SetFocus instead.
   *
   */
  /* void focus (in nsIDOMElement aElement); */
  NS_SCRIPTABLE NS_IMETHOD Focus(nsIDOMElement *aElement) = 0;

  /**
   * Force a garbage collection. This will run the cycle-collector twice to
   * make sure all garbage is collected.
   *
   * Will throw a DOM security error if called without UniversalXPConnect
   * privileges in non-debug builds. Available to all callers in debug builds.
   */
  /* void garbageCollect (); */
  NS_SCRIPTABLE NS_IMETHOD GarbageCollect(void) = 0;

  /**
   * Force processing of any queued paints
   */
  /* void processUpdates (); */
  NS_SCRIPTABLE NS_IMETHOD ProcessUpdates(void) = 0;

  /** Synthesize a simple gesture event for a window. The event types
   *  supported are: MozSwipeGesture, MozMagnifyGestureStart,
   *  MozMagnifyGestureUpdate, MozMagnifyGesture, MozRotateGestureStart,
   *  MozRotateGestureUpdate, MozRotateGesture, MozPressTapGesture, and
   *  MozTapGesture.
   *
   * Cannot be accessed from unprivileged context (not
   * content-accessible) Will throw a DOM security error if called
   * without UniversalXPConnect privileges.
   *
   * @param aType event type
   * @param aX x offset in CSS pixels
   * @param aY y offset in CSS pixels
   * @param aDirection direction, using constants defined in nsIDOMSimpleGestureEvent
   * @param aDelta  amount of magnification or rotation for magnify and rotation events
   * @param aModifiers modifiers pressed, using constants defined in nsIDOMNSEvent
   */
  /* void sendSimpleGestureEvent (in AString aType, in float aX, in float aY, in unsigned long aDirection, in double aDelta, in long aModifiers); */
  NS_SCRIPTABLE NS_IMETHOD SendSimpleGestureEvent(const nsAString & aType, float aX, float aY, PRUint32 aDirection, double aDelta, PRInt32 aModifiers) = 0;

  /**
   * Retrieve the element at point aX, aY in the window's document.
   *
   * @param aIgnoreRootScrollFrame whether or not to ignore the root scroll
   *        frame when retrieving the element. If false, this method returns
   *        null for coordinates outside of the viewport.
   * @param aFlushLayout flushes layout if true. Otherwise, no flush occurs.
   */
  /* nsIDOMElement elementFromPoint (in long aX, in long aY, in boolean aIgnoreRootScrollFrame, in boolean aFlushLayout); */
  NS_SCRIPTABLE NS_IMETHOD ElementFromPoint(PRInt32 aX, PRInt32 aY, PRBool aIgnoreRootScrollFrame, PRBool aFlushLayout, nsIDOMElement **_retval NS_OUTPARAM) = 0;

  /**
   * Compare the two canvases, returning the number of differing pixels and
   * the maximum difference in a channel.  This will throw an error if
   * the dimensions of the two canvases are different.
   *
   * This method requires UniversalXPConnect privileges.
   */
  /* PRUint32 compareCanvases (in nsIDOMHTMLCanvasElement aCanvas1, in nsIDOMHTMLCanvasElement aCanvas2, out unsigned long aMaxDifference); */
  NS_SCRIPTABLE NS_IMETHOD CompareCanvases(nsIDOMHTMLCanvasElement *aCanvas1, nsIDOMHTMLCanvasElement *aCanvas2, PRUint32 *aMaxDifference NS_OUTPARAM, PRUint32 *_retval NS_OUTPARAM) = 0;

  /**
   * Returns true if a MozAfterPaint event has been queued but not yet
   * fired.
   */
  /* readonly attribute boolean isMozAfterPaintPending; */
  NS_SCRIPTABLE NS_IMETHOD GetIsMozAfterPaintPending(PRBool *aIsMozAfterPaintPending) = 0;

  /**
   * Suppresses/unsuppresses user initiated event handling in window's document
   * and subdocuments.
   *
   * @throw NS_ERROR_DOM_SECURITY_ERR if called without UniversalXPConnect
   *        privileges and NS_ERROR_FAILURE if window doesn't have a document.
   */
  /* void suppressEventHandling (in boolean aSuppress); */
  NS_SCRIPTABLE NS_IMETHOD SuppressEventHandling(PRBool aSuppress) = 0;

  /* void clearMozAfterPaintEvents (); */
  NS_SCRIPTABLE NS_IMETHOD ClearMozAfterPaintEvents(void) = 0;

  /**
   * Disable or enable non synthetic test mouse events on *all* windows.
   *
   * Cannot be accessed from unprivileged context (not content-accessible).
   * Will throw a DOM security error if called without UniversalXPConnect
   * privileges.
   *
   * @param aDisable  If true, disable all non synthetic test mouse events
   *               on all windows.  Otherwise, enable them.
   */
  /* void disableNonTestMouseEvents (in boolean aDisable); */
  NS_SCRIPTABLE NS_IMETHOD DisableNonTestMouseEvents(PRBool aDisable) = 0;

  /**
   * Returns the scroll position of the window's currently loaded document.
   *
   * @param aFlushLayout flushes layout if true. Otherwise, no flush occurs.
   * @see nsIDOMWindow::scrollX/Y
   */
  /* void getScrollXY (in boolean aFlushLayout, out long aScrollX, out long aScrollY); */
  NS_SCRIPTABLE NS_IMETHOD GetScrollXY(PRBool aFlushLayout, PRInt32 *aScrollX NS_OUTPARAM, PRInt32 *aScrollY NS_OUTPARAM) = 0;

  /**
   * Creates a ChromeObjectWrapper for the object and returns it.
   *
   * @param scope The JavaScript object whose scope we'll use as the
   *        parent of the wrapper.
   * @param objToWrap The JavaScript object to wrap.
   * @return the wrapped object.
   */
  /* void getCOWForObject (); */
  NS_SCRIPTABLE NS_IMETHOD GetCOWForObject(void) = 0;

  /**
   * Get IME open state. TRUE means 'Open', otherwise, 'Close'.
   * This property works only when IMEEnabled is IME_STATUS_ENABLED.
   */
  /* readonly attribute boolean IMEIsOpen; */
  NS_SCRIPTABLE NS_IMETHOD GetIMEIsOpen(PRBool *aIMEIsOpen) = 0;

  /**
   * WARNING: These values must be same as nsIWidget's values.
   */
/**
   * DISABLED means users cannot use IME completely.
   * Note that this state is *not* same as |ime-mode: disabled;|.
   */
  enum { IME_STATUS_DISABLED = 0U };

  /**
   * ENABLED means users can use all functions of IME. This state is same as
   * |ime-mode: normal;|.
   */
  enum { IME_STATUS_ENABLED = 1U };

  /**
   * PASSWORD means users cannot use most functions of IME. But on GTK2,
   * users can use "Simple IM" which only supports dead key inputting.
   * The behavior is same as the behavior of the native password field.
   * This state is same as |ime-mode: disabled;|.
   */
  enum { IME_STATUS_PASSWORD = 2U };

  /**
   * PLUGIN means a plug-in has focus. At this time we should not touch to
   * controlling the IME state.
   */
  enum { IME_STATUS_PLUGIN = 3U };

  /**
   * Get IME status, see above IME_STATUS_* definitions.
   */
  /* readonly attribute unsigned long IMEStatus; */
  NS_SCRIPTABLE NS_IMETHOD GetIMEStatus(PRUint32 *aIMEStatus) = 0;

  /**
   * Get the number of screen pixels per CSS pixel.
   */
  /* readonly attribute float screenPixelsPerCSSPixel; */
  NS_SCRIPTABLE NS_IMETHOD GetScreenPixelsPerCSSPixel(float *aScreenPixelsPerCSSPixel) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMWindowUtils, NS_IDOMWINDOWUTILS_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMWINDOWUTILS \
  NS_SCRIPTABLE NS_IMETHOD GetImageAnimationMode(PRUint16 *aImageAnimationMode); \
  NS_SCRIPTABLE NS_IMETHOD SetImageAnimationMode(PRUint16 aImageAnimationMode); \
  NS_SCRIPTABLE NS_IMETHOD GetDocCharsetIsForced(PRBool *aDocCharsetIsForced); \
  NS_SCRIPTABLE NS_IMETHOD GetDocumentMetadata(const nsAString & aName, nsAString & _retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD Redraw(PRUint32 aCount, PRUint32 *_retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD SendMouseEvent(const nsAString & aType, float aX, float aY, PRInt32 aButton, PRInt32 aClickCount, PRInt32 aModifiers, PRBool aIgnoreRootScrollFrame); \
  NS_SCRIPTABLE NS_IMETHOD SendMouseScrollEvent(const nsAString & aType, float aX, float aY, PRInt32 aButton, PRInt32 aScrollFlags, PRInt32 aDelta, PRInt32 aModifiers); \
  NS_SCRIPTABLE NS_IMETHOD SendKeyEvent(const nsAString & aType, PRInt32 aKeyCode, PRInt32 aCharCode, PRInt32 aModifiers, PRBool aPreventDefault, PRBool *_retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD SendNativeKeyEvent(PRInt32 aNativeKeyboardLayout, PRInt32 aNativeKeyCode, PRInt32 aModifierFlags, const nsAString & aCharacters, const nsAString & aUnmodifiedCharacters); \
  NS_SCRIPTABLE NS_IMETHOD ActivateNativeMenuItemAt(const nsAString & indexString); \
  NS_SCRIPTABLE NS_IMETHOD ForceUpdateNativeMenuAt(const nsAString & indexString); \
  NS_SCRIPTABLE NS_IMETHOD Focus(nsIDOMElement *aElement); \
  NS_SCRIPTABLE NS_IMETHOD GarbageCollect(void); \
  NS_SCRIPTABLE NS_IMETHOD ProcessUpdates(void); \
  NS_SCRIPTABLE NS_IMETHOD SendSimpleGestureEvent(const nsAString & aType, float aX, float aY, PRUint32 aDirection, double aDelta, PRInt32 aModifiers); \
  NS_SCRIPTABLE NS_IMETHOD ElementFromPoint(PRInt32 aX, PRInt32 aY, PRBool aIgnoreRootScrollFrame, PRBool aFlushLayout, nsIDOMElement **_retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD CompareCanvases(nsIDOMHTMLCanvasElement *aCanvas1, nsIDOMHTMLCanvasElement *aCanvas2, PRUint32 *aMaxDifference NS_OUTPARAM, PRUint32 *_retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD GetIsMozAfterPaintPending(PRBool *aIsMozAfterPaintPending); \
  NS_SCRIPTABLE NS_IMETHOD SuppressEventHandling(PRBool aSuppress); \
  NS_SCRIPTABLE NS_IMETHOD ClearMozAfterPaintEvents(void); \
  NS_SCRIPTABLE NS_IMETHOD DisableNonTestMouseEvents(PRBool aDisable); \
  NS_SCRIPTABLE NS_IMETHOD GetScrollXY(PRBool aFlushLayout, PRInt32 *aScrollX NS_OUTPARAM, PRInt32 *aScrollY NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD GetCOWForObject(void); \
  NS_SCRIPTABLE NS_IMETHOD GetIMEIsOpen(PRBool *aIMEIsOpen); \
  NS_SCRIPTABLE NS_IMETHOD GetIMEStatus(PRUint32 *aIMEStatus); \
  NS_SCRIPTABLE NS_IMETHOD GetScreenPixelsPerCSSPixel(float *aScreenPixelsPerCSSPixel); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMWINDOWUTILS(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetImageAnimationMode(PRUint16 *aImageAnimationMode) { return _to GetImageAnimationMode(aImageAnimationMode); } \
  NS_SCRIPTABLE NS_IMETHOD SetImageAnimationMode(PRUint16 aImageAnimationMode) { return _to SetImageAnimationMode(aImageAnimationMode); } \
  NS_SCRIPTABLE NS_IMETHOD GetDocCharsetIsForced(PRBool *aDocCharsetIsForced) { return _to GetDocCharsetIsForced(aDocCharsetIsForced); } \
  NS_SCRIPTABLE NS_IMETHOD GetDocumentMetadata(const nsAString & aName, nsAString & _retval NS_OUTPARAM) { return _to GetDocumentMetadata(aName, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD Redraw(PRUint32 aCount, PRUint32 *_retval NS_OUTPARAM) { return _to Redraw(aCount, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SendMouseEvent(const nsAString & aType, float aX, float aY, PRInt32 aButton, PRInt32 aClickCount, PRInt32 aModifiers, PRBool aIgnoreRootScrollFrame) { return _to SendMouseEvent(aType, aX, aY, aButton, aClickCount, aModifiers, aIgnoreRootScrollFrame); } \
  NS_SCRIPTABLE NS_IMETHOD SendMouseScrollEvent(const nsAString & aType, float aX, float aY, PRInt32 aButton, PRInt32 aScrollFlags, PRInt32 aDelta, PRInt32 aModifiers) { return _to SendMouseScrollEvent(aType, aX, aY, aButton, aScrollFlags, aDelta, aModifiers); } \
  NS_SCRIPTABLE NS_IMETHOD SendKeyEvent(const nsAString & aType, PRInt32 aKeyCode, PRInt32 aCharCode, PRInt32 aModifiers, PRBool aPreventDefault, PRBool *_retval NS_OUTPARAM) { return _to SendKeyEvent(aType, aKeyCode, aCharCode, aModifiers, aPreventDefault, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SendNativeKeyEvent(PRInt32 aNativeKeyboardLayout, PRInt32 aNativeKeyCode, PRInt32 aModifierFlags, const nsAString & aCharacters, const nsAString & aUnmodifiedCharacters) { return _to SendNativeKeyEvent(aNativeKeyboardLayout, aNativeKeyCode, aModifierFlags, aCharacters, aUnmodifiedCharacters); } \
  NS_SCRIPTABLE NS_IMETHOD ActivateNativeMenuItemAt(const nsAString & indexString) { return _to ActivateNativeMenuItemAt(indexString); } \
  NS_SCRIPTABLE NS_IMETHOD ForceUpdateNativeMenuAt(const nsAString & indexString) { return _to ForceUpdateNativeMenuAt(indexString); } \
  NS_SCRIPTABLE NS_IMETHOD Focus(nsIDOMElement *aElement) { return _to Focus(aElement); } \
  NS_SCRIPTABLE NS_IMETHOD GarbageCollect(void) { return _to GarbageCollect(); } \
  NS_SCRIPTABLE NS_IMETHOD ProcessUpdates(void) { return _to ProcessUpdates(); } \
  NS_SCRIPTABLE NS_IMETHOD SendSimpleGestureEvent(const nsAString & aType, float aX, float aY, PRUint32 aDirection, double aDelta, PRInt32 aModifiers) { return _to SendSimpleGestureEvent(aType, aX, aY, aDirection, aDelta, aModifiers); } \
  NS_SCRIPTABLE NS_IMETHOD ElementFromPoint(PRInt32 aX, PRInt32 aY, PRBool aIgnoreRootScrollFrame, PRBool aFlushLayout, nsIDOMElement **_retval NS_OUTPARAM) { return _to ElementFromPoint(aX, aY, aIgnoreRootScrollFrame, aFlushLayout, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD CompareCanvases(nsIDOMHTMLCanvasElement *aCanvas1, nsIDOMHTMLCanvasElement *aCanvas2, PRUint32 *aMaxDifference NS_OUTPARAM, PRUint32 *_retval NS_OUTPARAM) { return _to CompareCanvases(aCanvas1, aCanvas2, aMaxDifference, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD GetIsMozAfterPaintPending(PRBool *aIsMozAfterPaintPending) { return _to GetIsMozAfterPaintPending(aIsMozAfterPaintPending); } \
  NS_SCRIPTABLE NS_IMETHOD SuppressEventHandling(PRBool aSuppress) { return _to SuppressEventHandling(aSuppress); } \
  NS_SCRIPTABLE NS_IMETHOD ClearMozAfterPaintEvents(void) { return _to ClearMozAfterPaintEvents(); } \
  NS_SCRIPTABLE NS_IMETHOD DisableNonTestMouseEvents(PRBool aDisable) { return _to DisableNonTestMouseEvents(aDisable); } \
  NS_SCRIPTABLE NS_IMETHOD GetScrollXY(PRBool aFlushLayout, PRInt32 *aScrollX NS_OUTPARAM, PRInt32 *aScrollY NS_OUTPARAM) { return _to GetScrollXY(aFlushLayout, aScrollX, aScrollY); } \
  NS_SCRIPTABLE NS_IMETHOD GetCOWForObject(void) { return _to GetCOWForObject(); } \
  NS_SCRIPTABLE NS_IMETHOD GetIMEIsOpen(PRBool *aIMEIsOpen) { return _to GetIMEIsOpen(aIMEIsOpen); } \
  NS_SCRIPTABLE NS_IMETHOD GetIMEStatus(PRUint32 *aIMEStatus) { return _to GetIMEStatus(aIMEStatus); } \
  NS_SCRIPTABLE NS_IMETHOD GetScreenPixelsPerCSSPixel(float *aScreenPixelsPerCSSPixel) { return _to GetScreenPixelsPerCSSPixel(aScreenPixelsPerCSSPixel); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMWINDOWUTILS(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetImageAnimationMode(PRUint16 *aImageAnimationMode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetImageAnimationMode(aImageAnimationMode); } \
  NS_SCRIPTABLE NS_IMETHOD SetImageAnimationMode(PRUint16 aImageAnimationMode) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetImageAnimationMode(aImageAnimationMode); } \
  NS_SCRIPTABLE NS_IMETHOD GetDocCharsetIsForced(PRBool *aDocCharsetIsForced) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDocCharsetIsForced(aDocCharsetIsForced); } \
  NS_SCRIPTABLE NS_IMETHOD GetDocumentMetadata(const nsAString & aName, nsAString & _retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetDocumentMetadata(aName, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD Redraw(PRUint32 aCount, PRUint32 *_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->Redraw(aCount, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SendMouseEvent(const nsAString & aType, float aX, float aY, PRInt32 aButton, PRInt32 aClickCount, PRInt32 aModifiers, PRBool aIgnoreRootScrollFrame) { return !_to ? NS_ERROR_NULL_POINTER : _to->SendMouseEvent(aType, aX, aY, aButton, aClickCount, aModifiers, aIgnoreRootScrollFrame); } \
  NS_SCRIPTABLE NS_IMETHOD SendMouseScrollEvent(const nsAString & aType, float aX, float aY, PRInt32 aButton, PRInt32 aScrollFlags, PRInt32 aDelta, PRInt32 aModifiers) { return !_to ? NS_ERROR_NULL_POINTER : _to->SendMouseScrollEvent(aType, aX, aY, aButton, aScrollFlags, aDelta, aModifiers); } \
  NS_SCRIPTABLE NS_IMETHOD SendKeyEvent(const nsAString & aType, PRInt32 aKeyCode, PRInt32 aCharCode, PRInt32 aModifiers, PRBool aPreventDefault, PRBool *_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->SendKeyEvent(aType, aKeyCode, aCharCode, aModifiers, aPreventDefault, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SendNativeKeyEvent(PRInt32 aNativeKeyboardLayout, PRInt32 aNativeKeyCode, PRInt32 aModifierFlags, const nsAString & aCharacters, const nsAString & aUnmodifiedCharacters) { return !_to ? NS_ERROR_NULL_POINTER : _to->SendNativeKeyEvent(aNativeKeyboardLayout, aNativeKeyCode, aModifierFlags, aCharacters, aUnmodifiedCharacters); } \
  NS_SCRIPTABLE NS_IMETHOD ActivateNativeMenuItemAt(const nsAString & indexString) { return !_to ? NS_ERROR_NULL_POINTER : _to->ActivateNativeMenuItemAt(indexString); } \
  NS_SCRIPTABLE NS_IMETHOD ForceUpdateNativeMenuAt(const nsAString & indexString) { return !_to ? NS_ERROR_NULL_POINTER : _to->ForceUpdateNativeMenuAt(indexString); } \
  NS_SCRIPTABLE NS_IMETHOD Focus(nsIDOMElement *aElement) { return !_to ? NS_ERROR_NULL_POINTER : _to->Focus(aElement); } \
  NS_SCRIPTABLE NS_IMETHOD GarbageCollect(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->GarbageCollect(); } \
  NS_SCRIPTABLE NS_IMETHOD ProcessUpdates(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ProcessUpdates(); } \
  NS_SCRIPTABLE NS_IMETHOD SendSimpleGestureEvent(const nsAString & aType, float aX, float aY, PRUint32 aDirection, double aDelta, PRInt32 aModifiers) { return !_to ? NS_ERROR_NULL_POINTER : _to->SendSimpleGestureEvent(aType, aX, aY, aDirection, aDelta, aModifiers); } \
  NS_SCRIPTABLE NS_IMETHOD ElementFromPoint(PRInt32 aX, PRInt32 aY, PRBool aIgnoreRootScrollFrame, PRBool aFlushLayout, nsIDOMElement **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->ElementFromPoint(aX, aY, aIgnoreRootScrollFrame, aFlushLayout, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD CompareCanvases(nsIDOMHTMLCanvasElement *aCanvas1, nsIDOMHTMLCanvasElement *aCanvas2, PRUint32 *aMaxDifference NS_OUTPARAM, PRUint32 *_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CompareCanvases(aCanvas1, aCanvas2, aMaxDifference, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD GetIsMozAfterPaintPending(PRBool *aIsMozAfterPaintPending) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIsMozAfterPaintPending(aIsMozAfterPaintPending); } \
  NS_SCRIPTABLE NS_IMETHOD SuppressEventHandling(PRBool aSuppress) { return !_to ? NS_ERROR_NULL_POINTER : _to->SuppressEventHandling(aSuppress); } \
  NS_SCRIPTABLE NS_IMETHOD ClearMozAfterPaintEvents(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ClearMozAfterPaintEvents(); } \
  NS_SCRIPTABLE NS_IMETHOD DisableNonTestMouseEvents(PRBool aDisable) { return !_to ? NS_ERROR_NULL_POINTER : _to->DisableNonTestMouseEvents(aDisable); } \
  NS_SCRIPTABLE NS_IMETHOD GetScrollXY(PRBool aFlushLayout, PRInt32 *aScrollX NS_OUTPARAM, PRInt32 *aScrollY NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScrollXY(aFlushLayout, aScrollX, aScrollY); } \
  NS_SCRIPTABLE NS_IMETHOD GetCOWForObject(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCOWForObject(); } \
  NS_SCRIPTABLE NS_IMETHOD GetIMEIsOpen(PRBool *aIMEIsOpen) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIMEIsOpen(aIMEIsOpen); } \
  NS_SCRIPTABLE NS_IMETHOD GetIMEStatus(PRUint32 *aIMEStatus) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIMEStatus(aIMEStatus); } \
  NS_SCRIPTABLE NS_IMETHOD GetScreenPixelsPerCSSPixel(float *aScreenPixelsPerCSSPixel) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetScreenPixelsPerCSSPixel(aScreenPixelsPerCSSPixel); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMWindowUtils : public nsIDOMWindowUtils
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMWINDOWUTILS

  nsDOMWindowUtils();

private:
  ~nsDOMWindowUtils();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMWindowUtils, nsIDOMWindowUtils)

nsDOMWindowUtils::nsDOMWindowUtils()
{
  /* member initializers and constructor code */
}

nsDOMWindowUtils::~nsDOMWindowUtils()
{
  /* destructor code */
}

/* attribute unsigned short imageAnimationMode; */
NS_IMETHODIMP nsDOMWindowUtils::GetImageAnimationMode(PRUint16 *aImageAnimationMode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMWindowUtils::SetImageAnimationMode(PRUint16 aImageAnimationMode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean docCharsetIsForced; */
NS_IMETHODIMP nsDOMWindowUtils::GetDocCharsetIsForced(PRBool *aDocCharsetIsForced)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* AString getDocumentMetadata (in AString aName); */
NS_IMETHODIMP nsDOMWindowUtils::GetDocumentMetadata(const nsAString & aName, nsAString & _retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* unsigned long redraw ([optional] in unsigned long aCount); */
NS_IMETHODIMP nsDOMWindowUtils::Redraw(PRUint32 aCount, PRUint32 *_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void sendMouseEvent (in AString aType, in float aX, in float aY, in long aButton, in long aClickCount, in long aModifiers, [optional] in boolean aIgnoreRootScrollFrame); */
NS_IMETHODIMP nsDOMWindowUtils::SendMouseEvent(const nsAString & aType, float aX, float aY, PRInt32 aButton, PRInt32 aClickCount, PRInt32 aModifiers, PRBool aIgnoreRootScrollFrame)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void sendMouseScrollEvent (in AString aType, in float aX, in float aY, in long aButton, in long aScrollFlags, in long aDelta, in long aModifiers); */
NS_IMETHODIMP nsDOMWindowUtils::SendMouseScrollEvent(const nsAString & aType, float aX, float aY, PRInt32 aButton, PRInt32 aScrollFlags, PRInt32 aDelta, PRInt32 aModifiers)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean sendKeyEvent (in AString aType, in long aKeyCode, in long aCharCode, in long aModifiers, [optional] in boolean aPreventDefault); */
NS_IMETHODIMP nsDOMWindowUtils::SendKeyEvent(const nsAString & aType, PRInt32 aKeyCode, PRInt32 aCharCode, PRInt32 aModifiers, PRBool aPreventDefault, PRBool *_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void sendNativeKeyEvent (in long aNativeKeyboardLayout, in long aNativeKeyCode, in long aModifierFlags, in AString aCharacters, in AString aUnmodifiedCharacters); */
NS_IMETHODIMP nsDOMWindowUtils::SendNativeKeyEvent(PRInt32 aNativeKeyboardLayout, PRInt32 aNativeKeyCode, PRInt32 aModifierFlags, const nsAString & aCharacters, const nsAString & aUnmodifiedCharacters)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void activateNativeMenuItemAt (in AString indexString); */
NS_IMETHODIMP nsDOMWindowUtils::ActivateNativeMenuItemAt(const nsAString & indexString)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void forceUpdateNativeMenuAt (in AString indexString); */
NS_IMETHODIMP nsDOMWindowUtils::ForceUpdateNativeMenuAt(const nsAString & indexString)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void focus (in nsIDOMElement aElement); */
NS_IMETHODIMP nsDOMWindowUtils::Focus(nsIDOMElement *aElement)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void garbageCollect (); */
NS_IMETHODIMP nsDOMWindowUtils::GarbageCollect()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void processUpdates (); */
NS_IMETHODIMP nsDOMWindowUtils::ProcessUpdates()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void sendSimpleGestureEvent (in AString aType, in float aX, in float aY, in unsigned long aDirection, in double aDelta, in long aModifiers); */
NS_IMETHODIMP nsDOMWindowUtils::SendSimpleGestureEvent(const nsAString & aType, float aX, float aY, PRUint32 aDirection, double aDelta, PRInt32 aModifiers)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMElement elementFromPoint (in long aX, in long aY, in boolean aIgnoreRootScrollFrame, in boolean aFlushLayout); */
NS_IMETHODIMP nsDOMWindowUtils::ElementFromPoint(PRInt32 aX, PRInt32 aY, PRBool aIgnoreRootScrollFrame, PRBool aFlushLayout, nsIDOMElement **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* PRUint32 compareCanvases (in nsIDOMHTMLCanvasElement aCanvas1, in nsIDOMHTMLCanvasElement aCanvas2, out unsigned long aMaxDifference); */
NS_IMETHODIMP nsDOMWindowUtils::CompareCanvases(nsIDOMHTMLCanvasElement *aCanvas1, nsIDOMHTMLCanvasElement *aCanvas2, PRUint32 *aMaxDifference NS_OUTPARAM, PRUint32 *_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean isMozAfterPaintPending; */
NS_IMETHODIMP nsDOMWindowUtils::GetIsMozAfterPaintPending(PRBool *aIsMozAfterPaintPending)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void suppressEventHandling (in boolean aSuppress); */
NS_IMETHODIMP nsDOMWindowUtils::SuppressEventHandling(PRBool aSuppress)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void clearMozAfterPaintEvents (); */
NS_IMETHODIMP nsDOMWindowUtils::ClearMozAfterPaintEvents()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void disableNonTestMouseEvents (in boolean aDisable); */
NS_IMETHODIMP nsDOMWindowUtils::DisableNonTestMouseEvents(PRBool aDisable)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getScrollXY (in boolean aFlushLayout, out long aScrollX, out long aScrollY); */
NS_IMETHODIMP nsDOMWindowUtils::GetScrollXY(PRBool aFlushLayout, PRInt32 *aScrollX NS_OUTPARAM, PRInt32 *aScrollY NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getCOWForObject (); */
NS_IMETHODIMP nsDOMWindowUtils::GetCOWForObject()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean IMEIsOpen; */
NS_IMETHODIMP nsDOMWindowUtils::GetIMEIsOpen(PRBool *aIMEIsOpen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long IMEStatus; */
NS_IMETHODIMP nsDOMWindowUtils::GetIMEStatus(PRUint32 *aIMEStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute float screenPixelsPerCSSPixel; */
NS_IMETHODIMP nsDOMWindowUtils::GetScreenPixelsPerCSSPixel(float *aScreenPixelsPerCSSPixel)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMWindowUtils_1_9_2 */
#define NS_IDOMWINDOWUTILS_1_9_2_IID_STR "b0f803f7-98c0-4152-812c-d6678ba23049"

#define NS_IDOMWINDOWUTILS_1_9_2_IID \
  {0xb0f803f7, 0x98c0, 0x4152, \
    { 0x81, 0x2c, 0xd6, 0x67, 0x8b, 0xa2, 0x30, 0x49 }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIDOMWindowUtils_1_9_2 : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMWINDOWUTILS_1_9_2_IID)

  /**
   * Dispatches aEvent via the nsIPresShell object of the window's document.
   * The event is dispatched to aTarget, which should be an object
   * which implements nsIContent interface (#element, #text, etc).
   *
   * Cannot be accessed from unprivileged context (not
   * content-accessible) Will throw a DOM security error if called
   * without UniversalXPConnect privileges.
   *
   * @note Event handlers won't get aEvent as parameter, but a similar event.
   *       Also, aEvent should not be reused.
   */
  /* boolean dispatchDOMEventViaPresShell (in nsIDOMNode aTarget, in nsIDOMEvent aEvent, in boolean aTrusted); */
  NS_SCRIPTABLE NS_IMETHOD DispatchDOMEventViaPresShell(nsIDOMNode *aTarget, nsIDOMEvent *aEvent, PRBool aTrusted, PRBool *_retval NS_OUTPARAM) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMWindowUtils_1_9_2, NS_IDOMWINDOWUTILS_1_9_2_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMWINDOWUTILS_1_9_2 \
  NS_SCRIPTABLE NS_IMETHOD DispatchDOMEventViaPresShell(nsIDOMNode *aTarget, nsIDOMEvent *aEvent, PRBool aTrusted, PRBool *_retval NS_OUTPARAM); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMWINDOWUTILS_1_9_2(_to) \
  NS_SCRIPTABLE NS_IMETHOD DispatchDOMEventViaPresShell(nsIDOMNode *aTarget, nsIDOMEvent *aEvent, PRBool aTrusted, PRBool *_retval NS_OUTPARAM) { return _to DispatchDOMEventViaPresShell(aTarget, aEvent, aTrusted, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMWINDOWUTILS_1_9_2(_to) \
  NS_SCRIPTABLE NS_IMETHOD DispatchDOMEventViaPresShell(nsIDOMNode *aTarget, nsIDOMEvent *aEvent, PRBool aTrusted, PRBool *_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->DispatchDOMEventViaPresShell(aTarget, aEvent, aTrusted, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMWindowUtils_1_9_2 : public nsIDOMWindowUtils_1_9_2
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMWINDOWUTILS_1_9_2

  nsDOMWindowUtils_1_9_2();

private:
  ~nsDOMWindowUtils_1_9_2();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMWindowUtils_1_9_2, nsIDOMWindowUtils_1_9_2)

nsDOMWindowUtils_1_9_2::nsDOMWindowUtils_1_9_2()
{
  /* member initializers and constructor code */
}

nsDOMWindowUtils_1_9_2::~nsDOMWindowUtils_1_9_2()
{
  /* destructor code */
}

/* boolean dispatchDOMEventViaPresShell (in nsIDOMNode aTarget, in nsIDOMEvent aEvent, in boolean aTrusted); */
NS_IMETHODIMP nsDOMWindowUtils_1_9_2::DispatchDOMEventViaPresShell(nsIDOMNode *aTarget, nsIDOMEvent *aEvent, PRBool aTrusted, PRBool *_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMWindowUtils_1_9_2_5 */
#define NS_IDOMWINDOWUTILS_1_9_2_5_IID_STR "915abb48-66d4-4135-a0d8-153fb87b99e6"

#define NS_IDOMWINDOWUTILS_1_9_2_5_IID \
  {0x915abb48, 0x66d4, 0x4135, \
    { 0xa0, 0xd8, 0x15, 0x3f, 0xb8, 0x7b, 0x99, 0xe6 }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIDOMWindowUtils_1_9_2_5 : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMWINDOWUTILS_1_9_2_5_IID)

  /**
   * Retrieve all nodes that intersect a rect in the window's document.
   *
   * @param aX x reference for the rectangle in CSS pixels
   * @param aY y reference for the rectangle in CSS pixels
   * @param aTopSize How much to expand up the rectangle
   * @param aRightSize How much to expand right the rectangle
   * @param aBottomSize How much to expand down the rectangle
   * @param aLeftSize How much to expand left the rectangle
   * @param aIgnoreRootScrollFrame whether or not to ignore the root scroll
   *        frame when retrieving the element. If false, this method returns
   *        null for coordinates outside of the viewport.
   * @param aFlushLayout flushes layout if true. Otherwise, no flush occurs.
   */
  /* nsIDOMNodeList nodesFromRect (in float aX, in float aY, in float aTopSize, in float aRightSize, in float aBottomSize, in float aLeftSize, in boolean aIgnoreRootScrollFrame, in boolean aFlushLayout); */
  NS_SCRIPTABLE NS_IMETHOD NodesFromRect(float aX, float aY, float aTopSize, float aRightSize, float aBottomSize, float aLeftSize, PRBool aIgnoreRootScrollFrame, PRBool aFlushLayout, nsIDOMNodeList **_retval NS_OUTPARAM) = 0;

  /**
   * Synthesize a query content event.
   *
   * @param aType  On of the following const values.  And see also each comment
   *               for the other parameters and the result.
   */
  /* nsIQueryContentEventResult sendQueryContentEvent (in unsigned long aType, in unsigned long aOffset, in unsigned long aLength, in long aX, in long aY); */
  NS_SCRIPTABLE NS_IMETHOD SendQueryContentEvent(PRUint32 aType, PRUint32 aOffset, PRUint32 aLength, PRInt32 aX, PRInt32 aY, nsIQueryContentEventResult **_retval NS_OUTPARAM) = 0;

  /**
   * QUERY_SELECTED_TEXT queries the first selection range's information.
   *
   * @param aOffset   Not used.
   * @param aLength   Not used.
   * @param aX        Not used.
   * @param aY        Not used.
   *
   * @return offset, reversed and text properties of the result are available.
   */
  enum { QUERY_SELECTED_TEXT = 3200U };

  /**
   * QUERY_TEXT_CONTENT queries the text at the specified range.
   *
   * @param aOffset   The first character's offset.  0 is the first character.
   * @param aLength   The length of getting text.  If the aLength is too long,
   *                  the result text is shorter than this value.
   * @param aX        Not used.
   * @param aY        Not used.
   *
   * @return text property of the result is available.
   */
  enum { QUERY_TEXT_CONTENT = 3201U };

  /**
   * QUERY_CARET_RECT queries the (collapsed) caret rect of the offset.
   * If the actual caret is there at the specified offset, this returns the
   * actual caret rect.  Otherwise, this guesses the caret rect from the
   * metrics of the text.
   *
   * @param aOffset   The caret offset.  0 is the left side of the first
   *                  caracter in LTR text.
   * @param aLength   Not used.
   * @param aX        Not used.
   * @param aY        Not used.
   *
   * @return left, top, width and height properties of the result are available.
   *         The left and the top properties are offset in the client area of
   *         the DOM window.
   */
  enum { QUERY_CARET_RECT = 3203U };

  /**
   * QUERY_TEXT_RECT queries the specified text's rect.
   *
   * @param aOffset   The first character's offset.  0 is the first character.
   * @param aLength   The length of getting text.  If the aLength is too long,
   *                  the extra length is ignored.
   * @param aX        Not used.
   * @param aY        Not used.
   *
   * @return left, top, width and height properties of the result are available.
   *         The left and the top properties are offset in the client area of
   *         the DOM window.
   */
  enum { QUERY_TEXT_RECT = 3204U };

  /**
   * QUERY_TEXT_RECT queries the focused editor's rect.
   *
   * @param aOffset   Not used.
   * @param aLength   Not used.
   * @param aX        Not used.
   * @param aY        Not used.
   *
   * @return left, top, width and height properties of the result are available.
   */
  enum { QUERY_EDITOR_RECT = 3205U };

  /**
   * QUERY_CHARACTER_AT_POINT queries the character information at the
   * specified point.  The point is offset in the window.
   * NOTE: If there are some panels at the point, this method send the query
   * event to the panel's widget automatically.
   *
   * @param aOffset   Not used.
   * @param aLength   Not used.
   * @param aX        X offset in the widget.
   * @param aY        Y offset in the widget.
   *
   * @return offset, notFound, left, top, width and height properties of the
   *         result are available.
   */
  enum { QUERY_CHARACTER_AT_POINT = 3208U };

  /**
   * Exposes the CSS parser's "initial syntax is valid" heuristic for
   * testing.
   */
  /* boolean cssInitialSyntaxIsValid (in AString aSheet); */
  NS_SCRIPTABLE NS_IMETHOD CssInitialSyntaxIsValid(const nsAString & aSheet, PRBool *_retval NS_OUTPARAM) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMWindowUtils_1_9_2_5, NS_IDOMWINDOWUTILS_1_9_2_5_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMWINDOWUTILS_1_9_2_5 \
  NS_SCRIPTABLE NS_IMETHOD NodesFromRect(float aX, float aY, float aTopSize, float aRightSize, float aBottomSize, float aLeftSize, PRBool aIgnoreRootScrollFrame, PRBool aFlushLayout, nsIDOMNodeList **_retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD SendQueryContentEvent(PRUint32 aType, PRUint32 aOffset, PRUint32 aLength, PRInt32 aX, PRInt32 aY, nsIQueryContentEventResult **_retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD CssInitialSyntaxIsValid(const nsAString & aSheet, PRBool *_retval NS_OUTPARAM); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMWINDOWUTILS_1_9_2_5(_to) \
  NS_SCRIPTABLE NS_IMETHOD NodesFromRect(float aX, float aY, float aTopSize, float aRightSize, float aBottomSize, float aLeftSize, PRBool aIgnoreRootScrollFrame, PRBool aFlushLayout, nsIDOMNodeList **_retval NS_OUTPARAM) { return _to NodesFromRect(aX, aY, aTopSize, aRightSize, aBottomSize, aLeftSize, aIgnoreRootScrollFrame, aFlushLayout, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SendQueryContentEvent(PRUint32 aType, PRUint32 aOffset, PRUint32 aLength, PRInt32 aX, PRInt32 aY, nsIQueryContentEventResult **_retval NS_OUTPARAM) { return _to SendQueryContentEvent(aType, aOffset, aLength, aX, aY, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD CssInitialSyntaxIsValid(const nsAString & aSheet, PRBool *_retval NS_OUTPARAM) { return _to CssInitialSyntaxIsValid(aSheet, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMWINDOWUTILS_1_9_2_5(_to) \
  NS_SCRIPTABLE NS_IMETHOD NodesFromRect(float aX, float aY, float aTopSize, float aRightSize, float aBottomSize, float aLeftSize, PRBool aIgnoreRootScrollFrame, PRBool aFlushLayout, nsIDOMNodeList **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->NodesFromRect(aX, aY, aTopSize, aRightSize, aBottomSize, aLeftSize, aIgnoreRootScrollFrame, aFlushLayout, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SendQueryContentEvent(PRUint32 aType, PRUint32 aOffset, PRUint32 aLength, PRInt32 aX, PRInt32 aY, nsIQueryContentEventResult **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->SendQueryContentEvent(aType, aOffset, aLength, aX, aY, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD CssInitialSyntaxIsValid(const nsAString & aSheet, PRBool *_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CssInitialSyntaxIsValid(aSheet, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMWindowUtils_1_9_2_5 : public nsIDOMWindowUtils_1_9_2_5
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMWINDOWUTILS_1_9_2_5

  nsDOMWindowUtils_1_9_2_5();

private:
  ~nsDOMWindowUtils_1_9_2_5();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMWindowUtils_1_9_2_5, nsIDOMWindowUtils_1_9_2_5)

nsDOMWindowUtils_1_9_2_5::nsDOMWindowUtils_1_9_2_5()
{
  /* member initializers and constructor code */
}

nsDOMWindowUtils_1_9_2_5::~nsDOMWindowUtils_1_9_2_5()
{
  /* destructor code */
}

/* nsIDOMNodeList nodesFromRect (in float aX, in float aY, in float aTopSize, in float aRightSize, in float aBottomSize, in float aLeftSize, in boolean aIgnoreRootScrollFrame, in boolean aFlushLayout); */
NS_IMETHODIMP nsDOMWindowUtils_1_9_2_5::NodesFromRect(float aX, float aY, float aTopSize, float aRightSize, float aBottomSize, float aLeftSize, PRBool aIgnoreRootScrollFrame, PRBool aFlushLayout, nsIDOMNodeList **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIQueryContentEventResult sendQueryContentEvent (in unsigned long aType, in unsigned long aOffset, in unsigned long aLength, in long aX, in long aY); */
NS_IMETHODIMP nsDOMWindowUtils_1_9_2_5::SendQueryContentEvent(PRUint32 aType, PRUint32 aOffset, PRUint32 aLength, PRInt32 aX, PRInt32 aY, nsIQueryContentEventResult **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean cssInitialSyntaxIsValid (in AString aSheet); */
NS_IMETHODIMP nsDOMWindowUtils_1_9_2_5::CssInitialSyntaxIsValid(const nsAString & aSheet, PRBool *_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMWindowUtils_h__ */
