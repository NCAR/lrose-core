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
// WxStn2Table.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2017
//
///////////////////////////////////////////////////////////////
//
// WxStn2Table reads surface station entries from an SPDB
// data base, and based on the specified parameters in the
// param file, reformats these into a text table.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <toolsa/TaXml.hh>
#include <toolsa/TaStr.hh>
#include <rapformats/WxObs.hh>
#include "WxStn2Table.hh"

using namespace std;

// Constructor

WxStn2Table::WxStn2Table(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "WxStn2Table";
  
  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // set start and end times

  _startTime.set(_params.start_time.year,
                 _params.start_time.month,
                 _params.start_time.day,
                 _params.start_time.hour,
                 _params.start_time.min,
                 _params.start_time.sec);
  
  _endTime.set(_params.end_time.year,
               _params.end_time.month,
               _params.end_time.day,
               _params.end_time.hour,
               _params.end_time.min,
               _params.end_time.sec);

  // set station names

  _stationNames.clear();
  string names(_params.station_names);
  if (names.size() != 0 && names != "all") {
    TaStr::tokenize(names, ",", _stationNames);
  }
  
  if (_params.debug) {
    cerr << "WxStn2Table running with the following params:" << endl;
    cerr << "  start time: " << DateTime::strm(_startTime.utime()) << endl;
    cerr << "  end   time: " << DateTime::strm(_endTime.utime()) << endl;
    if (_stationNames.size() > 0) {
      cerr << "  station names:" << endl;
      for (size_t ii = 0; ii < _stationNames.size(); ii++) {
        cerr << "    _stationNames[ii]" << endl;
      }
    }
    if (_params.apply_bounding_box) {
      cerr << "  bounding_box:" << endl;
      cerr << "    min_lon: " << _params.bounding_box.min_lon << endl;
      cerr << "    max_lon: " << _params.bounding_box.max_lon << endl;
      cerr << "    min_lat: " << _params.bounding_box.min_lat << endl;
      cerr << "    max_lat: " << _params.bounding_box.max_lat << endl;
    }
  }

}

// destructor

WxStn2Table::~WxStn2Table()

{

}

//////////////////////////////////////////////////
// Run

int WxStn2Table::Run ()
{

  _lineCount = 0;

  DsSpdb spdb;

  if (spdb.getInterval(_params.input_url,
                       _startTime.utime(),
                       _endTime.utime(),
                       0, 0)) {
    cerr << "ERROR - WxStn2Table::Run" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }

  // add comment lines is appropriate

  if (_params.add_commented_header) {
    _printComments(stdout);
  }

  // get chunks
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  if (_params.debug) {
    cerr << "==>> got n entries: " << chunks.size() << endl;
  }
  for (size_t ii = 0; ii < chunks.size(); ii++) {
    _printLine(stdout, chunks[ii]);
  }

  return 0;
  
}

//////////////////////////////////////////////////
// print comments at start

void WxStn2Table::_printComments(FILE *out)
{

  const char *com = _params.comment_character;

  // initial line has column headers
  
  fprintf(out, "%s", com);
  for (int ii = 0; ii < _params.fields_n; ii++) {
    fprintf(out, "%s", _params.column_delimiter);
    switch (_params._fields[ii]) {
      case Params::YEAR: fprintf(out, "year"); break;
      case Params::MONTH: fprintf(out, "month"); break;
      case Params::DAY: fprintf(out, "day"); break;
      case Params::HOUR: fprintf(out, "hour"); break;
      case Params::MIN: fprintf(out, "min"); break;
      case Params::SEC: fprintf(out, "sec"); break;
      case Params::UNIX_TIME_SECS: fprintf(out, "unix_time_secs"); break;
      case Params::UNIX_TIME_DAYS: fprintf(out, "unix_time_days"); break;
      case Params::LATITUDE: fprintf(out, "latitude"); break;
      case Params::LONGITUDE: fprintf(out, "longitude"); break;
      case Params::ALTITUDE_M: fprintf(out, "altitude_m"); break;
      case Params::TEMP_C: fprintf(out, "temp_c"); break;
      case Params::DEWPT_C: fprintf(out, "dewpt_c"); break;
      case Params::RH_PERCENT: fprintf(out, "rh_percent"); break;
      case Params::WIND_DIRN_DEGT: fprintf(out, "wind_dirn_degt"); break;
      case Params::WIND_SPEED_MPS: fprintf(out, "wind_speed_mps"); break;
      case Params::WIND_GUST_MPS: fprintf(out, "wind_gust_mps"); break;
      case Params::PRESSURE_HPA: fprintf(out, "pressure_hpa"); break;
      case Params::LIQUID_ACCUM_MM: fprintf(out, "liquid_accum_mm"); break;
      case Params::PRECIP_RATE_MM_PER_HR: fprintf(out, "precip_rate_mm_per_hr"); break;
      case Params::VISIBILITY_KM: fprintf(out, "visibility_km"); break;
      case Params::RVR_KM: fprintf(out, "rvr_km"); break;
      case Params::CEILING_M: fprintf(out, "ceiling_m"); break;
    }
  } // ii
  fprintf(out, "\n");

  // then comment lines

  fprintf(out, "%s============================================\n", com);
  fprintf(out, "%s  Table produced by WxStn2Table\n", com);
  fprintf(out, "%s  url: %s\n", com, _params.input_url);
  fprintf(out, "%s  start_time: %s\n", com,
          DateTime::strm(_startTime.utime()).c_str());
  fprintf(out, "%s  end_time: %s\n", com,
          DateTime::strm(_endTime.utime()).c_str());
  fprintf(out, "%s  station_names: %s\n", com,
          _params.station_names);
  if (_params.apply_bounding_box) {
    fprintf(out, "%s  bounding_box:\n", com);
    fprintf(out, "%s    min_lon: %lg\n", com, _params.bounding_box.min_lon);
    fprintf(out, "%s    max_lon: %lg\n", com, _params.bounding_box.max_lon);
    fprintf(out, "%s    min_lat: %lg\n", com, _params.bounding_box.min_lat);
    fprintf(out, "%s    max_lat: %lg\n", com, _params.bounding_box.max_lat);
  }
  fprintf(out, "%s============================================\n", com);

}

