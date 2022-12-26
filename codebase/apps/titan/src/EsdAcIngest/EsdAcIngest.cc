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
// EsdAcIngest.cc
//
// EsdAcIngest object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2000
//
///////////////////////////////////////////////////////////////
//
// EsdAcIngest reads aircraft data from an ASCII string and
// stores it in Spdb.
//
/////////////////////////////////////////////////////////////

#include <iomanip>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Tty.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include "EsdAcIngest.hh"
using namespace std;

const float EsdAcIngest::POLCAST2_MISSING_VALUE = 999999.9999;


// Constructor

EsdAcIngest::EsdAcIngest(int argc, char **argv)
  
{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "EsdAcIngest";
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

  // create input object

  if (_params.mode == Params::REALTIME) {
    _input = new TtyInput(_progName, _params);
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new FilelistInput(_progName, _params, _args.inputFileList);
  } else if (_params.mode == Params::TCP) {
    _input = new TcpInput(_progName, _params,
                          _params.tcp_server_host_name,
                          _params.tcp_server_port);
  } else if (_params.mode == Params::TEST) {
    _input = new TestInput(_progName, _params);
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

EsdAcIngest::~EsdAcIngest()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int EsdAcIngest::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // create DsSpdb object for output

  DsSpdb spdb, asciiSpdb;
  time_t prevPutTime = time(NULL);

  // create maps for flare counts

  flare_count_map_t bipMap;
  flare_count_map_t ejectMap;
  burn_count_deque_t burnDeq;

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

    if (_params.debug) {
      cerr << ">> " << line << " <<" << endl;
    }

    int iret = 0;
    ac_posn_wmod_t posn;
    ac_posn_wmod_init(&posn);
 
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
        
    } /* endswitch - _params.input_format */
    
    if (_params.debug >= Params::DEBUG_VERBOSE)
      ac_posn_wmod_print(stderr, "", &posn);
    
    if (iret == 0) {

      // check callsign
      
      if (!_acceptCallsign(posn)) {
	continue;
      }

      // check for valid speeds
      
      if (_params.check_ground_speed) {
        if (posn.gs < _params.min_valid_ground_speed) {
          continue;
        }
      }
      
      if (_params.check_air_speed) {
        if (posn.tas < _params.min_valid_air_speed) {
          continue;
        }
      }

      // compute dew point if applicable
      
      // success

      // compute the flare count

      _computeEjectFlares(posn, ejectMap);
      _computeBipFlares(validTime, posn, bipMap, burnDeq);
      
      if (_params.debug) {
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
	cerr << "ERROR - EsdAcIngest" << endl;
	cerr << spdb.getErrStr() << endl;
      }
      
      if (_params.output_ascii) {
	if (asciiSpdb.put(_params.ascii_url,
			  SPDB_ASCII_ID,
			  SPDB_ASCII_LABEL)) {
	  cerr << "ERROR - EsdAcIngest" << endl;
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
    cerr << "ERROR - EsdAcIngest" << endl;
    cerr << spdb.getErrStr() << endl;
  }
      
  if (_params.output_ascii) {
    if (asciiSpdb.put(_params.ascii_url,
		      SPDB_ASCII_ID,
		      SPDB_ASCII_LABEL)) {
      cerr << "ERROR - EsdAcIngest" << endl;
      cerr << spdb.getErrStr() << endl;
    }
  }

  return (0);

}

//////////////////////////////////////////////////
// decode new comma-delimited  format string

int EsdAcIngest::_decodeCommaDelimited(const char *line,
				       time_t &validTime,
				       ac_posn_wmod_t &posn)

{

  if (_params.debug) {
    cerr << "----> Decoding new comma delimited format" << endl;
  }
  
  // tokenize the line

  vector<string> tokens;

  char copy[BUFSIZ];
  STRncopy(copy, line, BUFSIZ);
  char *last;
  char *token = strtok_r(copy, ",", &last);
  while (token != NULL) {
    tokens.push_back(token);
    token = strtok_r(NULL, ",", &last);
  }

  if (tokens.size() < 10) {
    cerr << "ERROR - " << _progName << " - _decodeCommaDelimited" << endl;
    cerr << "  Only: " << tokens.size() << " in input line" << endl;
    cerr << "  Must have at least 10: " << endl;
    cerr << "    callsign,year,month,day,hour,min,sec,lat,lon,alt" << endl;
    cerr << "  line: " << line << endl;
    return -1;
  }

  // decode main 10 tokens

  date_time_t gps_time;
  STRncopy(posn.callsign,tokens[0].c_str(), AC_POSN_N_CALLSIGN);
  gps_time.year = atol(tokens[1].c_str());
  gps_time.month = atol(tokens[2].c_str());
  gps_time.day = atol(tokens[3].c_str());
  gps_time.hour = atol(tokens[4].c_str());
  gps_time.min = atol(tokens[5].c_str());
  gps_time.sec = atol(tokens[6].c_str());
  uconvert_to_utime(&gps_time);
  validTime = gps_time.unix_time;
  posn.lat =  atof(tokens[7].c_str());
  if (_params.change_lat_sign) {
    posn.lat *= -1.0;
  }
  posn.lon =  atof(tokens[8].c_str());
  posn.alt =  atof(tokens[9].c_str());

  // check number of tokens

  int nTokensExpected = _params.optional_field_names_n + 10;
  if ((int) tokens.size() < nTokensExpected) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "---------------------------------------------------" << endl;
      cerr << "WARNING - " << _progName << " - _decodeCommaDelimited" << endl;
      cerr << "  Param file indicates "
	   << _params.optional_field_names_n << " optional fields." << endl;
      cerr << "  Therefore " << nTokensExpected << " tokens expected." << endl;
      cerr << "  Only " << tokens.size() << " tokens found." << endl;
      cerr << "---------------------------------------------------" << endl;
    }
  }
  int nEnd = MIN(nTokensExpected, ((int) tokens.size()));

  // decode remaining tokens

  for (int ii = 10; ii < nEnd; ii++) {
    
    switch (_params._optional_field_names[ii - 10]) {
      
      case Params::GS:
        posn.gs = atof(tokens[ii].c_str());
        break;
      case Params::TAS:
        posn.tas = atof(tokens[ii].c_str());
        break;
      case Params::TEMP:
        posn.temp = atof(tokens[ii].c_str());
        break;
      case Params::DEW_PT:
        posn.dew_pt = atof(tokens[ii].c_str());
        break;
      case Params::LW:
        posn.lw = atof(tokens[ii].c_str());
        break;
      case Params::FSSP_CONC:
        posn.fssp = atof(tokens[ii].c_str());
        break;
      case Params::HEADING_DEG:
        posn.headingDeg = atof(tokens[ii].c_str());
        break;
      case Params::VERT_VEL_MPS:
        posn.vertVelMps = atof(tokens[ii].c_str());
        break;
      case Params::ERROR_FLAGS: {}
        break;
      case Params::R_BURN:
        if (tokens[ii] == "1") {
          posn.flare_flags |= RIGHT_BURN_FLAG;
        }
        break;
      case Params::L_BURN:
        if (tokens[ii] == "1") {
          posn.flare_flags |= LEFT_BURN_FLAG;
        }
        break;
      case Params::BURN_IN_PLACE:
        if (tokens[ii] == "1") {
          posn.flare_flags |= BURN_IN_PLACE_FLAG;
        }
        break;
      case Params::DRY_ICE:
        if (tokens[ii] == "1") {
          posn.flare_flags |= DRY_ICE_FLAG;
        }
        break;
      case Params::EJECTABLE:
        if (tokens[ii] == "1") {
          posn.flare_flags |= EJECTABLE_FLAG;
        }
        break;
      case Params::N_BURN_IN_PLACE:
        posn.n_burn_in_place = atoi(tokens[ii].c_str());
        break;
      case Params::N_EJECTABLE:
        posn.n_ejectable = atoi(tokens[ii].c_str());
        break;
        
      default: {}
        
    } // switch
    
  } // ii

  return 0;

}

