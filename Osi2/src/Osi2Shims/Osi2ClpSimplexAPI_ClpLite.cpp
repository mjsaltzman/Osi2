/*
  Copyright 2011 Lou Hafer, Matt Saltzman
  This code is licensed under the terms of the Eclipse Public License (EPL)

  $Id$
*/
/*! \file Osi2ClpSimplexAPI_ClpLite.cpp
    \brief Method definitions for Osi2ClpSimplexAPI_ClpLite

  Method definitions for ClpSimplexAPI_ClpLite, an implementation of the
  problem management API using the `lite' clp shim that dynamically loads
  methods on demand.
*/

#include <iostream>

// #include "ClpConfig.h"
// #include "Clp_C_Interface.h"
// #include "Osi2ClpShim.hpp"

#include "Osi2Config.h"
#include "Osi2nullptr.hpp"


// #include "Osi2API.hpp"
// #include "Osi2ClpSimplexAPI.hpp"
#include "Osi2ClpSimplexAPI_ClpLite.hpp"

#include "Osi2DynamicLibrary.hpp"

/*
  Various commonly used function typedefs, to avoid repeating them over and
  over.
*/

typedef double (*rDp0)(Clp_Simplex *) ;
typedef void (*rpVpD)(Clp_Simplex *, double) ;


namespace Osi2 {

/*
  Constructor
*/
ClpSimplexAPI_ClpLite::ClpSimplexAPI_ClpLite (DynamicLibrary *libClp)
    : paramMgr_(ParamBEAPI_Imp<ClpSimplexAPI_ClpLite>(this)),
      libClp_(libClp),
      clpC_(nullptr)
{
  std::string errStr ;
/*
  Look up Clp_newModel, coerce it into a suitable type, and invoke it to get a
  new Clp_Simplex object.
*/
  typedef Clp_Simplex *(*ClpFactory)() ;
  ClpFactory factory =
      libClp_->getFunc<ClpFactory>(std::string("Clp_newModel"),errStr) ;
  if (factory != nullptr) {
    clpC_ = factory() ;
  }
/*
  Register that we are a ClpSimplex API and that we support parameter
  management.
*/
  paramMgr_.addAPIID(ClpSimplexAPI::getAPIIDString(),this) ;
  paramMgr_.addAPIID(ParamBEAPI::getAPIIDString(),&paramMgr_) ;
/*
  Register the parameters exposed through parameter management
*/
  typedef ParamBEAPI_Imp<ClpSimplexAPI_ClpLite>::ParamEntry_Imp<double> DPE ;
  ParamBEAPI_Imp<ClpSimplexAPI_ClpLite>::ParamEntry *paramEntry ;
  paramEntry = new DPE("primal tolerance",
  		       &ClpSimplexAPI_ClpLite::primalTolerance,
  		       &ClpSimplexAPI_ClpLite::setPrimalTolerance) ;
  paramMgr_.addParam("primal tolerance",paramEntry) ;
  paramEntry = new DPE("dual tolerance",
  		       &ClpSimplexAPI_ClpLite::dualTolerance,
  		       &ClpSimplexAPI_ClpLite::setDualTolerance) ;
  paramMgr_.addParam("dual tolerance",paramEntry) ;
  return ;
}

ClpSimplexAPI_ClpLite::~ClpSimplexAPI_ClpLite ()
{
  std::string errStr ;
/*
  Look up Clp_deleteModel, coerce it into a suitable type, and invoke it to
  destroy the Clp_Simplex object.
*/
  typedef void (*ClpDeleteModelFunc)(Clp_Simplex *clp) ;
  ClpDeleteModelFunc deleteModel =
     libClp_->getFunc<ClpDeleteModelFunc>("Clp_deleteModel",errStr) ;
  if (deleteModel != nullptr) {
    deleteModel(clpC_) ;
    clpC_ = nullptr ;
    std::cout << "ClpSimplexAPI_ClpLite object destroyed." << std::endl ;
  }
}

/*
  Read a problem file in mps format.
*/
int ClpSimplexAPI_ClpLite::readMps (const char *filename, bool keepNames,
                              bool ignoreErrors)
{
  std::string errStr ;

  typedef int (*ClpReadMpsFunc)(Clp_Simplex *,const char *,int,int) ;
  static ClpReadMpsFunc readMps = nullptr ;

  if (readMps == nullptr) {
    readMps = libClp_->getFunc<ClpReadMpsFunc>("Clp_readMps",errStr) ;
  }
  int retval = -1 ;
  if (readMps != nullptr) {
    retval = readMps(clpC_,filename,keepNames,ignoreErrors) ;
    if (retval) {
	std::cout
	    << "Failure to read " << filename << ", error " << retval
	    << "." << std::endl ;
    } else {
	std::cout
	    << "Read " << filename << " without error." << std::endl ;
    }
  }
  return (retval) ;
}

/* Get / set primal tolerance */

double ClpSimplexAPI_ClpLite::primalTolerance () const
{
  std::string errStr ;

  static rDp0 getPrimalTolerance = nullptr ;

  if (getPrimalTolerance == nullptr) {
    getPrimalTolerance =
	libClp_->getFunc<rDp0>("Clp_primalTolerance",errStr) ;
  }
  return (getPrimalTolerance(clpC_)) ;
}

void ClpSimplexAPI_ClpLite::setPrimalTolerance (double val)
{
  std::string errStr ;

  static rpVpD setPrimalTolerance = nullptr ;

  if (setPrimalTolerance == nullptr) {
    setPrimalTolerance =
	libClp_->getFunc<rpVpD>("Clp_setPrimalTolerance",errStr) ;
  }
  return (setPrimalTolerance(clpC_,val)) ;
}

/* Get / set dual tolerance */

double ClpSimplexAPI_ClpLite::dualTolerance () const
{
  std::string errStr ;

  static rDp0 getDualTolerance = nullptr ;

  if (getDualTolerance == nullptr) {
    getDualTolerance =
	libClp_->getFunc<rDp0>("Clp_dualTolerance",errStr) ;
  }
  return (getDualTolerance(clpC_)) ;
}

void ClpSimplexAPI_ClpLite::setDualTolerance (double val)
{
  std::string errStr ;

  static rpVpD setDualTolerance = nullptr ;

  if (setDualTolerance == nullptr) {
    setDualTolerance =
	libClp_->getFunc<rpVpD>("Clp_setDualTolerance",errStr) ;
  }
  return (setDualTolerance(clpC_,val)) ;
}

/*
  Solve a problem
*/
int ClpSimplexAPI_ClpLite::initialSolve ()
{
  std::string errStr ;

  typedef int (*ClpInitSolFunc)(Clp_Simplex *) ;
  static ClpInitSolFunc initialSolve = nullptr ;

  if (initialSolve == nullptr) {
    initialSolve = libClp_->getFunc<ClpInitSolFunc>("Clp_initialSolve",errStr) ;
  }
  int retval = -1 ;
  if (initialSolve != nullptr) {
      retval = initialSolve(clpC_) ;
      if (retval < 0) {
	  std::cout
	      << "Solve failed; error " << retval << "." << std::endl ;
      } else {
	  std::cout
	      << "Solved; return status " << retval << "." << std::endl ;
      }
  }
  return (retval) ;
}

}    // end namespace Osi2
