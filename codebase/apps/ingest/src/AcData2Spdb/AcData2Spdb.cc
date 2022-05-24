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
// AcData2Spdb.cc
//
// AcData2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2011
//
///////////////////////////////////////////////////////////////
//
// AcData2Spdb reads aircraft data from an ASCII string and
// stores it in Spdb.
//
/////////////////////////////////////////////////////////////

#include <iomanip>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Tty.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <physics/thermo.h>
#include <physics/physics.h>
#include "AcData2Spdb.hh"
using namespace std;

const float AcData2Spdb::POLCAST2_MISSING_VALUE = 999999.9999;


// Constructor

AcData2Spdb::AcData2Spdb(int argc, char **argv)
  
{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "AcData2Spdb";
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
    _input = new FileInput(_progName, _params);
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new FileInput(_progName, _params,
                           _args.startTime, _args.endTime);
  } else if (_params.mode == Params::FILELIST) {
    _input = new FileInput(_progName, _params, _args.inputFileList);
  } else if (_params.mode == Params::SERIAL) {
    _input = new SerialInput(_progName, _params);
  } else if (_params.mode == Params::TCP) {
    _input = new TcpInput(_progName, _params,
                          _params.tcp_server_host_name,
                          _params.tcp_server_port);
  } else if (_params.mode == Params::SIM) {
    _input = new SimInput(_progName, _params);
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

AcData2Spdb::~AcData2Spdb()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int AcData2Spdb::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // create DsSpdb object for output

  DsSpdb spdb, asciiSpdb;
  time_t prevPutTime = time(NULL);

  // initialize for KML decoding

  _kmlTimeValid = false;
  _kmlTime = 0;

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
        
      case Params::IWG1_FORMAT :
        iret = _decodeIWG1(line, validTime, posn);
        break;
        
      case Params::KML_FORMAT :
        iret = _decodeKML(line, validTime, posn);
        break;

    } /* endswitch - _params.input_format */

    if (_params.override_callsign) {
      STRncopy(posn.callsign, _params.callsign, AC_POSN_N_CALLSIGN);
    }

    if (_params.change_lat_sign) {
      posn.lat *= -1.0;
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      ac_posn_wmod_print(stderr, "", &posn);
    }
    
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
        cerr << "ERROR - AcData2Spdb" << endl;
        cerr << spdb.getErrStr() << endl;
      }
      
      if (_params.output_ascii) {
        if (asciiSpdb.put(_params.ascii_url,
                          SPDB_ASCII_ID,
                          SPDB_ASCII_LABEL)) {
          cerr << "ERROR - AcData2Spdb" << endl;
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
    cerr << "ERROR - AcData2Spdb" << endl;
    cerr << spdb.getErrStr() << endl;
  }
      
  if (_params.output_ascii) {
    if (asciiSpdb.put(_params.ascii_url,
                      SPDB_ASCII_ID,
                      SPDB_ASCII_LABEL)) {
      cerr << "ERROR - AcData2Spdb" << endl;
      cerr << spdb.getErrStr() << endl;
    }
  }

  return (0);

}

//////////////////////////////////////////////////
// decode new comma-delimited  format string

