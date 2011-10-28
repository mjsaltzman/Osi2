/*
  Copyright 2011 Lou Hafer, Matt Saltzman
  This code is licensed under the terms of the Eclipse Public License (EPL)

  $Id$
*/
/*! \file Osi2ProbMgmtAPI_Clp.cpp
    \brief Method definitions for Osi2ProbMgmtAPI_Clp

  Method definitions for ProbMgmtAPI_Clp, an implementation of the
  problem management API using the `light' clp shim that dynamically loads
  methods on demand.
*/

#include <iostream>

#include "ClpConfig.h"
#include "Clp_C_Interface.h"
#include "Osi2ClpShim.hpp"

#include "Osi2Config.h"
#include "Osi2nullptr.hpp"

#include "Osi2API.hpp"
#include "Osi2ProbMgmtAPI.hpp"
#include "Osi2ProbMgmtAPI_Clp.hpp"

namespace Osi2 {


/*
  Capture a pointer to the underlying ClpSimplex object.
*/
ProbMgmtAPI_Clp::ProbMgmtAPI_Clp (DynamicLibrary *libClp,
                                  Clp_Simplex *clpSimplex)
    : libClp_(libClp),
      clpSimplex_(clpSimplex),
      readMps_(nullptr),
      initialSolve_(nullptr)
{
}

ProbMgmtAPI_Clp::~ProbMgmtAPI_Clp ()
{
  typedef void (*ClpDeleteModelFunc)(Clp_Simplex *clp) ;
  std::string errStr ;
  ClpDeleteModelFunc deleteModel =
      reinterpret_cast<ClpDeleteModelFunc>
      (libClp_->getSymbol("Clp_deleteModel",errStr)) ;
  if (deleteModel == nullptr) {
    std::cout << "Apparent failure to find Clp_deleteModel." << std::endl ;
    std::cout << errStr << std::endl ;
  } else {
    deleteModel(clpSimplex_) ;
    clpSimplex_ = nullptr ;
  }
  std::cout << "ProbMgmtAPI_Clp object destroyed." << std::endl ;
}

/*
  Read a problem file in mps format.
*/
int ProbMgmtAPI_Clp::readMps (const char *filename, bool keepNames,
                              bool ignoreErrors)
{
    std::string errStr ;
    if (readMps_ == nullptr) {
      ClpReadMpsFunc readMps =
	  reinterpret_cast<ClpReadMpsFunc>
	  (libClp_->getSymbol("Clp_readMps", errStr)) ;
      if (readMps == nullptr) {
	  std::cout << "Apparent failure to find Clp_readMps." << std::endl ;
	  std::cout << errStr << std::endl ;
	  return (-1) ;
      }
      readMps_ = readMps ;
    }

    int retval = readMps_(clpSimplex_, filename, keepNames, ignoreErrors) ;

    if (retval) {
        std::cout
                << "Failure to read " << filename << ", error " << retval
                << "." << std::endl ;
    } else {
        std::cout
                << "Read " << filename << " without error." << std::endl ;
    }

    return (retval) ;
}

/*
  Solve a problem
*/
int ProbMgmtAPI_Clp::initialSolve ()
{
    std::string errStr ;
    if (initialSolve_ == nullptr) {
      ClpInitialSolveFunc initialSolve =
	  reinterpret_cast<ClpInitialSolveFunc>
	  (libClp_->getSymbol("Clp_initialSolve", errStr)) ;
      if (initialSolve == nullptr) {
	  std::cout
	    << "Apparent failure to find Clp_initialSolve." << std::endl ;
	  std::cout << errStr << std::endl ;
	  return (-1) ;
      }
      initialSolve_ = initialSolve ;
    }

    int retval = initialSolve_(clpSimplex_) ;

    if (retval < 0) {
        std::cout
                << "Solve failed; error " << retval << "." << std::endl ;
    } else {
        std::cout
                << "Solved; return status " << retval << "." << std::endl ;
    }

    return (retval) ;
}

}
