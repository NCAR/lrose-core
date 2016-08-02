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

#include <rapformats/ds_radar.h>
#include "ridds2mom.h"
using namespace std;

void
start_of_tilt( int tilt_num, time_t beamTime )
{
   //
   // Write the startOfTilt only to the radar queue, not the archive queue
   //
   if ( Glob->params.write_fmq_output ) {
      Glob->radarQueue->putStartOfTilt( tilt_num, beamTime );
   }
}

void
end_of_tilt( int tilt_num )
{
   //
   // Write the endOfTilt only to the radar queue, not the archive queue
   //
   if ( Glob->params.write_fmq_output ) {
      Glob->radarQueue->putEndOfTilt( tilt_num );
   }                                              
}
 
void                                
start_of_volume( int vol_num, time_t beamTime )                     
{
   //
   // Write the startOfVolume only to the radar queue, not the archive queue
   //
   if ( Glob->params.write_fmq_output ) {      
      Glob->radarQueue->putStartOfVolume( vol_num, beamTime );
   }
}

void
end_of_volume( int vol_num )
{
   //
   // Write a special EndOfFile marker to the archive queue
   //
   if ( Glob->params.write_archive_fmq ) {
      Glob->archiveQueue->writeMsg( DsFmq::eof );
   }

   //
   // Write an EndOfVolume to the radar queue
   //
   if ( Glob->params.write_fmq_output ) {
      Glob->radarQueue->putEndOfVolume( vol_num );
   }
}

void
new_scan_type(int scan_type)
{
   //
   // Write the new scan type only to the radar queue, not the archive queue
   //
    if ( Glob->params.debug ) {
      fprintf(stderr, "\nFmq message: change scan strategy %d\n", scan_type );
    }

   if ( Glob->params.write_fmq_output ) {
      Glob->radarQueue->putNewScanType(scan_type);
   }
}