int AcData2Spdb::_decodeCommaDelimited(const char *line,
                                       time_t &validTime,
                                       ac_posn_wmod_t &posn)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "----> Decoding new comma delimited format" << endl;
  }
  
  // tokenize the line

  vector<string> toks;

  char copy[BUFSIZ];
  STRncopy(copy, line, BUFSIZ);
  char *last;
  char *token = strtok_r(copy, ",", &last);
  while (token != NULL) {
    toks.push_back(token);
    token = strtok_r(NULL, ",", &last);
  }

  if (toks.size() < 10) {
    cerr << "ERROR - " << _progName << " - _decodeCommaDelimited" << endl;
    cerr << "  Only: " << toks.size() << " in input line" << endl;
    cerr << "  Must have at least 10: " << endl;
    cerr << "    callsign,year,month,day,hour,min,sec,lat,lon,alt" << endl;
    cerr << "  line: " << line << endl;
    return -1;
  }

  // decode main 10 toks

  date_time_t gps_time;
  STRncopy(posn.callsign,toks[0].c_str(), AC_POSN_N_CALLSIGN);
  gps_time.year = atol(toks[1].c_str());
  gps_time.month = atol(toks[2].c_str());
  gps_time.day = atol(toks[3].c_str());
  gps_time.hour = atol(toks[4].c_str());
  gps_time.min = atol(toks[5].c_str());
  gps_time.sec = atol(toks[6].c_str());
  uconvert_to_utime(&gps_time);
  validTime = gps_time.unix_time;
  posn.lat =  atof(toks[7].c_str());
  posn.lon =  atof(toks[8].c_str());
  posn.alt =  atof(toks[9].c_str());

  // decode optional fields

  _tempC = -9999;
  _dewPtC = -9999;
  _rh = -9999;

  for (int ii = 0; ii < _params.comma_delimited_optional_fields_n; ii++) {

    const Params::optional_field_t &optional =
      _params._comma_delimited_optional_fields[ii];

    int index = optional.field_pos;
    if (index >= (int) toks.size()) {
      if (_params.debug) {
        cerr << "WARNING - " << _progName << " - _decodeCommaDelimited" << endl;
        cerr << "  Optional field pos too high: " << index << endl;
        cerr << "  Field type: " << _fieldType2Str(optional.field_type) << endl;
        cerr << "  Field will be ignored" << endl;
      }
      continue;
    }

    _loadOptionalField(optional, posn, toks, index);
    
  } // ii

  // compute dew pt from RH if available
  
  _computeDewPt();
  posn.dew_pt = _dewPtC;

  return 0;

}

//////////////////////////////////////////////////
// decode UND fixed format string

int AcData2Spdb::_decodeUndString(const char *line,
                                  time_t &validTime,
                                  ac_posn_wmod_t &posn)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---> Decoding UND fixed format" << endl;
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
  posn.lon = lon;
  posn.alt = alt;
  if (_params.valid_callsigns_n > 0) {
    STRncopy(posn.callsign, _params._valid_callsigns[0], AC_POSN_N_CALLSIGN);
  }
  posn.temp = temp;
  posn.dew_pt = dewpt;
  posn.lw = kingLwc;
  posn.fssp = fsspConc;
  posn.rosemount = rosemountVolts;

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Success with UND format" << endl;
    cerr << "pressure: " << pressure << endl;
    cerr << "rosemountVolts: " << rosemountVolts << endl;
  }

  return 0;
  
}

//////////////////////////////////////////////////
// decode old short WMI format string

int AcData2Spdb::_decodeWmiString(const char *line,
                                  time_t &validTime,
                                  ac_posn_wmod_t &posn)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---> Decoding old short WMI format" << endl;
  }

  char spaced_line[BUFSIZ];
  _addSpaces(line, spaced_line);

  if (_params.debug >= Params::DEBUG_EXTRA) {
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

int AcData2Spdb::_decodeWmiLongString(const char *line,
                                      time_t &validTime,
                                      ac_posn_wmod_t &posn)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---> Decoding long  format" << endl;
  }

  char spaced_line[BUFSIZ];
  _addSpacesLong(line, spaced_line);
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
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

int AcData2Spdb::_decodeSeaM300(const char *line,
                                time_t &validTime,
                                ac_posn_wmod_t &posn)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---> Decoding SEA M300 format" << endl;
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
  posn.lon = lon;
  posn.alt = alt_m;
  posn.temp = temperature;

  return 0;
  
}

//////////////////////////////////////////////////
// decode POLCAST2 format string

int AcData2Spdb::_decodePolcast2(const char *line,
                                 time_t &validTime,
                                 ac_posn_wmod_t &posn)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---> Decoding POLCAST2 format" << endl;
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
  
  if (lat == POLCAST2_MISSING_VALUE) {
    return -1;
  }
  posn.lat = lat;

  if (lon == POLCAST2_MISSING_VALUE) {
    return -1;
  }
  posn.lon = lon;

  if (alt_m == POLCAST2_MISSING_VALUE) {
    posn.alt = AC_POSN_MISSING_FLOAT;
  } else {
    posn.alt = alt_m;
  }

  STRncopy(posn.callsign, tail_num, AC_POSN_N_CALLSIGN);

  return 0;
  
}

