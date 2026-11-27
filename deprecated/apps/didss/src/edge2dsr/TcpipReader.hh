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
/************************************************************************
 * TcpipReader : Class for retrieving realtime data from the EDGE system
 *             via TCP/IP sockets.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef TcpipReader_HH
#define TcpipReader_HH

#include <cstdio>
#include <ctime>
#include <netinet/in.h>
using namespace std;

class TcpipReader
{

public:

  /*********************************************************************
   * Constructors
   */

   TcpipReader(const string &socket_name,
	       const string &host_name,
	       const int port_num,
	       const bool check_missing_beams);


  /*********************************************************************
   * Destructor
   */

   ~TcpipReader();


  /*********************************************************************
   * init() - Initialize the information in this object.
   *
   * Returns true if successful, false otherwise.
   */

   bool   init();


  /*********************************************************************
   * readMsg() - Try to read a message from the socket.  Returns true if
   *             a full message has been read, false otherwise.
   */

  bool readMsg();
  
   
  /*********************************************************************
   * getSocketFd() - Returns the file descriptor for the socket.  If the
   *                 socket isn't currently open, tries to open the socket.
   *                 Returns -1 if the socket cannot be successfully
   *                 opened.
   */

  int getSocketFd()
  {
    if (_socketFd >= 0)
      return _socketFd;
    
    _socketFd = _openSocket(_hostName, _portNum);
    
    return _socketFd;
  }
  
   
  /*********************************************************************
   * closeSocket() - Close the socket associated with this reader.  This
   *                 is generally done if there was an error of some kind
   *                 detected on the socket.
   */

  void closeSocket();
  
   
  /*********************************************************************
   * getSocketName() - Returns the name assigned to the socket for error
   *                   message purposes.
   */

  string &getSocketName()
  {
    return _socketName;
  }
  
   
  /*********************************************************************
   * getBufferLen() - Returns the size of the message buffer after a
   *                  message has been successfully read in.
   */

  int getBufferLen()
  {
    return _currMsgSize;
  }
  
   
  /*********************************************************************
   * getBufferPtr() - Returns a pointer to the message buffer after a
   *                  message has been successfully read in.
   */

  char *getBufferPtr()
  {
    return _currMsg;
  }
  
   
  /*********************************************************************
   * messageReady() - Check to see if there is a message ready on this
   *                   reader.
   *
   * Returns true if there is a message ready, false otherwise.
   */

  bool messageReady();


  /*********************************************************************
   * waitForMsg() - Block waiting for the next message from this socket.
   */

  void waitForMsg();
  

private:

  static const int HEADER_SIZE;
  static const int SOCKET_HUNG_SECS;

  string           _socketName;
  string           _hostName;
  int              _portNum;
  
  int              _socketFd;
  
  char            *_currMsg;
  int              _currMsgAlloc;
  int              _currMsgSize;
  
  mutable time_t   _lastMsgTime;
  
  double           _prevAzimuth;
  bool             _checkMissingBeams;
  

  /*********************************************************************
   * _openSocket() - Open a client socket on the EDGE host using the
   *                 specified port.
   *
   * On success, returns the file descriptor for the opened socket.
   * Otherwise, returns -1.
   */

  int _openSocket(const string &host_name, const int port_number) const;
  

  /***************************************************************************
   * _readBytes() - Read the bytes on the socket.
   */

  bool _readBytes(const int target_size);
  

};

#endif

   
