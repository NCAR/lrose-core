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
/////////////////////////////////////////////////
// Data Manager
//
// $Id: DataMgr.cc,v 1.7 2016/03/06 23:31:57 dixon Exp $
//
/////////////////////////////////////////////////
#include <string>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/MsgLog.hh>

#include "MetarRepeat.hh"
#include "DataMgr.hh"

//
// Static members
//
const int DataMgr::MAX_LINE      = 256;
const int DataMgr::MAX_FILE_PATH = 256;

DataMgr::DataMgr()
{
   currentTime.year  = 1970;
   currentTime.month = 1;
   currentTime.day   = 1;
   currentTime.hour  = 0;
   currentTime.min   = 0;
   currentTime.sec   = 0;

   dayLast           = 1;

   inputDir          = NULL;
   outputDir         = NULL;

   nFiles            = 0;
   fileIndex         = 0;
   fileList          = NULL;

   outputHour        = 0;
   outputMin         = 0;
   
}

DataMgr::~DataMgr()
{
   if( inputDir )
      STRfree( inputDir );
   if( outputDir )
      STRfree( outputDir );
   
   for( int i = 0; i < nFiles; i++ ) {
      free(fileList[i]);
   }
   free(fileList);

   vector< pair< string*, string* >* >::iterator it;
   for( it = ids.begin(); it != ids.end(); it++ ) {
      delete( (*it)->first );
      delete( (*it)->second );
   }

   ids.erase( ids.begin(), ids.end() );
}

int 
DataMgr::init( metar_repeat_day_tdrp_struct& params )
{
   char *fileName;
   int   hour, min;
   int   prevHour = 0;
   int   prevMin = 0;
   
   //
   // Set members that are based on the parameters
   //
   inputDir      = STRdup( params.input_dir ); 
   outputDir     = STRdup( params.output_dir ); 
      
   string *oldId;
   string *newId;
   pair< string*, string* > *idPair;
   for( int i = 0; i < params.location_translation.len; i++ ) {
      oldId =
          new string( params.location_translation.val[i].orig_location );
      
      newId =
	 new string( params.location_translation.val[i].new_location );

      idPair = new pair< string*, string* >( oldId, newId );
      
      ids.push_back( idPair );
   }

   //
   // Get list of files and sort them - They should
   // now be in order of increasing time.
   //
   nFiles = scandir( inputDir, &fileList, 
                     fileSelect, alphasort );

   if( nFiles < 1 ) {
      POSTMSG( ERROR, "Can't find files in %s", inputDir );
      return( FAILURE );
   }

   //
   // Find the current time
   //
   time_t now = time(0);
   UTIMunix_to_date( now, &currentTime );

   //
   // Find first file to process - Find the first file
   // with a time later than the current time, then back
   // up one.  That is the closest file before the current
   // time.
   //
   fileIndex = nFiles - 1;
   for( int i = 0; i < nFiles; i++)
   {
      //
      // Set file name
      //
      fileName = fileList[i]->d_name; 

      //
      // Get hours and minutes from file
      //
      if( fileTime( (const char*) fileName, &hour, &min ) != SUCCESS )
	 continue;

      //
      // Compare file time to current time
      //
      if( currentTime.hour < hour ||
          (currentTime.hour == hour &&  currentTime.min < min) ) {

         POSTMSG( DEBUG, "current time is %s", asctime( gmtime(&now)) );

         if( i == 0 ) {

	    POSTMSG( DEBUG, "Processing file %s", fileList[i]->d_name );
	    if( translateFile( (const char*) fileList[i]->d_name,
                               hour, min ) != SUCCESS )
	       return( FAILURE );
            fileIndex = i+1;

	 } else {

	    POSTMSG( DEBUG, "Processing file %s", fileList[i-1]->d_name );
	    if( translateFile( (const char*) fileList[i-1]->d_name,
                               prevHour, prevMin ) != SUCCESS )
	       return( FAILURE );
            fileIndex = i;

	 }

	 break;
      }

      prevHour = hour;
      prevMin  = min;

   }

   dayLast = currentTime.day;
   
   return( SUCCESS );
   
}


