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
// Mesonet2Spdb.cc
//
// Mesonet2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2010
//
///////////////////////////////////////////////////////////////////////
//
// Mesonet2Spdb reads mesonet weather station surface
// observations, and writes them to an SPDB data base
//
///////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaStr.hh>
#include <didss/DsInputPath.hh>
#include <rapformats/station_reports.h>
#include "Mesonet2Spdb.hh"
using namespace std;

// Constructor

Mesonet2Spdb::Mesonet2Spdb(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "Mesonet2Spdb";
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

  // load up the station locations

  if (_params.use_station_location_file) {
    if (_loadLocations(_params.station_location_file_path)) {
      isOK = false;
      return;
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

Mesonet2Spdb::~Mesonet2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Mesonet2Spdb::Run ()
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
      cerr << "WARNING - Mesonet2Spdb::Run" << endl;
      cerr << "  Errors in processing file: " << inputPath << endl;
    }
    
  } // while
    
  return 0;

}

////////////////////
// process the file

int Mesonet2Spdb::_processFile(const char *file_path)
  
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
  
  int nObs = _readFile(file_path, out);
  if (nObs < 0) {
    cerr << "ERROR - Mesonet2Spdb::_processFile" << endl;
    cerr << "  Cannot read file: " << file_path << endl;
    return -1;
  }

  // put the data

  if (out.put(_params.output_url,
	      SPDB_STATION_REPORT_ID,
	      SPDB_STATION_REPORT_LABEL)) {
    cerr << "ERROR - Mesonet2Spdb::_processFile" << endl;
    cerr << "  Cannot put station data to: "
	 << _params.output_url << endl;
    cerr << "  " << out.getErrStr() << endl;
    iret = -1;
  }

  if (_params.debug) {
    cerr << "  Done with file: " << file_path << endl;
  }

  return iret;
   
}

//////////////////////////////////////////////
// read the file, store chunks in SPDB object

int Mesonet2Spdb::_readFile(const char *file_path,
                            DsSpdb &out)
  
{

  // open file

  FILE *in;
  if((in = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Mesonet2Spdb::_readFile" << endl;
    cerr << "  Cannot open file: " << file_path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // read a line at a time

  char line[BUFSIZ];
  DateTime baseTime;
  bool baseTimeFound = false;
  vector<Params::field_type_t> fieldTypes;

  while (!feof(in)) {

    // get next line

    if (fgets(line, BUFSIZ, in) == NULL) {
      break;
    }
    if (strlen(line) < 2) {
      continue;
    }
    
    // remove line feed and ctrl-M
    
    int lineLen = strlen(line);
    for (int ii = 0; ii < lineLen; ii++) {
      if (line[ii] == 012 || line[ii] == 015) {
        line[ii] = ' ';
      }
    }
    
    // check for comments or header line

    if (line[0] == '#') {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Ignoring comment line: " << line << endl;
      }
      continue;
    }
    if (strchr(line, '!') != NULL) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Ignoring version line: " << line << endl;
      }
      continue;
    }

    // check for baseTime line
    
    if (strchr(line, '.') == NULL) {
      int nfields, year, month, day, hour, min, sec;
      if (sscanf(line, "%d %d %d %d %d %d %d",
                 &nfields, &year, &month, &day, &hour, &min, &sec) == 7) {
        baseTime.set(year, month, day, hour, min, sec);
        baseTimeFound = true;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Found base time line: " << line << endl;
          cerr << "Base time: " << DateTime::strm(baseTime.utime()) << endl;
        }
        continue;
      }
    }
    
    // tokenize the line

    vector<string> toks;
    TaStr::tokenize(line, " ", toks);

    // check for labels line

    if (_holdsFieldLabels(toks)) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Found field labels line:" << line << endl;
      }
      _setFieldTypes(toks, fieldTypes);
      continue;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Processing line: " << line << endl;
    }
    WxObs obs;
    if (_decodeObservation(toks, fieldTypes,
                           baseTime, baseTimeFound, obs) == 0) {

      if (_params.output_format == Params::STATION_REPORT) {
        obs.assembleAsReport(STATION_REPORT);
      } else {
        obs.assembleAsXml();
      }

      int stationId = Spdb::hash4CharsToInt32(obs.getStationId().c_str());

      out.addPutChunk(stationId,
                      obs.getObservationTime(),
                      obs.getObservationTime() + _params.expire_seconds,
                      obs.getBufLen(), obs.getBufPtr());

    }
    
  } // while (!foef(in))
  

  fclose(in);
  return 0;

}

