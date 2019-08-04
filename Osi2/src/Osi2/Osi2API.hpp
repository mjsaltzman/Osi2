/*
  Copyright 2011 Lou Hafer, Matt Saltzman
  This code is licensed under the terms of the Eclipse Public License (EPL)

  $Id$
*/
/*! \file Osi2API.hpp

  Defines the class Osi2::API, the top of the Osi2 API hierarchy.
*/

#ifndef Osi2API_HPP
# define Osi2API_HPP

/*! \brief Identification namespace for OSI2.

  All APIs and their implementations should be defined within the Osi2
  namespace. Also used to distinguish the implementation of Osi2.
*/
namespace Osi2 {

/*! \brief OSI2 %API virtual base class

  The general philosophy of the %API hierarchy is that the APIs are defined by
  abstract classes. Concrete implementations derive from the abstract classes.
  
  When used within the OSI2 plugin framework, it is expected that
  instantiations of %API objects will be managed using the ControlAPI
  interface.

  The API class forms the root of the OSI2 %API hierarchy. Its raison d'etre
  is to provide a common pointer for %API objects handled by the
  \link ControlAPI::createObject createObject \endlink and
  \link ControlAPI::destroyObject destroyObject \endlink methods of
  ControlAPI.  It also defines an interface to inquire about the
  APIs supported by an object and a hook where a control %API can record
  information for managing the %API object.
*/
class API {

public:

  /// Default constructor
  API ()
    : ctrlInfo_(0)
  { } ;

  /*! \brief Copy constructor

    C++ programmers will tend to use clone without thinking about it.
    Semantically, this is a problem because the clone was not created by
    ControlAPI::createObject (or some other control %API) and we cannot
    simply pass on the \link ctrlInfo_ control information \endlink.
    We need this copy constructor to avoid copying the control information
    pointer to the cloned object.
  */
  API (const API &orig)
    : ctrlInfo_(0)
  { } ;

  /// Virtual destructor.
  virtual ~API () { }

  /*! \name Object control support

    Set or retrieve object control information. See the comments for
    #ctrlInfo_.
  */
  //@{
    /// Set the object control information block
    inline void setCtrlInfo (const void *ctrlInfo)
    { ctrlInfo_ = ctrlInfo ; }

    /// Retrieve the object control information block
    inline const void *getCtrlInfo () const { return (ctrlInfo_) ; }
  //@}

  /*! \name Object API inquiry support

    The following methods provide a way to interrogate an object to find
    out what APIs the object implements and how to access those APIs.

    The "ident" string for an %API should be defined by its XxxAPI class.
    Supporting classes should define a static method
    \code{.cpp}
      char *XxxAPI::getAPIIDString()
    \endcode
    to return the ident string for the %API.
  */
  //@{

  /*! \brief Fetch the capabilities vector.

    The capabilities vector returned in \p idents should be a simple array
    of "ident" strings, one for each API supported by this object.

    An implementation class can choose to keep a static vector, or construct a
    reply dynamically. A return value of 0 means no %API support information
    is available.
  */
  virtual int getAPIs(const char **&idents) { return (0) ; }

  /*! \brief Return an object that can be used as type XxxAPI.

    The job of the access function getAPIPtr is to return an object that
    can be cast to the type XxxAPI corresponding to "ident". A return value
    of null means no such object exists.

    If the invoking object inherits from XxxAPI, simply return the invoking
    object; getAPIPtr is simply certifying it can be used as XxxAPI.
    Otherwise, getAPIPtr should return a pointer to an object that can be
    used as an object of class XxxAPI.

    To leave all options open, do not assume that the returned object inherits
    from API.
  */
  virtual void *getAPIPtr (const char *ident) { return (nullptr) ; }
  //@}

private:

    /*! \brief %API object control information

      This is an opaque pointer for exclusive use of the control %API
      (Osi2::ControlAPI or other control %API).  The exact content depends
      on the implementation of the control %API and the underlying plugin
      manager.
    */
    const void *ctrlInfo_ ;

} ;

} // end namespace Osi2

#endif
