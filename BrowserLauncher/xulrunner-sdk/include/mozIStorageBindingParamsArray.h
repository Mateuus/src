/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/storage/public/mozIStorageBindingParamsArray.idl
 */

#ifndef __gen_mozIStorageBindingParamsArray_h__
#define __gen_mozIStorageBindingParamsArray_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class mozIStorageBindingParams; /* forward declaration */


/* starting interface:    mozIStorageBindingParamsArray */
#define MOZISTORAGEBINDINGPARAMSARRAY_IID_STR "e676e1a3-1dc6-4802-ac03-291fa9de7f93"

#define MOZISTORAGEBINDINGPARAMSARRAY_IID \
  {0xe676e1a3, 0x1dc6, 0x4802, \
    { 0xac, 0x03, 0x29, 0x1f, 0xa9, 0xde, 0x7f, 0x93 }}

class NS_NO_VTABLE NS_SCRIPTABLE mozIStorageBindingParamsArray : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(MOZISTORAGEBINDINGPARAMSARRAY_IID)

  /**
   * Creates a new mozIStorageBindingParams object that can be added to this
   * array.
   *
   * @returns a mozIStorageBindingParams object that can be used to specify
   *          parameters that need to be bound.
   */
  /* mozIStorageBindingParams newBindingParams (); */
  NS_SCRIPTABLE NS_IMETHOD NewBindingParams(mozIStorageBindingParams **_retval NS_OUTPARAM) = 0;

  /**
   * Adds the parameters to the end of this array.
   *
   * @param aParameters
   *        The parameters to add to this array.
   */
  /* void addParams (in mozIStorageBindingParams aParameters); */
  NS_SCRIPTABLE NS_IMETHOD AddParams(mozIStorageBindingParams *aParameters) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(mozIStorageBindingParamsArray, MOZISTORAGEBINDINGPARAMSARRAY_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_MOZISTORAGEBINDINGPARAMSARRAY \
  NS_SCRIPTABLE NS_IMETHOD NewBindingParams(mozIStorageBindingParams **_retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD AddParams(mozIStorageBindingParams *aParameters); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_MOZISTORAGEBINDINGPARAMSARRAY(_to) \
  NS_SCRIPTABLE NS_IMETHOD NewBindingParams(mozIStorageBindingParams **_retval NS_OUTPARAM) { return _to NewBindingParams(_retval); } \
  NS_SCRIPTABLE NS_IMETHOD AddParams(mozIStorageBindingParams *aParameters) { return _to AddParams(aParameters); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_MOZISTORAGEBINDINGPARAMSARRAY(_to) \
  NS_SCRIPTABLE NS_IMETHOD NewBindingParams(mozIStorageBindingParams **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->NewBindingParams(_retval); } \
  NS_SCRIPTABLE NS_IMETHOD AddParams(mozIStorageBindingParams *aParameters) { return !_to ? NS_ERROR_NULL_POINTER : _to->AddParams(aParameters); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public mozIStorageBindingParamsArray
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_MOZISTORAGEBINDINGPARAMSARRAY

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, mozIStorageBindingParamsArray)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* mozIStorageBindingParams newBindingParams (); */
NS_IMETHODIMP _MYCLASS_::NewBindingParams(mozIStorageBindingParams **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void addParams (in mozIStorageBindingParams aParameters); */
NS_IMETHODIMP _MYCLASS_::AddParams(mozIStorageBindingParams *aParameters)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_mozIStorageBindingParamsArray_h__ */
