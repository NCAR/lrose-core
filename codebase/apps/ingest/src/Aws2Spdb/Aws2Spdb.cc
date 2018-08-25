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
////////////////////////////////////////////////////////////////////////
// Aws2Spdb.cc
//
// Aws2Spdb object
//
// Sue Dettling, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2004
//
///////////////////////////////////////////////////////////////
//
// Aws2Spdb reads ASCII files containing METAR data, converts the
// data to station_report_t, and writes to SPDB.
//
///////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <rapformats/station_reports.h>
#include "Aws2Spdb.hh"
using namespace std;

//
// Constructor
//
Aws2Spdb::Aws2Spdb(int argc, char **argv)

{

  isOK = true;

  //
  // set programe name
  //
  _progName = "Aws2Spdb";
  ucopyright((char *) _progName.c_str());

  //
  // get command line args
  //
  if (_args.parse(argc, argv, _progName)) 
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with command line args" << endl;
      isOK = FALSE;
      return;
    }

  //
  // get TDRP params
  //
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) 
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters" << endl;
      isOK = FALSE;
      return;
    }

  //
  // load up the station locations
  //
  if (_loadLocations(_params.st_location_path)) 
    {
      isOK = false;
      return;
    }

  //
  // init process mapper registration
  //
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

Aws2Spdb::~Aws2Spdb()

