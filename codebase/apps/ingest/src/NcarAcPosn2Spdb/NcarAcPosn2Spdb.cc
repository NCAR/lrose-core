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
///////////////////////////////////////////////////////////////
// NcarAcPosn2Spdb.cc
//
// NcarAcPosn2Spdb object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2015
//
///////////////////////////////////////////////////////////////
//
// NcarAcPosn2Spdb reads aircraft position data from ASCII file in
// JSON format. These files updated frequently and are
// overwritten. They are read each time they update, and the position
// data is then stored in SPDB.
//
/////////////////////////////////////////////////////////////

#include <iomanip>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
#include <physics/thermo.h>
#include "NcarAcPosn2Spdb.hh"
using namespace std;

// Constructor

NcarAcPosn2Spdb::NcarAcPosn2Spdb(int argc, char **argv)
  
{

  isOK = true;
  
  // set programe name

  _progName = "NcarAcPosn2Spdb";
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
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

NcarAcPosn2Spdb::~NcarAcPosn2Spdb()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int NcarAcPosn2Spdb::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  if (_params.mode == Params::REALTIME_JSON) {
    return _runRealtimeJsonMode();
  } else {
    return _runSimMode();
  }

}

//////////////////////////////////////////////////
// Run in realtime JSON mode

int NcarAcPosn2Spdb::_runRealtimeJsonMode()
{

  PMU_auto_register("_runRealtimeJsonMode");

  // initialize input
  
  vector<string> callSigns;
  vector<string> inputPaths;
  vector<time_t> prevModTimes;
  vector<time_t> prevRepeatTimes;
  
  for (int ii = 0; ii < _params.aircraft_json_n; ii++) {
    callSigns.push_back(_params._aircraft_json[ii].callsign);
    inputPaths.push_back(_params._aircraft_json[ii].input_path);
    prevModTimes.push_back(-1);
    prevRepeatTimes.push_back(-1);
  } // ii
  
  // watch for data
  
  int iret = 0;
  
  while (true) {

    for (size_t ii = 0; ii < callSigns.size(); ii++) {
      if (_checkForNewData(ii, callSigns[ii], inputPaths[ii], 
                           prevModTimes, prevRepeatTimes)) {
        iret = -1;
      }
    }
    
    // sleep a bit
    
    umsleep(1000);
    
  }

  return iret;

}

//////////////////////////////////////////////////
// Check for new data for an aircraft

int NcarAcPosn2Spdb::_checkForNewData(size_t index,
                                      const string &callSign,
                                      const string &inputPath,
                                      vector<time_t> &prevModTimes,
                                      vector<time_t> &prevRepeatTimes)
{

  PMU_auto_register("_checkForNewData");

  time_t now = time(NULL);
  bool useNow = false;

  // stat the file

  struct stat fstat;
  if (ta_stat(inputPath.c_str(), &fstat)) {
    cerr << "ERROR - NcarAcPosn2Spdb::_checkForNewData" << endl;
    cerr << "  Cannot find input path: " << inputPath << endl;
    perror(inputPath.c_str());
    return -1;
  }

  // first time?
  
  if (prevModTimes[index] < 0) {
    prevModTimes[index] = fstat.st_mtime;
    prevRepeatTimes[index] = now;
    return 0;
  }
  
  // has file updated?
  
  if (fstat.st_mtime <= prevModTimes[index]) {
    // file has not updated
    if (_params.repeat_location_if_no_change) {
      // check if we should repeat old message
      double tdiff = (double) now - prevRepeatTimes[index];
      double age = (double) now - fstat.st_mtime;
      if (tdiff < _params.repeat_period_secs) {
        if (_params.debug >= Params::DEBUG_EXTRA) {
          cerr << "File unchanged, not repeating: " << inputPath << endl;
        }
        return 0;
      } else if (age > _params.repeat_max_age_secs) {
        if (_params.debug >= Params::DEBUG_EXTRA) {
          cerr << "File too old, not repeating: " << inputPath << endl;
        }
        return 0;
      } else {
        prevRepeatTimes[index] = now;
        useNow = true;
      }
    } else {
      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "File unchanged, ignoring: " << inputPath << endl;
      }
      return 0;
    }
  }
  
  // file has updated, read it
  
  FILE *in;
  if ((in = fopen(inputPath.c_str(), "r")) == NULL) {
    cerr << "ERROR - NcarAcPosn2Spdb::_checkForNewData" << endl;
    cerr << "  Cannot open file: " << inputPath << endl;
    perror(inputPath.c_str());
    return -1;
  }

  char line[BUFSIZ];
  if (fgets(line, BUFSIZ, in) == NULL) {
    cerr << "ERROR - NcarAcPosn2Spdb::_checkForNewData" << endl;
    cerr << "  Cannot read file: " << inputPath << endl;
    perror(inputPath.c_str());
    fclose(in);
    return -1;
  }

  if (_params.debug) {
    cerr << "Read line for callsign, inputPath: " 
         << callSign << ", " << inputPath << endl;
    cerr << line;
  }
  
  // close file
  
  fclose(in);
  
  // update the modify time

  prevModTimes[index] = fstat.st_mtime;

  // decode data and write to SPDB

  if (_decodeAndWrite(callSign, line, useNow, now)) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// Decode the position data and write to SPDB

