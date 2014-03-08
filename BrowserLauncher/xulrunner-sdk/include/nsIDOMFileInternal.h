/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/content/base/public/nsIDOMFileInternal.idl
 */

#ifndef __gen_nsIDOMFileInternal_h__
#define __gen_nsIDOMFileInternal_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIFile; /* forward declaration */


/* starting interface:    nsIDOMFileInternal */
#define NS_IDOMFILEINTERNAL_IID_STR "047ca6c4-52b3-46f1-8976-e198b724f72f"

#define NS_IDOMFILEINTERNAL_IID \
  {0x047ca6c4, 0x52b3, 0x46f1, \
    { 0x89, 0x76, 0xe1, 0x98, 0xb7, 0x24, 0xf7, 0x2f }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIDOMFileInternal : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMFILEINTERNAL_IID)

  /* attribute nsIFile internalFile; */
  NS_SCRIPTABLE NS_IMETHOD GetInternalFile(nsIFile * *aInternalFile) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetInternalFile(nsIFile * aInternalFile) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMFileInternal, NS_IDOMFILEINTERNAL_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMFILEINTERNAL \
  NS_SCRIPTABLE NS_IMETHOD GetInternalFile(nsIFile * *aInternalFile); \
  NS_SCRIPTABLE NS_IMETHOD SetInternalFile(nsIFile * aInternalFile); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMFILEINTERNAL(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetInternalFile(nsIFile * *aInternalFile) { return _to GetInternalFile(aInternalFile); } \
  NS_SCRIPTABLE NS_IMETHOD SetInternalFile(nsIFile * aInternalFile) { return _to SetInternalFile(aInternalFile); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMFILEINTERNAL(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetInternalFile(nsIFile * *aInternalFile) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetInternalFile(aInternalFile); } \
  NS_SCRIPTABLE NS_IMETHOD SetInternalFile(nsIFile * aInternalFile) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetInternalFile(aInternalFile); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMFileInternal : public nsIDOMFileInternal
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMFILEINTERNAL

  nsDOMFileInternal();

private:
  ~nsDOMFileInternal();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMFileInternal, nsIDOMFileInternal)

nsDOMFileInternal::nsDOMFileInternal()
{
  /* member initializers and constructor code */
}

nsDOMFileInternal::~nsDOMFileInternal()
{
  /* destructor code */
}

/* attribute nsIFile internalFile; */
NS_IMETHODIMP nsDOMFileInternal::GetInternalFile(nsIFile * *aInternalFile)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsDOMFileInternal::SetInternalFile(nsIFile * aInternalFile)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMFileInternal_h__ */
