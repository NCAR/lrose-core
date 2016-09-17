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
// IwrfAcGeorefCompare.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2016
//
///////////////////////////////////////////////////////////////
//
// AcGeorefCompare reads multiple ac georef data sets from SPDB
// and compares them. It is designed to compare the NCAR GV INS
// with the HCR Cmigits unit.
//
////////////////////////////////////////////////////////////////

#include "AcGeorefCompare.hh"
#include <iostream>
#include <iomanip>
#include <cerrno>
#include <unistd.h>
#include <ctime>
#include <toolsa/toolsa_macros.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/DateTime.hh>
#include <rapformats/ac_georef.hh>

double AcGeorefCompare::_missingDbl = -9999.0;

using namespace std;

// Constructor

AcGeorefCompare::AcGeorefCompare(int argc, char **argv)
  
{

  isOK = true;
  _lineCount = 0;

  // set programe name
  
  _progName = "AcGeorefCompare";

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
  
  // init process mapper registration
  
  if (_params.reg_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  // check command line args

  _startTime = DateTime::NEVER;
  _endTime = DateTime::NEVER;
  
  _startTime = DateTime::parseDateTime(_params.start_time);
  if (_params.product_type == Params::TIME_SERIES_TABLE ||
      _params.product_type == Params::SINGLE_PERIOD_ARCHIVE) {
    if (_startTime == DateTime::NEVER) {
      cerr << "ERROR - AcGeorefCompare" << endl;
      cerr << "  For product type TIME_SERIES_TABLE or SINGLE_PERIOD_ARCHIVE\n" << endl;
      cerr << "  you must specify start time using '-start' on the command line" << endl;
      isOK = false;
    }
  }
  _endTime = DateTime::parseDateTime(_params.end_time);
  if (_params.product_type == Params::TIME_SERIES_TABLE) {
    if (_endTime == DateTime::NEVER) {
      cerr << "ERROR - AcGeorefCompare" << endl;
      cerr << "  For product type TIME_SERIES_TABLE\n" << endl;
      cerr << "  you must specify end time using '-end' on the command line" << endl;
      isOK = false;
    }
  }

  // initialize aircraft weight computations

  double lbPerKg = 2.20462;
  _takeoffWtKg = _params.takeoff_weight_lbs / lbPerKg;
  _aircraftWtKg = _takeoffWtKg;
  _fuelRateClimbKgPerSec = 
    (_params.fuel_burn_rate_initial_climb / lbPerKg) / 3600.0;
  _fuelRateCruiseKgPerSec =
    (_params.mean_fuel_burn_rate_cruise / lbPerKg) / 3600.0;
  _topOfClimbAltitudeM = _params.top_of_climb_altitude_ft * 0.3048;
  _inInitialClimb = true;
  _initialClimbSecs = 0.0;
  
}

// destructor

AcGeorefCompare::~AcGeorefCompare()

{

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int AcGeorefCompare::Run ()
{

  if (_params.product_type == Params::TIME_SERIES_TABLE) {
    return _runTimeSeriesTable();
  } else if (_params.product_type == Params::SINGLE_PERIOD_ARCHIVE) {
    return _runSinglePeriodArchive();
  } else if (_params.product_type == Params::SINGLE_PERIOD_REALTIME) {
    return _runSinglePeriodRealtime();
  } else {
    return -1;
  }

}

//////////////////////////////////////////////////
// Run in time series table mode

int AcGeorefCompare::_runTimeSeriesTable()
{

  int iret = 0;

  // analyze data in time blocks

  time_t blockStartTime = _startTime;
  time_t blockEndTime = blockStartTime + _params.time_block_secs;

  while (blockStartTime < _endTime) {

    if (blockEndTime > _endTime) {
      blockEndTime = _endTime;
    }
    
    if (_retrieveTimeBlock(blockStartTime, blockEndTime) == 0) {
      if (_produceTimeSeriesTable()) {
        iret = -1;
      }
    } else {
      iret = -1;
    }

    blockStartTime += _params.time_block_secs;
    blockEndTime = blockStartTime + _params.time_block_secs;

  } // while
  
  return iret;

}

//////////////////////////////////////////////////
// Run in single period archive mode

int AcGeorefCompare::_runSinglePeriodArchive()
{

  // get the data

  time_t endTime = _startTime + _params.period_secs;
  
  if (_retrieveTimeBlock(_startTime, endTime)) {
    cerr << "ERROR - AcGeorefCompare::_runSinglePeriodArchive()" << endl;
    cerr << "  Cannot retieve data for period:" << endl;
    cerr << "    start time: " << DateTime::strm(_startTime) << endl;
    cerr << "    end   time: " << DateTime::strm(endTime) << endl;
    return -1;
  }

  if (_produceSinglePeriodProduct()) {
    cerr << "ERROR - AcGeorefCompare::_runSinglePeriodArchive()" << endl;
    cerr << "  Cannot produce single period product" << endl;
    cerr << "    start time: " << DateTime::strm(_startTime) << endl;
    cerr << "    end   time: " << DateTime::strm(endTime) << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// Run in single period realtime mode

int AcGeorefCompare::_runSinglePeriodRealtime()
{
  cerr << "_runSinglePeriodRealtime() not yet implemented" << endl;
  return 0;
}


//////////////////////////////////////////////////
// Retrieve data for a block of time

int AcGeorefCompare::_retrieveTimeBlock(time_t startTime, time_t endTime)
{

  if (_params.debug) {
    cerr << "====>> Retrieving time block <<====" << endl;
    cerr << "  startTime: " << DateTime::strm(startTime) << endl;
    cerr << "  endTime: " << DateTime::strm(endTime) << endl;
  }

  // read primary URL

  DsSpdb spdbPrim;
  
  if(spdbPrim.getInterval(_params.primary_spdb_url,
                          startTime, endTime) < 0) {
    cerr << "ERROR - AcGeorefCompare::_retrieveTimeBlock" << endl;
    cerr << "  Cannot read data from primary URL: " << _params.primary_spdb_url << endl;
    return -1;
  }
  if (spdbPrim.getProdId() != SPDB_AC_GEOREF_ID) {
    cerr << "ERROR - AcGeorefCompare::_retrieveTimeBlock" << endl;
    cerr << "  Primary URL does not hold ac_georef data" << endl;
    cerr << "  Product found: " << spdbPrim.getProdLabel() << endl;
    return -1;
  }

  const vector<Spdb::chunk_t> &chunksPrim = spdbPrim.getChunks();
  size_t nChunksPrim = chunksPrim.size();
  
  // store the primary chunks

  _georefsPrimary.clear();
  ac_georef_t prevPrim;
  double psecs = 1.0 / _params.primary_frequency_hz;

  for(size_t iprim = 0; iprim < nChunksPrim; iprim++) {

    // check for correct chunk len

    if (chunksPrim[iprim].len != sizeof(ac_georef_t)) {
      cerr << "WARNING - AcGeorefCompare::_retrieveTimeBlock" << endl;
      cerr << "  Bad chunk length found in primary data: " << chunksPrim[iprim].len << endl;
      cerr << "  Should be: " << sizeof(ac_georef_t) << endl;
      cerr << "  Ignoring chunk" << endl;
      continue;
    }
    
    // decode chunk data

    ac_georef_t georefPrim(*((ac_georef_t *) chunksPrim[iprim].data));
    BE_to_ac_georef(&georefPrim);

    // check for sufficient time between observations

    bool useChunk = false;
    double deltaSecs = 0.0;
    if (iprim == 0) {
      useChunk = true;
    } else {
      deltaSecs = (double) georefPrim.time_secs_utc - (double) prevPrim.time_secs_utc;
      if (deltaSecs > 0.99 * psecs) {
        useChunk = true;
      }
    }

    if (useChunk) {
      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "=========== FOUND PRIMARY GEOREF ==========" << endl;
        ac_georef_print(stderr, "", &georefPrim);
        cerr << "===========================================" << endl;
      }
      _georefsPrimary.push_back(georefPrim);
      prevPrim = georefPrim;
    } else {
      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "Time interval between chunks too short: " << deltaSecs << endl;
        cerr << "  Discarding chunk at time: "
             << DateTime::strm(georefPrim.time_secs_utc) << endl;
      }
    }

  } // iprim
  
  if (_georefsPrimary.size() < 1) {
    if (_params.debug) {
      cerr << "==>> no primary georefs found for this time period" << endl;
    }
    return -1;
  }

  // read secondary URL

  DsSpdb spdbSec;
  
  if(spdbSec.getInterval(_params.secondary_spdb_url,
                         startTime, endTime) < 0) {
    cerr << "ERROR - AcGeorefCompare::_retrieveTimeBlock" << endl;
    cerr << "  Cannot read data from secondary URL: " << _params.secondary_spdb_url << endl;
    return -1;
  }
  if (spdbSec.getProdId() != SPDB_AC_GEOREF_ID) {
    cerr << "ERROR - AcGeorefCompare::_retrieveTimeBlock" << endl;
    cerr << "  Secondary URL does not hold ac_georef data" << endl;
    cerr << "  Product found: " << spdbSec.getProdLabel() << endl;
    return -1;
  }
  
  const vector<Spdb::chunk_t> &chunksSec = spdbSec.getChunks();
  size_t nChunksSec = chunksSec.size();

  // store the secondary chunks
  
  _georefsSecondary.clear();
  for(size_t isec = 0; isec < nChunksSec; isec++) {

    // check for correct chunk len

    if (chunksSec[isec].len != sizeof(ac_georef_t)) {
      cerr << "WARNING - AcGeorefCompare::_retrieveTimeBlock" << endl;
      cerr << "  Bad chunk length found in secondary data: " << chunksSec[isec].len << endl;
      cerr << "  Should be: " << sizeof(ac_georef_t) << endl;
      cerr << "  Ignoring chunk" << endl;
      continue;
    }

    // decode chunk data

    ac_georef_t georefSec(*((ac_georef_t *) chunksSec[isec].data));
    BE_to_ac_georef(&georefSec);
    _georefsSecondary.push_back(georefSec);

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "=========== SECONDARY GEOREF ==========" << endl;
      ac_georef_print(stderr, "", &georefSec);
      cerr << "=====================================" << endl;
    }
    
  } // isec

  if (_georefsSecondary.size() < 1) {
    if (_params.debug) {
      cerr << "==>> no secondary georefs found for this time period" << endl;
    }
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////
// Produce a time series table, for data previously retrieved

int AcGeorefCompare::_produceTimeSeriesTable()
{

  // loop through the primary chunks

  size_t isecLoc = 0;
  for (size_t iprim = 0; iprim < _georefsPrimary.size(); iprim++) {

    const ac_georef_t &georefPrim = _georefsPrimary[iprim];
    DateTime timePrim(georefPrim.time_secs_utc,
                      true,
                      georefPrim.time_nano_secs / 1.0e9);

    // find the closest secondary georef in time

    double timeDiff = 1.0e99;
    for (size_t isec = isecLoc; isec < _georefsSecondary.size(); isec++) {
      
      const ac_georef_t &georefSec = _georefsSecondary[isec];
      DateTime timeSec(georefSec.time_secs_utc,
                       true,
                       georefSec.time_nano_secs / 1.0e9);
      double tDiff = fabs(timeSec - timePrim);
      if (tDiff < timeDiff) {
        isecLoc = isec;
        timeDiff = tDiff;
      } else {
        // difference increasing, break out
        break;
      }
      
    } // isec

    if (timeDiff > _params.max_time_diff_secs) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Time difference secs too great: " << timeDiff << endl;
        cerr << "  Ignoring this pair" << endl;
      }
      continue;
    }

    const ac_georef_t &georefSec = _georefsSecondary[isecLoc];
    
    // analyze georefs

    _analyzeGeorefPair(georefPrim, georefSec, timePrim, timeDiff);

  } // iprim
  
  return 0;

}

///////////////////////////////////////////////////////////////
// Produce a single period product

int AcGeorefCompare::_produceSinglePeriodProduct()
{

  // loop through the primary chunks

  size_t isecLoc = 0;
  for (size_t iprim = 0; iprim < _georefsPrimary.size(); iprim++) {

    const ac_georef_t &georefPrim = _georefsPrimary[iprim];
    DateTime timePrim(georefPrim.time_secs_utc,
                      true,
                      georefPrim.time_nano_secs / 1.0e9);

    // find the closest secondary georef in time

    double timeDiff = 1.0e99;
    for (size_t isec = isecLoc; isec < _georefsSecondary.size(); isec++) {
      
      const ac_georef_t &georefSec = _georefsSecondary[isec];
      DateTime timeSec(georefSec.time_secs_utc,
                       true,
                       georefSec.time_nano_secs / 1.0e9);
      double tDiff = fabs(timeSec - timePrim);
      if (tDiff < timeDiff) {
        isecLoc = isec;
        timeDiff = tDiff;
      } else {
        // difference increasing, break out
        break;
      }
      
    } // isec

    if (timeDiff > _params.max_time_diff_secs) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Time difference secs too great: " << timeDiff << endl;
        cerr << "  Ignoring this pair" << endl;
      }
      continue;
    }

    const ac_georef_t &georefSec = _georefsSecondary[isecLoc];
    
    // analyze georefs

    _analyzeGeorefPair(georefPrim, georefSec, timePrim, timeDiff);

  } // iprim
  
  return 0;

}