int NcarAcPosn2Spdb::_decodeAndWrite(string callSign,
                                     const char *line,
                                     bool useNow,
                                     time_t now)
  
{

  // split the line into tokens

  vector<string> toks;
  TaStr::tokenize(line, "{,}", toks);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "toks:";
    for (size_t ii = 0; ii < toks.size(); ii++) {
      cerr << " |" << toks[ii] << "|";
    }
    cerr << endl;
  }

  // decode the toks

  DateTime validTime;
  double latitude = -9999.0;
  double longitude = -9999.0;
  double altitudeFt = -9999.0;
  double headingDeg = -9999.0;

  for (size_t ii = 0; ii < toks.size(); ii++) {

    int year, month, day, hour, min, sec;
    if (sscanf(toks[ii].c_str(), "\"timestamp\":\"%4d-%2d-%2d %2d:%2d:%2d\"",
               &year, &month, &day, &hour, &min, &sec) == 6) {
      validTime.set(year, month, day, hour, min, sec);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> time: " << DateTime::strm(validTime.utime()) << endl;
      }
    }
    
    if (sscanf(toks[ii].c_str(), "\"alt\":\"%lg\"", &altitudeFt) == 1) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> altitudeFt: " << altitudeFt << endl;
      }
    } else if (sscanf(toks[ii].c_str(), "\"alt\":%lg", &altitudeFt) == 1) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> altitudeFt: " << altitudeFt << endl;
      }
    }
    
    if (sscanf(toks[ii].c_str(), "\"lat\":\"%lg\"", &latitude) == 1) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> latitude: " <<  latitude << endl;
      }
    } else if (sscanf(toks[ii].c_str(), "\"lat\":%lg", &latitude) == 1) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> latitude: " <<  latitude << endl;
      }
    }
    
    if (sscanf(toks[ii].c_str(), "\"lon\":\"%lg\"", &longitude) == 1) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> longitude: " << longitude << endl;
      }
    } else if (sscanf(toks[ii].c_str(), "\"lon\":%lg", &longitude) == 1) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> longitude: " << longitude << endl;
      }
    }
    
    if (sscanf(toks[ii].c_str(), "\"head\":\"%lg\"", &headingDeg) == 1) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> headingDeg: " << headingDeg << endl;
      }
    } else if (sscanf(toks[ii].c_str(), "\"head\":%lg", &headingDeg) == 1) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "==>> headingDeg: " << headingDeg << endl;
      }
    }
    
  } // ii

  if (useNow) {
    validTime.set(now);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "===========>> overriding with now time: " << DateTime::strm(validTime.utime()) << endl;
    }
  } else {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << endl << endl;
      cerr << "*********** using reported time ******************" << DateTime::strm(validTime.utime()) << endl;
      cerr << endl << endl;
    }
  }

  // trim the callsign to 4 chars

  if (callSign.size() > 4) {
    callSign = callSign.substr(callSign.size() - 4, 4);
  }

  ac_posn_wmod_t posn;
  MEM_zero(posn);
  memcpy(posn.callsign, callSign.c_str(), AC_POSN_N_CALLSIGN);
  memcpy(posn.text, "NcarAcPosn2Spdb JSON", AC_POSN_WMOD_N_TEXT);
  posn.lat = latitude;
  posn.lon = longitude;
  if (altitudeFt > -9990) {
    posn.alt = altitudeFt * FEET_TO_M;
  } else {
    posn.alt = -9999.0;
  }
  posn.headingDeg = headingDeg;
  posn.tas = AC_POSN_MISSING_FLOAT;
  posn.gs = AC_POSN_MISSING_FLOAT;
  posn.temp = AC_POSN_MISSING_FLOAT;
  posn.dew_pt = AC_POSN_MISSING_FLOAT;
  posn.lw = AC_POSN_MISSING_FLOAT;
  posn.fssp = AC_POSN_MISSING_FLOAT;
  posn.rosemount = AC_POSN_MISSING_FLOAT;

  // store in SPDB
  // dataType is set to hashed callsign
  // dataType2 is set to SPDB_AC_POSN_WMOD_ID
  
  si32 dataType = Spdb::hash4CharsToInt32(posn.callsign);
  BE_from_ac_posn_wmod(&posn);
  
  DsSpdb spdb;
  spdb.addPutChunk(dataType,
                   validTime.utime(),
                   validTime.utime() + _params.valid_period,
                   sizeof(ac_posn_wmod_t),
                   &posn,
                   SPDB_AC_POSN_WMOD_ID);

  if (spdb.put(_params.output_url,
               SPDB_AC_POSN_ID,
               SPDB_AC_POSN_LABEL)) {
    cerr << "ERROR - NcarAcPosn2Spdb" << endl;
    cerr << spdb.getErrStr() << endl;
  }
      
  
  return 0;


