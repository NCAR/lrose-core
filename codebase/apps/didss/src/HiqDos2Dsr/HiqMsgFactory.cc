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
 * HiqMsgFactory : Class representing a message from the HiQ radar processor.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <string>

#include "HiqMsgFactory.hh"
using namespace std;


/*********************************************************************
 * createMessage() - Create a HiqMsg object using the given buffer
 *                   of data.
 */

HiqMsg *HiqMsgFactory::createMessage(const char *buffer,
				     const int buffer_len)
{
  static const string method_name = "HiqMsgFactory::createMessage()";

  HiqMsg *message = 0;

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
 * _createBeamMessage() - Create a HiqBeamMsg object using the given
 *                        buffer of data.
 */

HiqBeamMsg *HiqMsgFactory::_createBeamMessage(const char *buffer,
						const int buffer_len)
{
  HiqBeamMsg *message = new HiqBeamMsg();
  message->init(buffer);

  return message;
}


/*********************************************************************
 * _createRadarMessage() - Create a HiqRadarMsg object using the given
 *                         buffer of data.
 */

HiqRadarMsg *HiqMsgFactory::_createRadarMessage(const char *buffer,
						const int buffer_len)
{
  HiqRadarMsg *message = new HiqRadarMsg();
  message->init(buffer);

  return message;
}
