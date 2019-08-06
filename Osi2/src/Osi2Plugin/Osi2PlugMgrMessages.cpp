/*
  Copyright 2011 Lou Hafer, Matt Saltzman
  This code is licensed under the terms of the Eclipse Public License (EPL)
*/

#include <cstring>

#include "Osi2PlugMgrMessages.hpp"

namespace Osi2 {

/// Convenience structure for message definition.

typedef struct {
    /// Internal ID; used to specify a message
    PlugMgrMsg intID_ ;

    /*! \brief External ID; printed in the message prefix

      Also tied to severity; see CoinMessageHandler
    */
    int extID_ ;

    /*! Message log level

      Message will print if log level is less than or equal to the current
      log level.
    */
    char logLvl_ ;

    /// Message text (may contain format codes).
    const char *text_ ;
} OnePlugMgrMessage ;

/*
  Definitions of messages.  To add a new message:
    * Choose a unique ID. Add it to the enum in Osi2PlugMgrMessages.hpp
    * Choose an external ID number from the appropriate range (information,
      warning, nonfatal or fatal error).
    * Define the message in the appropriate place in the list by adding the
      initialisation expression for a OnePlugMgrMessage structure.

  Don't put anything after DUMMY_END. It's a marker used in the constructor.
*/
static OnePlugMgrMessage us_english[] = {
  // Information: 0 -- 2999
  { PLUGMGR_INIT, 0, 7, "Plugin Manager initialising." },
  { PLUGMGR_LIBLDOK, 1, 4, "Loaded %s library \"%s\"." },
  { PLUGMGR_LIBINITOK, 2, 4, "Initialised %s library \"%s\"." },
  { PLUGMGR_LIBEXITOK, 3, 4, "Shut down plugin library \"%s\"." },
  { PLUGMGR_LIBCLOSE, 4, 4, "Unloading plugin library \"%s\"." },
  { PLUGMGR_APIREGOK, 10, 5,
      "Registered API \"%s\" for plugin library \"%s\"." },
  { PLUGMGR_APIUNREG, 11, 5,
      "Unregistered API \"%s\" for plugin library \"%s\"." },
  { PLUGMGR_APICREATEOK, 12, 5, "Created object \"%s\" (%s)." },
  { PLUGMGR_APIDELOK, 13, 5, "Destroyed object \"%s\"." },

  // Warning: 3000 -- 5999
  { PLUGMGR_LIBLDDUP, 3000, 3, "Plugin library \"%s\" is already loaded." },
  { PLUGMGR_LIBNOTFOUND, 3001, 3,
    "Library \"%s\" is not loaded as \"%s\" or \"%s\"." },

  // Nonfatal Error: 6000 -- 8999

  { PLUGMGR_LIBLDFAIL, 6000, 1,
      "Load failed for plugin library \"%s\"; error \"%s\"." },
  { PLUGMGR_LIBINITFAIL, 6001, 1,
      "Initialisation failed for plugin library \"%s\"." },
  { PLUGMGR_LIBEXITFAIL, 6002, 1,
      "Shutdown failed for plugin library \"%s\"." },
  { PLUGMGR_SYMLDFAIL, 6020, 1,
      "Failed to find %s \"%s\" in plugin library \"%s\", error \"%s\"." },
  { PLUGMGR_APICREATEFAIL, 6030, 1, "Failed to create API \"%s\"; %s." },
  { PLUGMGR_APIDELFAIL, 6031, 1, "Failed to destroy API \"%s\"; %s." },

  { PLUGMGR_APIREGDUP, 6051, 1, "API \"%s\" is already registered." },
  { PLUGMGR_BADVER, 6052, 1,
      "Plugin version %d does not match Manager version %d." },
  { PLUGMGR_APIBADPARM, 6053, 1, "Invalid API registration parameters: %s." },

  // Fatal Error: 9000 -- 9999
  { PLUGMGR_DUMMY_END, 9999, 0, "" }
} ;

/*
  Custom constructor. Really all we're doing here is loading each
  OnePlugMgrMessage, in order according to internal ID, into a CoinOneMessage
  and installing it in the messages array.

  class_ is a gross hack. Arguably, we're class 2. See CoinMessageHandler.hpp.

  The string stored in source_ is limited to four chars (plus terminating
  null).
*/
PlugMgrMessages::PlugMgrMessages (Language language)
    : CoinMessages(sizeof(us_english) / sizeof(OnePlugMgrMessage))
{
    language_ = language ;
    std::strcpy(source_, "PMgr") ;
    class_ = 2 ;

    OnePlugMgrMessage *msg = &us_english[0] ;
    while (msg->intID_ != PLUGMGR_DUMMY_END) {
        CoinOneMessage tmpMsg(msg->extID_, msg->logLvl_, msg->text_) ;
        addMessage(msg->intID_, tmpMsg) ;
        msg++ ;
    }
    /*
      If we want to allow language-specific overrides of individual messages,
      code to do that should go here, prior to compacting the messages.
    */
    toCompact() ;
}

}  // end namespace Osi2

