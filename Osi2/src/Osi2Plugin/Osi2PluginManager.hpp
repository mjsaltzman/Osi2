/*
  Copyright 2011 Lou Hafer, Matt Saltzman
  This code is licensed under the terms of the Eclipse Public License (EPL)

  Based on and extended from original design and code by Gigi Sayfan published
  in five parts in Dr. Dobbs, starting November 2007.
*/
/*! \file Osi2PluginManager.hpp
    \brief Declarations for Osi2::PluginManagera

  A class to manage dynamic libraries and objects. Wraps the low-level methods
  of DynamicLibrary with convenient bookkeeping.
*/

#ifndef OSI2PLUGINMANAGER_HPP
#define OSI2PLUGINMANAGER_HPP

#include <vector>
#include <map>
#include "Osi2PlugMgrMessages.hpp"
#include "Osi2Plugin.hpp"


namespace Osi2 {

class DynamicLibrary ;
struct IObjectAdapter ;


/*! \brief Plugin library manager

  This class provides generic support for loading and unloading plugin
  libraries. As libraries are loaded, they register the APIs that they support
  with the manager. Clients can then ask for objects which support a registered
  API. APIs are identified by strings.

  There is a single instance of the plugin manager (declared as a static
  object private to #getInstance).

  The act of loading a plugin library is an atomic transaction managed by
  #loadOneLib: The library must be found, loaded, and successfully complete
  its initialisation function before registration information is copied into
  the manager's client-accessible data structures.

  APIs are registered by plugin libraries; a character string identifies each
  API. If the registration string is exactly "*", the registration is classed
  as a wildcard and entered in the #wildCardVec_. Otherwise, it will be
  entered in the #exactMatchMap_ using the character string as the key.

  Clients request an object supporting a specific API by specifying a
  character string.  If an exact match for the API requested by the client
  is not found in #exactMatchMap_, the #wildCardVec_ is scanned. For each
  entry, the corresponding create function is asked to create an object
  supporting the requested API. If this is successful, an entry is made in the
  #exactMatchMap_ so that subsequent requests for the same API can be
  satisfied more efficiently.

  Any request for an object supporting a particular API can be qualified with
  a request that the object be supplied by a particular library.
*/

class PluginManager {

public:

    /*! \brief Get a reference to the plugin manager

      Returns a reference to the single instance of the plugin manager.
    */
    static PluginManager &getInstance() ;

    /*! \name Public plugin library management methods

      Methods to load and unload plugin libraries.
    */
    //@{

    /// Get the default plugin directory
    inline std::string getDfltPluginDir() const {
        return (dfltPluginDir_) ;
    }

    /// Set the default plugin directory
    inline void setDfltPluginDir(std::string dfltDir) {
        dfltPluginDir_ = dfltDir ;
    }

    /// Return full path for a library
    std::string getLibPath(PluginUniqueID libID) ;

    /// Get the services provided by the plugin manager
    PlatformServices &getPlatformServices() ;

    /*! \brief Load and initialise the specified plugin library.

      Load the specified library \c dir/lib. If \p dir is not specified,
      #dfltPluginDir_ is used.

      The PluginUniqueID assigned to the library will be returned in \p uniqueID
      if a parameter is supplied.

      \return
      - -3: initialisation function failed
      - -2: failed to find the initialisation function
      - -1: library failed to load
      -  0: library loaded and initialised without error
      -  1: library is already loaded

      \todo Should deal with things like symbolic links but that functionality
      is currently disabled pending file system support.
    */
    int loadOneLib(const std::string &lib, const std::string *dir = 0,
                   PluginUniqueID *uniqueID = 0) ;

    /*! \brief Load and initialise all plugin libraries in the directory.

      \todo
      Unimplemented until we can create platform-independent file
      system support. For that matter, it's questionable whether we want this
      functionality.
    */
    int loadAllLibs(const std::string &pluginDirectory,
                    const InvokeServiceFunc func = NULL) ;

    /*! \brief unload the specified plugin library

      Removes all APIs registered by the specified plugin library, invokes the
      library's Osi2::ExitFunc, and unloads the library.
    */
    int unloadOneLib(const std::string &lib, const std::string *dir = 0) ;

    /*! \brief Shut down the plugin manager

      For each loaded plugin library, invoke the library's exit function and
      unload the library. Then erase the manager's bookkeeping information.
    */
    int shutdown() ;

    //@}

    /*! \name Factory methods

      Methods to create and destroy objects supported by plugins.

      To restrict attention to a particular plugin library, specify the
      \link Osi2::PluginUniqueID unique ID \endlink for that library. A value
      of 0 means no restriction.
    */
    //@{

    /*! \brief Invoked by client to create an object

      Plugins register the objects they can create. This method is invoked by
      the client to request an object. The pointer returned must be cast to the
      appropriate type; this is the client's responsibility.

      A nonzero value for \p libID restricts the action to the specified
      library. If \p libID is 0, it will be set on return to the unique ID
      of the library that satisfied the request.

      \todo An explanation of adapter is needed somewhere, once I (Lou)
      understand it.
    */
    void *createObject(const std::string &apiStr, PluginUniqueID &libID,
                       IObjectAdapter &adapter) ;

