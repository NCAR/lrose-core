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
////////////////////////////////////////////////////////////////////////
//  Class for managing the flow of data
//
//  Jaimi Yee, RAP, NCAR, Boulder, CO, 80307, USA.
//  July 2004
//  
//  Adapted from nexrad2dsr application by Terri Betancourt 
//  RAP, NCAR, Boulder, CO, 80307, USA
//
//  $Id: DataMgr.hh,v 1.7 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////
#ifndef _DATA_MGR_INC_
#define _DATA_MGR_INC_

#include <vector>

#include "Params.hh"
#include "Status.hh"
#include "NexradInput.hh"
#include "Ingester.hh"
using namespace std;

class DataMgr
{
public:

   //
   // Constructor
   //
   DataMgr();

   //
   // Destructor
   //
  ~DataMgr();

   //
   // Return 0 upon success, -1 upon failure
   //
   int init( Params& params );

   //
   // Return I/O status from processing the radar data
   //
   Status::info_t processData();

private:

   //
   // Radar data stream
   //
   NexradInput              *inputStream;

   //
   // Ingester
   //   Manages all the i/o after the input stream
   //
   Ingester                  ingester;

   //
   // Radar processing
   //
   bool                      firstCall;
   Status::info_t            inputStatus;
   Status::info_t            outputStatus;
   Status::info_t            tiltStatus;

};

#endif




