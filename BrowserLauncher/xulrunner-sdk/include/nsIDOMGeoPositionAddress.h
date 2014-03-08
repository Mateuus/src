/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/dom/interfaces/geolocation/nsIDOMGeoPositionAddress.idl
 */

#ifndef __gen_nsIDOMGeoPositionAddress_h__
#define __gen_nsIDOMGeoPositionAddress_h__


#ifndef __gen_domstubs_h__
#include "domstubs.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIDOMGeoPositionAddress */
#define NS_IDOMGEOPOSITIONADDRESS_IID_STR "0df49c5c-9845-42f9-a76c-62e09c110986"

#define NS_IDOMGEOPOSITIONADDRESS_IID \
  {0x0df49c5c, 0x9845, 0x42f9, \
    { 0xa7, 0x6c, 0x62, 0xe0, 0x9c, 0x11, 0x09, 0x86 }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIDOMGeoPositionAddress : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMGEOPOSITIONADDRESS_IID)

  /* readonly attribute DOMString streetNumber; */
  NS_SCRIPTABLE NS_IMETHOD GetStreetNumber(nsAString & aStreetNumber) = 0;

  /* readonly attribute DOMString street; */
  NS_SCRIPTABLE NS_IMETHOD GetStreet(nsAString & aStreet) = 0;

  /* readonly attribute DOMString premises; */
  NS_SCRIPTABLE NS_IMETHOD GetPremises(nsAString & aPremises) = 0;

  /* readonly attribute DOMString city; */
  NS_SCRIPTABLE NS_IMETHOD GetCity(nsAString & aCity) = 0;

  /* readonly attribute DOMString county; */
  NS_SCRIPTABLE NS_IMETHOD GetCounty(nsAString & aCounty) = 0;

  /* readonly attribute DOMString region; */
  NS_SCRIPTABLE NS_IMETHOD GetRegion(nsAString & aRegion) = 0;

  /* readonly attribute DOMString country; */
  NS_SCRIPTABLE NS_IMETHOD GetCountry(nsAString & aCountry) = 0;

  /* readonly attribute DOMString countryCode; */
  NS_SCRIPTABLE NS_IMETHOD GetCountryCode(nsAString & aCountryCode) = 0;

  /* readonly attribute DOMString postalCode; */
  NS_SCRIPTABLE NS_IMETHOD GetPostalCode(nsAString & aPostalCode) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMGeoPositionAddress, NS_IDOMGEOPOSITIONADDRESS_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMGEOPOSITIONADDRESS \
  NS_SCRIPTABLE NS_IMETHOD GetStreetNumber(nsAString & aStreetNumber); \
  NS_SCRIPTABLE NS_IMETHOD GetStreet(nsAString & aStreet); \
  NS_SCRIPTABLE NS_IMETHOD GetPremises(nsAString & aPremises); \
  NS_SCRIPTABLE NS_IMETHOD GetCity(nsAString & aCity); \
  NS_SCRIPTABLE NS_IMETHOD GetCounty(nsAString & aCounty); \
  NS_SCRIPTABLE NS_IMETHOD GetRegion(nsAString & aRegion); \
  NS_SCRIPTABLE NS_IMETHOD GetCountry(nsAString & aCountry); \
  NS_SCRIPTABLE NS_IMETHOD GetCountryCode(nsAString & aCountryCode); \
  NS_SCRIPTABLE NS_IMETHOD GetPostalCode(nsAString & aPostalCode); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMGEOPOSITIONADDRESS(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetStreetNumber(nsAString & aStreetNumber) { return _to GetStreetNumber(aStreetNumber); } \
  NS_SCRIPTABLE NS_IMETHOD GetStreet(nsAString & aStreet) { return _to GetStreet(aStreet); } \
  NS_SCRIPTABLE NS_IMETHOD GetPremises(nsAString & aPremises) { return _to GetPremises(aPremises); } \
  NS_SCRIPTABLE NS_IMETHOD GetCity(nsAString & aCity) { return _to GetCity(aCity); } \
  NS_SCRIPTABLE NS_IMETHOD GetCounty(nsAString & aCounty) { return _to GetCounty(aCounty); } \
  NS_SCRIPTABLE NS_IMETHOD GetRegion(nsAString & aRegion) { return _to GetRegion(aRegion); } \
  NS_SCRIPTABLE NS_IMETHOD GetCountry(nsAString & aCountry) { return _to GetCountry(aCountry); } \
  NS_SCRIPTABLE NS_IMETHOD GetCountryCode(nsAString & aCountryCode) { return _to GetCountryCode(aCountryCode); } \
  NS_SCRIPTABLE NS_IMETHOD GetPostalCode(nsAString & aPostalCode) { return _to GetPostalCode(aPostalCode); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMGEOPOSITIONADDRESS(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetStreetNumber(nsAString & aStreetNumber) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStreetNumber(aStreetNumber); } \
  NS_SCRIPTABLE NS_IMETHOD GetStreet(nsAString & aStreet) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStreet(aStreet); } \
  NS_SCRIPTABLE NS_IMETHOD GetPremises(nsAString & aPremises) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPremises(aPremises); } \
  NS_SCRIPTABLE NS_IMETHOD GetCity(nsAString & aCity) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCity(aCity); } \
  NS_SCRIPTABLE NS_IMETHOD GetCounty(nsAString & aCounty) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCounty(aCounty); } \
  NS_SCRIPTABLE NS_IMETHOD GetRegion(nsAString & aRegion) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRegion(aRegion); } \
  NS_SCRIPTABLE NS_IMETHOD GetCountry(nsAString & aCountry) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCountry(aCountry); } \
  NS_SCRIPTABLE NS_IMETHOD GetCountryCode(nsAString & aCountryCode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCountryCode(aCountryCode); } \
  NS_SCRIPTABLE NS_IMETHOD GetPostalCode(nsAString & aPostalCode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPostalCode(aPostalCode); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMGeoPositionAddress : public nsIDOMGeoPositionAddress
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMGEOPOSITIONADDRESS

  nsDOMGeoPositionAddress();

private:
  ~nsDOMGeoPositionAddress();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMGeoPositionAddress, nsIDOMGeoPositionAddress)