//////////////////////////////////////////////////
// decode UND fixed format string

int EsdAcIngest::_decodeUndString(const char *line,
				  time_t &validTime,
				  ac_posn_wmod_t &posn)
  
{

  if (_params.debug) {
    cerr << "---> Decoding UND fixed format" << endl;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "input line: " << line << endl;
  }
  
  date_time_t gps_time;
  double pressure, temp, dewpt;
  double wspd, wdirn;
  double lat, lon, alt;
  double kingLwc, fsspConc, rosemountVolts;

  if (sscanf(line,
	     "%2d:%2d:%2d %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg",
	     &gps_time.hour, &gps_time.min, &gps_time.sec,
	     &pressure, &temp, &dewpt,
	     &wspd, &wdirn,
	     &lon, &lat, &alt,
	     &kingLwc, &fsspConc, &rosemountVolts) != 14) {
    return -1;
  }

  // get date from wallclock time, adjust gps time

  date_time_t now;
  now.unix_time = time(NULL);
  uconvert_from_utime(&now);
  gps_time.year = now.year;
  gps_time.month = now.month;
  gps_time.day = now.day;
  uconvert_to_utime(&gps_time);
  
  if ((gps_time.unix_time - now.unix_time) > 43200) {
    gps_time.unix_time -= 86400;
  } else if ((now.unix_time - gps_time.unix_time) > 43200) {
    gps_time.unix_time += 86400;
  }
  
  validTime = gps_time.unix_time;
  
  // load struct
  
  posn.lat = lat;
  if (_params.change_lat_sign) {
    posn.lat *= -1.0;
  }
  posn.lon = lon;
  posn.alt = alt;
  if (_params.callsigns_n > 0) {
    STRncopy(posn.callsign, _params._callsigns[0], AC_POSN_N_CALLSIGN);
  }
  posn.temp = temp;
  posn.dew_pt = dewpt;
  posn.lw = kingLwc;
  posn.fssp = fsspConc;
  posn.rosemount = rosemountVolts;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Success with UND format" << endl;
    cerr << "pressure: " << pressure << endl;
    cerr << "rosemountVolts: " << rosemountVolts << endl;
  }

  return 0;
  
}