//////////////////////////////////////////////////
// decode NOAA AIRCRAFT format string

int AcData2Spdb::_decodeNoaaAircraft(const char *line,
                                     time_t &validTime,
                                     ac_posn_wmod_t &posn)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---> Decoding NOAA AIRCRAFT format" << endl;
  }

  // parse the line
  
  string tail_num;
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  double lat;
  double lon;
  double alt_m;
  
  vector<string> toks;
  TaStr::tokenizeAllowEmpty(line, ',', toks);
  if (toks.size() < 5) {
    return -1;
  }

  tail_num = toks[0];
  
  if (sscanf(toks[1].c_str(), "%4d%2d%2dT%2d%2d%2d",
             &year, &month, &day, &hour, &minute, &second) != 6) {
    return -1;
  }
  
  if (sscanf(toks[2].c_str(), "%lg", &lat) != 1) {
    return -1;
  }

  if (sscanf(toks[3].c_str(), "%lg", &lon) != 1) {
    return -1;
  }
  
  if (sscanf(toks[4].c_str(), "%lg", &alt_m) != 1) {
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
  posn.lon = lon;
  posn.alt = alt_m;

  int nCopy = AC_POSN_N_CALLSIGN;
  if (tail_num.size() < AC_POSN_N_CALLSIGN) {
    nCopy = tail_num.size() + 1;
  }
  STRncopy(posn.callsign, tail_num.c_str(), nCopy);

  // decode optional fields

  _tempC = -9999;
  _dewPtC = -9999;
  _rh = -9999;

  for (int ii = 0; ii < _params.noaa_aircraft_optional_fields_n; ii++) {

    const Params::optional_field_t &optional =
      _params._noaa_aircraft_optional_fields[ii];

    int index = optional.field_pos;
    if (index >= (int) toks.size()) {
      if (_params.debug) {
        cerr << "WARNING - " << _progName << " - _decodeNoaaAircraft" << endl;
        cerr << "  Optional field pos too high: " << index << endl;
        cerr << "  Field type: " << _fieldType2Str(optional.field_type) << endl;
        cerr << "  Field will be ignored" << endl;
      }
      continue;
    }
    
    _loadOptionalField(optional, posn, toks, index);
    
  } // ii

  // compute dew pt from RH if available
  
  _computeDewPt();
  posn.dew_pt = _dewPtC;

  return 0;
  
}

//////////////////////////////////////////////////
// decode NOAA SHIP format string

int AcData2Spdb::_decodeNoaaShip(const char *line,
                                 time_t &validTime,
                                 ac_posn_wmod_t &posn)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---> Decoding NOAA SHIP format" << endl;
  }

  // parse the line
  
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  double lat;
  double lon;
  
  vector<string> toks;
  string lineCopy(line);
  TaStr::tokenizeAllowEmpty(lineCopy, ',', toks);
  if (toks.size() < 6) {
    return -1;
  }

  if (sscanf(toks[0].c_str(), "%4d%2d%2d",
             &year, &month, &day) != 3) {
    return -1;
  }
  
  if (sscanf(toks[1].c_str(), "%2d%2d",
             &hour, &minute) != 2) {
    return -1;
  }
  second = 0;
  
  if (sscanf(toks[2].c_str(), "%lg", &lat) != 1) {
    return -1;
  }

  if (sscanf(toks[4].c_str(), "%lg", &lon) != 1) {
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

  // convert lat and lon

  int ilat = (int) (lat / 100.0);
  double latMinutes = lat - ilat * 100;
  lat = ilat + latMinutes / 60.0;
  if (toks[3] == "S") {
    lat *= -1.0;
  }
  
  int ilon = (int) (lon / 100.0);
  double lonMinutes = lon - ilon * 100;
  lon = ilon + lonMinutes / 60.0;
  if (toks[5] == "W") {
    lon *= -1.0;
  }
  
  // load struct
  
  posn.lat = lat;
  posn.lon = lon;
  posn.alt = 0.0;

  return 0;
  
}

