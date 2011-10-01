
#include <cstring>
#include <cassert>
#include <string>
#include <iostream>

/*
  #include "Osi2Directory.hpp"
  #include "Osi2Path.hpp"
*/
#include "Osi2PluginManager.hpp"
#include "Osi2DynamicLibrary.hpp"
#include "Osi2ObjectAdapter.hpp"

#ifndef OSI2PLUGINDIR
# define OSI2DFLTPLUGINDIR "/usr/local/lib"
#else
# define OSI2DFLTPLUGINDIR OSI2PLUGINDIR
#endif

#if defined(OSI2PLATFORM_MAC)
  static std::string dynamicLibraryExtension("dylib") ;
#elif defined(OSI2PLATFORM_LINUX)
  static std::string dynamicLibraryExtension("so") ;
#elif defined(OSI2PLATFORM_WINDOWS)
  static std::string dynamicLibraryExtension("dll") ;
#endif

// Protection against C++0x
const int nullptr = 0 ;


using namespace Osi2 ;

/*
  Plugin manager constructor.
*/
PluginManager::PluginManager()
  : initialisingPlugin_(false),
    dfltHandler_(true),
    logLvl_(7)
{
  msgHandler_ = new CoinMessageHandler() ;
  msgs_ = PlugMgrMessages() ;
  msgHandler_->setLogLevel(logLvl_) ;
  msgHandler_->message(PLUGMGR_INIT,msgs_) << CoinMessageEol ;
  dfltPluginDir_ = std::string(OSI2DFLTPLUGINDIR) ;
  platformServices_.version_.major_ = 1 ;
  platformServices_.version_.minor_ = 0 ;
  platformServices_.dfltPluginDir_ =
      reinterpret_cast<const CharString*>(dfltPluginDir_.c_str()) ;
  // can be populated during loadAll()
  platformServices_.invokeService_ = (nullptr) ;
  platformServices_.registerObject_ = registerObject ;
}

/*
  Plugin manager destructor

  Call shutdown to close out the plugins, then destroy the pieces of ourself.
*/
PluginManager::~PluginManager ()
{
  // Just in case it wasn't called earlier
  shutdown() ;
  /*
    If this is our handler, delete it. Otherwise it's the client's
    responsibility.
  */
  if (dfltHandler_) {
    delete msgHandler_ ;
    msgHandler_ = nullptr ;
  }
}

/*
  There's no particular reason to trust a plugin, so we need to do some
  validation of the parameters it gives to the plugin manager when registering
  an API.

  \parm Registration string supplied by plugin
  \parm Registration parameter block supplied by plugin

  \returns Pointer to the DynamicLibrary struct for this library, or
  	   null on error.
*/