//////////////////////////////////////////////////////
// Compute a mean of a vector of georefs observations

void AcGeorefCompare::_computeMeanGeorefs(const vector<ac_georef_t> &vals,
                                          ac_georef_t &mean)
  
{

  ac_georef_t sum;
  MEM_zero(mean);
  MEM_zero(sum);
  double count = 0.0;

  if (vals.size() < 1) {
    return;
  }

  for (size_t ii = 0; ii < vals.size(); ii++) {
    const ac_georef_t &georef = vals[ii];
    sum.time_secs_utc += georef.time_secs_utc;
    sum.time_nano_secs += georef.time_nano_secs;
    sum.longitude += georef.longitude;
    sum.latitude += georef.latitude;
    sum.altitude_msl_km += georef.altitude_msl_km;
    sum.ew_velocity_mps += georef.ew_velocity_mps;
    sum.ns_velocity_mps += georef.ns_velocity_mps;
    sum.vert_velocity_mps += georef.vert_velocity_mps;
    sum.heading_deg += georef.heading_deg;
    sum.drift_angle_deg += georef.drift_angle_deg;
    sum.track_deg += georef.track_deg;
    sum.roll_deg += georef.roll_deg;
    sum.pitch_deg += georef.pitch_deg;
    sum.temp_c += georef.temp_c;
    for (size_t ii = 0; ii < AC_GEOREF_N_CUSTOM; ii++) {
      sum.custom[ii] += georef.custom[ii];
    }
    count++;
  }
  
  mean.time_secs_utc = sum.time_secs_utc / count;
  mean.time_nano_secs = sum.time_nano_secs / count;
  mean.longitude = sum.longitude / count;
  mean.latitude = sum.latitude / count;
  mean.altitude_msl_km = sum.altitude_msl_km / count;
  mean.ew_velocity_mps = sum.ew_velocity_mps / count;
  mean.ns_velocity_mps = sum.ns_velocity_mps / count;
  mean.vert_velocity_mps = sum.vert_velocity_mps / count;
  mean.heading_deg = sum.heading_deg / count;
  mean.drift_angle_deg = sum.drift_angle_deg / count;
  mean.track_deg = sum.track_deg / count;
  mean.roll_deg = sum.roll_deg / count;
  mean.pitch_deg = sum.pitch_deg / count;
  mean.temp_c = sum.temp_c / count;
  for (size_t ii = 0; ii < AC_GEOREF_N_CUSTOM; ii++) {
    mean.custom[ii] = sum.custom[ii] / count;
  }

}
  
