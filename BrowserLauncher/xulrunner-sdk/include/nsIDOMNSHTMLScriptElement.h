/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/dom/interfaces/html/nsIDOMNSHTMLScriptElement.idl
 */

#ifndef __gen_nsIDOMNSHTMLScriptElement_h__
#define __gen_nsIDOMNSHTMLScriptElement_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMNSHTMLScriptElement */
#define NS_IDOMNSHTMLSCRIPTELEMENT_IID_STR "5b2065d7-7888-4529-8a29-e58390a40bd2"

#define NS_IDOMNSHTMLSCRIPTELEMENT_IID \
  {0x5b2065d7, 0x7888, 0x4529, \
    { 0x8a, 0x29, 0xe5, 0x83, 0x90, 0xa4, 0x0b, 0xd2 }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIDOMNSHTMLScriptElement : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMNSHTMLSCRIPTELEMENT_IID)

  /* attribute boolean async; */
  NS_SCRIPTABLE NS_IMETHOD GetAsync(PRBool *aAsync) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetAsync(PRBool aAsync) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMNSHTMLScriptElement, NS_IDOMNSHTMLSCRIPTELEMENT_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMNSHTMLSCRIPTELEMENT \
  NS_SCRIPTABLE NS_IMETHOD GetAsync(PRBool *aAsync); \
  NS_SCRIPTABLE NS_IMETHOD SetAsync(PRBool aAsync); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMNSHTMLSCRIPTELEMENT(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetAsync(PRBool *aAsync) { return _to GetAsync(aAsync); } \
  NS_SCRIPTABLE NS_IMETHOD SetAsync(PRBool aAsync) { return _to SetAsync(aAsync); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMNSHTMLSCRIPTELEMENT(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetAsync(PRBool *aAsync) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAsync(aAsync); } \
  NS_SCRIPTABLE NS_IMETHOD SetAsync(PRBool aAsync) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetAsync(aAsync); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMNSHTMLScriptElement : public nsIDOMNSHTMLScriptElement
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMNSHTMLSCRIPTELEMENT

  nsDOMNSHTMLScriptElement();

private:
  ~nsDOMNSHTMLScriptElement();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMNSHTMLScriptElement, nsIDOMNSHTMLScriptElement)

nsDOMNSHTMLScriptElement::nsDOMNSHTMLScriptElement()
{
  /* member initializers and constructor code */
}

nsDOMNSHTMLScriptElement::~nsDOMNSHTMLScriptElement()
{
  /* destructor code */
}

/* attribute boolean async; */
NS_IMETHODIMP nsDOMNSHTMLScriptElement::GetAsync(PRBool *aAsync)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMNSHTMLScriptElement::SetAsync(PRBool aAsync)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMNSHTMLScriptElement_h__ */