    /*! \brief Invoked by client to destroy an object

      This method should be invoked by the client to destroy an object. This
      allows the plugin to specify an arbitrary function which will not
      necessarily be equivalent to a standard C++ destructor. (Suppose the
      plugin really hands out multiple pointers to the same object and keeps a
      usage count. Directly invoking the object's destructor with `delete'
      would not be good.)

      A nonzero value for \p libID restricts the action to the specified
      library.
    */
    int destroyObject(const std::string &apiStr, PluginUniqueID libID,
                      void *victim) ;

    //@}

    /*! \name Plugin manager control methods

      Miscellaneous methods that control the behaviour of the plugin manager.
    */
    //@{

    /// Set the log (verbosity) level
    inline void setLogLvl(int logLvl) {
        logLvl_ = logLvl ;
        msgHandler_->setLogLevel(logLvl_) ;
    }

    /// Get the log (verbosity) level
    inline int getLogLvl() const {
        return (logLvl_) ;
    }

    /*! \brief Set the message handler

      Replaces the current message handler. If newHandler is null, the existing
      handler is replaced with a default handler. It is the responsibility of
      the client to destroy any handler it supplies. The plugin manager takes
      responsibility for a default handler.
    */
    void setMsgHandler(CoinMessageHandler *newHandler) ;

    /// Get the message handler
    inline CoinMessageHandler *getMsgHandler() const {
        return (msgHandler_) ;
    }

    /// Report owner of message handler (false if owned by client).
    inline bool dfltHandler () const {
        return (dfltHandler_) ;
    }

    //@}

private:
    /*! \brief Register an object type with the plugin manager

      Invoked by plugins to register the objects they can create.
    */
    static int32_t registerObject(const CharString *nodeType,
                                  const RegisterParams *params) ;

    /*! \name Constructors and Destructors

      Private because the plugin manager should be a single static instance.
    */
    //@{
    /// Default constructor
    PluginManager() ;
    /// Copy constructor
    PluginManager(const PluginManager &pm) ;
    /// Destructor
    ~PluginManager() ;
    //@}

    /*! \name Utility methods */
    //@{
    /*! \brief Validate registration parameters

      Check the validity of a registration parameter block supplied by
      a plugin to register an API.

      \returns A reference to the dynamic library object for the plugin
      	     library attempting the registration, or null on error.
    */
    DynamicLibrary *validateRegParams(const CharString *apiStr,
                                      const RegisterParams *params) const ;
    /*! \brief Construct an ObjectParams block

      Constructs the parameter block passed to the plugin for object creation
      or destruction.
    */
    ObjectParams *buildObjectParams(const std::string apiStr,
                                    const RegisterParams &rp) ;
    //@}

    /// Partially filled-in platform services record
    PlatformServices platformServices_ ;

    /*! \brief Plugin library management information

      This struct holds the information needed to manage a plugin library.
    */
    struct DynLibInfo {

        /// The dynamic library
        DynamicLibrary *dynLib_ ;
        /// Plugin library state object supplied by plugin (opaque pointer)
        PluginState *ctrlObj_ ;
        /// Exit (cleanup) function for the library; called prior to unload
        ExitFunc exitFunc_ ;

    } ;

    /// Map type to map library path strings to PluginUniqueID
    typedef std::map<std::string, PluginUniqueID> LibPathToIDMap ;

    /*! \brief Plugin library path to ID map

      So that we don't have to work with strings internally, map the full
      path to a more tractable ID.
    */
    LibPathToIDMap libPathToIDMap_ ;

    /// Map type for plugin library management
    typedef std::map<PluginUniqueID, DynLibInfo> DynamicLibraryMap ;

    /*! \brief Plugin library management map

      Maps the unique ID assigned to the plugin library to a block of
      information (#DynLibInfo) used to manage the library.
    */
    DynamicLibraryMap dynamicLibraryMap_ ;

    /// Map type for API management
    typedef std::multimap<std::string, RegisterParams> RegistrationMap ;
    /// Vector type for wildcard management
    typedef std::vector<RegisterParams> RegistrationVec ;

    /*! \brief Check for an existing entry in the API multimaps

      Checks both the key and the plugin unique ID. Returns an iterator for the
      entry if it exists, null otherwise.
    */
    RegistrationMap::iterator apiEntryExists(RegistrationMap &regMap,
            const std::string &key,
            PluginUniqueID libID) ;

    /*! \brief API management information map

      Maps specific APIs registered by plugin libraries to a block of
      information (#APIInfo) used to manage the API.
    */
    RegistrationMap exactMatchMap_ ;
    /*! \brief Wildcard management information

      Records management information for wildcard registrations by plugin
      libraries.
    */
    RegistrationVec wildCardVec_ ;

    /*! \brief Initialising a plugin?

      True during initialisation of a plugin library. Used to determine if plugin
      activity (API registration, etc) should go to temporary structures.
    */
    bool initialisingPlugin_ ;
    /*! \brief The library we're initialising;

      Valid exactly when #initialisingPlugin_ is true.
    */
    DynamicLibrary *libInInit_ ;

    /// Temporary API map used during plugin library initialisation
    RegistrationMap tmpExactMatchMap_ ;
    /// Temporary wildcard vector used during plugin library initialisation
    RegistrationVec tmpWildCardVec_ ;     // wild card ('*') object types

    /// Default plugin directory
    std::string dfltPluginDir_ ;

    /// Indicator; false if the message handler belongs to the client
    bool dfltHandler_ ;
    /// Message handler
    CoinMessageHandler *msgHandler_ ;
    /// Messages
    CoinMessages msgs_ ;
    /// Log (verbosity) level
    int logLvl_ ;

} ;

}  // end namespace Osi2
#endif
