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
//  Ingest class file and output shapefile
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  January 2003
//
//  $Id: DataMgr.cc,v 1.14 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <dirent.h>

#include <toolsa/pmu.h>
#include <toolsa/MsgLog.hh>
#include <toolsa/Path.hh>

#include "Driver.hh"
#include "DataMgr.hh"
using namespace std;

//
// static definitions
//
const char*  DataMgr::DELIMETER         = " ,:\n\r";
const char*  DataMgr::INDEX_FILENAME    = "index";


DataMgr::DataMgr()
{
   firstCall         = true;
   ldataIndex        = NULL;
   useIndexFile      = true;
   indexFileName     = INDEX_FILENAME;
   inputSuffix       = NULL;
   outputDbf         = NULL;
   outputShp         = NULL;
   eventDbf          = NULL;
}

DataMgr::~DataMgr()
{
   delete ldataIndex;

   if ( outputDbf ) {
      DBFClose( outputDbf );
   }

   if ( outputShp ) {
      SHPClose( outputShp );
   }

   if ( eventDbf ) {
      DBFClose( eventDbf );
   }

   for( size_t i=0;  i < dataFields.size(); i++ ) {
      delete dataFields[i];
   }
   for( size_t i=0;  i < headerFields.size(); i++ ) {
      delete headerFields[i];
   }
}

int
DataMgr::init( Params &params )
{
   Path   outputPath;
   char  *basepath;

   //
   // Hang onto the parameters themselves
   //
   _params = &params;

   //
   // Save relevant input parameters
   //
   inputPath.setDirectory( params.input_dir );
   inputSuffix     = params.input_suffix;
   indexFileName  += ".";
   indexFileName  += params.instance;
   singleLevel     = params.single_level;

   //
   // Instantiate the list of data field information
   // Users specify data fields of interest
   //
   Params::field_info_t  paramField;

   for( int i=0;  i < params.field_info_n;  i++ ) {
      paramField = params._field_info[i];

      //
      // All data fields are stored as doubles
      //
      dataFields.push_back( new FieldInfo( paramField.classFieldName,
                                           paramField.shapeFieldName,
                                           FTDouble,
                                           paramField.fieldWidth,
                                           paramField.fieldPrecision,
                                           paramField.fieldUnits,
                                           paramField.missingVal ));
   }

   // 
   // Set the directory and make sure it exists
   //
   outputPath.setDirectory( params.output_dir );
   if ( outputPath.makeDirRecurse() != 0 ) {
      POSTMSG( ERROR, "Unable to make directiory %s",
               outputPath.getPath().c_str() );
   }

   // 
   // Set the output filename and open the shapefile (create mode)
   //
   if ( ISEMPTY( params.shapefile_name ) ) {
      writeShapefile = false;
   }
   else {
      writeShapefile = true;
      outputPath.setFile( params.shapefile_name );
      basepath = (char*)outputPath.getPath().c_str();

      POSTMSG( DEBUG, "Creating shapefile %s", basepath );
      outputShp = SHPCreate( basepath, SHPT_POINT );
      outputDbf = DBFCreate( basepath );

      if ( outputShp == NULL  ||  outputDbf == NULL ) {
         POSTMSG( ERROR, "Unable to open shapefile" );
         return( -1 );
      }
   }

   //
   // Set the output event table (append mode)
   //
   if ( ISEMPTY( params.eventfile_name ) ) {
      writeEventfile = false;
   }
   else {
      writeEventfile = true;
      outputPath.setFile( params.eventfile_name );
      outputPath.setExt( "dbf" );
      basepath = (char*)outputPath.getPath().c_str();

      if ( outputPath.pathExists() ) {
         POSTMSG( DEBUG, "Appending to eventsfile %s", basepath );
         eventDbf = DBFOpen( basepath, "rb+" );
      }
      else {
         POSTMSG( DEBUG, "Creating eventsfile %s", basepath );
         eventDbf = DBFCreate( basepath );
      }

      if ( eventDbf == NULL ) {
         POSTMSG( ERROR, "Unable to open events file" );
         return( -1 );  
      }
   }

   //
   // Initialize input file indexing using the ldata index file
   // This feature allows the user to remember where the ingest last left off
   // By default we put the index file into the input_file_path
   // unless the user has explicitly set to a different directory
   //
   bool useRapDataDir = false;

   if ( !ISEMPTY( params.index_dir ) ) {
      ldataIndex = new LdataInfo( params.index_dir, useRapDataDir,
                                  indexFileName.c_str() );
   }
   else {
      ldataIndex = new LdataInfo( inputPath.getDirectory(), useRapDataDir,
                                  indexFileName.c_str() );
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
         POSTMSG( DEBUG, "No index file yet (%s).", 
                         ldataIndex->getInfoPath().c_str() );
      }
      else {
         //
         // Read the ldata index file
         //
         if( ldataIndex->read() != 0 ) {
            POSTMSG( WARNING, "   Cannot read ldata index file '%s'",
                              ldataIndex->getInfoPath().c_str() );
            return( -1 );
         }
         else {
            //
            // Store fileTime of most recently processed file
            // based on the contents of the index file
            //
            fileTime.set( ldataIndex->getLatestTime() );
            previousFileName = ldataIndex->getUserInfo1();
            POSTMSG( DEBUG, "Latest time in index: %s\n"
                            "Latest file in index: %s",
                            fileTime.dtime(), previousFileName.c_str() );
         }
      }
   }

   return( 0 );
}

