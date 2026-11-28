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
//  Ingest of class file and output of spdb data
//
//  $Id: DataMgr.cc,v 1.6 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>

#include <mdv/mdv_grid.h>
#include <toolsa/pmu.h>
#include <toolsa/MsgLog.hh>
#include <toolsa/Path.hh>
#include <dirent.h>

#include "ProfilerIngest.hh"
#include "DataMgr.hh"
using namespace std;

//
// static definitions, including column labels expected in the CLASS file
//
const unsigned int DataMgr::NFIELDS_IN = 6;

const char*  DataMgr::HEIGHT_LABEL      = "Alt";
const char*  DataMgr::U_WIND_LABEL      = "Uwind";
const char*  DataMgr::V_WIND_LABEL      = "Vwind";
const char*  DataMgr::PRESSURE_LABEL    = "Press";
const char*  DataMgr::REL_HUM_LABEL     = "RH";
const char*  DataMgr::TEMPERATURE_LABEL = "Temp";

const char*  DataMgr::DELIMETER         = " ,:\n\r";
const double DataMgr::MISSING_VALUE     = 999.0;

const char*  DataMgr::INDEX_FILENAME    = "classIngest.index";


DataMgr::DataMgr()
{
   ldataIndex        = NULL;
   useIndexFile      = true;
   expireSecs        = 0;
   fileSuffix        = NULL;
}

DataMgr::~DataMgr()
{
   delete ldataIndex;

#ifdef NOTYET
   //
   // Not until they are dynamically allocated 
   // right now they are of a fixed declared size -- uggh!
   //
   map<int, double*, less<int> >::iterator i;

   for( i=columnData.begin(); i != columnData.end(); i++ ) {
      delete( (*i).second ); 
   }
#endif

   columnData.erase( columnData.begin(), columnData.end() );
}

int
DataMgr::init( Params &params )
{
   //
   // Save relevant parameters
   //
   expireSecs      = params.expire_secs;
   fileSuffix      = params.input_suffix;
   inputFile.setDirectory( params.input_dir );

   // 
   // Instantiate the sounding
   //
   POSTMSG( DEBUG, "Initializing a sounding." );

   int i;
   string url;
   vector< string* > spdbUrls;

   for( i=0; i < params.spdb_urls_n; i++ ) {
      url = params._spdb_urls[i];
      spdbUrls.push_back( &url );
   }
   sounding.init( spdbUrls, Sounding::SONDE_ID, "CLASS" );

   sounding.setMissingValue( MISSING_VALUE );

   //
   // Initialize time management using the ldata index file
   // Don't use rapDataDir because it will already be handled
   // via DsInputPath file management upstream of us
   // By default we put the index file into the input_file_path
   // unless the user has explicitly set to a different directory
   //
   bool useRapDataDir = false;
   if ( !ISEMPTY( params.index_dir ) ) {
      ldataIndex = new LdataInfo( params.index_dir, useRapDataDir,
                                  INDEX_FILENAME );
   }
   else {
      ldataIndex = new LdataInfo( inputFile.getDirectory(), useRapDataDir,
                                  INDEX_FILENAME );
   }

   if( params.ignore_index_at_start ) {
      useIndexFile = false;
   }
   else {
      useIndexFile = true;
      if ( !Path::exists( ldataIndex->getInfoPath() )) {
         //
         // no index file yet
         //
         POSTMSG( DEBUG, "No index file yet.");
      }
      else {
         //
         // Read the ldata index file
         //
         if( ldataIndex->read() != 0 ) {
            POSTMSG( WARNING, "Cannot read ldata index file '%s'",
                              ldataIndex->getInfoPath().c_str() );
            return( -1 );
         }
         else {
            //
            // Store dataTime of most recently processed file
            // based on the contents of the index file
            //
            dataTime.set( ldataIndex->getLatestTime() );
            previousFileName = ldataIndex->getUserInfo1();
            POSTMSG( DEBUG, "Latest time in index: %s\n"
                            "Latest file in index: %s",
                            dataTime.dtime(), previousFileName.c_str() );
         }
      }
   }

   return( CI_SUCCESS );
}

