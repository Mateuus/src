/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/accessible/public/nsIAccessibilityService.idl
 */

#ifndef __gen_nsIAccessibilityService_h__
#define __gen_nsIAccessibilityService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIAccessibleRetrieval_h__
#include "nsIAccessibleRetrieval.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIAccessibleEventListener; /* forward declaration */

class nsIDocument; /* forward declaration */

class nsIFrame; /* forward declaration */

class nsObjectFrame; /* forward declaration */

class nsIContent; /* forward declaration */

class nsITimer; /* forward declaration */


/* starting interface:    nsIAccessibilityService */
#define NS_IACCESSIBILITYSERVICE_IID_STR "6a58f7e8-587c-40dd-b684-dc3e54f1342a"

#define NS_IACCESSIBILITYSERVICE_IID \
  {0x6a58f7e8, 0x587c, 0x40dd, \
    { 0xb6, 0x84, 0xdc, 0x3e, 0x54, 0xf1, 0x34, 0x2a }}

class NS_NO_VTABLE nsIAccessibilityService : public nsIAccessibleRetrieval {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IACCESSIBILITYSERVICE_IID)

  /* nsIAccessible createOuterDocAccessible (in nsIDOMNode aNode); */
  NS_IMETHOD CreateOuterDocAccessible(nsIDOMNode *aNode, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createRootAccessible (in nsIPresShell aShell, in nsIDocument aDocument); */
  NS_IMETHOD CreateRootAccessible(nsIPresShell *aShell, nsIDocument *aDocument, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTML4ButtonAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTML4ButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHyperTextAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHyperTextAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLBRAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLBRAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLButtonAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLAccessibleByMarkup (in nsIFrame aFrame, in nsIWeakReference aWeakShell, in nsIDOMNode aDOMNode); */
  NS_IMETHOD CreateHTMLAccessibleByMarkup(nsIFrame *aFrame, nsIWeakReference *aWeakShell, nsIDOMNode *aDOMNode, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLLIAccessible (in nsIFrame aFrame, in nsIFrame aBulletFrame, in AString aBulletText); */
  NS_IMETHOD CreateHTMLLIAccessible(nsIFrame *aFrame, nsIFrame *aBulletFrame, const nsAString & aBulletText, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLCheckboxAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLCheckboxAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLComboboxAccessible (in nsIDOMNode aNode, in nsIWeakReference aPresShell); */
  NS_IMETHOD CreateHTMLComboboxAccessible(nsIDOMNode *aNode, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLGenericAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLGenericAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLGroupboxAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLGroupboxAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLHRAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLHRAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLImageAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLImageAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLLabelAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLLabelAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLListboxAccessible (in nsIDOMNode aNode, in nsIWeakReference aPresShell); */
  NS_IMETHOD CreateHTMLListboxAccessible(nsIDOMNode *aNode, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLMediaAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLMediaAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLObjectFrameAccessible (in nsObjectFrame aFrame); */
  NS_IMETHOD CreateHTMLObjectFrameAccessible(nsObjectFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLRadioButtonAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLRadioButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLSelectOptionAccessible (in nsIDOMNode aNode, in nsIAccessible aAccParent, in nsIWeakReference aPresShell); */
  NS_IMETHOD CreateHTMLSelectOptionAccessible(nsIDOMNode *aNode, nsIAccessible *aAccParent, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLTableAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLTableAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLTableCellAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLTableCellAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLTextAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLTextAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLTextFieldAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLTextFieldAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible createHTMLCaptionAccessible (in nsIFrame aFrame); */
  NS_IMETHOD CreateHTMLCaptionAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible getAccessible (in nsIDOMNode aNode, in nsIPresShell aPresShell, in nsIWeakReference aWeakShell, inout nsIFrame frameHint, out boolean aIsHidden); */
  NS_IMETHOD GetAccessible(nsIDOMNode *aNode, nsIPresShell *aPresShell, nsIWeakReference *aWeakShell, nsIFrame **frameHint NS_INOUTPARAM, PRBool *aIsHidden NS_OUTPARAM, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* nsIAccessible addNativeRootAccessible (in voidPtr aAtkAccessible); */
  NS_IMETHOD AddNativeRootAccessible(void * aAtkAccessible, nsIAccessible **_retval NS_OUTPARAM) = 0;

  /* void removeNativeRootAccessible (in nsIAccessible aRootAccessible); */
  NS_IMETHOD RemoveNativeRootAccessible(nsIAccessible *aRootAccessible) = 0;

  /**
   * Invalidate the accessibility cache associated with aPresShell, for accessibles
   * that were generated for aContainerContent and its subtree.
   * The container content node for the change is passed in, rather than the
   * changed presentation for the content node itself.
   * @param aPresShell         The presShell where changes occured
   * @param aChangeContent     The affected DOM content
   * @param aEvent             The event from nsIAccessibleEvent that
   *                           caused the change:
   *                           Must be one of:
   *                           EVENT_REORDER (change),
   *                           EVENT_SHOW (make visible or create) or 
   *                           EVENT_HIDE (destroy or hide)
   */
  /* void invalidateSubtreeFor (in nsIPresShell aPresShell, in nsIContent aChangedContent, in PRUint32 aEvent); */
  NS_IMETHOD InvalidateSubtreeFor(nsIPresShell *aPresShell, nsIContent *aChangedContent, PRUint32 aEvent) = 0;

  /**
   *  An internal doc load event has occured. Handle the event and remove it from the list.
   *  @param aTimer      The timer created to handle this doc load event
   *  @param aClosure    The nsIWebProgress* for the load event
   *  @param aEventType  The type of load event, one of: nsIAccessibleEvent::EVENT_DOCUMENT_LOAD_START,
   *                                                     nsIAccessibleEvent::EVENT_DOCUMENT_LOAD_COMPLETE or
   *                                                     nsIAccessibleEvent::EVENT_DOCUMENT_LOAD_STOPPED
   */
  /* void processDocLoadEvent (in nsITimer aTimer, in voidPtr aClosure, in PRUint32 aEventType); */
  NS_IMETHOD ProcessDocLoadEvent(nsITimer *aTimer, void * aClosure, PRUint32 aEventType) = 0;

  /**
   * Fire accessible event of the given type for the given target.
   *
   * @param aEvent   [in] accessible event type
   * @param aTarget  [in] target of accessible event
   */
  /* void fireAccessibleEvent (in unsigned long aEvent, in nsIAccessible aTarget); */
  NS_IMETHOD FireAccessibleEvent(PRUint32 aEvent, nsIAccessible *aTarget) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIAccessibilityService, NS_IACCESSIBILITYSERVICE_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIACCESSIBILITYSERVICE \
  NS_IMETHOD CreateOuterDocAccessible(nsIDOMNode *aNode, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateRootAccessible(nsIPresShell *aShell, nsIDocument *aDocument, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTML4ButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHyperTextAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLBRAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLAccessibleByMarkup(nsIFrame *aFrame, nsIWeakReference *aWeakShell, nsIDOMNode *aDOMNode, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLLIAccessible(nsIFrame *aFrame, nsIFrame *aBulletFrame, const nsAString & aBulletText, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLCheckboxAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLComboboxAccessible(nsIDOMNode *aNode, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLGenericAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLGroupboxAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLHRAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLImageAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLLabelAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLListboxAccessible(nsIDOMNode *aNode, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLMediaAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLObjectFrameAccessible(nsObjectFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLRadioButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLSelectOptionAccessible(nsIDOMNode *aNode, nsIAccessible *aAccParent, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLTableAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLTableCellAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLTextAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLTextFieldAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD CreateHTMLCaptionAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD GetAccessible(nsIDOMNode *aNode, nsIPresShell *aPresShell, nsIWeakReference *aWeakShell, nsIFrame **frameHint NS_INOUTPARAM, PRBool *aIsHidden NS_OUTPARAM, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD AddNativeRootAccessible(void * aAtkAccessible, nsIAccessible **_retval NS_OUTPARAM); \
  NS_IMETHOD RemoveNativeRootAccessible(nsIAccessible *aRootAccessible); \
  NS_IMETHOD InvalidateSubtreeFor(nsIPresShell *aPresShell, nsIContent *aChangedContent, PRUint32 aEvent); \
  NS_IMETHOD ProcessDocLoadEvent(nsITimer *aTimer, void * aClosure, PRUint32 aEventType); \
  NS_IMETHOD FireAccessibleEvent(PRUint32 aEvent, nsIAccessible *aTarget); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIACCESSIBILITYSERVICE(_to) \
  NS_IMETHOD CreateOuterDocAccessible(nsIDOMNode *aNode, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateOuterDocAccessible(aNode, _retval); } \
  NS_IMETHOD CreateRootAccessible(nsIPresShell *aShell, nsIDocument *aDocument, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateRootAccessible(aShell, aDocument, _retval); } \
  NS_IMETHOD CreateHTML4ButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTML4ButtonAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHyperTextAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHyperTextAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLBRAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLBRAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLButtonAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLAccessibleByMarkup(nsIFrame *aFrame, nsIWeakReference *aWeakShell, nsIDOMNode *aDOMNode, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLAccessibleByMarkup(aFrame, aWeakShell, aDOMNode, _retval); } \
  NS_IMETHOD CreateHTMLLIAccessible(nsIFrame *aFrame, nsIFrame *aBulletFrame, const nsAString & aBulletText, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLLIAccessible(aFrame, aBulletFrame, aBulletText, _retval); } \
  NS_IMETHOD CreateHTMLCheckboxAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLCheckboxAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLComboboxAccessible(nsIDOMNode *aNode, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLComboboxAccessible(aNode, aPresShell, _retval); } \
  NS_IMETHOD CreateHTMLGenericAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLGenericAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLGroupboxAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLGroupboxAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLHRAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLHRAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLImageAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLImageAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLLabelAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLLabelAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLListboxAccessible(nsIDOMNode *aNode, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLListboxAccessible(aNode, aPresShell, _retval); } \
  NS_IMETHOD CreateHTMLMediaAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLMediaAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLObjectFrameAccessible(nsObjectFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLObjectFrameAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLRadioButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLRadioButtonAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLSelectOptionAccessible(nsIDOMNode *aNode, nsIAccessible *aAccParent, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLSelectOptionAccessible(aNode, aAccParent, aPresShell, _retval); } \
  NS_IMETHOD CreateHTMLTableAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLTableAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLTableCellAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLTableCellAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLTextAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLTextAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLTextFieldAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLTextFieldAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLCaptionAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return _to CreateHTMLCaptionAccessible(aFrame, _retval); } \
  NS_IMETHOD GetAccessible(nsIDOMNode *aNode, nsIPresShell *aPresShell, nsIWeakReference *aWeakShell, nsIFrame **frameHint NS_INOUTPARAM, PRBool *aIsHidden NS_OUTPARAM, nsIAccessible **_retval NS_OUTPARAM) { return _to GetAccessible(aNode, aPresShell, aWeakShell, frameHint, aIsHidden, _retval); } \
  NS_IMETHOD AddNativeRootAccessible(void * aAtkAccessible, nsIAccessible **_retval NS_OUTPARAM) { return _to AddNativeRootAccessible(aAtkAccessible, _retval); } \
  NS_IMETHOD RemoveNativeRootAccessible(nsIAccessible *aRootAccessible) { return _to RemoveNativeRootAccessible(aRootAccessible); } \
  NS_IMETHOD InvalidateSubtreeFor(nsIPresShell *aPresShell, nsIContent *aChangedContent, PRUint32 aEvent) { return _to InvalidateSubtreeFor(aPresShell, aChangedContent, aEvent); } \
  NS_IMETHOD ProcessDocLoadEvent(nsITimer *aTimer, void * aClosure, PRUint32 aEventType) { return _to ProcessDocLoadEvent(aTimer, aClosure, aEventType); } \
  NS_IMETHOD FireAccessibleEvent(PRUint32 aEvent, nsIAccessible *aTarget) { return _to FireAccessibleEvent(aEvent, aTarget); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIACCESSIBILITYSERVICE(_to) \
  NS_IMETHOD CreateOuterDocAccessible(nsIDOMNode *aNode, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateOuterDocAccessible(aNode, _retval); } \
  NS_IMETHOD CreateRootAccessible(nsIPresShell *aShell, nsIDocument *aDocument, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateRootAccessible(aShell, aDocument, _retval); } \
  NS_IMETHOD CreateHTML4ButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTML4ButtonAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHyperTextAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHyperTextAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLBRAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLBRAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLButtonAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLAccessibleByMarkup(nsIFrame *aFrame, nsIWeakReference *aWeakShell, nsIDOMNode *aDOMNode, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLAccessibleByMarkup(aFrame, aWeakShell, aDOMNode, _retval); } \
  NS_IMETHOD CreateHTMLLIAccessible(nsIFrame *aFrame, nsIFrame *aBulletFrame, const nsAString & aBulletText, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLLIAccessible(aFrame, aBulletFrame, aBulletText, _retval); } \
  NS_IMETHOD CreateHTMLCheckboxAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLCheckboxAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLComboboxAccessible(nsIDOMNode *aNode, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLComboboxAccessible(aNode, aPresShell, _retval); } \
  NS_IMETHOD CreateHTMLGenericAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLGenericAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLGroupboxAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLGroupboxAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLHRAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLHRAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLImageAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLImageAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLLabelAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLLabelAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLListboxAccessible(nsIDOMNode *aNode, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLListboxAccessible(aNode, aPresShell, _retval); } \
  NS_IMETHOD CreateHTMLMediaAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLMediaAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLObjectFrameAccessible(nsObjectFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLObjectFrameAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLRadioButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLRadioButtonAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLSelectOptionAccessible(nsIDOMNode *aNode, nsIAccessible *aAccParent, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLSelectOptionAccessible(aNode, aAccParent, aPresShell, _retval); } \
  NS_IMETHOD CreateHTMLTableAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLTableAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLTableCellAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLTableCellAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLTextAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLTextAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLTextFieldAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLTextFieldAccessible(aFrame, _retval); } \
  NS_IMETHOD CreateHTMLCaptionAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateHTMLCaptionAccessible(aFrame, _retval); } \
  NS_IMETHOD GetAccessible(nsIDOMNode *aNode, nsIPresShell *aPresShell, nsIWeakReference *aWeakShell, nsIFrame **frameHint NS_INOUTPARAM, PRBool *aIsHidden NS_OUTPARAM, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAccessible(aNode, aPresShell, aWeakShell, frameHint, aIsHidden, _retval); } \
  NS_IMETHOD AddNativeRootAccessible(void * aAtkAccessible, nsIAccessible **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->AddNativeRootAccessible(aAtkAccessible, _retval); } \
  NS_IMETHOD RemoveNativeRootAccessible(nsIAccessible *aRootAccessible) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveNativeRootAccessible(aRootAccessible); } \
  NS_IMETHOD InvalidateSubtreeFor(nsIPresShell *aPresShell, nsIContent *aChangedContent, PRUint32 aEvent) { return !_to ? NS_ERROR_NULL_POINTER : _to->InvalidateSubtreeFor(aPresShell, aChangedContent, aEvent); } \
  NS_IMETHOD ProcessDocLoadEvent(nsITimer *aTimer, void * aClosure, PRUint32 aEventType) { return !_to ? NS_ERROR_NULL_POINTER : _to->ProcessDocLoadEvent(aTimer, aClosure, aEventType); } \
  NS_IMETHOD FireAccessibleEvent(PRUint32 aEvent, nsIAccessible *aTarget) { return !_to ? NS_ERROR_NULL_POINTER : _to->FireAccessibleEvent(aEvent, aTarget); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsAccessibilityService : public nsIAccessibilityService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIACCESSIBILITYSERVICE

  nsAccessibilityService();

private:
  ~nsAccessibilityService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsAccessibilityService, nsIAccessibilityService)

nsAccessibilityService::nsAccessibilityService()
{
  /* member initializers and constructor code */
}

nsAccessibilityService::~nsAccessibilityService()
{
  /* destructor code */
}

/* nsIAccessible createOuterDocAccessible (in nsIDOMNode aNode); */
NS_IMETHODIMP nsAccessibilityService::CreateOuterDocAccessible(nsIDOMNode *aNode, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createRootAccessible (in nsIPresShell aShell, in nsIDocument aDocument); */
NS_IMETHODIMP nsAccessibilityService::CreateRootAccessible(nsIPresShell *aShell, nsIDocument *aDocument, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTML4ButtonAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTML4ButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHyperTextAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHyperTextAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLBRAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLBRAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLButtonAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLAccessibleByMarkup (in nsIFrame aFrame, in nsIWeakReference aWeakShell, in nsIDOMNode aDOMNode); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLAccessibleByMarkup(nsIFrame *aFrame, nsIWeakReference *aWeakShell, nsIDOMNode *aDOMNode, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLLIAccessible (in nsIFrame aFrame, in nsIFrame aBulletFrame, in AString aBulletText); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLLIAccessible(nsIFrame *aFrame, nsIFrame *aBulletFrame, const nsAString & aBulletText, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLCheckboxAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLCheckboxAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLComboboxAccessible (in nsIDOMNode aNode, in nsIWeakReference aPresShell); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLComboboxAccessible(nsIDOMNode *aNode, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLGenericAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLGenericAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLGroupboxAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLGroupboxAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLHRAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLHRAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLImageAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLImageAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLLabelAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLLabelAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLListboxAccessible (in nsIDOMNode aNode, in nsIWeakReference aPresShell); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLListboxAccessible(nsIDOMNode *aNode, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLMediaAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLMediaAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLObjectFrameAccessible (in nsObjectFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLObjectFrameAccessible(nsObjectFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLRadioButtonAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLRadioButtonAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLSelectOptionAccessible (in nsIDOMNode aNode, in nsIAccessible aAccParent, in nsIWeakReference aPresShell); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLSelectOptionAccessible(nsIDOMNode *aNode, nsIAccessible *aAccParent, nsIWeakReference *aPresShell, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLTableAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLTableAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLTableCellAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLTableCellAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLTextAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLTextAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLTextFieldAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLTextFieldAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible createHTMLCaptionAccessible (in nsIFrame aFrame); */
NS_IMETHODIMP nsAccessibilityService::CreateHTMLCaptionAccessible(nsIFrame *aFrame, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible getAccessible (in nsIDOMNode aNode, in nsIPresShell aPresShell, in nsIWeakReference aWeakShell, inout nsIFrame frameHint, out boolean aIsHidden); */
NS_IMETHODIMP nsAccessibilityService::GetAccessible(nsIDOMNode *aNode, nsIPresShell *aPresShell, nsIWeakReference *aWeakShell, nsIFrame **frameHint NS_INOUTPARAM, PRBool *aIsHidden NS_OUTPARAM, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIAccessible addNativeRootAccessible (in voidPtr aAtkAccessible); */
NS_IMETHODIMP nsAccessibilityService::AddNativeRootAccessible(void * aAtkAccessible, nsIAccessible **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeNativeRootAccessible (in nsIAccessible aRootAccessible); */
NS_IMETHODIMP nsAccessibilityService::RemoveNativeRootAccessible(nsIAccessible *aRootAccessible)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void invalidateSubtreeFor (in nsIPresShell aPresShell, in nsIContent aChangedContent, in PRUint32 aEvent); */
NS_IMETHODIMP nsAccessibilityService::InvalidateSubtreeFor(nsIPresShell *aPresShell, nsIContent *aChangedContent, PRUint32 aEvent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void processDocLoadEvent (in nsITimer aTimer, in voidPtr aClosure, in PRUint32 aEventType); */
NS_IMETHODIMP nsAccessibilityService::ProcessDocLoadEvent(nsITimer *aTimer, void * aClosure, PRUint32 aEventType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void fireAccessibleEvent (in unsigned long aEvent, in nsIAccessible aTarget); */
NS_IMETHODIMP nsAccessibilityService::FireAccessibleEvent(PRUint32 aEvent, nsIAccessible *aTarget)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

// for component registration
// {DE401C37-9A7F-4278-A6F8-3DE2833989EF}
#define NS_ACCESSIBILITY_SERVICE_CID \
{ 0xde401c37, 0x9a7f, 0x4278, { 0xa6, 0xf8, 0x3d, 0xe2, 0x83, 0x39, 0x89, 0xef } }
extern nsresult
NS_GetAccessibilityService(nsIAccessibilityService** aResult);

#endif /* __gen_nsIAccessibilityService_h__ */
