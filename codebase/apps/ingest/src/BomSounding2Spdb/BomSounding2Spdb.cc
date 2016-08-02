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
// BomSounding2Spdb.cc
//
// BomSounding2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2009
//
///////////////////////////////////////////////////////////////
//
// BomSounding2Spdb reads Australian BOM sounding data,
// converts them to sounding format and writes them
// to an SPDB data base.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <physics/physics.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaStr.hh>
#include <didss/DsInputPath.hh>
#include "BomSounding2Spdb.hh"
using namespace std;

// Constructor

const double BomSounding2Spdb::MissingVal = -9999.0;

BomSounding2Spdb::BomSounding2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "BomSounding2Spdb";
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

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

BomSounding2Spdb::~BomSounding2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int BomSounding2Spdb::Run ()
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
    
    if (_processFile(inputPath)) {
      cerr << "WARNING - BomSounding2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
    
  } // while
    
  return 0;

}

////////////////////
// process the file

int BomSounding2Spdb::_processFile(const char *file_path)
  
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

  // open the file

  FILE *in;
  if ((in = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - BomSounding2Spdb::_processFile" << endl;
    cerr << "  Cannot open file: " << file_path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }
  
  // create spdb output object
  
  SoundingPut sounding;
  sounding.init(_params.output_url, Sounding::SONDE_ID, "BOM-AXF" );
  sounding.setMissingValue(MissingVal);

  // Set the launch location

  _stationLat = _params.station_latitude;
  _stationLon = _params.station_longitude;
  _stationAlt = _params.station_altitude;

  // read the header

  _nLevels = 0;
  _nFields = 0;
  if (_readHeader(in, sounding)) {
    cerr << "ERROR - BomSounding2Spdb::_processFile" << endl;
    cerr << "  Cannot find header section, file: "
	 << file_path << endl;
    fclose(in);
    return -1;
  }

//   if (_nLevels == 0 || _nFields == 0) {
//     cerr << "ERROR - BomSounding2Spdb::_processFile" << endl;
//     cerr << "  Cannot find levels or fields keywords in header" << endl;
//     cerr << "  file: " << file_path << endl;
//     fclose(in);
//     return -1;
//   }

  sounding.setSiteId(_stationId);
  sounding.setSourceId(Sounding::SONDE_ID);
  sounding.setSiteName(_stationName);

  // read in the data

  if (_readData(in, sounding)) {
    cerr << "ERROR - BomSounding2Spdb::_processFile" << endl;
    cerr << "  Cannot read data, file: "
	 << file_path << endl;
    fclose(in);
    return -1;
  }

  // check for temp
  
  if (_params.require_temperature) {
    bool tempFound = false;
    for (size_t ii = 0; ii < _temperature.size(); ii++) {
      if (_temperature[ii] > -100) {
        tempFound = true;
        break;
      }
    }
    if (!tempFound) {
      cerr << "WARNING - no temperature found - ignoring" << endl;
      fclose(in);
      return 0;
    }
  }

  // set the data - may override position

  sounding.set(_aifsTime, &_height, &_u, &_v, &_w,
               &_prs, &_relHum, &_temperature);

  // set position

  sounding.setLocation(_stationLat, _stationLon, _stationAlt);

  // put the data

  time_t validTime = _aifsTime;
  time_t expireTime = validTime + _params.expire_seconds;
  int leadSecs = 0;

  if (sounding.writeSounding(validTime, expireTime, leadSecs)) {
    cerr << "ERROR - BomSounding2Spdb::_processFile" << endl;
    cerr << "  Cannot put sounding data to: "
	 << _params.output_url << endl;
    iret = -1;
  }

  if (_params.debug) {
    cerr << "  Done with file: " << file_path << endl;
  }

  return iret;
   
}

///////////////////////////
// read in header info

