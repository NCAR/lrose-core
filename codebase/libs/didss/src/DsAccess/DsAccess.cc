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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:03:31 $
//   $Id: DsAccess.cc,v 1.4 2016/03/03 18:03:31 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsAccess.cc: Class for controlling access to data.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <didss/DsAccess.hh>
using namespace std;



/**********************************************************************
 * Constructors
 */

DsAccess::DsAccess(const order_t check_order,
		   const bool deny_access_if_not_specific,
		   const bool debug_flag) :
  _debugFlag(debug_flag),
  _checkOrder(check_order),
  _denyAll(false),
  _allowAll(false),
  _denyAccessIfNotSpecific(deny_access_if_not_specific)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsAccess::~DsAccess()
{
  // Do nothing
}


/**********************************************************************
 * isAllowedAccess() - Returns true if the given object is allowed 
 *                     access to the resource, false otherwise.
 */

bool DsAccess::isAllowedAccess(const string &user) const
{
  bool is_allowed = false;
  
  switch (_checkOrder)
  {
  case DENY_ALLOW :
    is_allowed = true;
    
    // Check deny list

    if (_isInDenyList(user))
      is_allowed = false;
    
    // Check allow list

    if (_isInAllowList(user))
	is_allowed = true;
    
    break;
  
  case ALLOW_DENY :
  {
    is_allowed = false;
    
    // Check allow list

    if (_isInAllowList(user))
      is_allowed = true;
    
    // Check deny list

    if (_isInDenyList(user))
      is_allowed = false;
    
    break;
  }
  
  } /* endswitch - _checkOrder */
  
  return is_allowed;
}
  

bool DsAccess::isAllowedAccess(const IPAddress &address) const
{
  // Make sure the address is valid

  if (!address.isValid())
    return false;
  
  // Make sure the address is specific, if requested

  if (_denyAccessIfNotSpecific && !address.isSpecific())
    return false;
  
  // Now check the lists to see if the address is allowed
  // access.

  bool is_allowed = false;
  
  switch (_checkOrder)
  {
  case DENY_ALLOW :
    is_allowed = true;
    
    // Check deny list

    if (_isInDenyList(address))
      is_allowed = false;
    
    // Check allow list

    if (_isInAllowList(address))
	is_allowed = true;
    
    break;
  
  case ALLOW_DENY :
  {
    is_allowed = false;
    
    // Check allow list

    if (_isInAllowList(address))
      is_allowed = true;
    
    // Check deny list

    if (_isInDenyList(address))
      is_allowed = false;
    
    break;
  }
  
  } /* endswitch - _checkOrder */
  
  return is_allowed;
}
  

/**********************************************************************
 * print() - Print the access information to the given stream.
 */

void DsAccess::print(ostream &stream)
{
  stream << "DsAccess information:" << endl;
  stream << "---------------------" << endl;
  stream << "check order: " << checkOrder2String(_checkOrder) << endl;
  stream << "deny all flag: " << _denyAll << endl;
  stream << "allow all flag: " << _allowAll << endl;
  stream << "user allow list: " << endl;
  
  set< string >::iterator user_iter;
  
  for (user_iter = _userAllowList.begin();
       user_iter != _userAllowList.end();
       ++user_iter)
    stream << "   " << *user_iter << endl;
  
  stream << "user deny list: " << endl;
  
  for (user_iter = _userDenyList.begin();
       user_iter != _userDenyList.end();
       ++user_iter)
    stream << "   " << *user_iter << endl;
  
  stream << "deny access if not specific flag: " <<
    _denyAccessIfNotSpecific << endl;
  stream << "IP allow list: " << endl;
  
  set< IPAddress >::iterator ip_iter;
  
  for (ip_iter = _ipAllowList.begin();
       ip_iter != _ipAllowList.end();
       ++ip_iter)
    stream << "   " << *ip_iter << endl;
  
  stream << "IP deny list: " << endl;
  
  for (ip_iter = _ipDenyList.begin();
       ip_iter != _ipDenyList.end();
       ++ip_iter)
    stream << "   " << *ip_iter << endl;
  
  stream << endl;
}


/**********************************************************************
 * PRIVATE METHODS
 **********************************************************************/

/**********************************************************************
 * _isInAllowList() - Returns true if the object is in the appropriate
 *                    allow list, false otherwise.
 */

bool DsAccess::_isInAllowList(const string &user) const
{
  if (_allowAll)
    return true;
  
  set< string >::iterator allowed_user;
      
  for (allowed_user = _userAllowList.begin();
       allowed_user != _userAllowList.end();
       ++allowed_user)
  {
    if (user == *allowed_user)
      return true;
  }

  return false;
}


bool DsAccess::_isInAllowList(const IPAddress &address) const
{
  if (_allowAll)
    return true;
  
  set< IPAddress >::iterator allowed_address;
      
  for (allowed_address = _ipAllowList.begin();
       allowed_address != _ipAllowList.end();
       ++allowed_address)
  {
    if (address.matches(*allowed_address))
      return true;
  }

  return false;
}


/**********************************************************************
 * _isInDenyList() - Returns true if the object is in the appropriate
 *                   deny list, false otherwise.
 */

bool DsAccess::_isInDenyList(const string &user) const
{
  if (_denyAll)
    return true;
  
  set< string >::iterator denied_user;
      
  for (denied_user = _userDenyList.begin();
       denied_user != _userDenyList.end();
       ++denied_user)
  {
    if (user == *denied_user)
      return true;
  }

  return false;
}


bool DsAccess::_isInDenyList(const IPAddress &address) const
{
  if (_denyAll)
    return true;
  
  set< IPAddress >::iterator denied_address;
      
  for (denied_address = _ipDenyList.begin();
       denied_address != _ipDenyList.end();
       ++denied_address)
  {
    if (address.matches(*denied_address))
      return true;
  }

  return false;
}
