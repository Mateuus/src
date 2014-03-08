/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/dom/interfaces/html/nsIDOMNSHTMLInputElement.idl
 */

#ifndef __gen_nsIDOMNSHTMLInputElement_h__
#define __gen_nsIDOMNSHTMLInputElement_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIControllers; /* forward declaration */

class nsIDOMFileList; /* forward declaration */


/* starting interface:    nsIDOMNSHTMLInputElement */
#define NS_IDOMNSHTMLINPUTELEMENT_IID_STR "2cb61f32-b21f-4b87-904c-8876d8bb5f33"

#define NS_IDOMNSHTMLINPUTELEMENT_IID \
  {0x2cb61f32, 0xb21f, 0x4b87, \
    { 0x90, 0x4c, 0x88, 0x76, 0xd8, 0xbb, 0x5f, 0x33 }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIDOMNSHTMLInputElement : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMNSHTMLINPUTELEMENT_IID)

  /* readonly attribute nsIControllers controllers; */
  NS_SCRIPTABLE NS_IMETHOD GetControllers(nsIControllers * *aControllers) = 0;

  /* readonly attribute long textLength; */
  NS_SCRIPTABLE NS_IMETHOD GetTextLength(PRInt32 *aTextLength) = 0;

  /* attribute long selectionStart; */
  NS_SCRIPTABLE NS_IMETHOD GetSelectionStart(PRInt32 *aSelectionStart) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetSelectionStart(PRInt32 aSelectionStart) = 0;

  /* attribute long selectionEnd; */
  NS_SCRIPTABLE NS_IMETHOD GetSelectionEnd(PRInt32 *aSelectionEnd) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetSelectionEnd(PRInt32 aSelectionEnd) = 0;

  /* readonly attribute nsIDOMFileList files; */
  NS_SCRIPTABLE NS_IMETHOD GetFiles(nsIDOMFileList * *aFiles) = 0;

  /* attribute boolean indeterminate; */
  NS_SCRIPTABLE NS_IMETHOD GetIndeterminate(PRBool *aIndeterminate) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetIndeterminate(PRBool aIndeterminate) = 0;

  /* attribute boolean multiple; */
  NS_SCRIPTABLE NS_IMETHOD GetMultiple(PRBool *aMultiple) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetMultiple(PRBool aMultiple) = 0;

  /* void mozGetFileNameArray ([optional] out unsigned long aLength, [array, size_is (aLength), retval] out wstring aFileNames); */
  NS_SCRIPTABLE NS_IMETHOD MozGetFileNameArray(PRUint32 *aLength NS_OUTPARAM, PRUnichar ***aFileNames NS_OUTPARAM) = 0;

  /* void mozSetFileNameArray ([array, size_is (aLength)] in wstring aFileNames, in unsigned long aLength); */
  NS_SCRIPTABLE NS_IMETHOD MozSetFileNameArray(const PRUnichar **aFileNames, PRUint32 aLength) = 0;

  /* void setSelectionRange (in long selectionStart, in long selectionEnd); */
  NS_SCRIPTABLE NS_IMETHOD SetSelectionRange(PRInt32 selectionStart, PRInt32 selectionEnd) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMNSHTMLInputElement, NS_IDOMNSHTMLINPUTELEMENT_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMNSHTMLINPUTELEMENT \
  NS_SCRIPTABLE NS_IMETHOD GetControllers(nsIControllers * *aControllers); \
  NS_SCRIPTABLE NS_IMETHOD GetTextLength(PRInt32 *aTextLength); \
  NS_SCRIPTABLE NS_IMETHOD GetSelectionStart(PRInt32 *aSelectionStart); \
  NS_SCRIPTABLE NS_IMETHOD SetSelectionStart(PRInt32 aSelectionStart); \
  NS_SCRIPTABLE NS_IMETHOD GetSelectionEnd(PRInt32 *aSelectionEnd); \
  NS_SCRIPTABLE NS_IMETHOD SetSelectionEnd(PRInt32 aSelectionEnd); \
  NS_SCRIPTABLE NS_IMETHOD GetFiles(nsIDOMFileList * *aFiles); \
  NS_SCRIPTABLE NS_IMETHOD GetIndeterminate(PRBool *aIndeterminate); \
  NS_SCRIPTABLE NS_IMETHOD SetIndeterminate(PRBool aIndeterminate); \
  NS_SCRIPTABLE NS_IMETHOD GetMultiple(PRBool *aMultiple); \
  NS_SCRIPTABLE NS_IMETHOD SetMultiple(PRBool aMultiple); \
  NS_SCRIPTABLE NS_IMETHOD MozGetFileNameArray(PRUint32 *aLength NS_OUTPARAM, PRUnichar ***aFileNames NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD MozSetFileNameArray(const PRUnichar **aFileNames, PRUint32 aLength); \
  NS_SCRIPTABLE NS_IMETHOD SetSelectionRange(PRInt32 selectionStart, PRInt32 selectionEnd); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMNSHTMLINPUTELEMENT(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetControllers(nsIControllers * *aControllers) { return _to GetControllers(aControllers); } \
  NS_SCRIPTABLE NS_IMETHOD GetTextLength(PRInt32 *aTextLength) { return _to GetTextLength(aTextLength); } \
  NS_SCRIPTABLE NS_IMETHOD GetSelectionStart(PRInt32 *aSelectionStart) { return _to GetSelectionStart(aSelectionStart); } \
  NS_SCRIPTABLE NS_IMETHOD SetSelectionStart(PRInt32 aSelectionStart) { return _to SetSelectionStart(aSelectionStart); } \
  NS_SCRIPTABLE NS_IMETHOD GetSelectionEnd(PRInt32 *aSelectionEnd) { return _to GetSelectionEnd(aSelectionEnd); } \
  NS_SCRIPTABLE NS_IMETHOD SetSelectionEnd(PRInt32 aSelectionEnd) { return _to SetSelectionEnd(aSelectionEnd); } \
  NS_SCRIPTABLE NS_IMETHOD GetFiles(nsIDOMFileList * *aFiles) { return _to GetFiles(aFiles); } \
  NS_SCRIPTABLE NS_IMETHOD GetIndeterminate(PRBool *aIndeterminate) { return _to GetIndeterminate(aIndeterminate); } \
  NS_SCRIPTABLE NS_IMETHOD SetIndeterminate(PRBool aIndeterminate) { return _to SetIndeterminate(aIndeterminate); } \
  NS_SCRIPTABLE NS_IMETHOD GetMultiple(PRBool *aMultiple) { return _to GetMultiple(aMultiple); } \
  NS_SCRIPTABLE NS_IMETHOD SetMultiple(PRBool aMultiple) { return _to SetMultiple(aMultiple); } \
  NS_SCRIPTABLE NS_IMETHOD MozGetFileNameArray(PRUint32 *aLength NS_OUTPARAM, PRUnichar ***aFileNames NS_OUTPARAM) { return _to MozGetFileNameArray(aLength, aFileNames); } \
  NS_SCRIPTABLE NS_IMETHOD MozSetFileNameArray(const PRUnichar **aFileNames, PRUint32 aLength) { return _to MozSetFileNameArray(aFileNames, aLength); } \
  NS_SCRIPTABLE NS_IMETHOD SetSelectionRange(PRInt32 selectionStart, PRInt32 selectionEnd) { return _to SetSelectionRange(selectionStart, selectionEnd); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMNSHTMLINPUTELEMENT(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetControllers(nsIControllers * *aControllers) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetControllers(aControllers); } \
  NS_SCRIPTABLE NS_IMETHOD GetTextLength(PRInt32 *aTextLength) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTextLength(aTextLength); } \
  NS_SCRIPTABLE NS_IMETHOD GetSelectionStart(PRInt32 *aSelectionStart) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSelectionStart(aSelectionStart); } \
  NS_SCRIPTABLE NS_IMETHOD SetSelectionStart(PRInt32 aSelectionStart) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSelectionStart(aSelectionStart); } \
  NS_SCRIPTABLE NS_IMETHOD GetSelectionEnd(PRInt32 *aSelectionEnd) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetSelectionEnd(aSelectionEnd); } \
  NS_SCRIPTABLE NS_IMETHOD SetSelectionEnd(PRInt32 aSelectionEnd) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSelectionEnd(aSelectionEnd); } \
  NS_SCRIPTABLE NS_IMETHOD GetFiles(nsIDOMFileList * *aFiles) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFiles(aFiles); } \
  NS_SCRIPTABLE NS_IMETHOD GetIndeterminate(PRBool *aIndeterminate) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIndeterminate(aIndeterminate); } \
  NS_SCRIPTABLE NS_IMETHOD SetIndeterminate(PRBool aIndeterminate) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetIndeterminate(aIndeterminate); } \
  NS_SCRIPTABLE NS_IMETHOD GetMultiple(PRBool *aMultiple) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMultiple(aMultiple); } \
  NS_SCRIPTABLE NS_IMETHOD SetMultiple(PRBool aMultiple) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetMultiple(aMultiple); } \
  NS_SCRIPTABLE NS_IMETHOD MozGetFileNameArray(PRUint32 *aLength NS_OUTPARAM, PRUnichar ***aFileNames NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->MozGetFileNameArray(aLength, aFileNames); } \
  NS_SCRIPTABLE NS_IMETHOD MozSetFileNameArray(const PRUnichar **aFileNames, PRUint32 aLength) { return !_to ? NS_ERROR_NULL_POINTER : _to->MozSetFileNameArray(aFileNames, aLength); } \
  NS_SCRIPTABLE NS_IMETHOD SetSelectionRange(PRInt32 selectionStart, PRInt32 selectionEnd) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetSelectionRange(selectionStart, selectionEnd); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMNSHTMLInputElement : public nsIDOMNSHTMLInputElement
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMNSHTMLINPUTELEMENT

  nsDOMNSHTMLInputElement();