//////////////////////////////////////////////////
// decode IWG1 format

int AcData2Spdb::_decodeIWG1(const char *line,
                             time_t &validTime,
                             ac_posn_wmod_t &posn)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---> Decoding IWG1 format" << endl;
  }

  // split the comma-delimited line
  
  vector<string> toks;
  TaStr::tokenizeAllowEmpty(line, ',', toks);
  if (toks.size() < 7) {
    return -1;
  }

  // remove IWG1 token if present at start of message

  if (toks[0] == "IWG1") {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "===>> IWG1 line: " << line << endl;
      cerr << "===>> removing first token: IWG1" << endl;
    }
    toks.erase(toks.begin(), toks.begin()+1);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---> IWG1 line: " << line << endl;
    cerr << "     ntoks: " << toks.size() << endl;
    for (size_t ii = 0; ii < toks.size(); ii++) {
      cerr << "tok[" << ii << "]=" << toks[ii];
      if (ii == toks.size() - 1) {
        cerr << endl;
      } else {
        cerr << ",";
      }
    }
  }
  
  // time

  DateTime dtime;
  if (dtime.setFromW3c(toks[0].c_str())) {
    cerr << "ERROR - AcData2Spdb::_decodeIWG1" << endl;
    cerr << "  Cannot decode ISO time, token[1]: " << toks[1] << endl;
    cerr << "  line: " << line << endl;
    return -1;
  }
  validTime = dtime.utime();
  
  // lat, lon

  double lat = -9999.0;
  double lon = -9999.0;

  if (toks[1].size() > 0) {
    lat = atof(toks[1].c_str());
  }
  if (toks[2].size() > 0) {
    lon = atof(toks[2].c_str());
  }
  
  // altitude

  double altM = -9999.0;
  if (toks[3].size() > 0) {
    altM = atof(toks[3].c_str());
  } else if (toks[4].size() > 0) {
    altM = atof(toks[4].c_str());
  } else if (toks[5].size() > 0) {
    altM = atof(toks[5].c_str()) * 0.3048;
  }

  // speed
  
  if (toks.size() < 9) {
    return 0;
  }

  double groundSpeedMps = -9999.0;
  double tasMps = -9999.0;

  if (toks[7].size() > 0) {
    groundSpeedMps = atof(toks[7].c_str());
  }
  if (toks[8].size() > 0) {
    tasMps = atof(toks[8].c_str());
  }

  // heading
  
  if (toks.size() < 13) {
    return 0;
  }

  double headingDeg = -9999.0;

  if (toks[12].size() > 0) {
    headingDeg = atof(toks[12].c_str());
  }

  // temp, dewpt
  
  if (toks.size() < 21) {
    return 0;
  }

  double tempC = -9999.0;
  double dewptC = -9999.0;

  if (toks[19].size() > 0) {
    tempC = atof(toks[19].c_str());
  }
  if (toks[20].size() > 0) {
    dewptC = atof(toks[20].c_str());
  }

  // double rh = -9999.0;
  // if (tempC > -9990.0 && dewptC > -9990.0) {
  //   rh = PHYrelh(tempC, dewptC);
  // }

  // wind

  if (toks.size() < 27) {
    return 0;
  }

  double windSpeedMps = -9999.0;
  double windDirnDegT = -9999.0;

  if (toks[25].size() > 0) {
    windSpeedMps = atof(toks[25].c_str());
  }
  if (toks[26].size() > 0) {
    windDirnDegT = atof(toks[26].c_str());
  }

  // double uu = -9999.0;
  // double vv = -9999.0;
  
  // if (windSpeedMps > -9990.0 && windDirnDegT > -9990.0) {
  //   uu = PHYwind_u(windSpeedMps, windDirnDegT);
  //   vv = PHYwind_v(windSpeedMps, windDirnDegT);
  // }
  
  // load struct

  STRncopy(posn.callsign, "IWG1", AC_POSN_N_CALLSIGN);
  if (_params.override_callsign) {
    STRncopy(posn.callsign, _params.callsign, AC_POSN_N_CALLSIGN);
  }

  posn.lat = lat;
  posn.lon = lon;
  posn.alt = altM;

  posn.tas = tasMps;
  posn.gs = groundSpeedMps;
  posn.temp = tempC;
  posn.dew_pt = dewptC;
  posn.headingDeg = headingDeg;

  posn.lw = windSpeedMps;
  posn.fssp = windDirnDegT;
  
  return 0;
  
}