#ifdef JUNK
  
  // create DsSpdb object for output

  // DsSpdb spdb;
  // time_t prevPutTime = time(NULL);

  char line[BUFSIZ];
  while (_input->readLine(line, BUFSIZ) == 0) {

    PMU_auto_register("Reading data");

    if (line[strlen(line)-1] == '\n') {
      line[strlen(line)-1] = '\0';
    }

    if (_params.echo) {
      if (_params.new_line_on_echo) {
	fprintf(stdout, "%s\n", line);
      } else {
	fprintf(stdout, "  %s\r", line);
      }
      fflush(stdout);
    }

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "input line: " << line << endl;
    }
  
    int iret = 0;
    ac_posn_wmod_t posn;
    MEM_zero(posn);
    posn.lat = AC_POSN_MISSING_FLOAT;
    posn.lon = AC_POSN_MISSING_FLOAT;
    posn.alt = AC_POSN_MISSING_FLOAT;
    posn.tas = AC_POSN_MISSING_FLOAT;
    posn.gs = AC_POSN_MISSING_FLOAT;
    posn.temp = AC_POSN_MISSING_FLOAT;
    posn.dew_pt = AC_POSN_MISSING_FLOAT;
    posn.lw = AC_POSN_MISSING_FLOAT;
    posn.fssp = AC_POSN_MISSING_FLOAT;
    posn.rosemount = AC_POSN_MISSING_FLOAT;

    time_t validTime;

    switch (_params.input_format) {

      case Params::AUTOMATIC_FORMAT :
        if (strchr(line, ',') != NULL) {
          // new comma-delimited format
          iret = _decodeCommaDelimited(line, validTime, posn);
        } else if (line[2] == ':' && line[5] == ':') {
          // long fixed string format
          iret = _decodeUndString(line, validTime, posn);
        } else if (strlen(line) >= 78) {
          // long fixed string format
          iret = _decodeWmiLongString(line, validTime, posn);
        } else {
          // old  string format
          iret = _decodeWmiString(line, validTime, posn);
        }
        break;
        
      case Params::COMMA_DELIMITED_FORMAT :
        iret = _decodeCommaDelimited(line, validTime, posn);
        break;
        
      case Params::LONG_FIXED_STRING_FORMAT :
        iret = _decodeUndString(line, validTime, posn);
        break;
        
      case Params::WMI_LONG_STRING_FORMAT :
        iret = _decodeWmiLongString(line, validTime, posn);
        break;
        
      case Params::WMI_STRING_FORMAT :
        iret = _decodeWmiString(line, validTime, posn);
        break;
        
      case Params::SEA_M300_FORMAT :
        iret = _decodeSeaM300(line, validTime, posn);
        break;
        
      case Params::CSV_GPS_FORMAT :
        iret = _decodeCsvGps(line, validTime, posn);
        break;
        
      case Params::POLCAST2_FORMAT :
        iret = _decodePolcast2(line, validTime, posn);
        break;
        
      case Params::NOAA_AIRCRAFT_FORMAT :
        iret = _decodeNoaaAircraft(line, validTime, posn);
        break;
        
      case Params::NOAA_SHIP_FORMAT :
        iret = _decodeNoaaShip(line, validTime, posn);
        break;
        
    } /* endswitch - _params.input_format */

    if (_params.override_callsign) {
      STRncopy(posn.callsign, _params.callsign, AC_POSN_N_CALLSIGN);
    }

    if (_params.change_lat_sign) {
      posn.lat *= -1.0;
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA)
      ac_posn_wmod_print(stderr, "", &posn);
    
    if (iret == 0) {

      // check callsign

      if (!_acceptCallsign(posn)) {
	continue;
      }
      
      // success

      // compute the flare count

      _computeEjectFlares(posn, ejectMap);
      _computeBipFlares(validTime, posn, bipMap, burnDeq);

      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "----> Data at time: " << DateTime::str(validTime) << endl;
	ac_posn_wmod_print(stderr, "  ", &posn);
      }

      // store in SPDB
      // dataType is set to hashed callsign
      // dataType2 is set to SPDB_AC_POSN_WMOD_ID

      si32 dataType = Spdb::hash4CharsToInt32(posn.callsign);
      BE_from_ac_posn_wmod(&posn);
      
      spdb.addPutChunk(dataType,
		       validTime,
		       validTime + _params.valid_period,
		       sizeof(ac_posn_wmod_t),
		       &posn,
		       SPDB_AC_POSN_WMOD_ID);
      
      if (_params.output_ascii) {
	string asciiLine(line);
	asciiLine += "\n";
	asciiSpdb.addPutChunk(dataType,
			      validTime,
			      validTime + _params.valid_period,
			      asciiLine.size() + 1,
			      asciiLine.c_str(),
			 SPDB_AC_POSN_WMOD_ID);
      }

    } // if (iret == 0)

    // put data at constant intervals
    
    time_t now = time(NULL);
    int elapsed = now - prevPutTime;
    if (elapsed >= _params.output_interval) {
      
      if (spdb.put(_params.output_url,
		   SPDB_AC_POSN_ID,
		   SPDB_AC_POSN_LABEL)) {
	cerr << "ERROR - NcarAcPosn2Spdb" << endl;
	cerr << spdb.getErrStr() << endl;
      }
      
      if (_params.output_ascii) {
	if (asciiSpdb.put(_params.ascii_url,
			  SPDB_ASCII_ID,
			  SPDB_ASCII_LABEL)) {
	  cerr << "ERROR - NcarAcPosn2Spdb" << endl;
	  cerr << spdb.getErrStr() << endl;
	}
      }
      
      prevPutTime = now;
      spdb.clearPutChunks();
      asciiSpdb.clearPutChunks();

    } // if (elapsed)

  } // while

  // write out any remaining entries

  if (spdb.put(_params.output_url,
	       SPDB_AC_POSN_ID,
	       SPDB_AC_POSN_LABEL)) {
    cerr << "ERROR - NcarAcPosn2Spdb" << endl;
    cerr << spdb.getErrStr() << endl;
  }
      
  if (_params.output_ascii) {
    if (asciiSpdb.put(_params.ascii_url,
		      SPDB_ASCII_ID,
		      SPDB_ASCII_LABEL)) {
      cerr << "ERROR - NcarAcPosn2Spdb" << endl;
      cerr << spdb.getErrStr() << endl;
    }
  }

#endif

  return 0;

}

//////////////////////////////////////////////////
// Run in simulate mode

int NcarAcPosn2Spdb::_runSimMode()
{

  return 0;

}


