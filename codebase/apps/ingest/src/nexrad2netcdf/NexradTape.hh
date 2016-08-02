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
//  Nexrad sub-class for reading from TAPE
//
//  Responsible for reading the input data from TAPE
//  and parceling out the radar data one nexrad message buffer at a time.
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  Note:  Grabbed as is from nexrad2dsr...  Jaimi Yee
//         Sept. 2004
//
//  $Id: NexradTape.hh,v 1.4 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _NEXRAD_TAPE_INC_
#define _NEXRAD_TAPE_INC_

#include "Params.hh"
#include "Status.hh"
#include "NexradInput.hh"
using namespace std;


class NexradTape : public NexradInput
{
public:
   NexradTape();
  ~NexradTape();

   //
   // Return 0 upon success, -1 upon failure
   //
   int init( Params& params );

   //
   // Sub-classes are required to provide a read method
   // which sets a pointer to the buffer.  The input buffer
   // is owned by the sub-class NOT by the caller.
   //
   Status::info_t      readNexradMsg( ui08* &buffer,
                                      bool  &volumeTitleSeen );

private:

   bool  firstTime;
   bool  shortTape;
   bool  remoteDevice;
   int   tapeId;

   //
   // Start with a physical record of fixed length.
   // This is where the actual data will reside.
   //
   static const size_t  MAX_REC_LEN = 65536;
   ui08                 physicalRecord[MAX_REC_LEN];

   //
   // Read physical records from a tape device
   // Upon success returns number of bytes read in the reference argument
   // Upon failure returns -1
   //
   Status::info_t       readPhysicalRecord( size_t& physicalBytes );
   int                  readRemoteDevice();
   int                  readLocalDevice();

   //
   // Index into the physical record to create logical records.
   // The logical record is only a pointer into the physical record.
   //
   size_t               bytesLeft;
   size_t               byteOffset;
   ui08                *logicalRecord;

   //
   // Establish a logical record
   //
   Status::info_t        readLogical( size_t& logicalBytes );
   Status::info_t        readLogicalShort();

};

#endif