/////////////////////////////////////////////////////////////////
// decode CSV FORMAT 1 - South Africa Weather Service data system
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

int AcData2Spdb::_decodeCsvGps(const char *line,
                               time_t &validTime,
                               ac_posn_wmod_t &posn)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---> Decoding CSV GPS format" << endl;
  }

  // tokenize the line on commas, since it is comma-delimited

  vector<string> toks;
  TaStr::tokenizeAllowEmpty(line, ',', toks);
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
  posn.lon = longitude;
  posn.alt = antennaAltitudeM;
  
  if (_params.valid_callsigns_n > 0) {
    STRncopy(posn.callsign, _params._valid_callsigns[0], AC_POSN_N_CALLSIGN);
  }

  return 0;

}
  
/////////////////////////////////////////////////////////////////
// decode KML
//
// Because of the nature of XML, this proceeds on a multi-line basis

int AcData2Spdb::_decodeKML(const char *line,
                            time_t &validTime,
                            ac_posn_wmod_t &posn)
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "---> Decoding KML format" << endl;
  }

  // are we in a track?

  if (strstr(line, "<gx:Track>") != NULL) {
    _kmlInTrack = true;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "AcData2Spdb::_decodeKML() - start of Track" << endl;
    }
    return -1; // no position yet
  }
  if (strstr(line, "</gx:Track>") != NULL) {
    _kmlInTrack = false;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "AcData2Spdb::_decodeKML() - end of Track" << endl;
    }
    return -1; // no position yet
  }
  if (!_kmlInTrack) {
    // not yet in a track block
    return -1;
  }

  // get time - this is on a separate line
  
  if (strstr(line, "<when>") != NULL) {
    time_t tval;
    if (TaXml::readTime(line, "when", tval) == 0) {
      _kmlTimeValid = true;
      _kmlTime = tval;
    }
    return -1; // no position yet
  }
  if (!_kmlTimeValid) {
    // no time yet
    return -1;
  }
  
  // get position - this is on a separate line

  if (strstr(line, "<gx:coord>") != NULL) {
    string posStr;
    if (TaXml::readString(line, "gx:coord", posStr) == 0) {
      // got position string
      double lon, lat, alt;
      if (sscanf(posStr.c_str(), "%lg %lg %lg", &lon, &lat, &alt) == 3) {
        // success on reading pos
        validTime = _kmlTime;
        posn.lat = lat;
        posn.lon = lon;
        posn.alt = alt;
        STRncopy(posn.callsign, _params.callsign, AC_POSN_N_CALLSIGN);
        _kmlTimeValid = false;
        return 0;
      }
    }
  }
  
  // still looking

  return -1;

}
  
//////////////////////////////////////////////////
// add spaces to the input line

