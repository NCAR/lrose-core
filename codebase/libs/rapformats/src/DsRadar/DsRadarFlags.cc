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
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// April 1998
//
////////////////////////////////////////////////////////////////////////////////


#include <toolsa/umisc.h>
#include <dataport/bigend.h>
#include <rapformats/DsRadarFlags.hh>
#include <iostream>
using namespace std;

#define BOOL_STR(x) (x? "TRUE" : "FALSE")

DsRadarFlags::DsRadarFlags()
{
   clear();
}

DsRadarFlags::~DsRadarFlags()
{

}

DsRadarFlags&
DsRadarFlags::operator=( const DsRadarFlags &radarFlags )
{
   copy( radarFlags );
   return( *this );
}

void
DsRadarFlags::copy( const DsRadarFlags& radarFlags )
{
   
  if (this == &radarFlags)
    return;

   time          = radarFlags.time;
   volumeNum     = radarFlags.volumeNum;
   tiltNum       = radarFlags.tiltNum;
   scanType       = radarFlags.scanType;
   startOfTilt   = radarFlags.startOfTilt;
   endOfTilt     = radarFlags.endOfTilt;
   startOfVolume = radarFlags.startOfVolume;
   endOfVolume   = radarFlags.endOfVolume;
   newScanType   = radarFlags.newScanType;
   
}

bool
DsRadarFlags::operator==( const DsRadarFlags& otherFlags ) const
{
   if ( time          != otherFlags.time )          return false;
   if ( volumeNum     != otherFlags.volumeNum )     return false;
   if ( tiltNum       != otherFlags.tiltNum )       return false;
   if ( scanType      != otherFlags.scanType )      return false;
   if ( startOfTilt   != otherFlags.startOfTilt )   return false;
   if ( endOfTilt     != otherFlags.endOfTilt )     return false;
   if ( startOfVolume != otherFlags.startOfVolume ) return false;
   if ( endOfVolume   != otherFlags.endOfVolume )   return false;
   if ( newScanType   != otherFlags.newScanType )   return false;

   return( true );
}

void
DsRadarFlags::clear()
{
   
   time          = -1;
   volumeNum     = -1;
   tiltNum       = -1;
   scanType      = -1;
   startOfTilt   = FALSE;
   endOfTilt     = FALSE;
   startOfVolume = FALSE;
   endOfVolume   = FALSE;
   newScanType   = FALSE;
   
}

void
DsRadarFlags::print( FILE *out ) const
{
   
   fprintf(out, "RADAR FLAGS\n");
   
   fprintf(out, "  time:  %s\n", utimstr(time));

   fprintf(out, "  volume number:  %d\n", volumeNum);
   fprintf(out, "  tilt number:  %d\n", tiltNum);
   fprintf(out, "  scan type:  %d\n", scanType);
   
   fprintf(out, "  startOfTilt:  %s\n", BOOL_STR(startOfTilt));
   fprintf(out, "  endOfTilt:  %s\n", BOOL_STR(endOfTilt));
   fprintf(out, "  startOfVolume:  %s\n", BOOL_STR(startOfVolume));
   fprintf(out, "  endOfVolume:  %s\n", BOOL_STR(endOfVolume));
   fprintf(out, "  newScanType:  %s\n", BOOL_STR(newScanType));;
   
   fprintf(out, "\n");
}

void
DsRadarFlags::print( ostream &out ) const
{
   
  out << "RADAR FLAGS" << endl;
   
  out << "  time:  " << utimstr(time) << endl;

  out << "  volume number:  " << volumeNum << endl;
  out << "  tilt number:  " << tiltNum << endl;
  out << "  scan type:  " << scanType << endl;
  
  out << "  startOfTilt:  " << BOOL_STR(startOfTilt) << endl;
  out << "  endOfTilt:  " << BOOL_STR(endOfTilt) << endl;
  out << "  startOfVolume:  " << BOOL_STR(startOfVolume) << endl;
  out << "  endOfVolume:  " << BOOL_STR(endOfVolume) << endl;
  out << "  newScanType:  " << BOOL_STR(newScanType) << endl;

  out << endl;

}

void
DsRadarFlags::decode( DsRadarFlags_t *msg_flags)

{

  DsRadarFlags_t flags = *msg_flags;
  BE_to_DsRadarFlags(&flags);
   
  time          = flags.time;
  
  volumeNum     = flags.vol_num;
  tiltNum       = flags.tilt_num;
  scanType      = flags.scan_type;
  
  startOfTilt   = flags.start_of_tilt;
  endOfTilt     = flags.end_of_tilt;
  
  startOfVolume = flags.start_of_volume;
  endOfVolume   = flags.end_of_volume;
  
  newScanType   = flags.new_scan_type;

}

void
DsRadarFlags::encode( DsRadarFlags_t *msg_flags)
{

   //
   // Encode the flags header
   //

   memset( msg_flags, 0, sizeof(DsRadarFlags_t) );

   msg_flags->time            = time;
   msg_flags->vol_num         = volumeNum;
   msg_flags->tilt_num        = tiltNum;
   msg_flags->scan_type       = scanType;

   msg_flags->start_of_tilt   = startOfTilt;
   msg_flags->end_of_tilt     = endOfTilt;

   msg_flags->start_of_volume = startOfVolume;
   msg_flags->end_of_volume   = endOfVolume;

   msg_flags->new_scan_type   = newScanType;

   BE_from_DsRadarFlags(msg_flags);

}