//////////////////////////////////////////////////
// decode old short WMI format string

int EsdAcIngest::_decodeWmiString(const char *line,
				  time_t &validTime,
				  ac_posn_wmod_t &posn)
  
{

  if (_params.debug) {
    cerr << "---> Decoding old short WMI format" << endl;
  }

  char spaced_line[BUFSIZ];
  _addSpaces(line, spaced_line);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "spaced_line: " << spaced_line << endl;
  }

  char callsign[8];
  char dummy1[8], dummy2[8];
  int ilat, ilon;
  int itas, igps_alt, ipres_alt;
  int itdry, ilwjw;
  int left_burn, right_burn;
  int burn_in_place, n_burn_in_place;
  int ejectable, n_ejectable;
  int igps_var, igps_err, iarnav_warn;

  date_time_t gps_time;
  
  if (sscanf(spaced_line,
	     "%s %d %d %d %d %d "
	     "%d %d %s "
	     "%d %d %d %d "
	     "%s %d %d "
	     "%d %d %d "
	     "%d %d %d ",
	     callsign, &ilat, &ilon, &itas, &igps_alt, &ipres_alt,
	     &itdry, &ilwjw, dummy1,
	     &n_burn_in_place, &n_ejectable, &right_burn, &left_burn,
	     dummy2, &burn_in_place, &ejectable,
	     &gps_time.hour, &gps_time.min, &gps_time.sec,
	     &igps_var, &igps_err, &iarnav_warn) != 22) {
    return -1;
  }

  // get date from wallclock time, adjust gps time

  date_time_t now;
  now.unix_time = time(NULL);
  uconvert_from_utime(&now);
  gps_time.year = now.year;
  gps_time.month = now.month;
  gps_time.day = now.day;
  uconvert_to_utime(&gps_time);

  if ((gps_time.unix_time - now.unix_time) > 43200) {
    gps_time.unix_time -= 86400;
  } else if ((now.unix_time - gps_time.unix_time) > 43200) {
    gps_time.unix_time += 86400;
  }
  
  validTime = gps_time.unix_time;
  
  // load struct
  
  posn.lat = ilat / 10000.0;
  if (_params.change_lat_sign) {
    posn.lat *= -1.0;
  }
  posn.lon = ilon / 10000.0;
  posn.alt = igps_alt / 100.0;
  STRncopy(posn.callsign, callsign, AC_POSN_N_CALLSIGN);
  posn.tas = itas;
  posn.temp = itdry / 10.0;
  posn.lw = ilwjw / 100.0;
  if (right_burn) {
    posn.flare_flags |= RIGHT_BURN_FLAG;
  }
  if (left_burn) {
    posn.flare_flags |= LEFT_BURN_FLAG;
  }
  if (burn_in_place) {
    posn.flare_flags |= BURN_IN_PLACE_FLAG;
  }
  if (ejectable) {
    posn.flare_flags |= EJECTABLE_FLAG;
  }
  posn.n_ejectable = n_ejectable;
  posn.n_burn_in_place = n_burn_in_place;
  
  return 0;
  
}

