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
// CwbObs2Spdb.cc
//
// CwbObs2Spdb object
//
// Dan Megenahrdt, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 2010
//
///////////////////////////////////////////////////////////////
//
// CwbObs2Spdb reads automated weather station surface
// observations, converts them to station_report_t format
// and writes them to an SPDB data base
//
///////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <rapmath/math_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <didss/DsInputPath.hh>
#include <physics/physics.h>
#include "CwbObs2Spdb.hh"
using namespace std;

// Constructor

CwbObs2Spdb::CwbObs2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "CwbObs2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char*)"unknown";
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

CwbObs2Spdb::~CwbObs2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int CwbObs2Spdb::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // file input object
  
  DsInputPath *input = NULL; // Set to NULL to get around compiler warnings.
  
  if (_params.mode == Params::FILELIST) {
    
    // FILELIST mode
    
    input = new DsInputPath(_progName,
			    _params.debug >= Params::DEBUG_VERBOSE,
			    _args.inputFileList);
    
  } else if (_params.mode == Params::ARCHIVE) {
    
    // archive mode - start time to end time
    
    input = new DsInputPath(_progName,
			    _params.debug >= Params::DEBUG_VERBOSE,
			    _params.input_dir,
			    _args.startTime, _args.endTime);
    
  } else if (_params.mode == Params::REALTIME) {
    
    // realtime mode - no latest_data_info file
    
    input = new DsInputPath(_progName,
			    _params.debug >= Params::DEBUG_VERBOSE,
			    _params.input_dir,
			    _params.max_realtime_valid_age,
			    PMU_auto_register,
			    _params.latest_data_info_avail,
			    true);
    
    if (_params.strict_subdir_check) {
      input->setStrictDirScan(true);
    }
    
    if (_params.file_name_check) {
      input->setSubString(_params.file_match_string);
    }
    
  }
  
  // loop through available files
  
  char *inputPath;
  while ((inputPath = input->next()) != NULL) {

    if (_params.station_type == Params::SFC_STATION) {
      if (_processSfcFile(inputPath)) {
	cerr << "WARNING - CwbObs2Spdb::Run" << endl;
	cerr << "  Errors in processing mdf file: " << inputPath << endl;
      }
    } else if (_params.station_type == Params::GAUGE_STATION) {
      if (_processGaugeFile(inputPath)) {
	cerr << "WARNING - CwbObs2Spdb::Run" << endl;
	cerr << "  Errors in processing precip file: " << inputPath << endl;
      }
    } else {
      cerr << "WARNING - CwbObs2Spdb::Run" << endl;
      cerr << " Errors : Unknown station_type" << _params.station_type << endl;
    }
    
  } // while
    
  return 0;

}

////////////////////
// process station file

int CwbObs2Spdb::_processSfcFile(const char *file_path)
  
