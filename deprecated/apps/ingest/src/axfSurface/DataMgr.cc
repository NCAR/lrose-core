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
//  Ingest of axf surface observation file and output of spdb data
//
//  Notes about the AXF format for surface observation files:
//
//  * Bad data values are -9999.0
//  * Assumed AXF units are:
//    StationHeight (m), WindDir (deg), WindSpeed (knots), WindGust (m/s), 
//    AirTemp (C), DewPoint (C), RelHumidity (%), SeaPress (mb)
//  * Values are converted when necessary for storage into spdb database.   
//    Alt (m), Uwind (m/s), Vwind (m/s), maxGust (m/s ),
//    Temp (C), RH (%), Press (mb)
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  August 1999
//
//  $Id: DataMgr.cc,v 1.14 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <strstream>

#include <sys/stat.h>
#include <rapmath/umath.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <toolsa/MsgLog.hh>
#include <Spdb/Sounding.hh>

#include "AxfSurface.hh"
#include "StationReport.hh"
#include "StationDB.hh"
#include "DataMgr.hh"
using namespace std;

//
// static definitions
//
const double DataMgr::MISSING_VALUE     = -9999.0;
const char*  DataMgr::INDEX_FILENAME    = "axfSurface.index";

DataMgr::DataMgr() :
//
//        Field         Start    Field
//       Contents      Position, Width
//
         STATION_NAME     (   0, 19 ),
         STATION_DESCRIP  (  19, 30 ),
         STATION_ID       (  49,  8 ),
         STATION_LAT      (  57, 15 ),
         STATION_LON      (  72, 15 ),
         STATION_ALT      (  87, 15 ),
         TIME_STAMP       ( 102, 15 ),
         WIND_DIR         ( 117,  8 ),
         WIND_SPEED       ( 125,  8 ),
         WIND_GUST        ( 133,  8 ),
         TEMPERATURE      ( 141, 15 ),
         DEW_POINT        ( 156, 15 ),
         REL_HUMIDITY     ( 171,  8 ),
         PRESSURE         ( 179, 15 ),
         VISIBILITY       ( 224, 8 )
{
   statTime      = DateTime::NEVER;
   dataTime      = DateTime::NEVER;
   setTimeNow    = false;
   database      = NULL;
   ldataIndex    = NULL;
   doClipping    = false;
   clippingArea  = NULL;
}

DataMgr::~DataMgr()
{
   clearObservations();
   clearStations();

   delete database;
   delete ldataIndex;
   delete clippingArea;
}

void
DataMgr::clearObservations()
{
   //
   // Clear out references to station records
   //
   // NOTE: this does not free up the station records
   //       it only clears out the list of observations
   //       that are written to the database
   //
   while( observations.size() > 0 ) {
      observations.pop_back();
   }
}

void
DataMgr::clearStations()
{
   //
   // Delete all of the station records
   //
   map< int, StationReport*, less<int> >::iterator item;

   for( item=stations.begin(); item != stations.end(); item++ ) {
      delete (*item).second;
   }
}