//////////////////////////////////////////////////
// decode new long WMI format string

int EsdAcIngest::_decodeWmiLongString(const char *line,
				      time_t &validTime,
				      ac_posn_wmod_t &posn)
  
{
  
  if (_params.debug) {
    cerr << "---> Decoding long  format" << endl;
  }

  char spaced_line[BUFSIZ];
  _addSpacesLong(line, spaced_line);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "spaced_line: " << spaced_line << endl;
  }

  // parse the line

  char callsign[8];
  int ilat, ilon;
  int itas, igps_alt, ipres_alt;
  int itdry, ilwjw, idewpt, ifssp;
  int burn_in_place, n_burn_in_place;
  int ejectable, n_ejectable;
  int left_burn, right_burn, ice_hopper;
  char gps_time_str[16];
  int igps_var, igps_err, igps_warn;
  
  if (sscanf(spaced_line,
	     "%s %d %d %d %d %d "
	     "%d %d %d %d "
	     "%d %d %d %d "
	     "%d %d %d "
	     "%s "
	     "%d %d %d ",
	     callsign, &ilat, &ilon, &itas, &igps_alt, &ipres_alt,
	     &itdry, &ilwjw, &idewpt, &ifssp,
	     &burn_in_place, &n_burn_in_place, &ejectable, &n_ejectable,
	     &left_burn, &right_burn, &ice_hopper,
	     gps_time_str,
	     &igps_var, &igps_err, &igps_warn) != 21) {
    return -1;
  }

  // get date from wallclock time, adjust gps time

  date_time_t now;
  now.unix_time = time(NULL);
  uconvert_from_utime(&now);

  int hour, min, sec;
  if (sscanf(gps_time_str, "%2d%2d%2d", &hour, &min, &sec) == 3) {

    date_time_t gps_time;
    gps_time.year = now.year;
    gps_time.month = now.month;
    gps_time.day = now.day;
    gps_time.hour = hour;
    gps_time.min = min;
    gps_time.sec = sec;
    uconvert_to_utime(&gps_time);
    
    if ((gps_time.unix_time - now.unix_time) > 43200) {
      gps_time.unix_time -= 86400;
    } else if ((now.unix_time - gps_time.unix_time) > 43200) {
      gps_time.unix_time += 86400;
    }
  
    validTime = gps_time.unix_time;

  } else {

    validTime = now.unix_time;

  }
  
  // load struct
  
  posn.lat = ilat / 10000.0;
  if (_params.change_lat_sign) {
    posn.lat *= -1.0;
  }
  posn.lon = ilon / 10000.0;
  if (igps_alt == 0) {
    posn.alt = ipres_alt * 30.48;
  } else {
    posn.alt = igps_alt / 100.0;
  }
  STRncopy(posn.callsign, callsign, AC_POSN_N_CALLSIGN);
  posn.tas = itas;
  posn.temp = itdry / 10.0;
  posn.lw = ilwjw / 100.0;
  posn.dew_pt = idewpt / 10.0;

  if (right_burn) {
    posn.flare_flags |= RIGHT_BURN_FLAG;
  }
  if (left_burn) {
    posn.flare_flags |= LEFT_BURN_FLAG;
  }
  if (burn_in_place) {
    posn.flare_flags |= BURN_IN_PLACE_FLAG;
  }
  if (ejectable) {
    posn.flare_flags |= EJECTABLE_FLAG;
  }
  if (ice_hopper) {
    posn.flare_flags |= DRY_ICE_FLAG;
  }
  posn.n_ejectable = n_ejectable;
  posn.n_burn_in_place = n_burn_in_place;
  
  return 0;
  
}

//////////////////////////////////////////////////
// decode SEA M300 format string

int EsdAcIngest::_decodeSeaM300(const char *line,
				time_t &validTime,
				ac_posn_wmod_t &posn)
  