//////////////////////////////////////////////////
// Analyze georef pair for differences

void AcGeorefCompare::_analyzeGeorefPair(const ac_georef_t &georefPrim,
                                         const ac_georef_t &georefSec,
                                         const DateTime &timePrim,
                                         double timeDiff)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "+++++++++++ GEOREF PAIR IN TIME +++++++++++" << endl;
    cerr << "Time diff (secs): " << timeDiff << endl;
    cerr << "++++++++++++++ PRIMARY GEOREF +++++++++++++" << endl;
    ac_georef_print(stderr, "", &georefPrim);
    cerr << "+++++++++++++++++++++++++++++++++++++++++++" << endl;
    cerr << "+++++++++++++ SECONDARY GEOREF ++++++++++++" << endl;
    ac_georef_print(stderr, "", &georefSec);
    cerr << "+++++++++++++++++++++++++++++++++++++++++++" << endl;
  }

  // get state variables

  double lon = georefPrim.longitude;
  double lat = georefPrim.latitude;
  double altPresM = georefPrim.altitude_pres_m;
  double altGpsM = georefPrim.altitude_msl_km * 1000.0;
  double flightTime = georefPrim.flight_time_secs;

  double temp = georefPrim.temp_c;
  double pressure = georefPrim.pressure_hpa;
  double rh = georefPrim.rh_percent;
  double density = georefPrim.density_kg_m3;
  double vertVel = georefPrim.vert_velocity_mps;
  double aoa = georefPrim.angle_of_attack_deg;
  double ias = georefPrim.ind_airspeed_mps;
  double tas = georefPrim.true_airspeed_mps;
  
  double pitch = georefPrim.pitch_deg;
  double roll = georefPrim.roll_deg;
  double drift = georefPrim.drift_angle_deg;
  
  double accelNorm = georefPrim.accel_normal;
  double accelLat = georefPrim.accel_latitudinal;
  double accelLon = georefPrim.accel_longitudinal;

  double custom0 = georefPrim.custom[0];
  double custom1 = georefPrim.custom[1];
  double custom2 = georefPrim.custom[2];
  double custom3 = georefPrim.custom[3];
  double custom4 = georefPrim.custom[4];
  double custom5 = georefPrim.custom[5];
  double custom6 = georefPrim.custom[6];
  double custom7 = georefPrim.custom[7];
  double custom8 = georefPrim.custom[8];
  double custom9 = georefPrim.custom[9];

  double rollSec = georefSec.roll_deg;
  double pitchSec = georefSec.pitch_deg;
  double driftSec = georefSec.drift_angle_deg;
  double vertVelSec = georefSec.vert_velocity_mps;

  double tempSec = georefSec.temp_c;
  double custom0Sec = georefSec.custom[0];
  double custom1Sec = georefSec.custom[1];
  double custom2Sec = georefSec.custom[2];
  double custom3Sec = georefSec.custom[3];
  double custom4Sec = georefSec.custom[4];
  double custom5Sec = georefSec.custom[5];
  double custom6Sec = georefSec.custom[6];
  double custom7Sec = georefSec.custom[7];
  double custom8Sec = georefSec.custom[8];
  double custom9Sec = georefSec.custom[9];

  // compute differences (primary minus secondary)

  double lonDiff = georefPrim.longitude - georefSec.longitude;
  double latDiff = georefPrim.latitude - georefSec.latitude;
  
  double altDiff = georefPrim.altitude_msl_km - georefSec.altitude_msl_km;
  double ewVelDiff = georefPrim.ew_velocity_mps - georefSec.ew_velocity_mps;
  double nsVelDiff = georefPrim.ns_velocity_mps - georefSec.ns_velocity_mps;
  double vertVelDiff = georefPrim.vert_velocity_mps - georefSec.vert_velocity_mps;
  double ewWindDiff = georefPrim.ew_horiz_wind_mps - georefSec.ew_horiz_wind_mps;
  double nsWindDiff = georefPrim.ns_horiz_wind_mps - georefSec.ns_horiz_wind_mps;
  double vertWindDiff = georefPrim.vert_wind_mps - georefSec.vert_wind_mps;
  double pitchDiff = georefPrim.pitch_deg - georefSec.pitch_deg;
  double rollDiff = georefPrim.roll_deg - georefSec.roll_deg;
  double hdgDiff = georefPrim.heading_deg - georefSec.heading_deg;
  double driftDiff = georefPrim.drift_angle_deg - georefSec.drift_angle_deg;
  double trackDiff = georefPrim.track_deg - georefSec.track_deg;

  if (hdgDiff < -180) {
    hdgDiff += 360.0;
  } else if (hdgDiff > 180) {
    hdgDiff -= 360.0;
  }
  if (trackDiff < -180) {
    trackDiff += 360.0;
  } else if (trackDiff > 180) {
    trackDiff -= 360.0;
  }

  // estimate aircraft weight

  if (flightTime > 0) {
    if (_inInitialClimb) {
      _initialClimbSecs = flightTime;
      if (altPresM > (_topOfClimbAltitudeM * 0.95) && vertVel < 2.5) {
        _inInitialClimb = false;
        if (_params.debug) {
          cerr << "==>> top of climb, alt, vertVel: "
               << altPresM << ", " << vertVel << endl;
        }
      }
    }
    double fuelUsedKg = _initialClimbSecs * _fuelRateClimbKgPerSec;
    if (!_inInitialClimb) {
      fuelUsedKg += (flightTime - _initialClimbSecs) * _fuelRateCruiseKgPerSec;
    }
    _aircraftWtKg = _takeoffWtKg - fuelUsedKg;
  }

  // prepare text

  char oline[10000];
  string ostr;

  // date and time

  sprintf(oline, "%8d%s%.4d%s%.2d%s%.2d%s%.2d%s%.2d%s%.2d%s%ld%s%12.6f",
          _lineCount, _params.column_delimiter,
          timePrim.getYear(), _params.column_delimiter,
          timePrim.getMonth(), _params.column_delimiter,
          timePrim.getDay(), _params.column_delimiter,
          timePrim.getHour(), _params.column_delimiter,
          timePrim.getMin(), _params.column_delimiter,
          timePrim.getSec(), _params.column_delimiter,
          (long) timePrim.utime(), _params.column_delimiter,
          (double) timePrim.utime() / 86400.0);
  ostr += oline;

  // state variables

  ostr += _formatVal(lon, "%15.9f");
  ostr += _formatVal(lat, "%15.9f");

  ostr += _formatVal(altPresM, "%12.6f");
  ostr += _formatVal(altGpsM, "%12.6f");

  ostr += _formatVal(flightTime, "%12.6f");
  ostr += _formatVal(_aircraftWtKg, "%12.6f");

  ostr += _formatVal(temp, "%12.6f");
  ostr += _formatVal(pressure, "%12.6f");
  ostr += _formatVal(rh, "%12.6f");
  ostr += _formatVal(density, "%12.6f");
  ostr += _formatVal(vertVel, "%12.6f");
  ostr += _formatVal(aoa, "%12.6f");
  ostr += _formatVal(ias, "%12.6f");
  ostr += _formatVal(tas, "%12.6f");

  ostr += _formatVal(pitch, "%12.6f");
  ostr += _formatVal(roll, "%12.6f");
  ostr += _formatVal(drift, "%12.6f");

  ostr += _formatVal(accelNorm, "%12.6f");
  ostr += _formatVal(accelLat, "%12.6f");
  ostr += _formatVal(accelLon, "%12.6f");

  ostr += _formatVal(custom0, "%12.6f");
  ostr += _formatVal(custom1, "%12.6f");
  ostr += _formatVal(custom2, "%12.6f");
  ostr += _formatVal(custom3, "%12.6f");
  ostr += _formatVal(custom4, "%12.6f");
  ostr += _formatVal(custom5, "%12.6f");
  ostr += _formatVal(custom6, "%12.6f");
  ostr += _formatVal(custom7, "%12.6f");
  ostr += _formatVal(custom8, "%12.6f");
  ostr += _formatVal(custom9, "%12.6f");

  ostr += _formatVal(rollSec, "%12.6f");
  ostr += _formatVal(pitchSec, "%12.6f");
  ostr += _formatVal(driftSec, "%12.6f");
  ostr += _formatVal(vertVelSec, "%12.6f");

  ostr += _formatVal(tempSec, "%12.6f");
  ostr += _formatVal(custom0Sec, "%12.6f");
  ostr += _formatVal(custom1Sec, "%12.6f");
  ostr += _formatVal(custom2Sec, "%12.6f");
  ostr += _formatVal(custom3Sec, "%12.6f");
  ostr += _formatVal(custom4Sec, "%12.6f");
  ostr += _formatVal(custom5Sec, "%12.6f");
  ostr += _formatVal(custom6Sec, "%12.6f");
  ostr += _formatVal(custom7Sec, "%12.6f");
  ostr += _formatVal(custom8Sec, "%12.6f");
  ostr += _formatVal(custom9Sec, "%12.6f");

  // difference variables

  ostr += _formatVal(lonDiff, "%12.6f");
  ostr += _formatVal(latDiff, "%12.6f");
  ostr += _formatVal(altDiff, "%12.6f");

  ostr += _formatVal(ewVelDiff, "%12.6f");
  ostr += _formatVal(nsVelDiff, "%12.6f");
  ostr += _formatVal(vertVelDiff, "%12.6f");
  ostr += _formatVal(ewWindDiff, "%12.6f");
  ostr += _formatVal(nsWindDiff, "%12.6f");
  ostr += _formatVal(vertWindDiff, "%12.6f");

  ostr += _formatVal(pitchDiff, "%12.6f");
  ostr += _formatVal(rollDiff, "%12.6f");
  ostr += _formatVal(hdgDiff, "%12.6f");
  ostr += _formatVal(driftDiff, "%12.6f");
  ostr += _formatVal(trackDiff, "%12.6f");

  if (_lineCount == 0) {
    if (_params.write_commented_header) {
      _writeCommentedHeader(stdout);
    }
  }

  cout << ostr << endl;
  _lineCount++;

}

