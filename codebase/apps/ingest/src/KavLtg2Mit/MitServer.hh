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
 *   $Date: 2016/03/07 01:23:01 $
 *   $Id: MitServer.hh,v 1.3 2016/03/07 01:23:01 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MitServer: Class of objects that serve out MIT/Lincoln Labs data
 *            over a TCP/IP socket.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2002
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MitServer_HH
#define MitServer_HH


#include <string>
#include <vector>

#include <dataport/port_types.h>
#include <toolsa/Socket.hh>
#include <toolsa/ServerSocket.hh>
using namespace std;


class MitServer : public ServerSocket
{
 public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    LRTC_LGHT_ARSI = 3321,
    LRTC_LGHT_STATUS = 3322
  } msg_types_t;
  
  
  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructor
   */

  MitServer(const int port = 0);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~MitServer(void);
  

  /////////////////////////////
  // Client handling methods //
  /////////////////////////////

  /*********************************************************************
   * getClients() - Make connections to any clients that are trying to
   *                connect.
   */

  void getClients();


  //////////////////////////
  // Msg handling methods //
  //////////////////////////

  /*********************************************************************
   * sendMsgToAllClients() - Send the indicated message to all clients.
   */

  void sendMsgToAllClients(const int message_type,
			   const void *message,
			   const int message_len);
  

 private:

  ///////////////////
  // Private types //
  ///////////////////

  typedef struct
  {
    si16 rec_type;
    si16 rec_len;
    si32 rec_seq;
  } mit_wrapper_t;
  
  
  /////////////////////
  // Private members //
  /////////////////////

  int _port;
  
  vector< Socket* > _clientSockets;
  

  /////////////////////
  // Private methods //
  /////////////////////

};


#endif