DynamicLibrary *PluginManager::validateRegParams (const CharString *apiStr,
					 const RegisterParams *params) const
{
  bool retval = true ;
  std::string errStr = "" ;
  DynamicLibrary *dynLib = nullptr ;

  // The API to be registered must have a name
  if (!apiStr || !(*apiStr)) {
    errStr += "; bad API ID string" ;
    retval = false ;
  }

  // If we're missing the parameter block, nothing more to say.
  if (!params) {
    errStr += "; no registration parameter block" ;
    retval = false ;
  } else {
    /*
      The pluginID must be valid. If we're initialising the library, it will
      not appear in dynamicLibraryMap.
    */
    if (initialisingPlugin_) {
      if (libInInit_ != params->pluginID_) {
	errStr += "; bad plugin library ID" ;
	retval = false ;
      } else {
        dynLib = libInInit_ ;
      }
    } else {
      DynamicLibraryMap::const_iterator dlmIter ;
      for (dlmIter = dynamicLibraryMap_.begin() ;
	   dlmIter != dynamicLibraryMap_.end() ;
	   dlmIter++)
      {
	if (params->pluginID_ == dlmIter->second.dynLib_) break ;
      }
      if (dlmIter == dynamicLibraryMap_.end()) {
	errStr += "; bad plugin library ID" ;
	retval = false ;
      } else {
	dynLib = dlmIter->second.dynLib_ ;
      }
    }
    // There must be a constructor
    if (!params->createFunc_) {
      errStr += "; missing constructor" ;
      retval = false ;
    }
    // There must be a destructor
    if (!params->destroyFunc_) {
      errStr += "; missing destructor" ;
      retval = false ;
    }
    // The major version must match
    PluginAPIVersion ver = platformServices_.version_ ;
    if (ver.major_ != params->version_.major_) {
      msgHandler_->message(PLUGMGR_BADVER,msgs_)
	<< params->version_.major_ << ver.major_ << CoinMessageEol ;
      errStr += "; version mismatch" ;
      retval = false ;
    }
  }

  if (!retval) {
    errStr = errStr.substr(2) ;
    msgHandler_->message(PLUGMGR_BADAPIPARM,msgs_) << errStr << CoinMessageEol ;
  }
  
  return (dynLib) ;
}
/*
  This method is used by the plugin to register the APIs it supports. A pointer
  to this method is passed to the plugin in a PlatformServices object.

  Returns 0 for success, -1 for failure.
*/
int32_t PluginManager::registerObject (const CharString *apiStr,
				       const RegisterParams *params)
{
  PluginManager &pm = getInstance() ;

  // Validate the parameter block
  DynamicLibrary *dynLib = pm.validateRegParams(apiStr,params) ;
  if (dynLib == nullptr) return (-1) ;
/*
  If the registration is a wild card, add it to the wild card vector. If the
  registration is for a specific API, check before adding to the exact match
  vector --- duplicates are not allowed.

  If this call comes while we're initialising a plugin, add the APIs to
  temporary vectors for merge if initialisation is successful.

  Note that we cannot just store a pointer to the RegisterParams block;
  there's no telling how the plugin is handling it and we don't get ownership.
*/
  std::string key((const char *) apiStr) ;
  int retval = 0 ;

  if (key == std::string("*")) {
    if (pm.initialisingPlugin_) {
      pm.tmpWildCardVec_.push_back(*params) ;
    } else {
      pm.wildCardVec_.push_back(*params) ;
    }
  } else {
    RegistrationMap::const_iterator oldAPI = pm.exactMatchMap_.find(key) ;
    if (oldAPI != pm.exactMatchMap_.end()) {
      retval = -1 ;
    } else
    if (pm.initialisingPlugin_) {
      oldAPI = pm.tmpExactMatchMap_.find(key) ;
      if (oldAPI != pm.tmpExactMatchMap_.end()) {
	retval = -1 ;
      }
    }
    if (retval) {
      pm.msgHandler_->message(PLUGMGR_REGDUPAPI,pm.msgs_)
	  << key << CoinMessageEol ;
    } else {
      if (pm.initialisingPlugin_) {
	pm.tmpExactMatchMap_[key] = *params ;
      } else {
	pm.exactMatchMap_[key] = *params ;
      }
    }
  }
  if (!retval)
    pm.msgHandler_->message(PLUGMGR_REGAPIOK,pm.msgs_)
	<< key << dynLib->getLibPath() << CoinMessageEol ;
  return (retval) ; 
}

/*
  Instantiate a single instance of the manager.
*/
PluginManager &PluginManager::getInstance()
{
  static PluginManager instance ;
  
  return (instance) ;
}


