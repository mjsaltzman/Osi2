/*
  Copyright 2019 Lou Hafer, Matt Saltzman
  This code is licensed under the terms of the Eclipse Public License (EPL)
*/

#include <cstring>

#include "Osi2Config.h"

#include "Osi2ParamBEAPIMessages.hpp"

namespace Osi2 {

/// Convenience structure for message definition.

typedef struct {
    /// Internal ID; used to specify a message
    ParamBEAPIMsg intID_ ;

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
} OneParamBEAPIMessage ;

/*
  Definitions of messages.  To add a new message:
    * Choose a unique ID. Add it to the enum in Osi2ParamBEAPIMessages.hpp
    * Choose an external ID number from the appropriate range (information,
      warning, nonfatal or fatal error).
    * Define the message in the appropriate place in the list by adding the
      initialisation expression for a OneParamBEAPIMessage structure.

  Don't put anything after DUMMY_END. It's a marker used in the constructor.
*/
static OneParamBEAPIMessage us_english[] = {

    // Information: 0 -- 2999

    { PMBEAPI_INIT, 0, 7, "Parameter back end API constructor (%s)." },

    // Warning: 3000 -- 5999

    // Nonfatal Error: 6000 -- 8999

    // Fatal Error: 9000 -- 9999

    { PMBEAPI_DUMMY_END, 9999, 0, "" }
} ;

/*
  Custom constructor. Really all we're doing here is loading each
  OneParamBEAPIMessage, in order according to internal ID, into a
  CoinOneMessage and installing it in the messages array.

  class_ is a gross hack. Arguably, we're class 2. See CoinMessageHandler.hpp.

  The string stored in source_ is limited to four chars (plus terminating
  null).
*/
ParamBEAPIMessages::ParamBEAPIMessages (Language language)
    : CoinMessages(sizeof(us_english) / sizeof(OneParamBEAPIMessage))
{
    language_ = language ;
    std::strcpy(source_, "PMgmtBE") ;
    class_ = 2 ;

    OneParamBEAPIMessage *msg = &us_english[0] ;
    while (msg->intID_ != PMBEAPI_DUMMY_END) {
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

