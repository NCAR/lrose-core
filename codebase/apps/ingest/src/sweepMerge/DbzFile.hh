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
//////////////////////////////////////////////////////////
// DbzFile - handles processing of a long range scan
//           sweep file
/////////////////////////////////////////////////////////
#ifndef _DBZ_FILE_INC_
#define _DBZ_FILE_INC_

#include "InputFile.hh"

//
// Forward class declarations
//
class FileInfo;

class DbzFile : public InputFile
{
public:

   //
   // Constructor:
   //   fileInfo = container class with the following necessary
   //              information - volume number, sweep number, elevation, and
   //              current path name
   //   azTol    = azimuth tolerance - used in setting up index
   //              array to be used in deciding which beam to
   //              match up when creating new dbz data in the
   //              corresponding vel file
   //
   DbzFile( FileInfo& fileInfo, double azTol );

   //
   // Destructor
   //
   ~DbzFile();

   //
   // Initialize object
   //
   int init( string* azimuthName, string* elevationName, string* timeName,
             string* dbzName );

   //
   // Return the index array
   //
   //   returns NULL on failure 
   //
   int* getIndexArray(){ return azimuthIdex; }
   
   //
   // Move the file to the backup location
   //
   int moveFile();

   //
   // Constants
   //
   static const int MISSING_INDEX;
   
private:

   //
   // Azimuth tolerance
   //   
   double azimuthTol;

   //
   // Azimuth index data
   //   numIndeces  = length of the azimuthIdex array
   //   azimuthIdex = array which tells which index in the
   //                 azimuth data array corresponds to the
   //                 given azimuth as defined by i * DELTA_AZIMUTH
   //                 where i is the index into the azimuthIdex
   //                 array and DELTA_AZIMUTH is the resolution
   //                 in azimuths
   //
   int  numIndeces;
   int *azimuthIdex;
   
   //
   // Fill in the index array using the information from this file
   //
   int fillIndexArray();

   //
   // Figure out which index from the azimuth data array
   // should be used at a given point in the azimuthIdex array
   //   i = index into azimuth data array - the ith ray in the
   //       data arrays
   //   j = index into azimuthIdex array
   //
   void setIndex( int i, int j );

   //
   // Write out the azimuthIdex array
   //   Used for debugging
   //
   void writeAzIndex();
};

#endif