void AcData2Spdb::_addSpaces (const char *line,
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

void AcData2Spdb::_addSpacesLong (const char *line,
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

bool AcData2Spdb::_acceptCallsign(ac_posn_wmod_t &posn)
  
{
  
  if (_params.check_callsigns) {
    bool in_list = false;
    for (int i = 0; i < _params.valid_callsigns_n; i++) {
      if (!strcmp(posn.callsign, _params._valid_callsigns[i])) {
        in_list = true;
        break;
      }
    }
    if (!in_list) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
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

void AcData2Spdb::_computeEjectFlares(ac_posn_wmod_t &posn,
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
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
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

void AcData2Spdb::_computeBipFlares(time_t validTime,
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
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
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

  if (_params.debug >= Params::DEBUG_EXTRA && burnDeq.size() > 0) {
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

  if (_params.debug >= Params::DEBUG_VERBOSE && burnDeq.size() > 0) {
    cerr << "Flares bip for " << callsign << " : " << count << endl;
  }

  // set the number for this time to the count

  posn.n_burn_in_place = count;

}

////////////////////////////
// get string for field type

string AcData2Spdb::_fieldType2Str(int fieldType)

{

  switch (fieldType) {

    case Params::PRESSURE_HPA:
      return "PRESSURE_HPA";
    case Params::GS_KTS:
      return "GS_KTS";
    case Params::TAS_KTS:
      return "TAS_KTS";
    case Params::TEMP_C:
      return "TEMP_C";
    case Params::DEW_PT_C:
      return "DEW_PT_C";
    case Params::RH_PERCENT:
      return "RH_PERCENT";
    case Params::LW_G_PER_M3:
      return "LW_G_PER_M3";
    case Params::ROSEMOUNT_TOTAL_TEMP_C:
      return "ROSEMOUNT_TOTAL_TEMP_C";
    case Params::FSSP_CONC_G_PER_M3:
      return "FSSP_CONC_G_PER_M3";
    case Params::FLARE_BURN_L_FLAG:
      return "FLARE_BURN_L_FLAG";
    case Params::FLARE_BURN_R_FLAG:
      return "FLARE_BURN_R_FLAG";
    case Params::FLARE_BURN_IN_PLACE_FLAG:
      return "FLARE_BURN_IN_PLACE_FLAG";
    case Params::FLARE_EJECTABLE_FLAG:
      return "FLARE_EJECTABLE_FLAG";
    case Params::FLARE_N_BURN_IN_PLACE:
      return "FLARE_N_BURN_IN_PLACE";
    case Params::FLARE_N_EJECTABLE:
      return "FLARE_N_EJECTABLE";
    case Params::ERROR_FLAGS:
      return "ERROR_FLAGS";
    default:
      return "UNKNOWN";

  } // switch

}

////////////////////////////
// load up optional field

void AcData2Spdb::_loadOptionalField(const Params::optional_field_t &optional,
                                     ac_posn_wmod_t &posn,
                                     const vector<string> &toks,
                                     int index)
  
{

  switch (optional.field_type) {
    
    case Params::PRESSURE_HPA:
      break;
    case Params::GS_KTS:
      posn.gs = atof(toks[index].c_str());
      break;
    case Params::TAS_KTS:
      posn.tas = atof(toks[index].c_str());
      break;
    case Params::TEMP_C:
      _tempC = atof(toks[index].c_str());
      posn.temp = _tempC;
      break;
    case Params::DEW_PT_C:
      _dewPtC = atof(toks[index].c_str());
      posn.dew_pt = _dewPtC;
      break;
    case Params::RH_PERCENT:
      _rh = atof(toks[index].c_str());
      break;
    case Params::LW_G_PER_M3:
      posn.lw = atof(toks[index].c_str());
      break;
    case Params::ERROR_FLAGS: {}
      break;
    case Params::FLARE_BURN_R_FLAG:
      if (toks[index] == "1") {
        posn.flare_flags |= RIGHT_BURN_FLAG;
      }
      break;
    case Params::FLARE_BURN_L_FLAG:
      if (toks[index] == "1") {
        posn.flare_flags |= LEFT_BURN_FLAG;
      }
      break;
    case Params::FLARE_BURN_IN_PLACE_FLAG:
      if (toks[index] == "1") {
        posn.flare_flags |= BURN_IN_PLACE_FLAG;
      }
      break;
    case Params::FLARE_EJECTABLE_FLAG:
      if (toks[index] == "1") {
        posn.flare_flags |= EJECTABLE_FLAG;
      }
      break;
    case Params::FLARE_N_BURN_IN_PLACE:
      posn.n_burn_in_place = atoi(toks[index].c_str());
      break;
    case Params::FLARE_N_EJECTABLE:
      posn.n_ejectable = atoi(toks[index].c_str());
      break;
      
    default: {}
      
  } // switch
  
}

////////////////////////////
// compute dewpt if possible

void AcData2Spdb::_computeDewPt()
  
{

  if (_dewPtC < -9990) {
    // dew pt not set
    if (_tempC > -9990 && _rh > -9990) {
      _dewPtC= PHYrhdp(_tempC, _rh);
    }
  }

}

