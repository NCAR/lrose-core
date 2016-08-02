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
 *   $Date: 2016/03/06 23:53:42 $
 *   $Id: EdgeTcpip.hh,v 1.5 2016/03/06 23:53:42 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * EdgeTcpip : Class for retrieving realtime data from the EDGE system
 *             via TCP/IP sockets.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef EdgeTcpip_HH
#define EdgeTcpip_HH

#include <cstdio>
#include <netinet/in.h>

#include "Edge2Dsr.hh"
#include "TcpipReader.hh"
using namespace std;


class EdgeTcpip
{

public:

  /*********************************************************************
   * Constructors
   */

   EdgeTcpip(const string &edge_host, const int edge_port,
	     const int max_size);


  /*********************************************************************
   * Destructor
   */

   ~EdgeTcpip();


  /*********************************************************************
   * init() - Initialize the information in this object.
   *
   * Returns true if successful, false otherwise.
   */

   bool   init();


  /*********************************************************************
   * readTcpip() - Read the latest information from the TCP/IP interface.
   *
   * Returns the number of bytes in the buffer read in, or -1 if there
   * was an error.
   */

   int    readTcpip();
   

  /*********************************************************************
   * getNextMsg() - Gets the next ready message.  Must only be called
   *                after readTcpip() returns true.
   *
   * Returns the number of bytes in the message and fills in the given
   * message buffer on success.  On error, returns -1.
   */

  int getNextMsg(char *buffer, const int max_buffer_size)
  {
    if (_currReader == 0)
    {
      POSTMSG(ERROR,
	      "Trying to get message when no reader has one");
      
      return -1;
    }
    
    int buffer_len = _currReader->getBufferLen();
    
    if (buffer_len > max_buffer_size)
    {
      POSTMSG(ERROR,
	      "*** EDGE message buffer not big enough!");
      
      return -1;
    }
    
    memcpy((void *)buffer, (void *)_currReader->getBufferPtr(),
	   buffer_len);
    
    return buffer_len;
    
  }
  
   
  /*********************************************************************
   * getBuffer() - Gets a pointer to the buffer of the last message read in.
   */

  char *getBuffer()
  {
    return _buffer;
  }
  

private:

  // Socket numbers used for communications.  These are defined as
  // constants in the EDGE documentation.  If they ever become configurable
  // on the EDGE system, we should move these to the parameter file and
  // set them in the init() method.

  static const int STATUS_SOCKET;
  static const int CORR_REFL_SOCKET;
  static const int UNCORR_REFL_SOCKET;
  static const int VEL_SOCKET;
  static const int SW_SOCKET;
  static const int ALL_SOCKET;
  

  string           _edgeHost;
  
  TcpipReader      _statusReader;
  TcpipReader      _corrReflReader;
  TcpipReader      _uncorrReflReader;
  TcpipReader      _velReader;
  TcpipReader      _swReader;
  TcpipReader      _allReader;
  
  TcpipReader     *_currReader;
  

  int _bufferLen;
  char *_buffer;
  

};

#endif

   