/*
  Load a plugin given a full path to the plugin.

  Returns:
    -3: library failed to initialise
    -2: failed to find initFunc
    -1: library failed to load
     0: library loaded and initialised without error


  \todo Implement platform-independent path handling.
  \todo Implement resolution of symbolic links.
  \todo Implement conversion to absolute path.
*/
int PluginManager::loadOneLib (const std::string &lib, const std::string *dir)
{
/*
  Construct the full path for the library.

  Should this be converted to a standardised absolute path? That'd avoid issues
  with different ways of specifying the same library.
*/
    std::string fullPath ;
    if (dir == nullptr || (dir->compare("") == 0)) {
      fullPath += getDfltPluginDir() ;
    } else {
      fullPath += *dir ;
    }
    fullPath += "/"+lib ;
/*
  Is this library already loaded? If so, don't do it again.
*/
    if (dynamicLibraryMap_.find(fullPath) != dynamicLibraryMap_.end()) {
      msgHandler_->message(PLUGMGR_LIBLDDUP,msgs_)
	<< fullPath << CoinMessageEol ;
      return (1) ;
    }
/*
  Attempt to load the library. A null return indicates failure. Report the
  problem and return an error.
*/
    std::string errStr ;
    DynamicLibrary *dynLib = DynamicLibrary::load(fullPath, errStr) ;
    if (!dynLib) {
      msgHandler_->message(PLUGMGR_LIBLDFAIL,msgs_)
	<< fullPath << errStr << CoinMessageEol ;
      return (-1) ;
    }
/*
  Find the initialisation function "initPlugin". If this entry point is missing
  from the library, we're in trouble.
*/
    InitFunc initFunc = (InitFunc)(dynLib->getSymbol("initPlugin",errStr)) ;
    if (!initFunc) {
      msgHandler_->message(PLUGMGR_SYMLDFAIL,msgs_)
	<< "function" << "initPlugin" << fullPath << errStr << CoinMessageEol ;
      return (-2) ;
    }
/*
  Invoke the initialisation function, which will (in the typical case) trigger
  the registration of the various APIs implemented in this plugin library. We
  should get back the exit function for the library.

  Set initialisingPlugin_ to true so that the APIs will be registered into the
  temporary wildcard and exact match vectors. If initialisation is successful,
  we'll transfer them to the permanent vectors.
*/
  initialisingPlugin_ = true ;
  libInInit_ = dynLib ;
  tmpExactMatchMap_.clear() ;
  tmpWildCardVec_.clear() ;
  platformServices_.dfltPluginDir_ =
      reinterpret_cast<const CharString*>(dfltPluginDir_.c_str()) ;
  platformServices_.pluginID_ = dynLib ;
  std::cout
    << "Plugin ID passed to plugin is " << platformServices_.pluginID_
    << std::endl ;
  ExitFunc exitFunc = initFunc(&platformServices_) ;
  if (!exitFunc) {
      msgHandler_->message(PLUGMGR_LIBINITFAIL,msgs_)
	<< fullPath << CoinMessageEol ;
    /// Should unload library here!
    initialisingPlugin_ = false ;
    return (-3) ;
  }
  msgHandler_->message(PLUGMGR_LIBINITOK,msgs_)
	<< fullPath << CoinMessageEol ;
/*
  We have happiness: the library is loaded and initialised. Do the
  bookkeeping.  Enter the library in the library map. Add the exit function
  to the vector of exit functions, and copy information from the temporary
  wildcard and exact match vectors to the permanent vectors.
*/
  DynLibInfo &info = dynamicLibraryMap_[fullPath] ;
  info.dynLib_ = dynLib ; 
  info.exitFunc_ = exitFunc ;
  exactMatchMap_.insert(tmpExactMatchMap_.begin(),tmpExactMatchMap_.end()) ;
  tmpExactMatchMap_.clear() ;
  wildCardVec_.insert(wildCardVec_.end(),
		      tmpWildCardVec_.begin(),tmpWildCardVec_.end()) ;
  tmpWildCardVec_.clear() ;
  initialisingPlugin_ = false ;
    
  msgHandler_->message(PLUGMGR_LIBLDOK,msgs_) << fullPath << CoinMessageEol ;

  return (0) ;
}


/*
 We're not going to implement loadAll functionality in the first round.
 Make this method into a guaranteed failure, so we'll catch it when it's
 called.
*/

int PluginManager::loadAllLibs (const std::string &libDir,
				InvokeServiceFunc func)
{
  assert(false) ;

  return (-1) ;
}

