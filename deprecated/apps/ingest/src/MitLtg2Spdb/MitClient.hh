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
 *   $Date: 2016/03/07 01:23:02 $
 *   $Id: MitClient.hh,v 1.6 2016/03/07 01:23:02 dixon Exp $
 *   $Revision: 1.6 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MitClient: Class of clients of MIT/Lincoln Labs data over a TCP/IP
 *            socket.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2002
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MitClient_HH
#define MitClient_HH


#include <string>

#include <dataport/port_types.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/Socket.hh>
using namespace std;


class MitClient
{
 public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    LRTC_LGHT_LLP = 3311,
    LRTC_LGHT_ARSI = 3321,
    LRTC_LGHT_STATUS = 3322
  } msg_types_t;
  
  
  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructor
   */

  MitClient(const string &hostname = "",
	    const int port = 0,
	    const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~MitClient(void);
  

  //////////////////////////
  // Msg handling methods //
  //////////////////////////
 
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

  bool getNextMsg(const int msg_type, const int msg_wait_secs);
  

  /*********************************************************************
   * getMsgBuffer() - Get a pointer to the most recently received
   *                  message information.
   */

  const void *getMsgBuffer()
  {
    return _msgBuffer.getBufPtr();
  }
  

  /*********************************************************************
   * getMsgLen() - Get the length (number of bytes) of the most recently
   *               received message information.
   */

  int getMsgLen()
  {
    return _msgBuffer.getBufLen();
  }
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * setDebugFlag() - Set the debug flag for the object.  The default is
   *                  for the debug flag to be false.
   */

  inline void setDebugFlag(const bool debug_flag)
  {
    _debugFlag = debug_flag;
  }
  

  /*********************************************************************
   * setHostname() - Set the hostname for the connection.
   */

  inline void setHostname(const string &hostname)
  {
    _hostname = hostname;
  }
  

  /*********************************************************************
   * setPort() - Set the port for the connection.
   */

  inline void setPort(const int port)
  {
    _port = port;
  }
  

 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const int MIT_WRAPPER_LEN;
  

  ///////////////////
  // Private types //
  ///////////////////

  typedef struct
  {
    si16 rec_type;
    si16 rec_len;    /* Record length in 2-byte units */
    si32 rec_seq;
  } mit_wrapper_t;
  
  
  /////////////////////
  // Private members //
  /////////////////////

  bool _debugFlag;
  
  string _hostname;
  int _port;
  Socket *_socket;
  
  // Buffer for holding read messages

  MemBuf _msgBuffer;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * _openSocket() - Open the socket with the server.  This method will
   *                 block (while still sending heartbeat messages to
   *                 procmap) until the connection is made.
   */

  void _openSocket(const int sleep_msecs);
  

};


#endif