{
  
  if (_params.debug) {
    cerr << "---> Decoding SEA M300 format" << endl;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "input line: " << line << endl;
  }
  
  // parse the line

  int hour;
  int minute;
  int second;
  int month;
  int day;
  int year;
  float lat;
  float lon;
  float alt_ft;
  float alt_m;
  float temperature;
  float rh;
  
  if (sscanf(line,
	     "%d:%d:%d %d/%d/%d %f %f %f %f %f %f ",
	     &hour, &minute, &second, &month, &day, &year,
	     &lat, &lon, &alt_ft, &alt_m, &temperature, &rh) != 12) {
    return -1;
  }

  // Convert the valid time

  date_time_t valid_time;
  
  valid_time.year = year;
  valid_time.month = month;
  valid_time.day = day;
  valid_time.hour = hour;
  valid_time.min = minute;
  valid_time.sec = second;
  uconvert_to_utime(&valid_time);
  
  validTime = valid_time.unix_time;
  
  // load struct
  
  posn.lat = lat;
  if (_params.change_lat_sign) {
    posn.lat *= -1.0;
  }
  posn.lon = lon;
  posn.alt = alt_m;
  posn.temp = temperature;

  return 0;
  
}

//////////////////////////////////////////////////
// decode POLCAST2 format string

int EsdAcIngest::_decodePolcast2(const char *line,
				 time_t &validTime,
				 ac_posn_wmod_t &posn)
  
{
  
  if (_params.debug) {
    cerr << "---> Decoding POLCAST2 format" << endl;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "input line: " << line << endl;
  }
  
  // parse the line

  char tail_num[20];
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  float lat;
  float lon;
  float alt_m;
  float ground_speed;
  float voltage;
  
  if (sscanf(line,
	     "%s %d %d %d %d %d %d %f %f %f %f %f ",
	     tail_num, &year, &month, &day, &hour, &minute, &second,
	     &lat, &lon, &alt_m, &ground_speed, &voltage) != 12) {
    return -1;
  }

  // Convert the valid time

  date_time_t valid_time;
  
  valid_time.year = year;
  valid_time.month = month;
  valid_time.day = day;
  valid_time.hour = hour;
  valid_time.min = minute;
  valid_time.sec = second;
  uconvert_to_utime(&valid_time);
  
  validTime = valid_time.unix_time;
  
  // load struct
  
  if (lat == POLCAST2_MISSING_VALUE)
  {
    posn.lat = AC_POSN_MISSING_FLOAT;
  }
  else
  {
    posn.lat = lat;
    if (_params.change_lat_sign)
      posn.lat *= -1.0;
  }

  if (lon == POLCAST2_MISSING_VALUE)
    posn.lon = AC_POSN_MISSING_FLOAT;
  else
    posn.lon = lon;

  if (alt_m == POLCAST2_MISSING_VALUE)
    posn.alt = AC_POSN_MISSING_FLOAT;
  else
    posn.alt = alt_m;

  STRncopy(posn.callsign, tail_num, AC_POSN_N_CALLSIGN);

  return 0;
  
}

/////////////////////////////////////////////////////////////////
// decode CSV FORMAT 1 - South Africa Weayther Service data system
//
// Data line should contain the following comma-delited list
//   Time
//   GPS UTC time
//   Latitude
//   Longitude
//   quality
//   satellites
//   horiz precision
//   Antenna altitude(m)
//   Geoidal speration(m)
//   age of deff correction
//   defer station ID
//   Checksum
//   Status

int EsdAcIngest::_decodeCsvGps(const char *line,
                               time_t &validTime,
                               ac_posn_wmod_t &posn)
  