int BomSounding2Spdb::_readHeader(FILE *in, SoundingPut &sounding)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading header" << endl;
  }

  char line[BUFSIZ];

  bool foundHeader = false;
  while (!feof(in)) {
    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Got pre-header line: " << line;
    }
    if (strstr(line, "[Header]")) {
      foundHeader = true;
      break;
    }
  }
  if (!foundHeader) {
    cerr << "Cannot find [Header]" << endl;
    return -1;
  }

  while (!feof(in)) {

    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Got header line: " << line;
    }
    if (strstr(line, "aifstime") || strstr(line, "time=")) {
      string timeStr = _parseStringVal(line);
      if (timeStr.size() < 12) {
        cerr << "Time string too short: " << line;
        return -1;
      }
      int year, month, day, hour, min;
      if (sscanf(timeStr.c_str(), "%4d%2d%2d%2d%2d",
                 &year, &month, &day, &hour, &min) != 5) {
        cerr << "Cannot parse time: " << timeStr << endl;
        return -1;
      }
      DateTime dtime(year, month, day, hour, min, 0);
      _aifsTime = dtime.utime();
    }
    if (strstr(line, "aviation_id")) {
      _stationName = _parseStringVal(line);
    }
    if (strstr(line, "station_name")) {
      _stationName = _parseStringVal(line);
    }
    if (strstr(line, "wmo_number")) {
      _stationId = _parseIntVal(line);
    }
    if (strstr(line, "stationnumber")) {
      _stationId = _parseIntVal(line);
    }
    if (strstr(line, "levels")) {
      _nLevels = _parseIntVal(line);
    }
    if (strstr(line, "fields")) {
      _nFields = _parseIntVal(line);
    }
    if (strstr(line, "w_units")) {
      _wUnits = _parseIntVal(line);
    }
    if (strstr(line, "[$]")) {
      // done
      return 0;
    }
  }
  if (!foundHeader) {
    return -1;
  }

  return -1;

}

int BomSounding2Spdb::_readData(FILE *in, SoundingPut &sounding)