int
DataMgr::init( Params &params )
{
   int i;
   int status = 0;

   //
   // Save relevant parameters
   //
   inputFile       = params.input_file_path;
   numDestinations = params.spdb_destinations_n;

   if ( params.limit_data_region ) {
      switch ( params.data_region_n ) {
         case 0:
              POSTMSG( ERROR, "No data region points have been specified." );
              status = -1;
              break;
         case 1:
              POSTMSG( ERROR, "Must specify at least two data region points." );
              status = -1;
              break;
         case 2:
              //
              // The two clipping points are assumed to be 
              // lowerLeft and upperRight -- fill in the missing two points
              //
              numVerticies = 4;
              clippingArea = new Point_d[numVerticies];
              clippingArea[0].x = PJGrange360( params._data_region[0].lon );
              clippingArea[0].y = PJGrange360( params._data_region[0].lat );
              clippingArea[1].x = PJGrange360( params._data_region[0].lon );
              clippingArea[1].y = PJGrange360( params._data_region[1].lat );
              clippingArea[2].x = PJGrange360( params._data_region[1].lon );
              clippingArea[2].y = PJGrange360( params._data_region[1].lat );
              clippingArea[3].x = PJGrange360( params._data_region[1].lon );
              clippingArea[3].y = PJGrange360( params._data_region[0].lat );
              break;
         default:
              numVerticies = params.data_region_n;
              clippingArea = new Point_d[numVerticies];
              for( i=0; i < params.data_region_n; i++ ) {
                 clippingArea[i].x = PJGrange360( params._data_region[i].lon );
                 clippingArea[i].y = PJGrange360( params._data_region[i].lat );
              }
              break;
      }
      if ( status == 0 ) {
         doClipping = true;
      }
      else {
         return( status );
      }
   }

   // 
   // Instantiate the station database
   //
   POSTMSG( DEBUG, "Initializing the station database." );
   database = new StationDB( params._spdb_destinations,
                             params.spdb_destinations_n,
                             params.expire_secs );

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
      PMU_auto_register("Reading surface station file");
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
   size_t  numObs   = 0;
   int     status   = 0;
   char   *filePath = (char*)inputFile.getPath().c_str();
   char   *fileName = (char*)inputFile.getFile().c_str();

   POSTMSG( DEBUG, "Processing file: %s", fileName );

   //
   // Open the file as an input stream
   //
   inputStream.open( filePath );
   if ( !inputStream ) {
      POSTMSG( WARNING, "Cannot open surface observation file '%s'", filePath );
      return( 0 );
   }

   while( !inputStream.eof() ) {

      //
      // Clear out any references to previous surface observations
      //
      clearObservations();

      //
      // Read the data file, adding observations to the database list
      //
      if ( readData() != 0 ) {
         POSTMSG( WARNING, "Unexpected results reading observations "
                           "from file '%s'\n"
                           "Skipping current file.", fileName );
         status = 0;
         break;
      }
      else {
         numObs += observations.size();
      }

      //
      // Write out the surface stations for the current time 
      // Bail out if the write fails for any reason
      //
      PMU_auto_register( "Writing to surface observation database" );
      if ( writeObservations() != 0 ) {
         POSTMSG( ERROR, "Unable to write surface observations for time %s",
                          dataTime.dtime() );
         status = -1;
         break;
      }
   }

   //
   // How much data did we process from this file?
   //
   POSTMSG( DEBUG, "   Processed %d observations from the current file.",
                       numObs );

   //
   // Close the file
   //
   inputStream.close();

   return( status );
}

int
DataMgr::writeObservations()
{
   int     status = 0;
   size_t  i;

   //
   // Reset the database and add the current observations
   //
   database->reset( observations.size() );
   for( i=0; status == 0  &&  i < observations.size(); i++ ) {
      status = database->add( *(observations[i]) );
   }

   if ( status == 0 ) {
      status = database->write();
   }

   if ( status  != numDestinations ) {
      if ( numDestinations == 1 ) {
         POSTMSG( ERROR, "Could not write to the "
                         "surface observation database." );
      }
      else {
         POSTMSG( ERROR, "Could not write to all the "
                         "surface observation databases.\n"
                         "Expected to write to %d destinations -- "
                         "only wrote to %d.",
                          numDestinations, status );
      }
      return( -1 );
   }
   POSTMSG( DEBUG, "%5d surface obs written for time %s", 
                       observations.size(), dataTime.dtime() );

   return( 0 );
}

