/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/widget/public/nsIWinTaskbar.idl
 */

#ifndef __gen_nsIWinTaskbar_h__
#define __gen_nsIWinTaskbar_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIBaseWindow_h__
#include "nsIBaseWindow.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDocShell; /* forward declaration */

class nsITaskbarTabPreview; /* forward declaration */

class nsITaskbarWindowPreview; /* forward declaration */

class nsITaskbarPreviewController; /* forward declaration */

class nsITaskbarProgress; /* forward declaration */


/* starting interface:    nsIWinTaskbar */
#define NS_IWINTASKBAR_IID_STR "0a3abac7-35b7-4b50-8381-7dbc3c55b061"

#define NS_IWINTASKBAR_IID \
  {0x0a3abac7, 0x35b7, 0x4b50, \
    { 0x83, 0x81, 0x7d, 0xbc, 0x3c, 0x55, 0xb0, 0x61 }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIWinTaskbar : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IWINTASKBAR_IID)

  /**
   * Returns true if the taskbar service is available. This value does not
   * change during runtime.
   */
  /* readonly attribute boolean available; */
  NS_SCRIPTABLE NS_IMETHOD GetAvailable(PRBool *aAvailable) = 0;

  /**
   * Creates a taskbar preview. The docshell is used to find the toplevel window.
   * See the documentation for nsITaskbarTabPreview for more information.
   */
  /* nsITaskbarTabPreview createTaskbarTabPreview (in nsIDocShell shell, in nsITaskbarPreviewController controller); */
  NS_SCRIPTABLE NS_IMETHOD CreateTaskbarTabPreview(nsIDocShell *shell, nsITaskbarPreviewController *controller, nsITaskbarTabPreview **_retval NS_OUTPARAM) = 0;

  /**
   * Gets the taskbar preview for a window. The docshell is used to find the
   * toplevel window. See the documentation for nsITaskbarTabPreview for more
   * information.
   *
   * Note: to implement custom drawing or buttons, a controller is required.
   */
  /* nsITaskbarWindowPreview getTaskbarWindowPreview (in nsIDocShell shell); */
  NS_SCRIPTABLE NS_IMETHOD GetTaskbarWindowPreview(nsIDocShell *shell, nsITaskbarWindowPreview **_retval NS_OUTPARAM) = 0;

  /**
   * Gets the taskbar progress for a window. The docshell is used to find the
   * toplevel window. See the documentation for nsITaskbarProgress for more
   * information.
   */
  /* nsITaskbarProgress getTaskbarProgress (in nsIDocShell shell); */
  NS_SCRIPTABLE NS_IMETHOD GetTaskbarProgress(nsIDocShell *shell, nsITaskbarProgress **_retval NS_OUTPARAM) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIWinTaskbar, NS_IWINTASKBAR_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIWINTASKBAR \
  NS_SCRIPTABLE NS_IMETHOD GetAvailable(PRBool *aAvailable); \
  NS_SCRIPTABLE NS_IMETHOD CreateTaskbarTabPreview(nsIDocShell *shell, nsITaskbarPreviewController *controller, nsITaskbarTabPreview **_retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD GetTaskbarWindowPreview(nsIDocShell *shell, nsITaskbarWindowPreview **_retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD GetTaskbarProgress(nsIDocShell *shell, nsITaskbarProgress **_retval NS_OUTPARAM); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIWINTASKBAR(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetAvailable(PRBool *aAvailable) { return _to GetAvailable(aAvailable); } \
  NS_SCRIPTABLE NS_IMETHOD CreateTaskbarTabPreview(nsIDocShell *shell, nsITaskbarPreviewController *controller, nsITaskbarTabPreview **_retval NS_OUTPARAM) { return _to CreateTaskbarTabPreview(shell, controller, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD GetTaskbarWindowPreview(nsIDocShell *shell, nsITaskbarWindowPreview **_retval NS_OUTPARAM) { return _to GetTaskbarWindowPreview(shell, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD GetTaskbarProgress(nsIDocShell *shell, nsITaskbarProgress **_retval NS_OUTPARAM) { return _to GetTaskbarProgress(shell, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIWINTASKBAR(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetAvailable(PRBool *aAvailable) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetAvailable(aAvailable); } \
  NS_SCRIPTABLE NS_IMETHOD CreateTaskbarTabPreview(nsIDocShell *shell, nsITaskbarPreviewController *controller, nsITaskbarTabPreview **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateTaskbarTabPreview(shell, controller, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD GetTaskbarWindowPreview(nsIDocShell *shell, nsITaskbarWindowPreview **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTaskbarWindowPreview(shell, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD GetTaskbarProgress(nsIDocShell *shell, nsITaskbarProgress **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetTaskbarProgress(shell, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsWinTaskbar : public nsIWinTaskbar
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIWINTASKBAR

  nsWinTaskbar();

private:
  ~nsWinTaskbar();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsWinTaskbar, nsIWinTaskbar)

nsWinTaskbar::nsWinTaskbar()
{
  /* member initializers and constructor code */
}

nsWinTaskbar::~nsWinTaskbar()
{
  /* destructor code */
}

/* readonly attribute boolean available; */
NS_IMETHODIMP nsWinTaskbar::GetAvailable(PRBool *aAvailable)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsITaskbarTabPreview createTaskbarTabPreview (in nsIDocShell shell, in nsITaskbarPreviewController controller); */
NS_IMETHODIMP nsWinTaskbar::CreateTaskbarTabPreview(nsIDocShell *shell, nsITaskbarPreviewController *controller, nsITaskbarTabPreview **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsITaskbarWindowPreview getTaskbarWindowPreview (in nsIDocShell shell); */
NS_IMETHODIMP nsWinTaskbar::GetTaskbarWindowPreview(nsIDocShell *shell, nsITaskbarWindowPreview **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsITaskbarProgress getTaskbarProgress (in nsIDocShell shell); */
NS_IMETHODIMP nsWinTaskbar::GetTaskbarProgress(nsIDocShell *shell, nsITaskbarProgress **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIWinTaskbar_h__ */
