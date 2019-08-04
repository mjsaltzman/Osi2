/*
  Copyright 2011 Lou Hafer, Matt Saltzman
  This code is licensed under the terms of the Eclipse Public License (EPL)

  $Id$
*/
/*! \file Osi2ClpSimplexAPI_ClpLite.hpp
    \brief The clp `lite' implementation of Osi2::ClpSimplexAPI
*/
#ifndef Osi2ClpSimplexAPI_ClpLite_HPP
#define Osi2ClpSimplexAPI_ClpLite_HPP

#include "Osi2API.hpp"
#define COIN_EXTERN_C
#include "Osi2ClpSimplexAPI.hpp"

#include "Osi2APIMgmt_Imp.hpp"
#include "Osi2ParamBEAPI_Imp.hpp"

namespace Osi2 {

class DynamicLibrary ;

/*! \brief Proof of concept API.

  Capable of loading and solving a problem. Period.
*/
class ClpSimplexAPI_ClpLite : public ClpSimplexAPI {

public:
/*! \name Constructors, destructor, and copy */
//@{
  /// Constructor
  ClpSimplexAPI_ClpLite(DynamicLibrary *libClp) ;

  /// Destructor
  ~ClpSimplexAPI_ClpLite() ;
//@}

/*! \name Methods to read or write a problem file */
//@{
  /// Read an mps file from the given filename
  int readMps(const char *filename, bool keepNames = false,
	      bool ignoreErrors = false) ;
//@}

/*! \name Parameter Gets & Sets */
//@{
  /// Primal zero tolerance
  double primalTolerance() const ;
  void setPrimalTolerance(double val) ;

  /// Dual zero tolerance
  double dualTolerance() ;
  void setDualTolerance(double val) ;
//@}

/*! \name Functions must useful to user */
//@{
  /// General solve algorithm
  int initialSolve() ;
//@}

/*! \name API management & enquiry */
//@{
  inline int getAPIs(const char **&idents)
  { return (paramMgr_.getAPIs(idents)) ; }

  inline void *getAPIPtr (const char *ident)
  { return (paramMgr_.getAPIPtr(ident)) ; }

private:
  /*! \name Dynamic object management information */
  //@{
    /// Parameter management object
    ParamBEAPI_Imp<ClpSimplexAPI_ClpLite> paramMgr_ ;
    /// Dynamic library handle
    DynamicLibrary *libClp_ ;
    /// Clp object
    Clp_Simplex *clpC_ ;
  //@}

} ;

}  // end namespace Osi2

#endif

