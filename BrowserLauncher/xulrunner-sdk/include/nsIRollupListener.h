/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/widget/public/nsIRollupListener.idl
 */

#ifndef __gen_nsIRollupListener_h__
#define __gen_nsIRollupListener_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIContent; /* forward declaration */


/* starting interface:    nsIRollupListener */
#define NS_IROLLUPLISTENER_IID_STR "0ca103e5-80d4-4b81-a310-be0708f8eaa9"

#define NS_IROLLUPLISTENER_IID \
  {0x0ca103e5, 0x80d4, 0x4b81, \
    { 0xa3, 0x10, 0xbe, 0x07, 0x08, 0xf8, 0xea, 0xa9 }}

class NS_NO_VTABLE nsIRollupListener : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IROLLUPLISTENER_IID)

  /**
   * Notifies the object to rollup, optionally returning the node that
   * was just rolled up.
   *
   * aCount is the number of popups in a chain to close. If this is
   * PR_UINT32_MAX, then all popups are closed.
   *
   * @result NS_Ok if no errors
   */
  /* nsIContent Rollup (in unsigned long aCount); */
  NS_IMETHOD Rollup(PRUint32 aCount, nsIContent **_retval NS_OUTPARAM) = 0;

  /**
   * Asks the RollupListener if it should rollup on mousevents
   * @result NS_Ok if no errors
   */
  /* void ShouldRollupOnMouseWheelEvent (out PRBool aShould); */
  NS_IMETHOD ShouldRollupOnMouseWheelEvent(PRBool *aShould NS_OUTPARAM) = 0;

  /**
   * Asks the RollupListener if it should rollup on mouse activate, eg. X-Mouse
   * @result NS_Ok if no errors
   */
  /* void ShouldRollupOnMouseActivate (out PRBool aShould); */
  NS_IMETHOD ShouldRollupOnMouseActivate(PRBool *aShould NS_OUTPARAM) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIRollupListener, NS_IROLLUPLISTENER_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIROLLUPLISTENER \
  NS_IMETHOD Rollup(PRUint32 aCount, nsIContent **_retval NS_OUTPARAM); \
  NS_IMETHOD ShouldRollupOnMouseWheelEvent(PRBool *aShould NS_OUTPARAM); \
  NS_IMETHOD ShouldRollupOnMouseActivate(PRBool *aShould NS_OUTPARAM); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIROLLUPLISTENER(_to) \
  NS_IMETHOD Rollup(PRUint32 aCount, nsIContent **_retval NS_OUTPARAM) { return _to Rollup(aCount, _retval); } \
  NS_IMETHOD ShouldRollupOnMouseWheelEvent(PRBool *aShould NS_OUTPARAM) { return _to ShouldRollupOnMouseWheelEvent(aShould); } \
  NS_IMETHOD ShouldRollupOnMouseActivate(PRBool *aShould NS_OUTPARAM) { return _to ShouldRollupOnMouseActivate(aShould); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIROLLUPLISTENER(_to) \
  NS_IMETHOD Rollup(PRUint32 aCount, nsIContent **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->Rollup(aCount, _retval); } \
  NS_IMETHOD ShouldRollupOnMouseWheelEvent(PRBool *aShould NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->ShouldRollupOnMouseWheelEvent(aShould); } \
  NS_IMETHOD ShouldRollupOnMouseActivate(PRBool *aShould NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->ShouldRollupOnMouseActivate(aShould); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsRollupListener : public nsIRollupListener
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIROLLUPLISTENER

  nsRollupListener();

private:
  ~nsRollupListener();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsRollupListener, nsIRollupListener)

nsRollupListener::nsRollupListener()
{
  /* member initializers and constructor code */
}

nsRollupListener::~nsRollupListener()
{
  /* destructor code */
}

/* nsIContent Rollup (in unsigned long aCount); */
NS_IMETHODIMP nsRollupListener::Rollup(PRUint32 aCount, nsIContent **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void ShouldRollupOnMouseWheelEvent (out PRBool aShould); */
NS_IMETHODIMP nsRollupListener::ShouldRollupOnMouseWheelEvent(PRBool *aShould NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void ShouldRollupOnMouseActivate (out PRBool aShould); */
NS_IMETHODIMP nsRollupListener::ShouldRollupOnMouseActivate(PRBool *aShould NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIRollupListener_h__ */
