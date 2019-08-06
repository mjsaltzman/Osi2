/*
  Copyright 2019 Lou Hafer, Matt Saltzman
  This code is licensed under the terms of the Eclipse Public License (EPL)
*/

#include <cstring>

#include "Osi2Config.h"

#include "Osi2ParamMgmtAPIMessages.hpp"

namespace Osi2 {

/// Convenience structure for message definition.

typedef struct {
    /// Internal ID; used to specify a message
    ParamMgmtAPIMsg intID_ ;

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
} OneParamMgmtAPIMessage ;

/*
  Definitions of messages.  To add a new message:
    * Choose a unique ID. Add it to the enum in Osi2ParamMgmtAPIMessages.hpp
    * Choose an external ID number from the appropriate range (information,
      warning, nonfatal or fatal error).
    * Define the message in the appropriate place in the list by adding the
      initialisation expression for a OneParamMgmtAPIMessage structure.

  Don't put anything after DUMMY_END. It's a marker used in the constructor.
*/
static OneParamMgmtAPIMessage us_english[] = {

    // Information: 0 -- 2999

    { PMMGAPI_INIT, 0, 7, "Parameter Management API constructor (%s)." },

    // Warning: 3000 -- 5999

    // Nonfatal Error: 6000 -- 8999

    { PMMGAPI_NOSUPPORT, 6000, 2,
      "Ident \"%s\": object does not support parameter management." },
    { PMMGAPI_DUPIDENT, 6001, 2, "Ident \"%s\": duplicate ident string." },
    { PMMGAPI_UNREG, 6001, 3, "Ident \"%s\": ident string not registered." },
    { PMMGAPI_PARMUNREG, 6002, 3,
      "Ident \"%s\": parameter \"%s\" not registered." },
    { PMMGAPI_OPFAIL, 6003, 3,
      "Ident \"%s\": %s failed for parameter \"%s\"." },

    // Fatal Error: 9000 -- 9999

    { PMMGAPI_NOPLUGMGR, 9000, 1, "Cannot find plugin manager!" },

    { PMMGAPI_DUMMY_END, 9999, 0, "" }
} ;

/*
  Custom constructor. Really all we're doing here is loading each
  OneParamMgmtAPIMessage, in order according to internal ID, into a
  CoinOneMessage and installing it in the messages array.

  class_ is a gross hack. Arguably, we're class 2. See CoinMessageHandler.hpp.

  The string stored in source_ is limited to four chars (plus terminating
  null).
*/
ParamMgmtAPIMessages::ParamMgmtAPIMessages (Language language)
    : CoinMessages(sizeof(us_english) / sizeof(OneParamMgmtAPIMessage))
{
    language_ = language ;
    std::strcpy(source_, "PMgmt") ;
    class_ = 2 ;

    OneParamMgmtAPIMessage *msg = &us_english[0] ;
    while (msg->intID_ != PMMGAPI_DUMMY_END) {
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

