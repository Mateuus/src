/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/modules/plugin/base/public/nsIPluginInstance.idl
 */

#ifndef __gen_nsIPluginInstance_h__
#define __gen_nsIPluginInstance_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

#ifndef __gen_nsIPluginStreamListener_h__
#include "nsIPluginStreamListener.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIPluginInstanceOwner; /* forward declaration */

class nsIOutputStream; /* forward declaration */

#include "nsplugindefs.h"
#include "nsStringGlue.h"
struct JSContext;
struct JSObject;
#define NPRUNTIME_JSCLASS_NAME "NPObject JS wrapper class"

/* starting interface:    nsIPluginInstance */
#define NS_IPLUGININSTANCE_IID_STR "67d606f4-1d6d-4fe2-a2d6-10bda65788e1"

#define NS_IPLUGININSTANCE_IID \
  {0x67d606f4, 0x1d6d, 0x4fe2, \
    { 0xa2, 0xd6, 0x10, 0xbd, 0xa6, 0x57, 0x88, 0xe1 }}

class NS_NO_VTABLE nsIPluginInstance : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IPLUGININSTANCE_IID)

  /**
     * Initializes a newly created plugin instance.
     * 
     * @param aOwner - the plugin instance owner
     * @param aMime - the mime type for the instance
     * @result      - NS_OK if this operation was successful
     */
  /* void initialize (in nsIPluginInstanceOwner aOwner, in string aMIMEType); */
  NS_IMETHOD Initialize(nsIPluginInstanceOwner *aOwner, const char *aMIMEType) = 0;

  /**
     * Called to instruct the plugin instance to start. This will be
     * called after the plugin is first created and initialized, and
     * may be called after the plugin is stopped (via the Stop method)
     * if the plugin instance is returned to in the browser window's
     * history.
     *
     * @result - NS_OK if this operation was successful
     */
  /* void start (); */
  NS_IMETHOD Start(void) = 0;

  /**
     * Called to instruct the plugin instance to stop, thereby
     * suspending its state.  This method will be called whenever the
     * browser window goes on to display another page and the page
     * containing the plugin goes into the window's history list.
     *
     * @result - NS_OK if this operation was successful
     */
  /* void stop (); */
  NS_IMETHOD Stop(void) = 0;

  /**
     * Called when the window containing the plugin instance changes.
     *
     * (Corresponds to NPP_SetWindow.)
     *
     * @param aWindow - the plugin window structure
     * @result        - NS_OK if this operation was successful
     */
  /* void setWindow (in nsPluginWindowPtr aWindow); */
  NS_IMETHOD SetWindow(nsPluginWindow * aWindow) = 0;

  /**
     * Called to tell the plugin that the initial src/data stream is
     * ready.  Expects the plugin to return a nsIPluginStreamListener.
     *
     * (Corresponds to NPP_NewStream.)
     *
     * @param aListener - listener the browser will use to give the plugin the data
     * @result          - NS_OK if this operation was successful
     */
  /* void newStreamToPlugin (out nsIPluginStreamListener aListener); */
  NS_IMETHOD NewStreamToPlugin(nsIPluginStreamListener **aListener NS_OUTPARAM) = 0;

  /**
     * This operation is called by the plugin instance when it wishes to send
     * a stream of data to the browser. It constructs a new output stream to which
     * the plugin may send the data. When complete, the Close and Release methods
     * should be called on the output stream.
     *
     * (Corresponds to NPN_NewStream.)
     *
     * @param aType   - MIME type of the stream to create
     * @param aTarget - the target window name to receive the data
     * @param aResult - the resulting output stream
     * @result        - NS_OK if this operation was successful
     */
  /* void newStreamFromPlugin (in string aType, in string aTarget, out nsIOutputStream aResult); */
  NS_IMETHOD NewStreamFromPlugin(const char *aType, const char *aTarget, nsIOutputStream **aResult NS_OUTPARAM) = 0;

  /**
     * Called to instruct the plugin instance to print itself to a printer.
     *
     * (Corresponds to NPP_Print.)
     *
     * @param aPlatformPrint - platform-specific printing information
     * @result               - NS_OK if this operation was successful
     */
  /* void print (in nsPluginPrintPtr aPlatformPrint); */
  NS_IMETHOD Print(nsPluginPrint * aPlatformPrint) = 0;

  /**
     * Returns the value of a variable associated with the plugin instance.
     *
     * @param aVariable - the plugin instance variable to get
     * @param aValue    - the address of where to store the resulting value
     * @result          - NS_OK if this operation was successful
     */
  /* void getValue (in nsPluginInstanceVariable aVariable, in voidPtr aValue); */
  NS_IMETHOD GetValue(nsPluginInstanceVariable aVariable, void * aValue) = 0;

  /**
     * Handles an event.
     *
     * Note that for Unix and Mac the nsPluginEvent structure is different
     * from the old NPEvent structure -- it's no longer the native event
     * record, but is instead a struct. This was done for future extensibility,
     * and so that the Mac could receive the window argument too. For Windows
     * and OS2, it's always been a struct, so there's no change for them.
     *
     * (Corresponds to NPP_HandleEvent.)
     *
     * @param aEvent   - the event to be handled
     * @param aHandled - set to PR_TRUE if event was handled
     * @result - NS_OK if this operation was successful
     */
  /* void handleEvent (in nsPluginEventPtr aEvent, out boolean aHandled); */
  NS_IMETHOD HandleEvent(nsPluginEvent * aEvent, PRBool *aHandled NS_OUTPARAM) = 0;

  /** 
     * Corresponds to NPN_InvalidateRect
     */
  /* void invalidateRect (in nsPluginRectPtr aRect); */
  NS_IMETHOD InvalidateRect(nsPluginRect * aRect) = 0;

  /** 
     * Corresponds to NPN_InvalidateRegion
     */
  /* void invalidateRegion (in nsPluginRegion aRegion); */
  NS_IMETHOD InvalidateRegion(nsPluginRegion aRegion) = 0;

  /** 
     * Corresponds to NPN_ForceRedraw
     */
  /* void forceRedraw (); */
  NS_IMETHOD ForceRedraw(void) = 0;

  /**
     * Returns the MIME type of the plugin instance. 
     *
     * (Corresponds to NPP_New's MIMEType argument.)
     *
     * @param aMIMEType - resulting MIME type
     * @result          - NS_OK if this operation was successful
     */
  /* void getMIMEType ([shared, const] out string aValue); */
  NS_IMETHOD GetMIMEType(const char **aValue NS_OUTPARAM) = 0;

  /**
     * Get the JavaScript context to this plugin instance.
     *
     * @param aJSContext - the resulting JavaScript context
     * @result           - NS_OK if this operation was successful
     */
  /* readonly attribute JSContextPtr JSContext; */
  NS_IMETHOD GetJSContext(JSContext * *aJSContext) = 0;

  /* attribute nsIPluginInstanceOwner owner; */
  NS_IMETHOD GetOwner(nsIPluginInstanceOwner * *aOwner) = 0;
  NS_IMETHOD SetOwner(nsIPluginInstanceOwner * aOwner) = 0;

  /**
     * This operation causes status information to be displayed on the window
     * associated with the plugin instance. 
     *
     * (Corresponds to NPN_Status.)
     *
     * @param aMessage - the status message to display
     * @result         - NS_OK if this operation was successful
     */
  /* void showStatus (in string aMessage); */
  NS_IMETHOD ShowStatus(const char *aMessage) = 0;

  /**
     * Drop our reference to our owner.
     */
  /* void invalidateOwner (); */
  NS_IMETHOD InvalidateOwner(void) = 0;

  /* JSObjectPtr GetJSObject (in JSContextPtr cx); */
  NS_IMETHOD GetJSObject(JSContext * cx, JSObject * *_retval NS_OUTPARAM) = 0;

  /* readonly attribute AString formValue; */
  NS_IMETHOD GetFormValue(nsAString & aFormValue) = 0;

  /* void pushPopupsEnabledState (in boolean aEnabled); */
  NS_IMETHOD PushPopupsEnabledState(PRBool aEnabled) = 0;

  /* void popPopupsEnabledState (); */
  NS_IMETHOD PopPopupsEnabledState(void) = 0;

  /* readonly attribute PRUint16 pluginAPIVersion; */
  NS_IMETHOD GetPluginAPIVersion(PRUint16 *aPluginAPIVersion) = 0;

  /* void defineJavaProperties (); */
  NS_IMETHOD DefineJavaProperties(void) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIPluginInstance, NS_IPLUGININSTANCE_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPLUGININSTANCE \
  NS_IMETHOD Initialize(nsIPluginInstanceOwner *aOwner, const char *aMIMEType); \
  NS_IMETHOD Start(void); \
  NS_IMETHOD Stop(void); \
  NS_IMETHOD SetWindow(nsPluginWindow * aWindow); \
  NS_IMETHOD NewStreamToPlugin(nsIPluginStreamListener **aListener NS_OUTPARAM); \
  NS_IMETHOD NewStreamFromPlugin(const char *aType, const char *aTarget, nsIOutputStream **aResult NS_OUTPARAM); \
  NS_IMETHOD Print(nsPluginPrint * aPlatformPrint); \
  NS_IMETHOD GetValue(nsPluginInstanceVariable aVariable, void * aValue); \
  NS_IMETHOD HandleEvent(nsPluginEvent * aEvent, PRBool *aHandled NS_OUTPARAM); \
  NS_IMETHOD InvalidateRect(nsPluginRect * aRect); \
  NS_IMETHOD InvalidateRegion(nsPluginRegion aRegion); \
  NS_IMETHOD ForceRedraw(void); \
  NS_IMETHOD GetMIMEType(const char **aValue NS_OUTPARAM); \
  NS_IMETHOD GetJSContext(JSContext * *aJSContext); \
  NS_IMETHOD GetOwner(nsIPluginInstanceOwner * *aOwner); \
  NS_IMETHOD SetOwner(nsIPluginInstanceOwner * aOwner); \
  NS_IMETHOD ShowStatus(const char *aMessage); \
  NS_IMETHOD InvalidateOwner(void); \
  NS_IMETHOD GetJSObject(JSContext * cx, JSObject * *_retval NS_OUTPARAM); \
  NS_IMETHOD GetFormValue(nsAString & aFormValue); \
  NS_IMETHOD PushPopupsEnabledState(PRBool aEnabled); \
  NS_IMETHOD PopPopupsEnabledState(void); \
  NS_IMETHOD GetPluginAPIVersion(PRUint16 *aPluginAPIVersion); \
  NS_IMETHOD DefineJavaProperties(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPLUGININSTANCE(_to) \
  NS_IMETHOD Initialize(nsIPluginInstanceOwner *aOwner, const char *aMIMEType) { return _to Initialize(aOwner, aMIMEType); } \
  NS_IMETHOD Start(void) { return _to Start(); } \
  NS_IMETHOD Stop(void) { return _to Stop(); } \
  NS_IMETHOD SetWindow(nsPluginWindow * aWindow) { return _to SetWindow(aWindow); } \
  NS_IMETHOD NewStreamToPlugin(nsIPluginStreamListener **aListener NS_OUTPARAM) { return _to NewStreamToPlugin(aListener); } \
  NS_IMETHOD NewStreamFromPlugin(const char *aType, const char *aTarget, nsIOutputStream **aResult NS_OUTPARAM) { return _to NewStreamFromPlugin(aType, aTarget, aResult); } \
  NS_IMETHOD Print(nsPluginPrint * aPlatformPrint) { return _to Print(aPlatformPrint); } \
  NS_IMETHOD GetValue(nsPluginInstanceVariable aVariable, void * aValue) { return _to GetValue(aVariable, aValue); } \
  NS_IMETHOD HandleEvent(nsPluginEvent * aEvent, PRBool *aHandled NS_OUTPARAM) { return _to HandleEvent(aEvent, aHandled); } \
  NS_IMETHOD InvalidateRect(nsPluginRect * aRect) { return _to InvalidateRect(aRect); } \
  NS_IMETHOD InvalidateRegion(nsPluginRegion aRegion) { return _to InvalidateRegion(aRegion); } \
  NS_IMETHOD ForceRedraw(void) { return _to ForceRedraw(); } \
  NS_IMETHOD GetMIMEType(const char **aValue NS_OUTPARAM) { return _to GetMIMEType(aValue); } \
  NS_IMETHOD GetJSContext(JSContext * *aJSContext) { return _to GetJSContext(aJSContext); } \
  NS_IMETHOD GetOwner(nsIPluginInstanceOwner * *aOwner) { return _to GetOwner(aOwner); } \
  NS_IMETHOD SetOwner(nsIPluginInstanceOwner * aOwner) { return _to SetOwner(aOwner); } \
  NS_IMETHOD ShowStatus(const char *aMessage) { return _to ShowStatus(aMessage); } \
  NS_IMETHOD InvalidateOwner(void) { return _to InvalidateOwner(); } \
  NS_IMETHOD GetJSObject(JSContext * cx, JSObject * *_retval NS_OUTPARAM) { return _to GetJSObject(cx, _retval); } \
  NS_IMETHOD GetFormValue(nsAString & aFormValue) { return _to GetFormValue(aFormValue); } \
  NS_IMETHOD PushPopupsEnabledState(PRBool aEnabled) { return _to PushPopupsEnabledState(aEnabled); } \
  NS_IMETHOD PopPopupsEnabledState(void) { return _to PopPopupsEnabledState(); } \
  NS_IMETHOD GetPluginAPIVersion(PRUint16 *aPluginAPIVersion) { return _to GetPluginAPIVersion(aPluginAPIVersion); } \
  NS_IMETHOD DefineJavaProperties(void) { return _to DefineJavaProperties(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPLUGININSTANCE(_to) \
  NS_IMETHOD Initialize(nsIPluginInstanceOwner *aOwner, const char *aMIMEType) { return !_to ? NS_ERROR_NULL_POINTER : _to->Initialize(aOwner, aMIMEType); } \
  NS_IMETHOD Start(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Start(); } \
  NS_IMETHOD Stop(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Stop(); } \
  NS_IMETHOD SetWindow(nsPluginWindow * aWindow) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetWindow(aWindow); } \
  NS_IMETHOD NewStreamToPlugin(nsIPluginStreamListener **aListener NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->NewStreamToPlugin(aListener); } \
  NS_IMETHOD NewStreamFromPlugin(const char *aType, const char *aTarget, nsIOutputStream **aResult NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->NewStreamFromPlugin(aType, aTarget, aResult); } \
  NS_IMETHOD Print(nsPluginPrint * aPlatformPrint) { return !_to ? NS_ERROR_NULL_POINTER : _to->Print(aPlatformPrint); } \
  NS_IMETHOD GetValue(nsPluginInstanceVariable aVariable, void * aValue) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetValue(aVariable, aValue); } \
  NS_IMETHOD HandleEvent(nsPluginEvent * aEvent, PRBool *aHandled NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->HandleEvent(aEvent, aHandled); } \
  NS_IMETHOD InvalidateRect(nsPluginRect * aRect) { return !_to ? NS_ERROR_NULL_POINTER : _to->InvalidateRect(aRect); } \
  NS_IMETHOD InvalidateRegion(nsPluginRegion aRegion) { return !_to ? NS_ERROR_NULL_POINTER : _to->InvalidateRegion(aRegion); } \
  NS_IMETHOD ForceRedraw(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ForceRedraw(); } \
  NS_IMETHOD GetMIMEType(const char **aValue NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetMIMEType(aValue); } \
  NS_IMETHOD GetJSContext(JSContext * *aJSContext) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetJSContext(aJSContext); } \
  NS_IMETHOD GetOwner(nsIPluginInstanceOwner * *aOwner) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetOwner(aOwner); } \
  NS_IMETHOD SetOwner(nsIPluginInstanceOwner * aOwner) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetOwner(aOwner); } \
  NS_IMETHOD ShowStatus(const char *aMessage) { return !_to ? NS_ERROR_NULL_POINTER : _to->ShowStatus(aMessage); } \
  NS_IMETHOD InvalidateOwner(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->InvalidateOwner(); } \
  NS_IMETHOD GetJSObject(JSContext * cx, JSObject * *_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetJSObject(cx, _retval); } \
  NS_IMETHOD GetFormValue(nsAString & aFormValue) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFormValue(aFormValue); } \
  NS_IMETHOD PushPopupsEnabledState(PRBool aEnabled) { return !_to ? NS_ERROR_NULL_POINTER : _to->PushPopupsEnabledState(aEnabled); } \
  NS_IMETHOD PopPopupsEnabledState(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->PopPopupsEnabledState(); } \
  NS_IMETHOD GetPluginAPIVersion(PRUint16 *aPluginAPIVersion) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPluginAPIVersion(aPluginAPIVersion); } \
  NS_IMETHOD DefineJavaProperties(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->DefineJavaProperties(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsPluginInstance : public nsIPluginInstance
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPLUGININSTANCE

  nsPluginInstance();

private:
  ~nsPluginInstance();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsPluginInstance, nsIPluginInstance)

nsPluginInstance::nsPluginInstance()
{
  /* member initializers and constructor code */
}

nsPluginInstance::~nsPluginInstance()
{
  /* destructor code */
}

/* void initialize (in nsIPluginInstanceOwner aOwner, in string aMIMEType); */
NS_IMETHODIMP nsPluginInstance::Initialize(nsIPluginInstanceOwner *aOwner, const char *aMIMEType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void start (); */
NS_IMETHODIMP nsPluginInstance::Start()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void stop (); */
NS_IMETHODIMP nsPluginInstance::Stop()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void setWindow (in nsPluginWindowPtr aWindow); */
NS_IMETHODIMP nsPluginInstance::SetWindow(nsPluginWindow * aWindow)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void newStreamToPlugin (out nsIPluginStreamListener aListener); */
NS_IMETHODIMP nsPluginInstance::NewStreamToPlugin(nsIPluginStreamListener **aListener NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void newStreamFromPlugin (in string aType, in string aTarget, out nsIOutputStream aResult); */
NS_IMETHODIMP nsPluginInstance::NewStreamFromPlugin(const char *aType, const char *aTarget, nsIOutputStream **aResult NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void print (in nsPluginPrintPtr aPlatformPrint); */
NS_IMETHODIMP nsPluginInstance::Print(nsPluginPrint * aPlatformPrint)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getValue (in nsPluginInstanceVariable aVariable, in voidPtr aValue); */
NS_IMETHODIMP nsPluginInstance::GetValue(nsPluginInstanceVariable aVariable, void * aValue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void handleEvent (in nsPluginEventPtr aEvent, out boolean aHandled); */
NS_IMETHODIMP nsPluginInstance::HandleEvent(nsPluginEvent * aEvent, PRBool *aHandled NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void invalidateRect (in nsPluginRectPtr aRect); */
NS_IMETHODIMP nsPluginInstance::InvalidateRect(nsPluginRect * aRect)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void invalidateRegion (in nsPluginRegion aRegion); */
NS_IMETHODIMP nsPluginInstance::InvalidateRegion(nsPluginRegion aRegion)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void forceRedraw (); */
NS_IMETHODIMP nsPluginInstance::ForceRedraw()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getMIMEType ([shared, const] out string aValue); */
NS_IMETHODIMP nsPluginInstance::GetMIMEType(const char **aValue NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute JSContextPtr JSContext; */
NS_IMETHODIMP nsPluginInstance::GetJSContext(JSContext * *aJSContext)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIPluginInstanceOwner owner; */
NS_IMETHODIMP nsPluginInstance::GetOwner(nsIPluginInstanceOwner * *aOwner)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP nsPluginInstance::SetOwner(nsIPluginInstanceOwner * aOwner)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void showStatus (in string aMessage); */
NS_IMETHODIMP nsPluginInstance::ShowStatus(const char *aMessage)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void invalidateOwner (); */
NS_IMETHODIMP nsPluginInstance::InvalidateOwner()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* JSObjectPtr GetJSObject (in JSContextPtr cx); */
NS_IMETHODIMP nsPluginInstance::GetJSObject(JSContext * cx, JSObject * *_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AString formValue; */
NS_IMETHODIMP nsPluginInstance::GetFormValue(nsAString & aFormValue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void pushPopupsEnabledState (in boolean aEnabled); */
NS_IMETHODIMP nsPluginInstance::PushPopupsEnabledState(PRBool aEnabled)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void popPopupsEnabledState (); */
NS_IMETHODIMP nsPluginInstance::PopPopupsEnabledState()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute PRUint16 pluginAPIVersion; */
NS_IMETHODIMP nsPluginInstance::GetPluginAPIVersion(PRUint16 *aPluginAPIVersion)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void defineJavaProperties (); */
NS_IMETHODIMP nsPluginInstance::DefineJavaProperties()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIPluginInstance_h__ */