////////////////////////////////////////////
// Does this line contain the field labels?

bool Mesonet2Spdb::_holdsFieldLabels(const vector<string> &toks)

{  

  // Each label must contain some alpha characters
  // If one does not, this cannot be the labels line

  for (size_t ii = 0; ii < toks.size(); ii++) {
    const string &tok = toks[ii];
    bool hasAlpha = false;
    for (size_t jj = 0; jj < tok.size(); jj++) {
      if (isalpha(tok[jj])) {
        hasAlpha = true;
        break;
      }
    } // jj
    if (!hasAlpha) {
      return false;
    }
  } // ii

  return true;

}

////////////////////////////////////////////////
// set the field types

void Mesonet2Spdb::_setFieldTypes(const vector<string> &toks,
                                  vector<Params::field_type_t> &fieldTypes)

{

  fieldTypes.clear();

  for (size_t ii = 0; ii < toks.size(); ii++) {
    const string &tok = toks[ii];
    Params::field_type_t ftype = Params::FIELD_UNKNOWN;
    for (int jj = 0; jj < _params.field_defs_n; jj++) {
      const Params::field_def_t &def = _params._field_defs[jj];
      string label(def.label);
      if (label == tok) {
        ftype = def.ftype;
      }
    } // jj
    fieldTypes.push_back(ftype);
  } // ii

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "nLabels: " << fieldTypes.size() << endl;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Decoding field labels:" << endl;
    for (size_t ii = 0; ii < toks.size(); ii++) {
      cerr << "  Label, type: " << toks[ii]
           << ", " << _fieldType2Str(fieldTypes[ii]) << endl;
    }
  }

}

////////////////////////////////////////////////
// decode an observation
//
// Returns 0 on success, -1 on failure

int Mesonet2Spdb::_decodeObservation(const vector<string> &toks,
                                     vector<Params::field_type_t> &fieldTypes,
                                     const DateTime &baseTime,
                                     bool baseTimeFound,
                                     WxObs &obs)
  
