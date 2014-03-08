/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/js/src/xpconnect/idl/nsIJSRuntimeService.idl
 */

#ifndef __gen_nsIJSRuntimeService_h__
#define __gen_nsIJSRuntimeService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIXPCScriptable; /* forward declaration */


/* starting interface:    nsIJSRuntimeService */
#define NS_IJSRUNTIMESERVICE_IID_STR "e7d09265-4c23-4028-b1b0-c99e02aa78f8"

#define NS_IJSRUNTIMESERVICE_IID \
  {0xe7d09265, 0x4c23, 0x4028, \
    { 0xb1, 0xb0, 0xc9, 0x9e, 0x02, 0xaa, 0x78, 0xf8 }}

class NS_NO_VTABLE nsIJSRuntimeService : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IJSRUNTIMESERVICE_IID)

  /* readonly attribute JSRuntime runtime; */
  NS_IMETHOD GetRuntime(JSRuntime * *aRuntime) = 0;

  /* readonly attribute nsIXPCScriptable backstagePass; */
  NS_IMETHOD GetBackstagePass(nsIXPCScriptable * *aBackstagePass) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIJSRuntimeService, NS_IJSRUNTIMESERVICE_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIJSRUNTIMESERVICE \
  NS_IMETHOD GetRuntime(JSRuntime * *aRuntime); \
  NS_IMETHOD GetBackstagePass(nsIXPCScriptable * *aBackstagePass); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIJSRUNTIMESERVICE(_to) \
  NS_IMETHOD GetRuntime(JSRuntime * *aRuntime) { return _to GetRuntime(aRuntime); } \
  NS_IMETHOD GetBackstagePass(nsIXPCScriptable * *aBackstagePass) { return _to GetBackstagePass(aBackstagePass); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIJSRUNTIMESERVICE(_to) \
  NS_IMETHOD GetRuntime(JSRuntime * *aRuntime) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRuntime(aRuntime); } \
  NS_IMETHOD GetBackstagePass(nsIXPCScriptable * *aBackstagePass) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetBackstagePass(aBackstagePass); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsJSRuntimeService : public nsIJSRuntimeService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIJSRUNTIMESERVICE

  nsJSRuntimeService();

private:
  ~nsJSRuntimeService();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsJSRuntimeService, nsIJSRuntimeService)

nsJSRuntimeService::nsJSRuntimeService()
{
  /* member initializers and constructor code */
}

nsJSRuntimeService::~nsJSRuntimeService()
{
  /* destructor code */
}

/* readonly attribute JSRuntime runtime; */
NS_IMETHODIMP nsJSRuntimeService::GetRuntime(JSRuntime * *aRuntime)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIXPCScriptable backstagePass; */
NS_IMETHODIMP nsJSRuntimeService::GetBackstagePass(nsIXPCScriptable * *aBackstagePass)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIJSRuntimeService_MOZILLA_1_9_2 */
#define NS_IJSRUNTIMESERVICE_MOZILLA_1_9_2_IID_STR "d23f5479-39a1-4127-8cdd-0f7cb7e8054d"

#define NS_IJSRUNTIMESERVICE_MOZILLA_1_9_2_IID \
  {0xd23f5479, 0x39a1, 0x4127, \
    { 0x8c, 0xdd, 0x0f, 0x7c, 0xb7, 0xe8, 0x05, 0x4d }}

class NS_NO_VTABLE nsIJSRuntimeService_MOZILLA_1_9_2 : public nsIJSRuntimeService {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IJSRUNTIMESERVICE_MOZILLA_1_9_2_IID)

  /**
     * Register additional GC callback which will run after the
     * standard XPConnect callback.
     */
  /* [noscript, notxpcom] void registerGCCallback (in JSGCCallback func); */
  NS_IMETHOD_(void) RegisterGCCallback(JSGCCallback func) = 0;

  /* [noscript, notxpcom] void unregisterGCCallback (in JSGCCallback func); */
  NS_IMETHOD_(void) UnregisterGCCallback(JSGCCallback func) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIJSRuntimeService_MOZILLA_1_9_2, NS_IJSRUNTIMESERVICE_MOZILLA_1_9_2_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIJSRUNTIMESERVICE_MOZILLA_1_9_2 \
  NS_IMETHOD_(void) RegisterGCCallback(JSGCCallback func); \
  NS_IMETHOD_(void) UnregisterGCCallback(JSGCCallback func); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIJSRUNTIMESERVICE_MOZILLA_1_9_2(_to) \
  NS_IMETHOD_(void) RegisterGCCallback(JSGCCallback func) { return _to RegisterGCCallback(func); } \
  NS_IMETHOD_(void) UnregisterGCCallback(JSGCCallback func) { return _to UnregisterGCCallback(func); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIJSRUNTIMESERVICE_MOZILLA_1_9_2(_to) \
  NS_IMETHOD_(void) RegisterGCCallback(JSGCCallback func) { return !_to ? NS_ERROR_NULL_POINTER : _to->RegisterGCCallback(func); } \
  NS_IMETHOD_(void) UnregisterGCCallback(JSGCCallback func) { return !_to ? NS_ERROR_NULL_POINTER : _to->UnregisterGCCallback(func); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsJSRuntimeService_MOZILLA_1_9_2 : public nsIJSRuntimeService_MOZILLA_1_9_2
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIJSRUNTIMESERVICE_MOZILLA_1_9_2

  nsJSRuntimeService_MOZILLA_1_9_2();

private:
  ~nsJSRuntimeService_MOZILLA_1_9_2();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsJSRuntimeService_MOZILLA_1_9_2, nsIJSRuntimeService_MOZILLA_1_9_2)

nsJSRuntimeService_MOZILLA_1_9_2::nsJSRuntimeService_MOZILLA_1_9_2()
{
  /* member initializers and constructor code */
}

nsJSRuntimeService_MOZILLA_1_9_2::~nsJSRuntimeService_MOZILLA_1_9_2()
{
  /* destructor code */
}

/* [noscript, notxpcom] void registerGCCallback (in JSGCCallback func); */
NS_IMETHODIMP_(void) nsJSRuntimeService_MOZILLA_1_9_2::RegisterGCCallback(JSGCCallback func)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript, notxpcom] void unregisterGCCallback (in JSGCCallback func); */
NS_IMETHODIMP_(void) nsJSRuntimeService_MOZILLA_1_9_2::UnregisterGCCallback(JSGCCallback func)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIJSRuntimeService_h__ */