int
DataMgr::convert()
{
   int             nFiles, fileIndex = 0;
   string          currentFileName;
   struct dirent **fileList;

   //
   // Get list of files
   //
   nFiles = scandir( inputPath.getDirectory().c_str(), &fileList,
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
         inputPath.setFile( currentFileName );
      }

      //
      // Process the file
      // Continue on to the next file if the current file has problems
      //
      PMU_auto_register("Reading sounding file");
      if( processFile() != 0 ) {
         POSTMSG( WARNING, "Skipping current file." );
         continue;
      }
      firstCall = false;

      //
      // Save the index file for comparison next time around
      //
      ldataIndex->setUserInfo1( (char*)inputPath.getFile().c_str() );
      if ( ldataIndex->write( fileTime.utime() ) != 0 ) {
         POSTMSG( ERROR, "Could not write index file "
                         "to input directory '%s'",
                         inputPath.getDirectory().c_str() );
         return( -1 );
      }
      else {
         POSTMSG( DEBUG, "   Wrote the index for time %s",
                             fileTime.dtime() );
      }
   }

   //
   // Clean up from scandir
   //
   for( int i = 0; i < nFiles; i++ ) {
      free( (void *) fileList[i] );
   }
   free( (void *) fileList );

   return( 0 );
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

   int   status   = 0;
   char *filePath = (char*)inputPath.getPath().c_str();
   char *fileName = (char*)inputPath.getFile().c_str();

   POSTMSG( DEBUG, "Processing file: %s", fileName );

   //
   // Open the file
   //
   if ( (inputFile = ta_fopen_uncompress( filePath, "r" )) == NULL ) {
      POSTMSG( WARNING, "Cannot open input class file '%s'", filePath );
      return( -1 );
   }

   //
   // Read the file
   //
   if ( (status = readHeader()) != 0 ) {
      POSTMSG( WARNING, "Cannot read sounding header from file '%s'", 
                         fileName );
   }

   if ( writeShapefile ) {
      if ( status == 0  &&  (status = findColumns()) != 0 ) {
         POSTMSG( WARNING, "Cannot find expected column labels from file '%s'", 
                           fileName );
      }

      if ( status == 0  &&  (status = findFirstData()) != 0 ) {
         POSTMSG( WARNING, "Cannot find first data line in file '%s'", 
                           fileName );
      }

      if ( status == 0  &&  (status = processData()) != 0 ) {
         POSTMSG( WARNING, "Unexpected results reading data from file '%s'", 
                           fileName );
      }
   }

   //
   // Close the file
   //
   fclose( inputFile );

   if ( status == 0 ) {
      //
      // Update the time of the data based on the input file name
      // And save the fileName
      //
      fileTime.set( fileName );
      previousFileName = fileName;
   }

   if ( writeEventfile ) {
      if ( status == 0  &&  (status = processEvent()) != 0 ) {
         POSTMSG( WARNING, "Unexpected results processing event from file '%s'",
                           fileName );
      }
   }

   return( status );
}