{
  
  obs.reset();
  obs.setObservationTime(baseTime.utime());
                         
  int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;
  int secsSinceBase = 0;
  double accumSecs = 86400;
  string stationId;

  // strings

  for (size_t ii = 0; ii < toks.size(); ii++) {
    if (fieldTypes[ii] == Params::STATION_ID) {
      const string &tok = toks[ii];
      stationId = tok;
      obs.setStationId(stationId);
    }
  } // strings

  // integers

  for (size_t ii = 0; ii < toks.size(); ii++) {
    const string &tok = toks[ii];
    int ival;
    if (sscanf(tok.c_str(), "%d", &ival) != 1) {
      continue;
    }
    if (ival < 0) {
      continue;
    }
    switch (fieldTypes[ii]) {
      case Params::SECS_SINCE_BASE_TIME:
        secsSinceBase = ival;
        break;
      case Params::MINS_SINCE_BASE_TIME:
        secsSinceBase = ival * 60;
        break;
      case Params::YEAR:
        year = ival;
        break;
      case Params::MONTH:
        month = ival;
        break;
      case Params::DAY:
        day = ival;
        break;
      case Params::HOUR:
        hour = ival;
        break;
      case Params::MIN:
        min = ival;
        break;
      case Params::SEC:
        sec = ival;
        break;
      default: {}
    }
  } // integers

  // floats

  for (size_t ii = 0; ii < toks.size(); ii++) {
    const string &tok = toks[ii];
    double dval;
    if (sscanf(tok.c_str(), "%lg", &dval) != 1) {
      continue;
    } else {
      if (dval < -900) {
        continue;
      }
    }
    switch (fieldTypes[ii]) {
      case Params::LATITUDE:
        obs.setLatitude(dval);
        break;
      case Params::LONGITUDE:
        obs.setLongitude(dval);
        break;
      case Params::ELEVATION_M:
        obs.setElevationM(dval);
        break;
      case Params::TEMP_C:
        obs.setTempC(dval);
        break;
      case Params::MIN_TEMP_C:
        obs.setMinTempC(dval, 86400);
        break;
      case Params::MAX_TEMP_C:
        obs.setMaxTempC(dval, 86400);
        break;
      case Params::DEWPOINT_C:
        obs.setDewpointC(dval);
        break;
      case Params::RH_PERCENT:
        obs.setRhPercent(dval);
        break;
      case Params::WIND_DIRN_DEGT:
        obs.setWindDirnDegT(dval);
        break;
      case Params::WIND_SPEED_MPS:
        obs.setWindSpeedMps(dval);
        break;
      case Params::WIND_GUST_MPS:
        obs.setWindGustMps(dval);
        break;
      case Params::VISIBILITY_KM:
        obs.setVisibilityKm(dval);
        break;
      case Params::EXTINCTION_PER_KM:
        obs.setExtinctionPerKm(dval);
        break;
      case Params::VERT_VIS_KM:
        obs.setVertVisKm(dval);
        break;
      case Params::CEILING_KM:
        obs.setCeilingKm(dval);
        break;
      case Params::RVR_KM:
        obs.setRvrKm(dval);
        break;
      case Params::PRESSURE_MB:
        obs.setPressureMb(dval);
        break;
      case Params::MSL_PRESSURE_MB:
        obs.setSeaLevelPressureMb(dval);
        break;
      case Params::MSL_PRESSURE_IN_HG:
        obs.setSeaLevelPressureInHg(dval);
        break;
      case Params::PRESS_TEND_MB:
        obs.setPressureTendencyMb(dval, 3600 * 6);
        break;
      case Params::PRECIP_LIQUID_MM:
        obs.setPrecipLiquidMm(dval, accumSecs);
        break;
      case Params::PRECIP_RATE_MMPH:
        obs.addPrecipRateMmPerHr(dval, 300);
        break;
      case Params::SNOW_DEPTH_MM:
        obs.setSnowDepthMm(dval, accumSecs);
        break;
      case Params::FIELD_UNKNOWN:
      default: {}
    }
  } // floats

  // time

  if (year != 0 && month != 0 && day != 0) {
    DateTime obsTime(year, month, day, hour, min, sec);
    obs.setObservationTime(obsTime.utime());
  } else {
    obs.setObservationTime(baseTime.utime() + secsSinceBase);
  }
  
  // location

  if (_params.use_station_location_file) {

    // check if in the main list
    
    map< string, StationLoc, less<string> >::iterator iloc;
    iloc = _locations.find(stationId);
    if (iloc == _locations.end()) {
      // not found
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - cannot find location for station, ignoring: "
             << stationId << endl;
      }
      return -1;
    }
    StationLoc &loc = iloc->second;
    obs.setLatitude(loc.lat);
    obs.setLongitude(loc.lon);
    obs.setElevationM(loc.alt);
  
  } // if (_params.use_station_location_file) 
  
  // should we check accepted_stations?

  if (_params.use_accepted_stations_list) {
    bool accept = false;
    for (int ii = 0; ii < _params.accepted_stations_n; ii++) {
      if (stationId == _params._accepted_stations[ii]) {
	accept = true;
	break;
      }
    }
    if (!accept) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << endl;
	cerr << "Rejecting station: " << stationId << endl;
	cerr << "  Not in accepted_stations list" << endl;
      }
      return -1;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << endl;
      cerr << "Conditionally accepting station: " << stationId << endl;
      cerr << "  Is in accepted_stations list" << endl;
    }
  }

  // should we check rejectedStations?

  if (_params.use_rejected_stations_list) {
    bool reject = false;
    for (int ii = 0; ii < _params.rejected_stations_n; ii++) {
      if (stationId == _params._rejected_stations[ii]) {
	reject = true;
	break;
      }
    }
    if (reject) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << endl;
	cerr << "Rejecting station: " << stationId << endl;
	cerr << "  Station name is in rejected_stations list" << endl;
      }
      return -1;
    }
  }

  // should we check bounding box?

  if (_params.check_bounding_box) {
    if (obs.getLatitude() < _params.bounding_box.min_lat ||
	obs.getLatitude() > _params.bounding_box.max_lat ||
	obs.getLongitude() < _params.bounding_box.min_lon ||
	obs.getLongitude() > _params.bounding_box.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << endl;
	cerr << "Rejecting station: " << stationId << endl;
	cerr << "  Station position no within bounding box" << endl;
      }
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    obs.print(cerr);
  }

  return 0;

}

////////////////////////////////
// load up the station locations

int Mesonet2Spdb::_loadLocations(const char* station_location_path)
  
