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
//  Ingest of axf profiler data and output of spdb data
//
//  Notes about the AXF format for profiler data:
//
//  * Bad data values are -9999.0
//  * Assumed AXF units are:
//    WindDir (deg), WindSpeed (m/s), Height (m)
//  * Values are converted when necessary for storage into spdb database.   
//    Uwind (m/s), Vwind (m/s), Alt (m),  Press (mb)
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  January 2000
//
//  $Id: DataMgr.cc,v 1.6 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <strstream>

#include <sys/stat.h>
#include <rapmath/umath.h>
#include <physics/thermo.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/MsgLog.hh>

#include "AxfProfiler.hh"
#include "DataMgr.hh"
using namespace std;

//
// static definitions
//
const double DataMgr::WIND_DIR_MISSING     = -99.0;
const double DataMgr::WIND_SPEED_MISSING   = -99.90;
const double DataMgr::WIND_W_MISSING       = -99.99;

const double DataMgr::SPDB_MISSING_VALUE   = -9999.0;
const char*  DataMgr::INDEX_FILENAME       = "axfProfiler.index";

DataMgr::DataMgr() :
//
//        Field         Start    Field
//       Contents      Position, Width
//
         HEIGHT           (   0,  7 ),
         WIND_DIR         (   7,  8 ),
         WIND_SPEED       (  15, 15 ),
         WIND_W           (  30, 15 ),
         QUAL_HORIZ       (  45,  8 ),
         QUAL_VERT        (  53,  8 ),
         TIME_STAMP       ( 144, 20 ),
         STATION_ID       ( 184,  6 ),
         STATION_NAME     ( 190, 35 )
{
   numPoints     = 0;
   expireSecs    = 0;
   setTimeNow    = false;
   statTime      = DateTime::NEVER;
   dataTime      = DateTime::NEVER;
   ldataIndex    = NULL;
}

DataMgr::~DataMgr()
{
   clearData();
   delete ldataIndex;
}

void
DataMgr::clearData()
{
   assert( numPoints == height.size()
        && numPoints == uwind.size()
        && numPoints == vwind.size()
        && numPoints == wwind.size()
        && numPoints == pressure.size() );

   for( size_t i=0; i < numPoints; i++ ) {
      height.pop_back();
      uwind.pop_back();
      vwind.pop_back();
      wwind.pop_back();
      pressure.pop_back();
   }
}

int
DataMgr::init( Params &params )
{
   //
   // Save relevant parameters
   //
   inputFile       = params.input_file_path;
   expireSecs      = params.expire_secs;

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
   sounding.init( spdbUrls, Sounding::PROFILER_ID, "AXF" );

   sounding.setMissingValue( SPDB_MISSING_VALUE );

   //
   // Set location info
   //  
   sounding.setLocation( params.station_latitude, 
                         params.station_longitude, 
                         params.station_altitude );

   //
   // Initialize time management using the ldata index file
   // Don't use rapDataDir because it will already be handled
   // via DsInputPath file management upstream of us
   // By default we put the index file into the input_file_path
   // unless the user has explicitly set to a different directory
   //
   if ( !ISEMPTY( params.index_dir ) ) {
      ldataIndex = new LdataInfo( params.index_dir, false, 
                                  INDEX_FILENAME );
   }
   else {
      ldataIndex = new LdataInfo( inputFile.getDirectory(), false,
                                  INDEX_FILENAME );
   }
   if ( !Path::exists( ldataIndex->getInfoPath() )) {
      //
      // no index file yet
      //
      POSTMSG( DEBUG, "No index file yet: '%s'", 
                       ldataIndex->getInfoPath().c_str());
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
         // Store statTime of most recently processed file
         // based on the contents of the index file
         //
         statTime.set( ldataIndex->getLatestTime() );
         POSTMSG( DEBUG, "Latest time in index: %s", statTime.dtime() );
      }
   }

   return( 0 );
}

int
DataMgr::ingest()
{
   struct stat statInfo;

   //
   // Get the modification time of the current data file
   //
   if ( !Path::stat( inputFile, statInfo ) ) {
      POSTMSG( DEBUG, "No input data file yet: '%s'", 
                       inputFile.getPath().c_str());
      return( 0 );
   }

   //
   // See if the data file is more current than the last
   //
   if ( statTime < statInfo.st_mtime ) {

      //
      // Process the file
      // Continue on to the next file if the current file has problems
      //
      PMU_auto_register("Reading sounding file");
      if( processFile() != 0 ) {
         return( -1 );
      }

      //
      // Save the statTime and index file for comparison next time around
      //
      statTime = statInfo.st_mtime;
      if ( ldataIndex->write( statTime.utime() ) != 0 ) {
         POSTMSG( WARNING, "Could not write index file "
                           "to database directory '%s'", 
                           inputFile.getDirectory().c_str() );
      }
      else {
         POSTMSG( DEBUG, "   Wrote the stat index for time %s",
                             statTime.dtime() );
      }
   }
   else if ( DEBUG_ENABLED ) {
      POSTMSG( DEBUG, "Data file '%s' has not changed.",
                       inputFile.getFile().c_str() );
   }

   return( 0 );
}

