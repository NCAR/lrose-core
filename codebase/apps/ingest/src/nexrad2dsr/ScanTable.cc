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
//  Class for managing a keyed table (dictionary) of scan types
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: ScanTable.cc,v 1.12 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include "Driver.hh"
#include "ScanTable.hh"
using namespace std;

//
// Initialization of NEXRAD scan types, a.k.a. VCP (volume control patterns)
// Elevation list for each scan type
//
const float ScanTable::vcp32[7]  = { 0.5, 0.5, 1.5, 1.5, 2.5, 3.5, 4.5 };

const float ScanTable::vcp31[8]  = { 0.5, 0.5, 1.5, 1.5, 2.5, 2.5, 3.5, 4.5 };

const float ScanTable::vcp21[11] = { 0.5, 0.5, 1.45, 1.45, 2.4, 3.35, 4.3,
                                     6.0, 9.9, 14.6, 19.5 };

const float ScanTable::vcp11[16] = { 0.5, 0.5, 1.45, 1.45, 2.4, 3.35, 4.3,
                                     5.25, 6.2, 7.5, 8.7, 10.0, 12.0, 14.0, 
                                     16.7, 19.5 };

const float ScanTable::vcp12[17] = { 0.5, 0.5,  .9,  .9, 1.3,  1.3,  1.8,  2.4,
                                     3.1, 4.0, 5.1, 6.4, 8.0, 10.0, 12.5, 15.6, 
                                     19.5 };

const float ScanTable::vcp121[20] = { .5, .5, .5, .5, 1.45, 1.45, 1.45, 1.45,
                                     2.4, 2.4, 2.4, 3.35, 3.35, 3.35, 4.3, 
                                     4.3, 6.0, 9.9, 14.6, 19.5 }; 

const float ScanTable::vcp82[11] = { .5, .5, 1.5, 1.5, 2.4, 3.3, 4.3, 5.2, 6.2,
                                     7.5, 8.9 };

const float ScanTable::vcp221[11] = { 0.5, 0.5, 1.45, 1.45, 2.4, 3.35, 4.3,
				      6.0, 9.9, 14.6, 19.5 };

const float ScanTable::vcp211[16] = { 0.5, 0.5, 1.45, 1.45, 2.4, 3.35, 4.3,
				      5.25, 6.2, 7.5, 8.7, 10.0, 12.0, 14.0, 
				      16.7, 19.5 };

const float ScanTable::vcp212[17] = { 0.5, 0.5,  .9,  .9, 1.3,  1.3,  1.8,  2.4,
				      3.1, 4.0, 5.1, 6.4, 8.0, 10.0, 12.5, 15.6, 
				      19.5 };
//
// Additional information for each for NEXRAD scan type
//
const ScanTable::scanTypeInfo_t
             ScanTable::scanTypeInfo[NUM_SCAN_TYPES] = {
         //
         // Scan Id    Scan                 Pulse   Number of    List of
         //  (VCP)     Name                 Width   Elevations   Elevations
         //
             { 32, "Clear air/short pulse", 1.5,       7,  (float*)&vcp32 },
             { 31, "Clear air/long pulse",  4.7,       8,  (float*)&vcp31 },
             { 21, "Precipitation",         1.5,      11,  (float*)&vcp21 },
             { 11, "Severe weather",        1.5,      16,  (float*)&vcp11 },
             { 12, "VCP12",                 1.5,      17,  (float*)&vcp12 },
             { 121,"VCP121",                1.5,      20,  (float*)&vcp121 },
             { 82, "VCP82",                 1.5,      11,  (float*)&vcp82 },
             { 221,"Precipitation2",        1.5,      11,  (float*)&vcp221 },
             { 211,"Severe weather2",       1.5,      16,  (float*)&vcp211 },
             { 212,"VCP212",                1.5,      17,  (float*)&vcp212 } };
              

ScanTable::ScanTable()
{
  return;
}

ScanTable::~ScanTable()
{
   scanTypeById::iterator i;

   //
   // Clear out all scan types
   //
   for( i=scanTypes.begin(); i != scanTypes.end(); i++ ) {
      delete( (*i).second );
   }
   scanTypes.clear();

   return;

}

int
ScanTable::init( Params& params )
{
   size_t     i;
   ScanType*  scanType;

   //
   // Create the NEXRAD scan types.
   // We do this step in init() rather than the constructor because although
   // right now we are initializing the dictionary of scan types from
   // the static initializations above,  later we could add support
   // for an option to initialize from an external file as well as
   // the option to write out a sample vcp file from the initializations above.
   //
   POSTMSG( DEBUG, "Initializing scan table" );

   for( i=0; i < NUM_SCAN_TYPES; i++ ) {
      scanType = new ScanType( scanTypeInfo[i].id, 
                               scanTypeInfo[i].name,
                               scanTypeInfo[i].pulseWidth,
                               scanTypeInfo[i].numElev,
                               (float*)scanTypeInfo[i].elevList );
      scanTypes[scanTypeInfo[i].id] = scanType;
   }

   return( 0 );
}

ScanType*
ScanTable::getScanType( int scanTypeId )
{
   scanTypeById::iterator i = scanTypes.find( scanTypeId );

   if ( i != scanTypes.end() ) {
      return( (*i).second );
   }
   else {
      //
      // We're getting an unexpected scan type id.
      // This used to exit with assert(false) - 
      // now it just exits, which avoids a core file, but
      // ideally I'd like it set up so that the program keeps munching on.
      // With its current structure, though, that would be quite a bit of
      // work, and I have other things to be getting on with ... Niles.
      //
      POSTMSG( ERROR, "Scan type %d is undefined.", scanTypeId );
      exit(-1);
   }
   return NULL; // Never happens - but avoids compiler warning.
}
