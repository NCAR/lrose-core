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
//   $Id: EdgeTcpip.cc,v 1.7 2016/03/06 23:53:42 dixon Exp $
//   $Revision: 1.7 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * EdgeTcpip : Class for retrieving realtime data from the EDGE system
 *             via TCP/IP sockets.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/sockutil.h>
#include <toolsa/uusleep.h>
#include <toolsa/MsgLog.hh>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif

#include "EdgeTcpip.hh"
#include "Edge2Dsr.hh"
#include "EdgeMsg.hh"
using namespace std;


// Define constants

const int EdgeTcpip::STATUS_SOCKET = 6600;
const int EdgeTcpip::CORR_REFL_SOCKET = 6601;
const int EdgeTcpip::UNCORR_REFL_SOCKET = 6602;
const int EdgeTcpip::VEL_SOCKET = 6603;
const int EdgeTcpip::SW_SOCKET = 6604;
const int EdgeTcpip::ALL_SOCKET = 6605;


/*********************************************************************
 * Constructor
 */

EdgeTcpip::EdgeTcpip(const string &edge_host, const int edge_port,
		     const int max_size) :
  _edgeHost(edge_host),
  _statusReader("status socket", edge_host, STATUS_SOCKET, false),
  _corrReflReader("corr refl socket", edge_host, CORR_REFL_SOCKET, true),
  _uncorrReflReader("uncorr refl socket", edge_host, UNCORR_REFL_SOCKET, true),
  _velReader("vel socket", edge_host, VEL_SOCKET, true),
  _swReader("sw socket", edge_host, SW_SOCKET, true),
  _allReader("all socket", edge_host, edge_port, false),
  _currReader(0)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

EdgeTcpip::~EdgeTcpip() 
{
  // Do nothing
}


/*********************************************************************
 * init() - Initialize the information in this object.
 *
 * Returns true if successful, false otherwise.
 */

bool EdgeTcpip::init() 
{
  //
  // Initialize the reader
  //

//   _statusReader.init();
//   _corrReflReader.init();
//   _uncorrReflReader.init();
//   _velReader.init();
//   _swReader.init();
  
  _allReader.init();
  
  return true;
}


/*********************************************************************
 * readTcpip() - Read the latest information from the TCP/IP interface.
 *
 * Returns the number of bytes in the buffer read in, or -1 if there
 * was an error.
 */

int EdgeTcpip::readTcpip()
{
  // Wait for data on one of the sockets.  If data is received or an
  // error occurs, we will return directly from inside this loop.

  _currReader = 0;
    
  // Wait for a message on the socket

  _allReader.waitForMsg();
  
  if (_allReader.readMsg())
  {	
    _currReader = &_allReader;
    return true;
  }
  
//  while (true)
//  {
//    // Check for a message on any of the sockets
//
//    if (_statusReader.messageReady())
//    {
//      if (_statusReader.readMsg())
//      {
//	_currReader = &_statusReader;
//	return true;
//      }
//    }
//    
//    if (_corrReflReader.messageReady())
//    {
//      if (_corrReflReader.readMsg())
//      {
//	_currReader = &_corrReflReader;
//	return true;
//      }
//    }
//    
////    if (_uncorrReflReader.messageReady())
////    {
////      if (_uncorrReflReader.readMsg())
////      {
////	_currReader = &_uncorrReflReader;
////	return true;
////      }
////    }
//    
//    if (_velReader.messageReady())
//    {
//      if (_velReader.readMsg())
//      {
//	_currReader = &_velReader;
//	return true;
//      }
//    }
//    
//    if (_swReader.messageReady())
//    {
//      if (_swReader.readMsg())
//      {
////	POSTMSG(DEBUG,
////		"Read spectrum width msg");
//	
//	_currReader = &_swReader;
//	return true;
//      }
//    }
//    
//    // If we get here there were no messages on any of the sockets.
//    // Sleep for a second so we don't peg the CPU when the server is
//    // down.
//
//    POSTMSG(DEBUG,
//	    "No messages available on any sockets -- sleeping for 1 second");
//    
//    sleep(1);
//
//    uusleep(10000);
//    
//  } /* endwhile - true */
  
  // We should never get here.  If we do, it's an error.

  return -1;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