{

  int iret = 0;

  if (_params.debug) {
    cerr << "Processing station file: " << file_path << endl;
  }

  // registration

  char procmapString[BUFSIZ];
  Path path(file_path);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  // compute the time from the file name

  Path ppath(file_path);
  string fileName = ppath.getFile();

/*
  int year, month, day, hour, min;
  if (sscanf(fileName.c_str(), "%4d%2d%2d%2d%2d",
	     &year, &month, &day, &hour, &min) != 5) {
    cerr << "ERROR - CwbObs2Spdb::_processSfcFile" << endl;
    cerr << "  Cannot compute time from file name" << endl;
    cerr << "  File path: " << file_path << endl;
    cerr << "  File name: " << fileName << endl;
    return -1;
  }
  DateTime validTime(year, month, day, hour, min, 0);

  // create spdb output object
  
  DsSpdb out;
*/
  // read in file, loading up chunks in output object spdb

  FILE *in;
  if ((in = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - CwbObs2Spdb::_processSfcFile" << endl;
    cerr << "  Cannot open file: " << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  int nObs = 0;
  int nline = 0;

  //  header reading
  char hdline[100];
  int year, month, day, hour, min;
  char tmpstr[2];

  feof(in);
  fgets(hdline,100,in);
  //   cerr << hdline << endl;
  fgets(hdline,100,in);
  //   cerr << hdline << endl;
  if (sscanf(hdline, "%s%d%d%d%d%d",
	 tmpstr, &year, &month, &day, &hour, &min) != 6) {
      cerr << "  Cannot compute time from header file" << endl;
  //    cerr <<  tmpstr << year << month << day << hour <<  min << endl;
  }
  DateTime validTime(year, month, day, hour, min, 0);

  // create spdb output object
  DsSpdb out;

  //  data reading
  while (!feof(in)) {

    char line[1024];

    if (fgets(line, 1024, in) == NULL) {
      break;
    }

    nline++;

    char stid[128];
    char stnm[128];
    int TIME;
    double lat, lon, elev, wdir, wspd, temp, humd, pres;

    if (sscanf(line, "%s%s%d%lg%lg%lg%lg%lg%lg%lg%lg",
	       stid, stnm, &TIME,
	       &lat, &lon, &elev, &wdir, &wspd, &temp, &humd, &pres) != 11) {
      if (strstr(line, "STID") == NULL) {
	// not first line, print error message
	cerr << "WARNING - CwbObs2Spdb::_processSfcFile" << endl;
	cerr << "  Cannot read line: " << nline << line;
      }
      continue;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read line: " << line;
      cerr << " stid: " << stid << endl;
      cerr << " lat: " << lat << endl;
      cerr << " lon: " << lon << endl;
      cerr << " elev: " << elev << endl;
      cerr << " wdir: " << wdir << endl;
      cerr << " wspd: " << wspd << endl;
      cerr << " temp: " << temp << endl;
      cerr << " humd: " << humd << endl;
      cerr << " pres: " << pres << endl;
    }
    
    // fill out station report
    
    station_report_t report;
    memset(&report, 0, sizeof(report));
    
    STRncopy(report.station_label, stid, ST_LABEL_SIZE);
    
    report.msg_id = SENSOR_REPORT;
    report.time = validTime.utime();
    
    report.lat = lat;
    report.lon = lon;
    report.alt = elev;

    if( temp <= _params.bad_data_value)
    {
      report.temp = STATION_NAN;
    }
    else
    {
      report.temp = temp;
    }
    
    if(humd <= _params.bad_data_value)
    {
      report.relhum = STATION_NAN;
    }
    else
    {
      report.relhum = humd * 100;
    }
    

    if (report.temp == STATION_NAN || report.relhum == STATION_NAN)
    {
      report.dew_point = STATION_NAN;
    }
    else
    {
      double dew_point = PHYrhdp(temp, humd * 100.0);
      dew_point = ((int) floor (dew_point * 10.0 + 0.5)) / 10.0;
      report.dew_point = dew_point;
    }
    
    if(wspd <= _params.bad_data_value)
    {
      report.windspd = STATION_NAN;
    }
    else
    {
      report.windspd = wspd;
    }

    if(wdir <= _params.bad_data_value)
    {
      report.winddir = STATION_NAN;
    }
    else
    {
      report.winddir = wdir;
    }

    if(pres <= _params.bad_data_value)
    {
      report.pres = STATION_NAN;
    }
    else
    {
      report.pres = pres;
    }

    report.windgust = STATION_NAN;
    report.liquid_accum = STATION_NAN;
    report.precip_rate = STATION_NAN;
    report.visibility = STATION_NAN;
    report.ceiling = STATION_NAN;
    report.rvr = STATION_NAN;

    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "====== FOUND REPORT ======" << endl;
      print_station_report(stderr, "", &report);
      cerr << "==========================" << endl;
    }
    
    // swap bytes
    
    station_report_to_be(&report);
    
    // add chunk
    
    int stationId = Spdb::hash4CharsToInt32(stid + 1);
    out.addPutChunk(stationId,
		    validTime.utime(),
		    validTime.utime() + _params.expire_seconds,
		    sizeof(report), &report);
    
    nObs++;
    
  } // while

  if (nObs < 0) {
    cerr << "ERROR - CwbObs2Spdb::_processSfcFile" << endl;
    cerr << "  No valid obs in file: " << file_path << endl;
    return -1;
  }

  // put the data
  
  if (out.put(_params.output_url,
	      SPDB_STATION_REPORT_ID,
	      SPDB_STATION_REPORT_LABEL)) {
    cerr << "ERROR - CwbObs2Spdb::_processSfcFile" << endl;
    cerr << "  Cannot put station data to: "
	 << _params.output_url << endl;
    cerr << "  " << out.getErrStr() << endl;
    iret = -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote " << nObs << " reports to url: "
	 << _params.output_url << endl;
  }

  if (_params.debug) {
    cerr << "  Done with file: " << file_path << endl;
  }

  return iret;
   
}

///////////////////////
// process Gauge file

int CwbObs2Spdb::_processGaugeFile(const char *file_path)
  
{

  int iret = 0;

  if (_params.debug) {
    cerr << "Processing gauge file: " << file_path << endl;
  }

  // registration

  char procmapString[BUFSIZ];
  Path path(file_path);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);

  // compute the time from the file name

  Path ppath(file_path);
  string fileName = ppath.getFile();
/*
  int year, month, day, hour, min;
  if (sscanf(fileName.c_str(), "%4d%2d%2d%2d%2d",
	     &year, &month, &day, &hour, &min) != 5) {
    cerr << "ERROR - CwbObs2Spdb::_processGaugeFile" << endl;
    cerr << "  Cannot compute time from file name" << endl;
    cerr << "  File path: " << file_path << endl;
    cerr << "  File name: " << fileName << endl;
    return -1;
  }
  DateTime validTime(year, month, day, hour, min, 0);
  
  // create spdb output object
  
  DsSpdb out;
*/
  // read in file, loading up chunks in output object spdb
  
  FILE *in;
  if ((in = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - CwbObs2Spdb::_processGaugeFile" << endl;
    cerr << "  Cannot open file: " << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  int nObs = 0;
  int nline = 0;

  //  header reading
  char hdline[100];
  int year, month, day, hour, min;
  char tmpstr[2];

  feof(in);
  fgets(hdline,100,in);
  //   cerr << hdline << endl;
  fgets(hdline,100,in);
  //   cerr << hdline << endl;
  if (sscanf(hdline, "%s%d%d%d%d%d",
	 tmpstr, &year, &month, &day, &hour, &min) != 6) {
      cerr << "  Cannot compute time from header file" << endl;
  //    cerr <<  tmpstr << year << month << day << hour <<  min << endl;
  }
  DateTime validTime(year, month, day, hour, min, 0);

  // create spdb output object
  DsSpdb out;

  //  data reading
  while (!feof(in)) {

    char line[1024];

    if (fgets(line, 1024, in) == NULL) {
      break;
    }

    nline++;

    char stid[128];
    char stnm[128];
    int TIME;
    double lat, lon, elev, rain, min10, hr03, hr06, hr12, hr24;

    if (sscanf(line, "%s%s%d%lg%lg%lg%lg%lg%lg%lg%lg%lg",
	       stid, stnm, &TIME, &lat, &lon, &elev, 
	       &rain, &min10, &hr03, &hr06, &hr12, &hr24) != 12) {
      if (strstr(line, "STID") == NULL) {
	// not comments lines, print error message
	cerr << "WARNING - CwbObs2Spdb::_processGaugeFile" << endl;
	cerr << "  Cannot read line: " <<  nline << line;
      }
      continue;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read line: " << line;
      cerr << " stid: " << stid << endl;
      cerr << " lat: " << lat << endl;
      cerr << " lon: " << lon << endl;
      cerr << " elev: " << elev << endl;
      cerr << " Rain: " << rain << endl;
      cerr << " 10 minute accumulation " << min10 << endl;
      cerr << " 3 hr accumulation " << hr03 << endl;
      cerr << " 6 hr accumulation " << hr06 << endl;
      cerr << " 12 hr accumulation " << hr12 << endl;
      cerr << " 24 hr accumulation " << hr24 << endl;
    }
    
    // fill out station report
    
    station_report_t report;
    memset(&report, 0, sizeof(report));
    
    STRncopy(report.station_label, stid, ST_LABEL_SIZE);
    
    report.msg_id = SENSOR_REPORT;
    report.time = validTime.utime();
    
    report.lat = lat;
    report.lon = lon;
    report.alt = elev;
    if( hr03 <= _params.bad_data_value)
    {
      report.liquid_accum = 0;
    }
    else
    {
      report.liquid_accum = hr03;
    }

    report.temp = STATION_NAN;
    report.dew_point = STATION_NAN;
    report.relhum = STATION_NAN;
    report.windspd = STATION_NAN;
    report.winddir = STATION_NAN;
    report.windgust = STATION_NAN;
    report.pres = STATION_NAN;

    if( min10 <= _params.bad_data_value)
    {
      report.precip_rate = 0;
    }
    else
    {
      report.precip_rate = min10 * 6;
    }

    report.visibility = STATION_NAN;
    report.ceiling = STATION_NAN;
    report.rvr = STATION_NAN;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "====== FOUND REPORT ======" << endl;
      print_station_report(stderr, "", &report);
      cerr << "==========================" << endl;
    }
    
    // swap bytes
    
    station_report_to_be(&report);
    
    // add chunk
    
    int stationId = Spdb::hash4CharsToInt32(stid + 1);
    out.addPutChunk(stationId,
		    validTime.utime(),
		    validTime.utime() + _params.expire_seconds,
		    sizeof(report), &report);
    
    nObs++;
    
  } // while

  if (nObs < 0) {
    cerr << "ERROR - CwbObs2Spdb::_processGaugeFile" << endl;
    cerr << "  No valid obs in file: " << file_path << endl;
    return -1;
  }

  // put the data
  
  if (out.put(_params.output_url,
	      SPDB_STATION_REPORT_ID,
	      SPDB_STATION_REPORT_LABEL)) {
    cerr << "ERROR - CwbObs2Spdb::_processGaugeFile" << endl;
    cerr << "  Cannot put station data to: "
	 << _params.output_url << endl;
    cerr << "  " << out.getErrStr() << endl;
    iret = -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Wrote " << nObs << " reports to url: "
	 << _params.output_url << endl;
  }

  if (_params.debug) {
    cerr << "  Done with file: " << file_path << endl;
  }

  return iret;
   
}