#if defined(OSI2_IMPLEMENT_LOADALL)

int32_t PluginManager::loadAll(const std::string &libDir,
				   InvokeServiceFunc func)
{
  if (libDir.empty()) // Check that the path is non-empty.
    return -1 ;

  platformServices_.invokeService = func ;

  Path dir_path(libDir) ;
  if (!dir_path.exists() || !dir_path.isDirectory())
    return -1 ;

  Directory::Entry e ;
  Directory::Iterator di(dir_path) ;
  while (di.next(e))
  {
    Path full_path(dir_path + Path(e.path)) ;

    // Skip directories
    if (full_path.isDirectory())
      continue ;

    // Skip files with the wrong extension
    std::string ext = std::string(full_path.getExtension()) ;
    if (ext != dynamicLibraryExtension)
      continue ;

    // Ignore return value
    /*int32_t res = */ loadOneLib(std::string(full_path)) ;
  }

  return 0 ;
}
#endif		// OSI2_IMPLEMENT_LOADALL

/*
  Run through the exit functions of the loaded plugins and execute each one.
  Then clear out the maps in the manager.

  \todo: The exit functions can throw? Why isn't there a catch block for all
	 the others (init function, etc.)
*/
int PluginManager::shutdown()
{
  int overallResult = 0 ;

  for (DynamicLibraryMap::iterator dlmIter = dynamicLibraryMap_.begin() ;
       dlmIter != dynamicLibraryMap_.end() ;
       dlmIter++)
  { int result = 0 ;
    bool threwError = false ;
    ExitFunc func = dlmIter->second.exitFunc_ ;
    DynamicLibrary *dynLib = dlmIter->second.dynLib_ ;
    try
    {
      result = (*func)() ;
    }
    catch (...)
    {
      // std::cout << "Whoa! Exit function threw!" << std::endl ;
      threwError = true ;
    }
    if (threwError || result != 0) {
      msgHandler_->message(PLUGMGR_LIBEXITFAIL,msgs_)
          << dynLib->getLibPath() << CoinMessageEol ;
      overallResult-- ;
    } else {
      msgHandler_->message(PLUGMGR_LIBEXITOK,msgs_)
          << dynLib->getLibPath() << CoinMessageEol ;
    }
  }
  /*
    Clear out the maps. Before we clear dynamicLibraryMap_, go through and
    delete the DynamicLibrary objects; the destructor will unload the library.
    The other vectors hold actual objects rather than pointers.
  */
  for (DynamicLibraryMap::iterator dlmIter = dynamicLibraryMap_.begin() ;
       dlmIter != dynamicLibraryMap_.end() ;
       dlmIter++) {
    DynamicLibrary *dynLib = dlmIter->second.dynLib_ ;
    msgHandler_->message(PLUGMGR_LIBCLOSE,msgs_)
      << dynLib->getLibPath() << CoinMessageEol ;
    delete dynLib ;
  }
  dynamicLibraryMap_.clear() ;
  exactMatchMap_.clear() ;
  wildCardVec_.clear() ;
  
  return (overallResult) ;
}

/*
  Replace the current handler with a new handler. The current handler may or
  may not be our responsibility. If newHandler is null, create a default
  handler. (We can't be without one; the code isn't prepared for that.)
*/
void PluginManager::setMsgHandler (CoinMessageHandler *newHandler)
{
  if (dfltHandler_) {
    delete msgHandler_ ;
    msgHandler_ = nullptr ;
  }
  if (newHandler) {
    dfltHandler_ = false ;
    msgHandler_ = newHandler ;
  } else {
    dfltHandler_ = true ;
    msgHandler_ = new CoinMessageHandler() ;
    msgHandler_->setLogLevel(logLvl_) ;
  }
}


#ifdef UNDEFINED