{

  //
  // unregister process
  //
  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// 
// Run
//
int Aws2Spdb::Run ()
{

  //
  // register with procmap
  //
  PMU_auto_register("Run");

  //
  // file input object
  //
  DsInputPath *input = NULL; 
  
  if (_params.mode == Params::FILELIST) 
    {
      //
      // FILELIST mode
      //
      input = new DsInputPath(_progName,
			      _params.debug >= Params::DEBUG_VERBOSE,
			      _args.inputFileList);
      
    } 
  else if (_params.mode == Params::ARCHIVE) 
    {
      //
      // ARCHIVE mode - start time to end time
      //
      input = new DsInputPath(_progName,
			      _params.debug >= Params::DEBUG_VERBOSE,
			      _params.input_dir,
			      _args.startTime, _args.endTime);
      
    } 
  else if (_params.mode == Params::REALTIME) 
    {
      //
      // realtime mode
      //
      input = new DsInputPath(_progName,
			      _params.debug >= Params::DEBUG_VERBOSE,
			      _params.input_dir,
			      _params.max_realtime_valid_age,
			      PMU_auto_register,
			      _params.latest_data_info_avail,
			      false);
      
    }

  //
  // loop through available files
  //
  char *inputPath;
  while ((inputPath = input->next()) != NULL) 
    {
    
      time_t inputTime;
      input->getDataTime(inputPath, inputTime);
      
      if (_processFile(inputPath, inputTime)) 
	{
	  cerr << "WARNING - Aws2Spdb::Run" << endl;
	  cerr << "  Errors in processing file: " << inputPath << endl;
	}
      
    } // while
  
  return 0;

}

////////////////////
//
//  process the file
//
int Aws2Spdb::_processFile(const char *file_path,
			     time_t file_time)
  
{

  if (_params.debug) 
    {
      cerr << "\nProcessing file: " << file_path << endl;
      cerr << "  File time: " << utimstr(file_time) << "\n" << endl;
    }

  //
  // registration with procmap
  //
  char procmapString[BUFSIZ];
  
  Path path(file_path);
  
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  
  PMU_force_register(procmapString);

  //
  // create output object
  //
  DsSpdb spdb;

  //
  // Open the file
  //
  FILE *fp;
  if((fp = fopen(file_path, "r")) == NULL) 
    {
      int errNum = errno;
      cerr << "ERROR - Aws2Spdb::_processFile" << endl;
      cerr << "  Cannot open AWS file: "
	   << file_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      return 1;
    }
   
  
  //
  // Read file
  // 
  char line[BUFSIZ];
  
  while( fgets(line, BUFSIZ, fp) != NULL ) 
    {
      
      //
      // Report variables
      //
      int year;
      
      int month;
      
      int day;
      
      int hour;
      
      int minute;
      
      int stationID;
      
      //
      // Mean wind speed over past two minutes (m/s)
      // 
      float mWsp2;
      
      //
      // Mean wind dir over past two minutes (degrees)
      // 
      float mWdir2;
      
      //
      // Mean wind speed over past ten minutes (m/s)
      // 
      float mWsp10;
      
      //
      // Mean wind dir over past two minutes (degrees)
      // 
      float mWdir10;
      
      //
      // Extreme wind speed over past hour (m/s)
      // 
      float eWsp;
      
      //
      // Direction of extreme wind (degrees)
      // 
      float eWdir;
      
      //
      // Time of extreme wind speed over past hour (m/s)
      // 
      int tEwsp;
      
      //
      // Max wind speed over past hour (m/s)
      // 
      float maxWsp;
      
      //
      // Direction of extreme wind (degrees)
      // 
      float maxWdir;
      
      //
      // Time of max wind speed (unix time)
      // 
      int tMaxWsp;
      
      //
      // Precip in 1 hour (mm)
      //
      float precipHour;
      
      //
      // Precip in 1 day. (mm)
      //
      float precipDay;
      
      //
      // Precip in 10 mins (mm)
      //
      float precip10;
      
      //
      // Start time for 10 minute precip accumulation
      // (unix time)
      //
      int tPrecip10;
      
      //
      // mean Tempover one hour period (degrees C)
      //
      float mTempHour; 
      
      //
      // Max temp over one hour period(degrees C)
      //
      float maxTemp;
      
      //
      // Time at which maximum temp occured (unix time)
      //
      int tMaxTemp;
      
      //
      // Min temp over one hour period (degrees C)
      //
      float minTemp;
      
      //
      // Time at which minimum occured(unix time)
      //
      int tMinTemp;
      
      
      //
      // rh and pressure are initialized since they might not
      // be present in the surface report. (percent)
      //
      float rh = STATION_NAN; 
      
      float pressure = STATION_NAN;
      
      
      //
      // Read the data.
      //
      sscanf(line,
	     " %d" // year
	     " %d" // month
	     " %d" // day
	     " %d" // hour 
	     " %d" // minute
	     " %d" // surface station id
	     " %f" // mean wind speed ( 10 * m/s) of past two minutes
	     " %f" // mean wind dir of past two minutes( 0 is north and clockwise is increasing)
	     " %f" // mean wind speed ( 10 * m/s) of past ten minutes
	     " %f" // mean wind dir of past ten minutes( 0 is north and clockwise is increasing)
	     " %f" // extreme wind speed ( 10 * m/s) during past hour
	     " %f" // direction of extreme wind ( 0 is north and clockwise is increasing)
	     " %d" // time of extreme wind meausrement
	     " %f" // max wind speed ( 10 * m/s) during past hour
	     " %f" // direction of max wind( 0 is north and clockwise is increasing)
	     " %d" // time of max wind meausrement
	     " %f" // precip over hour (10 * mm)
	     " %f" // precip over day (10 * mm)
	     " %f" // precip over 10 mins (10 * mm)
	     " %d" // starting time for 10 minute precip measurement
	     " %f" // mean temp in one hour (10 * C)
	     " %f" // max temp over one hour
	     " %d" // time of max temp measurement
	     " %f" // min temp over one hour
	     " %d" // time of min temp measurement
	     " %f" // relative humidity (not necessarily present in AWS record)
	     " %f",// pressure (not necessarily present in AWS record
	     &year, &month, &day, &hour, &minute, &stationID, &mWsp2, &mWdir2, &mWsp10, &mWdir10,
	     &eWsp, &eWdir, &tEwsp, &maxWsp, &maxWdir, &tMaxWsp, &precipHour, &precipDay, &precip10,
	     &tPrecip10, &mTempHour, &maxTemp, &tMaxTemp, &minTemp, &tMinTemp, &rh, &pressure);
    //
    // Convert raw units
    //
    
    //
    // m/s for speed measurements
    //
    mWsp2 = mWsp2/10;
    
    mWsp10 = mWsp10/10;
    
    eWsp = eWsp/10;

    maxWsp = maxWsp/10;
    
    //
    // mm for precip measurements
    //
    precipHour = precipHour/10;

    precipDay = precipDay/10;

    precip10 = precip10/10;

    //
    // degrees C for temp measurements
    //
    mTempHour = mTempHour/10;

    maxTemp = maxTemp/10;

    minTemp = minTemp/10;

    //
    // mb for pressure.
    //
    if (pressure != STATION_NAN)
      pressure = pressure/10;
    
    //
    // Check to see if AWS station is in the list
    //
    map< int, StationLoc, less<string> >::iterator iloc;
    iloc = _locations.find(stationID);
    if (iloc == _locations.end()) 
      {
	if (_params.debug >= Params::DEBUG_VERBOSE)
	  {
	    cerr << "WARNING - Aws2Spdb::processFile" << endl;
	    
	    cerr << " Station " << stationID << " not found in list\n" << endl;
	  
	    cerr << "-----------------------------------------------------" << endl;
	   } 
	continue;
	  
      }

    //
    // Print report for debugging
    //
    if ( _params.debug >= Params::DEBUG_VERBOSE)
      {
	cerr << "year: "       << year       << " "
	     << "month: "      << month      << " "
	     << "day: "        << day        << " "
	     << "hour: "       << hour       << " "
	     << "minute: "     << minute     << " "
	     << "stationId: "  << stationID  << " "
	     << "mWsp2: "      << mWsp2      << " "
	     << "mWdir2: "     << mWdir2     << " "
	     << "mWsp10: "     << mWsp10     << " "
	     << "mWdir10: "    << mWdir10    << " "
	     << "eWsp: "       << eWsp       << " "
	     << "eWdir: "      << eWdir      << " "
	     << "tEwsp: "      << tEwsp      << " "
	     << "maxWsp: "     << maxWsp     << " "
	     << "maxWdir: "    << maxWdir    << " "
	     << "tMaxWsp: "    << tMaxWsp    << " "
	     << "precipHour: " << precipHour << " "
	     << "precipDay: "  << precipDay  << " "
	     << "precip10: "   << precip10   << " "
	     << "tPrecip10: "  << tPrecip10  << " "
	     << "mTempHour: "  << mTempHour  << " "
	     << "maxTemp: "    << maxTemp    << " "
	     << "tMaxTemp: "   << tMaxTemp   << " "
	     << "minTemp: "    << minTemp    << " "
	     << "tMinTemp: "   << tMinTemp   << " "
	     << "rh: "         << rh         << " "
	     << "pressure: "   << pressure   << "\n";
	
      }

    //
    // Create unix time of report
    //
    DateTime dateTime(year, month, day, hour, minute);
    
    time_t repTime = dateTime.utime();

    station_report_t report;
    
    report.msg_id = stationID;
    
    report.time = repTime;

    report.accum_start_time = 0;

    report.weather_type = 0;

    report.lat = (*iloc).second.lat;

    report.lon  = (*iloc).second.lon;

    report.alt = (*iloc).second.alt;
    
    report.temp = mTempHour;

    report.relhum = rh;

    report.windspd = mWsp2;
    
    report.winddir = mWdir2;
    
    report.windgust =  maxWsp;

    report.pres = pressure;
    
    report.liquid_accum = precipHour;

    report.liquid_accum = precipHour;

    report.visibility = STATION_NAN;

    report.rvr = STATION_NAN;
 
    report.ceiling = STATION_NAN;

    //
    // Convert report to big endian;
    //
    station_report_to_be(&report);

    //
    // Write report to database
    //
    spdb.setPutMode(Spdb::putModeAdd);

    if (_params.debug >= Params::DEBUG_VERBOSE)
      {
	cerr << "\nWriting report to " << _params. output_url << endl;
	cerr << "-----------------------------------------------------------\n\n";

      }

    if (spdb.put(_params. output_url,
		 SPDB_STATION_REPORT_ID,
		 SPDB_STATION_REPORT_LABEL,
		 report.msg_id,
		 repTime,
		 repTime + _params.expire_seconds,
		 sizeof(station_report_t),
		 &report) != 0)
   
      {
	   cerr << "ERROR - Aws2Spdb::_processFile" << endl;
	   cerr << "  Cannot put aws report to: "
		<< _params.output_url << endl;
	   cerr << "  " << spdb.getErrStr() << endl;
	   return(1);
      }

    //
    // Swapping back from big-endian
    //
    station_report_from_be(&report);

  } // while (fgets ...
  
  fclose(fp);       

  return 0;
   
}



////////////////////////////////
// load up the station locations

int Aws2Spdb::_loadLocations(const char* station_location_path)
  
{

  FILE *fp;
  char line[BUFSIZ];
  int stationId;
  double lat, lon, alt;
  
  if((fp = fopen(station_location_path, "r")) == NULL) 
    {
      int errNum = errno;
      cerr << "ERROR - Aws2Spdb::loadLocations" << endl;
      cerr << "  Cannot open station location file: "
	   << station_location_path << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
   
  int count = 0;

  while( fgets(line, BUFSIZ, fp) != NULL ) 
    {

      // If the line is not a comment or blank, process it
      
      if( line[0] == '#' || line[0] == '\n') 
	{
	  continue;
	}
      
      // Read in the line
      
      if( sscanf(line, "%d %lg %lg %lg", 
		 &stationId, &lat, &lon, &alt) != 4 ) 
	{
	  if (_params.debug  >= Params::DEBUG_VERBOSE) 
	    {
	      cerr << "WARNING - Aws2Spdb::loadLocations" << endl;
	      cerr << "  Cannot read line from station location file: "
		   << station_location_path << endl;
	      cerr << "  Line: " << line << endl;
	    }
	  
	  continue;
	  
	}
    
      count++;
      
      // Create new AWS location and add it to the map

      pair<int, StationLoc> pr;
      pr.first = stationId;
      pr.second.set(stationId, lat, lon, alt);
      _locations.insert(_locations.begin(), pr);
      if (_params.debug >= Params::DEBUG_VERBOSE) 
	{
	  cerr << "Aws2Spdb::loadLocations" << endl;
	  cerr << "  Inserting " << pr.first << " into station locations list" << endl;
	}
      
    }

  fclose(fp);

  if (count == 0) 
    {
      cerr << "ERROR - Aws2Spdb::loadLocations" << endl;
      cerr << "  No suitable locations in file: : "
	   << station_location_path << endl;
      return -1;
    }
  
  return 0;
  
}
















