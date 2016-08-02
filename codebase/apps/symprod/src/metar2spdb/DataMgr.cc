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
/////////////////////////////////////////////////

#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <toolsa/MsgLog.hh>  
#include <toolsa/Path.hh>  
#include <symprod/spdb_products.h>
#include <sys/stat.h>

#include "Ingester.hh"
#include "Metar.hh"
#include "SpdbBuffer.hh"
#include "Metar2Spdb.hh"
#include "DataMgr.hh"
using namespace std;

DataMgr::DataMgr(const Params &params) :
  _params(params)

{
   latestFileTime     = 0;
   currentFileTime    = 0;
   nDestinations      = 0;
   ingester           = NULL;
   decodedSpdbBuf     = NULL;
   rawSpdbBuf         = NULL;
   inputDir           = NULL;
   tstamp             = NULL;

   currentTime.year   = 1970;
   currentTime.month  = 1;
   currentTime.day    = 1;
   currentTime.hour   = 0;
   currentTime.min    = 0;
   currentTime.sec    = 0;

}

DataMgr::~DataMgr()
{
   clearMetars();

   if( inputDir )
      STRfree( inputDir );
   if( ingester )
      delete ingester;
   if( decodedSpdbBuf )
      delete decodedSpdbBuf;
   if( rawSpdbBuf )
      delete rawSpdbBuf;
   if ( tstamp )
     delete tstamp;
}

int 
DataMgr::init()
{
   //
   // Set members that are based on the parameters
   //
   inputDir = STRdup( _params.input_dir ); 
   if (_params.mode == Params::ARCHIVE) {
     maxRealtimeValidAge = -1;
   } else {
     maxRealtimeValidAge = _params.max_realtime_valid_age;
   }

   //
   // Instantiate class members
   //
   ingester  = new Ingester( inputDir, _params );

   //
   // Initialize the ingester by setting up the metar locations
   //
   if( ingester->setLocations( _params.st_location_path ) ) {
      return ( FAILURE );
   }

   // instantiate SPDB destinations

   if (_params.use_URLs) {

     decodedSpdbBuf = new SpdbBuffer( SPDB_STATION_REPORT_ID,
				      SPDB_STATION_REPORT_LABEL,
				      _params.expire_seconds,
				      true );
     decodedSpdbBuf->addDest(_params.decoded_output_url);

     if (_params.store_raw_metars) {
       rawSpdbBuf = new SpdbBuffer( SPDB_ASCII_ID,
				    SPDB_ASCII_LABEL,
				    _params.expire_seconds,
				    true );
       rawSpdbBuf->addDest(_params.raw_output_url);
     }
     
   } else {

     decodedSpdbBuf = new SpdbBuffer( SPDB_STATION_REPORT_ID,
				      SPDB_STATION_REPORT_LABEL,
				      _params.expire_seconds,
				      false );
     for (int  i = 0; i < _params.destinations_n; i++) {
       decodedSpdbBuf->addDest(_params._destinations[i]);
     }
     
     if (_params.store_raw_metars) {
       rawSpdbBuf = new SpdbBuffer( SPDB_ASCII_ID,
				    SPDB_ASCII_LABEL,
				    _params.expire_seconds,
				    false );
       for (int  i = 0; i < _params.raw_destinations_n; i++) {
	 rawSpdbBuf->addDest(_params._raw_destinations[i]);
       }
     }

   }

   // 
   // Set flag, current time and latest time if we are going to use the 
   // index file
   //
   if( _params.mode == Params::REALTIME ) {

      char time_stamp_file_name[1024];
      sprintf(time_stamp_file_name, "metar2psdb.tstamp.%s", _params.instance);
      tstamp = new LdataInfo(_params.time_stamp_dir,
			     false, time_stamp_file_name);
       
      setCurrentTime(1970, 1, 1, 0, 0);

      if( !_params.ignore_index_at_start ) {

	 if( tstamp->read(-1) ) {
            //
            // no index file yet
            //
	    POSTMSG( DEBUG, "No time stamp file yet");
	    
         } else {
            //
            // Get the name of the latest file
            //
	    const date_time_t &ltime = tstamp->getLatestTimeStruct();
            setCurrentTime(ltime.year, ltime.month, 
                           ltime.day, ltime.hour,
                           ltime.min );
	
            POSTMSG( DEBUG, "Latest time stamp: %ld/%ld/%ld %.2ld:%.2ld",
                     currentTime.month,
                     currentTime.day,
                     currentTime.year,
                     currentTime.hour,
                     currentTime.min );
         }
      }

      updateLatestTime();
      
   }

   //
   // Make local copies of the specifications for
   // storing raw metar data in an SPDB.
   //
   store_raw_metars=_params.store_raw_metars;

   return( SUCCESS );
}