FieldInfo*
DataMgr::getHeaderField( string fieldName, DBFFieldType fieldType, 
                         int fieldWidth, int fieldPrecision )
{
   FieldInfo *fieldInfo;

   for( size_t i=0;  i < headerFields.size();  i++ ) {
      fieldInfo = headerFields[i];
      if ( fieldName == fieldInfo->shapeFieldName ) {
         return( fieldInfo );
      }
   }

   //
   // The requested field does not exist yet -- create it
   //
   fieldInfo = new FieldInfo( fieldName, fieldType, 
                              fieldWidth, fieldPrecision );
   headerFields.push_back( fieldInfo );
   return( fieldInfo );
}

int
DataMgr::readHeader()
{
   char       junk[64];
   int        year, month, day, hour, min, sec;
   char      *lptr, line[1024];
   string     headerString;
   FieldInfo *fieldInfo;

   //
   // Instantiate header information derived from user parameters
   // These header values are not found in the input file headers
   //
   fieldInfo = getHeaderField( "SiteName", FTString,  4, 0 );
   fieldInfo->valueStr = _params->site_name;

   fieldInfo = getHeaderField( "SiteId", FTInteger, 5, 0 );
   fieldInfo->valueInt = _params->site_id;

   //
   // The remaining header fields are included in the shapefile upon request
   // It's most efficient to read these in the anticipated order
   //
   // Get the Project id, if requested
   //
   if ( headerRequested( Params::PROJECT_ID, headerString )) {
      char *projId;
      lptr = getHeaderLine( headerString.c_str(), line );
      if ( lptr ) {
         projId = getHeaderContent( line );
         if ( projId == NULL ) {
            POSTMSG( WARNING, "   Requested header field "
                              "contains no content: %s",
                              headerString.c_str() );
         }
         fieldInfo = getHeaderField( "Project", FTString, 32, 0 );
         fieldInfo->valueStr = projId;
      }
      else {
         POSTMSG( WARNING, "   Requested header field does not exist: %s",
                            headerString.c_str() );
      }
   }

   //
   // Get the launch site type, if requested
   // This can be overridden by the site_descipt parameter
   //
   char shapeField[] = "SiteDescrip";

   if ( !ISEMPTY( _params->site_descrip ) ) {
      fieldInfo = getHeaderField( shapeField, FTString, 32, 0 );
      fieldInfo->valueStr = _params->site_descrip;
   }
   else if ( headerRequested( Params::LAUNCH_SITE_TYPE, headerString )) {
      char *siteDescrip;
      lptr = getHeaderLine( headerString.c_str(), line );
      if ( lptr ) {
         siteDescrip = getHeaderContent( line );
         if ( siteDescrip == NULL ) {
            POSTMSG( WARNING, "   Requested header field "
                              "contains no content: %s",
                              headerString.c_str() );
         }
         fieldInfo = getHeaderField( shapeField, FTString, 32, 0 );
         fieldInfo->valueStr = siteDescrip;
      }
      else {
         POSTMSG( WARNING, "   Requested header field does not exist: %s",
                            headerString.c_str() );
      }
   }

   //
   // Get the launch location -- this is always needed for the .shp file
   // NOTE: This header string MUST occur as "Launch Location"
   // However, we write it as an .dbf attribute field only if requested
   //
   lat = lon = alt = 0.0;
   lptr = getHeaderLine( "Launch Location", line );
   if ( lptr ) {
      sscanf( lptr, "%[^:]:%[^,],%[^,],%lf,%lf,%lf", junk, junk, junk,
                    &lon, &lat, &alt );

      if ( headerRequested( Params::LAUNCH_LOCATION, headerString )) {
         //
         // Add the launch location to our list of header fields
         //
         fieldInfo = getHeaderField( "LaunchLat", FTDouble, 7, 3 );
         fieldInfo->valueDbl = lat;

         fieldInfo = getHeaderField( "LaunchLon", FTDouble, 7, 3 );
         fieldInfo->valueDbl = lon;

         fieldInfo = getHeaderField( "LaunchAlt", FTDouble, 7, 3 );
         fieldInfo->valueDbl = alt;

         POSTMSG( DEBUG, "   Launched from (lat, lon) (%lf, %lf)", lat, lon );
      }
   }
   else {
      POSTMSG( ERROR, "   Location header field does not exist!" );
      return( -1 );
   }

   //
   // Get the launch time, if requested
   // NOTE -- it is not always the same as the file time
   //
   if ( headerRequested( Params::LAUNCH_TIME_UTC, headerString )) {
      lptr = getHeaderLine( headerString.c_str(), line );
      if ( lptr ) {
         sscanf( lptr, "%[^:]:%[^0-9]%d, %d, %d, %d:%d:%d",
                        junk, junk, &year, &month, &day, &hour, &min, &sec );
         //
         // Add the launch time to our list of header fields
         // For now we store it as six fields: YYYY MM DD HH MM SS
         //
         DateTime when( year, month, day, hour, min, sec );
         fieldInfo = getHeaderField( "LaunchYear", FTInteger, 4, 0 );
         fieldInfo->valueInt = year;
         fieldInfo = getHeaderField( "LaunchMon", FTInteger, 2, 0 );
         fieldInfo->valueInt = month;
         fieldInfo = getHeaderField( "LaunchDay", FTInteger, 2, 0 );
         fieldInfo->valueInt = day;
         fieldInfo = getHeaderField( "LaunchHour", FTInteger, 2, 0 );
         fieldInfo->valueInt = hour;
         fieldInfo = getHeaderField( "LaunchMin", FTInteger, 2, 0 );
         fieldInfo->valueInt = min;
         fieldInfo = getHeaderField( "LaunchSec", FTInteger, 2, 0 );
         fieldInfo->valueInt = sec;
         fieldInfo = getHeaderField( "LaunchUtime", FTInteger, 10, 0 );
         fieldInfo->valueInt = when.utime();

         POSTMSG( DEBUG, "   Launched at %s", when.dtime() );
      }
      else {
         POSTMSG( WARNING, "   Requested header field does not exist: %s",
                            headerString.c_str() );
      }

   }

   //
   // Get the sonde type, if requested
   //
   if ( headerRequested( Params::SONDE_TYPE, headerString )) {
      char *sondeType;
      lptr = getHeaderLine( headerString.c_str(), line );
      if ( lptr ) {
         sondeType = getHeaderContent( line );
         if ( sondeType == NULL ) {
            POSTMSG( WARNING, "   Requested header field "
                              "contains no content: %s",
                              headerString.c_str() );
         }
         fieldInfo = getHeaderField( "SondeType", FTString, 32, 0 );
         fieldInfo->valueStr = sondeType;
      }
      else {
         POSTMSG( WARNING, "   Requested header field does not exist: %s",
                            headerString.c_str() );
      }
   }

   //
   // Get the sensor id, if requested
   // NOTE: this gets stored it in the serial number field!!!
   //
   if ( headerRequested( Params::SENSOR_ID, headerString )) {
      char sensorID[32];
      lptr = getHeaderLine( headerString.c_str(), line );
      if ( lptr ) {
         sscanf( lptr, "%[^:]:%s", junk, sensorID );
         fieldInfo = getHeaderField( "SerialNo", FTString, 32, 0 );
         fieldInfo->valueStr = sensorID;
      }
      else {
         POSTMSG( WARNING, "   Requested header field does not exist: %s",
                            headerString.c_str() );
      }
   }

   //
   // Get the sonde operator, if requested
   //
   if ( headerRequested( Params::SONDE_OPERATOR, headerString )) {
      char *sondeOperator;
      lptr = getHeaderLine( headerString.c_str(), line );
      if ( lptr ) {
         sondeOperator = getHeaderContent( line, 0 );
         if ( sondeOperator == NULL ) {
            POSTMSG( WARNING, "   Requested header field "
                              "contains no content: %s",
                              headerString.c_str() );
         }
         fieldInfo = getHeaderField( "Operator", FTString, 32, 0 );
         fieldInfo->valueStr = sondeOperator;
      }
      else {
         POSTMSG( WARNING, "   Requested header field does not exist: %s",
                            headerString.c_str() );
      }
   }

   //
   // Get the sonde manufacturer, if requested
   // NOTE: this gets stored it in the sonde type field!!!
   //
   if ( headerRequested( Params::SONDE_MANUFACTURER, headerString )) {
      char *sondeType;
      lptr = getHeaderLine( headerString.c_str(), line );
      if ( lptr ) {
         sondeType = getHeaderContent( line );
         if ( sondeType == NULL ) {
            POSTMSG( WARNING, "   Requested header field "
                              "contains no content: %s",
                              headerString.c_str() );
         }
         fieldInfo = getHeaderField( "SondeType", FTString, 32, 0 );
         fieldInfo->valueStr = sondeType;
      }
      else {
         POSTMSG( WARNING, "   Requested header field does not exist: %s",
                            headerString.c_str() );
      }
   }

   //
   // Get the sonde serial number, if requested
   //
   if ( headerRequested( Params::SONDE_SERIAL_NO, headerString )) {
      char serialNo[32];
      lptr = getHeaderLine( headerString.c_str(), line );
      if ( lptr ) {
         sscanf( lptr, "%[^:]:%s", junk, serialNo );
         fieldInfo = getHeaderField( "SerialNo", FTString, 32, 0 );
         fieldInfo->valueStr = serialNo;
      }
      else {
         POSTMSG( WARNING, "   Requested header field does not exist: %s",
                            headerString.c_str() );
      }
   }

   //
   // Get the ascension number, if requested
   //
   if ( headerRequested( Params::SONDE_ASCENSION_NO, headerString )) {
      char ascensionNo[32];
      lptr = getHeaderLine( headerString.c_str(), line );
      if ( lptr ) {
         sscanf( lptr, "%[^:]:%s", junk, ascensionNo );
         fieldInfo = getHeaderField( "AscensionNo", FTString, 32, 0 );
         fieldInfo->valueStr = ascensionNo;
      }
      else {
         POSTMSG( WARNING, "   Requested header field does not exist: %s",
                            headerString.c_str() );
      }
   }

   return 0;
}

