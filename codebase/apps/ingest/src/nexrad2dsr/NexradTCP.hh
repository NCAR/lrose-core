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
////////////////////////////////////////////////////////////////////////////////
//
//  Nexrad sub-class for reading from a TCP/IP socket
//
//  Responsible for reading the input data from a TCP/IP socket
//  and parceling out the radar data one nexrad message buffer at a time.
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  March 2002
//
//  $Id: NexradTCP.hh,v 1.7 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _NEXRAD_TCP_INC_
#define _NEXRAD_TCP_INC_

#include <string>
#include <cstdio>
#include <toolsa/Socket.hh>
#include <rapformats/ridds.h>

#include "Params.hh"
#include "Status.hh"
#include "NexradInput.hh"

using namespace std;


class NexradTCP : public NexradInput
{
public:
  NexradTCP();
  ~NexradTCP();
  
  //
  // Return 0 upon success, -1 upon failure
  //
  int init( Params& params );
  
  //
  // Sub-classes are required to provide a read method
  // which sets a pointer to the buffer.  The input buffer
  // is owned by the sub-class NOT by the caller.
  //
  Status::info_t     readNexradMsg( ui08* &buffer, 
				    bool  &volumeTitleSeen );
  
private:
  
  RIDDS_msg_hdr riddsHeader;
  ui08 headerBuffer[sizeof(RIDDS_msg_hdr)];
  ui08 digitalRadarMessageBuffer[5000]; // More than big enough.

  //
  // TCP/IP socket handling
  //
  Socket             socket;

};

#endif
