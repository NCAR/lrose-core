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
// CsuSounding2Spdb.cc
//
// CsuSounding2Spdb object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2015
//
///////////////////////////////////////////////////////////////
//
// CsuSounding2Spdb reads sounding data from Colorado State
// University (CSU),
// converts it to sounding format and writes them
// to an SPDB data base.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <physics/physics.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <toolsa/TaStr.hh>
#include <didss/DsInputPath.hh>
#include "CsuSounding2Spdb.hh"
using namespace std;

// Constructor

const double CsuSounding2Spdb::MissingVal = -9999.0;

CsuSounding2Spdb::CsuSounding2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "CsuSounding2Spdb";
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

CsuSounding2Spdb::~CsuSounding2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int CsuSounding2Spdb::Run ()
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
      cerr << "WARNING - CsuSounding2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
    
  } // while
    
  return 0;

}

////////////////////
// process the file

int CsuSounding2Spdb::_processFile(const char *file_path)
  
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
    cerr << "ERROR - CsuSounding2Spdb::_processFile" << endl;
    cerr << "  Cannot open file: " << file_path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }
  
  // create spdb output object
  
  SoundingPut sounding;
  sounding.init(_params.output_url, Sounding::SONDE_ID, "CSU" );
  sounding.setMissingValue(MissingVal);

  // Initialize time from file name

  _releaseTime = time(NULL);
  const char *firstDigitOfName = NULL;
  for (size_t ii = 0; ii < path.getFile().size(); ii++) {
    if (isdigit(path.getFile()[ii])) {
      firstDigitOfName = path.getFile().c_str() + ii;
      break;
    }
  }
  if (firstDigitOfName != NULL) {
    int year, month, day, hour;
    if (sscanf(firstDigitOfName, "%4d%2d%2d_%2d",
	       &year, &month, &day, &hour) == 4) {
      DateTime rtime(year, month, day, hour, 0, 0);
      _releaseTime = rtime.utime();
    }
  }

  // initialze launch location from parameters
  
  _stationLat = _params.launch_latitude_deg;
  _stationLon = _params.launch_longitude_deg;
  _stationAltMeters = _params.launch_altitude_m;

  // read the header

  if (_readHeader(in, sounding)) {
    cerr << "ERROR - CsuSounding2Spdb::_processFile" << endl;
    cerr << "  Cannot find header section, file: "
	 << file_path << endl;
    fclose(in);
    return -1;
  }

  if (_params.debug) {
    cerr << "Read header" << endl;
    cerr << "  _stationName: " << _stationName << endl;
    cerr << "  _releaseTime: " << DateTime::strm(_releaseTime) << endl;
    cerr << "  _stationLat: " << _stationLat << endl;
    cerr << "  _stationLon: " << _stationLon << endl;
    cerr << "  _stationAlt: " << _stationAltMeters << endl;
  }

  int offset = _stationName.size() - 5;
  if (offset < 0) {
    offset = 0;
  }
  sounding.setSiteId(Spdb::hash4CharsToInt32(_stationName.c_str() + offset));
  sounding.setSourceId(Sounding::SONDE_ID);
  sounding.setSiteName(_stationName);

  // read in the data

  if (_readData(in, sounding)) {
    cerr << "ERROR - CsuSounding2Spdb::_processFile" << endl;
    cerr << "  Cannot read data, file: "
	 << file_path << endl;
    fclose(in);
    return -1;
  }

  // set the data - may override position

  sounding.set(_releaseTime, &_heightMeters, &_uu, &_vv, &_ww,
               &_pressureHpa, &_relHum, &_tempC);

  // set position

  sounding.setLocation(_stationLat, _stationLon, _stationAltMeters / 1000.0);

  // put the data

  time_t validTime = _releaseTime;
  time_t expireTime = validTime + _params.expire_seconds;
  int leadSecs = 0;

  if (sounding.writeSounding(validTime, expireTime, leadSecs)) {
    cerr << "ERROR - CsuSounding2Spdb::_processFile" << endl;
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

int CsuSounding2Spdb::_readHeader(FILE *in, SoundingPut &sounding)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading header" << endl;
  }

  char line[BUFSIZ];

  while (!feof(in)) {
    
    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    line[strlen(line)-1] = '\0';
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Got header line: " << line << endl;
    }
    
    if (strstr(line, "Balloon release date and time") ||
	strstr(line, "Launch time")) {
      string timeStr = _parseDigitVal(line);
      if (timeStr.size() < 12) {
        cerr << "Time string too short: " << line;
        return -1;
      }
      DateTime dtime(timeStr);
      _releaseTime = dtime.utime();
    }
    
    if (strstr(line, "Station name") ||
	strstr(line, "Station")) {
      _stationName = _parseStringVal(line);
    }

    if (strstr(line, "Release point latitude")) {
      string latStr = _parseStringVal(line);
      char ns;
      if (sscanf(latStr.c_str(), "%lg?%c", &_stationLat, &ns) == 2) {
        if (ns == 'S') {
          _stationLat *= -1.0;
        }
      }
    }

    if (strstr(line, "Release point longitude")) {
      string lonStr = _parseStringVal(line);
      char ew;
      if (sscanf(lonStr.c_str(), "%lg?%c", &_stationLon, &ew) == 2) {
        if (ew == 'W') {
          _stationLon *= -1.0;
        }
      }
    }

    if (strstr(line, "Release point height")) {
      string htStr = _parseStringVal(line);
      if (sscanf(htStr.c_str(), "%lg ", &_stationAltMeters) == 1) {
      }
    }

    if (strstr(line, "Elapsed time")) {
      // done with header
      return 0;
    }

    if (strstr(line, "Temp") &&
	strstr(line, "Dew") &&
	strstr(line, "RH")) {
      // done with header
      return 0;
    }

  }

  return -1;

}

