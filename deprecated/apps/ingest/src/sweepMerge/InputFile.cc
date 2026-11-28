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
// InputFile - base class which handles processing of
//             various types of input files
/////////////////////////////////////////////////////////
#include "InputFile.hh"
#include "SweepMerge.hh"
#include "FileInfo.hh"

//
// Constants
//
const string InputFile::MISSING_VAL_NAME = "missing_value";
const double InputFile::DELTA_AZIMUTH    = 0.10;

InputFile::InputFile( FileInfo& fileInformation )
    : fileInfo( fileInformation ) 
{
   ncInput       = NULL;
   volNum        = fileInfo.getVolumeNum();
   sweepNum      = fileInfo.getSweepNum();
   elevation     = fileInfo.getElevation();
   numRays       = 0;
   numCells      = 0;
   dbzData       = NULL;
   elevationData = NULL;
   azimuthData   = NULL;
   timeData      = NULL;
   dbzMissingVal = 0;
   currentPath   = fileInfo.getFilePath();
   dbzName       = NULL;
}

InputFile::~InputFile() 
{
   delete [] dbzData;
   delete [] elevationData;
   delete [] azimuthData;
   delete [] timeData;
   delete ncInput;
   delete dbzName;
}

int InputFile::processFile( string* azimuthName, string* elevationName, 
                            string* timeName, string* reflName ) 
{
   //
   // Do the backup
   //
   if( fileInfo.doBackup() != SUCCESS ) {
      return( FAILURE );
   }
   
   //
   // Set dbz name
   //
   dbzName = new string( *reflName );
   
   //
   // Get the azimuth data
   //
   Nc3Var *azimuthPtr = ncInput->get_var( azimuthName->c_str() );
   if( !azimuthPtr ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s",
               azimuthName->c_str(), currentPath->c_str() );
      return( FAILURE );
   }

   int   numAzDims   = azimuthPtr->num_dims();
   long *azimuthDims = azimuthPtr->edges();
   
   numRays     = (int) azimuthDims[0];
   azimuthData = new float[numRays];

   delete[] azimuthDims;

   if( numAzDims != 1 ) {
      POSTMSG( ERROR, "Not expecting %d dimensions on azimuth in file %s",
               numAzDims, currentPath->c_str() );
      return( FAILURE );
   }
      
   int status = azimuthPtr->get( azimuthData, numRays );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not get data for variable %s from file %s",
               azimuthName->c_str(), currentPath->c_str() );
      return( FAILURE );
   }
      
   //
   // Get the elevation data
   //
   Nc3Var *elevationPtr = ncInput->get_var( elevationName->c_str() );
   if( !elevationPtr ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s",
               elevationName->c_str(), currentPath->c_str() );
      return( FAILURE );
   }

   long *elevationDims    = elevationPtr->edges();
   int   numElevDims      = elevationPtr->num_dims();
   int   numElevationRays = elevationDims[0];

   delete [] elevationDims;
      
   if( numElevDims != 1 || numElevationRays != numRays ) {
      POSTMSG( ERROR, "Number of elevation values does not match "
               "the number of azimuth values" );
      return( FAILURE );
   }

   elevationData = new float[numRays];

   status = elevationPtr->get( elevationData, numRays );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not get data for variable %s from file %s",
               elevationName->c_str(), currentPath->c_str() );
      return( FAILURE );
   }
      
   //
   // Get time data
   //
   Nc3Var *timePtr = ncInput->get_var( timeName->c_str() );
   if( !timePtr ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s",
               timeName->c_str(), currentPath->c_str() );
      return( FAILURE );
   }

   long *timeDims    = timePtr->edges();
   int   numTimeDims = timePtr->num_dims();
   int   numTimeRays = timeDims[0];

   delete [] timeDims;
      
   if( numTimeDims != 1 || numTimeRays != numRays ) {
      POSTMSG( ERROR, "Number of time values does not match the number "
               "of azimuths" );
      return( FAILURE );
   }

   timeData = new double [numRays];

   status = timePtr->get( timeData, numRays );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not get data for variable %s from file %s",
               timeName->c_str(), currentPath->c_str() );
      return( FAILURE );
   }
         
   //
   // Get dbz data
   //
   Nc3Var *dbzPtr = ncInput->get_var( dbzName->c_str() );
   if( !dbzPtr || !dbzPtr->is_valid() ) {
      POSTMSG( ERROR, "Could not get variable %s from file %s",
               dbzName->c_str(), currentPath->c_str() );
      return( FAILURE );
   }
      
   int   numDims  = dbzPtr->num_dims();
   long *dimArray = dbzPtr->edges();
      
   if( numDims != 2 || dimArray[0] != numRays ) {
      POSTMSG( ERROR, "Dimensions on %s data in file %s do not make sense",
               dbzName->c_str(), currentPath->c_str() );
      return( FAILURE );
   }

   numCells = dimArray[1];
      
   dbzData = new short[numRays*numCells];
      
   status = dbzPtr->get( dbzData, dimArray );
   if( status == 0 ) {
      POSTMSG( ERROR, "Could not get data for variable %s from file %s",
               dbzName->c_str(), currentPath->c_str() );
      return( FAILURE );
   }

   delete [] dimArray;

   Nc3Att *dbzAtt = dbzPtr->get_att( MISSING_VAL_NAME.c_str() );
   if( !dbzAtt ) {
      POSTMSG( ERROR, "Could not get attribute %s from variable %s from "
               "file %s", MISSING_VAL_NAME.c_str(), dbzName->c_str(),
               currentPath->c_str() );
      return( FAILURE );
   }
   
   dbzMissingVal = dbzAtt->as_short( 0 );

   delete dbzAtt;
   
   return( SUCCESS );
   
}