////////////////////////////////
// format a value for printing

string AcGeorefCompare::_formatVal(double val, const char * format)
  
{

  string ostr = _params.column_delimiter;
  char oline[1000];
  if (fabs(val - -9999.0) < 0.1) {
    val = NAN;
  }
  sprintf(oline, format, val);
  ostr += oline;
  return ostr;

}

///////////////////////////////////////
// write a commented header to stdout

void AcGeorefCompare::_writeCommentedHeader(FILE *out)

{

  const char *com = _params.comment_character;
  const char *delim = _params.column_delimiter;

  // initial line has column headers

  if (strlen(com) > 0) {
    fprintf(out, "%s%s", com, delim);
  }
  fprintf(out, "count%s", delim);

  fprintf(out, "year%s", delim);
  fprintf(out, "month%s", delim);
  fprintf(out, "day%s", delim);
  fprintf(out, "hour%s", delim);
  fprintf(out, "min%s", delim);
  fprintf(out, "sec%s", delim);
  fprintf(out, "unix_time%s", delim);
  fprintf(out, "unix_day%s", delim);
  
  fprintf(out, "lon%s", delim);
  fprintf(out, "lat%s", delim);
  fprintf(out, "altPresM%s", delim);
  fprintf(out, "altGpsM%s", delim);
  fprintf(out, "flightTime%s", delim);
  fprintf(out, "weightKg%s", delim);

  fprintf(out, "temp%s", delim);
  fprintf(out, "pressure%s", delim);
  fprintf(out, "rh%s", delim);
  fprintf(out, "density%s", delim);
  fprintf(out, "vertVel%s", delim);
  fprintf(out, "aoa%s", delim);
  fprintf(out, "ias%s", delim);
  fprintf(out, "tas%s", delim);

  fprintf(out, "pitch%s", delim);
  fprintf(out, "roll%s", delim);
  fprintf(out, "drift%s", delim);

  fprintf(out, "accelNorm%s", delim);
  fprintf(out, "accelLat%s", delim);
  fprintf(out, "accelLon%s", delim);

  fprintf(out, "custom0%s", delim);
  fprintf(out, "custom1%s", delim);
  fprintf(out, "custom2%s", delim);
  fprintf(out, "custom3%s", delim);
  fprintf(out, "custom4%s", delim);
  fprintf(out, "custom5%s", delim);
  fprintf(out, "custom6%s", delim);
  fprintf(out, "custom7%s", delim);
  fprintf(out, "custom8%s", delim);
  fprintf(out, "custom9%s", delim);

  fprintf(out, "rollSec%s", delim);
  fprintf(out, "pitchSec%s", delim);
  fprintf(out, "driftSec%s", delim);
  fprintf(out, "vertVelSec%s", delim);

  fprintf(out, "tempSec%s", delim);
  fprintf(out, "custom0Sec%s", delim);
  fprintf(out, "custom1Sec%s", delim);
  fprintf(out, "custom2Sec%s", delim);
  fprintf(out, "custom3Sec%s", delim);
  fprintf(out, "custom4Sec%s", delim);
  fprintf(out, "custom5Sec%s", delim);
  fprintf(out, "custom6Sec%s", delim);
  fprintf(out, "custom7Sec%s", delim);
  fprintf(out, "custom8Sec%s", delim);
  fprintf(out, "custom9Sec%s", delim);

  fprintf(out, "lonDiff%s", delim);
  fprintf(out, "latDiff%s", delim);
  fprintf(out, "altDiff%s", delim);

  fprintf(out, "ewVelDiff%s", delim);
  fprintf(out, "nsVelDiff%s", delim);
  fprintf(out, "vertVelDiff%s", delim);
  fprintf(out, "ewWindDiff%s", delim);
  fprintf(out, "nsWindDiff%s", delim);
  fprintf(out, "vertWindDiff%s", delim);

  fprintf(out, "pitchDiff%s", delim);
  fprintf(out, "rollDiff%s", delim);
  fprintf(out, "hdgDiff%s", delim);
  fprintf(out, "driftDiff%s", delim);
  fprintf(out, "trackDiff");

  fprintf(out, "\n");

}
  