int
DataMgr::ingest()
{
   int             nFiles, fileIndex = 0;
   string          currentFileName;
   struct dirent **fileList;

   //
   // Get list of files
   //
   nFiles = scandir( inputFile.getDirectory().c_str(), &fileList,
                     fileSelect, alphasort );

   //
   // Process files in fileList
   //
   for( fileIndex = 0; fileIndex < nFiles; fileIndex++) {
      //
      // Set input file name
      // Throwing out files which preceed the index file, if necessary
      //
      currentFileName = fileList[fileIndex]->d_name;
      if ( useIndexFile  &&  currentFileName <= previousFileName ) {
         POSTMSG( DEBUG, "Passing over file '%s'", currentFileName.c_str() );
         continue;
      }
      else {
         inputFile.setFile( currentFileName );
      }

      //
      // Process the file
      // Continue on to the next file if the current file has problems
      //
      PMU_auto_register("Reading sounding file");
      if( processFile() != CI_SUCCESS ) {
         POSTMSG( WARNING, "Skipping current file." );
         continue;
      }

      //
      // Write out the sounding and update the index
      // Bail out if the write fails for any reason
      //
      PMU_auto_register( "Writing sounding spdb" );
      if ( writeSounding() != CI_SUCCESS ) {
         POSTMSG( ERROR, "Exiting.  Check validity of output destination(s)." );
         return( CI_FAILURE );
      }

      //
      // Save the index file for comparison next time around
      //
      ldataIndex->setUserInfo1( (char*)inputFile.getFile().c_str() );
      if ( ldataIndex->write( dataTime.utime() ) != 0 ) {
         POSTMSG( ERROR, "Could not write index file "
                         "to database directory '%s'",
                         inputFile.getDirectory().c_str() );
         return( CI_FAILURE );
      }
      else {
         POSTMSG( DEBUG, "   Wrote the index for time %s",
                             dataTime.dtime() );
      }
   }

   //
   // Clean up from scandir
   //
   for( int i = 0; i < nFiles; i++ ) {
      free( (void *) fileList[i] );
   }
   free( (void *) fileList );

   return( CI_SUCCESS );
}

int
DataMgr::processFile()
{
   /////////////////////////////////////////////////////////////////////////////
   //
   // Notes about the CLASS format for sounding files:
   //
   //  * One line in the header must start with the words "Data Type" and have
   //    the word "SOUNDING" on that line to indicate a sounding file.
   //    Only the first "Data Type" line is considered.
   //  * The labels line must be present and must start with the label "Time".
   //    The labels for all fields of interest must be present.
   //    They can be in any order and don't have to be sequential.
   //  * Data is assumed to be in the same column as its label.  The number of
   //    fields per line of data is computed from the first line of data, not
   //    the labels line.
   //  * Bad data values are 999.0 for fields Uwind, Vwind, Wspd, and Dir; and
   //    99999.0 for field Alt.  Any line of data with at least one bad data
   //    value is disregarded.
   //  * Assumed units are Uwind (m/s), Vwind (m/s), Alt (m),
   //    Press (mb), RH (%), Temp (C).
   //  * Alt is converted to km for storage into spdb database.
   //
   /////////////////////////////////////////////////////////////////////////////

   int   status   = CI_SUCCESS;
   char *filePath = (char*)inputFile.getPath().c_str();
   char *fileName = (char*)inputFile.getFile().c_str();

   POSTMSG( DEBUG, "Processing file: %s", fileName );

   //
   // Open the file
   //
   if ( (fp = ta_fopen_uncompress( filePath, "r" )) == NULL ) {
      POSTMSG( WARNING, "Cannot open sounding file '%s'", 
                        filePath );
      return( CI_FAILURE );
   }

   //
   // Read the file
   //
   if ( status == CI_SUCCESS  &&  (status = readHeader()) != CI_SUCCESS ) {
      POSTMSG( WARNING, "Cannot read sounding header from file '%s'", 
                         fileName );
   }

   if ( status == CI_SUCCESS  &&  (status = findColumns()) != CI_SUCCESS ) {
      POSTMSG( WARNING, "Cannot find expected column labels from file '%s'", 
                        fileName );
   }

   if ( status == CI_SUCCESS  &&  (status = findFirstData()) != CI_SUCCESS ) {
      POSTMSG( WARNING, "Cannot find first data line in file '%s'", 
                        fileName );
   }

   if ( status == CI_SUCCESS  &&  (status = readData()) != CI_SUCCESS ) {
      POSTMSG( WARNING, "Unexpected results reading data from file '%s'", 
                        fileName );
   }

   //
   // Close the file
   //
   fclose( fp );

   if ( status == CI_SUCCESS ) {
      //
      // Update the time of the data based on the input file name
      // And save the fileName
      //
      dataTime.set( fileName );
      previousFileName = fileName;
   }

   return( status );
}

