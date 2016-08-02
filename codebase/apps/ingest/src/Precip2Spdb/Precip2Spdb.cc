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
// Precip2Spdb.cc
//
// Precip2Spdb object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2015
//
///////////////////////////////////////////////////////////////
//
// Precip2Spdb reads automated weather station surface
// observations, converts them to station_report_t format
// and writes them to an SPDB data base
//
///////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <physics/thermo.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/WxObs.hh>
#include "Precip2Spdb.hh"
using namespace std;

// Constructor

Precip2Spdb::Precip2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "Precip2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
    return;
  }

  // init process mapper registration

  if (_params.mode == Params::REALTIME) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  return;

}

// destructor

Precip2Spdb::~Precip2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Precip2Spdb::Run ()
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
  int iret = 0;
  while ((inputPath = input->next()) != NULL) {

    if (_processFile(inputPath)) {
      cerr << "WARNING - Precip2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
      iret = -1;
    }
    
  } // while
    
  return iret;

}

////////////////////
// process file

int Precip2Spdb::_processFile(const char *file_path)
  
{

  int iret = 0;

  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
  }

  // registration
  
  char procmapString[BUFSIZ];
  Path path(file_path);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_force_register(procmapString);
  
  // create spdb output object
  
  DsSpdb out;

  // read in file, loading up chunks in output object spdb
  
  FILE *in;
  if ((in = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Precip2Spdb::_processAwsFile" << endl;
    cerr << "  Cannot open file: " << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  int nObs = 0;
  if (_params.input_format == Params::RAL_STEP_ASCII) {
    if (_readRalStepAscii(file_path, in, out, nObs)) {
      cerr << "ERROR - Precip2Spdb::_processAwsFile" << endl;
      cerr << "  Error reading data from file: " << file_path << endl;
      fclose(in);
      return -1;
    }
  } else if (_params.input_format == Params::RAL_NCDC_CSV) {
    if (_readRalNcdcCsv(file_path, in, out, nObs)) {
      cerr << "ERROR - Precip2Spdb::_processAwsFile" << endl;
      cerr << "  Error reading data from file: " << file_path << endl;
      fclose(in);
      return -1;
    }
  }

  // close file

  fclose(in);

  // check for sanity
  
  if (nObs < 0) {
    cerr << "WARNING - Precip2Spdb::_processAwsFile" << endl;
    cerr << "  No valid obs in file: " << file_path << endl;
    return -1;
  }

  // put the data
  
  if (out.put(_params.output_url,
	      SPDB_STATION_REPORT_ID,
	      SPDB_STATION_REPORT_LABEL)) {
    cerr << "ERROR - Precip2Spdb::_processAwsFile" << endl;
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

///////////////////////////
// read in STEP-format data

int Precip2Spdb::_readRalStepAscii(const char *file_path,
                                   FILE *in,
                                   DsSpdb &out,
                                   int &nObs)
  
{

  DateTime startTime, endTime;
  nObs = 0;
  while (!feof(in)) {

    // read in a line

    char line[1024];
    if (fgets(line, 1024, in) == NULL) {
      break;
    }

    // set start and end times

    if (strstr(line, "Start Time") != NULL) {
      char *colon = strrchr(line, ':');
      if (colon != NULL) {
        int year, month, day, hour, min;
        if (sscanf(colon + 2, "%4d%2d%2d %2d%2d",
                   &year, &month, &day, &hour, &min) == 5) {
          startTime.set(year, month, day, hour, min, 0);
        }
      }
      continue;
    }

    if (strstr(line, "End Time") != NULL) {
      char *colon = strrchr(line, ':');
      if (colon != NULL) {
        int year, month, day, hour, min;
        if (sscanf(colon + 2, "%4d%2d%2d %2d%2d",
                   &year, &month, &day, &hour, &min) == 5) {
          endTime.set(year, month, day, hour, min, 0);
        }
      }
      continue;
    }

    // skip rest of comments
    
    if (line[0] == _params.comment_character[0]) {
      continue;
    }
    
    // tokenize the line
    
    vector<string> toks;
    TaStr::tokenize(line, _params.data_column_delimiter, toks);
    if (toks.size() != 9) {
      // need 9 columns
      continue;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read obs line: " << line;
    }

    // get data from toks
    
    string idStr = toks[0];
    int stationId = Spdb::hash4CharsToInt32(idStr.c_str());

    double lat = atof(toks[1].c_str());
    double lon = atof(toks[2].c_str());
    double altM = atof(toks[3].c_str());
    double P24h_mm = atof(toks[4].c_str());
    double P6h_06utc = atof(toks[5].c_str());
    double P6h_12utc = atof(toks[6].c_str());
    double P6h_18utc = atof(toks[7].c_str());
    double P6h_00utc = atof(toks[8].c_str());

    // check bounding box

    if (_params.constrain_using_bounding_box) {
      if (_params.bounding_box_min_lon < _params.bounding_box_max_lon) {
        if (lon < _params.bounding_box_min_lon ||
            lon > _params.bounding_box_max_lon) {
          continue;
        }
      } else {
        if (lon < _params.bounding_box_min_lon &&
            lon > _params.bounding_box_max_lon) {
          continue;
        }
      }
      if (lat < _params.bounding_box_min_lat ||
          lat > _params.bounding_box_max_lat) {
        continue;
      }
    }

    // store data

    if (P6h_06utc >= 0) {

      // 6 hr accum at 06 UTC
      
      WxObs obs;
      obs.setObservationTime(startTime.utime() + 21600);
      obs.setStationId(idStr);
      obs.setLatitude(lat);
      obs.setLongitude(lon);
      obs.setElevationM(altM);
      obs.addPrecipLiquidMm(P6h_06utc, 21600);
      obs.assembleAsXml();

      time_t validTime = obs.getObservationTime();
      out.addPutChunk(stationId,
                      validTime,
                      validTime + _params.expire_seconds,
                      obs.getBufLen(), obs.getBufPtr());

      nObs++;
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "=== 6-hr accum at 06 UTC ===" << endl;
        obs.print(cerr);
        cerr << "============================" << endl;
      }

    } // 6 hr accum at 06 UTC
      
    
    if (P6h_12utc >= 0) {

      // 6 hr accum at 12 UTC
      
      WxObs obs;
      obs.setObservationTime(startTime.utime() + 43200);
      obs.setStationId(idStr);
      obs.setLatitude(lat);
      obs.setLongitude(lon);
      obs.setElevationM(altM);
      obs.addPrecipLiquidMm(P6h_12utc, 21600);
      obs.assembleAsXml();

      time_t validTime = obs.getObservationTime();
      out.addPutChunk(stationId,
                      validTime,
                      validTime + _params.expire_seconds,
                      obs.getBufLen(), obs.getBufPtr());

      nObs++;
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "=== 6-hr accum at 12 UTC ===" << endl;
        obs.print(cerr);
        cerr << "============================" << endl;
      }

    } // 6 hr accum at 12 UTC
      
    
    if (P6h_18utc >= 0) {

      // 6 hr accum at 18 UTC
      
      WxObs obs;
      obs.setObservationTime(startTime.utime() + 64800);
      obs.setStationId(idStr);
      obs.setLatitude(lat);
      obs.setLongitude(lon);
      obs.setElevationM(altM);
      obs.addPrecipLiquidMm(P6h_18utc, 21600);
      obs.assembleAsXml();
      
      time_t validTime = obs.getObservationTime();
      out.addPutChunk(stationId,
                      validTime,
                      validTime + _params.expire_seconds,
                      obs.getBufLen(), obs.getBufPtr());

      nObs++;
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "=== 6-hr accum at 18 UTC ===" << endl;
        obs.print(cerr);
        cerr << "============================" << endl;
      }

    } // 6 hr accum at 18 UTC
      
    
    if (P6h_00utc >= 0 || P24h_mm >= 0) {
      
      // 6 hr and 24 hr accum at 0 UTC
      
      WxObs obs;
      obs.setObservationTime(startTime.utime() + 86400);
      obs.setStationId(idStr);
      obs.setLatitude(lat);
      obs.setLongitude(lon);
      obs.setElevationM(altM);
      if (P6h_00utc >= 0) {
        obs.addPrecipLiquidMm(P6h_00utc, 21600);
      }
      if (P24h_mm >= 0) {
        obs.addPrecipLiquidMm(P24h_mm, 86400);
      }

      obs.assembleAsXml();

      time_t validTime = obs.getObservationTime();
      out.addPutChunk(stationId,
                      validTime,
                      validTime + _params.expire_seconds,
                      obs.getBufLen(), obs.getBufPtr());

      nObs++;
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "=== 6-hr and 24-hr accum at 00 UTC ===" << endl;
        obs.print(cerr);
        cerr << "============================" << endl;
      }
      
    } // 6 hr and 24 hr accum at 00 UTC
    
  } // while

  return 0;
   
}

