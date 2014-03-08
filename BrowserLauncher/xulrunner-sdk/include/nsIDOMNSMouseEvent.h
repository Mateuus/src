/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/dom/interfaces/events/nsIDOMNSMouseEvent.idl
 */

#ifndef __gen_nsIDOMNSMouseEvent_h__
#define __gen_nsIDOMNSMouseEvent_h__


#ifndef __gen_nsIDOMMouseEvent_h__
#include "nsIDOMMouseEvent.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMNSMouseEvent */
#define NS_IDOMNSMOUSEEVENT_IID_STR "1b8e528d-7dca-44ee-8ee6-c44594ebcef1"

#define NS_IDOMNSMOUSEEVENT_IID \
  {0x1b8e528d, 0x7dca, 0x44ee, \
    { 0x8e, 0xe6, 0xc4, 0x45, 0x94, 0xeb, 0xce, 0xf1 }}

/**
 * The nsIDOMNSMouseEvent interface extends nsIDOMMouseEvent
 * by providing various information related to the mouse event.
 */
class NS_NO_VTABLE NS_SCRIPTABLE nsIDOMNSMouseEvent : public nsIDOMMouseEvent {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMNSMOUSEEVENT_IID)

  /* readonly attribute float mozPressure; */
  NS_SCRIPTABLE NS_IMETHOD GetMozPressure(float *aMozPressure) = 0;

  /* void initNSMouseEvent (in DOMString typeArg, in boolean canBubbleArg, in boolean cancelableArg, in nsIDOMAbstractView viewArg, in long detailArg, in long screenXArg, in long screenYArg, in long clientXArg, in long clientYArg, in boolean ctrlKeyArg, in boolean altKeyArg, in boolean shiftKeyArg, in boolean metaKeyArg, in unsigned short buttonArg, in nsIDOMEventTarget relatedTargetArg, in float pressure); */
  NS_SCRIPTABLE NS_IMETHOD InitNSMouseEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIDOMAbstractView *viewArg, PRInt32 detailArg, PRInt32 screenXArg, PRInt32 screenYArg, PRInt32 clientXArg, PRInt32 clientYArg, PRBool ctrlKeyArg, PRBool altKeyArg, PRBool shiftKeyArg, PRBool metaKeyArg, PRUint16 buttonArg, nsIDOMEventTarget *relatedTargetArg, float pressure) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMNSMouseEvent, NS_IDOMNSMOUSEEVENT_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMNSMOUSEEVENT \
  NS_SCRIPTABLE NS_IMETHOD GetMozPressure(float *aMozPressure); \
  NS_SCRIPTABLE NS_IMETHOD InitNSMouseEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIDOMAbstractView *viewArg, PRInt32 detailArg, PRInt32 screenXArg, PRInt32 screenYArg, PRInt32 clientXArg, PRInt32 clientYArg, PRBool ctrlKeyArg, PRBool altKeyArg, PRBool shiftKeyArg, PRBool metaKeyArg, PRUint16 buttonArg, nsIDOMEventTarget *relatedTargetArg, float pressure); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMNSMOUSEEVENT(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetMozPressure(float *aMozPressure) { return _to GetMozPressure(aMozPressure); } \
  NS_SCRIPTABLE NS_IMETHOD InitNSMouseEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIDOMAbstractView *viewArg, PRInt32 detailArg, PRInt32 screenXArg, PRInt32 screenYArg, PRInt32 clientXArg, PRInt32 clientYArg, PRBool ctrlKeyArg, PRBool altKeyArg, PRBool shiftKeyArg, PRBool metaKeyArg, PRUint16 buttonArg, nsIDOMEventTarget *relatedTargetArg, float pressure) { return _to InitNSMouseEvent(typeArg, canBubbleArg, cancelableArg, viewArg, detailArg, screenXArg, screenYArg, clientXArg, clientYArg, ctrlKeyArg, altKeyArg, shiftKeyArg, metaKeyArg, buttonArg, relatedTargetArg, pressure); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMNSMOUSEEVENT(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetMozPressure(float *aMozPressure) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMozPressure(aMozPressure); } \
  NS_SCRIPTABLE NS_IMETHOD InitNSMouseEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIDOMAbstractView *viewArg, PRInt32 detailArg, PRInt32 screenXArg, PRInt32 screenYArg, PRInt32 clientXArg, PRInt32 clientYArg, PRBool ctrlKeyArg, PRBool altKeyArg, PRBool shiftKeyArg, PRBool metaKeyArg, PRUint16 buttonArg, nsIDOMEventTarget *relatedTargetArg, float pressure) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitNSMouseEvent(typeArg, canBubbleArg, cancelableArg, viewArg, detailArg, screenXArg, screenYArg, clientXArg, clientYArg, ctrlKeyArg, altKeyArg, shiftKeyArg, metaKeyArg, buttonArg, relatedTargetArg, pressure); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMNSMouseEvent : public nsIDOMNSMouseEvent
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMNSMOUSEEVENT

  nsDOMNSMouseEvent();

private:
  ~nsDOMNSMouseEvent();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMNSMouseEvent, nsIDOMNSMouseEvent)

nsDOMNSMouseEvent::nsDOMNSMouseEvent()
{
  /* member initializers and constructor code */
}

nsDOMNSMouseEvent::~nsDOMNSMouseEvent()
{
  /* destructor code */
}

/* readonly attribute float mozPressure; */
NS_IMETHODIMP nsDOMNSMouseEvent::GetMozPressure(float *aMozPressure)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void initNSMouseEvent (in DOMString typeArg, in boolean canBubbleArg, in boolean cancelableArg, in nsIDOMAbstractView viewArg, in long detailArg, in long screenXArg, in long screenYArg, in long clientXArg, in long clientYArg, in boolean ctrlKeyArg, in boolean altKeyArg, in boolean shiftKeyArg, in boolean metaKeyArg, in unsigned short buttonArg, in nsIDOMEventTarget relatedTargetArg, in float pressure); */
NS_IMETHODIMP nsDOMNSMouseEvent::InitNSMouseEvent(const nsAString & typeArg, PRBool canBubbleArg, PRBool cancelableArg, nsIDOMAbstractView *viewArg, PRInt32 detailArg, PRInt32 screenXArg, PRInt32 screenYArg, PRInt32 clientXArg, PRInt32 clientYArg, PRBool ctrlKeyArg, PRBool altKeyArg, PRBool shiftKeyArg, PRBool metaKeyArg, PRUint16 buttonArg, nsIDOMEventTarget *relatedTargetArg, float pressure)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMNSMouseEvent_h__ */