{
  
  if (_params.debug) {
    cerr << "---> Decoding CSV GPS format" << endl;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "input line: " << line << endl;
  }

  // tokenize the line on commas, since it is comma-delimited

  vector<string> toks;
  TaStr::tokenize(line, ",", toks);
  if (toks.size() < 9) {
    return -1;
  }

  // parse the tokens

  double dtimeSecs;
  if (sscanf(toks[0].c_str(), "%lg", &dtimeSecs) != 1) {
    return -1;
  }

  double latitude;
  if (sscanf(toks[2].c_str(), "%lg", &latitude) != 1) {
    return -1;
  }

  double longitude;
  if (sscanf(toks[3].c_str(), "%lg", &longitude) != 1) {
    return -1;
  }

  double antennaAltitudeM;
  if (sscanf(toks[7].c_str(), "%lg", &antennaAltitudeM) != 1) {
    return -1;
  }

  // get the reference time, from the input data
  // this is a time close to the data time
  
  time_t refTime = _input->getRefTime();

  // the date is not included in the data, so we have to guess
  // and then see if it is close to the reference time

  int iday = refTime / SECS_IN_DAY;
  double timeGuess = (double) iday * SECS_IN_DAY + dtimeSecs;
  if ((timeGuess - refTime) > SECS_IN_DAY / 2) {
    timeGuess -= SECS_IN_DAY;
  } else if ((refTime - timeGuess) > SECS_IN_DAY / 2) {
    timeGuess += SECS_IN_DAY;
  }
  
  DateTime vtime((time_t) (timeGuess + 0.5));
  validTime = vtime.utime();
  
  // load struct
  
  posn.lat = latitude;
  if (_params.change_lat_sign) {
    posn.lat *= -1.0;
  }
  posn.lon = longitude;
  posn.alt = antennaAltitudeM;
  
  if (_params.callsigns_n > 0) {
    STRncopy(posn.callsign, _params._callsigns[0], AC_POSN_N_CALLSIGN);
  }

  return 0;

}
  
//////////////////////////////////////////////////
// add spaces to the input line

void EsdAcIngest::_addSpaces (const char *line,
			      char *spaced_line)
{

  int fieldLen[] =
  {4, 7, 8, 3, 4, 4, 4, 3, 2, 2, 2, 1, 1, 2, 1, 1, 3, 3, 2, 4, 3, 4, -1};

  int pos = 0;
  int spaced_pos = 0;

  int ifield = 0;
  while (fieldLen[ifield] > 0) {

    for (int i = 0; i < fieldLen[ifield]; i++) {
      if (line[pos] != ':') {
	spaced_line[spaced_pos] = line[pos];
	spaced_pos++;
      }
      pos++;
    }

    spaced_line[spaced_pos] = ' ';
    spaced_pos++;
    ifield++;
    
  }
  
  spaced_line[spaced_pos] = '\0';


}

//////////////////////////////////////////////////
// add spaces to the input line
// Long WMI fixed format, added dewpoint etc.

void EsdAcIngest::_addSpacesLong (const char *line,
				  char *spaced_line)
{

  int fieldLen[] =
  {4, 7, 8, 3, 4, 4, 4, 3, 4, 4, 1, 4, 1, 4, 1, 1, 1, 8, 5, 3, 4, -1};

  int pos = 0;
  int spaced_pos = 0;

  int ifield = 0;
  while (fieldLen[ifield] > 0) {

    for (int i = 0; i < fieldLen[ifield]; i++) {
      if (line[pos] != ':') {
	spaced_line[spaced_pos] = line[pos];
	spaced_pos++;
      }
      pos++;
    }

    spaced_line[spaced_pos] = ' ';
    spaced_pos++;
    ifield++;
    
  }
  
  spaced_line[spaced_pos] = '\0';


}

///////////////////////////////////////////////////////
// check callsign

bool EsdAcIngest::_acceptCallsign(ac_posn_wmod_t &posn)
  
