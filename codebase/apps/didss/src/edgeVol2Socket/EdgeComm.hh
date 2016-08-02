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
 *   $Id: EdgeComm.hh,v 1.3 2016/03/06 23:53:42 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * EdgeComm : Base class for objects that send EDGE messages through a
 *            socket or group of sockets.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef EdgeComm_HH
#define EdgeComm_HH


#include "EdgeMsg.hh"
using namespace std;


class EdgeComm
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  EdgeComm();


  /*********************************************************************
   * Destructor
   */

  virtual ~EdgeComm();


  /*********************************************************************
   * init() - Method to perform any communication initializations required
   *          within the object.  This method must be called before any
   *          other object methods.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  virtual bool init() = 0;


  /*********************************************************************
   * openClient() - Method for opening the EDGE data client.  This method
   *                blocks until a client is opened.
   *
   * Returns true if a client was successfully opened, false otherwise.
   */

  virtual bool openClient() = 0;


  /*********************************************************************
   * closeClient() - Method for closing the current EDGE data client.
   */

  virtual void closeClient() = 0;
  

  /*********************************************************************
   * sendMsg() - Send the given message to the current EDGE client.
   *
   * Returns true if the message was successfully sent, false otherwise.
   */

  virtual bool sendMsg(EdgeMsg &edge_msg) = 0;
  

protected:

};

#endif

   
