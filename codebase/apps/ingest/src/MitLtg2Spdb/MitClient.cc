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
//   $Date: 2016/03/07 01:23:02 $
//   $Id: MitClient.cc,v 1.10 2016/03/07 01:23:02 dixon Exp $
//   $Revision: 1.10 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MitClient: Class of clients of MIT/Lincoln Labs data over a TCP/IP
 *            socket.
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
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "MitClient.hh"
using namespace std;


// Global variables

const int MitClient::MIT_WRAPPER_LEN = 8;


/*********************************************************************
 * Constructor
 */

MitClient::MitClient(const string &hostname,
		     const int port,
		     const bool debug_flag) :
  _debugFlag(debug_flag),
  _hostname(hostname),
  _port(port),
  _socket(0)
{
}


/*********************************************************************
 * Destructor
 */

MitClient::~MitClient()
{
}


/*********************************************************************
 * getNextMsg() - Check to see if there is a message of the given type
 *                available from the server.  If the message type is -1,
 *                any message type will be returned.  Will wait up to
 *                msg_wait_secs seconds for a message to be received
 *                from the server.
 *
 * Returns true if there is a message of the given type available from
 * the server.  In this case, use the getMsgBuffer() and getMsgLen()
 * methods to access the actual message information.
 *
 * Returns false if there are no messages available from the server
 * or if the next message available from the server is of a different
 * type than the one requested.
 */

bool MitClient::getNextMsg(const int msg_type, const int msg_wait_secs)
{
  static const string method_name = "MitClient::getNextMsg()";
  
  const int msg_wait_msecs = msg_wait_secs * 1000;
  
  // Make sure we have an open socket

  if (_socket == 0)
    _openSocket(msg_wait_msecs);
  
  // Wait for a message on the socket.  If there isn't one, return
  // false.

  if (_socket->readSelect(msg_wait_msecs) != 0)
    return false;


  // Read the message wrapper.  Read it into a character buffer
  // and then copy the pieces into the wrapper structure to handle
  // any places in the structure that aren't word aligned.  Currently,
  // the wrapper fields should be word aligned, but since the messages
  // themselves aren't this seemed like the safest way to handle the
  // wrapper, too, in case there are future changes here.
  
  char *wrapper_buffer = new char[MIT_WRAPPER_LEN];
    
  if (_socket->readBuffer(wrapper_buffer, MIT_WRAPPER_LEN) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading message wrapper from server" << endl;
    cerr << _socket->getErrString() << endl;
      
    _openSocket(msg_wait_msecs);
      
    return false;
  }
    
  // Copy the wrapper information into the wrapper structure.
  // See above comment.

  mit_wrapper_t wrapper;
  char *wrapper_ptr = wrapper_buffer;
    
  memcpy(&wrapper.rec_type, wrapper_ptr, sizeof(wrapper.rec_type));
  wrapper_ptr += sizeof(wrapper.rec_type);
    
  memcpy(&wrapper.rec_len, wrapper_ptr, sizeof(wrapper.rec_len));
  wrapper_ptr += sizeof(wrapper.rec_len);
    
  memcpy(&wrapper.rec_seq, wrapper_ptr, sizeof(wrapper.rec_seq));
  wrapper_ptr += sizeof(wrapper.rec_len);

  if (_debugFlag)
  {
    cerr << "Before swapping: " << endl;
    cerr << "   wrapper.rec_type = " << wrapper.rec_type << endl;
    cerr << "   wrapper.rec_len = " << wrapper.rec_len << endl;
    cerr << "   wrapper.rec_seq = " << wrapper.rec_seq << endl;
  }
  
  // Unswap the wrapper information

  wrapper.rec_type = BE_to_si16(wrapper.rec_type);
  wrapper.rec_len =  BE_to_si16(wrapper.rec_len);
  wrapper.rec_seq =  BE_to_si32(wrapper.rec_seq);
  
  if (_debugFlag)
  {
    cerr << "After swapping: " << endl;
    cerr << "   wrapper.rec_type = " << wrapper.rec_type << endl;
    cerr << "   wrapper.rec_len = " << wrapper.rec_len << endl;
    cerr << "   wrapper.rec_seq = " << wrapper.rec_seq << endl;
  }
  
  // Read the message itself

  int msg_len = (wrapper.rec_len * 2) - MIT_WRAPPER_LEN;
  char *msg = new char[msg_len];
    
  if (_debugFlag)
    cerr << "---> Trying to read message data..." << endl;
  
  if (_socket->readBuffer(msg, msg_len) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading message info from server" << endl;
    cerr << _socket->getErrString() << endl;
      
    _openSocket(msg_wait_msecs);
      
    delete msg;
    
    return false;
  }
    
  if (_debugFlag)
    cerr << "     Message successfully read" << endl;
  
  if (msg_type == -1 ||
      wrapper.rec_type == msg_type)
  {
    if (_debugFlag)
      cerr << "**** Received desired message type" << endl;
    
    _msgBuffer.reserve(msg_len);
    _msgBuffer.load(msg, msg_len);
   
    delete msg;
    
    return true;
  }

  // If we get here, it was the wrong message type so we are ignoring
  // it.

  if (_debugFlag)
    cerr << "---> Ignoring message of type: " << wrapper.rec_type << endl;
    
  delete msg;
    
  return false;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _openSocket() - Open the socket with the server.  This method will
 *                 block (while still sending heartbeat messages to
 *                 procmap) until the connection is made.
 */

void MitClient::_openSocket(const int sleep_msecs)
{
  static const string method_name = "MitClient::_openSocket()";
  
  // Create a new socket object

  delete _socket;
  _socket = new Socket();
  
  // Now open the connection

  bool warning_printed = false;
  
  while (_socket->open(_hostname.c_str(), _port, sleep_msecs) != 0)
  {
    PMU_auto_register("Trying to connect to server");
    
    if (!warning_printed)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Cannot make connection to server socket" << endl;
      cerr << _socket->getErrString() << endl;

      warning_printed = true;
    }
    
  }

//  if (_debugFlag)
    cerr << "*** Made connection to server ***" << endl;
}
