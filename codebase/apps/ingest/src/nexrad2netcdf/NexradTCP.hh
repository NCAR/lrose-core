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
//  Note:  Grabbed as is from nexrad2dsr...  Jaimi Yee
//         Sept. 2004
//
//  $Id: NexradTCP.hh,v 1.4 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _NEXRAD_TCP_INC_
#define _NEXRAD_TCP_INC_

#include <string>
#include <cstdio>
#include <toolsa/Socket.hh>

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

   //
   // Standard NEXRAD message size is 2432 bytes, but the ctm information
   // is not included in the TCP/IP stream (as it is in the TAPE and LDM)
   //
   static const size_t NEX_BUFFER_SIZE = NEX_PACKET_SIZE 
                                       - sizeof(RIDDS_ctm_info);
   //
   // At the end of a volume, some extra bytes show up in the TCP/IP stream
   // Haven't quite decifered what is in the extra bytes, but we know the size.
   //
   // Bytes are actually delivered in multiples of EXTRA_BYTES, ie.
   // 100, 200, 300 .....
   //
   // Niles Oien, September 2002.
   //
   static const int EXTRA_BYTES = 100;

   //
   // Maximum number of EXTRA_BYTE size packets. Niles Oien.
   //
   static const int MAX_EXTRA_PACKETS = 5;

   //
   // Input data read from the socket
   //
   ui08 tcpBuffer[NEX_BUFFER_SIZE + MAX_EXTRA_PACKETS * EXTRA_BYTES];

   //
   // Read logical records -- the logical record is a pointer into
   // the physical tcpBuffer
   //
   bool               firstCall;
   ui08              *logicalRecord;

   Status::info_t     readLogicalRecord();

   //
   // Read physical records, i.e. packets of a specified size
   // from the TCP/IP socket
   //
   Status::info_t     readPhysicalRecord( ui08 *bufPtr, int nbytes );

   //
   // TCP/IP socket handling
   //
   Socket             socket;

};

#endif
