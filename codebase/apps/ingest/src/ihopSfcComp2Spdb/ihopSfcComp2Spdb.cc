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
// ihopSfcComp2Spdb.cc
//
// ihopSfcComp2Spdb object
//
// Sue Dettling, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 2004
//
// ihopSfcComp2Spdb reads ASCII files containing ihop surface data and  converts the
// data to station_report_t, and writes to SPDB.
//
///////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/DateTime.hh>

#include "ihopSfcComp2Spdb.hh"
using namespace std;

// Constructor

ihopSfcComp2Spdb::ihopSfcComp2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "ihopSfcComp2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
    return;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

ihopSfcComp2Spdb::~ihopSfcComp2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ihopSfcComp2Spdb::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // file input object
  
  DsInputPath *input = NULL; // Set to NULL to get around compiler warnings.
    
  input = new DsInputPath(_progName,
			  _params.debug >= Params::DEBUG_VERBOSE,
			  _args.inputFileList);
    
  // loop through available files

  char *inputPath;
  while ((inputPath = input->next()) != NULL) {
    
    time_t inputTime;
    input->getDataTime(inputPath, inputTime);
    
    if (_processFile(inputPath, inputTime)) {
      cerr << "WARNING - ihopSfcComp2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
    
  } // while
  
  return 0;

}

////////////////////
// process the file

int ihopSfcComp2Spdb::_processFile(const char *file_path,
			     time_t file_time)
  