/*
  Presumably this avoids requiring the client to cast to the appropriate type.
  Might be worth exploring.  -- lh, 110929 --
*/
template <typename T, typename U>
T *PluginManager::createObject(const std::string & apiStr, IObjectAdapter<T, U> & adapter)
{
  // "*" is not a valid object type
  if (apiStr == std::string("*"))
    return (nullptr) ;
  
  // Prepare object params
  ObjectParams np ;
  np.apiStr = apiStr.c_str() ;
  np.platformServices = &ServiceProvider::getInstance() ;

  // Exact match found
  if (exactMatchMap_.find(apiStr) != exactMatchMap_.end())
  {        
    RegisterParams & rp = exactMatchMap_[apiStr] ;
    IObject * object = createObject(rp, np, adapter) ;
    if (object) // great, it worked
      return object ;
  }
  
  for (Size i = 0; i < wildCardVec_.size(); ++i)
  {
    RegisterParams & rp = wildCardVec_[i] ;
    IObject * object = createObject(rp, np, adapter) ;
    if (object) // great, it worked
    {
      // promote registration to exactMatc_ 
      // (but keep also the wild card registration for other object types)
      int32_t res = registerObject(np.apiStr, &rp) ;
      if (res < 0)
      {
        THROW << "PluginManager::createObject(" << np.apiStr << "), registration failed" ;
        delete object ;
        return (nullptr) ;
      }
      return object ;
    }      
  }

  // Too bad no one can create this objectType
  return (nullptr) ;
}
#endif

// ---------------------------------------------------------------


void *PluginManager::createObject (const std::string &apiStr,
				   IObjectAdapter &adapter)
{
/*
  "*" is not a valid object type --- some qualification is needed.
*/
  if (apiStr == std::string("*")) return (nullptr) ;
  
/*
  Set up a parameter block to pass to the plugin (assuming we find one
  that's suitable.
*/
  ObjectParams np ;
  np.apiStr_ = reinterpret_cast<const CharString *>(apiStr.c_str()) ;
  np.platformServices_ = &platformServices_ ;

/*
  Check for an exact (string) match. If so, add the plugin's management object
  to the parameter block and ask for an object. If we're successful, we need
  one last step for a C plugin --- wrap it for C++ use.
*/
  if (exactMatchMap_.find(apiStr) != exactMatchMap_.end()) {        
    RegisterParams &rp = exactMatchMap_[apiStr] ;
    np.ctrlObj_ = rp.ctrlObj_ ;
    void *object = rp.createFunc_(&np) ;
    if (object) {
      if (rp.lang_ == Plugin_C)
        object = adapter.adapt(object, rp.destroyFunc_) ;
      return (object) ; 
    }
  }
/*
  No exact match. Try for a wildcard match. If some plugin volunteers an
  object, register 

  Surely we could do better here than just blindly calling create functions?
  Maybe not. It's not really all that different from having a separate `Can
  you do this?' method, given the feedback where the manager adds the requested
  string to the exactMatchMap via the registerObject method. Note that the
  entry in the wildCardVec is not removed.
  -- lh, 110913 --
*/
  for (size_t i = 0 ; i < wildCardVec_.size() ; ++i) {
    RegisterParams &rp = wildCardVec_[i] ;
    np.ctrlObj_ = rp.ctrlObj_ ;
    void *object = rp.createFunc_(&np) ;
    if (object) {
      if (rp.lang_ == Plugin_C)
        object = adapter.adapt(object,rp.destroyFunc_) ;
      int32_t res = registerObject(np.apiStr_,&rp) ;
      if (res < 0) {
	std::cout
	  << "Eh? Can't register exact match entry for object with API "
	  << np.apiStr_ << " successfully created as wildcard."
	  << std::endl ;
        rp.destroyFunc_(object,&np) ;
        return (nullptr) ;
      }
      return (object) ;
    }
  }
/*
  No plugin volunteered. We can't create this object.
*/
  std::cout
    << "No plugin was able to create an object with API "
    << np.apiStr_ << "." << std::endl ;
  return (nullptr) ;
}


PlatformServices &PluginManager::getPlatformServices ()
{
  return (platformServices_) ;
}


