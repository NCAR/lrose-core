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
/////////////////////////////////////////////////////////////
// VelFile - handles processing of short range scan sweep
//           file without matching dbz data
//
// $Id: 
//
/////////////////////////////////////////////////////////////
#include "VelFile.hh"
#include "DbzFile.hh"
#include "SweepMerge.hh"

VelFile::VelFile( FileInfo& fileInfo, double elevTol, double timeTol ) 
    : InputFile( fileInfo )
{
   elevTolerance = elevTol;
   timeTolerance = timeTol;
   newDbz        = NULL;
   dbzAzUsed     = NULL;
}

VelFile::~VelFile() 
{
   delete [] newDbz;
   delete [] dbzAzUsed;
}

int VelFile::init( string* azimuthName, string* elevationName, 
                   string* timeName, string* dbzName ) 
{
   ncInput = new NcFile( currentPath->c_str(), NcFile::Write );
   
   if( !ncInput || !ncInput->is_valid() ) {
      POSTMSG( ERROR, "Could not open %s", currentPath->c_str() );
      return( FAILURE );
   }

   if( processFile( azimuthName, elevationName, 
                    timeName, dbzName ) != SUCCESS ) {
      return( FAILURE );
   }
   
   return( SUCCESS );
}

int VelFile::createNewScan( DbzFile& dbzFile ) 
{
   POSTMSG( DEBUG, "Creating new scan for file %s using file %s",
            currentPath->c_str(), dbzFile.getPath()->c_str() );

   //
   // Set up new dbz data array
   //   Make sure all the values in the dbz data array from
   //   this file are initialized to missing
   //
   newDbz = new short[numRays*numCells];
   for( int i = 0; i < numCells * numRays; i++ ) {
      newDbz[i] = dbzMissingVal;
   }
   
   //
   // What is the number of cells or gates to use
   // in this processing
   //
   int minNumCells = min( dbzFile.getNumCells(), numCells );

   //
   // Get the number of rays from the dbz file
   //
   int numDbzRays = dbzFile.getNumRays();

   //
   // Get the index array
   //
   int *indexArray = dbzFile.getIndexArray();

   //
   // Get elevation, azimuth and time data from the dbz file
   //
   float  *dbzElevation = dbzFile.getElevationData();
   float  *dbzAzimuth   = dbzFile.getAzimuthData();
   double *dbzTime      = dbzFile.getTimeData();

   //
   // Get the dbz data from the dbz file
   //
   short *dbzScanData = dbzFile.getDbzData();
   
   //
   // Set up the array that will keep track of which azimuths
   // to use from the dbz file
   //
   dbzAzUsed = new float[numRays];

   for( int i = 0; i < numRays; i++ ) {

      //
      // Initialize azimuth used data at this ray
      //
      dbzAzUsed[i] = NC_FILL_FLOAT;
      
      //
      // Find the index of the azimuth
      //
      int velAzIndex = (int) ( azimuthData[i] / DELTA_AZIMUTH + 0.5 );

      //
      // Find the index of the beam to use from the dbz data
      //
      int dbzIndex = indexArray[velAzIndex];

      //
      // Check index
      //
      if( dbzIndex < 0 || dbzIndex > numDbzRays ) {
         continue;
      }

      //
      // Check elevation
      //
      if( fabs( elevationData[i] - dbzElevation[dbzIndex] ) > elevTolerance ) {
         continue;
      }
      
      //
      // Check time
      //
      if( fabs( timeData[i] - dbzTime[dbzIndex] ) > timeTolerance ) {
         continue;
      }
      
      //
      // Record the azimuth we are going to use
      //
      dbzAzUsed[i] = dbzAzimuth[dbzIndex];

      //
      // Copy dbz rays into vel grid
      //
      for( int j = 0; j < minNumCells; j++ ) {
         newDbz[i * numCells + j] = dbzScanData[dbzIndex * numCells + j];
      }
   }
   
   return( SUCCESS );
}
      

int VelFile::write() 
{
   long edges[2];
   
   edges[0] = (long) numRays;
   edges[1] = (long) numCells;

   NcVar *dbzPtr = ncInput->get_var( dbzName->c_str() );
   if( !dbzPtr || !dbzPtr->is_valid() ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s",
               dbzName->c_str(), currentPath->c_str() );
      return( FAILURE );
   }

   int status = dbzPtr->put( newDbz, edges );
   
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write data to variable %s in file %s",
               dbzName->c_str(), currentPath->c_str() );
      return( FAILURE );
   }

   status = ncInput->close();

   if( status == 0 ) {
      POSTMSG( ERROR, "Could not write file %s", currentPath->c_str() );
      return( FAILURE );
   }

   POSTMSG( DEBUG, "Wrote file %s with new scan", currentPath->c_str() );
   
   return( SUCCESS );
}

