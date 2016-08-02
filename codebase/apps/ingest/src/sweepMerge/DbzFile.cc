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
//
// $Id: DbzFile.cc,v 1.7 2016/03/07 01:23:11 dixon Exp $
//
/////////////////////////////////////////////////////////
#include <math.h>
#include "DbzFile.hh"
#include "SweepMerge.hh"

//
// Constants
//
const int DbzFile::MISSING_INDEX = -1;

DbzFile::DbzFile( FileInfo& fileInfo, double azTol )  
    : InputFile( fileInfo )
{
   double indexCount = 360.0 / DELTA_AZIMUTH;
   
   azimuthTol    = azTol / DELTA_AZIMUTH;
   numIndeces    = (int) indexCount;
   azimuthIdex   = new int[numIndeces];

   for( int i = 0; i < numIndeces; i++ ) {
      azimuthIdex[i] = MISSING_INDEX;
   }
   
}

DbzFile::~DbzFile() 
{
   delete [] azimuthIdex;
}

int DbzFile::init( string* azimuthName, string* elevationName, 
                   string* timeName, string* dbzName ) 
{
   ncInput = new NcFile( currentPath->c_str(), NcFile::ReadOnly );
   
   if( !ncInput || !ncInput->is_valid() ) {
      POSTMSG( ERROR, "Could not open %s", currentPath->c_str() );
      return( FAILURE );
   }

   if( processFile( azimuthName, elevationName, 
                    timeName, dbzName ) != SUCCESS ) {
      return( FAILURE );
   }

   if( fillIndexArray() != SUCCESS ) {
      return( FAILURE );
   }
   
   return( SUCCESS );
}
   

int DbzFile::fillIndexArray() 
{
   for( int i = 0; i < numRays; i++ ) {
      
      //
      // Find the 'index' into the index array for this
      // azimuth
      //   Note:  i            = index into input azimuth data array
      //          idex         = index into the array of azimuth indexes
      //          azimuthIndex = array filled with indexes into
      //                         original azimuth data array
      //          azimuthData  = array of original azimuth data
      //                         from input file
      //
      int idex = (int) ( azimuthData[i] / DELTA_AZIMUTH + 0.5 );

      azimuthIdex[idex] = i;
      
      //
      // Fill in around this point in the index array
      //
      int startIdex = idex - (int) (azimuthTol + 0.5);
      int endIdex   = idex + (int) (azimuthTol + 0.5);

      //
      // If we are in the negative range for the start index,
      // that means that we should wrap around to 360 to check
      // for azimuths there.  Then we can go on from azimuth 0
      //
      if( startIdex < 0 ) {
         for( int j = numIndeces + startIdex; j < numIndeces; j++ ) {
            setIndex( i, j );
         }

         startIdex = 0;
      }
      
      //
      // If we are past the end of the array for the end index,
      // that means that we should wrap around to 0 to check for
      // azimuths there.  Then we can stop at the end of the array
      // for the next set of checking.
      //
      if( endIdex >= numIndeces ) {
         for( int j = 0; j <= endIdex - numIndeces; j++ ) {
            setIndex( i, j );
         }
         
         endIdex = numIndeces - 1;
      }
      
      //
      // Fill in all the values between the start and end index
      //
      for( int j = startIdex; j <= endIdex; j++ ) {
         setIndex( i, j );
      }
      
   }

   //
   // Write out the azimuthIdex array, if needed
   //
   //if( DEBUG_ENABLED ) {
   //   writeAzIndex();
   //}

   return( SUCCESS );
}

void DbzFile::setIndex( int i, int j ) 
{
   //
   // Get the index value at this place in the array
   //
   int currentAzIndex = azimuthIdex[j];

   //
   // If there isn't anything there already, fill it
   // with the current information
   //         
   if( currentAzIndex == MISSING_INDEX ) {
      azimuthIdex[j] = i;
   }

   //
   // If there is something already there, decide
   // which beam to use - use the closest to the
   // current azimuth
   //
   else {

      //
      // What is the azimuth value here?
      //
      float azValue = j * DELTA_AZIMUTH;
            
      //
      // Is it closer to the value that is already there
      // or the one coming in?  If it is closer to the one
      // coming in, replace the azimuthIdex value here.
      // Otherwise do nothing - it is fine the way it is.
      //
      if( fabs( azValue - azimuthData[i] ) <
          fabs( azValue - azimuthData[currentAzIndex] ) ) {
         azimuthIdex[j] = i;
      }
   }
   
}

void DbzFile::writeAzIndex() 
{
   for( int i = 0; i < numIndeces; i++ ) {
      if( azimuthIdex[i] == MISSING_INDEX ) {
         POSTMSG( DEBUG, "azimuthIdex[%d] = missing", i );
      }
      else {
         POSTMSG( DEBUG, "azimuthIdex[%d] = %d", i, azimuthIdex[i] );
      }
   }
}

int DbzFile::moveFile() 
{
   //
   // Close the file before we do anything to it
   //
   int status = ncInput->close();

   if( status == 0 ) {
      POSTMSG( ERROR, "Could not close file %s", currentPath->c_str() );
      return( FAILURE );
   }
   
   //
   // The file has been copied already, remove it
   // from the current directory
   //
   if( unlink( currentPath->c_str() ) != 0 ) {
      POSTMSG( ERROR, "Could not remove file %s, error = %s", 
               currentPath->c_str(), strerror( errno ) );
      return( FAILURE );
   }

   POSTMSG( DEBUG, "Moved file %s to backup area", currentPath->c_str() );
   
   return( SUCCESS );
}