int
DataMgr::writeSounding()
{
   int     status;
   double *wwind = NULL;
   time_t  when = dataTime.utime();

   //
   // Set the data values in the Sounding class
   //
   status = sounding.set( when, numPoints, height, uwind, vwind, wwind,
                          pressure, relHum, temperature );
   if ( status != 0 ) {
      POSTMSG( ERROR, "Could not set the sounding values." );
      return( CI_FAILURE );
   }

   //
   // Tell the user about the sounding data we're getting ready to write
   //
   if ( DEBUG_ENABLED ) {
      POSTMSG( DEBUG, "   Set sounding data for '%s'\n"
                      "   Launched from (lat, lon) (%lf, %lf)",
                      sondeId, lat, lon );

      double min, max, avg;
      sounding.getStats( Sounding::ALTITUDE, &min, &max, &avg );
      POSTMSG( DEBUG, "   HEIGHT:  (min, max, avg) (%9.2lf%9.2lf%9.2lf)", 
                                    min, max, avg );
      sounding.getStats( Sounding::U_WIND, &min, &max, &avg );
      POSTMSG( DEBUG, "   U_WIND:  (min, max, avg) (%9.2lf%9.2lf%9.2lf)", 
                                    min, max, avg );
      sounding.getStats( Sounding::V_WIND, &min, &max, &avg );
      POSTMSG( DEBUG, "   V_WIND:  (min, max, avg) (%9.2lf%9.2lf%9.2lf)", 
                                    min, max, avg );
      sounding.getStats( Sounding::PRESSURE, &min, &max, &avg );
      POSTMSG( DEBUG, "   PRESS :  (min, max, avg) (%9.2lf%9.2lf%9.2lf)", 
                                    min, max, avg );
      sounding.getStats( Sounding::REL_HUMIDITY, &min, &max, &avg );
      POSTMSG( DEBUG, "   RELHUM:  (min, max, avg) (%9.2lf%9.2lf%9.2lf)", 
                                    min, max, avg );
      sounding.getStats( Sounding::TEMPERATURE, &min, &max, &avg );
      POSTMSG( DEBUG, "   TEMP  :  (min, max, avg) (%9.2lf%9.2lf%9.2lf)", 
                                    min, max, avg );
   }

   //
   // Write the sounding to the database
   //
   status = sounding.writeSounding( when, when + expireSecs );

   if ( status  != 0 ) {
      POSTMSG( ERROR, "Could not write to the sounding database." );
      return( CI_FAILURE );
   }
   else {
      POSTMSG( DEBUG, "   Wrote the sounding for time %s", 
                      dataTime.dtime() );
   }

   return( CI_SUCCESS );
}

