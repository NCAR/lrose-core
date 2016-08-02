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
//   $Id: IPAddress.cc,v 1.6 2016/03/03 18:03:31 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * IPAddress.cc: Class representing an IP address, used by the
 *               DsAccess class.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <didss/IPAddress.hh>
#include <toolsa/str.h>
#include <iostream>
#include <cstdlib>
using namespace std;


/*
 * Define global constants
 */

// public constants

const int IPAddress::MATCHES_ALL = -1;


// private constants

const string IPAddress::CLASS_NAME = "IPAddress";

const int IPAddress::NUM_ADDRESS_PARTS = 4;

const int IPAddress::MAX_TOKENS = 10;
const int IPAddress::MAX_TOKEN_LEN = 20;


/**********************************************************************
 * Constructors
 */

IPAddress::IPAddress(const int part0,
		     const int part1,
		     const int part2,
		     const int part3,
		     const bool debug_flag) :
  _debugFlag(debug_flag)
{
  _addressParts = new int[NUM_ADDRESS_PARTS];
  
  _addressParts[0] = part0;
  _addressParts[1] = part1;
  _addressParts[2] = part2;
  _addressParts[3] = part3;

  // Allocate space for token parsing members

  tokens = new char*[MAX_TOKENS];
  
  for (int i = 0; i < MAX_TOKENS; ++i)
    tokens[i] = new char[MAX_TOKEN_LEN];
}


/**********************************************************************
 * Destructor
 */

IPAddress::~IPAddress()
{
  // Reclaim space for address parts

  delete _addressParts;
  
  // Reclaim space for token parsing members

  for (int i = 0; i < MAX_TOKENS; ++i)
    delete tokens[i];
  
  delete tokens;
}


/**********************************************************************
 * setFromString() - Set the IP address values using the given string.
 *
 * Returns true if the address was successfully parsed, false otherwise.
 */

bool IPAddress::setFromString(const string &address_string)
{
  const string method_name = CLASS_NAME + string("::setFromString()");
  
  // Parse the address into tokens

  int num_tokens;
  
  if ((num_tokens = STRparse_delim((char *)address_string.c_str(), tokens,
				   address_string.length(), ".",
				   MAX_TOKENS, MAX_TOKEN_LEN))
      > 4)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing IP address string: " << address_string << endl;
    cerr << "Too many parts" << endl;
    
    return false;
  }
  
  _setAllParts(MATCHES_ALL);
  
  for (int token_num = 0, part_num = NUM_ADDRESS_PARTS-1;
       token_num < num_tokens;
       ++token_num, --part_num)
  {
    if (strlen(tokens[token_num]) == 0)
      _addressParts[part_num] = MATCHES_ALL;
    else
      _addressParts[part_num] = atoi(tokens[token_num]);
  } /* endfor - token_num, part_num */
  
  return true;
}


/**********************************************************************
 * PRIVATE METHODS
 **********************************************************************/

/**********************************************************************
 * _setAllParts() - Set all of the address parts to the given value.
 */

void IPAddress::_setAllParts(const int part_value)
{
  for (int i = 0; i < NUM_ADDRESS_PARTS; ++i)
    _addressParts[i] = part_value;
}


/**********************************************************************
 * FRIEND METHODS
 **********************************************************************/

ostream &operator<<(ostream &output, const IPAddress &address)
{
  for (int i = 0; i < IPAddress::NUM_ADDRESS_PARTS; ++i)
  {
    if (address._addressParts[i] == IPAddress::MATCHES_ALL)
      output << "*";
    else
      output << address._addressParts[i];
    
    if (i < IPAddress::NUM_ADDRESS_PARTS - 1)
      output << ".";
  }
  
  return output;
}