{
  
  if (_params.check_callsigns) {
    bool in_list = false;
    for (int i = 0; i < _params.callsigns_n; i++) {
      if (!strcmp(posn.callsign, _params._callsigns[i])) {
	in_list = true;
	break;
      }
    }
    if (!in_list) {
      if (_params.debug) {
	cerr << "Note: rejecting callsign: " << posn.callsign << endl;
      }
      return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////
// Compute the ejectable flare count.
// Keep track of the number of flares which are fired
// between data arrivals

void EsdAcIngest::_computeEjectFlares(ac_posn_wmod_t &posn,
				      flare_count_map_t &ejectMap)

{

  // get the previous count

  int nprev;
  count_iter ii;
  ii = ejectMap.find(posn.callsign);
  if (ii == ejectMap.end()) {
    nprev = posn.n_ejectable;
  } else {
    nprev = (*ii).second;
    ejectMap.erase(ii);
  }

  // compute the difference

  int delta = posn.n_ejectable - nprev;
  if (delta < 0) {
    delta = 0; // in case count wraps
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "----> " << posn.callsign << endl;
    cerr << "  prev ejectMap: " << nprev << endl;
    cerr << "  now  ejectMap: " << posn.n_ejectable << endl;
    cerr << "  eject delta      : " << delta << endl;
  }

  // store the current count for future use

  flare_pair_t pp;
  pp.first = posn.callsign;
  pp.second = posn.n_ejectable;
  ejectMap.insert(pp);

  // set the number for this time to the difference
  
  posn.n_ejectable = delta;
  
}

///////////////////////////////////////////////////////
// Compute the burn-in-place flare count,
// and keep track of how many burn-in-place flares are
// burning for each aircraft.

void EsdAcIngest::_computeBipFlares(time_t validTime,
				    ac_posn_wmod_t &posn,
				    flare_count_map_t &bipMap,
				    burn_count_deque_t &burnDeq)
  
{

  // get the previous count

  string callsign = posn.callsign;
  int nprev;
  count_iter ii;
  ii = bipMap.find(callsign);
  if (ii == bipMap.end()) {
    nprev = posn.n_burn_in_place;
  } else {
    nprev = (*ii).second;
    bipMap.erase(ii);
  }

  // compute the change in count

  int delta = posn.n_burn_in_place - nprev;
  if (delta < 0) {
    delta = 0;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "----> " << callsign << endl;
    cerr << "  prev bipMap: " << nprev << endl;
    cerr << "  now  bipMap: " << posn.n_burn_in_place << endl;
    cerr << "  bip delta      : " << delta << endl;
  }

  // store the current count for the future

  flare_pair_t pp;
  pp.first = callsign;
  pp.second = posn.n_burn_in_place;
  bipMap.insert(pp);

  // compute the expire time for flares started now

  time_t expireTime = validTime + _params.burn_in_place_time;
  burn_pair_t bb;
  bb.first = expireTime;
  bb.second = callsign;

  // delete old items from the back of the queue

  while (burnDeq.size() > 0 && burnDeq.back().first < validTime) {
    burnDeq.pop_back();
  }

  // add one entry to the queue for each flare started now

  for (int jj = 0; jj < delta; jj++) {
    burnDeq.push_front(bb);
  }

  // print the queue

  if (_params.debug >= Params::DEBUG_VERBOSE && burnDeq.size() > 0) {
    cerr << "======== Queue for burn-in_place flares ======" << endl;
    cerr << "Expire time : callsign" << endl;
    for (size_t jj = 0; jj < burnDeq.size(); jj++) {
      cerr << DateTime::str(burnDeq[jj].first) << ": "
	   << burnDeq[jj].second << endl;
    }
    cerr << "==============================================" << endl;
  }

  // count up the number of flares burning for this callsign

  int count = 0;
  for (size_t kk = 0; kk < burnDeq.size(); kk++) {
    if (burnDeq[kk].second == callsign) {
      count++;
    }
  }

  if (_params.debug && burnDeq.size() > 0) {
    cerr << "Flares bip for " << callsign << " : " << count << endl;
  }

  // set the number for this time to the count

  posn.n_burn_in_place = count;

}

#ifdef JUNK

// old WMI format

3144+469242-096806000000000000+000000+0000000000000000000018:13:39+04800001024
3144+469242-096806000000000000+000000+0000000000000000000018:13:41+04800001024
3144+469242-096806000000000000+000000+0000000000000000000018:13:43+04800001024
3144+469242-096806000000000000+000000+0000000000000000000018:13:44+04800001024
3144+469242-096806000000000000+000000+0000000000000000000018:13:46+04800001024
3144+469242-096806000000000000+000000+0000000000000000000018:13:48+04800001024

// UND format

17:26:39  981.8   5.6 -31.3 296.8   7.5    0.0000    0.0000     0.0  -1.00    0.00   2.489
17:26:40  981.8   5.6 -31.4 296.6   7.5    0.0000    0.0000     0.0  -1.00    0.00   2.489
17:26:41  981.8   5.7 -31.4 296.7   7.5    0.0000    0.0000     0.0  -1.00    0.00   2.489
17:26:42  981.8   5.7 -31.4 296.7   7.5    0.0000    0.0000     0.0  -1.00    0.00   2.489
17:26:43  981.8   5.7 -31.4 296.6   7.5    0.0000    0.0000     0.0  -1.00    0.00   2.489


#endif