int
DataMgr::readHeader()
{
   char junk[64];
   char siteId[64];
   char *lptr, line[1024];
   double alt;

   //
   // Get the siteId to be used in case the sondeId is not available
   //
   siteId[0] = '\0';
   lptr = getHeaderLine( "Launch Site Type/Site ID", line );
   if ( lptr ) {
      sscanf( lptr, "%[^:]:%[^,], %s", junk, junk, siteId );
   }

   //
   // Get/set the launch location
   //
   lat = lon = alt = 0.0;
   lptr = getHeaderLine( "Launch Location", line );
   if ( lptr ) {
      sscanf( lptr, "%[^:]:%[^,],%[^,],%lf,%lf,%lf", junk, junk, junk,
                    &lon, &lat, &alt );
   }
   sounding.setLocation( lat, lon, alt );

   //
   // Get/set the sonde id, initialize sondeId to siteId
   // in case we don't find what we're looking for
   //
   strcpy( sondeId, siteId );
   lptr = getHeaderLine( "Sonde Type/ID", line );
   if ( lptr ) {
      sscanf( lptr, "%[^:]:%[^,], %s", junk, junk, sondeId );
      if ( strcasecmp( sondeId, "UNKNOWN" ) == 0 ) {
         strcpy( sondeId, siteId );
      }
   }
   sounding.setSiteName( sondeId );

   return CI_SUCCESS;
}

char *
DataMgr::getHeaderLine( const char* label, char lineText[] )
{
   long  posStart, posCurrent = -1L;
   bool  found = false;
   char  line[BUFSIZ], *lptr;

   //
   // Hang onto the starting file position
   //
   if ( (posStart = ftell(fp)) == -1L ) {
      return NULL;
   }

   //
   // Read each line until we either find what we're looking for
   // or we come back around to our starting point
   //
   while( posCurrent != posStart ) {
      lptr = fgets( line, BUFSIZ, fp );
      posCurrent = ftell(fp);

      if ( lptr == NULL ) {
         //
         // We've reached the end of file -- wrap around
         // 
         fseek( fp, posStart, SEEK_SET );
      }
      else {
         //
         // See if this is the line we want
         //
         if ( strstr( line, label ) != NULL ) {
            //
            // This is it
            //
            found = true;
            break;
         }
      }
   }

   if ( found ) {
      strcpy( lineText, lptr );
      return lineText;
   }
   else
      return NULL;
}

int
DataMgr::findColumns()
{
   int   column = 0;
   char *field, line[BUFSIZ];

   //
   // Find the labels line.  It must start with the label "Time".
   //
   if ( fgets( line, BUFSIZ, fp ) == NULL ) {
      return CI_FAILURE;
   }
   field = strtok( line, DELIMETER );
   while( (field == NULL ) ||
         ((field != NULL) && (strcmp( field, "Time" ) != 0)) ) {
      if ( fgets( line, BUFSIZ, fp ) == NULL ) {
         return CI_FAILURE;
      }
      field = strtok( line, DELIMETER );
   }

   //
   // Look for all the labels we need and note the column for each.
   //
   while( field != NULL ) {

      if ( strcmp( field, U_WIND_LABEL ) == 0 ) {
         columnData[column] = &uwind[0];
         POSTMSG( DEBUG, "   Field '%s' found in column %d.", 
                             (char*)U_WIND_LABEL, column );
      }
      else if ( strcmp( field, V_WIND_LABEL ) == 0 ) {
         columnData[column] = &vwind[0];
         POSTMSG( DEBUG, "   Field '%s' found in column %d.", 
                             (char*)V_WIND_LABEL, column );
      }
      else if ( strcmp( field, HEIGHT_LABEL ) == 0 ) {
         columnData[column] = &height[0];
         POSTMSG( DEBUG, "   Field '%s' found in column %d.", 
                             (char*)HEIGHT_LABEL, column );
      }
      else if ( strcmp( field, PRESSURE_LABEL ) == 0 ) {
         columnData[column] = &pressure[0];
         POSTMSG( DEBUG, "   Field '%s' found in column %d.", 
                             (char*)PRESSURE_LABEL, column );
      }
      else if ( strcmp( field, TEMPERATURE_LABEL ) == 0 ) {
         columnData[column] = &temperature[0];
         POSTMSG( DEBUG, "   Field '%s' found in column %d.", 
                             (char*)TEMPERATURE_LABEL, column );
      }
      else if ( strcmp( field, REL_HUM_LABEL ) == 0 ) {
         columnData[column] = &relHum[0];
         POSTMSG( DEBUG, "   Field '%s' found in column %d.", 
                             (char*)REL_HUM_LABEL, column );
      }

      field = strtok(NULL, DELIMETER);
      column++;
   }

   //
   // Make sure we found all the fields we were interested in
   //
   if ( columnData.size() != NFIELDS_IN ) {
      return CI_FAILURE;
   }
 
   return CI_SUCCESS;
}