int 
DataMgr::processFiles()
{

  DIR *dirp;
  if ((dirp = opendir(inputDir)) == NULL) {
    POSTMSG( ERROR, "Cannot read directory '%s'", inputDir );
    return -1;
  }

  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    char *fileName = dp->d_name;

    // exclude dir entries and files beginning with '.'
    
    if (fileName[0] == '.')
      continue;
    
    // throw out files which are too old
    
    struct stat fileStat;
    string filePath = inputDir;
    filePath += PATH_DELIM;
    filePath += fileName;
    if (stat(filePath.c_str(), &fileStat)) {
      perror(filePath.c_str());
      continue;
    }
    
    if (maxRealtimeValidAge > 0) {
      if ((time(NULL) - fileStat.st_mtime) > maxRealtimeValidAge)
	continue;
    }
    
    //
    // Throw out files which precede the index file,
    // if necessary
    //
    if ( tstamp  &&  
	 strncmp( fileName, latestFileName, strlen(fileName) ) <= 0 )
      continue;
    
    if (processFile(fileName)) {
      continue;
    }

  } // fileIndex

   return(SUCCESS);

}

int
DataMgr::processFile(char *fileName, time_t fileTime /* = -1*/,
		     int ExtendFilename /* = 1*/ )

{ 

  char procmapString[BUFSIZ];
  Path path(fileName);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  //
  // Clear out list of metars
  //
  clearMetars();

  //
  // Process the metars in the file add them to the
  // list of metars
  // 
  POSTMSG( DEBUG, "File %s will be processed", fileName );
  
  // set the ingester time
  if (fileTime == -1) {
    if( ingester->setTime(fileName) != SUCCESS ) {
      POSTMSG( WARNING, "file %s not processed", fileName );
      return (FAILURE);
    }
  } else {
    ingester->setTime(fileTime);
  }

  // write the time stamp file now - so that if the ingest
  // fails and the program crashes the file will not be used again
  
  if( tstamp ) {
    setCurrentTime( ingester->getYear(), ingester->getMonth(),
		    ingester->getDay(), ingester->getHour(),
		    ingester->getMin() );
    writeIndex();
  }
  
  // process the file
  
  if( ingester->processFile(fileName,
			    metars,
			    ExtendFilename) != SUCCESS ) {
    POSTMSG( WARNING, "file %s not processed", fileName );
    return (FAILURE);
  }
  
  if( metars.size() < 1) {
    POSTMSG( DEBUG, "no metars in file %s", fileName);
    return (SUCCESS);
  }
  
  //
  // Package up metars and send them to spdb
  //
  PMU_auto_register("Writing decoded metars");
  decodedSpdbBuf->reset();
  vector< Metar* >::iterator metarItem;
  for( metarItem = metars.begin();
       metarItem != metars.end(); metarItem++ ) {
    decodedSpdbBuf->addDecodedMetar(**metarItem);
  }

  PMU_force_register("Start write decoded metars");
  decodedSpdbBuf->send();
  PMU_auto_register("End write decoded metars");

  if (store_raw_metars) {
    PMU_auto_register("Start write raw metars");
    rawSpdbBuf->reset();
    vector< Metar* >::iterator metarItem;
    for( metarItem = metars.begin();
	 metarItem != metars.end(); metarItem++ ) {
      rawSpdbBuf->addRawMetar(**metarItem);
    }
    rawSpdbBuf->send();
    PMU_auto_register("End write raw metars");
  }
  
  //
  // Clear out list of metars
  //
  clearMetars();

  return (SUCCESS);

}

void
DataMgr::clearMetars() 
{
   vector< Metar* >::iterator item;
   
   for(item = metars.begin(); item != metars.end(); item++) {
      delete( (*item) );
   }
   
   metars.erase(metars.begin(), metars.end());
}

int
DataMgr::writeIndex()
{
   
   if( currentFileTime > latestFileTime ) {
      updateLatestTime();

      if ( tstamp->write( latestFileTime ) ) {
         return ( FAILURE );
      }
      return( SUCCESS );
   } 

   return( SUCCESS );
   
}

void
DataMgr::setCurrentTime(int year, int month, int day, int hour, int min) 
{
   currentTime.year  = year;
   currentTime.month = month;
   currentTime.day   = day;
   currentTime.hour  = hour;
   currentTime.min   = min;
   currentTime.sec   = 0;

   currentFileTime = UTIMdate_to_unix(&currentTime);
  
}

void
DataMgr::updateLatestTime()
{
    sprintf( latestFileName, "%s.%.4ld%.2ld%.2ld%.2ld%.2ld",
	    FILE_PRFX, 
            currentTime.year, 
            currentTime.month, 
            currentTime.day, 
            currentTime.hour, 
            currentTime.min);
    
    latestFileTime = currentFileTime;
}