int
DataMgr::readData()
{
   int             id;
   float           windDir, windSpeed, windGust;
   float           temperature, dewPoint, relHumidity, pressure, visibility;
   float           latitude, longitude, altitude;
   Point_d         location;
   DateTime        timestamp;
   char            name[STATION_NAME.second + 1];
   char            description[STATION_DESCRIP.second + 1];
   char            aifstime[TIME_STAMP.second + 1 ];
   char            line[BUFSIZ];
   StationReport  *stationReport;
   streampos       pos;
   istrstream      iss( line, BUFSIZ );

   map< int, StationReport*, less<int> >::iterator item;

   //
   // Hang onto the current input file stream position
   // in case we have to back up
   //
   pos = inputStream.tellg();

   //
   // Read in each line of data and convert the values to the expected units
   //
   for( inputStream.getline( line, BUFSIZ );  inputStream; 
        inputStream.getline( line, BUFSIZ ) ) {

      //
      // STATION_ID: see if we already have created a station record
      //
      iss.seekg( STATION_ID.first, ios::beg );
      iss >> id;
      item = stations.find( id );
      if ( item != stations.end() ) {
         stationReport = (*item).second;
      }
      else {
         //
         // We've never seen this station before
         //
         // STATION_NAME: get the short station name
         //
         iss.seekg( STATION_NAME.first, ios::beg );
         iss >> name;

         //
         // STATION_DESCRIP: get the longer station description
         //
         iss.seekg( STATION_DESCRIP.first, ios::beg );
         iss >> description;

         //
         // STATION_LAT: get the station latitude
         //
         iss.seekg( STATION_LAT.first, ios::beg );
         iss >> latitude;

         //
         // STATION_LON: get the station longitude   
         //
         iss.seekg( STATION_LON.first, ios::beg );
         iss >> longitude;

         //
         // Check for data region limits, if necessary
         //
         if ( doClipping ) {
            location.x = PJGrange360( (double)longitude );
            location.y = PJGrange360( (double)latitude );
            if ( !EG_point_in_polygon( location, clippingArea, numVerticies )) {
               //
               // Outside the clipping area
               // Skip to the next station observation
               //
               POSTMSG( DEBUG, "Skipping %4s observation at "
                               "%7.2f (lat) %7.2f (lon)", 
                               name, latitude, longitude );
               continue;
            }
         }

         //
         // STATION_ALT: get the station altitude   
         //
         iss.seekg( STATION_ALT.first, ios::beg );
         iss >> altitude;

         //
         // Create a new stationReport and
         // store it in our list of stations
         //
         stationReport = new StationReport( name, description, id,
                                            latitude, longitude, altitude,
                                            MISSING_VALUE );
         stations[id] = stationReport;
      }

      //
      // TIME_STAMP: see if we've changed surface station times
      //
      iss.seekg( TIME_STAMP.first, ios::beg );
      iss >> aifstime;
      if ( setTimeNow ) {
         timestamp = time( NULL );
      }
      else {
         timestamp = aifstime;
      }
      if ( observations.size() == 0 ) {
         dataTime = timestamp;
      }
      else if ( dataTime != timestamp ) {
         //
         // Set the input file stream's get pointer 
         // back to the begining of this line (pos)
         // so that it is re-read for the next entry into the database
         //
         inputStream.seekg( pos, ios::beg );
         return 0;
      }
      stationReport->setObsTime( dataTime );
         
      //
      // WIND_DIR: get the windDirection value
      //
      iss.seekg( WIND_DIR.first, ios::beg );
      iss >> windDir;
      stationReport->setWindDir( windDir );

      //
      // WIND_SPEED: convert the windSpeed to m/s
      //
      iss.seekg( WIND_SPEED.first, ios::beg );
      iss >> windSpeed;
      stationReport->setWindSpeed( windSpeed/NMH_PER_MS );

      //
      // WIND_GUST: get the windGust value
      //
      iss.seekg( WIND_GUST.first, ios::beg );
      iss >> windGust;
      stationReport->setWindGust( windGust/NMH_PER_MS );

      //
      // TEMPERATURE: get the temperature value
      //
      iss.seekg( TEMPERATURE.first, ios::beg );
      iss >> temperature;
      stationReport->setTemperature( temperature );

      //
      // DEW_POINT: get the dewPoint value
      //
      iss.seekg( DEW_POINT.first, ios::beg );
      iss >> dewPoint;
      stationReport->setDewPoint( dewPoint );

      //
      // REL_HUMIDITY: get the relHumidity value
      //
      iss.seekg( REL_HUMIDITY.first, ios::beg );
      iss >> relHumidity;
      stationReport->setRelHumidity( relHumidity );

      //
      // PRESSURE: get the pressure value
      //
      iss.seekg( PRESSURE.first, ios::beg );
      iss >> pressure;
      stationReport->setPressure( pressure );

      //
      // VISIBILITY: get the visibility value
      // 
      iss.seekg( VISIBILITY.first, ios::beg );
      iss >> visibility;
      stationReport->setVisibility( visibility );

      //
      // Reset the read position of the input buffer,
      // and hang onto the current input file stream position
      //
      pos = inputStream.tellg();

      //
      // Add the current stationReport to our list of observations 
      // to be written to the database
      //
      observations.push_back( stationReport );
   }

   //
   // See if we found any data: we should only get here at eof
   //
   if ( observations.size() == 0 ) {
      POSTMSG( WARNING, "No data points processed "
                        "from the surface observation file." );
   }

   return 0;
}
