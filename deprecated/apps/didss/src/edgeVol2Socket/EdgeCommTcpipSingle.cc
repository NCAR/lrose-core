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
//   $Date: 2016/03/06 23:53:42 $
//   $Id: EdgeCommTcpipSingle.cc,v 1.6 2016/03/06 23:53:42 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * EdgeCommTcpipSingle : Class for objects that send EDGE messages
 *                       to a client through a single TCP/IP socket.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cerrno>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include <toolsa/sockutil.h>

#include "EdgeCommTcpipSingle.hh"
using namespace std;


// Define constants

const int EdgeCommTcpipSingle::TCPIP_PORT = 6605;


/*********************************************************************
 * Constructor
 */

EdgeCommTcpipSingle::EdgeCommTcpipSingle(const int port_number) :
  EdgeComm(),
  _portNumber(port_number),
  _serverFd(-1),
  _clientFd(-1),
  _msgBufferAlloc(0),
  _msgBuffer(0)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

EdgeCommTcpipSingle::~EdgeCommTcpipSingle() 
{
  // Close the socket if it is still open

  closeClient();

  if (_serverFd >= 0)
    SKU_close(_serverFd);
  
  // Reclaim the space for the message buffer

  delete _msgBuffer;
}


/*********************************************************************
 * init() - Method to perform any communication initializations required
 *          within the object.  This method must be called before any
 *          other object methods.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool EdgeCommTcpipSingle::init()
{
  static const string method_name = "EdgeCommTcpipSingle::init()";
  
  // Open the server socket and listen for a client

  if ((_serverFd = SKU_open_server(_portNumber)) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening server socket: " << _serverFd << endl;
    switch (_serverFd)
    {
    case -1 :
      cerr << "Could not open socket (max file descriptors reached??)" << endl;
      break;
      
    case -2 :
      cerr << "Could not bind to specified port: " << _portNumber << endl;
      break;
      
    case -3 :
      cerr << "Could not listen on port for some reason" << endl;
      cerr << "SKUerrno = " << SKUerrno << endl;
      break;
      
    default:
      cerr << "Unknown error return value" << endl;
      break;
    } /* endswitch - _serverFd */

    return false;
  }
    
  return true;
}


/*********************************************************************
 * openClient() - Method for opening the EDGE data client.  This method
 *                blocks until a client is opened.
 *
 * Returns true if a client was successfully opened, false otherwise.
 */

bool EdgeCommTcpipSingle::openClient()
{
  static const string method_name = "EdgeCommTcpipSingle::openClient()";
  
  if ((_clientFd = SKU_get_client(_serverFd)) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting TCP/IP client for port: " << _portNumber << endl;
    
    switch (_clientFd)
    {
    case -1 :
      cerr << "Accept call failed" << endl;
      break;
      
    default:
      cerr << "Unknown error returned from SKU call: " << _clientFd << endl;
      break;
    } /* endswitch - _clientFd */

    return false;
  }
  
  // Increase the socket buffer size

  int bufsize = 81920;
  setsockopt(_clientFd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
  
  return true;
}


/*********************************************************************
 * closeClient() - Method for closing the current EDGE data client.
 */

void EdgeCommTcpipSingle::closeClient()
{
  if (_clientFd >= 0)
    SKU_close(_clientFd);
  _clientFd = -1;
}


/*********************************************************************
 * sendMsg() - Send the given message to the current EDGE client.
 *
 * Returns true if the message was successfully sent, false otherwise.
 */

bool EdgeCommTcpipSingle::sendMsg(EdgeMsg &edge_msg)
{
  static const string method_name = "EdgeCommTcpipSingle::sendMsg()";
  
  // Make sure we have a connection to the client

  if (_clientFd < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No connection to client" << endl;
    cerr << "Trying to make new connection..." << endl;
    
    openClient();
  }
  
  // First get the message size so we can make sure our buffer is
  // large enough for the message.

  int buffer_size = edge_msg.getMsgSize();
  
  if (buffer_size > _msgBufferAlloc)
  {
    delete _msgBuffer;
    
    _msgBufferAlloc = buffer_size;
    _msgBuffer = new char[_msgBufferAlloc];
  }
  
  // Now copy the message into the buffer

  if (!edge_msg.writeMsgToBuffer(_msgBuffer))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing message to buffer" << endl;
    
    return false;
  }
  
  // Finally, write the message to the socket.

  int sku_return;
  
  if ((sku_return = SKU_write_select(_clientFd, 1000)) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error returned from write select: " << sku_return << endl;
    
    closeClient();
    
    return false;
  }
  
  int bytes_written;
  
  if ((bytes_written = SKU_write(_clientFd, _msgBuffer, buffer_size, -1))
      != buffer_size)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing message to socket" << endl;
    cerr << "Tried to write " << buffer_size << " bytes, wrote " <<
      bytes_written << " bytes" << endl;
    
    closeClient();
    
    return false;
  }
  
  return true;
}

  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