int 
DataMgr::processFiles()
{
   char *fileName;
   int   hour, min;

   //
   // Get the current time
   //
   time_t now = time(0);
   UTIMunix_to_date( now, &currentTime );

   //
   // Reset fileIndex if we've run past midnight
   //
   if( currentTime.day != dayLast )
      fileIndex = 0;
   
   //
   // Find the file that is the closest after our current
   // hour and minute - Ignore the date
   //
   for( int i = fileIndex; i < nFiles; i++)
   {
      //
      // Set file name
      //
      fileName = fileList[i]->d_name; 

      //
      // Get the hours and minutes from the file name
      //
      if( fileTime( (const char*) fileName, &hour, &min ) != SUCCESS )
	 continue;

      //
      // Compare the times to the current time
      //
      if( currentTime.hour > hour ||
          (currentTime.hour == hour && currentTime.min >= min) ) {

         POSTMSG( DEBUG, "current time is %s", asctime( gmtime(&now) ) );
	 POSTMSG( DEBUG, "Processing file %s", fileName );

         if( translateFile( (const char*) fileName, 
                            hour, min ) != SUCCESS )
	    return( FAILURE );
         fileIndex = i+1;
         dayLast = currentTime.day;
	 break;
      }

   }
   
   return(SUCCESS);
}

int
DataMgr::fileTime( const char* fileName, int* hour, int* min ) 
{
   const char *fileDate;
   int year, month, day;
   
   //
   // Find the time from the file name
   //
   fileDate  = strstr( fileName, FILE_PRFX );
   fileDate += strlen( FILE_PRFX );

   //
   // First look for the longer file format, 
   // with minutes: YYYYMMDDHHMM
   //
   if( sscanf(fileDate, ".%4d%2d%2d%2d%2d", 
              &year, &month, &day, hour, min) != 5 ) {
      //
      // Try again, look for the shorter file format, 
      // without min: YYYYMMDDHH
      //
      if( sscanf(fileDate, ".%4d%2d%2d%2d", 
                 &year, &month, &day, hour) != 4 ) {
	 return( FAILURE );
      }

      //
      // Clear out minutes
      //
      *min = 0;
   }

   return( SUCCESS );
}


int
DataMgr::translateFile( const char* fileName, int hour, int min ) 
{
   FILE  *inputFp, *tempFp;
   char   line[MAX_LINE];
   int    position;
   string metar;

   vector< pair< string*, string* >* >::iterator it;
   pair< string*, string* > *idPair;

   //
   // Set up paths
   //
   char inputPath[MAX_FILE_PATH];
   char tempPath[MAX_FILE_PATH];
   char outputPath[MAX_FILE_PATH];
   
   sprintf( inputPath, "%s/%s", inputDir, fileName );
   sprintf( tempPath, "%s/%s.tmp", outputDir, FILE_PRFX );
   sprintf( outputPath,"%s/%s.%.4ld%.2ld%.2ld%.2d%.2d",
	    outputDir, FILE_PRFX, currentTime.year, 
            currentTime.month, currentTime.day, hour, min );
      
   //
   // Open files
   //
   if( (inputFp = 
        ta_fopen_uncompress( inputPath, "r" )) == NULL ) {
      POSTMSG( ERROR, "Could not open file %s\n", inputPath );
      return( FAILURE );
   } 

   if( (tempFp = 
        ta_fopen_uncompress( tempPath, "w" )) == NULL ) {
      POSTMSG( ERROR, "Could not open file %s\n", tempPath );
      return( FAILURE );
   }
   
   //
   // Read in a line from the input file.  Replace the location as
   // necessary and write the line to the output file.
   //
   while( fgets( line, MAX_LINE, inputFp ) != NULL ) {
      metar = line;
      for( it = ids.begin(); it != ids.end(); it++ ) {

         idPair = *it;
         position = metar.find( *(idPair->first), 0 );
	 if( position < (int) metar.length() && position >= 0 ) {
	    POSTMSG( DEBUG, "Replacing %s with %s", 
		     idPair->first->c_str(), idPair->second->c_str() );
	    metar.replace( position, idPair->first->length(),
                           *(idPair->second) );
	 }

      }

      fputs( (char *) metar.c_str(), tempFp );
      
   }

   fclose( inputFp );
   fclose( tempFp );

   //
   // Move temp file to output file
   //
   rename( tempPath, outputPath );
   POSTMSG( DEBUG, "File %s written", outputPath );
   
   return( SUCCESS );
	 
}

   









