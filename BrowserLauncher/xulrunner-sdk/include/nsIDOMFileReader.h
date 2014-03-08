/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/content/base/public/nsIDOMFileReader.idl
 */

#ifndef __gen_nsIDOMFileReader_h__
#define __gen_nsIDOMFileReader_h__


#ifndef __gen_nsIDOMEventTarget_h__
#include "nsIDOMEventTarget.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMEventListener; /* forward declaration */

class nsIDOMFile; /* forward declaration */

class nsIDOMFileError; /* forward declaration */


/* starting interface:    nsIDOMFileReader */
#define NS_IDOMFILEREADER_IID_STR "5db0ce80-de44-40c0-a346-e28aac4aa978"

#define NS_IDOMFILEREADER_IID \
  {0x5db0ce80, 0xde44, 0x40c0, \
    { 0xa3, 0x46, 0xe2, 0x8a, 0xac, 0x4a, 0xa9, 0x78 }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIDOMFileReader : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMFILEREADER_IID)

  /* void readAsBinaryString (in nsIDOMFile filedata); */
  NS_SCRIPTABLE NS_IMETHOD ReadAsBinaryString(nsIDOMFile *filedata) = 0;

  /* void readAsText (in nsIDOMFile filedata, [optional] in DOMString encoding); */
  NS_SCRIPTABLE NS_IMETHOD ReadAsText(nsIDOMFile *filedata, const nsAString & encoding) = 0;

  /* void readAsDataURL (in nsIDOMFile file); */
  NS_SCRIPTABLE NS_IMETHOD ReadAsDataURL(nsIDOMFile *file) = 0;

  /* void abort (); */
  NS_SCRIPTABLE NS_IMETHOD Abort(void) = 0;

  enum { EMPTY = 0U };

  enum { LOADING = 1U };

  enum { DONE = 2U };

  /* readonly attribute unsigned short readyState; */
  NS_SCRIPTABLE NS_IMETHOD GetReadyState(PRUint16 *aReadyState) = 0;

  /* readonly attribute DOMString result; */
  NS_SCRIPTABLE NS_IMETHOD GetResult(nsAString & aResult) = 0;

  /* readonly attribute nsIDOMFileError error; */
  NS_SCRIPTABLE NS_IMETHOD GetError(nsIDOMFileError * *aError) = 0;

  /* attribute nsIDOMEventListener onloadend; */
  NS_SCRIPTABLE NS_IMETHOD GetOnloadend(nsIDOMEventListener * *aOnloadend) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetOnloadend(nsIDOMEventListener * aOnloadend) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMFileReader, NS_IDOMFILEREADER_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMFILEREADER \
  NS_SCRIPTABLE NS_IMETHOD ReadAsBinaryString(nsIDOMFile *filedata); \
  NS_SCRIPTABLE NS_IMETHOD ReadAsText(nsIDOMFile *filedata, const nsAString & encoding); \
  NS_SCRIPTABLE NS_IMETHOD ReadAsDataURL(nsIDOMFile *file); \
  NS_SCRIPTABLE NS_IMETHOD Abort(void); \
  NS_SCRIPTABLE NS_IMETHOD GetReadyState(PRUint16 *aReadyState); \
  NS_SCRIPTABLE NS_IMETHOD GetResult(nsAString & aResult); \
  NS_SCRIPTABLE NS_IMETHOD GetError(nsIDOMFileError * *aError); \
  NS_SCRIPTABLE NS_IMETHOD GetOnloadend(nsIDOMEventListener * *aOnloadend); \
  NS_SCRIPTABLE NS_IMETHOD SetOnloadend(nsIDOMEventListener * aOnloadend); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMFILEREADER(_to) \
  NS_SCRIPTABLE NS_IMETHOD ReadAsBinaryString(nsIDOMFile *filedata) { return _to ReadAsBinaryString(filedata); } \
  NS_SCRIPTABLE NS_IMETHOD ReadAsText(nsIDOMFile *filedata, const nsAString & encoding) { return _to ReadAsText(filedata, encoding); } \
  NS_SCRIPTABLE NS_IMETHOD ReadAsDataURL(nsIDOMFile *file) { return _to ReadAsDataURL(file); } \
  NS_SCRIPTABLE NS_IMETHOD Abort(void) { return _to Abort(); } \
  NS_SCRIPTABLE NS_IMETHOD GetReadyState(PRUint16 *aReadyState) { return _to GetReadyState(aReadyState); } \
  NS_SCRIPTABLE NS_IMETHOD GetResult(nsAString & aResult) { return _to GetResult(aResult); } \
  NS_SCRIPTABLE NS_IMETHOD GetError(nsIDOMFileError * *aError) { return _to GetError(aError); } \
  NS_SCRIPTABLE NS_IMETHOD GetOnloadend(nsIDOMEventListener * *aOnloadend) { return _to GetOnloadend(aOnloadend); } \
  NS_SCRIPTABLE NS_IMETHOD SetOnloadend(nsIDOMEventListener * aOnloadend) { return _to SetOnloadend(aOnloadend); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMFILEREADER(_to) \
  NS_SCRIPTABLE NS_IMETHOD ReadAsBinaryString(nsIDOMFile *filedata) { return !_to ? NS_ERROR_NULL_POINTER : _to->ReadAsBinaryString(filedata); } \
  NS_SCRIPTABLE NS_IMETHOD ReadAsText(nsIDOMFile *filedata, const nsAString & encoding) { return !_to ? NS_ERROR_NULL_POINTER : _to->ReadAsText(filedata, encoding); } \
  NS_SCRIPTABLE NS_IMETHOD ReadAsDataURL(nsIDOMFile *file) { return !_to ? NS_ERROR_NULL_POINTER : _to->ReadAsDataURL(file); } \
  NS_SCRIPTABLE NS_IMETHOD Abort(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Abort(); } \
  NS_SCRIPTABLE NS_IMETHOD GetReadyState(PRUint16 *aReadyState) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetReadyState(aReadyState); } \
  NS_SCRIPTABLE NS_IMETHOD GetResult(nsAString & aResult) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetResult(aResult); } \
  NS_SCRIPTABLE NS_IMETHOD GetError(nsIDOMFileError * *aError) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetError(aError); } \
  NS_SCRIPTABLE NS_IMETHOD GetOnloadend(nsIDOMEventListener * *aOnloadend) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOnloadend(aOnloadend); } \
  NS_SCRIPTABLE NS_IMETHOD SetOnloadend(nsIDOMEventListener * aOnloadend) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOnloadend(aOnloadend); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMFileReader : public nsIDOMFileReader
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMFILEREADER

  nsDOMFileReader();

