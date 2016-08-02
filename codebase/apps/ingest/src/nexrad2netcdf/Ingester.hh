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
///////////////////////////////////////////////////////
// Ingester - class that manages data ingest and 
//            writing data to output
//
// Jaimi Yee, RAP, NCAR, Boulder, CO, 80307, USA
// September 2004
//
// $Id: Ingester.hh,v 1.18 2016/03/07 01:23:10 dixon Exp $
//
//////////////////////////////////////////////////////
#ifndef _INGESTER_INC_
#define _INGESTER_INC_

#include <netcdf.hh>
#include "Status.hh"
#include "NcOutput.hh"

//
// Forward class declarations
//
class Params;
class SweepData;

class Ingester 
{
public:

   //
   // Constructor
   //
   Ingester();

   //
   // Destructor
   //
   ~Ingester();

   //
   // Initialize object
   //   params = input tdrp parameters
   //
   Status::info_t init( Params& params );

   //
   // Add a beam for processing
   //   buffer          = message passed on from input stream
   //   volumeTitleSeen = true if we have seen a start of volume
   //                     indicator
   //
   //   See Status.hh for meaning of return values
   //
   Status::info_t addBeam( ui08* buffer, bool volumeTitleSeen );

   // 
   // Indicate that we are at the end of the data stream.
   // Write out any remaining data
   //
   Status::info_t endOfData();

private:

   //
   // Keeps track of whether this is the first beam 
   // or not
   //
   bool firstBeam;

   //
   // Keep track of how long it takes to create a file
   //
   time_t fileStart;
   time_t fileEnd;

   //
   // Sweeps
   //
   int        prevElevNumber;
   SweepData *currentSweep;
   SweepData *prevSweep;

   //
   // NetCDF output
   //
   NcOutput  *ncOutput;

   //
   // Handle the first beam ever seperately
   //
   Status::info_t handleFirstBeam( RIDDS_data_hdr* nexradData,
                                   time_t beamTime );
   
};

#endif
   
   