{

  FILE *fp;
  char line[BUFSIZ];
  char station[128];
  double lat, lon, alt;
  
  string stationId;
  
  if((fp = fopen(station_location_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Mesonet2Spdb::loadLocations" << endl;
    cerr << "  Cannot open station location file: "
	 << station_location_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
   
  int count = 0;

  while( fgets(line, BUFSIZ, fp) != NULL ) {

    // If the line is not a comment, process it
    
    if( line[0] == '#' ) {
      continue;
    }

    // Read in the line - try different formats
    
    if( sscanf(line, "%4s, %lg, %lg, %lg", 
	       station, &lat, &lon, &alt) != 4 ) {
      if( sscanf(line, "%4s,%lg,%lg,%lg", 
		 station, &lat, &lon, &alt) != 4 ) {
	if( sscanf(line, "%3s,%lg,%lg,%lg", 
		   station, &lat, &lon, &alt) != 4 ) {
	  if (_params.debug) {
	    cerr << "WARNING - Mesonet2Spdb::loadLocations" << endl;
	    cerr << "  Cannot read line from station location file: "
		 << station_location_path << endl;
	    cerr << "  Line: " << line << endl;
	  }
	  continue;
	}
      }
    }
    
    count++;
      
    // Convert station to a string
    
    stationId = station;
    
    // Check for missing altitude

    if(alt <= -990.0) {
      alt = STATION_NAN;
    }
    
    // Create new location and add it to the map

    pair<string, StationLoc> pr;
    pr.first = stationId;
    pr.second.set(stationId, lat, lon, alt);
    _locations.insert(_locations.begin(), pr);

  }

  fclose(fp);

  if (count == 0) {
    cerr << "ERROR - Mesonet2Spdb::loadLocations" << endl;
    cerr << "  No suitable locations in file: : "
	 << station_location_path << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////
// get field type as a string

string Mesonet2Spdb::_fieldType2Str(Params::field_type_t ftype)
{
  switch (ftype) {
    case Params::STATION_ID: return "STATION_ID";
    case Params::STATION_NUM: return "STATION_NUM";
    case Params::SECS_SINCE_BASE_TIME: return "SECS_SINCE_BASE_TIME";
    case Params::YEAR: return "YEAR";
    case Params::MONTH: return "MONTH";
    case Params::DAY: return "DAY";
    case Params::HOUR: return "HOUR";
    case Params::MIN: return "MIN";
    case Params::SEC: return "SEC";
    case Params::LATITUDE: return "LATITUDE";
    case Params::LONGITUDE: return "LONGITUDE";
    case Params::ELEVATION_M: return "ELEVATION_M";
    case Params::TEMP_C: return "TEMP_C";
    case Params::MIN_TEMP_C: return "MIN_TEMP_C";
    case Params::MAX_TEMP_C: return "MAX_TEMP_C";
    case Params::DEWPOINT_C: return "DEWPOINT_C";
    case Params::RH_PERCENT: return "RH_PERCENT";
    case Params::WIND_DIRN_DEGT: return "WIND_DIRN_DEGT";
    case Params::WIND_SPEED_MPS: return "WIND_SPEED_MPS";
    case Params::WIND_GUST_MPS: return "WIND_GUST_MPS";
    case Params::VISIBILITY_KM: return "VISIBILITY_KM";
    case Params::EXTINCTION_PER_KM: return "EXTINCTION_PER_KM";
    case Params::VERT_VIS_KM: return "VERT_VIS_KM";
    case Params::CEILING_KM: return "CEILING_KM";
    case Params::RVR_KM: return "RVR_KM";
    case Params::PRESSURE_MB: return "PRESSURE_MB";
    case Params::MSL_PRESSURE_MB: return "MSL_PRESSURE_MB";
    case Params::MSL_PRESSURE_IN_HG: return "MSL_PRESSURE_IN_HG";
    case Params::PRESS_TEND_MB: return "PRESS_TEND_MB";
    case Params::PRECIP_LIQUID_MM: return "PRECIP_LIQUID_MM";
    case Params::PRECIP_RATE_MMPH: return "PRECIP_RATE_MMPH";
    case Params::SNOW_DEPTH_MM: return "SNOW_DEPTH_MM";
    case Params::FIELD_UNKNOWN:
    default:
      return "FIELD_UNKNOWN";
  }
  return "FIELD_UNKNOWN";
}