//////////////////////////////////////////////////
// print a line of data

void WxStn2Table::_printLine(FILE *out,
                             const Spdb::chunk_t &chunk)
  
{

  if (chunk.data == NULL) {
    return;
  }

  // decode chunk

  time_t valid_time = chunk.valid_time;
  string stationName;
  double lat, lon, elevM;
  if (WxObs::disassembleStationDetails(chunk.data,
                                       chunk.len,
                                       stationName, lat, lon, elevM)) {
    return;
  }

  // condition the longitude
  
  if (lon < -180.0) {
    lon += 360.0;
  } else if (lon > 180.0) {
    lon -= 360.0;
  }

  // compute times
  
  DateTime vtime(valid_time);
  time_t utime = vtime.utime();
  double uday = (double) utime / 86400.0;

  // check station name

  if (_stationNames.size() > 0) {
    bool found = false;
    for (size_t ii = 0; ii < _stationNames.size(); ii++) {
      if (stationName == _stationNames[ii]) {
        found = true;
        break;
      }
    }
    if (!found) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Discarding station based on name: " << stationName << endl;
      }
      return;
    }
  }

  // check bounding box
  
  if (_params.apply_bounding_box) {
    if (lon < _params.bounding_box.min_lon ||
        lon > _params.bounding_box.max_lon) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Discarding station based on longitude: "
             << stationName << ", " << lon << endl;
      }
      return;
    }
    if (lat < _params.bounding_box.min_lat ||
        lat > _params.bounding_box.max_lat) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Discarding station based on latitude: "
             << stationName << ", " << lat << endl;
      }
      return;
    }
  }

  // write output line
  
  WxObs obs;
  obs.disassemble(chunk.data, chunk.len);

  for (int ii = 0; ii < _params.fields_n; ii++) {
    switch (_params._fields[ii]) {
      case Params::YEAR: fprintf(out, "%d", vtime.getYear()); break;
      case Params::MONTH: fprintf(out, "%d", vtime.getMonth()); break;
      case Params::DAY: fprintf(out, "%d", vtime.getDay()); break;
      case Params::HOUR: fprintf(out, "%d", vtime.getHour()); break;
      case Params::MIN: fprintf(out, "%d", vtime.getMin()); break;
      case Params::SEC: fprintf(out, "%d", vtime.getSec()); break;
      case Params::UNIX_TIME_SECS: fprintf(out, "%ld", utime); break;
      case Params::UNIX_TIME_DAYS: fprintf(out, "%lg", uday); break;
      case Params::LATITUDE: fprintf(out, "%lg", lat); break;
      case Params::LONGITUDE: fprintf(out, "%lg", lon); break;
      case Params::ALTITUDE_M: fprintf(out, "%lg", elevM); break;
      case Params::TEMP_C: fprintf(out, "%lg", obs.getTempC()); break;
      case Params::DEWPT_C: fprintf(out, "%lg", obs.getDewpointC()); break;
      case Params::RH_PERCENT: fprintf(out, "%lg", obs.getRhPercent()); break;
      case Params::WIND_DIRN_DEGT: fprintf(out, "%lg", obs.getWindDirnDegt()); break;
      case Params::WIND_SPEED_MPS: fprintf(out, "%lg", obs.getWindSpeedMps()); break;
      case Params::WIND_GUST_MPS: fprintf(out, "%lg", obs.getWindGustMps()); break;
      case Params::PRESSURE_HPA: fprintf(out, "%lg", obs.getSeaLevelPressureMb()); break;
      case Params::LIQUID_ACCUM_MM: fprintf(out, "%lg", obs.getPrecipLiquidMm()); break;
      case Params::PRECIP_RATE_MM_PER_HR: fprintf(out, "%lg", obs.getPrecipRateMmPerHr()); break;
      case Params::VISIBILITY_KM: fprintf(out, "%lg", obs.getVisibilityKm()); break;
      case Params::RVR_KM: fprintf(out, "%lg", obs.getRvrKm()); break;
      case Params::CEILING_M: fprintf(out, "%lg", obs.getCeilingKm()); break;
    }
    if (ii < _params.fields_n - 1) {
      fprintf(out, "%s", _params.column_delimiter);
    }
  } // ii
  fprintf(out, "\n");
  _lineCount++;

  return;

}