int CsuSounding2Spdb::_readData(FILE *in, SoundingPut &sounding)

{
  
  _elapsedSecs.clear();
  _heightMeters.clear();
  _pressureHpa.clear();
  _tempC.clear();
  _dewptC.clear();
  _relHum.clear();
  
  _windSpeed.clear();
  _windDirn.clear();
  _ascentRate.clear();
  _lat.clear();
  _lon.clear();
  _gpsHt.clear();

  _uu.clear();
  _vv.clear();
  _ww.clear();

  // read through the field data
  
  char line[BUFSIZ];
  while (!feof(in)) {

    if (fgets(line, BUFSIZ, in) == NULL) {
      // end of file
      return 0;
    }

    line[strlen(line)-1] = '\0';
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Got line: " << line << endl;
    }

    // get number of tokens in line

    vector<string> toks;
    TaStr::tokenize(line, " ", toks);

    // parse the line

    double elapsedSecs = -9999;
    double heightMeters = -9999;
    double pressureHpa = -9999;
    double tempC = -9999;
    double dewptC = -9999;
    double relHum = -9999;
    
    double windSpeed = -9999;
    double windDirn = -9999;
    double uu, vv, ww;
    double ascentRate = -9999;
    double lat = -9999;
    double lon = -9999;
    double gpsHt = -9999;

    bool success = false;
    if (toks.size() == 12) {
      if (sscanf(line, "%lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg",
		 &elapsedSecs,
		 &heightMeters,
		 &pressureHpa,
		 &tempC,
		 &dewptC,
		 &relHum,
		 &windSpeed,
		 &windDirn,
		 &ascentRate,
		 &lat,
		 &lon,
		 &gpsHt) == 12) {
	success = true;
      }
    } else if (toks.size() == 11) {
      if (sscanf(line, "%lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg",
		 &elapsedSecs,
		 &ascentRate,
		 &heightMeters,
		 &pressureHpa,
		 &tempC,
		 &dewptC,
		 &relHum,
		 &windSpeed,
		 &windDirn,
		 &lat,
		 &lon) == 11) {
	success = true;
      }
    } else if (toks.size() == 8) {
      if (sscanf(line, "%lg %lg %lg %lg %lg %lg %lg %lg",
		 &ascentRate,
		 &heightMeters,
		 &pressureHpa,
		 &tempC,
		 &dewptC,
		 &relHum,
		 &windSpeed,
		 &windDirn) == 8) {
	success = true;
      }
    }

    if (success) {

      // derive quantities if needed
      
      if (windSpeed > -9990 && windDirn > -9999) {
        uu = PHYwind_u(windSpeed, windDirn);
        vv = PHYwind_v(windSpeed, windDirn);
      }
      ww = -9999.0; // not known
      
      if (relHum < -9990 && tempC > -9990 && dewptC > -9990) {
        relHum = PHYrelh(tempC, dewptC);
      }
      
      if (heightMeters < -9990) {
        heightMeters = _stdAtmos.pres2ht(pressureHpa) / 1000.0;
      }

      _elapsedSecs.push_back(elapsedSecs);
      _heightMeters.push_back(heightMeters);
      _pressureHpa.push_back(pressureHpa);
      _tempC.push_back(tempC);
      _dewptC.push_back(dewptC);
      _relHum.push_back(relHum);
      _uu.push_back(uu);
      _vv.push_back(vv);
      _ww.push_back(ww);
      _ascentRate.push_back(ascentRate);
      _lat.push_back(lat);
      _lon.push_back(lon);
      _gpsHt.push_back(gpsHt);

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "==>> Read elapsedSecs, htM, pressHpa, tempC, dewptC, "
	     << "RH, wspd, wdirn, ascRate, lat, lon: "
	     << elapsedSecs << ", "
	     << heightMeters << ", "
	     << pressureHpa << ", "
	     << tempC << ", "
	     << dewptC << ", "
	     << relHum << ", "
	     << windSpeed << ", "
	     << windDirn << ", "
	     << ascentRate << ", "
	     << lat << ", "
	     << lon << endl;
      }
      
    }
    
  } // while

  return 0;

}

string CsuSounding2Spdb::_parseStringVal(const string &line)
  
{
  size_t firstTab = line.find('\t');
  size_t startPos = firstTab + 1;
  string sval(line.substr(startPos));
  return sval;
}

string CsuSounding2Spdb::_parseDigitVal(const string &line)
  
{
  ssize_t firstDigit = 0;
  for (size_t ii = 0; ii < line.size(); ii++) {
    if (isdigit(line[ii])) {
      firstDigit = ii;
      break;
    }
  }
  string sval(line.substr(firstDigit));
  return sval;
}

int CsuSounding2Spdb::_parseIntVal(const string &line)

{
  size_t equals = line.find('=');
  if (equals == string::npos) {
    return -9999;
  }
  string sval(line.substr(equals + 1));
  int ival = atoi(sval.c_str());
  return ival;
}