{
  
  _height.clear();
  _u.clear();
  _v.clear();
  _w.clear();
  _prs.clear();
  _relHum.clear();
  _temperature.clear();

  char line[BUFSIZ];

  // fine the Trace keyword line
  
  bool foundTrace = false;
  while (!feof(in)) {
    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Got pre-trace line: " << line;
    }
    if (strstr(line, "[Trace]")) {
      foundTrace = true;
      break;
    }
  }
  if (!foundTrace) {
    cerr << "Cannot find [Trace]" << endl;
    return -1;
  }
  
  // get the fields list
  
  if (fgets(line, BUFSIZ, in) == NULL) {
    return -1;
  }
  
  // tokenize fields list using comma separator

  vector<string> toks;
  TaStr::tokenize(line, ",", toks);

  if (_nFields == 0) {
    _nFields = toks.size();
  } else {
    if ((int) toks.size() != _nFields) {
      cerr << "ERROR - field list does not match _nFields" << endl;
      cerr << "  n fields in header: " << _nFields << endl;
      cerr << "  n fields in field list: " << toks.size() << endl;
      cerr << "  field list: " << line << endl;
      return -1;
    }
  }

  // set field indices

  int pressurePos = -1;
  int tempPos = -1;
  int dpPos = -1;
  int rhPos = -1;
  int wdirPos = -1;
  int wspdPos = -1;
  int latPos = -1;
  int lonPos = -1;
  int altPos = -1;
  
  for (int ii = 0; ii < (int) toks.size(); ii++) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> tokenizing, ii, tok: " << ii << toks[ii] << endl;
    }
    if (toks[ii].find("Pres") != string::npos) {
      pressurePos = ii;
    } else if (toks[ii].find("Temp") != string::npos) {
      tempPos = ii;
    } else if (toks[ii].find("Dew") != string::npos) {
      dpPos = ii;
    } else if (toks[ii] == "rltv_hum") {
      rhPos = ii;
    } else if (toks[ii] == "Dir" ||
               toks[ii].find("Direction") != string::npos) {
      wdirPos = ii;
    } else if (toks[ii] == "Spd" ||
               toks[ii].find("Speed") != string::npos) {
      wspdPos = ii;
    } else if (toks[ii] == "lat") {
      latPos = ii;
    } else if (toks[ii] == "lon") {
      lonPos = ii;
    } else if (toks[ii].find("ICAO") != string::npos) {
      altPos = ii;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "--->> pressurePos: " << pressurePos << endl;
    cerr << "--->> tempPos: " << tempPos << endl;
    cerr << "--->> dpPos: " << dpPos << endl;
    cerr << "--->> rhPos: " << rhPos << endl;
    cerr << "--->> wdirPos: " << wdirPos << endl;
    cerr << "--->> wspdPos: " << wspdPos << endl;
    cerr << "--->> latPos: " << latPos << endl;
    cerr << "--->> lonPos: " << lonPos << endl;
    cerr << "--->> altPos: " << altPos << endl;
  }
  
  if (_params.require_temperature) {
    if (tempPos < 0) {
      cerr << "WARNING - no temperature found - ignoring" << endl;
      return -1;
    }
  }

  // read through the field data

  int nLevelsFound = 0;
  while (!feof(in)) {

    if (fgets(line, BUFSIZ, in) == NULL) {
      // end of file
      return 0;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Got data line: " << line;
    }

    TaStr::tokenize(line, ",", toks);
    if (_nFields > 0 && toks.size() > 1 && (int) toks.size() != _nFields) {
      cerr << "WARNING - data nfields does not match _nFields" << endl;
      cerr << "  n fields in header: " << _nFields << endl;
      cerr << "  n fields in data: " << toks.size() << endl;
      cerr << "  data: " << line << endl;
      continue;
    }

    double pressure = -9999;
    double temp = -9999;
    double dp = -9999;
    double wdir = -9999;
    double wspd = -9999;
    double rh = -9999;
    double u = -9999;
    double v = -9999;
    double lat = -9999;
    double lon = -9999;
    double alt = -9999;

    if (pressurePos >= 0) {
      sscanf(toks[pressurePos].c_str(), "%lg", &pressure);
    }
    if (tempPos >= 0) {
      sscanf(toks[tempPos].c_str(), "%lg", &temp);
    }
    if (dpPos >= 0) {
      sscanf(toks[dpPos].c_str(), "%lg", &dp);
    }
    if (rhPos >= 0) {
      sscanf(toks[rhPos].c_str(), "%lg", &rh);
    }
    if (wspdPos >= 0) {
      sscanf(toks[wspdPos].c_str(), "%lg", &wspd);
    }
    if (wdirPos >= 0) {
      sscanf(toks[wdirPos].c_str(), "%lg", &wdir);
    }
    if (latPos >= 0) {
      sscanf(toks[latPos].c_str(), "%lg", &lat);
    }
    if (lonPos >= 0) {
      sscanf(toks[lonPos].c_str(), "%lg", &lon);
    }
    if (altPos >= 0) {
      sscanf(toks[altPos].c_str(), "%lg", &alt);
    }

    if (wspd > -9990 && wdir > -9999) {
      u = PHYwind_u(wspd, wdir);
      v = PHYwind_v(wspd, wdir);
    }

    if (rh < -9990 && temp > -9990 && dp > -9990) {
      rh = PHYrelh(temp, dp);
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "======>> pressure: " << pressure << endl;
      cerr << "======>> temp: " << temp << endl;
      cerr << "======>> dp: " << dp << endl;
      cerr << "======>> wdir: " << wdir << endl;
      cerr << "======>> wspd: " << wspd << endl;
      cerr << "======>> rh: " << rh << endl;
      cerr << "======>> u: " << u << endl;
      cerr << "======>> v: " << v << endl;
      cerr << "======>> lat: " << lat << endl;
      cerr << "======>> lon: " << lon << endl;
      cerr << "======>> alt: " << alt << endl;
    }
    
    if (alt < -9990) {
      double heightMeters = _stdAtmos.pres2ht(pressure) / 1000.0;
      _height.push_back(heightMeters);
    } else {
      _height.push_back(alt);
    }
    _u.push_back(u);
    _v.push_back(v);
    _w.push_back(-9999);
    _prs.push_back(pressure);
    _relHum.push_back(rh);
    _temperature.push_back(temp);

    if (nLevelsFound == 0) {
      // override params for position, if possible
      if (lat > -9990) {
        _stationLat = lat;
      }
      if (lon > -9990) {
        _stationLon = lon;
      }
      if (alt > -9990) {
        _stationAlt = alt / 1000;
      }
    }

    nLevelsFound++;
    if (_nLevels > 0 && nLevelsFound == _nLevels) {
      return 0;
    }
    
  } // while

  return 0;

}

string BomSounding2Spdb::_parseStringVal(const string &line)
  
{
  size_t firstQuote = line.find('"');
  size_t lastQuote = line.find('"', firstQuote + 1);
  if (firstQuote != string::npos && lastQuote != string::npos) {
    string sval(line.substr(firstQuote + 1, lastQuote - firstQuote - 1));
    return sval;
  }
  size_t equals = line.find('=');
  if (equals == string::npos) {
    return "";
  }
  string sval(line.substr(equals + 1));
  return sval;
}

int BomSounding2Spdb::_parseIntVal(const string &line)

{
  size_t equals = line.find('=');
  if (equals == string::npos) {
    return -9999;
  }
  string sval(line.substr(equals + 1));
  int ival = atoi(sval.c_str());
  return ival;
}