private:
  ~nsDOMNSHTMLInputElement();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMNSHTMLInputElement, nsIDOMNSHTMLInputElement)

nsDOMNSHTMLInputElement::nsDOMNSHTMLInputElement()
{
  /* member initializers and constructor code */
}

nsDOMNSHTMLInputElement::~nsDOMNSHTMLInputElement()
{
  /* destructor code */
}

/* readonly attribute nsIControllers controllers; */
NS_IMETHODIMP nsDOMNSHTMLInputElement::GetControllers(nsIControllers * *aControllers)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long textLength; */
NS_IMETHODIMP nsDOMNSHTMLInputElement::GetTextLength(PRInt32 *aTextLength)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long selectionStart; */
NS_IMETHODIMP nsDOMNSHTMLInputElement::GetSelectionStart(PRInt32 *aSelectionStart)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMNSHTMLInputElement::SetSelectionStart(PRInt32 aSelectionStart)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long selectionEnd; */
NS_IMETHODIMP nsDOMNSHTMLInputElement::GetSelectionEnd(PRInt32 *aSelectionEnd)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMNSHTMLInputElement::SetSelectionEnd(PRInt32 aSelectionEnd)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMFileList files; */
NS_IMETHODIMP nsDOMNSHTMLInputElement::GetFiles(nsIDOMFileList * *aFiles)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean indeterminate; */
NS_IMETHODIMP nsDOMNSHTMLInputElement::GetIndeterminate(PRBool *aIndeterminate)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMNSHTMLInputElement::SetIndeterminate(PRBool aIndeterminate)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean multiple; */
NS_IMETHODIMP nsDOMNSHTMLInputElement::GetMultiple(PRBool *aMultiple)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMNSHTMLInputElement::SetMultiple(PRBool aMultiple)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void mozGetFileNameArray ([optional] out unsigned long aLength, [array, size_is (aLength), retval] out wstring aFileNames); */
NS_IMETHODIMP nsDOMNSHTMLInputElement::MozGetFileNameArray(PRUint32 *aLength NS_OUTPARAM, PRUnichar ***aFileNames NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void mozSetFileNameArray ([array, size_is (aLength)] in wstring aFileNames, in unsigned long aLength); */
NS_IMETHODIMP nsDOMNSHTMLInputElement::MozSetFileNameArray(const PRUnichar **aFileNames, PRUint32 aLength)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setSelectionRange (in long selectionStart, in long selectionEnd); */
NS_IMETHODIMP nsDOMNSHTMLInputElement::SetSelectionRange(PRInt32 selectionStart, PRInt32 selectionEnd)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMNSHTMLInputElement_h__ */
