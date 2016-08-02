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
 *   $Id: IPAddress.hh,v 1.4 2016/03/03 18:03:31 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * IPAddress.hh: Class representing an IP address, used by the
 *               DsAccess class.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2000
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef IPAddress_HH
#define IPAddress_HH

/*
 **************************** includes **********************************
 */

#include <string>
#include <cassert>
using namespace std;


/*
 ************************* class definitions ****************************
 */

class IPAddress
{

public:

  //////////////////////
  // Public constants //
  //////////////////////

  static const int MATCHES_ALL;
  

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  IPAddress(const int part0 = 0,
	    const int part1 = 0,
	    const int part2 = 0,
	    const int part3 = 0,
	    const bool debug_flag = false);


  /**********************************************************************
   * Destructor
   */

  virtual ~IPAddress();


  /**********************************************************************
   * setFromString() - Set the IP address values using the given string.
   *
   * Returns true if the address was successfully parsed, false otherwise.
   */

  bool setFromString(const string &address_string);


  ////////////////////////
  // Validation methods //
  ////////////////////////

  /**********************************************************************
   * isValid() - Returns true if the IP address is valid, false otherwise.
   *             Valid IP address can contain wild cards.
   */

  inline bool isValid(void) const
  {
    for (int i = 0; i < NUM_ADDRESS_PARTS; ++i)
    {
      if (_addressParts[i] != MATCHES_ALL &&
	  (_addressParts[i] < 0 || _addressParts[i] > 255))
	return false;
    } /* endfor - i */
    
    return true;
  }
  

  /**********************************************************************
   * isSpecific() - Returns true if the IP address is valid and doesn't
   *                include any wildcards, false otherwise.
   */

  inline bool isSpecific(void) const
  {
    for (int i = 0; i < NUM_ADDRESS_PARTS; ++i)
    {
      if (_addressParts[i] < 0 || _addressParts[i] > 255)
	return false;
    } /* endfor - i */
    
    return true;
  }
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * matches() - Returns true if the given IP address matches this one,
   *             false otherwise.  This method differs from the == operator
   *             in that wild cards are handled properly.
   */

  inline bool matches(const IPAddress& rhs) const
  {
    for (int i = 0; i < NUM_ADDRESS_PARTS; ++i)
    {
      if (_addressParts[i] != MATCHES_ALL &&
	  rhs._addressParts[i] != MATCHES_ALL &&
	  _addressParts[i] != rhs._addressParts[i])
	return false;
    } /* endfor - i */
    
    return true;
  }

  
  ///////////////
  // Operators //
  ///////////////

  inline bool operator==(const IPAddress& rhs) const
  {
    for (int i = 0; i < NUM_ADDRESS_PARTS; ++i)
    {
      if (_addressParts[i] != rhs._addressParts[i])
	return false;
    } /* endfor - i */
    
    return true;
  }
  
  inline bool operator<(const IPAddress& rhs) const
  {
    for (int i = 0; i < NUM_ADDRESS_PARTS; ++i)
    {
      if (_addressParts[i] < rhs._addressParts[i])
	return true;
    } /* endfor - i */
    
    return false;
  }
  
  inline bool operator<=(const IPAddress& rhs) const
  {
    if (*this > rhs)
      return false;
    
    return true;
  }
  
  inline bool operator>(const IPAddress& rhs) const
  {
    for (int i = 0; i < NUM_ADDRESS_PARTS; ++i)
    {
      if (_addressParts[i] > rhs._addressParts[i])
	return true;
    } /* endfor - i */
    
    return false;
  }
  
  inline bool operator>=(const IPAddress& rhs) const
  {
    if (*this < rhs)
      return false;
    
    return true;
  }
  

  friend ostream &operator<<(ostream &output, const IPAddress &address);
  
  
private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const string CLASS_NAME;
  
  static const int NUM_ADDRESS_PARTS;
  

  /////////////////////
  // Private members //
  /////////////////////

  bool _debugFlag;
  
  int *_addressParts;
  
  // Members used to parse IP address strings

  char **tokens;
  
  static const int MAX_TOKENS;
  static const int MAX_TOKEN_LEN;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /**********************************************************************
   * _setAllParts() - Set all of the address parts to the given value.
   */
  
  inline void _setAllParts(const int part_value);
  
};


#endif /* IPAddress_HH */