private:
  ~nsDOMFileReader();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMFileReader, nsIDOMFileReader)

nsDOMFileReader::nsDOMFileReader()
{
  /* member initializers and constructor code */
}

nsDOMFileReader::~nsDOMFileReader()
{
  /* destructor code */
}

/* void readAsBinaryString (in nsIDOMFile filedata); */
NS_IMETHODIMP nsDOMFileReader::ReadAsBinaryString(nsIDOMFile *filedata)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void readAsText (in nsIDOMFile filedata, [optional] in DOMString encoding); */
NS_IMETHODIMP nsDOMFileReader::ReadAsText(nsIDOMFile *filedata, const nsAString & encoding)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void readAsDataURL (in nsIDOMFile file); */
NS_IMETHODIMP nsDOMFileReader::ReadAsDataURL(nsIDOMFile *file)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void abort (); */
NS_IMETHODIMP nsDOMFileReader::Abort()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned short readyState; */
NS_IMETHODIMP nsDOMFileReader::GetReadyState(PRUint16 *aReadyState)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString result; */
NS_IMETHODIMP nsDOMFileReader::GetResult(nsAString & aResult)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDOMFileError error; */
NS_IMETHODIMP nsDOMFileReader::GetError(nsIDOMFileError * *aError)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIDOMEventListener onloadend; */
NS_IMETHODIMP nsDOMFileReader::GetOnloadend(nsIDOMEventListener * *aOnloadend)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMFileReader::SetOnloadend(nsIDOMEventListener * aOnloadend)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#define NS_FILEREADER_CID                            \
{0x06aa7c21, 0xfe05, 0x4cf2,                         \
{0xb1, 0xc4, 0x0c, 0x71, 0x26, 0xa4, 0xf7, 0x13}}
#define NS_FILEREADER_CONTRACTID \
"@mozilla.org/files/filereader;1"

#endif /* __gen_nsIDOMFileReader_h__ */
