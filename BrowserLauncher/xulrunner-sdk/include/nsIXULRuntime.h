/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/xpcom/system/nsIXULRuntime.idl
 */

#ifndef __gen_nsIXULRuntime_h__
#define __gen_nsIXULRuntime_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIXULRuntime */
#define NS_IXULRUNTIME_IID_STR "17311145-97da-49eb-b984-965bdee8879c"

#define NS_IXULRUNTIME_IID \
  {0x17311145, 0x97da, 0x49eb, \
    { 0xb9, 0x84, 0x96, 0x5b, 0xde, 0xe8, 0x87, 0x9c }}

/**
 * Provides information about the XUL runtime.
 * @status UNSTABLE - This interface is not frozen and will probably change in
 *                    future releases. If you need this functionality to be
 *                    stable/frozen, please contact Benjamin Smedberg.
 */
class NS_NO_VTABLE NS_SCRIPTABLE nsIXULRuntime : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IXULRUNTIME_IID)

  /**
   * Whether the application was launched in safe mode.
   */
  /* readonly attribute boolean inSafeMode; */
  NS_SCRIPTABLE NS_IMETHOD GetInSafeMode(PRBool *aInSafeMode) = 0;

  /**
   * Whether to write console errors to a log file. If a component
   * encounters startup errors that might prevent the app from showing
   * proper UI, it should set this flag to "true".
   */
  /* attribute boolean logConsoleErrors; */
  NS_SCRIPTABLE NS_IMETHOD GetLogConsoleErrors(PRBool *aLogConsoleErrors) = 0;
  NS_SCRIPTABLE NS_IMETHOD SetLogConsoleErrors(PRBool aLogConsoleErrors) = 0;

  /**
   * A string tag identifying the current operating system. This is taken
   * from the OS_TARGET configure variable. It will always be available.
   */
  /* readonly attribute AUTF8String OS; */
  NS_SCRIPTABLE NS_IMETHOD GetOS(nsACString & aOS) = 0;

  /**
   * A string tag identifying the binary ABI of the current processor and
   * compiler vtable. This is taken from the TARGET_XPCOM_ABI configure
   * variable. It may not be available on all platforms, especially
   * unusual processor or compiler combinations.
   *
   * The result takes the form <processor>-<compilerABI>, for example:
   *   x86-msvc
   *   ppc-gcc3
   *
   * This value should almost always be used in combination with "OS".
   *
   * @throw NS_ERROR_NOT_AVAILABLE if not available.
   */
  /* readonly attribute AUTF8String XPCOMABI; */
  NS_SCRIPTABLE NS_IMETHOD GetXPCOMABI(nsACString & aXPCOMABI) = 0;

  /**
   * A string tag identifying the target widget toolkit in use.
   * This is taken from the MOZ_WIDGET_TOOLKIT configure variable.
   */
  /* readonly attribute AUTF8String widgetToolkit; */
  NS_SCRIPTABLE NS_IMETHOD GetWidgetToolkit(nsACString & aWidgetToolkit) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIXULRuntime, NS_IXULRUNTIME_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIXULRUNTIME \
  NS_SCRIPTABLE NS_IMETHOD GetInSafeMode(PRBool *aInSafeMode); \
  NS_SCRIPTABLE NS_IMETHOD GetLogConsoleErrors(PRBool *aLogConsoleErrors); \
  NS_SCRIPTABLE NS_IMETHOD SetLogConsoleErrors(PRBool aLogConsoleErrors); \
  NS_SCRIPTABLE NS_IMETHOD GetOS(nsACString & aOS); \
  NS_SCRIPTABLE NS_IMETHOD GetXPCOMABI(nsACString & aXPCOMABI); \
  NS_SCRIPTABLE NS_IMETHOD GetWidgetToolkit(nsACString & aWidgetToolkit); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIXULRUNTIME(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetInSafeMode(PRBool *aInSafeMode) { return _to GetInSafeMode(aInSafeMode); } \
  NS_SCRIPTABLE NS_IMETHOD GetLogConsoleErrors(PRBool *aLogConsoleErrors) { return _to GetLogConsoleErrors(aLogConsoleErrors); } \
  NS_SCRIPTABLE NS_IMETHOD SetLogConsoleErrors(PRBool aLogConsoleErrors) { return _to SetLogConsoleErrors(aLogConsoleErrors); } \
  NS_SCRIPTABLE NS_IMETHOD GetOS(nsACString & aOS) { return _to GetOS(aOS); } \
  NS_SCRIPTABLE NS_IMETHOD GetXPCOMABI(nsACString & aXPCOMABI) { return _to GetXPCOMABI(aXPCOMABI); } \
  NS_SCRIPTABLE NS_IMETHOD GetWidgetToolkit(nsACString & aWidgetToolkit) { return _to GetWidgetToolkit(aWidgetToolkit); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIXULRUNTIME(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetInSafeMode(PRBool *aInSafeMode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetInSafeMode(aInSafeMode); } \
  NS_SCRIPTABLE NS_IMETHOD GetLogConsoleErrors(PRBool *aLogConsoleErrors) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLogConsoleErrors(aLogConsoleErrors); } \
  NS_SCRIPTABLE NS_IMETHOD SetLogConsoleErrors(PRBool aLogConsoleErrors) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetLogConsoleErrors(aLogConsoleErrors); } \
  NS_SCRIPTABLE NS_IMETHOD GetOS(nsACString & aOS) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOS(aOS); } \
  NS_SCRIPTABLE NS_IMETHOD GetXPCOMABI(nsACString & aXPCOMABI) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetXPCOMABI(aXPCOMABI); } \
  NS_SCRIPTABLE NS_IMETHOD GetWidgetToolkit(nsACString & aWidgetToolkit) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetWidgetToolkit(aWidgetToolkit); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsXULRuntime : public nsIXULRuntime
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXULRUNTIME

  nsXULRuntime();

private:
  ~nsXULRuntime();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsXULRuntime, nsIXULRuntime)

nsXULRuntime::nsXULRuntime()
{
  /* member initializers and constructor code */
}

nsXULRuntime::~nsXULRuntime()
{
  /* destructor code */
}

/* readonly attribute boolean inSafeMode; */
NS_IMETHODIMP nsXULRuntime::GetInSafeMode(PRBool *aInSafeMode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute boolean logConsoleErrors; */
NS_IMETHODIMP nsXULRuntime::GetLogConsoleErrors(PRBool *aLogConsoleErrors)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsXULRuntime::SetLogConsoleErrors(PRBool aLogConsoleErrors)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AUTF8String OS; */
NS_IMETHODIMP nsXULRuntime::GetOS(nsACString & aOS)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AUTF8String XPCOMABI; */
NS_IMETHODIMP nsXULRuntime::GetXPCOMABI(nsACString & aXPCOMABI)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AUTF8String widgetToolkit; */
NS_IMETHODIMP nsXULRuntime::GetWidgetToolkit(nsACString & aWidgetToolkit)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIXULRuntime_h__ */
