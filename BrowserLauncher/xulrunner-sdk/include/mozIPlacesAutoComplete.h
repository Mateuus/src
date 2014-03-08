/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM e:/builds/moz2_slave/release-mozilla-1.9.2-xulrunner_win32_build/build/toolkit/components/places/public/mozIPlacesAutoComplete.idl
 */

#ifndef __gen_mozIPlacesAutoComplete_h__
#define __gen_mozIPlacesAutoComplete_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    mozIPlacesAutoComplete */
#define MOZIPLACESAUTOCOMPLETE_IID_STR "a5ae8332-333c-412a-bb02-a35df8247714"

#define MOZIPLACESAUTOCOMPLETE_IID \
  {0xa5ae8332, 0x333c, 0x412a, \
    { 0xbb, 0x02, 0xa3, 0x5d, 0xf8, 0x24, 0x77, 0x14 }}

/**
 * This interface provides some constants used by the Places AutoComplete
 * search provider.
 */
class NS_NO_VTABLE NS_SCRIPTABLE mozIPlacesAutoComplete : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(MOZIPLACESAUTOCOMPLETE_IID)

  /**
   * Match anywhere in each searchable term.
   */
  enum { MATCH_ANYWHERE = 0 };

  /**
   * Match first on word boundaries, and if we do not get enough results, then
   * match anywhere in each searchable term.
   */
  enum { MATCH_BOUNDARY_ANYWHERE = 1 };

  /**
   * Match on word boundaries in each searchable term.
   */
  enum { MATCH_BOUNDARY = 2 };

  /**
   * Match only the beginning of each search term.
   */
  enum { MATCH_BEGINNING = 3 };

  /**
   * Search through history.
   */
  enum { BEHAVIOR_HISTORY = 1 };

  /**
   * Search though bookmarks.
   */
  enum { BEHAVIOR_BOOKMARK = 2 };

  /**
   * Search through tags.
   */
  enum { BEHAVIOR_TAG = 4 };

  /**
   * Search the title of pages.
   */
  enum { BEHAVIOR_TITLE = 8 };

  /**
   * Search the URL of pages.
   */
  enum { BEHAVIOR_URL = 16 };

  /**
   * Search for typed pages.
   */
  enum { BEHAVIOR_TYPED = 32 };

  /**
   * Search javascript: URLs.
   */
  enum { BEHAVIOR_JAVASCRIPT = 64 };

};

  NS_DEFINE_STATIC_IID_ACCESSOR(mozIPlacesAutoComplete, MOZIPLACESAUTOCOMPLETE_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_MOZIPLACESAUTOCOMPLETE \

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_MOZIPLACESAUTOCOMPLETE(_to) \

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_MOZIPLACESAUTOCOMPLETE(_to) \

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public mozIPlacesAutoComplete
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_MOZIPLACESAUTOCOMPLETE

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, mozIPlacesAutoComplete)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* End of implementation class template. */
#endif


#endif /* __gen_mozIPlacesAutoComplete_h__ */
