/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/xpcom/threads/nsIProcess.idl
 */

#ifndef __gen_nsIProcess_h__
#define __gen_nsIProcess_h__


#ifndef __gen_nsIFile_h__
#include "nsIFile.h"
#endif

#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIObserver; /* forward declaration */


/* starting interface:    nsIProcess */
#define NS_IPROCESS_IID_STR "3c09efde-5f49-42b8-8520-57be02a5f84e"

#define NS_IPROCESS_IID \
  {0x3c09efde, 0x5f49, 0x42b8, \
    { 0x85, 0x20, 0x57, 0xbe, 0x02, 0xa5, 0xf8, 0x4e }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIProcess : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IPROCESS_IID)

  /**
   * Initialises the process with an executable to be run. Call the run method
   * to run the executable.
   * @param executable The executable to run.
   */
  /* void init (in nsIFile executable); */
  NS_SCRIPTABLE NS_IMETHOD Init(nsIFile *executable) = 0;

  /**
   * Kills the running process. After exiting the process will either have
   * been killed or a failure will have been returned.
   */
  /* void kill (); */
  NS_SCRIPTABLE NS_IMETHOD Kill(void) = 0;

  /**
   * Executes the file this object was initialized with
   * @param blocking   Whether to wait until the process terminates before
                       returning or not.
   * @param args       An array of arguments to pass to the process in the
   *                   native character set.
   * @param count      The length of the args array.
   */
  /* void run (in boolean blocking, [array, size_is (count)] in string args, in unsigned long count); */
  NS_SCRIPTABLE NS_IMETHOD Run(PRBool blocking, const char **args, PRUint32 count) = 0;

  /**
   * Executes the file this object was initialized with optionally calling
   * an observer after the process has finished running.
   * @param args       An array of arguments to pass to the process in the
   *                   native character set.
   * @param count      The length of the args array.
   * @param observer   An observer to notify when the process has completed. It
   *                   will receive this process instance as the subject and
   *                   "process-finished" or "process-failed" as the topic. The
   *                   observer will be notified on the main thread.
   * @param holdWeak   Whether to use a weak reference to hold the observer.
   */
  /* void runAsync ([array, size_is (count)] in string args, in unsigned long count, [optional] in nsIObserver observer, [optional] in boolean holdWeak); */
  NS_SCRIPTABLE NS_IMETHOD RunAsync(const char **args, PRUint32 count, nsIObserver *observer, PRBool holdWeak) = 0;

  /**
   * The process identifier of the currently running process. This will only
   * be available after the process has started and may not be available on
   * some platforms.
   */
  /* readonly attribute unsigned long pid; */
  NS_SCRIPTABLE NS_IMETHOD GetPid(PRUint32 *aPid) = 0;

  /**
   * The exit value of the process. This is only valid after the process has
   * exited.
   */
  /* readonly attribute long exitValue; */
  NS_SCRIPTABLE NS_IMETHOD GetExitValue(PRInt32 *aExitValue) = 0;

  /**
   * Returns whether the process is currently running or not.
   */
  /* readonly attribute boolean isRunning; */
  NS_SCRIPTABLE NS_IMETHOD GetIsRunning(PRBool *aIsRunning) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIProcess, NS_IPROCESS_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIPROCESS \
  NS_SCRIPTABLE NS_IMETHOD Init(nsIFile *executable); \
  NS_SCRIPTABLE NS_IMETHOD Kill(void); \
  NS_SCRIPTABLE NS_IMETHOD Run(PRBool blocking, const char **args, PRUint32 count); \
  NS_SCRIPTABLE NS_IMETHOD RunAsync(const char **args, PRUint32 count, nsIObserver *observer, PRBool holdWeak); \
  NS_SCRIPTABLE NS_IMETHOD GetPid(PRUint32 *aPid); \
  NS_SCRIPTABLE NS_IMETHOD GetExitValue(PRInt32 *aExitValue); \
  NS_SCRIPTABLE NS_IMETHOD GetIsRunning(PRBool *aIsRunning); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIPROCESS(_to) \
  NS_SCRIPTABLE NS_IMETHOD Init(nsIFile *executable) { return _to Init(executable); } \
  NS_SCRIPTABLE NS_IMETHOD Kill(void) { return _to Kill(); } \
  NS_SCRIPTABLE NS_IMETHOD Run(PRBool blocking, const char **args, PRUint32 count) { return _to Run(blocking, args, count); } \
  NS_SCRIPTABLE NS_IMETHOD RunAsync(const char **args, PRUint32 count, nsIObserver *observer, PRBool holdWeak) { return _to RunAsync(args, count, observer, holdWeak); } \
  NS_SCRIPTABLE NS_IMETHOD GetPid(PRUint32 *aPid) { return _to GetPid(aPid); } \
  NS_SCRIPTABLE NS_IMETHOD GetExitValue(PRInt32 *aExitValue) { return _to GetExitValue(aExitValue); } \
  NS_SCRIPTABLE NS_IMETHOD GetIsRunning(PRBool *aIsRunning) { return _to GetIsRunning(aIsRunning); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIPROCESS(_to) \
  NS_SCRIPTABLE NS_IMETHOD Init(nsIFile *executable) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(executable); } \
  NS_SCRIPTABLE NS_IMETHOD Kill(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Kill(); } \
  NS_SCRIPTABLE NS_IMETHOD Run(PRBool blocking, const char **args, PRUint32 count) { return !_to ? NS_ERROR_NULL_POINTER : _to->Run(blocking, args, count); } \
  NS_SCRIPTABLE NS_IMETHOD RunAsync(const char **args, PRUint32 count, nsIObserver *observer, PRBool holdWeak) { return !_to ? NS_ERROR_NULL_POINTER : _to->RunAsync(args, count, observer, holdWeak); } \
  NS_SCRIPTABLE NS_IMETHOD GetPid(PRUint32 *aPid) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPid(aPid); } \
  NS_SCRIPTABLE NS_IMETHOD GetExitValue(PRInt32 *aExitValue) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetExitValue(aExitValue); } \
  NS_SCRIPTABLE NS_IMETHOD GetIsRunning(PRBool *aIsRunning) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetIsRunning(aIsRunning); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsProcess : public nsIProcess
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPROCESS

  nsProcess();

private:
  ~nsProcess();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsProcess, nsIProcess)

nsProcess::nsProcess()
{
  /* member initializers and constructor code */
}

nsProcess::~nsProcess()
{
  /* destructor code */
}

/* void init (in nsIFile executable); */
NS_IMETHODIMP nsProcess::Init(nsIFile *executable)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void kill (); */
NS_IMETHODIMP nsProcess::Kill()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void run (in boolean blocking, [array, size_is (count)] in string args, in unsigned long count); */
NS_IMETHODIMP nsProcess::Run(PRBool blocking, const char **args, PRUint32 count)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void runAsync ([array, size_is (count)] in string args, in unsigned long count, [optional] in nsIObserver observer, [optional] in boolean holdWeak); */
NS_IMETHODIMP nsProcess::RunAsync(const char **args, PRUint32 count, nsIObserver *observer, PRBool holdWeak)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long pid; */
NS_IMETHODIMP nsProcess::GetPid(PRUint32 *aPid)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute long exitValue; */
NS_IMETHODIMP nsProcess::GetExitValue(PRInt32 *aExitValue)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute boolean isRunning; */
NS_IMETHODIMP nsProcess::GetIsRunning(PRBool *aIsRunning)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

#define NS_PROCESS_CONTRACTID "@mozilla.org/process/util;1"
#define NS_PROCESS_CLASSNAME "Process Specification"

#endif /* __gen_nsIProcess_h__ */