bool
DataMgr::headerRequested( Params::header_id_t headerId, 
                          string& headerString ) 
{
   for( int i=0; i < _params->header_info_n; i++ ) {
      if ( _params->_header_info[i].headerId == headerId  &&
           _params->_header_info[i].includeInShapefile == TRUE ) {
         headerString = _params->_header_info[i].headerName;
         return( TRUE );
      }
   }
   return( FALSE );
}

char *
DataMgr::getHeaderLine( const char* label, char lineText[] )
{
   long  posStart, posCurrent = -1L;
   bool  found = false;
   char  line[BUFSIZ], *lptr=NULL;

   //
   // Degenerate case
   //
   if ( ISEMPTY( label ) ) {
      return( NULL );
   }

   //
   // Hang onto the starting file position
   //
   if ( (posStart = ftell(inputFile)) == -1L ) {
      return NULL;
   }

   //
   // Read each line until we either find what we're looking for
   // or we come back around to our starting point
   //
   while( posCurrent != posStart ) {
      lptr = fgets( line, BUFSIZ, inputFile );
      posCurrent = ftell( inputFile );

      if ( feof( inputFile ) ) {
         //
         // We've reached the end of file -- wrap around
         // 
         rewind( inputFile );
         posCurrent = ftell( inputFile );
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

char *
DataMgr::getHeaderContent( const char* lineText, int contentPos )
{
   //
   // Degenerate case
   //
   assert( lineText != NULL );
   if ( ISEMPTY( lineText ) ) {
      return( NULL );
   }

   //
   // Move to the the header colon (:)
   //
   char *bytePos = (char*)lineText;
   while( *bytePos != ':'  && bytePos != '\0'  &&  *bytePos != '\n' ) {
      bytePos++;
   }
   if ( *bytePos == '\0' || *bytePos == '\n' ) {
      return( NULL );
   }
   
   //
   // Move beyond subsequent whitespace
   //
   bytePos++;
   while( *bytePos == ' '  && *bytePos != '\0'  &&  *bytePos != '\n' ) {
      bytePos++;
   }
   if ( *bytePos == '\0' || *bytePos == '\n' ) {
      return( NULL );
   }

   //
   // Presumably, the rest of the line is the content that we are looking for
   //
   if ( contentPos == -1 ) {
      //
      // Return all of the content
      //
      return( bytePos );
   }
   else {
      //
      // Return the specified comma-separated field
      //
      int currentPos = 0;
      while( currentPos < contentPos ) {
         while( *bytePos != ',' && *bytePos != '\0'  &&  *bytePos != '\n' ) {
            bytePos++;
         }
         if ( *bytePos == '\0' || *bytePos == '\n' ) {
            return( NULL );
         }
         currentPos++;
      }

      //
      // We're at the correct comma-separated field, 
      // read only up to the next comma, if any
      //
      char *endPos = bytePos;
      while( *endPos != ',' && *endPos != '\0'  &&  *endPos != '\n' ) {
         endPos++;
      }
      *endPos = '\0';
      return( bytePos );
   }
}

int
DataMgr::findColumns()
{
   int        column = 0;
   size_t     i;
   char      *fieldName, line[BUFSIZ];
   FieldInfo *fieldInfo;

   //
   // Find the labels line.  It must start with the label "Time".
   //
   if ( fgets( line, BUFSIZ, inputFile ) == NULL ) {
      return -1;
   }
   fieldName = strtok( line, DELIMETER );
   while( (fieldName == NULL ) ||
         ((fieldName != NULL) && (strcmp( fieldName, "Time" ) != 0)) ) {
      if ( fgets( line, BUFSIZ, inputFile ) == NULL ) {
         return -1;
      }
      fieldName = strtok( line, DELIMETER );
   }

   //
   // Look for all the fieldNames we need and note the column for each.
   //
   while( fieldName != NULL ) {

      for( i=0; i < dataFields.size();  i++ ) {
         fieldInfo = dataFields[i];
         if ( fieldInfo->classFieldName == fieldName ) {
            fieldInfo->classColumn = column;
            break;
         }
      }

      fieldName = strtok(NULL, DELIMETER);
      column++;
   }

   //
   // We now know enough to define what the output shapefile should look like
   // If this is our first time through, create output fields in the shapefile
   // These include user header fields as well as data fields
   //
   if ( firstCall ) {
      //
      // Header fields
      //
      for( i=0; i < headerFields.size();  i++ ) {
         fieldInfo = headerFields[i];
         if ( createDbfField( fieldInfo ) == -1 ) {
            POSTMSG( ERROR, "   Unable to add field '%s' to shapefile",
                                fieldInfo->shapeFieldName.c_str() );
            return( -1 );
         }
      }

      //
      // Data fields
      //
      for( i=0; i < dataFields.size();  i++ ) {
         fieldInfo = dataFields[i];
         if ( fieldInfo->classColumn == -1 ) {
            POSTMSG( WARNING, "   Field '%s' was not found!", 
                                  fieldInfo->classFieldName.c_str() );
         }
         else if ( createDbfField( fieldInfo ) == -1 ) {
            POSTMSG( ERROR, "   Unable to add field '%s' to shapefile", 
                                fieldInfo->shapeFieldName.c_str() );
            return( -1 );
         }
      }
   }
 
   return 0;
}

int
DataMgr::findFirstData()
{
   char *fieldName, line[BUFSIZ];
   long fpos;

   //
   // Hang onto the current file position.
   //
   if ( (fpos = ftell(inputFile)) == -1L ) {
      return -1;
   }

   //
   // Look for the first line of data.  There might be non-data lines between
   // the labels line and the first line of data.
   //
   if ( fgets( line, BUFSIZ, inputFile ) == NULL ) {
      return -1;
   }
   fieldName = strtok( line, DELIMETER );

   //
   // Check the first two chars of field since a digit may be negative and
   // thus, start with '-'.
   //
   while( (fieldName == NULL) ||
          ((fieldName != NULL) && (!isdigit(*fieldName) && 
                                   !isdigit(*(fieldName+1))))) {
      //
      // Get the current file position.
      //
      if ( (fpos = ftell(inputFile)) == -1L ) {
          return -1;
      }

      if ( fgets( line, BUFSIZ, inputFile ) == NULL ) {
         return -1;
      }
      fieldName = strtok(line, DELIMETER);
   }

   //
   // Reset the file pointer to the beginning of this line of data.
   //
   fseek( inputFile, fpos, SEEK_SET );
   return 0;
}

int
DataMgr::processData()
{
   int             targetCol, icol, status;
   size_t          ifield;
   double          value;
   FieldInfo      *fieldInfo;

   char            line[BUFSIZ], lineCopy[BUFSIZ], *fptr;
   const char      *BLANK = " ";

   //
   // Read in each line of data.
   //
   while ( fgets( line, BUFSIZ, inputFile ) != NULL ) {

      //
      // Get data values for each field of interest
      //
      for( ifield=0;  ifield < dataFields.size(); ifield ++ ) {

         fieldInfo = dataFields[ifield];

         //
         // Get the nth column value from the line 
         // for this field of interest 
         //
         status = 1;
         targetCol = fieldInfo->classColumn;
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
            //
            fieldInfo->valueDbl = value;
         }
      }
      //
      // After processing each input line, 
      // write the corresponding shapefile record
      //
      writeShapeRecord();

      //
      // For testing purposes only -- bail out after one record input
      // 
      if ( singleLevel ) {
         return 0 ;
      }
   }

   return 0;
}

int
DataMgr::writeShapeRecord()
{
   int         shapeIndex;
   size_t      i;
   FieldInfo  *fieldInfo;

   //
   // Create a simple point object to be used for writing shapefile records
   //
   shpObject = SHPCreateSimpleObject( SHPT_POINT, 1, &lon, &lat, &alt );

   //
   // Write the shapefile point location record
   //
   shapeIndex = SHPWriteObject( outputShp, -1, shpObject );
   SHPDestroyObject( shpObject );

   //
   // Write the corresponding dbf record of header data
   //
   for( i=0; i < headerFields.size();  i++ ) {
      fieldInfo = headerFields[i];
      if ( writeDbfField( shapeIndex, fieldInfo ) == -1 ) {
         return( -1 );
      }
   }

   //
   // Write the corresponding dbf record of field data
   // for which a shapefile column was successfully created
   //
   for( i=0; i < dataFields.size();  i++ ) {
      fieldInfo = dataFields[i];
      if ( fieldInfo->shapeColumn > -1 ) {
         if ( writeDbfField( shapeIndex, fieldInfo ) == -1 ) {
            return( -1 );
         }
      }
   }

   return( 0 );
}

int
DataMgr::createDbfField( FieldInfo *fieldInfo )
{
   fieldInfo->shapeColumn = DBFAddField( outputDbf, 
                                         fieldInfo->shapeFieldName.c_str(), 
                                         fieldInfo->dataType, 
                                         fieldInfo->fieldWidth, 
                                         fieldInfo->fieldPrecision );

   return( fieldInfo->shapeColumn );
}

int
DataMgr::writeDbfField( int recordNum, FieldInfo *fieldInfo )
{
   int status = 0;

   switch( fieldInfo->dataType ) {
      case FTInteger:
           status = DBFWriteIntegerAttribute( outputDbf, recordNum,
                                              fieldInfo->shapeColumn, 
                                              fieldInfo->valueInt );
           break;

      case FTDouble:
           //
           // Missing values are not stored, i.e., remain an null db values
           //
           if ( fieldInfo->valueDbl == fieldInfo->missingValue ) {
              break;
           }

           status = DBFWriteDoubleAttribute( outputDbf, recordNum,
                                             fieldInfo->shapeColumn, 
                                             fieldInfo->valueDbl );
           break;

      case FTString:
           status = DBFWriteStringAttribute( outputDbf, recordNum,
                                             fieldInfo->shapeColumn, 
                                             fieldInfo->valueStr.c_str() );
           break;

      case FTInvalid:
           POSTMSG( ERROR, "Attempt to write invalid data type to shapefile" );
           status = -1;
   }

   if ( status == -1 ) {
      POSTMSG( ERROR, "Unable to write record/field %d/%d to shapefile", 
                       recordNum, fieldInfo->shapeColumn );
      return( -1 );
   }

   return( 0 );
}

int
DataMgr::processEvent()
{
   int        columnNum, status=0;
   FieldInfo *fieldInfo;
   int        eventRecord = DBFGetRecordCount( eventDbf );

   if ( DBFGetFieldCount( eventDbf ) == 0 ) {
      status = DBFAddField( eventDbf, "SiteName", FTString,  4, 0 );
      status = DBFAddField( eventDbf, "EventUtime", FTInteger, 10, 0 );
      status = DBFAddField( eventDbf, "StartTime", FTString, 20, 0 );
      status = DBFAddField( eventDbf, "EndTime", FTString, 20, 0 );
   }

   //
   // Event field: SiteName
   //
   fieldInfo = getHeaderField( "SiteName", FTString,  4, 0 );
   columnNum = 0;
   status = DBFWriteStringAttribute( eventDbf, eventRecord, columnNum, 
                                      fieldInfo->valueStr.c_str() );

   //
   // Event field: LaunchUtim
   //
   fieldInfo = getHeaderField( "LaunchUtime", FTInteger, 10, 0 );
   columnNum = 1;
   status = DBFWriteIntegerAttribute( eventDbf, eventRecord, columnNum,
                                       fieldInfo->valueInt );

   //
   // Event field: StartTime (derived from LaunchUtime above)
   //
   DateTime start( fieldInfo->valueInt );
   columnNum = 2;
   status = DBFWriteStringAttribute( eventDbf, eventRecord, columnNum, 
                                     start.getStrPlain().c_str() );

   //
   // Event field: EndTime
   //
   columnNum = 3;
   status = DBFWriteStringAttribute( eventDbf, eventRecord, columnNum, 
                                     fileTime.getStrPlain().c_str() );

   if ( status == -1 ) {
      POSTMSG( ERROR, "   Unable to write record %d to eventfile", 
                          eventRecord );
      return( -1 );
   }

   return( 0 );
}
