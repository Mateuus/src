/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/netwerk/base/public/nsIStreamListenerTee_1_9_2.idl
 */

#ifndef __gen_nsIStreamListenerTee_1_9_2_h__
#define __gen_nsIStreamListenerTee_1_9_2_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIOutputStream; /* forward declaration */

class nsIRequestObserver; /* forward declaration */

class nsIStreamListener; /* forward declaration */


/* starting interface:    nsIStreamListenerTee_1_9_2 */
#define NS_ISTREAMLISTENERTEE_1_9_2_IID_STR "18b1e3d7-8083-4b19-a077-82ceea6fd296"

#define NS_ISTREAMLISTENERTEE_1_9_2_IID \
  {0x18b1e3d7, 0x8083, 0x4b19, \
    { 0xa0, 0x77, 0x82, 0xce, 0xea, 0x6f, 0xd2, 0x96 }}

/**
 * As data "flows" into a stream listener tee, it is copied to the output stream
 * and then forwarded to the real listener.
 */
class NS_NO_VTABLE NS_SCRIPTABLE nsIStreamListenerTee_1_9_2 : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ISTREAMLISTENERTEE_1_9_2_IID)

  /** 
     * Initalize the tee.
     *
     * @param listener
     *    the original listener the tee will propagate onStartRequest,
     *    onDataAvailable and onStopRequest notifications to, exceptions from 
     *    the listener will be propagated back to the channel
     * @param sink
     *    the stream the data coming from the channel will be written to,
     *    should be blocking
     * @param requestObserver
     *    optional parameter, listener that gets only onStartRequest and
     *    onStopRequest notifications; exceptions threw within this optional
     *    observer are also propagated to the channel, but exceptions from
     *    the original listener (listener parameter) are privileged 
     */
  /* void initWithObserver (in nsIStreamListener listener, in nsIOutputStream sink, [optional] in nsIRequestObserver requestObserver); */
  NS_SCRIPTABLE NS_IMETHOD InitWithObserver(nsIStreamListener *listener, nsIOutputStream *sink, nsIRequestObserver *requestObserver) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIStreamListenerTee_1_9_2, NS_ISTREAMLISTENERTEE_1_9_2_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISTREAMLISTENERTEE_1_9_2 \
  NS_SCRIPTABLE NS_IMETHOD InitWithObserver(nsIStreamListener *listener, nsIOutputStream *sink, nsIRequestObserver *requestObserver); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISTREAMLISTENERTEE_1_9_2(_to) \
  NS_SCRIPTABLE NS_IMETHOD InitWithObserver(nsIStreamListener *listener, nsIOutputStream *sink, nsIRequestObserver *requestObserver) { return _to InitWithObserver(listener, sink, requestObserver); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISTREAMLISTENERTEE_1_9_2(_to) \
  NS_SCRIPTABLE NS_IMETHOD InitWithObserver(nsIStreamListener *listener, nsIOutputStream *sink, nsIRequestObserver *requestObserver) { return !_to ? NS_ERROR_NULL_POINTER : _to->InitWithObserver(listener, sink, requestObserver); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsStreamListenerTee_1_9_2 : public nsIStreamListenerTee_1_9_2
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISTREAMLISTENERTEE_1_9_2

  nsStreamListenerTee_1_9_2();

private:
  ~nsStreamListenerTee_1_9_2();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsStreamListenerTee_1_9_2, nsIStreamListenerTee_1_9_2)

nsStreamListenerTee_1_9_2::nsStreamListenerTee_1_9_2()
{
  /* member initializers and constructor code */
}

nsStreamListenerTee_1_9_2::~nsStreamListenerTee_1_9_2()
{
  /* destructor code */
}

/* void initWithObserver (in nsIStreamListener listener, in nsIOutputStream sink, [optional] in nsIRequestObserver requestObserver); */
NS_IMETHODIMP nsStreamListenerTee_1_9_2::InitWithObserver(nsIStreamListener *listener, nsIOutputStream *sink, nsIRequestObserver *requestObserver)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIStreamListenerTee_1_9_2_h__ */