nsDOMGeoPositionAddress::nsDOMGeoPositionAddress()
{
  /* member initializers and constructor code */
}

nsDOMGeoPositionAddress::~nsDOMGeoPositionAddress()
{
  /* destructor code */
}

/* readonly attribute DOMString streetNumber; */
NS_IMETHODIMP nsDOMGeoPositionAddress::GetStreetNumber(nsAString & aStreetNumber)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString street; */
NS_IMETHODIMP nsDOMGeoPositionAddress::GetStreet(nsAString & aStreet)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString premises; */
NS_IMETHODIMP nsDOMGeoPositionAddress::GetPremises(nsAString & aPremises)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString city; */
NS_IMETHODIMP nsDOMGeoPositionAddress::GetCity(nsAString & aCity)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString county; */
NS_IMETHODIMP nsDOMGeoPositionAddress::GetCounty(nsAString & aCounty)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString region; */
NS_IMETHODIMP nsDOMGeoPositionAddress::GetRegion(nsAString & aRegion)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString country; */
NS_IMETHODIMP nsDOMGeoPositionAddress::GetCountry(nsAString & aCountry)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString countryCode; */
NS_IMETHODIMP nsDOMGeoPositionAddress::GetCountryCode(nsAString & aCountryCode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute DOMString postalCode; */
NS_IMETHODIMP nsDOMGeoPositionAddress::GetPostalCode(nsAString & aPostalCode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


/* starting interface:    nsIDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH */
#define NS_IDOMGEOPOSITIONADDRESS_MOZILLA_1_9_2_BRANCH_IID_STR "98808deb-c8e4-422c-ba97-08bf2031464c"

#define NS_IDOMGEOPOSITIONADDRESS_MOZILLA_1_9_2_BRANCH_IID \
  {0x98808deb, 0xc8e4, 0x422c, \
    { 0xba, 0x97, 0x08, 0xbf, 0x20, 0x31, 0x46, 0x4c }}

class NS_NO_VTABLE NS_SCRIPTABLE nsIDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOMGEOPOSITIONADDRESS_MOZILLA_1_9_2_BRANCH_IID)

  /* readonly attribute string streetNumber; */
  NS_SCRIPTABLE NS_IMETHOD GetStreetNumber(char * *aStreetNumber) = 0;

  /* readonly attribute string street; */
  NS_SCRIPTABLE NS_IMETHOD GetStreet(char * *aStreet) = 0;

  /* readonly attribute string premises; */
  NS_SCRIPTABLE NS_IMETHOD GetPremises(char * *aPremises) = 0;

  /* readonly attribute string city; */
  NS_SCRIPTABLE NS_IMETHOD GetCity(char * *aCity) = 0;

  /* readonly attribute string county; */
  NS_SCRIPTABLE NS_IMETHOD GetCounty(char * *aCounty) = 0;

  /* readonly attribute string region; */
  NS_SCRIPTABLE NS_IMETHOD GetRegion(char * *aRegion) = 0;

  /* readonly attribute string country; */
  NS_SCRIPTABLE NS_IMETHOD GetCountry(char * *aCountry) = 0;

  /* readonly attribute string countryCode; */
  NS_SCRIPTABLE NS_IMETHOD GetCountryCode(char * *aCountryCode) = 0;

  /* readonly attribute string postalCode; */
  NS_SCRIPTABLE NS_IMETHOD GetPostalCode(char * *aPostalCode) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsIDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH, NS_IDOMGEOPOSITIONADDRESS_MOZILLA_1_9_2_BRANCH_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSIDOMGEOPOSITIONADDRESS_MOZILLA_1_9_2_BRANCH \
  NS_SCRIPTABLE NS_IMETHOD GetStreetNumber(char * *aStreetNumber); \
  NS_SCRIPTABLE NS_IMETHOD GetStreet(char * *aStreet); \
  NS_SCRIPTABLE NS_IMETHOD GetPremises(char * *aPremises); \
  NS_SCRIPTABLE NS_IMETHOD GetCity(char * *aCity); \
  NS_SCRIPTABLE NS_IMETHOD GetCounty(char * *aCounty); \
  NS_SCRIPTABLE NS_IMETHOD GetRegion(char * *aRegion); \
  NS_SCRIPTABLE NS_IMETHOD GetCountry(char * *aCountry); \
  NS_SCRIPTABLE NS_IMETHOD GetCountryCode(char * *aCountryCode); \
  NS_SCRIPTABLE NS_IMETHOD GetPostalCode(char * *aPostalCode); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSIDOMGEOPOSITIONADDRESS_MOZILLA_1_9_2_BRANCH(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetStreetNumber(char * *aStreetNumber) { return _to GetStreetNumber(aStreetNumber); } \
  NS_SCRIPTABLE NS_IMETHOD GetStreet(char * *aStreet) { return _to GetStreet(aStreet); } \
  NS_SCRIPTABLE NS_IMETHOD GetPremises(char * *aPremises) { return _to GetPremises(aPremises); } \
  NS_SCRIPTABLE NS_IMETHOD GetCity(char * *aCity) { return _to GetCity(aCity); } \
  NS_SCRIPTABLE NS_IMETHOD GetCounty(char * *aCounty) { return _to GetCounty(aCounty); } \
  NS_SCRIPTABLE NS_IMETHOD GetRegion(char * *aRegion) { return _to GetRegion(aRegion); } \
  NS_SCRIPTABLE NS_IMETHOD GetCountry(char * *aCountry) { return _to GetCountry(aCountry); } \
  NS_SCRIPTABLE NS_IMETHOD GetCountryCode(char * *aCountryCode) { return _to GetCountryCode(aCountryCode); } \
  NS_SCRIPTABLE NS_IMETHOD GetPostalCode(char * *aPostalCode) { return _to GetPostalCode(aPostalCode); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSIDOMGEOPOSITIONADDRESS_MOZILLA_1_9_2_BRANCH(_to) \
  NS_SCRIPTABLE NS_IMETHOD GetStreetNumber(char * *aStreetNumber) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStreetNumber(aStreetNumber); } \
  NS_SCRIPTABLE NS_IMETHOD GetStreet(char * *aStreet) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetStreet(aStreet); } \
  NS_SCRIPTABLE NS_IMETHOD GetPremises(char * *aPremises) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPremises(aPremises); } \
  NS_SCRIPTABLE NS_IMETHOD GetCity(char * *aCity) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCity(aCity); } \
  NS_SCRIPTABLE NS_IMETHOD GetCounty(char * *aCounty) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCounty(aCounty); } \
  NS_SCRIPTABLE NS_IMETHOD GetRegion(char * *aRegion) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetRegion(aRegion); } \
  NS_SCRIPTABLE NS_IMETHOD GetCountry(char * *aCountry) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCountry(aCountry); } \
  NS_SCRIPTABLE NS_IMETHOD GetCountryCode(char * *aCountryCode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetCountryCode(aCountryCode); } \
  NS_SCRIPTABLE NS_IMETHOD GetPostalCode(char * *aPostalCode) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetPostalCode(aPostalCode); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH : public nsIDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDOMGEOPOSITIONADDRESS_MOZILLA_1_9_2_BRANCH

  nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH();

private:
  ~nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH, nsIDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH)

nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH::nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH()
{
  /* member initializers and constructor code */
}

nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH::~nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH()
{
  /* destructor code */
}

/* readonly attribute string streetNumber; */
NS_IMETHODIMP nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH::GetStreetNumber(char * *aStreetNumber)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute string street; */
NS_IMETHODIMP nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH::GetStreet(char * *aStreet)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute string premises; */
NS_IMETHODIMP nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH::GetPremises(char * *aPremises)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute string city; */
NS_IMETHODIMP nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH::GetCity(char * *aCity)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute string county; */
NS_IMETHODIMP nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH::GetCounty(char * *aCounty)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute string region; */
NS_IMETHODIMP nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH::GetRegion(char * *aRegion)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute string country; */
NS_IMETHODIMP nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH::GetCountry(char * *aCountry)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute string countryCode; */
NS_IMETHODIMP nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH::GetCountryCode(char * *aCountryCode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute string postalCode; */
NS_IMETHODIMP nsDOMGeoPositionAddress_MOZILLA_1_9_2_BRANCH::GetPostalCode(char * *aPostalCode)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIDOMGeoPositionAddress_h__ */
