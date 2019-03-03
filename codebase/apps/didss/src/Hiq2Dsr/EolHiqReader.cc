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
/*********************************************************************
 * EolHiqReader: Class for objects used to read HiQ beam data from an
 *               EOL processor.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <dataport/smallend.h>
#include <cstring>
#include "EolHiqReader.hh"

using namespace std;


/*********************************************************************
 * Constructor
 */

EolHiqReader::EolHiqReader(MsgFactory *msg_factory, const bool debug) :
  HiqReader(msg_factory, debug)
{
}


/*********************************************************************
 * Destructor
 */

EolHiqReader::~EolHiqReader() 
{
}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getMessageLen() - Get the message length from this message pointer.
 */

int EolHiqReader::_getMessageLen(char *msg_ptr) const
{
  si16 message_len;
  
  memcpy(&message_len, &msg_ptr[4], sizeof(message_len));
  message_len = SE_to_si16(message_len);

  return message_len;
}

  
/*********************************************************************
 * _isHeader() - Returns true if this message starts with a header,
 *               false otherwise.
 */

bool EolHiqReader::_isHeader(char *msg_ptr) const
{
  // Check for EOL beam message

  if (msg_ptr[0] == 'D' && msg_ptr[1] == 'W' &&
      msg_ptr[2] == 'E' && msg_ptr[3] == 'L')
    return true;
    
  // Check for EOL radar message

  if (msg_ptr[0] == 'R' && msg_ptr[1] == 'H' &&
      msg_ptr[2] == 'D' && msg_ptr[3] == 'R')
    return true;
    
  // Check for ARC beam message

  if (msg_ptr[4] == 'D' && msg_ptr[5] == 'W' &&
      msg_ptr[6] == 'L' && msg_ptr[7] == 'X')
    return true;
    
  return false;
}