int
DataMgr::findFirstData()
{
   char *field, line[BUFSIZ];
   long fpos;

   //
   // Hang onto the current file position.
   //
   if ( (fpos = ftell(fp)) == -1L ) {
      return CI_FAILURE;
   }

   //
   // Look for the first line of data.  There might be non-data lines between
   // the labels line and the first line of data.
   //
   if ( fgets( line, BUFSIZ, fp ) == NULL ) {
      return CI_FAILURE;
   }
   field = strtok( line, DELIMETER );

   //
   // Check the first two chars of field since a digit may be negative and
   // thus, start with '-'.
   //
   while( (field == NULL) ||
          ((field != NULL) && (!isdigit(*field) && !isdigit(*(field+1))))) {
      //
      // Get the current file position.
      //
      if ( (fpos = ftell(fp)) == -1L ) {
          return CI_FAILURE;
      }

      if ( fgets( line, BUFSIZ, fp ) == NULL ) {
         return CI_FAILURE;
      }
      field = strtok(line, DELIMETER);
   }

   //
   // Reset the file pointer to the beginning of this line of data.
   //
   fseek( fp, fpos, SEEK_SET );
   return CI_SUCCESS;
}

int
DataMgr::readData()
{
   int             targetCol, icol, status;
   double          value, *dataArray;
   char            line[BUFSIZ], lineCopy[BUFSIZ], *fptr;
   const char           *BLANK = " ";

   map<int, double*, less<int> >::iterator item;

   //
   // Bad altitude indicator is 99999.0 in the CLASS format.
   //
   const float BAD_ALT  = 99999.0;

   //
   // Read in each line of data.
   //
   numPoints = 0;
   while ( fgets( line, BUFSIZ, fp ) != NULL ) {

      //
      // Get each field of interest
      //
      for( item=columnData.begin(); item != columnData.end(); item ++ ) {

         //
         // Get the nth column value from the line 
         // for this field of interest 
         // and stick it in the appropriate data array
         //
         status = 1;
         targetCol = (*item).first;
         strncpy( lineCopy, line, BUFSIZ );

         //
         // Scan for the zero'th column
         //
         fptr = strtok( lineCopy, BLANK );
         sscanf( fptr, "%lf", &value );

         //
         // Scan for the target column
         //
         for( icol=0; icol < targetCol && fptr; icol++ ) {
            fptr = strtok( NULL, BLANK );
            sscanf( fptr, "%lf", &value );
         }
         if ( fptr ) {
            //
            // We've found the target column we're looking for
            // Get a handle to the data array for this field of interest
            //
            dataArray = (*item).second;

            //
            // If this is a bad altitude value, ignore this line
            // because we key the data from altitudes
            //
            if ( value == BAD_ALT ) {
               if ( dataArray != &height[0] ) {
                  //
                  // This should never happen
                  // Something went really wrong in determining which column
                  // to read for the altitude values
                  //
                  POSTMSG( INTERNAL, "Inconsistency in parsing sounding file" );
                  return CI_FAILURE;
               }
               status = (int)BAD_ALT;
               break;
            }
            else {
               //
               // For all other fields of interest (besides alt),
               // store the data value
               //
               dataArray[numPoints] = value;
            }
         }
      }
      if ( status != (int)BAD_ALT ) {
         numPoints++;
      }
   }


   //
   // Make sure we found some data
   //
   if ( numPoints <= 0 ) {
      POSTMSG( ERROR, "No data points in the sounding file" );
      return CI_FAILURE;
   }
   else
      return CI_SUCCESS;
}
