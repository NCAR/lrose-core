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
//   $Date: 2016/03/06 23:53:40 $
//   $Id: ArcHiqReader.cc,v 1.3 2016/03/06 23:53:40 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ArcHiqReader: Class for objects used to read HiQ beam data from an
 *               ARC processor.
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
#include "ArcHiqReader.hh"

using namespace std;


/*********************************************************************
 * Constructor
 */

ArcHiqReader::ArcHiqReader(MsgFactory *msg_factory, const bool debug) :
  HiqReader(msg_factory, debug)
{
}


/*********************************************************************
 * Destructor
 */

ArcHiqReader::~ArcHiqReader() 
{
}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/


/*********************************************************************
 * _getMessageLen() - Get the message length from this message pointer.
 */

int ArcHiqReader::_getMessageLen(char *msg_ptr) const
{
  si32 message_len;
  
  memcpy(&message_len, &msg_ptr[0], sizeof(message_len));
  message_len = SE_to_si32(message_len);

  return message_len;
}

  
/*********************************************************************
 * _isHeader() - Returns true if this message starts with a header,
 *               false otherwise.
 */

bool ArcHiqReader::_isHeader(char *msg_ptr) const
{
  // Check for ARC beam message

  if (msg_ptr[4] == 'D' && msg_ptr[5] == 'W' &&
      msg_ptr[6] == 'L' && msg_ptr[7] == 'X')
    return true;
    
  return false;
}
