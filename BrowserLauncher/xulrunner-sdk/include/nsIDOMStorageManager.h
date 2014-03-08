/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/dom/interfaces/storage/nsIDOMStorageManager.idl
 */

#ifndef __gen_nsIDOMStorageManager_h__
#define __gen_nsIDOMStorageManager_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIDOMStorage; /* forward declaration */

class nsIPrincipal; /* forward declaration */


/* starting interface:    nsIDOMStorageManager */
#define NS_IDOMSTORAGEMANAGER_IID_STR "c8e54beb-48f3-4538-a0ce-d6229f4d8f45"

#define NS_IDOMSTORAGEMANAGER_IID \
  {0xc8e54beb, 0x48f3, 0x4538, \
    { 0xa0, 0xce, 0xd6, 0x22, 0x9f, 0x4d, 0x8f, 0x45 }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIDOMStorageManager : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMSTORAGEMANAGER_IID)

  /**
   * Return the amount of disk space used by a domain.  Usage is checked
   * against the domain of the page that set the key (the owner domain), not
   * the domain of the storage object.
   *
   * @param aOwnerDomain The domain to check.
   * @returns the space usage of the domain, in bytes.
   */
  /* long getUsage (in AString aOwnerDomain); */
  NS_SCRIPTABLE NS_IMETHOD GetUsage(const nsAString & aOwnerDomain, PRInt32 *_retval NS_OUTPARAM) = 0;

  /**
   * Clear keys owned by offline applications.  All data owned by a domain
   * with the "offline-app" permission will be removed from the database.
   */
  /* void clearOfflineApps (); */
  NS_SCRIPTABLE NS_IMETHOD ClearOfflineApps(void) = 0;

  /**
   * Returns instance of localStorage object for aURI's origin.
   * This method ensures there is always only a single instance
   * for a single origin.
   */
  /* nsIDOMStorage getLocalStorageForPrincipal (in nsIPrincipal aPrincipal); */
  NS_SCRIPTABLE NS_IMETHOD GetLocalStorageForPrincipal(nsIPrincipal *aPrincipal, nsIDOMStorage **_retval NS_OUTPARAM) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMStorageManager, NS_IDOMSTORAGEMANAGER_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMSTORAGEMANAGER \
  NS_SCRIPTABLE NS_IMETHOD GetUsage(const nsAString & aOwnerDomain, PRInt32 *_retval NS_OUTPARAM); \
  NS_SCRIPTABLE NS_IMETHOD ClearOfflineApps(void); \
  NS_SCRIPTABLE NS_IMETHOD GetLocalStorageForPrincipal(nsIPrincipal *aPrincipal, nsIDOMStorage **_retval NS_OUTPARAM); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMSTORAGEMANAGER(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetUsage(const nsAString & aOwnerDomain, PRInt32 *_retval NS_OUTPARAM) { return _to GetUsage(aOwnerDomain, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD ClearOfflineApps(void) { return _to ClearOfflineApps(); } \
  NS_SCRIPTABLE NS_IMETHOD GetLocalStorageForPrincipal(nsIPrincipal *aPrincipal, nsIDOMStorage **_retval NS_OUTPARAM) { return _to GetLocalStorageForPrincipal(aPrincipal, _retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMSTORAGEMANAGER(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetUsage(const nsAString & aOwnerDomain, PRInt32 *_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetUsage(aOwnerDomain, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD ClearOfflineApps(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ClearOfflineApps(); } \
  NS_SCRIPTABLE NS_IMETHOD GetLocalStorageForPrincipal(nsIPrincipal *aPrincipal, nsIDOMStorage **_retval NS_OUTPARAM) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetLocalStorageForPrincipal(aPrincipal, _retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMStorageManager : public nsIDOMStorageManager
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMSTORAGEMANAGER

  nsDOMStorageManager();

private:
  ~nsDOMStorageManager();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMStorageManager, nsIDOMStorageManager)

nsDOMStorageManager::nsDOMStorageManager()
{
  /* member initializers and constructor code */
}

nsDOMStorageManager::~nsDOMStorageManager()
{
  /* destructor code */
}

/* long getUsage (in AString aOwnerDomain); */
NS_IMETHODIMP nsDOMStorageManager::GetUsage(const nsAString & aOwnerDomain, PRInt32 *_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void clearOfflineApps (); */
NS_IMETHODIMP nsDOMStorageManager::ClearOfflineApps()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDOMStorage getLocalStorageForPrincipal (in nsIPrincipal aPrincipal); */
NS_IMETHODIMP nsDOMStorageManager::GetLocalStorageForPrincipal(nsIPrincipal *aPrincipal, nsIDOMStorage **_retval NS_OUTPARAM)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMStorageManager_h__ */
