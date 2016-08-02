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
//////////////////////////////////////////////////////
// DataMgr - handles data i/o and processing
//
// $Id: DataMgr.cc,v 1.5 2016/03/07 01:23:11 dixon Exp $
//
/////////////////////////////////////////////////////
#include <sys/stat.h>
#include <netcdf.hh>
#include "DataMgr.hh"
#include "SweepMerge.hh"
#include "DbzFile.hh"
#include "VelFile.hh"
#include "VolScans.hh"
#include "FileInfo.hh"

//
// Constants
//
const int DataMgr::SUFFIX_LEN   = 10;
const int DataMgr::RADAR_ID_LEN = 5;

DataMgr::DataMgr() 
{
   azimuthTol    = 0.0;
   elevationTol  = 0.0;
   timeTol       = 0.0;
   azimuthName   = NULL;
   elevationName = NULL;
   timeName      = NULL;
   dbzName       = NULL;
   backupPath    = NULL;
}

DataMgr::~DataMgr() 
{
   map< int, VolScans*, less<int> >::iterator it;
   for( it = volumeScans.begin(); it != volumeScans.end(); it++ ) {
      delete( (*it).second );
   }
   volumeScans.erase( volumeScans.begin(), volumeScans.end() );

   map< int, FileInfo*, less<int> >::iterator fileIt;
   for( fileIt = files.begin(); fileIt != files.end(); fileIt++ ) {
      delete( (*fileIt).second );
   }
   files.erase( files.begin(), files.end() );

   elevationList.erase( elevationList.begin(), elevationList.end() );

   delete azimuthName;
   delete elevationName;
   delete timeName;
   delete dbzName;
   delete backupPath;
}


int DataMgr::init( Params& params ) 
{
   azimuthTol     = params.azimuth_tolerance;
   elevationTol   = params.elevation_tolerance;
   timeTol        = params.time_tolerance;

   azimuthName    = new string( params.azimuth_name );
   elevationName  = new string( params.elevation_name );
   timeName       = new string( params.time_name );
   dbzName        = new string( params.refl_name );
   backupPath     = new string( params.backup_path );
   
   for( int i = 0; i < params.elevations_n; i++ ) {
      elevationList.push_back( params._elevations[i] );
   }
   
   if( createFileList( params.input_dir ) != SUCCESS ) {
      return( FAILURE );
   }

   if( findPairs() != SUCCESS ) {
      return( FAILURE );
   }

   return( SUCCESS );
}

int DataMgr::createFileList( const char* inputDir ) 
{
   //
   // This code was adapted from DsInputPath::_load_day
   //
   char  volDelim;
   int   year;
   int   month;
   int   day;
   int   hour;
   int   minute;
   int   volNum;
   int   sweepNum;
   float elevation;
   float seconds;

   char  radarId[RADAR_ID_LEN];
   char  suffix[SUFFIX_LEN];

   struct dirent *dp;

   //
   // Open directory
   //  
   DIR *dirp = opendir( inputDir );
   
   if ( !dirp ) {
      POSTMSG( ERROR, "Could not open directory %s, error = %s", 
               inputDir, strerror( errno ) );
      return( FAILURE );
   }

   //
   // Look through the files in the directory
   //
   for ( dp = readdir(dirp); dp != NULL; dp = readdir(dirp) ) {
     
      //
      // Exclude dir entries and files beginning with '.'
      //
      if ( dp->d_name[0] == '.' )
         continue;

      //    
      // Read pertinant information from the file name 
      //   Note that we require the ncswp prefix and the
      //   radar id be 4 characters.  There is a constant
      //   declared above to set up the radar id array,
      //   but it cannot be used in the format below, so
      //   we have to assume that the constant is set to
      //   5, which allows a null termination character.
      //
      int ret = sscanf( dp->d_name, "ncswp_%4c_%4d%2d%2d_%2d%2d%f_%c%d_"
                        "s%d_%f%s", radarId, &year, &month, &day, &hour, 
                        &minute, &seconds, &volDelim, &volNum, &sweepNum, 
                        &elevation, suffix );

      radarId[RADAR_ID_LEN-1] = '\0';
      
      if( ret == 12 ) {

         //
         // Check the elevation - only use this file if it is one
         // of the elevations we need to combine
         //
         for( int i = 0; i < (int) elevationList.size(); i++ ) {
            if( elevation == elevationList[i] ) {

               FileInfo *currentFile = new FileInfo( inputDir, dp->d_name, 
                                                     volNum, sweepNum, 
                                                     elevation, *backupPath );
               files[sweepNum] = currentFile;
               break;
               
            }
         }
      }
      
   }

   closedir( dirp );
   
   return( SUCCESS );
}

int DataMgr::findPairs() 
{
   map< int, FileInfo*, less<int> >::iterator it = files.begin();
   
   while( it != files.end() ) {
      
      //
      // Get the info for the current file
      //
      FileInfo *currentFile = (*it).second;

      int sweepNum = (*it).first;
      int volNum   = currentFile->getVolumeNum();
      
      //
      // See if we have created an object for the current
      // volume already or not
      //    If not create one. Otherwise, use the one we have
      //
      map< int, VolScans*, less<int> >::iterator volIt = 
         volumeScans.find( volNum );
      
      VolScans *currentVol = NULL;
      
      if( volIt == volumeScans.end() ) {
         currentVol = new VolScans( volNum, azimuthName, elevationName,
                                    timeName, dbzName );
         volumeScans[volNum] = currentVol;
      }
      else {
         currentVol = (*volIt).second;
      }
      
      //
      // See if we have a file for the next sweep
      //   If we do that means that the current file is a 
      //   dbz only file, so process both files and go onto
      //   the file after that
      //
      map< int, FileInfo*, less<int> >::iterator nextIt = 
         files.find( sweepNum+1 );
      
      if( nextIt != files.end() ) {
         if( (*nextIt).second->getVolumeNum() == volNum &&
             (*nextIt).second->getElevation() == 
               currentFile->getElevation() ) {
         
            if( currentVol->addDbzFile( *((*it).second), 
                                        azimuthTol ) != SUCCESS ) {
               return( FAILURE );
            }

            it++;
         
            if( currentVol->addVelFile( *((*nextIt).second), 
                                        elevationTol, 
                                        timeTol ) != SUCCESS ) {
               return( FAILURE );
            }

            it++;

            continue;
         }
         
      }

      //   
      //  We are missing the matching file, so skip this one and 
      //  go onto the next one
      //
      POSTMSG( WARNING, "No matching file - not processing %s",
               (*it).second->getFilePath()->c_str() );
      it++;
   }
   
   return( SUCCESS );
}

int DataMgr::run()
{
   map< int, VolScans*, less<int> >::iterator it;
   
   for( it = volumeScans.begin(); it != volumeScans.end(); it++ ) {

      VolScans *currentVolume = (*it).second;
      
      if( currentVolume->createNewScan() != SUCCESS ) {
         return( FAILURE );
      }
   }
   
   return( SUCCESS );
   
}









      
      








