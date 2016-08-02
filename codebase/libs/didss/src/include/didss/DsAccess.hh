// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:03:31 $
 *   $Id: DsAccess.hh,v 1.4 2016/03/03 18:03:31 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsAccess.hh: Class for controlling access to data.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2000
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef DsAccess_HH
#define DsAccess_HH

/*
 **************************** includes **********************************
 */

#include <iostream>
#include <set>
#include <string>
#include <cassert>

#include <didss/IPAddress.hh>
using namespace std;


/*
 ************************* class definitions ****************************
 */

class DsAccess
{

public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    DENY_ALLOW,
    ALLOW_DENY
  } order_t;
  
  
  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsAccess(const order_t check_order = DENY_ALLOW,
	   const bool deny_access_if_not_specific = true,
	   const bool debug_flag = false);


  /**********************************************************************
   * Destructor
   */

  virtual ~DsAccess();


  ///////////////////////
  // User list methods //
  ///////////////////////

  /**********************************************************************
   * addUserToAllowList() - Add the given name to the list of users
   *                        explicitly allowed access to the resource.
   */

  inline void addUserToAllowList(const string &user)
  {
    _userAllowList.insert(user);
  }
  
  /**********************************************************************
   * addUserToDenyList() - Add the given name to the list of users
   *                       explicitly denied access to the resource.
   */

  inline void addUserToDenyList(const string &user)
  {
    _userDenyList.insert(user);
  }
  

  /////////////////////////////
  // IP address list methods //
  /////////////////////////////

  /**********************************************************************
   * addIpToAllowList() - Add the given IP address to the list of addresses
   *                      explicitly allowed access to the resource.
   */

  inline void addIpToAllowList(const IPAddress &address)
  {
    _ipAllowList.insert(address);
  }
  
  /**********************************************************************
   * addIpToDenyList() - Add the given IP address to the list of addresses
   *                     explicitly denied access to the resource.
   */

  inline void addIpToDenyList(const IPAddress &address)
  {
    _ipDenyList.insert(address);
  }
  

  ////////////////////////////
  // General access methods //
  ////////////////////////////

  /**********************************************************************
   * getOrder() - Retrieve the order for checking access.
   */

  inline order_t getOrder(void)
  {
    return _checkOrder;
  }
  

  /**********************************************************************
   * setOrder() - Set the order for checking access.
   */

  inline void setOrder(const order_t& order)
  {
    _checkOrder = order;
  }
  

  /**********************************************************************
   * getAllowAllFlag() - Retrieve the allow all flag.
   */

  inline bool getAllowAllFlag(void)
  {
    return _allowAll;
  }
  

  /**********************************************************************
   * setAllowAllFlag() - Set the allow all flag.
   */

  inline void setAllowAllFlag(const bool& allow_all)
  {
    _allowAll = allow_all;
  }
  

  /**********************************************************************
   * getDenyAllFlag() - Retrieve the deny all flag.
   */

  inline bool getDenyAllFlag(void)
  {
    return _denyAll;
  }
  

  /**********************************************************************
   * setDenyAllFlag() - Set the deny all flag.
   */

  inline void setDenyAllFlag(const bool& deny_all)
  {
    _denyAll = deny_all;
  }
  

  /////////////////////////////////
  // Methods for checking access //
  /////////////////////////////////

  /**********************************************************************
   * isAllowedAccess() - Returns true if the given object is allowed 
   *                     access to the resource, false otherwise.
   */

  bool isAllowedAccess(const string &user) const;
  bool isAllowedAccess(const IPAddress &address) const;

  
  ////////////////////
  // Output methods //
  ////////////////////

  /**********************************************************************
   * checkOrder2String() - Convert the given check order value to a string
   *                       for printing.
   */

  static string checkOrder2String(const order_t check_order)
  {
    switch (check_order)
    {
    case DENY_ALLOW :
      return "DENY_ALLOW";
      
    case ALLOW_DENY :
      return "ALLOW_DENY";
    } /* endswitch - check_order */
    
    return "*** Unrecognized check order ***";
  }
  

  /**********************************************************************
   * print() - Print the access information to the given stream.
   */

  void print(ostream &stream);
  

private:

  //////////////////////
  // Private typedefs //
  //////////////////////

  /////////////////////
  // Private members //
  /////////////////////

  bool _debugFlag;
  
  order_t _checkOrder;
  
  bool _denyAll;
  bool _allowAll;
  
  // User access

  set< string > _userAllowList;
  set< string > _userDenyList;
  

  // IP address access

  bool _denyAccessIfNotSpecific;
  
  set< IPAddress > _ipAllowList;
  set< IPAddress > _ipDenyList;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /**********************************************************************
   * _isInAllowList() - Returns true if the object is in the appropriate
   *                    allow list, false otherwise.
   */

  bool _isInAllowList(const string &user) const;
  bool _isInAllowList(const IPAddress &address) const;
  
  
  /**********************************************************************
   * _isInDenyList() - Returns true if the object is in the appropriate
   *                   deny list, false otherwise.
   */

  bool _isInDenyList(const string &user) const;
  bool _isInDenyList(const IPAddress &address) const;

};

#endif /* DsAccess_HH */


