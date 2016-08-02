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
//  Abstract base class for radar input stream
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: NexradInput.hh,v 1.8 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _NEXRAD_INPUT_INC_
#define _NEXRAD_INPUT_INC_

#include <rapformats/ridds.h>

#include "Params.hh"
#include "Status.hh"
using namespace std;


class NexradInput
{
public:
   NexradInput(){ volumeTitleSeen = false; }
   virtual ~NexradInput(){};

   //
   // Optional initialization for the sub-classes
   // Return 0 upon success, -1 upon failure
   //
   virtual int init( Params& params ){ return 0; }

   //
   // Sub-classes are required to provide a read method
   // which sets a pointer to the buffer.  The input buffer
   // is owned by the sub-class NOT by the caller.
   //
   virtual Status::info_t readNexradMsg( ui08* &buffer, 
                                         bool &volumeTitleSeen ) = 0;

protected:

   //
   // Flag for indicating if a volume title record has been seen
   // during the process of reading a nexrad message
   // This is used ultimately by the reformatter to provide an
   // an additional check for startOfVolume since the nexrad stream
   // is not completely reliable in setting the radial_status
   //
   bool volumeTitleSeen;

};

#endif