//////////////////////////////
// read in RAL NCDC CVS format

int Precip2Spdb::_readRalNcdcCsv(const char *file_path,
                                 FILE *in,
                                 DsSpdb &out,
                                 int &nObs)
  
{

  DateTime startTime, endTime;
  nObs = 0;
  while (!feof(in)) {

    // read in a line

    char line[1024];
    if (fgets(line, 1024, in) == NULL) {
      break;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read line: " << line;
    }

    // skip comments
    
    if (line[0] == _params.comment_character[0]) {
      continue;
    }
    
    // tokenize the line
    
    vector<string> toks;
    TaStr::tokenize(line, _params.data_column_delimiter, toks);
    if (toks.size() != 11) {
      // need 11 columns
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - bad line: " << line;
        cerr << "  Should have 11 columns" << endl;
        continue;
      }
    }

    // get data from toks
    
    string idStr = toks[0];

    double lat = atof(toks[1].c_str());
    double lon = atof(toks[2].c_str());
    double altM = atof(toks[3].c_str());
    int year = atoi(toks[4].c_str());
    int month = atoi(toks[5].c_str());
    int day = atoi(toks[6].c_str());
    int hour = atoi(toks[7].c_str());
    int min = atoi(toks[8].c_str());
    int sec = atoi(toks[9].c_str());
    double accum24hr = atof(toks[10].c_str());

    if (year == 0 && month == 0 && day == 0) {
      continue;
    }

    if (accum24hr < 0) {
      continue;
    }

    // check bounding box

    if (_params.constrain_using_bounding_box) {
      if (_params.bounding_box_min_lon < _params.bounding_box_max_lon) {
        if (lon < _params.bounding_box_min_lon ||
            lon > _params.bounding_box_max_lon) {
          continue;
        }
      } else {
        if (lon < _params.bounding_box_min_lon &&
            lon > _params.bounding_box_max_lon) {
          continue;
        }
      }
      if (lat < _params.bounding_box_min_lat ||
          lat > _params.bounding_box_max_lat) {
        continue;
      }
    }

    // compute time

    if (hour == -999) {
      hour = _params.hour_if_time_missing;
    } else {
      hour = hour / 100;
    }

    DateTime localTime(year, month, day, hour, min, sec);
    DateTime utcTime(localTime);
    if (_params.time_correction_secs) {
      utcTime.set(localTime.utime() + _params.time_correction_secs);
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "============>> localTime: " << DateTime::strm(localTime.utime()) << endl;
      cerr << "============>> utcTime: " << DateTime::strm(utcTime.utime()) << endl;
    }
    
    // set data

    WxObs obs;
    obs.setObservationTime(utcTime.utime());
    obs.setStationId(idStr);
    obs.setLatitude(lat);
    obs.setLongitude(lon);
    obs.setElevationM(altM);
    obs.addPrecipLiquidMm(accum24hr, 86400);
    obs.assembleAsXml();

    int dataType = Spdb::hash4CharsToInt32(idStr.c_str() + idStr.size() - 4);
    int dataType2 = Spdb::hash4CharsToInt32(idStr.c_str() + idStr.size() - 8);
    
    time_t validTime = obs.getObservationTime();
    out.addPutChunk(dataType,
                    validTime,
                    validTime + _params.expire_seconds,
                    obs.getBufLen(), obs.getBufPtr(),
                    dataType2,
                    idStr.c_str());
    
    nObs++;
      
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      obs.print(cerr);
      cerr << "==================" << endl;
    }

  } // while

  return 0;
   
}

