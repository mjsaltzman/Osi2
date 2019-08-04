/*
  Copyright 2011 Lou Hafer, Matt Saltzman
  This code is licensed under the terms of the Eclipse Public License (EPL)

  $Id$
*/
/*! \file Osi2ClpShim.cpp
    \brief Method definitions for ClpShim.

  This shim is written to dynamically load libClp. As such, it doesn't
  need to be statically linked with libClp. The tradeoff is that it must
  work through clp's C interface. The shim, and any objects it constructs,
  must dynamically load any methods they wish to use.
*/

#include <iostream>

#include "ClpConfig.h"
#include "Osi2ClpShim.hpp"
#include "Clp_C_Interface.h"

#include "Osi2Config.h"
#include "Osi2nullptr.hpp"
// #include "Osi2DynamicLibrary.hpp"

#include "Osi2ClpLite_Wrap.hpp"

typedef Clp_Simplex *(*ClpFactory)() ;

using namespace Osi2 ;

/*
  Default constructor.
*/
ClpShim::ClpShim ()
    : services_(0),
      libClp_(0),
      ourID_(0),
      verbosity_(1)
{ }

/*! \brief Object factory

  Create clp-specific objects to satisfy the OSI2 API specified as the
  \p objectType member of of \p params.

  The name "WildClpSimplex" is used for testing wildcard object creation.
*/
void *ClpShim::create (const ObjectParams *params)
{

  std::string what = reinterpret_cast<const char *>(params->apiStr_) ;
  void *retval = nullptr ;

  std::cout << "Clp create: " << what << " API requested." << std::endl ;
/*
  For an object that supports the ClpSimplex API, create a ClpLite_Wrap object
  and add a ClpSimplexAPI_ClpLite object to it.
*/
  if (what == "ClpSimplex" || what == "WildClpSimplex") {
    std::cout
      << "  " << what << " API is  supported." << std::endl ;
    ClpShim *shim = static_cast<ClpShim*>(params->ctrlObj_) ;
    DynamicLibrary *libClp = shim->libClp_ ;
    ClpLite_Wrap *clpliteWrapper = new ClpLite_Wrap() ;
    clpliteWrapper->addClpSimplex(libClp) ;
    retval = clpliteWrapper ;
  } else {
    std::cout
      << "    " << what << " API is not supported." << std::endl ;
  }

  return (retval) ;
}

/*! \brief Object destructor

  Given that ClpShim only hands out C++ objects that are derived from
  Osi2::API, we can simply invoke the destructor with delete.
*/
int32_t ClpShim::destroy (void *victim, const ObjectParams *objParms)
{
    std::string what = reinterpret_cast<const char *>(objParms->apiStr_) ;
    std::cout
            << "Request to destroy " << what << " recognised." << std::endl ;
    API *api = static_cast<API *>(victim) ;
    delete api ;

    return (0) ;
}


/*
  Plugin initialisation method. The job here is to construct a parameter
  structure which can be passed to the plugin manager's registration method
  (supplied in the services parameter). The return value is the exit method to
  be called before the plugin is unloaded.
*/
extern "C"
ExitFunc initPlugin (PlatformServices *services)
{
  std::string version = CLP_VERSION ;
  std::cout
      << "Executing ClpShim::initPlugin, clp version "
      << version << "." << std::endl ;
/*
  Attempt to load clp.
*/
  std::string libClpName = "libClp.so.0" ;
  const char *tmp = reinterpret_cast<const char*>(services->dfltPluginDir_) ;
  std::string libPath(tmp) ;
  std::string errMsg ;
  std::string fullPath = libPath + "/" + libClpName ;
  DynamicLibrary *libClp = DynamicLibrary::load(fullPath, errMsg) ;
  if (libClp == nullptr) {
    std::cout
	<< "Apparent failure opening " << fullPath << "." << std::endl ;
    std::cout
	<< "Error is " << errMsg << "." << std::endl ;
    return (nullptr) ;
  }
/*
  Create the plugin library state object, ClpShim.  Arrange to remember the
  handle to libClp, and our unique ID from the plugin manager.  Then stash a
  pointer to the shim in PlatformServices to return it to the plugin manager.
  This allows the plugin manager to hand back the shim object with each call,
  which in turn allows us to remember what we're doing.
*/
  ClpShim *shim = new ClpShim() ;
  shim->setLibClp(libClp) ;
  shim->setPluginID(services->pluginID_) ;
  services->ctrlObj_ = static_cast<PluginState *>(shim) ;
/*
  Register our APIs.

  For each API, fill in the the registration parameters and invoke the
  registration function. We could specify a separate state object for each
  API, but so far that doesn't seem necessary --- just use the library's
  state object.
*/
  APIRegInfo reginfo ;
  reginfo.version_.major_ = 1 ;
  reginfo.version_.minor_ = CLP_VERSION_MINOR ;
  reginfo.pluginID_ = shim->getPluginID() ;
  reginfo.lang_ = Plugin_CPP ;
  reginfo.ctrlObj_ = static_cast<APIState *>(shim) ;
  reginfo.createFunc_ = ClpShim::create ;
  reginfo.destroyFunc_ = ClpShim::destroy ;
  int retval = services->registerAPI_(
	  reinterpret_cast<const CharString*>("ClpSimplex"),&reginfo) ;
  if (retval < 0) {
    std::cout
	<< "Apparent failure to register ClpSimplex API." << std::endl ;
    return (nullptr) ;
  }
/*
  Register a wildcard object (just for testing)
*/
  retval =
      services->registerAPI_(
	  reinterpret_cast<const CharString*>("*"), &reginfo) ;
  if (retval < 0) {
    std::cout
	<< "Apparent failure to register wildcard plugin." << std::endl ;
    return (nullptr) ;
  }
  return (cleanupPlugin) ;
}

/*
  Plugin cleanup method. Does whatever is needed to clean up after the plugin
  prior to unloading the library.
*/
extern "C" int32_t cleanupPlugin (const PlatformServices *services)
{
    return (0) ;
}