int
DataMgr::processFile()
{
   int   status   = 0;
   char *filePath = (char*)inputFile.getPath().c_str();
   char *fileName = (char*)inputFile.getFile().c_str();

   POSTMSG( DEBUG, "Processing file: %s", fileName );

   //
   // Open the file as an input stream
   //
   inputStream.open( filePath );
   if ( !inputStream ) {
      POSTMSG( WARNING, "Cannot open sounding file '%s'", filePath );
      return( 0 );
   }

   while( !inputStream.eof() ) {

      //
      // Clear out any old data in our data vectors
      //
      clearData();

      //
      // Read the data file
      //
      if ( readData() != 0 ) {
         POSTMSG( WARNING, "Unexpected results reading data from file '%s'\n"
                           "Skipping current file.", 
                           fileName );
         status = 0;
         break;
      }

      //
      // Write out the sounding
      // Bail out if the write fails for any reason
      //
      PMU_auto_register( "Writing sounding spdb" );
      if ( writeSounding() != 0 ) {
         POSTMSG( ERROR, "Check validity of output destination(s)." );
         status = -1;
         break;
      }
   }

   //
   // Close the file
   //
   inputStream.close();

   return( status );
}

int
DataMgr::writeSounding()
{
   int     status;
   time_t  when = dataTime.utime();

   vector<double>  *noRelHumidity = NULL;
   vector<double>  *noTemperature = NULL;

   //
   // Set the data values in the Sounding class
   //
   status = sounding.set( when, &height, &uwind, &vwind, &wwind,
                          &pressure, noRelHumidity, noTemperature );
   if ( status != 0 ) {
      POSTMSG( ERROR, "Could not set the sounding values." );
      return( -1 );
   }

   //
   // Write the sounding to the database
   //
   status = sounding.writeSounding( when, when + expireSecs );

   if ( status != 0 ) {
      POSTMSG( ERROR, "Could not write to the sounding database(s)." );
   }
   else {
      POSTMSG( DEBUG, "   %5d sounding observations written at time %s", 
                          height.size(), dataTime.dtime() );
   }

   return( status );
}

int
DataMgr::readData()
{
   float           u, v;
   int             stationId;
   double          direction, speed, altitude;
   DateTime        timestamp;
   char            name[STATION_NAME.second + 1];
   char            aifstime[TIME_STAMP.second + 1 ];
   char            line[BUFSIZ];
   streampos       pos;
   istrstream      iss( line, BUFSIZ );

   //
   // Read in each line of data and convert the values to the expected units
   //
   numPoints = 0;

   //
   // Hang onto the current input file stream position
   // in case we have to back up
   //
   pos = inputStream.tellg();

   for( inputStream.getline( line, BUFSIZ );  inputStream; 
        inputStream.getline( line, BUFSIZ ) ) {

      //
      // STATION_NAME:
      //
      if ( numPoints == 0 ) {
         //
         // Set the STATION_ID and STATION_NAME the first time around
         //
         iss.seekg( STATION_NAME.first, ios::beg );
         iss.width( STATION_NAME.second + 1 );
         iss.get( name, STATION_NAME.second + 1 );
         STRblnk( name );
         sounding.setSiteName( name );

         iss.seekg( STATION_ID.first, ios::beg );
         iss >> stationId;
         sounding.setSiteId( stationId );
      }

      //
      // TIME_STAMP: see if we've changed sounding times
      //
      iss.seekg( TIME_STAMP.first, ios::beg );
      iss >> aifstime;
      if ( setTimeNow ) {
         timestamp = time( NULL );
      }
      else {
         timestamp = aifstime;
      }
      if ( numPoints == 0 ) {
         dataTime = timestamp;
      }
      else if ( dataTime != timestamp ) {
         //
         // Set the input file stream's get pointer 
         // back to the begining of this line (pos)
         // so that it is re-read for the next entry into the database
         //
         inputStream.seekg( pos , ios::beg);
         return 0;
      }
         
      //
      // HEIGHT: convert to pressure
      //
      iss.seekg( HEIGHT.first, ios::beg );
      iss >> altitude;
      height.push_back( altitude );
      pressure.push_back( PHYmeters2mb( altitude ) );

      //
      // WIND_DIR / WIND_SPEED: convert to u/v components
      //
      iss.seekg( WIND_DIR.first, ios::beg );
      iss >> direction;

      iss.seekg( WIND_SPEED.first, ios::beg );
      iss >> speed;

      if ( direction != WIND_DIR_MISSING  &&  speed != WIND_SPEED_MISSING ) {
         wind_dir_speed_2_uv( (float)speed, (float)direction, &u, &v );
         uwind.push_back( u );
         vwind.push_back( v );
      }
      else {
         uwind.push_back( SPDB_MISSING_VALUE );
         vwind.push_back( SPDB_MISSING_VALUE );
      }

      //
      // WIND_W: take this value as is
      //
      iss.seekg( WIND_W.first, ios::beg );
      iss >> speed;
      if ( speed != WIND_W_MISSING ) {
         wwind.push_back( speed );
      }
      else {
         wwind.push_back( SPDB_MISSING_VALUE );
      }

      //
      // Increment the number of valid points,
      // reset the read position of the input buffer,
      // and hang onto the current input file stream position
      //
      numPoints++;
      pos = inputStream.tellg();
   }


   //
   // See if we found any data
   //
   if ( numPoints == 0 ) {
      POSTMSG( ERROR, "No data points processed from the profiler data" );
      return -1;
   }

   return 0;
}
