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
//   $Date: 2016/03/07 01:23:01 $
//   $Id: MitServer.cc,v 1.4 2016/03/07 01:23:01 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MitServer: Class of objects that serve out MIT/Lincoln Labs data
 *            over a TCP/IP socket.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2002
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <iostream>
#include <string>

#include <toolsa/os_config.h>
#include <dataport/bigend.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "MitServer.hh"
using namespace std;


// Global variables



/*********************************************************************
 * Constructor
 */

MitServer::MitServer(const int port) :
  _port(port)
{
}


/*********************************************************************
 * Destructor
 */

MitServer::~MitServer()
{
  // Delete the client sockets

  vector< Socket* >::iterator client_iter;
  
  for (client_iter = _clientSockets.begin();
       client_iter != _clientSockets.end(); ++client_iter)
    delete *client_iter;
  
}


/*********************************************************************
 * getClients() - Make connections to any clients that are trying to
 *                connect.
 */

void MitServer::getClients()
{
  Socket *client_socket;
  
  while ((client_socket = getClient(1)) != 0)
  {
    cerr << "---> Got new client" << endl;
    
    _clientSockets.push_back(client_socket);
  }
  
}


/*********************************************************************
 * sendMsgToAllClients() - Send the indicated message to all clients.
 */

void MitServer::sendMsgToAllClients(const int message_type,
				    const void *message,
				    const int message_len)
{
  static const string method_name = "MitServer::sendMsgToAllClients()";
  
  // Assemble the message, including the MIT wrapper

  mit_wrapper_t wrapper;
  
  wrapper.rec_type = message_type;
  wrapper.rec_len = sizeof(mit_wrapper_t) + message_len;
  wrapper.rec_seq = 0;
  
  wrapper.rec_type = BE_from_si16(wrapper.rec_type);
  wrapper.rec_len =  BE_from_si16(wrapper.rec_len);
  wrapper.rec_seq =  BE_from_si32(wrapper.rec_seq);
  
  MemBuf msg_buffer;
  msg_buffer.reserve(sizeof(mit_wrapper_t) + message_len);
  msg_buffer.load(&wrapper, sizeof(mit_wrapper_t));
  msg_buffer.add(message, message_len);
  
  // Send the message to all of the clients

  vector< Socket* >::iterator client_iter;
  
  for (client_iter = _clientSockets.begin();
       client_iter != _clientSockets.end(); ++client_iter)
  {
    Socket *client_socket = *client_iter;
    
    // See if the client socket is ready for a write operation

    if (client_socket->writeSelect(100) != 0)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Client socket not ready for write operation" << endl;
      cerr << client_socket->getErrString() << endl;
      cerr << "*** Closing client socket ***" << endl;
      
      client_socket->close();
      delete client_socket;
      _clientSockets.erase(client_iter, client_iter);

      continue;
    }
    
    // Try to send the message

    if (client_socket->writeBuffer(msg_buffer.getBufPtr(),
				   msg_buffer.getBufLen()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing message to client socket" << endl;
      cerr << client_socket->getErrString() << endl;
      cerr << "*** Closing client socket ***" << endl;
      
      client_socket->close();
      delete client_socket;
      _clientSockets.erase(client_iter, client_iter);
      
      continue;
    }
    
  } /* endfor - client_iter */
  
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