{

  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
    cerr << "  File time: " << utimstr(file_time) << endl;
  }

  // registration

  char procmapString[BUFSIZ];
  Path path(file_path);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  // create output objects

  DsSpdb spdb;


  // Open the file

  FILE *fp;
  if((fp = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ihopSfcComp2Spdb::_processFile" << endl;
    cerr << "  Cannot open surface composite data file: "
	 << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
   
   
  char line[BUFSIZ];

  //
  // Get past first 3 lines which specify fields and units
  //
  fgets(line, BUFSIZ, fp);
  fgets(line, BUFSIZ, fp);
  fgets(line, BUFSIZ, fp);

  //
  // ingest data and create station_report_t objects and write to spdb
  // database
  //
  while( fgets(line, BUFSIZ, fp) != NULL ) {
    if(_params.debug)
      {
	cerr <<"-------------------------------------------------\n" << endl;
	cerr << line << endl;
      }
    

    char nominal_date[25];
    
    char nominal_time[25];

    char actual_date[25];
    
    char actual_time[25];
    
    char networkId[25];
    
    char stationId[25];
    
    //
    // latitude (decimal degrees, South is negative)
    //
    float lat;
    
    //
    // longitude (decimal degrees, West is negative)
    //
    float lon;
    
    //
    // station elevation (meters)
    //
    float stationAlt;
    
    //
    // station pressure (mb)
    //
    float stationP;

    //
    // sea level pressure (mb)
    //
    float seaLevelP;
    
    //
    // dry bulb temperature (celcius)
    // 
    float temp;
    
    //
    // dew point (celcius)
    //
    float dewPt;
    
    //
    // wind speed(m/s)
    // 
    float windSpeed;
    
    //
    // wind direction degrees
    //
    float windDir;
    
    //
    // total precip (mm)
    //
    float precip;
    
    //
    // squall/gust indicator (code value)
    //
    char gustIndicator;


    float gustValue;
   
    //
    // Visibility(m)
    //
    float visibility;
    
    //
    // ceiling height (hundreds of feet)
    //
    float ceiling;
    
    //
    // Quality Control Values for Varaibles.
    //
    //      
    // QC Code   Description
    // -------   -----------
    // U         Unchecked
    // G         Good
    // M         Normally recorded but missing.
    // D         Questionable
    // B         Unlikely
    // N         Not available or Not observed
    // X         Glitch                        
    // E         Estimated
    // C         Reported value exceeds output format field size or
    //             was negative precipitation.
    // T         Trace precipitation amount recorded
    // I         Derived parameter can not be computed due to
    //             insufficient data.


    char stationPQC;
    char seaLevelPQC;
    char tempQC;
    char dewPtQC;
    char windSpeedQC;
    char windDirQC;
    char precipQC;
    char visQC;
    char ceilingQC;
    char gustQC;

    sscanf(line,
	   " %s"   // nominal date
	   " %s"   // nominal time
	   " %s"   // date of observation
	   " %s"   // time of observation
           " %s"   // network id
	   " %s"   // station id
	   " %f"   // latitude (degrees)
	   " %f"   // longitude (degrees)
	   " %*f"  // station occurence (unitless)
	   " %f"   // station elevation (meters)
	   " %f"   // station pressure (mb)
	   " %c"   // station pressure QC (code)
	   " %f"   // reported sea level pressure
	   " %c"   // reported sea level pressure QC
	   " %*f"  // computed sea level pressure
	   " %*c"  // computed sea level pressure QC
	   " %f"   // dry bulb temp(celsius)
	   " %c"   // dry bulb temp QC 
	   " %f"   // dew point(celsius)
	   " %c"   // dew point QC
	   " %f"   // wind speed (m/s)
	   " %c"   // wind speed QC
	   " %f"   // wind direction(degrees)
	   " %c"   // wind direction QC
	   " %f"   // total precip(mm)
	   " %c"   // total precip QC
	   " %c"   // squall/gust indicator
	   " %f"   // squall/gust value(m/s)
	   " %c"   // squall/gust QC
	   " %*f"  // present weather
	   " %*c"  // present weather QC
	   " %f"   // visibility(meters)
	   " %c"   // visibility QC
	   " %f"   // ceiling height first layer (hundreds of feet)
	   " %*g"  // ceiling flag first layer
	   " %c"   // ceiling height QC first layer 
	   " %*f"  // cloud amount first layer
	   " %*g"  // cloud amount QC first layer  
	   " %*f"  // ceiling height second layer
	   " %*g"  // ceiling flag second layer
	   " %*g"  // ceiling height QC second layer (hundreds of feet)
	   " %*f"  // cloud amount second layer
	   " %*g"  // cloud amount QC second layer
	   " %*f"  // ceiling height third layer (hundreds of feet)
	   " %*g"  // ceiling flag third layer
	   " %*g"  // ceiling height QC third layer
	   " %*f"  // cloud amount third layer
	   " %*g", // cloud amount QC third layer
	   nominal_date, nominal_time, actual_date, actual_time, networkId, 
           stationId, &lat, &lon, &stationAlt, &stationP, &stationPQC, &seaLevelP, &seaLevelPQC, &temp, 
           &tempQC, &dewPt, &dewPtQC,&windSpeed, &windSpeedQC, &windDir, 
           &windDirQC, &precip, &precipQC, &gustIndicator, &gustValue, 
	   &gustQC, &visibility, &visQC, &ceiling, &ceilingQC);

    if (_params.debug)
      {
	cerr << "nominal_date "  << nominal_date  << "\n"
             << "nominal_time "  << nominal_time  << "\n"
	     << "actual_date "   << actual_date   << "\n"
	     << "actual_time "   << actual_time   << "\n"
	     << "networkId "     << networkId     << "\n"
	     << "stationId "     << stationId     << "\n"
	     << "lat "           << lat           << "\n"
	     << "lon "           << lon           << "\n"
	     << "stationAlt "    << stationAlt    << "\n"
	     << "stationP "      << stationP      << "\n" 
	     << "stationPQC "    << stationPQC    << "\n"
	     << "seaLevelP "     << seaLevelP     << "\n"
	     << "seaLevelPQC "   << seaLevelPQC   << "\n"
	     << "temp "          << temp          << "\n"
	     << "tempQC "        << tempQC        << "\n"
	     << "dewPt "         << dewPt         << "\n"
	     << "dewPtQC "       << dewPtQC       << "\n"
	     << "windSpeed "     << windSpeed     << "\n"
	     << "windSpeedQC "   << windSpeedQC   << "\n"
	     << "windDir "       << windDir       << "\n"
	     << "windDirQC "     << windDirQC     << "\n"
	     << "precip "        << precip        << "\n"
	     << "precipQC "      << precipQC      << "\n"
	     << "gustIndicator " << gustIndicator << "\n"
	     << "gustValue "     << gustValue     << "\n"
             << "gustQC "        << gustQC        << "\n"
	     << "visibility "    << visibility    << "\n"
	     << "visQC "         << visQC         << "\n"
	     << "ceiling "       << ceiling       << "\n"
	     << "ceilingQC "     << ceilingQC     << "\n";
	cerr <<"-------------------------------------------------\n" << endl;
      }

    //
    // get report time
    //
    char date_time_str[50];

    if (_params.time == Params::NOMINAL_TIME)
      sprintf(date_time_str, "%s %s", nominal_date, nominal_time);
    else
      sprintf(date_time_str, "%s %s", actual_date, actual_time);

    DateTime *dateTime = new DateTime( date_time_str);

    time_t rep_time = dateTime->utime();
    
    delete dateTime;

    //
    // set station_report_t. If QC value is 'B' or unlikely or 
    // QC value is 'M' or missing, then set field to STATION_NAN.
    //
    station_report_t station_rep;

    station_rep.msg_id = PRESSURE_STATION_REPORT;

    station_rep.time = rep_time;

    station_rep.accum_start_time = rep_time - 3600;

    station_rep.weather_type = 0;

    station_rep.lat = lat;

    station_rep.lon = lon;

    station_rep.alt = stationAlt;

    if (tempQC == 'B' || tempQC == 'M')
      station_rep.temp = STATION_NAN;
    else
      station_rep.temp = temp;

    if (dewPtQC == 'B'|| dewPtQC == 'M')
      station_rep.dew_point = STATION_NAN;
    else
      station_rep.dew_point = dewPt;
    
    station_rep.relhum = STATION_NAN;

    if (windSpeedQC == 'B'|| windSpeedQC == 'M')
      station_rep.windspd  = STATION_NAN;
    station_rep.windspd = windSpeed;

    if (windDirQC == 'B'|| windDirQC == 'M')
      station_rep.winddir = STATION_NAN;
    else
      station_rep.winddir = windDir;
    
    if (gustQC == 'B'|| gustQC == 'M')
      station_rep.windgust = STATION_NAN;
    else
      station_rep.windgust = gustValue;

    if (stationPQC == 'B'|| stationPQC == 'M')
      station_rep.shared.pressure_station.stn_pres = STATION_NAN;
    else
      station_rep.shared.pressure_station.stn_pres = stationP;
    
    if (seaLevelPQC == 'B'|| seaLevelPQC == 'M')
      station_rep.pres = STATION_NAN;
    else
      station_rep.pres = seaLevelP;

    if (precipQC == 'B'|| precipQC == 'M')
      station_rep.liquid_accum = precip; 
    else
      station_rep.precip_rate = STATION_NAN;

    //
    // Vis in km (convert from meters)
    //
    if (visQC == 'B' || visQC == 'M')
      station_rep.visibility = STATION_NAN;
    else
      station_rep.visibility = visibility/1000;

    station_rep.rvr = STATION_NAN;

    //
    // ceiling in km (convert from hundreds of feet to km)
    //
    if (ceilingQC == 'B' || ceilingQC == 'M')
      station_rep.ceiling = STATION_NAN;
    else
      station_rep.ceiling = ceiling / 32.808399;
    
    snprintf(station_rep.station_label, 8, "%s", stationId);

    //
    // Swap the data in a station report into big-endian
    // format.  
    //
    station_report_to_be(&station_rep);

    //
    // Set valid and expire times for data
    //
    time_t valid_time = rep_time;
    time_t expire_time = valid_time + _params.expire_seconds; 
    
    //
    // Write output
    //
    spdb.setPutMode(Spdb::putModeAddUnique);

    if (spdb.put(_params.output_url,
		 SPDB_STATION_REPORT_ID,
		 SPDB_STATION_REPORT_LABEL,
		 Spdb::hash4CharsToInt32(station_rep.station_label),
		 valid_time,
		 expire_time,
		 sizeof(station_report_t),
		 &station_rep)) 
      {
	cerr << "ERROR - ihopSfcComp2Spdb::_processFile" << endl;
	cerr << "  Cannot put output to: " << _params.output_url << endl;
	cerr << "  " << spdb.getErrStr() << endl;
	return 1;
      }
    
  } // while (fgets ...
  
  fclose(fp);
  
  return 0;
   
}






