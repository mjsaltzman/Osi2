/*
  Copyright 2011 Lou Hafer, Matt Saltzman
  This code is licensed under the terms of the Eclipse Public License (EPL)

  $Id$
*/
/*! \file Osi2ClpShim.hpp
    \brief Declarations for Osi2::ClpShim.

  This shim is written to dynamically load libClp. As such, it doesn't need to
  be linked with libClp. The tradeoff is that it must work through clp's C
  interface and dynamically load the methods it wants to use.
*/

#ifndef Osi2ClpShim_H
#define Osi2ClpShim_H

#include "Osi2Plugin.hpp"
#include "Osi2DynamicLibrary.hpp"

namespace Osi2 {
/*! \brief Light shim for the clp solver

  This shim is written to dynamically load libClp.  This means that it
  must work through clp's C interface and dynamically load the methods it
  wants to use.
*/

class ClpShim {

public:

    /// Default constructor
    ClpShim() ;

    /*! \brief Object factory

      Create clp-specific objects to satisfy the Osi2 API specified as the
      \p objectType member of of \p params.
    */
    static void *create (const ObjectParams *params) ;

    /*! \brief Object destructor

      Destroys objects created by this shim.
    */
    static int32_t destroy (void *victim, const ObjectParams *params) ;

    /// Set our unique ID (supplied by the plugin manager)
    inline void setPluginID (PluginUniqueID id) {
        ourID_ = id ;
    }
    /// Get our unique ID
    inline PluginUniqueID getPluginID () const {
        return (ourID_) ;
    }
    /// Set libClp handle
    inline void setLibClp (DynamicLibrary *libClp) {
        libClp_ = libClp ;
    }
    /// Get libClp handle
    inline DynamicLibrary *getLibClp () const {
        return (libClp_) ;
    }
    /// Set verbosity
    inline void setVerbosity (int verbosity) {
        verbosity_ = verbosity ;
    }
    /// Get verbosity
    inline int getVerbosity () const {
        return (verbosity_) ;
    }

private:

    /*! \brief Plugin manager services structure

      Structure passed in at plugin initialisation specifying services available
      from the plugin manager.
    */
    const PlatformServices *services_ ;

    /// The handle for libClp
    DynamicLibrary *libClp_ ;

    /// Our registration ID from the plugin manager
    PluginUniqueID ourID_ ;

    /// Verbosity level for information messages
    int verbosity_ ;

} ;

/*! \brief Plugin initialisation method
    \relates ClpShim

  Given a parameter specifying the plugin manager's registration method,
  constructs a registration parameter object and calls the registration
  method.

  This method needs to have C linkage so it can be easily loaded with
  DynamicLibrary::getSymbol.
*/
extern "C"
ExitFunc initPlugin (PlatformServices *services) ;

/*! \brief Plugin cleanup method
    \relates ClpShim

  This method handles any necessary cleanup prior to unloading the method. It
  will be passed to the plugin manager and invoked just before the plugin
  manager unloads the plugin.

  It's unclear to me that this method actually needs C linkage, since it's
  passed to the plugin manager as the return value of #initPlugin().
*/
extern "C"
int32_t cleanupPlugin (const PlatformServices *services) ;

}  // end namespace Osi2

#endif		// Clpshim_H

