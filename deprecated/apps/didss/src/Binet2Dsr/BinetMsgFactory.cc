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
//   $Date: 2016/03/06 23:53:39 $
//   $Id: BinetMsgFactory.cc,v 1.3 2016/03/06 23:53:39 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * BinetMsgFactory : Class representing a message from the Binet radar processor.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <string>

#include "BinetMsgFactory.hh"
using namespace std;


/*********************************************************************
 * createMessage() - Create a BinetMsg object using the given buffer
 *                   of data.
 */

BinetMsg *BinetMsgFactory::createMessage(const char *buffer,
					 const int buffer_len)
{
  static const string method_name = "BinetMsgFactory::createMessage()";

  BinetMsg *message = 0;

  switch (buffer[0])
  {
  case 'D' :
    message = _createBeamMessage(buffer, buffer_len);
    break;

  case 'R' :
    message = _createRadarMessage(buffer, buffer_len);
    break;

  default:
    cerr << "ERROR: " << method_name << endl;
    cerr << "Invalid message type: " << (int)buffer[0];
    if (isprint(buffer[0]))
      cerr << " (" << buffer[0] << ")";
    cerr << endl;
    break;
  }

  return message;
}


/*********************************************************************
 * _createBeamMessage() - Create a BinetBeamMsg object using the given
 *                        buffer of data.
 */

BinetBeamMsg *BinetMsgFactory::_createBeamMessage(const char *buffer,
						  const int buffer_len)
{
  BinetBeamMsg *message = new BinetBeamMsg();
  message->init(buffer);

  return message;
}


/*********************************************************************
 * _createRadarMessage() - Create a BinetRadarMsg object using the given
 *                         buffer of data.
 */

BinetRadarMsg *BinetMsgFactory::_createRadarMessage(const char *buffer,
						    const int buffer_len)
{
  BinetRadarMsg *message = new BinetRadarMsg();
  message->init(buffer);

  return message;
}
