/*
  Copyright 2019 Lou Hafer, Matt Saltzman, Kevin Poton
  This code is licensed under the terms of the Eclipse Public License (EPL)

  $Id$
*/
/*! \file Osi2APIMgmt_Imp.hpp

  Defines Osi2::APIMgmt_Imp, an implementation of the capabilities needed to
  keep track of the APIs supplied by an object.
*/


#ifndef Osi2APIMgmt_Imp_HPP
#define Osi2APIMgmt_Imp_HPP

#include <map>
#include <cstring>

namespace Osi2 {

/*
  \brief A simple implementation of API management capabilities.

  This class provides a simple implementation of the capabilities required to
  maintain a set of supported APIs.  It provides methods to add and remove
  APIs from the set of supported APIs, and methods that are suitable for
  implementing object API inquiry as defined in Osi2::API.  It is intended
  to be used as an included object.
*/
class APIMgmt_Imp {

public:

/*! \name Constructors & related */
//@{
  /// Default constructor
  APIMgmt_Imp ()
    : apiIDCnt_(0),
      apiIDLen_(0),
      apiIDs_(nullptr)
  { }
  /// Copy constructor
  APIMgmt_Imp (const APIMgmt_Imp &rhs)
  {
    apiIDCnt_ = rhs.apiIDCnt_ ;
    apiIDLen_ = rhs.apiIDLen_ ;
    apiIDs_ = new const char*[apiIDLen_] ;
    for (int ndx = 0 ; ndx < apiIDCnt_ ; ndx++) {
      apiIDs_[ndx] = rhs.apiIDs_[ndx] ;
    }
    objForAPI_ = rhs.objForAPI_ ;
  }
  /// Assignment
  APIMgmt_Imp &operator= (const APIMgmt_Imp &rhs)
  {
    /// Self-assignment requires no work.
    if (this == &rhs) return (*this) ;
    /// Otherwise, do the work
    apiIDCnt_ = rhs.apiIDCnt_ ;
    apiIDLen_ = rhs.apiIDLen_ ;
    apiIDs_ = new const char*[apiIDLen_] ;
    for (int ndx = 0 ; ndx < apiIDCnt_ ; ndx++) {
      apiIDs_[ndx] = rhs.apiIDs_[ndx] ;
    }
    objForAPI_ = rhs.objForAPI_ ;
    return (*this) ;
  }
  /// Destructor
  ~APIMgmt_Imp ()
  { delete[] apiIDs_ ; }

//@}

/*! \name API management methods */
//@{
  /// Get a vector of supported APIs
  inline int getAPIs (const char **&idents)
  {
    idents = apiIDs_ ;
    return (apiIDCnt_) ;
  }
  /// Get the object implementing the specified %API.
  inline void *getAPIPtr(const char *ident)
  {
    if (objForAPI_.find(ident) == objForAPI_.end()) {
      return (nullptr) ;
    } else {
      return (objForAPI_.find(ident)->second) ;
    }
  }
  /// Add an %API to the set of supported APIs
  inline void addAPIID (const char *apiID, void *obj)
  {
    if (getAPIPtr(apiID) == nullptr) {
      if (apiIDCnt_+1 > apiIDLen_) {
	const char **tmp = new const char*[apiIDLen_+5] ;
        for (int ndx = 0 ; ndx < apiIDCnt_ ; ndx++)
	{ tmp[ndx] = apiIDs_[ndx] ; }
	delete[] apiIDs_ ;
	apiIDs_ = tmp ;
      }
      apiIDs_[apiIDCnt_++] = apiID ;
      objForAPI_[apiID] = obj ;
    }
    return ;
  }
  /// Remove an %API from the set of supported APIs.
  inline void delAPIID (const char *apiID)
  {
    if (objForAPI_.erase(apiID) > 0) {
      for (int ndx = 0 ; ndx < apiIDCnt_ ; ndx++) {
        if (strcmp(apiIDs_[ndx],apiID) == 0) {
	  if (ndx < --apiIDCnt_) {
	    apiIDs_[ndx] = apiIDs_[apiIDCnt_] ;
	  }
	  apiIDs_[apiIDCnt_] = nullptr ;
	  break ;
	}
      }
    }
    return ;
  }
//@}

private:

  /// Number of API id strings
  int apiIDCnt_ ;
  /// Capacity of apiIDs_
  int apiIDLen_ ;
  /// Simple vector, pointers to null-terminated char strings.
  const char **apiIDs_ ;
  /// Map to record the implementing object for an API
  std::map<std::string,void*> objForAPI_ ;

} ;

}    // namespace Osi2

#endif 

